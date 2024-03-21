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
 * Summary: Old SAX version 1 handler, deprecated
 * Description: DEPRECATED set of SAX version 1 interfaces used to
 *              build the DOM tree.
 */

#ifndef __XML_SAX_H__
#define __XML_SAX_H__

#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xlink.h>

#ifdef LIBXML_LEGACY_ENABLED

#ifdef __cplusplus
extern "C" {
#endif
XMLPUBFUN const xmlChar * XMLCALL
		getPublicId			(void *ctx);
XMLPUBFUN const xmlChar * XMLCALL
		getSystemId			(void *ctx);
XMLPUBFUN void XMLCALL
		setDocumentLocator		(void *ctx,
						 xmlSAXLocatorPtr loc);

XMLPUBFUN int XMLCALL
		getLineNumber			(void *ctx);
XMLPUBFUN int XMLCALL
		getColumnNumber			(void *ctx);

XMLPUBFUN int XMLCALL
		isStandalone			(void *ctx);
XMLPUBFUN int XMLCALL
		hasInternalSubset		(void *ctx);
XMLPUBFUN int XMLCALL
		hasExternalSubset		(void *ctx);

XMLPUBFUN void XMLCALL
		internalSubset			(void *ctx,
						 const xmlChar *name,
						 const xmlChar *ExternalID,
						 const xmlChar *SystemID);
XMLPUBFUN void XMLCALL
		externalSubset			(void *ctx,
						 const xmlChar *name,
						 const xmlChar *ExternalID,
						 const xmlChar *SystemID);
XMLPUBFUN xmlEntityPtr XMLCALL
		getEntity			(void *ctx,
						 const xmlChar *name);
XMLPUBFUN xmlEntityPtr XMLCALL
		getParameterEntity		(void *ctx,
						 const xmlChar *name);
XMLPUBFUN xmlParserInputPtr XMLCALL
		resolveEntity			(void *ctx,
						 const xmlChar *publicId,
						 const xmlChar *systemId);

XMLPUBFUN void XMLCALL
		entityDecl			(void *ctx,
						 const xmlChar *name,
						 int type,
						 const xmlChar *publicId,
						 const xmlChar *systemId,
						 xmlChar *content);
XMLPUBFUN void XMLCALL
		attributeDecl			(void *ctx,
						 const xmlChar *elem,
						 const xmlChar *fullname,
						 int type,
						 int def,
						 const xmlChar *defaultValue,
						 xmlEnumerationPtr tree);
XMLPUBFUN void XMLCALL
		elementDecl			(void *ctx,
						 const xmlChar *name,
						 int type,
						 xmlElementContentPtr content);
XMLPUBFUN void XMLCALL
		notationDecl			(void *ctx,
						 const xmlChar *name,
						 const xmlChar *publicId,
						 const xmlChar *systemId);
XMLPUBFUN void XMLCALL
		unparsedEntityDecl		(void *ctx,
						 const xmlChar *name,
						 const xmlChar *publicId,
						 const xmlChar *systemId,
						 const xmlChar *notationName);

XMLPUBFUN void XMLCALL
		startDocument			(void *ctx);
XMLPUBFUN void XMLCALL
		endDocument			(void *ctx);
XMLPUBFUN void XMLCALL
		attribute			(void *ctx,
						 const xmlChar *fullname,
						 const xmlChar *value);
XMLPUBFUN void XMLCALL
		startElement			(void *ctx,
						 const xmlChar *fullname,
						 const xmlChar **atts);
XMLPUBFUN void XMLCALL
		endElement			(void *ctx,
						 const xmlChar *name);
XMLPUBFUN void XMLCALL
		reference			(void *ctx,
						 const xmlChar *name);
XMLPUBFUN void XMLCALL
		characters			(void *ctx,
						 const xmlChar *ch,
						 int len);
XMLPUBFUN void XMLCALL
		ignorableWhitespace		(void *ctx,
						 const xmlChar *ch,
						 int len);
XMLPUBFUN void XMLCALL
		processingInstruction		(void *ctx,
						 const xmlChar *target,
						 const xmlChar *data);
XMLPUBFUN void XMLCALL
		globalNamespace			(void *ctx,
						 const xmlChar *href,
						 const xmlChar *prefix);
XMLPUBFUN void XMLCALL
		setNamespace			(void *ctx,
						 const xmlChar *name);
XMLPUBFUN xmlNsPtr XMLCALL
		getNamespace			(void *ctx);
XMLPUBFUN int XMLCALL
		checkNamespace			(void *ctx,
						 xmlChar *nameSpace);
XMLPUBFUN void XMLCALL
		namespaceDecl			(void *ctx,
						 const xmlChar *href,
						 const xmlChar *prefix);
XMLPUBFUN void XMLCALL
		comment				(void *ctx,
						 const xmlChar *value);
XMLPUBFUN void XMLCALL
		cdataBlock			(void *ctx,
						 const xmlChar *value,
						 int len);

#ifdef LIBXML_SAX1_ENABLED
XMLPUBFUN void XMLCALL
		initxmlDefaultSAXHandler	(xmlSAXHandlerV1 *hdlr,
						 int warning);
#ifdef LIBXML_HTML_ENABLED
XMLPUBFUN void XMLCALL
		inithtmlDefaultSAXHandler	(xmlSAXHandlerV1 *hdlr);
#endif
#ifdef LIBXML_DOCB_ENABLED
XMLPUBFUN void XMLCALL
		initdocbDefaultSAXHandler	(xmlSAXHandlerV1 *hdlr);
#endif
#endif /* LIBXML_SAX1_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* LIBXML_LEGACY_ENABLED */

#endif /* __XML_SAX_H__ */
