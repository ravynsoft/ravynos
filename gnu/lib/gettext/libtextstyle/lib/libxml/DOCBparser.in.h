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
 * Summary: old DocBook SGML parser
 * Description: interface for a DocBook SGML non-verifying parser
 * This code is DEPRECATED, and should not be used anymore.
 */

#ifndef __DOCB_PARSER_H__
#define __DOCB_PARSER_H__
#include <libxml/xmlversion.h>

#ifdef LIBXML_DOCB_ENABLED

#include <libxml/parser.h>
#include <libxml/parserInternals.h>

#ifndef IN_LIBXML
#ifdef __GNUC__
#warning "The DOCBparser module has been deprecated in libxml2-2.6.0"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Most of the back-end structures from XML and SGML are shared.
 */
typedef xmlParserCtxt docbParserCtxt;
typedef xmlParserCtxtPtr docbParserCtxtPtr;
typedef xmlSAXHandler docbSAXHandler;
typedef xmlSAXHandlerPtr docbSAXHandlerPtr;
typedef xmlParserInput docbParserInput;
typedef xmlParserInputPtr docbParserInputPtr;
typedef xmlDocPtr docbDocPtr;

/*
 * There is only few public functions.
 */
XMLPUBFUN int XMLCALL
		     docbEncodeEntities(unsigned char *out,
                                        int *outlen,
                                        const unsigned char *in,
                                        int *inlen, int quoteChar);

XMLPUBFUN docbDocPtr XMLCALL
		     docbSAXParseDoc   (xmlChar *cur,
                                        const char *encoding,
                                        docbSAXHandlerPtr sax,
                                        void *userData);
XMLPUBFUN docbDocPtr XMLCALL
		     docbParseDoc      (xmlChar *cur,
                                        const char *encoding);
XMLPUBFUN docbDocPtr XMLCALL
		     docbSAXParseFile  (const char *filename,
                                        const char *encoding,
                                        docbSAXHandlerPtr sax,
                                        void *userData);
XMLPUBFUN docbDocPtr XMLCALL
		     docbParseFile     (const char *filename,
                                        const char *encoding);

/**
 * Interfaces for the Push mode.
 */
XMLPUBFUN void XMLCALL
		     docbFreeParserCtxt      (docbParserCtxtPtr ctxt);
XMLPUBFUN docbParserCtxtPtr XMLCALL
		     docbCreatePushParserCtxt(docbSAXHandlerPtr sax,
                                              void *user_data,
                                              const char *chunk,
                                              int size,
                                              const char *filename,
                                              xmlCharEncoding enc);
XMLPUBFUN int XMLCALL
		     docbParseChunk          (docbParserCtxtPtr ctxt,
                                              const char *chunk,
                                              int size,
                                              int terminate);
XMLPUBFUN docbParserCtxtPtr XMLCALL
		     docbCreateFileParserCtxt(const char *filename,
                                              const char *encoding);
XMLPUBFUN int XMLCALL
		     docbParseDocument       (docbParserCtxtPtr ctxt);

#ifdef __cplusplus
}
#endif

#endif /* LIBXML_DOCB_ENABLED */

#endif /* __DOCB_PARSER_H__ */
