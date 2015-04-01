//
//  Copyright (c) 2014 richards-tech.
//
//  This file is part of SyntroNet
//
//  SyntroNet is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  SyntroNet is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with SyntroNet.  If not, see <http://www.gnu.org/licenses/>.
//

#include "PanaCamConsole.h"
#include "SyntroPanaCam.h"
#include "CameraClient.h"
#include "PanaCamIF.h"

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

PanaCamConsole::PanaCamConsole(QObject *parent)
	: QThread(parent)
{
	m_logTag = PRODUCT_TYPE;
	m_parent = parent;
	m_nfps = 0;
    m_camera = NULL;
	SyntroUtils::syntroAppInit();

	m_client = new CameraClient(this);
	m_client->resumeThread();
	connect(m_client, SIGNAL(running()), this, SLOT(clientRunning()), Qt::QueuedConnection);
    m_frameRateTimer = startTimer(3 * SYNTRO_CLOCKS_PER_SEC);

	start();
}

PanaCamConsole::~PanaCamConsole()
{
}

void PanaCamConsole::clientRunning()
{
	m_camera = new PanaCamIF();
	connect(m_camera, SIGNAL(newImage(QImage)), this, SLOT(newImage(QImage)), Qt::DirectConnection);
	connect(m_camera, SIGNAL(newImage(QImage)), m_client, SLOT(newImage(QImage)), Qt::DirectConnection);
    connect(m_client, SIGNAL(setPTZ(SYNTRO_PTZ *)), m_camera, SLOT(setPTZ(SYNTRO_PTZ *)), Qt::DirectConnection);
	m_client->setVideoFormat(640,480, 10);
    m_camera->resumeThread();
}

void PanaCamConsole::newImage(QImage)
{
	m_frameCount++;
}


void PanaCamConsole::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_frameRateTimer) 
	{
		QString fps;
		double count = m_frameCount;
		m_frameCount = 0;
		m_nfps =  count / 3.0;
	}
}

void PanaCamConsole::showHelp()
{
	printf("\nOptions are:\n\n");
	printf("  H - Show help\n");
	printf("  S - Show status\n");
	printf("  X - Exit\n");
}

void PanaCamConsole::showStatus()
{
	printf("\nSyntroControl status is:%s\n", qPrintable(m_client->getLinkState()));
	printf("Frame rate is    :%f fps\n", m_nfps);
}


void PanaCamConsole::run()
{
	bool	mustExit;

#ifndef WIN32
        struct termios	ctty;

        tcgetattr(fileno(stdout), &ctty);
        ctty.c_lflag &= ~(ICANON);
        tcsetattr(fileno(stdout), TCSANOW, &ctty);
#endif
	mustExit = false;
	while(!mustExit)
	{
                printf("\nEnter option: ");
#ifdef WIN32
		switch (toupper(_getch()))
#else
                switch (toupper(getchar()))
#endif
		{
			case 'H':					// help
				showHelp();
				break;

			case 'S':					// show status
				showStatus();
				break;

			case 'X':					// exit program
                printf("\nExiting\n");
				if (m_frameRateTimer) {
					killTimer(m_frameRateTimer);
					m_frameRateTimer = 0;
				}
				m_camera->exitThread();
				m_client->exitThread();
				mustExit = true;
				SyntroUtils::syntroAppExit();
				((QCoreApplication *)m_parent)->exit();
				break;

			case '\n':
				continue;
		}
	}
}
