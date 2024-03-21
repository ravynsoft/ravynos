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
 * Summary: unfinished XLink detection module
 * Description: unfinished XLink detection module
 */

#ifndef __XML_XLINK_H__
#define __XML_XLINK_H__

#include <libxml/xmlversion.h>
#include <libxml/tree.h>

#ifdef LIBXML_XPTR_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Various defines for the various Link properties.
 *
 * NOTE: the link detection layer will try to resolve QName expansion
 *       of namespaces. If "foo" is the prefix for "http://foo.com/"
 *       then the link detection layer will expand role="foo:myrole"
 *       to "http://foo.com/:myrole".
 * NOTE: the link detection layer will expand URI-Refences found on
 *       href attributes by using the base mechanism if found.
 */
typedef xmlChar *xlinkHRef;
typedef xmlChar *xlinkRole;
typedef xmlChar *xlinkTitle;

typedef enum {
    XLINK_TYPE_NONE = 0,
    XLINK_TYPE_SIMPLE,
    XLINK_TYPE_EXTENDED,
    XLINK_TYPE_EXTENDED_SET
} xlinkType;

typedef enum {
    XLINK_SHOW_NONE = 0,
    XLINK_SHOW_NEW,
    XLINK_SHOW_EMBED,
    XLINK_SHOW_REPLACE
} xlinkShow;

typedef enum {
    XLINK_ACTUATE_NONE = 0,
    XLINK_ACTUATE_AUTO,
    XLINK_ACTUATE_ONREQUEST
} xlinkActuate;

/**
 * xlinkNodeDetectFunc:
 * @ctx:  user data pointer
 * @node:  the node to check
 *
 * This is the prototype for the link detection routine.
 * It calls the default link detection callbacks upon link detection.
 */
typedef void (*xlinkNodeDetectFunc) (void *ctx, xmlNodePtr node);

/*
 * The link detection module interact with the upper layers using
 * a set of callback registered at parsing time.
 */

/**
 * xlinkSimpleLinkFunk:
 * @ctx:  user data pointer
 * @node:  the node carrying the link
 * @href:  the target of the link
 * @role:  the role string
 * @title:  the link title
 *
 * This is the prototype for a simple link detection callback.
 */
typedef void
(*xlinkSimpleLinkFunk)	(void *ctx,
			 xmlNodePtr node,
			 const xlinkHRef href,
			 const xlinkRole role,
			 const xlinkTitle title);

/**
 * xlinkExtendedLinkFunk:
 * @ctx:  user data pointer
 * @node:  the node carrying the link
 * @nbLocators: the number of locators detected on the link
 * @hrefs:  pointer to the array of locator hrefs
 * @roles:  pointer to the array of locator roles
 * @nbArcs: the number of arcs detected on the link
 * @from:  pointer to the array of source roles found on the arcs
 * @to:  pointer to the array of target roles found on the arcs
 * @show:  array of values for the show attributes found on the arcs
 * @actuate:  array of values for the actuate attributes found on the arcs
 * @nbTitles: the number of titles detected on the link
 * @title:  array of titles detected on the link
 * @langs:  array of xml:lang values for the titles
 *
 * This is the prototype for a extended link detection callback.
 */
typedef void
(*xlinkExtendedLinkFunk)(void *ctx,
			 xmlNodePtr node,
			 int nbLocators,
			 const xlinkHRef *hrefs,
			 const xlinkRole *roles,
			 int nbArcs,
			 const xlinkRole *from,
			 const xlinkRole *to,
			 xlinkShow *show,
			 xlinkActuate *actuate,
			 int nbTitles,
			 const xlinkTitle *titles,
			 const xmlChar **langs);

/**
 * xlinkExtendedLinkSetFunk:
 * @ctx:  user data pointer
 * @node:  the node carrying the link
 * @nbLocators: the number of locators detected on the link
 * @hrefs:  pointer to the array of locator hrefs
 * @roles:  pointer to the array of locator roles
 * @nbTitles: the number of titles detected on the link
 * @title:  array of titles detected on the link
 * @langs:  array of xml:lang values for the titles
 *
 * This is the prototype for a extended link set detection callback.
 */
typedef void
(*xlinkExtendedLinkSetFunk)	(void *ctx,
				 xmlNodePtr node,
				 int nbLocators,
				 const xlinkHRef *hrefs,
				 const xlinkRole *roles,
				 int nbTitles,
				 const xlinkTitle *titles,
				 const xmlChar **langs);

/**
 * This is the structure containing a set of Links detection callbacks.
 *
 * There is no default xlink callbacks, if one want to get link
 * recognition activated, those call backs must be provided before parsing.
 */
typedef struct _xlinkHandler xlinkHandler;
typedef xlinkHandler *xlinkHandlerPtr;
struct _xlinkHandler {
    xlinkSimpleLinkFunk simple;
    xlinkExtendedLinkFunk extended;
    xlinkExtendedLinkSetFunk set;
};

/*
 * The default detection routine, can be overridden, they call the default
 * detection callbacks.
 */

XMLPUBFUN xlinkNodeDetectFunc XMLCALL
		xlinkGetDefaultDetect	(void);
XMLPUBFUN void XMLCALL
		xlinkSetDefaultDetect	(xlinkNodeDetectFunc func);

/*
 * Routines to set/get the default handlers.
 */
XMLPUBFUN xlinkHandlerPtr XMLCALL
		xlinkGetDefaultHandler	(void);
XMLPUBFUN void XMLCALL
		xlinkSetDefaultHandler	(xlinkHandlerPtr handler);

/*
 * Link detection module itself.
 */
XMLPUBFUN xlinkType XMLCALL
		xlinkIsLink		(xmlDocPtr doc,
					 xmlNodePtr node);

#ifdef __cplusplus
}
#endif

#endif /* LIBXML_XPTR_ENABLED */

#endif /* __XML_XLINK_H__ */
