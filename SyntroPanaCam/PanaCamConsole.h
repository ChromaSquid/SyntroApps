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


#ifndef PANACAMCONSOLE_H
#define PANACAMCONSOLE_H

#include <QThread>
#include "SyntroLib.h"
#include "Camera.h"

class	CameraClient;
class	PanaCamIF;

class PanaCamConsole : public QThread
{
	Q_OBJECT

public:
	PanaCamConsole(QObject *parent);
	~PanaCamConsole();

public slots:
	void newImage(QImage);
	void clientRunning();

protected:
	void run();
	void showHelp();
	void showStatus();

	void timerEvent(QTimerEvent *event);

	QObject *m_parent;
	CameraClient *m_client;
	PanaCamIF *m_camera;

	int m_frameCount;
	int m_frameRateTimer;
	double m_nfps;

	void startCapture();
	void stopCapture();
	bool createCamera();

	QString m_logTag;

};

#endif // PANACAMCONSOLE_H
