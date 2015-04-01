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

//	Some parts of this code are from Panasonic's NetCamAPI SDK.
//	Original copyright notice below:
/***************************************************************************
	Project		: Panasonic Network Camera SDK
	Subsystem	: SDK for HCM280
	Description	: Camera control API library.
	Date		: 2004-08-17
	Update		: 2004-08-17
	(C)COPYRIGHT 2003 Panasonic Communications Co.,Ltd.
 ***************************************************************************/


#ifndef PANACAMIF_H
#define PANACAMIF_H

#include "Camera.h"
#include "SyntroLib.h"
#include "base64.h"
#include "SyntroAV/SyntroAVDefs.h"

#define	NETCAM_CONNECT_TIMEOUT		(5 * SYNTRO_CLOCKS_PER_SEC)		// how long to wait between connection retries
#define	NETCAM_INACTIVE_TIMEOUT		(5 * SYNTRO_CLOCKS_PER_SEC)		// how long to wait with no frames
#define	NETCAM_BGND_INTERVAL		(SYNTRO_CLOCKS_PER_SEC/2)	// background interval rate - dictated by pan/tilt rate

#define	NETCAM_PAN_STEPS			10						// number of pan steps in each direction
#define	NETCAM_TILT_STEPS			7						// number of tilt steps

#define	NETCAM_PAN_CENTER			0						// home position
#define	NETCAM_TILT_CENTER			0						// home position
#define	NETCAM_ZOOM_CENTER			0						// home position
#define	NETCAM_PAN_SCALE			(SYNTRO_SERVO_RANGE / NETCAM_PAN_STEPS)	// scale factor for received settings
#define	NETCAM_TILT_SCALE			(SYNTRO_SERVO_RANGE / NETCAM_TILT_STEPS)	// scale factor for received settings

//	NetCam defs from SDK

#define		EOS		0										/* End of string */

#define HTTPC_OK						(0)
#define HTTPCE_SOCKET_ERROR				(-1)
#define HTTPCE_METHOD_NOT_SUPPORTED		(-2)
#define HTTPCE_PARSING_ERROR			(-4)

/* HTTP Method */
#define HTTP_GET		(0)
#define HTTP_POST		(1)
#define HTTP_PUT		(2)

/* Parsing State */
#define PST_STATUS_LINE		(0)
#define PST_HEADERS			(1)
#define PST_CONTENT			(2)

/* Connection Type */
#define HTTPH_CONN_KEEPALIVE	(0)
#define HTTPH_CONN_CLOSE		(1)

/* Presence of Proxy */
#define HTTP_NO_PROXY	(0)
#define HTTP_USE_PROXY	(1)


typedef struct http_st {			// Note: [M]-Mandatory, [O]-Optional
	int				method;			// [M] method or response code (recv)
	char*			version;		// [O] if null, "HTTP/1.1" will be assigned as default
	const char*		request_uri;	// [O] null terminated starting with '/'
	char*			target_name;	// [M] null terminated target HTTP server's name, used in Request-URI and Host header
	unsigned short	target_port;	// [O] target HTTP server's port number
	const char*		user_agent;		// [O] null terminated User-Agent header value
	int				proxy;			// [O] 0: no proxy, 1: use proxy
	int				conntype;		// [O] Connection type: 0: Keep-Alive, 1: close
	char*			auth_basic;		// [O] null terminated Authorization header value
	char*			content_type;	// [O] null terminated Content-type header value
	int				content_len;	// [O] Content-length header value
	char*			content;		// [O] Content data (treated as octet-stream)
} http_t;


#define MAX_SEND_BUFFER_LENGTH		(512)
#define MAX_RECV_BUFFER_LENGTH		(60000)

//	http command strings

static const char frmSetHome[]			=	"/nphControlCamera?Direction=HomePosition";
static const char frmSetPreset[]			=	"/nphControlCamera?Direction=Preset&PresetOperation=Move&Data=%d";
static const char frmSetZoomIn[]			=	"/nphControlCamera?Direction=ZoomTele";
static const char frmSetZoomOut[]			=	"/nphControlCamera?Direction=ZoomWide";
static const char frmSetPanleft[]			=	"/nphControlCamera?Direction=PanLeft";
static const char frmSetPanright[]			=	"/nphControlCamera?Direction=PanRight";
static const char frmSetTiltdown[]			=	"/nphControlCamera?Direction=TiltDown";
static const char frmSetTiltup[]			=	"/nphControlCamera?Direction=TiltUp";
static const char frmGetMotionJPEG[]		=	"/nphMotionJpeg?Resolution=%dx%d&Quality=Clarity";
static const char boundary[]				=	"--myboundary";
static const char frmUserAgent[]			=	"PCC_NWCAM_SDK";
static const char frmConnectionKeepAlive[]	=	"Keep-Alive";

#define CCAP_OK						(0)
#define CCAP_SOCKET_ERROR			(-1) // on_exit (from listener)
#define CCAP_ABORTED				(-2) // on_exit (from listener)
#define CCAP_MEMORY_ERROR			(-3) // on_exit (from listener)
#define CCAP_REMOTE_DISCONNECTED	(-4) // on_exit (from listener)
#define CCAP_NOT_AUTHORIZED			(-5) // on_exit (from parse_jpeg)
#define CCAP_JPEG_FRAME_TOO_LARGE	(-6) // on_exit (from parse_jpeg)
#define CCAP_COMPLETE				(-7) // on_exit (from on_receive)
#define CCAP_FRAME_COUNT_ERROR		(-8)
#define CCAP_HTTP_BUILD_ERROR		(-9)
#define JPEG_FRAME_BUFFER_SIZE		(1024*60)	/* 60KB at maximum for a JPEG frame */
#define CCAP_INFINITE				(-1)

typedef struct camaddr_st {
	char			targetname[SYNTRO_MAX_NONTAG];
	int				targetport;
	char			username[SYNTRO_MAX_NONTAG];
	char			password[SYNTRO_MAX_NONTAG];
} camaddr_t;

typedef struct {
	char			jpg[JPEG_FRAME_BUFFER_SIZE];
	int				len;
	unsigned long	ts;							/* Time Stamp in milliseconds */
} jpg_t;

typedef enum CAP_STATE_T {
	CAPS_IDLE = 0,		/* idle: TCP connection is not established or a request has not been sent. */
	CAPS_CONNECTING,	/* waiting for a response from a server */
	CAPS_SEEK_BOUNDARY,	/* seeking boundary '--myboundary' */
	CAPS_SEEK_CRLFCRLF,	/* seeking start of content '\n\n' */
} CAP_STATE;

typedef struct camcap_st {
	char			base64[64];
	CAP_STATE		capstate;
	jpg_t			jpg[2];
	int				procidx;						/* in process jpeg frame index (0 or 1) for member jpg[] */
	int				framecount;
	int				parse_framecount;
	int				parse_compi;
	int				parse_complfi;
	bool			parse_bContent;
	bool			parse_bCapturing;
	bool			parse_bFirstChar;
	int				parse_fblen;					/* stored data length in the frame buffer */
} camcap_t;


//	Connection states

enum 
{
	NETCAM_STATE_IDLE = 0,							// before open is called
	NETCAM_STATE_CONNECTING,						// connect has been sent
	NETCAM_STATE_CONNECTED,							// netcam is connected
	NETCAM_STATE_DISCONNECTED						// netcam has been disconnected
};

typedef struct
{
	int currentPan;									// the current pan position
	int requestedPan;								// the one wanted
	bool panInProgress;								// if panning now
	int currentTilt;								// the current tilt position
	int requestedTilt;								// the one wanted
	bool tiltInProgress;							// if tilting now
	int currentZoom;								// the current zoom position
	int requestedZoom;								// the one wanted
	bool zoomInProgress;							// if zooming now
	bool homeInProgress;							// if actually going home

	char *controlString;							// the control string that needs to be sent
} NETCAM_PTZ;

class PanaCamIF : public SyntroThread
{
	Q_OBJECT

public:
	PanaCamIF();
	virtual ~PanaCamIF();

	virtual QSize getImageSize();


signals:
	void newImage(QImage frame);
	void netcamStatus(QString status);

public slots:
	void onVideoConnect();
	void onVideoClose();
	void onVideoReceive();
	void onVideoError(QAbstractSocket::SocketError socketError);
	void onVideoState(QAbstractSocket::SocketState socketState);
	void onControlConnect();
	void onControlClose();
	void onControlReceive();
	void onControlError(QAbstractSocket::SocketError socketError);
	void onControlState(QAbstractSocket::SocketState socketState);
	void newCamera();

	void setPTZ(SYNTRO_PTZ * ptz);

protected:
	 void timerEvent(QTimerEvent *event);
	 void initThread();
	 void finishThread();

private:
	bool open();
	void close();
	int m_videoState;										// video connection state
	int m_controlState;										// control connection state
	int m_width;
	int m_height;
	int m_frameRate;

	bool m_open;
	QImage m_lastImage;										// used to store last image for when required
	int m_frameCount;										// number of frames received

	NETCAM_PTZ m_PTZ;										// PTZ control info

	camaddr_t m_camAddr;									// camera address structure
	camcap_t m_camcap;										// the capture structure
	QTcpSocket *m_videoSocket;								// the socket for the video connection
	QTcpSocket *m_controlSocket;							// the socket for the control connection
	http_t m_http;											// http structure
	bool m_waitingForControlResponse;
	qint64 m_noFrameTimer;									// used to timeout connection if no frames

	QMutex m_netCamLock;
	int m_timer;											// ID for background timer
	qint64 m_videoConnectTimer;								// used to timeout video connect attempts
	qint64 m_controlConnectTimer;							// used to timeout control connect attempts
	qint64 m_controlResponseTimer;							// used to timeout waiting for control responses

	void processNetcamImage(jpg_t *jpg);					// process a complete frame
	void clearInProgressFlags();

	void netcamVideoConnect();								// to connect to the camera for video
	void netcamControlConnect();							// connect for control
	void netcamStartStreaming();							// start streaming images
	void netcamSendControl(char *control);					// send a control string
	void netcamSetDefaultHTTP();
	int netcamHTTPBuild(char* message);
	int netcamHTTPParse(char *message, int len);			// parse a control response
	char* getNextLine(char* pos);
	char* trim(char* p);
	int netcamParseJPEG(char* rcvbuf, int len);				// parse a video response
	void netcamVideoDisconnect();							// clean up after a disconnect on the video channel
	void netcamControlDisconnect();							// clean up after a disconnect on the control channel

	char *m_recvBuff;										// where the received data goes
};

#endif // PANACAMIF_H
