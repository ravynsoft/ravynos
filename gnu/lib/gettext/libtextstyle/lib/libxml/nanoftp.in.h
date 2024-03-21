/* libxml2 - Library for parsing XML documents
 * Copyright (C) 2006-2019 Free Software Foundation, Inc.
 *
 * This file is not part of the GNU gettext program, but is used with
 * GNU gettext.
 *
 * The original copyright notice is as follows:
 */

/*
 * Copyright (C) 1998-2012 Daniel Veillard.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is fur-
 * nished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
 * NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author: Daniel Veillard
 */

/*
 * Summary: minimal FTP implementation
 * Description: minimal FTP implementation allowing to fetch resources
 *              like external subset.
 */

#ifndef __NANO_FTP_H__
#define __NANO_FTP_H__

#include <libxml/xmlversion.h>

#ifdef LIBXML_FTP_ENABLED

/* Needed for portability to Windows 64 bits */
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <winsock2.h>
#else
/**
 * SOCKET:
 *
 * macro used to provide portability of code to windows sockets
 */
#define SOCKET int
/**
 * INVALID_SOCKET:
 *
 * macro used to provide portability of code to windows sockets
 * the value to be used when the socket is not valid
 */
#undef  INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ftpListCallback:
 * @userData:  user provided data for the callback
 * @filename:  the file name (including "->" when links are shown)
 * @attrib:  the attribute string
 * @owner:  the owner string
 * @group:  the group string
 * @size:  the file size
 * @links:  the link count
 * @year:  the year
 * @month:  the month
 * @day:  the day
 * @hour:  the hour
 * @minute:  the minute
 *
 * A callback for the xmlNanoFTPList command.
 * Note that only one of year and day:minute are specified.
 */
typedef void (*ftpListCallback) (void *userData,
	                         const char *filename, const char *attrib,
	                         const char *owner, const char *group,
				 unsigned long size, int links, int year,
				 const char *month, int day, int hour,
				 int minute);
/**
 * ftpDataCallback:
 * @userData: the user provided context
 * @data: the data received
 * @len: its size in bytes
 *
 * A callback for the xmlNanoFTPGet command.
 */
typedef void (*ftpDataCallback) (void *userData,
				 const char *data,
				 int len);

/*
 * Init
 */
XMLPUBFUN void XMLCALL
	xmlNanoFTPInit		(void);
XMLPUBFUN void XMLCALL
	xmlNanoFTPCleanup	(void);

/*
 * Creating/freeing contexts.
 */
XMLPUBFUN void * XMLCALL
	xmlNanoFTPNewCtxt	(const char *URL);
XMLPUBFUN void XMLCALL
	xmlNanoFTPFreeCtxt	(void * ctx);
XMLPUBFUN void * XMLCALL
	xmlNanoFTPConnectTo	(const char *server,
				 int port);
/*
 * Opening/closing session connections.
 */
XMLPUBFUN void * XMLCALL
	xmlNanoFTPOpen		(const char *URL);
XMLPUBFUN int XMLCALL
	xmlNanoFTPConnect	(void *ctx);
XMLPUBFUN int XMLCALL
	xmlNanoFTPClose		(void *ctx);
XMLPUBFUN int XMLCALL
	xmlNanoFTPQuit		(void *ctx);
XMLPUBFUN void XMLCALL
	xmlNanoFTPScanProxy	(const char *URL);
XMLPUBFUN void XMLCALL
	xmlNanoFTPProxy		(const char *host,
				 int port,
				 const char *user,
				 const char *passwd,
				 int type);
XMLPUBFUN int XMLCALL
	xmlNanoFTPUpdateURL	(void *ctx,
				 const char *URL);

/*
 * Rather internal commands.
 */
XMLPUBFUN int XMLCALL
	xmlNanoFTPGetResponse	(void *ctx);
XMLPUBFUN int XMLCALL
	xmlNanoFTPCheckResponse	(void *ctx);

/*
 * CD/DIR/GET handlers.
 */
XMLPUBFUN int XMLCALL
	xmlNanoFTPCwd		(void *ctx,
				 const char *directory);
XMLPUBFUN int XMLCALL
	xmlNanoFTPDele		(void *ctx,
				 const char *file);

XMLPUBFUN SOCKET XMLCALL
	xmlNanoFTPGetConnection	(void *ctx);
XMLPUBFUN int XMLCALL
	xmlNanoFTPCloseConnection(void *ctx);
XMLPUBFUN int XMLCALL
	xmlNanoFTPList		(void *ctx,
				 ftpListCallback callback,
				 void *userData,
				 const char *filename);
XMLPUBFUN SOCKET XMLCALL
	xmlNanoFTPGetSocket	(void *ctx,
				 const char *filename);
XMLPUBFUN int XMLCALL
	xmlNanoFTPGet		(void *ctx,
				 ftpDataCallback callback,
				 void *userData,
				 const char *filename);
XMLPUBFUN int XMLCALL
	xmlNanoFTPRead		(void *ctx,
				 void *dest,
				 int len);

#ifdef __cplusplus
}
#endif
#endif /* LIBXML_FTP_ENABLED */
#endif /* __NANO_FTP_H__ */
