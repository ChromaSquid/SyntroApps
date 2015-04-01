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

#include "PanaCamIF.h"

PanaCamIF::PanaCamIF() : SyntroThread("PanaCamIF", "SyntroPanaCam")
{
	m_videoState = NETCAM_STATE_IDLE;
	m_controlState = NETCAM_STATE_IDLE;
	m_recvBuff = NULL;
	m_videoSocket = NULL;
	m_controlSocket = NULL;
	m_open = false;
}

PanaCamIF::~PanaCamIF()
{
}

void PanaCamIF::initThread()
{
	m_videoSocket = new QTcpSocket();
	m_controlSocket = new QTcpSocket();
	connect(m_videoSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onVideoError(QAbstractSocket::SocketError)));
	connect(m_videoSocket, SIGNAL(stateChanged( QAbstractSocket::SocketState) ), this, SLOT(onVideoState( QAbstractSocket::SocketState)));
	connect(m_videoSocket, SIGNAL(connected()), this, SLOT(onVideoConnect()));
	connect(m_videoSocket, SIGNAL(disconnected()), this, SLOT(onVideoClose()));
	connect(m_videoSocket, SIGNAL(readyRead()), this, SLOT(onVideoReceive()));
	connect(m_controlSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onControlError(QAbstractSocket::SocketError)));
	connect(m_controlSocket, SIGNAL(stateChanged( QAbstractSocket::SocketState) ), this, SLOT(onControlState( QAbstractSocket::SocketState)));
	connect(m_controlSocket, SIGNAL(connected()), this, SLOT(onControlConnect()));
	connect(m_controlSocket, SIGNAL(disconnected()), this, SLOT(onControlClose()));
	connect(m_controlSocket, SIGNAL(readyRead()), this, SLOT(onControlReceive()));
	open();
}

void PanaCamIF::finishThread()
{
	if (m_videoSocket != NULL)
		delete m_videoSocket;
	if (m_controlSocket != NULL)
		delete m_controlSocket;
	close();
}

void PanaCamIF::newCamera()
{
	close();
	open();
}

bool PanaCamIF::open()
{
	QSettings *settings = SyntroUtils::getSettings();

	settings->beginGroup(SYNTROPANACAM_PANACAM_GROUP);

	memset(&m_camAddr, 0, sizeof(camaddr_t));
	strcpy(m_camAddr.targetname, qPrintable(settings->value(SYNTROPANACAM_CAMERA_IPADDRESS).toString()));
	m_camAddr.targetport = settings->value(SYNTROPANACAM_CAMERA_TCPPORT).toInt();
	strcpy(m_camAddr.username, qPrintable(settings->value(SYNTROPANACAM_CAMERA_USERNAME).toString()));
	strcpy(m_camAddr.password, qPrintable(settings->value(SYNTROPANACAM_CAMERA_PASSWORD).toString()));

	m_width = settings->value(SYNTROPANACAM_CAMERA_WIDTH).toInt();
	m_height = settings->value(SYNTROPANACAM_CAMERA_HEIGHT).toInt();
	m_frameRate = settings->value(SYNTROPANACAM_CAMERA_FRAMERATE).toInt();

	m_recvBuff = (char *)malloc(MAX_RECV_BUFFER_LENGTH);
	
	m_frameCount = 0;
	m_timer = startTimer(NETCAM_BGND_INTERVAL);
	netcamVideoConnect();
	emit netcamStatus(QString("Connecting to ") + QString(m_camAddr.targetname));
	memset(&m_PTZ, 0, sizeof(SYNTRO_PTZ));			
	m_PTZ.requestedPan = NETCAM_PAN_CENTER;						// force return to neutral position
	m_PTZ.requestedTilt = NETCAM_TILT_CENTER;					
	m_PTZ.requestedZoom = NETCAM_ZOOM_CENTER;		

	settings->endGroup();

	delete settings;
	m_open = true;
	return true;
}

void PanaCamIF::close()
{
	if (!m_open)
		return;
	if (m_recvBuff != NULL)
		free(m_recvBuff);
	m_recvBuff = NULL;
	killTimer(m_timer);
	netcamVideoDisconnect();
	netcamControlDisconnect();
	m_open = false;
}

QSize PanaCamIF::getImageSize()
{
	return QSize(m_width, m_height);
}


void PanaCamIF::processNetcamImage(jpg_t *jpg)
{
	m_netCamLock.lock();
	m_noFrameTimer = SyntroClock();

	m_lastImage.loadFromData(QByteArray(jpg->jpg, jpg->len), "JPEG");
	emit newImage(m_lastImage);
	m_netCamLock.unlock();
	m_frameCount++;
	emit netcamStatus(QString("Received %1 frames") .arg(m_frameCount));
}

void PanaCamIF::setPTZ(SYNTRO_PTZ * PTZ)
{
	m_PTZ.requestedPan = SyntroUtils::convertUC2ToUInt(PTZ->pan);
	m_PTZ.requestedTilt = SyntroUtils::convertUC2ToUInt(PTZ->tilt);
	m_PTZ.requestedZoom = SyntroUtils::convertUC2ToUInt(PTZ->zoom);

	// now modify settings for camera steps

	m_PTZ.requestedPan = (m_PTZ.requestedPan - SYNTRO_SERVO_CENTER) / NETCAM_PAN_SCALE;
	if (m_PTZ.requestedPan < -NETCAM_PAN_STEPS)
		m_PTZ.requestedPan = -NETCAM_PAN_STEPS;
	if (m_PTZ.requestedPan > NETCAM_PAN_STEPS)
		m_PTZ.requestedPan = NETCAM_PAN_STEPS;
	m_PTZ.requestedTilt = (m_PTZ.requestedTilt - SYNTRO_SERVO_CENTER) / NETCAM_TILT_SCALE;
	if (m_PTZ.requestedTilt < -NETCAM_TILT_STEPS)
		m_PTZ.requestedTilt = -NETCAM_TILT_STEPS;
	if (m_PTZ.requestedTilt > NETCAM_TILT_STEPS)
		m_PTZ.requestedTilt = NETCAM_TILT_STEPS;

}


void PanaCamIF::timerEvent(QTimerEvent *)
{
	switch (m_videoState) {
		case NETCAM_STATE_CONNECTED:
			if ((SyntroClock() - m_noFrameTimer) >= NETCAM_INACTIVE_TIMEOUT) {
				emit netcamStatus("Netcam timed out");
				netcamVideoDisconnect();
			}
			break;

		case NETCAM_STATE_CONNECTING:
			if ((SyntroClock() - m_videoConnectTimer) > NETCAM_INACTIVE_TIMEOUT) {
				netcamVideoDisconnect();
			}
			break;

		case NETCAM_STATE_DISCONNECTED:
			netcamVideoConnect();
			break;
	}	

	switch (m_controlState) {
		case NETCAM_STATE_IDLE:
			if (m_PTZ.currentPan != m_PTZ.requestedPan) {
				m_PTZ.panInProgress = true;
				if (m_PTZ.currentPan > m_PTZ.requestedPan)
					m_PTZ.controlString = (char *)frmSetPanright;
				else
					m_PTZ.controlString = (char *)frmSetPanleft;
				netcamControlConnect();
				break;
			}
			if (m_PTZ.currentTilt != m_PTZ.requestedTilt) {
				m_PTZ.tiltInProgress = true;
				if (m_PTZ.currentTilt > m_PTZ.requestedTilt)
					m_PTZ.controlString = (char *)frmSetTiltup;
				else
					m_PTZ.controlString = (char *)frmSetTiltdown;
				netcamControlConnect();
				break;
			}
			break;

		case NETCAM_STATE_CONNECTING:
			if ((SyntroClock() - m_controlConnectTimer) > NETCAM_INACTIVE_TIMEOUT) {
				netcamControlDisconnect();
				clearInProgressFlags();
			}
			break;
	}
}

void PanaCamIF::clearInProgressFlags()
{
	m_PTZ.panInProgress = false;
	m_PTZ.tiltInProgress = false;
	m_PTZ.zoomInProgress = false;
	m_PTZ.homeInProgress = false;
}

//----------------------------------------------------------
//	Low level netcam interface code
//

void PanaCamIF::netcamVideoConnect()
{
	m_videoState = NETCAM_STATE_CONNECTING;
	m_videoSocket->connectToHost(m_camAddr.targetname, m_camAddr.targetport);
	emit netcamStatus(QString("Connecting to Netcam ") + QString(m_camAddr.targetname));
	m_videoConnectTimer = SyntroClock();
}

void PanaCamIF::netcamControlConnect()
{
	m_controlState = NETCAM_STATE_CONNECTING;
	m_controlSocket->connectToHost(m_camAddr.targetname, m_camAddr.targetport);
	TRACE0("Connecting to Netcam on control channel");
	m_controlConnectTimer = SyntroClock();
}

void PanaCamIF::netcamVideoDisconnect()
{
	emit netcamStatus("Disconnected");
	if ((m_videoState == NETCAM_STATE_IDLE) || (m_videoState == NETCAM_STATE_DISCONNECTED)) 
		return;												// nothing to do
	m_videoState = NETCAM_STATE_DISCONNECTED;
	m_videoSocket->close();
}

void PanaCamIF::netcamControlDisconnect()
{
	clearInProgressFlags();
	TRACE0("Control channel disconnected");
	if (m_controlState == NETCAM_STATE_IDLE) 
		return;												// nothing to do
	m_controlState = NETCAM_STATE_IDLE;
	m_controlSocket->close();
}

void PanaCamIF::netcamStartStreaming()
{

	char* sendbuff;
	int sendlen;
	char command[256];

	emit netcamStatus("Sending streaming request");

	memset(&m_camcap, 0, sizeof(camcap_t));

	char szTemp[64];
	if (m_camAddr.username == 0 || m_camAddr.password == 0 )
		m_camcap.base64[0] = 0;
	else if (m_camAddr.username[0] == EOS )
		m_camcap.base64[0] = 0;
	else
	{
		sprintf(szTemp, "%s:%s", m_camAddr.username, m_camAddr.password);
		base64enc((unsigned char*)m_camcap.base64, (unsigned char*)szTemp, (int)strlen(szTemp));
	}

	m_camcap.capstate = CAPS_IDLE;
	m_camcap.procidx = 0;
	m_camcap.jpg[0].len = 0;
	m_camcap.jpg[1].len = 0;
	m_camcap.framecount = CCAP_INFINITE;
	m_camcap.parse_framecount = 0;
	m_camcap.parse_compi = 0;
	m_camcap.parse_complfi = 0;
	m_camcap.parse_bContent = false;
	m_camcap.parse_bCapturing = false;
	m_camcap.parse_bFirstChar = true;
	m_camcap.parse_fblen = 0;	

	/* Construct CGI command */
	sprintf(command, frmGetMotionJPEG, m_width, m_height);

	/* Allocate send buffer */
	sendbuff = (char *)malloc(MAX_SEND_BUFFER_LENGTH);

	/* Construct HTTP request message */
	netcamSetDefaultHTTP();
	m_http.request_uri = command;							// Set requested CGI command
	sendlen = netcamHTTPBuild(sendbuff);					// Build!
	if ( sendlen < 0 )
	{
		free(sendbuff);
		return;
	}

	/* Send message */

	m_camcap.capstate = CAPS_CONNECTING;
	m_videoSocket->write(sendbuff, sendlen);
	free(sendbuff);
	m_noFrameTimer = SyntroClock();
}

void PanaCamIF::netcamSendControl(char *control)
{

	char* sendbuff;
	int sendlen;
	char base64[64], baseTemp[64];

	TRACE1("Sending control %s", control);

	// do a check to see if need but be home but it isn't

	if ((m_PTZ.requestedPan == 0) && (m_PTZ.requestedTilt == 0) && 
			((m_PTZ.currentPan != 0) || (m_PTZ.currentTilt != 0))) {
		control = m_PTZ.controlString = (char *)frmSetHome;
		m_PTZ.homeInProgress = true;
		m_PTZ.panInProgress = false;
		m_PTZ.tiltInProgress = false;
	}

	sendbuff = (char *)malloc(MAX_SEND_BUFFER_LENGTH);

	if (m_camAddr.username == 0 || m_camAddr.password == 0 )
		base64[0] = 0;
	else if (m_camAddr.username[0] == EOS ) {
		base64[0] = 0;
	} else {
		sprintf(baseTemp, "%s:%s", m_camAddr.username, m_camAddr.password);
		base64enc((unsigned char*)base64, (unsigned char*)baseTemp, (int)strlen(baseTemp));
	}

	/* Construct HTTP request message */
	netcamSetDefaultHTTP();
	m_http.auth_basic = base64;
	m_http.request_uri = control;						
	sendlen = netcamHTTPBuild(sendbuff);		
	if ( sendlen < 0 )
	{
		free(sendbuff);
		return;
	}

	/* Send message */

	m_waitingForControlResponse = true;
	m_controlSocket->write(sendbuff, sendlen);
	free(sendbuff);
	m_controlResponseTimer = SyntroClock();
}

void PanaCamIF::netcamSetDefaultHTTP()
{
	/* set all zero */
	memset(&m_http, 0, sizeof(http_t));

	/* Construct HTTP request message */
	m_http.method = HTTP_GET;
	m_http.user_agent = frmUserAgent;
	m_http.target_name = m_camAddr.targetname;
	m_http.target_port = m_camAddr.targetport;
	m_http.conntype = HTTPH_CONN_KEEPALIVE;

	if ( m_camcap.base64[0] != 0 )
		m_http.auth_basic = m_camcap.base64;
}



int PanaCamIF::netcamParseJPEG(char* rcvbuf, int len)
{
	char ver[9], status[4], phrase[32];
	int i;
	int blen = (int)strlen(boundary);	/* length of bounday string */


	/* start parsing...*/

	for ( i=0 ; i<len ; i++ ) {
		switch (m_camcap.capstate) {
			case CAPS_CONNECTING:
				sscanf(rcvbuf, "%s %s %s", ver, status, phrase);
				if (strcmp(status, "200") == 0 ) {
					char* pos = strchr(rcvbuf, '\n');
					if ( pos )
						i = (int)(pos - rcvbuf);
					m_camcap.parse_framecount = 0;
					m_camcap.parse_compi = 0;
					m_camcap.parse_complfi = 0;
					m_camcap.parse_bContent = false;
					m_camcap.parse_bCapturing = false;
					m_camcap.parse_bFirstChar = true;
					m_camcap.parse_fblen = 0;

					m_camcap.capstate = CAPS_SEEK_CRLFCRLF;
				} else {
					return CCAP_NOT_AUTHORIZED;
				}
				break;

			case CAPS_SEEK_BOUNDARY:
				if (m_camcap.parse_bCapturing) {
					if (m_camcap.parse_fblen >= JPEG_FRAME_BUFFER_SIZE)
						return CCAP_JPEG_FRAME_TOO_LARGE;

					m_camcap.jpg[m_camcap.procidx].jpg[m_camcap.parse_fblen] = rcvbuf[i];

					/* debug purpose. to be removed. */
					if (m_camcap.parse_fblen == 0 && rcvbuf[i] == 8) {
						m_camcap.parse_compi = 0;
						m_camcap.parse_bContent = false;
						m_camcap.parse_bCapturing = false;
						m_camcap.parse_bFirstChar = true;
						m_camcap.capstate = CAPS_SEEK_CRLFCRLF;
						break;
					}

					m_camcap.parse_fblen++;
				}

				if ( rcvbuf[i] == boundary[m_camcap.parse_compi] && (m_camcap.parse_compi > 0 ||
								(m_camcap.parse_bFirstChar && m_camcap.parse_compi == 0)) ) {
					m_camcap.parse_compi++;
					if (m_camcap.parse_compi == blen ) {
						m_camcap.parse_compi = 0;
						m_camcap.capstate = CAPS_SEEK_CRLFCRLF;

						if (m_camcap.parse_bCapturing) {
							m_camcap.jpg[m_camcap.procidx].len = m_camcap.parse_fblen/*Stored Buffer Length*/ - blen/*Boundary Length*/ - 2/*CRLF*/;


							/****************************
							*  Got a complete frame!!  *
							 ****************************/

							processNetcamImage(&(m_camcap.jpg[m_camcap.procidx]));
							m_camcap.parse_framecount++;

							/* initializing for the next frame */
							switch (m_camcap.procidx) {
								case 0 :
									m_camcap.procidx = 1;
									break;
						
								case 1 :	
									m_camcap.procidx = 0;	
									break;
						
								default:	
									m_camcap.procidx = 0;	
									break;	
							}

							m_camcap.parse_bCapturing = false;
							m_camcap.jpg[m_camcap.procidx].len = 0;
							m_camcap.parse_fblen = 0;
						}
					}
				} else {
					m_camcap.parse_compi = 0;
				}

				break;

			case CAPS_SEEK_CRLFCRLF:
				switch (m_camcap.parse_complfi) {
					case 0:
						if (rcvbuf[i] == '\r') /*CR*/
							m_camcap.parse_complfi++;
						break;

					case 1:
						if (rcvbuf[i] == '\n') /*LF*/
							m_camcap.parse_complfi++;
						else
							m_camcap.parse_complfi = 0;
						break;

					case 2:
						if ( rcvbuf[i] == '\r') /*CR*/
							m_camcap.parse_complfi++;
						else
							m_camcap.parse_complfi = 0;
						break;

					case 3:
						if ( rcvbuf[i] == '\n') /*LF*/ {
							if (m_camcap.parse_bContent )
								m_camcap.parse_bCapturing = true;
							else
								m_camcap.parse_bContent = true;

							m_camcap.capstate = CAPS_SEEK_BOUNDARY;
						}

						m_camcap.parse_complfi = 0;
						break;
				}
				break;

			default:
				break;
			}

			if ( rcvbuf[i] == '\n' )
				m_camcap.parse_bFirstChar = true;
			else
				m_camcap.parse_bFirstChar = false;

		//if ( is_stop_request() )
		//	break;

	}/*for*/

	return m_camcap.parse_framecount;
}


int PanaCamIF::netcamHTTPBuild(char* message)
{
	char* pos = message;

	/* add method */
	switch (m_http.method ) {
		case HTTP_GET:
			sprintf(pos, "GET ");	pos += 4;
			break;

		case HTTP_POST:
			sprintf(pos, "POST ");	pos += 5;
			break;

		case HTTP_PUT:
			sprintf(pos, "PUT ");	pos += 4;
			break;

		default:
			return HTTPCE_METHOD_NOT_SUPPORTED;
	}
	sprintf(pos, "%s HTTP/1.1\r\n", m_http.request_uri);

	pos += (int)strlen(pos);

	/* add user-agent header */
	if (m_http.user_agent )
		sprintf(pos, "User-Agent: %s\r\n", m_http.user_agent);

	pos += (int)strlen(pos);

	/* add host header */
	if (m_http.target_port == 80 )
		sprintf(pos, "Host: %s\r\n", m_http.target_name);
	else
		sprintf(pos, "Host: %s:%d\r\n", m_http.target_name, m_http.target_port);

	pos += (int)strlen(pos);

	if (m_http.conntype == HTTPH_CONN_KEEPALIVE )
		sprintf(pos, "Connection: Keep-Alive\r\n");
	else
		sprintf(pos, "Connection: close\r\n");

	pos += (int)strlen(pos);

	/* add authorization header */
	if (m_http.auth_basic )
		if (m_http.auth_basic[0] != EOS )
		{
			sprintf(pos, "Authorization: Basic %s\r\n", m_http.auth_basic);
			pos += (int)strlen(pos);
		}

	/* add authorization header */
	if (m_http.content_type )
	{
		sprintf(pos, "Content-Type: %s\r\n", m_http.content_type);
		pos += (int)strlen(pos);
	}

	/* add content length and CRLFCRLF*/
	if (m_http.content_len > 0 )
		sprintf(pos, "Content-Length: %d\r\n\r\n", m_http.content_len);
	else
		sprintf(pos, "\r\n");

	pos += (int)strlen(pos);

	/* add content */
	if (m_http.content_len > 0 )
	{
		memcpy(pos, m_http.content, m_http.content_len);
		pos += m_http.content_len;
	}

	return (int)(pos - message);
}

int PanaCamIF::netcamHTTPParse(char *message, int len)
{
	char *pos, *next, *ep;
	int state = PST_STATUS_LINE;
	int done = false;

	pos = message;

	memset(&m_http, 0, sizeof(http_t));

	while ( !done )
	{
		next = getNextLine(pos);

		switch ( state )
		{
		case PST_STATUS_LINE:
			pos = trim(pos);
			m_http.version = pos;
			if ( (ep = strchr(pos, ' ')) == NULL )
				return HTTPCE_PARSING_ERROR;
			*ep = EOS;
			pos = ep + 1;
			if ( (ep = strchr(pos, ' ')) == NULL )
				return HTTPCE_PARSING_ERROR;
			*ep = EOS;
			m_http.method = atoi(pos);	// method is used for resp code
			m_http.request_uri = ep + 1;	// req-uri is used for reason phrase
			state = PST_HEADERS;
			pos = next;
			break;

		case PST_HEADERS:
			if ( strlen(pos) == 0 )
			{
				state = PST_CONTENT;
			}
			else
			{
				/* TODO: Implement here for parsing headers.. */
			}
			
			pos = next;
			if ( pos == NULL )
			{
				m_http.content_len = 0;
				m_http.content = NULL;
				done = true;
			}
			break;

		case PST_CONTENT:
			m_http.content = pos;
			m_http.content_len = len - (int)(pos - message);
			done = true;
			break;
		}
	}

	return HTTPC_OK;
}

char* PanaCamIF::getNextLine(char* pos)
{
	char *ep = pos;
	if ( pos == NULL )
		return NULL;

	ep = strchr(pos, '\r');
	if ( ep == NULL ) {
		return NULL;
	}

	*ep = EOS;
	ep++;
//	assert(*ep == '\n');
	ep++;

	return ep;
}

char* PanaCamIF::trim(char* p)
{
	char *sp, *ep;

	sp = p;
	ep = sp + strlen(sp);	/* pointing EOS */

	if (sp < ep)	
		ep--;

	if (sp == ep)
	{
		if (*sp == ' ' || *sp == '\t' || *sp == '\r' || *sp == '\n')
			return ++sp;
		return sp;
	}

	/* trim head */
	while (sp < ep)
	{
		if (*sp == ' ' || *sp == '\t' || *sp == '\r' || *sp == '\n')
			sp++;
		else
			break;
	}

	/* trim tail */
	while (sp < ep)
	{
		if (*ep == ' ' || *ep == '\t' || *ep == '\r' || *ep == '\n')
			ep--;
		else
			break;
	}

	*(ep + 1) = EOS;
	return sp;
}

//----------------------------------------------------------
//	socket slots
//

void PanaCamIF::onVideoConnect()
{
	m_videoState = NETCAM_STATE_CONNECTED;
	netcamStartStreaming();
}

void PanaCamIF::onVideoClose()
{
	m_videoState = NETCAM_STATE_DISCONNECTED;
	m_videoSocket->close();
	emit netcamStatus("Disconnected");
}

void PanaCamIF::onVideoReceive()
{
	int parse_res;
	int	numRead;

	while(1) {
		numRead = m_videoSocket->read(m_recvBuff, MAX_RECV_BUFFER_LENGTH-1);
		if (numRead < 0)
		{
			onVideoClose();
			return;
		}
		if (numRead == 0) {
			return;
		}
		parse_res = netcamParseJPEG(m_recvBuff, numRead);
		if ( parse_res < 0 )
		{
			onVideoClose();
			return;	
		}
	}
}

void PanaCamIF::onVideoError(QAbstractSocket::SocketError)
{
	logError(QString("Netcam TCP socket error %1").arg(m_videoSocket->errorString()));
}

void PanaCamIF::onVideoState(QAbstractSocket::SocketState socketState)
{
	TRACE1("Netcam TCP socket state %d", socketState);
	if ((socketState == QAbstractSocket::UnconnectedState) && (m_videoState != NETCAM_STATE_DISCONNECTED) &&
						(m_videoState != NETCAM_STATE_IDLE))
		netcamVideoDisconnect();							// no signal generated in this situation
}


void PanaCamIF::onControlConnect()
{
	m_controlState = NETCAM_STATE_CONNECTED;
	netcamSendControl(m_PTZ.controlString);
}

void PanaCamIF::onControlClose()
{
	clearInProgressFlags();
	m_controlState = NETCAM_STATE_IDLE;
	m_controlSocket->close();
	TRACE0("Control disconnected");
}

void PanaCamIF::onControlReceive()
{
	int	numRead;
	int totalRead;
	char *recvBuf;

	if (!m_waitingForControlResponse)
		return;					// just ignore

	recvBuf = (char *)calloc(MAX_RECV_BUFFER_LENGTH, 1);
	totalRead = 0;
	while(1) {
		numRead = m_controlSocket->read(recvBuf + totalRead, MAX_RECV_BUFFER_LENGTH - totalRead - 1);
		if (numRead < 0) {
			logError(QString("Control channel read returned error %1").arg(numRead));
			onControlClose();
			free(recvBuf);
			return;
		}
		totalRead += numRead;
		if (numRead == 0) {
			m_waitingForControlResponse = false;
			if (netcamHTTPParse(recvBuf, totalRead) != HTTPC_OK) {
				logError("Control channel response parse error reported");
				free(recvBuf);
				return;
			}
			free(recvBuf);

			if (m_http.method == 401) {
				logError("Control channel reported user not authorized");
				return;
			}

			if (m_http.method != 200) {
				logError("Control channel reported error");
				return;
			}
			if (m_PTZ.homeInProgress) {
				m_PTZ.currentPan = NETCAM_PAN_CENTER;
				m_PTZ.currentTilt = NETCAM_TILT_CENTER;
			}
			if (m_PTZ.panInProgress) {
				if (m_PTZ.currentPan < m_PTZ.requestedPan)
					m_PTZ.currentPan++;
				else
					m_PTZ.currentPan--;
			}
			if (m_PTZ.tiltInProgress) {
				if (m_PTZ.currentTilt < m_PTZ.requestedTilt)
					m_PTZ.currentTilt++;
				else
					m_PTZ.currentTilt--;
			}
			if (m_PTZ.zoomInProgress) {
				if (m_PTZ.currentZoom < m_PTZ.requestedZoom)
					m_PTZ.currentZoom++;
				else
					m_PTZ.currentZoom--;
			}
			clearInProgressFlags();
			return;
		}
	}
}

void PanaCamIF::onControlError(QAbstractSocket::SocketError)
{
	logError(QString("Netcam control socket error %1").arg(m_controlSocket->errorString()));
}

void PanaCamIF::onControlState(QAbstractSocket::SocketState socketState)
{
	TRACE1("Netcam control socket state %d", socketState);
	if ((socketState == QAbstractSocket::UnconnectedState) && (m_controlState != NETCAM_STATE_IDLE))
		netcamControlDisconnect();							// no signal generated in this situation
}
