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


#ifndef CAMERA_H
#define CAMERA_H

#include <QSize>
#include <QSettings>

#include "SyntroAV/SyntroAVDefs.h"

//		Settings keys

//	Stream group

#define	SYNTROPANACAM_STREAM_GROUP				"StreamGroup"				// group for stream-related entries

#define	SYNTROPANACAM_HIGHRATE_VIDEO_MININTERVAL	"HighRateVideoMinInterval"	// min frame interval in mS
#define	SYNTROPANACAM_HIGHRATE_VIDEO_MAXINTERVAL	"HighRateVideoMaxInterval"	// max full frame interval in mS
#define	SYNTROPANACAM_HIGHRATE_VIDEO_NULLINTERVAL "HighRateVideoNullInterval"	// max null frame interval in mS

#define	SYNTROPANACAM_LOWRATE_VIDEO_MININTERVAL	"LowRateVideoMinInterval"	// min interval in mS
#define	SYNTROPANACAM_LOWRATE_VIDEO_MAXINTERVAL	"LowRateVideoMaxInterval"	// max full frame interval in mS
#define	SYNTROPANACAM_LOWRATE_VIDEO_NULLINTERVAL "LowRateVideoNullInterval"	// max null frame interval in mS

#define SYNTROPANACAM_MOTION_GROUP				"MotionGroup"

#define	SYNTROPANACAM_MOTION_MIN_DELTA			"MotionMinDelta"			// min delta for image changed flag
#define SYNTROPANACAM_MOTION_DELTA_INTERVAL		"MotionDeltaInterval"
#define SYNTROPANACAM_MOTION_PREROLL			"MotionPreroll"
#define SYNTROPANACAM_MOTION_POSTROLL			"MotionPostroll"

//	PanaCam group

#define	SYNTROPANACAM_PANACAM_GROUP			"PanaCamGroup"					// group for camera related entries

#define	SYNTROPANACAM_CAMERA_IPADDRESS		"CameraIPAddress"				// camera IP address
#define	SYNTROPANACAM_CAMERA_USERNAME		"CameraUsername"				// username
#define	SYNTROPANACAM_CAMERA_PASSWORD		"CameraPassword"				// password
#define	SYNTROPANACAM_CAMERA_TCPPORT			"CameraTcpport"					// TCP port to use
#define	SYNTROPANACAM_CAMERA_WIDTH			"CameraWidth"					// image width
#define	SYNTROPANACAM_CAMERA_HEIGHT			"CameraHeight"					// image height
#define	SYNTROPANACAM_CAMERA_FRAMERATE		"CameraFrameRate"				// frame rate

#define SYNTROPANACAM_DEFAULT_DEVICE			"<default device>"

#endif // CAMERA_H
