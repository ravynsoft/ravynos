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
 * Summary: XML Schemastron implementation
 * Description: interface to the XML Schematron validity checking.
 */

#ifndef __XML_SCHEMATRON_H__
#define __XML_SCHEMATRON_H__

#include <libxml/xmlversion.h>

#ifdef LIBXML_SCHEMATRON_ENABLED

#include <libxml/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    XML_SCHEMATRON_OUT_QUIET = 1 << 0,	/* quiet no report */
    XML_SCHEMATRON_OUT_TEXT = 1 << 1,	/* build a textual report */
    XML_SCHEMATRON_OUT_XML = 1 << 2,	/* output SVRL */
    XML_SCHEMATRON_OUT_ERROR = 1 << 3,  /* output via xmlStructuredErrorFunc */
    XML_SCHEMATRON_OUT_FILE = 1 << 8,	/* output to a file descriptor */
    XML_SCHEMATRON_OUT_BUFFER = 1 << 9,	/* output to a buffer */
    XML_SCHEMATRON_OUT_IO = 1 << 10	/* output to I/O mechanism */
} xmlSchematronValidOptions;

/**
 * The schemas related types are kept internal
 */
typedef struct _xmlSchematron xmlSchematron;
typedef xmlSchematron *xmlSchematronPtr;

/**
 * xmlSchematronValidityErrorFunc:
 * @ctx: the validation context
 * @msg: the message
 * @...: extra arguments
 *
 * Signature of an error callback from a Schematron validation
 */
typedef void (*xmlSchematronValidityErrorFunc) (void *ctx, const char *msg, ...);

/**
 * xmlSchematronValidityWarningFunc:
 * @ctx: the validation context
 * @msg: the message
 * @...: extra arguments
 *
 * Signature of a warning callback from a Schematron validation
 */
typedef void (*xmlSchematronValidityWarningFunc) (void *ctx, const char *msg, ...);

/**
 * A schemas validation context
 */
typedef struct _xmlSchematronParserCtxt xmlSchematronParserCtxt;
typedef xmlSchematronParserCtxt *xmlSchematronParserCtxtPtr;

typedef struct _xmlSchematronValidCtxt xmlSchematronValidCtxt;
typedef xmlSchematronValidCtxt *xmlSchematronValidCtxtPtr;

/*
 * Interfaces for parsing.
 */
XMLPUBFUN xmlSchematronParserCtxtPtr XMLCALL
	    xmlSchematronNewParserCtxt	(const char *URL);
XMLPUBFUN xmlSchematronParserCtxtPtr XMLCALL
	    xmlSchematronNewMemParserCtxt(const char *buffer,
					 int size);
XMLPUBFUN xmlSchematronParserCtxtPtr XMLCALL
	    xmlSchematronNewDocParserCtxt(xmlDocPtr doc);
XMLPUBFUN void XMLCALL
	    xmlSchematronFreeParserCtxt	(xmlSchematronParserCtxtPtr ctxt);
/*****
XMLPUBFUN void XMLCALL
	    xmlSchematronSetParserErrors(xmlSchematronParserCtxtPtr ctxt,
					 xmlSchematronValidityErrorFunc err,
					 xmlSchematronValidityWarningFunc warn,
					 void *ctx);
XMLPUBFUN int XMLCALL
		xmlSchematronGetParserErrors(xmlSchematronParserCtxtPtr ctxt,
					xmlSchematronValidityErrorFunc * err,
					xmlSchematronValidityWarningFunc * warn,
					void **ctx);
XMLPUBFUN int XMLCALL
		xmlSchematronIsValid	(xmlSchematronValidCtxtPtr ctxt);
 *****/
XMLPUBFUN xmlSchematronPtr XMLCALL
	    xmlSchematronParse		(xmlSchematronParserCtxtPtr ctxt);
XMLPUBFUN void XMLCALL
	    xmlSchematronFree		(xmlSchematronPtr schema);
/*
 * Interfaces for validating
 */
XMLPUBFUN void XMLCALL
	    xmlSchematronSetValidStructuredErrors(
	                                  xmlSchematronValidCtxtPtr ctxt,
					  xmlStructuredErrorFunc serror,
					  void *ctx);
/******
XMLPUBFUN void XMLCALL
	    xmlSchematronSetValidErrors	(xmlSchematronValidCtxtPtr ctxt,
					 xmlSchematronValidityErrorFunc err,
					 xmlSchematronValidityWarningFunc warn,
					 void *ctx);
XMLPUBFUN int XMLCALL
	    xmlSchematronGetValidErrors	(xmlSchematronValidCtxtPtr ctxt,
					 xmlSchematronValidityErrorFunc *err,
					 xmlSchematronValidityWarningFunc *warn,
					 void **ctx);
XMLPUBFUN int XMLCALL
	    xmlSchematronSetValidOptions(xmlSchematronValidCtxtPtr ctxt,
					 int options);
XMLPUBFUN int XMLCALL
	    xmlSchematronValidCtxtGetOptions(xmlSchematronValidCtxtPtr ctxt);
XMLPUBFUN int XMLCALL
            xmlSchematronValidateOneElement (xmlSchematronValidCtxtPtr ctxt,
			                 xmlNodePtr elem);
 *******/

XMLPUBFUN xmlSchematronValidCtxtPtr XMLCALL
	    xmlSchematronNewValidCtxt	(xmlSchematronPtr schema,
					 int options);
XMLPUBFUN void XMLCALL
	    xmlSchematronFreeValidCtxt	(xmlSchematronValidCtxtPtr ctxt);
XMLPUBFUN int XMLCALL
	    xmlSchematronValidateDoc	(xmlSchematronValidCtxtPtr ctxt,
					 xmlDocPtr instance);

#ifdef __cplusplus
}
#endif

#endif /* LIBXML_SCHEMATRON_ENABLED */
#endif /* __XML_SCHEMATRON_H__ */
