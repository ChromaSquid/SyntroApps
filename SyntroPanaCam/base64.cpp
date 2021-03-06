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

#include "base64.h"
#include "ctype.h"

static const char base64digits[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BAD	-1
static const char base64val[] = {
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
     52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
     41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};

#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)


/* raw bytes in quasi-big-endian order to base 64 string (NUL-terminated) */
void base64enc(unsigned char *out, const unsigned char *in, int inlen)
{
	for (; inlen >= 3; inlen -= 3)
	{
		*out++ = base64digits[in[0] >> 2];
		*out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
		*out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
		*out++ = base64digits[in[2] & 0x3f];
		in += 3;
	}
	if (inlen > 0)
	{
		unsigned char fragment;

		*out++ = base64digits[in[0] >> 2];
		fragment = (in[0] << 4) & 0x30;
		if (inlen > 1)
			fragment |= in[1] >> 4;
		*out++ = base64digits[fragment];
		*out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
		*out++ = '=';
	}
	*out = '\0';
}

/* base 64 to raw bytes in quasi-big-endian order, returning count of bytes */
/* maxlen limits output buffer size, set to zero to ignore */
int base64dec(char *out, const char *in, int maxlen)
{
    int len = 0;
    register unsigned char digit1, digit2, digit3, digit4;

    if (in[0] == '+' && in[1] == ' ')
	in += 2;
    if (*in == '\r')
	return(0);

    do 
	{
		digit1 = in[0];
		if (DECODE64(digit1) == BAD)
			return(-1);
		digit2 = in[1];
		if (DECODE64(digit2) == BAD)
			return(-1);
		digit3 = in[2];
		if (digit3 != '=' && DECODE64(digit3) == BAD)
			return(-1); 
		digit4 = in[3];
		if (digit4 != '=' && DECODE64(digit4) == BAD)
			return(-1);
		in += 4;
		++len;
		if (maxlen && len > maxlen)
			return(-1);
		*out++ = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
		if (digit3 != '=')
		{
			++len;
			if (maxlen && len > maxlen)
				return(-1);
			*out++ = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
			if (digit4 != '=')
			{
				++len;
			if (maxlen && len > maxlen)
				return(-1);
			*out++ = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
			}
		}
    } while (*in && *in != '\r' && digit4 != '=');

    return (len);
}
