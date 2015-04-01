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

#ifndef __BASE64_H__
#define __BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif

void base64enc(unsigned char *out, const unsigned char *in, int inlen);
int base64dec(char *out, const char *in, int maxlen);

#ifdef __cplusplus
};
#endif

#endif/*__BASE64_H__*/
