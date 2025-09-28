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
 * Summary: minimal HTTP implementation
 * Description: minimal HTTP implementation allowing to fetch resources
 *              like external subset.
 */

#ifndef __NANO_HTTP_H__
#define __NANO_HTTP_H__

#include <libxml/xmlversion.h>

#ifdef LIBXML_HTTP_ENABLED

#ifdef __cplusplus
extern "C" {
#endif
XMLPUBFUN void XMLCALL
	xmlNanoHTTPInit		(void);
XMLPUBFUN void XMLCALL
	xmlNanoHTTPCleanup	(void);
XMLPUBFUN void XMLCALL
	xmlNanoHTTPScanProxy	(const char *URL);
XMLPUBFUN int XMLCALL
	xmlNanoHTTPFetch	(const char *URL,
				 const char *filename,
				 char **contentType);
XMLPUBFUN void * XMLCALL
	xmlNanoHTTPMethod	(const char *URL,
				 const char *method,
				 const char *input,
				 char **contentType,
				 const char *headers,
				 int   ilen);
XMLPUBFUN void * XMLCALL
	xmlNanoHTTPMethodRedir	(const char *URL,
				 const char *method,
				 const char *input,
				 char **contentType,
				 char **redir,
				 const char *headers,
				 int   ilen);
XMLPUBFUN void * XMLCALL
	xmlNanoHTTPOpen		(const char *URL,
				 char **contentType);
XMLPUBFUN void * XMLCALL
	xmlNanoHTTPOpenRedir	(const char *URL,
				 char **contentType,
				 char **redir);
XMLPUBFUN int XMLCALL
	xmlNanoHTTPReturnCode	(void *ctx);
XMLPUBFUN const char * XMLCALL
	xmlNanoHTTPAuthHeader	(void *ctx);
XMLPUBFUN const char * XMLCALL
	xmlNanoHTTPRedir	(void *ctx);
XMLPUBFUN int XMLCALL
	xmlNanoHTTPContentLength( void * ctx );
XMLPUBFUN const char * XMLCALL
	xmlNanoHTTPEncoding	(void *ctx);
XMLPUBFUN const char * XMLCALL
	xmlNanoHTTPMimeType	(void *ctx);
XMLPUBFUN int XMLCALL
	xmlNanoHTTPRead		(void *ctx,
				 void *dest,
				 int len);
#ifdef LIBXML_OUTPUT_ENABLED
XMLPUBFUN int XMLCALL
	xmlNanoHTTPSave		(void *ctxt,
				 const char *filename);
#endif /* LIBXML_OUTPUT_ENABLED */
XMLPUBFUN void XMLCALL
	xmlNanoHTTPClose	(void *ctx);
#ifdef __cplusplus
}
#endif

#endif /* LIBXML_HTTP_ENABLED */
#endif /* __NANO_HTTP_H__ */
