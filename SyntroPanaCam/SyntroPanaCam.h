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


#ifndef SYNTROPANACAM_H
#define SYNTROPANACAM_H

#include "ui_SyntroPanaCam.h"
#include "Camera.h"
#include <qlabel.h>
#include <qmutex.h>
#include <qqueue.h>

//	Settings keys

#define	PRODUCT_TYPE	"SyntroPanaCam"

class CameraClient;
class PanaCamIF;

class SyntroPanaCam : public QMainWindow
{
	Q_OBJECT

public:
	SyntroPanaCam();
	~SyntroPanaCam();

public slots:
	void onAbout();
	void onBasicSetup();
	void onConfigureCamera();
	void onConfigureMotion();
	void onConfigureStreams();
	void newImage(QImage grab);
	void netcamStatus(QString status);
	void clientRunning();

protected:
	void timerEvent(QTimerEvent *event);
	void closeEvent(QCloseEvent *event);

private:
	void showImage(const QImage& frame);
	bool createCamera();
	void clearQueue();
	void saveWindowState();
	void restoreWindowState();

	Ui::SyntroPanaCamClass ui;
	CameraClient *m_cameraClient;
	PanaCamIF *m_camera;
	QMutex m_frameQMutex;
	QQueue <QImage> m_frameQ;
	int m_frameRateTimer;
	int m_frameRefreshTimer;
	QLabel *m_frameRateStatus;
	QLabel *m_controlStatus;
	QLabel *m_netcamStatus;
	int m_frameCount;
	bool m_scaling;
	QLabel *m_cameraView;

};

#endif // SYNTROPANACAM_H
