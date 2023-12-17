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
 * Summary: regular expressions handling
 * Description: basic API for libxml regular expressions handling used
 *              for XML Schemas and validation.
 */

#ifndef __XML_REGEXP_H__
#define __XML_REGEXP_H__

#include <libxml/xmlversion.h>

#ifdef LIBXML_REGEXP_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * xmlRegexpPtr:
 *
 * A libxml regular expression, they can actually be far more complex
 * thank the POSIX regex expressions.
 */
typedef struct _xmlRegexp xmlRegexp;
typedef xmlRegexp *xmlRegexpPtr;

/**
 * xmlRegExecCtxtPtr:
 *
 * A libxml progressive regular expression evaluation context
 */
typedef struct _xmlRegExecCtxt xmlRegExecCtxt;
typedef xmlRegExecCtxt *xmlRegExecCtxtPtr;

#ifdef __cplusplus
}
#endif
#include <libxml/tree.h>
#include <libxml/dict.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * The POSIX like API
 */
XMLPUBFUN xmlRegexpPtr XMLCALL
		    xmlRegexpCompile	(const xmlChar *regexp);
XMLPUBFUN void XMLCALL			 xmlRegFreeRegexp(xmlRegexpPtr regexp);
XMLPUBFUN int XMLCALL
		    xmlRegexpExec	(xmlRegexpPtr comp,
					 const xmlChar *value);
XMLPUBFUN void XMLCALL
		    xmlRegexpPrint	(FILE *output,
					 xmlRegexpPtr regexp);
XMLPUBFUN int XMLCALL
		    xmlRegexpIsDeterminist(xmlRegexpPtr comp);

/**
 * xmlRegExecCallbacks:
 * @exec: the regular expression context
 * @token: the current token string
 * @transdata: transition data
 * @inputdata: input data
 *
 * Callback function when doing a transition in the automata
 */
typedef void (*xmlRegExecCallbacks) (xmlRegExecCtxtPtr exec,
	                             const xmlChar *token,
				     void *transdata,
				     void *inputdata);

/*
 * The progressive API
 */
XMLPUBFUN xmlRegExecCtxtPtr XMLCALL
		    xmlRegNewExecCtxt	(xmlRegexpPtr comp,
					 xmlRegExecCallbacks callback,
					 void *data);
XMLPUBFUN void XMLCALL
		    xmlRegFreeExecCtxt	(xmlRegExecCtxtPtr exec);
XMLPUBFUN int XMLCALL
		    xmlRegExecPushString(xmlRegExecCtxtPtr exec,
					 const xmlChar *value,
					 void *data);
XMLPUBFUN int XMLCALL
		    xmlRegExecPushString2(xmlRegExecCtxtPtr exec,
					 const xmlChar *value,
					 const xmlChar *value2,
					 void *data);

XMLPUBFUN int XMLCALL
		    xmlRegExecNextValues(xmlRegExecCtxtPtr exec,
					 int *nbval,
					 int *nbneg,
					 xmlChar **values,
					 int *terminal);
XMLPUBFUN int XMLCALL
		    xmlRegExecErrInfo	(xmlRegExecCtxtPtr exec,
					 const xmlChar **string,
					 int *nbval,
					 int *nbneg,
					 xmlChar **values,
					 int *terminal);
#ifdef LIBXML_EXPR_ENABLED
/*
 * Formal regular expression handling
 * Its goal is to do some formal work on content models
 */

/* expressions are used within a context */
typedef struct _xmlExpCtxt xmlExpCtxt;
typedef xmlExpCtxt *xmlExpCtxtPtr;

XMLPUBFUN void XMLCALL
			xmlExpFreeCtxt	(xmlExpCtxtPtr ctxt);
XMLPUBFUN xmlExpCtxtPtr XMLCALL
			xmlExpNewCtxt	(int maxNodes,
					 xmlDictPtr dict);

XMLPUBFUN int XMLCALL
			xmlExpCtxtNbNodes(xmlExpCtxtPtr ctxt);
XMLPUBFUN int XMLCALL
			xmlExpCtxtNbCons(xmlExpCtxtPtr ctxt);

/* Expressions are trees but the tree is opaque */
typedef struct _xmlExpNode xmlExpNode;
typedef xmlExpNode *xmlExpNodePtr;

typedef enum {
    XML_EXP_EMPTY = 0,
    XML_EXP_FORBID = 1,
    XML_EXP_ATOM = 2,
    XML_EXP_SEQ = 3,
    XML_EXP_OR = 4,
    XML_EXP_COUNT = 5
} xmlExpNodeType;

/*
 * 2 core expressions shared by all for the empty language set
 * and for the set with just the empty token
 */
XMLPUBVAR xmlExpNodePtr forbiddenExp;
XMLPUBVAR xmlExpNodePtr emptyExp;

/*
 * Expressions are reference counted internally
 */
XMLPUBFUN void XMLCALL
			xmlExpFree	(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr expr);
XMLPUBFUN void XMLCALL
			xmlExpRef	(xmlExpNodePtr expr);

/*
 * constructors can be either manual or from a string
 */
XMLPUBFUN xmlExpNodePtr XMLCALL
			xmlExpParse	(xmlExpCtxtPtr ctxt,
					 const char *expr);
XMLPUBFUN xmlExpNodePtr XMLCALL
			xmlExpNewAtom	(xmlExpCtxtPtr ctxt,
					 const xmlChar *name,
					 int len);
XMLPUBFUN xmlExpNodePtr XMLCALL
			xmlExpNewOr	(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr left,
					 xmlExpNodePtr right);
XMLPUBFUN xmlExpNodePtr XMLCALL
			xmlExpNewSeq	(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr left,
					 xmlExpNodePtr right);
XMLPUBFUN xmlExpNodePtr XMLCALL
			xmlExpNewRange	(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr subset,
					 int min,
					 int max);
/*
 * The really interesting APIs
 */
XMLPUBFUN int XMLCALL
			xmlExpIsNillable(xmlExpNodePtr expr);
XMLPUBFUN int XMLCALL
			xmlExpMaxToken	(xmlExpNodePtr expr);
XMLPUBFUN int XMLCALL
			xmlExpGetLanguage(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr expr,
					 const xmlChar**langList,
					 int len);
XMLPUBFUN int XMLCALL
			xmlExpGetStart	(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr expr,
					 const xmlChar**tokList,
					 int len);
XMLPUBFUN xmlExpNodePtr XMLCALL
			xmlExpStringDerive(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr expr,
					 const xmlChar *str,
					 int len);
XMLPUBFUN xmlExpNodePtr XMLCALL
			xmlExpExpDerive	(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr expr,
					 xmlExpNodePtr sub);
XMLPUBFUN int XMLCALL
			xmlExpSubsume	(xmlExpCtxtPtr ctxt,
					 xmlExpNodePtr expr,
					 xmlExpNodePtr sub);
XMLPUBFUN void XMLCALL
			xmlExpDump	(xmlBufferPtr buf,
					 xmlExpNodePtr expr);
#endif /* LIBXML_EXPR_ENABLED */
#ifdef __cplusplus
}
#endif

#endif /* LIBXML_REGEXP_ENABLED */

#endif /*__XML_REGEXP_H__ */
