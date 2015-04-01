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

#include "SyntroPanaCam.h"
#include <qapplication.h>
#include <QtDebug>
#include <QSettings>

#include "SyntroUtils.h"
#include "PanaCamConsole.h"

int runGuiApp(int argc, char *argv[]);
int runConsoleApp(int argc, char *argv[]);
void loadSettings();

int main(int argc, char *argv[])
{
        if (SyntroUtils::checkConsoleModeFlag(argc, argv))
		return runConsoleApp(argc, argv);
	else
		return runGuiApp(argc, argv);
}

int runGuiApp(int argc, char *argv[])
{
	QApplication a(argc, argv);

	SyntroUtils::loadStandardSettings(PRODUCT_TYPE, a.arguments());
	loadSettings();

	SyntroPanaCam *w = new SyntroPanaCam();

	w->show();

	return a.exec();
}

int runConsoleApp(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	SyntroUtils::loadStandardSettings(PRODUCT_TYPE, a.arguments());
	loadSettings();

	PanaCamConsole wc(&a);

	return a.exec();
	return 1;
}

void loadSettings()
{
	QSettings *settings = SyntroUtils::getSettings();

	settings->beginGroup(SYNTROPANACAM_PANACAM_GROUP);

	if (!settings->contains(SYNTROPANACAM_CAMERA_IPADDRESS))
		settings->setValue(SYNTROPANACAM_CAMERA_IPADDRESS, "192.168.0.253");

	if (!settings->contains(SYNTROPANACAM_CAMERA_USERNAME))
		settings->setValue(SYNTROPANACAM_CAMERA_USERNAME, "admin");

	if (!settings->contains(SYNTROPANACAM_CAMERA_PASSWORD))
		settings->setValue(SYNTROPANACAM_CAMERA_PASSWORD, "default");

	if (!settings->contains(SYNTROPANACAM_CAMERA_TCPPORT))
		settings->setValue(SYNTROPANACAM_CAMERA_TCPPORT, 80);

	if (!settings->contains(SYNTROPANACAM_CAMERA_WIDTH))
		settings->setValue(SYNTROPANACAM_CAMERA_WIDTH, 640);

	if (!settings->contains(SYNTROPANACAM_CAMERA_HEIGHT))
		settings->setValue(SYNTROPANACAM_CAMERA_HEIGHT, 480);

	if (!settings->contains(SYNTROPANACAM_CAMERA_FRAMERATE))
		settings->setValue(SYNTROPANACAM_CAMERA_FRAMERATE, 30);

	settings->endGroup();
	delete settings;
}
