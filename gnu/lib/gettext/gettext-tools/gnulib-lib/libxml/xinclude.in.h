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
 * Summary: implementation of XInclude
 * Description: API to handle XInclude processing,
 * implements the
 * World Wide Web Consortium Last Call Working Draft 10 November 2003
 * http://www.w3.org/TR/2003/WD-xinclude-20031110
 */

#ifndef __XML_XINCLUDE_H__
#define __XML_XINCLUDE_H__

#include <libxml/xmlversion.h>
#include <libxml/tree.h>

#ifdef LIBXML_XINCLUDE_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * XINCLUDE_NS:
 *
 * Macro defining the Xinclude namespace: http://www.w3.org/2003/XInclude
 */
#define XINCLUDE_NS (const xmlChar *) "http://www.w3.org/2003/XInclude"
/**
 * XINCLUDE_OLD_NS:
 *
 * Macro defining the draft Xinclude namespace: http://www.w3.org/2001/XInclude
 */
#define XINCLUDE_OLD_NS (const xmlChar *) "http://www.w3.org/2001/XInclude"
/**
 * XINCLUDE_NODE:
 *
 * Macro defining "include"
 */
#define XINCLUDE_NODE (const xmlChar *) "include"
/**
 * XINCLUDE_FALLBACK:
 *
 * Macro defining "fallback"
 */
#define XINCLUDE_FALLBACK (const xmlChar *) "fallback"
/**
 * XINCLUDE_HREF:
 *
 * Macro defining "href"
 */
#define XINCLUDE_HREF (const xmlChar *) "href"
/**
 * XINCLUDE_PARSE:
 *
 * Macro defining "parse"
 */
#define XINCLUDE_PARSE (const xmlChar *) "parse"
/**
 * XINCLUDE_PARSE_XML:
 *
 * Macro defining "xml"
 */
#define XINCLUDE_PARSE_XML (const xmlChar *) "xml"
/**
 * XINCLUDE_PARSE_TEXT:
 *
 * Macro defining "text"
 */
#define XINCLUDE_PARSE_TEXT (const xmlChar *) "text"
/**
 * XINCLUDE_PARSE_ENCODING:
 *
 * Macro defining "encoding"
 */
#define XINCLUDE_PARSE_ENCODING (const xmlChar *) "encoding"
/**
 * XINCLUDE_PARSE_XPOINTER:
 *
 * Macro defining "xpointer"
 */
#define XINCLUDE_PARSE_XPOINTER (const xmlChar *) "xpointer"

typedef struct _xmlXIncludeCtxt xmlXIncludeCtxt;
typedef xmlXIncludeCtxt *xmlXIncludeCtxtPtr;

/*
 * standalone processing
 */
XMLPUBFUN int XMLCALL
		xmlXIncludeProcess	(xmlDocPtr doc);
XMLPUBFUN int XMLCALL
		xmlXIncludeProcessFlags	(xmlDocPtr doc,
					 int flags);
XMLPUBFUN int XMLCALL
		xmlXIncludeProcessFlagsData(xmlDocPtr doc,
					 int flags,
					 void *data);
XMLPUBFUN int XMLCALL
                xmlXIncludeProcessTreeFlagsData(xmlNodePtr tree,
                                         int flags,
                                         void *data);
XMLPUBFUN int XMLCALL
		xmlXIncludeProcessTree	(xmlNodePtr tree);
XMLPUBFUN int XMLCALL
		xmlXIncludeProcessTreeFlags(xmlNodePtr tree,
					 int flags);
/*
 * contextual processing
 */
XMLPUBFUN xmlXIncludeCtxtPtr XMLCALL
		xmlXIncludeNewContext	(xmlDocPtr doc);
XMLPUBFUN int XMLCALL
		xmlXIncludeSetFlags	(xmlXIncludeCtxtPtr ctxt,
					 int flags);
XMLPUBFUN void XMLCALL
		xmlXIncludeFreeContext	(xmlXIncludeCtxtPtr ctxt);
XMLPUBFUN int XMLCALL
		xmlXIncludeProcessNode	(xmlXIncludeCtxtPtr ctxt,
					 xmlNodePtr tree);
#ifdef __cplusplus
}
#endif

#endif /* LIBXML_XINCLUDE_ENABLED */

#endif /* __XML_XINCLUDE_H__ */
