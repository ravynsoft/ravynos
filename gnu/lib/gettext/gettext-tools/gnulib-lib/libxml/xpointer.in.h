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
 * Summary: API to handle XML Pointers
 * Description: API to handle XML Pointers
 * Base implementation was made accordingly to
 * W3C Candidate Recommendation 7 June 2000
 * http://www.w3.org/TR/2000/CR-xptr-20000607
 *
 * Added support for the element() scheme described in:
 * W3C Proposed Recommendation 13 November 2002
 * http://www.w3.org/TR/2002/PR-xptr-element-20021113/
 */

#ifndef __XML_XPTR_H__
#define __XML_XPTR_H__

#include <libxml/xmlversion.h>

#ifdef LIBXML_XPTR_ENABLED

#include <libxml/tree.h>
#include <libxml/xpath.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A Location Set
 */
typedef struct _xmlLocationSet xmlLocationSet;
typedef xmlLocationSet *xmlLocationSetPtr;
struct _xmlLocationSet {
    int locNr;		      /* number of locations in the set */
    int locMax;		      /* size of the array as allocated */
    xmlXPathObjectPtr *locTab;/* array of locations */
};

/*
 * Handling of location sets.
 */

XMLPUBFUN xmlLocationSetPtr XMLCALL
		    xmlXPtrLocationSetCreate	(xmlXPathObjectPtr val);
XMLPUBFUN void XMLCALL
		    xmlXPtrFreeLocationSet	(xmlLocationSetPtr obj);
XMLPUBFUN xmlLocationSetPtr XMLCALL
		    xmlXPtrLocationSetMerge	(xmlLocationSetPtr val1,
						 xmlLocationSetPtr val2);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewRange		(xmlNodePtr start,
						 int startindex,
						 xmlNodePtr end,
						 int endindex);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewRangePoints	(xmlXPathObjectPtr start,
						 xmlXPathObjectPtr end);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewRangeNodePoint	(xmlNodePtr start,
						 xmlXPathObjectPtr end);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewRangePointNode	(xmlXPathObjectPtr start,
						 xmlNodePtr end);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewRangeNodes	(xmlNodePtr start,
						 xmlNodePtr end);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewLocationSetNodes	(xmlNodePtr start,
						 xmlNodePtr end);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewLocationSetNodeSet(xmlNodeSetPtr set);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewRangeNodeObject	(xmlNodePtr start,
						 xmlXPathObjectPtr end);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrNewCollapsedRange	(xmlNodePtr start);
XMLPUBFUN void XMLCALL
		    xmlXPtrLocationSetAdd	(xmlLocationSetPtr cur,
						 xmlXPathObjectPtr val);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrWrapLocationSet	(xmlLocationSetPtr val);
XMLPUBFUN void XMLCALL
		    xmlXPtrLocationSetDel	(xmlLocationSetPtr cur,
						 xmlXPathObjectPtr val);
XMLPUBFUN void XMLCALL
		    xmlXPtrLocationSetRemove	(xmlLocationSetPtr cur,
						 int val);

/*
 * Functions.
 */
XMLPUBFUN xmlXPathContextPtr XMLCALL
		    xmlXPtrNewContext		(xmlDocPtr doc,
						 xmlNodePtr here,
						 xmlNodePtr origin);
XMLPUBFUN xmlXPathObjectPtr XMLCALL
		    xmlXPtrEval			(const xmlChar *str,
						 xmlXPathContextPtr ctx);
XMLPUBFUN void XMLCALL
		    xmlXPtrRangeToFunction	(xmlXPathParserContextPtr ctxt,
						 int nargs);
XMLPUBFUN xmlNodePtr XMLCALL
		    xmlXPtrBuildNodeList	(xmlXPathObjectPtr obj);
XMLPUBFUN void XMLCALL
		    xmlXPtrEvalRangePredicate	(xmlXPathParserContextPtr ctxt);
#ifdef __cplusplus
}
#endif

#endif /* LIBXML_XPTR_ENABLED */
#endif /* __XML_XPTR_H__ */
