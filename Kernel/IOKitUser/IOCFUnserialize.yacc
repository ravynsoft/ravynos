/*
 * Copyright (c) 1999-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * HISTORY
 *
 * 09 Dec 99 rsulack created by rsulack
 */

// parser for unserializing OSContainer objects serialized to XML
//
// to build :
//	bison -p IOCFUnserialize -o IOCFUnserialize.temp IOCFUnserialize.yacc
//	head -50 IOCFUnserialize.yacc > IOCFUnserialize.tab.c
//	cat IOCFUnserialize.temp >> IOCFUnserialize.tab.c
//
//	when changing code check in both IOCFUnserialize.yacc and IOCFUnserialize.tab.c
//
//
//
//
//
//		 DO NOT EDIT IOCFUnserialize.tab.c
//
//			this means you!
//
//
//
//
//
//

     
%pure_parser

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <syslog.h>
    
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFDictionary.h>

#define YYSTYPE object_t *
#define YYPARSE_PARAM	state
#define YYLEX_PARAM	(parser_state_t *)state

// this is the internal struct used to hold objects on parser stack
// it represents objects both before and after they have been created
typedef	struct object {
	struct object	*next;
	struct object	*free;
	struct object	*elements;
	CFTypeRef	object;
	CFStringRef	key;			// for dictionary
	int		size;
	void		*data;			// for data
	char		*string;		// for string & symbol
	long long 	number;			// for number
	int		idref;
} object_t;

// this code is reentrant, this structure contains all
// state information for the parsing of a single buffer
typedef struct parser_state {
	const char	*parseBuffer;		// start of text to be parsed
	int		parseBufferIndex;	// current index into text
	int		lineNumber;		// current line number
	CFAllocatorRef 	allocator;		// which allocator to use
	object_t	*objects;		// internal objects in use
	object_t	*freeObjects;		// internal objects that are free
	CFMutableDictionaryRef tags;		// used to remember "ID" tags
	CFStringRef 	*errorString;		// parse error with line
	CFTypeRef	parsedObject;		// resultant object of parsed text
} parser_state_t;

#define STATE		((parser_state_t *)state)

#undef yyerror 	
#define yyerror(s)	IOCFUnserializeerror(STATE, (s))
static int		IOCFUnserializeerror(parser_state_t *state, const char *s);

static int		yylex(YYSTYPE *lvalp, parser_state_t *state);

static object_t 	*newObject(parser_state_t *state);
static void 		freeObject(parser_state_t *state, object_t *o);
static void		rememberObject(parser_state_t *state, intptr_t tag, CFTypeRef o);
static object_t		*retrieveObject(parser_state_t *state, intptr_t tag);
static void		cleanupObjects(parser_state_t *state);

static object_t		*buildDictionary(parser_state_t *state, object_t *o);
static object_t		*buildArray(parser_state_t *state, object_t *o);
static object_t		*buildSet(parser_state_t *state, object_t *o);
static object_t		*buildString(parser_state_t *state, object_t *o);
static object_t		*buildData(parser_state_t *state, object_t *o);
static object_t		*buildNumber(parser_state_t *state, object_t *o);
static object_t		*buildBoolean(parser_state_t *state, object_t *o);

%}
%token ARRAY
%token BOOLEAN
%token DATA
%token DICTIONARY
%token IDREF
%token KEY
%token NUMBER
%token SET
%token STRING
%token SYNTAX_ERROR     
%% /* Grammar rules and actions follow */

input:	  /* empty */		{ yyerror("unexpected end of buffer");
				  YYERROR;
				}
	| object		{ STATE->parsedObject = $1->object;
				  $1->object = 0;
				  freeObject(STATE, $1);
				  YYACCEPT;
				}
	| SYNTAX_ERROR		{ yyerror("syntax error");
				  YYERROR;
				}
	;

object:	  dict			{ $$ = buildDictionary(STATE, $1); }
	| array			{ $$ = buildArray(STATE, $1); }
	| set			{ $$ = buildSet(STATE, $1); }
	| string		{ $$ = buildString(STATE, $1); }
	| data			{ $$ = buildData(STATE, $1); }
	| number		{ $$ = buildNumber(STATE, $1); }
	| boolean		{ $$ = buildBoolean(STATE, $1); }
	| idref			{ $$ = retrieveObject(STATE, $1->idref);
				  if ($$) {
				    CFRetain($$->object);
				  } else { 
				    yyerror("forward reference detected");
				    YYERROR;
				  }
				  freeObject(STATE, $1);
				}
	;

//------------------------------------------------------------------------------

dict:	  '{' '}'		{ $$ = $1;
				  $$->elements = NULL;
				}
	| '{' pairs '}'		{ $$ = $1;
				  $$->elements = $2;
				}
	| DICTIONARY
	;

pairs:	  pair
	| pairs pair		{ $$ = $2;
				  $$->next = $1;
				}
	;

pair:	  key object		{ $$ = $1;
				  $$->key = (CFStringRef)$$->object;
				  $$->object = $2->object;
				  $$->next = NULL; 
				  $2->object = 0;
				  freeObject(STATE, $2);
				}
	;

key:	  KEY			{ $$ = buildString(STATE, $1); }
	;

//------------------------------------------------------------------------------

array:	  '(' ')'		{ $$ = $1;
				  $$->elements = NULL;
				}
	| '(' elements ')'	{ $$ = $1;
				  $$->elements = $2;
				}
	| ARRAY
	;

set:	  '[' ']'		{ $$ = $1;
				  $$->elements = NULL;
				}
	| '[' elements ']'	{ $$ = $1;
				  $$->elements = $2;
				}
	| SET
	;

elements: object		{ $$ = $1; 
				  $$->next = NULL; 
				}
	| elements object	{ $$ = $2;
				  $$->next = $1;
				}
	;

//------------------------------------------------------------------------------

boolean:  BOOLEAN
	;

data:	  DATA
	;

idref:	  IDREF
	;

number:	  NUMBER
	;

string:	  STRING
	;

%%

int
IOCFUnserializeerror(parser_state_t * state, const char *s)  /* Called by yyparse on errors */
{
    if (state->errorString) {
	*(state->errorString) = CFStringCreateWithFormat(state->allocator, NULL, 
							 CFSTR("IOCFUnserialize: %s near line %d"), 
							 s, state->lineNumber);
    }

    return 0;
}

#define TAG_MAX_LENGTH		32
#define TAG_MAX_ATTRIBUTES	32
#define TAG_BAD			0
#define TAG_START		1
#define TAG_END			2
#define TAG_EMPTY		3
#define TAG_IGNORE		4

#define currentChar()	(state->parseBuffer[state->parseBufferIndex])
#define nextChar()	(state->parseBuffer[++state->parseBufferIndex])
#define prevChar()	(state->parseBuffer[state->parseBufferIndex - 1])

#define isSpace(c)	((c) == ' ' || (c) == '\t')
#define isAlpha(c)	(((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define isDigit(c)	((c) >= '0' && (c) <= '9')
#define isAlphaDigit(c)	((c) >= 'a' && (c) <= 'f')
#define isHexDigit(c)	(isDigit(c) || isAlphaDigit(c))
#define isAlphaNumeric(c) (isAlpha(c) || isDigit(c) || ((c) == '-')) 

static int
getTag(parser_state_t *state,
       char tag[TAG_MAX_LENGTH],
       int *attributeCount, 
       char attributes[TAG_MAX_ATTRIBUTES][TAG_MAX_LENGTH],
       char values[TAG_MAX_ATTRIBUTES][TAG_MAX_LENGTH] )
{
	int length = 0;
	int c = currentChar();
	int tagType = TAG_START;

	*attributeCount = 0;

	if (c != '<') return TAG_BAD;
        c = nextChar();		// skip '<'


	// <!TAG   declarations     >
	// <!--     comments      -->
        if (c == '!') {
	    c = nextChar();  
	    bool isComment = (c == '-') && ((c = nextChar()) != 0) && (c == '-');
	    if (!isComment && !isAlpha(c)) return TAG_BAD;   // <!1, <!-A, <!eos

	    while (c && (c = nextChar()) != 0) {
		if (c == '\n') state->lineNumber++;
		if (isComment) {
		    if (c != '-') continue;
		    c = nextChar();
		    if (c != '-') continue;
		    c = nextChar();
		}
		if (c == '>') {
		    (void)nextChar();
		    return TAG_IGNORE;
		}
		if (isComment) break;
	    }
	    return TAG_BAD;
	}

	else

	// <? Processing Instructions  ?>
        if (c == '?') {
                while ((c = nextChar()) != 0) {
                        if (c == '\n') state->lineNumber++;
		if (c != '?') continue;
			c = nextChar();
                        if (c == '>') {
                                (void)nextChar();
		    return TAG_IGNORE;
                        }
                }
	    return TAG_BAD;
        }

	else

	// </ end tag >    
	if (c == '/') {
		c = nextChar();		// skip '/'
		tagType = TAG_END;
	}
        if (!isAlpha(c)) return TAG_BAD;

	/* find end of tag while copying it */
	while (isAlphaNumeric(c)) {
		tag[length++] = c;
		c = nextChar();
		if (length >= (TAG_MAX_LENGTH - 1)) return TAG_BAD;
	}

	tag[length] = 0;

//	printf("tag %s, type %d\n", tag, tagType);
	
	// look for attributes of the form attribute = "value" ...
	while ((c != '>') && (c != '/')) {
		while (isSpace(c)) c = nextChar();

		length = 0;
		while (isAlphaNumeric(c)) {
			attributes[*attributeCount][length++] = c;
			if (length >= (TAG_MAX_LENGTH - 1)) return TAG_BAD;
			c = nextChar();
		}
		attributes[*attributeCount][length] = 0;

		while (isSpace(c)) c = nextChar();
		
		if (c != '=') return TAG_BAD;
		c = nextChar();
		
		while (isSpace(c)) c = nextChar();

		if (c != '"') return TAG_BAD;
		c = nextChar();
		length = 0;
		while (c != '"') {
			values[*attributeCount][length++] = c;
			if (length >= (TAG_MAX_LENGTH - 1)) return TAG_BAD;
			c = nextChar();
		}
		values[*attributeCount][length] = 0;

		c = nextChar(); // skip closing quote

//		printf("	attribute '%s' = '%s', nextchar = '%c'\n", 
//		       attributes[*attributeCount], values[*attributeCount], c);

		(*attributeCount)++;
		if (*attributeCount >= TAG_MAX_ATTRIBUTES) return TAG_BAD;
	}

	if (c == '/') {
		c = nextChar();		// skip '/'
		tagType = TAG_EMPTY;
	}
	if (c != '>') return TAG_BAD;
	c = nextChar();		// skip '>'

	return tagType;
}

static char *
getString(parser_state_t *state)
{
	int c = currentChar();
	int start, length, i, j;
	char * tempString;

	start = state->parseBufferIndex;
	/* find end of string */

	while (c != 0) {
		if (c == '\n') state->lineNumber++;
		if (c == '<') {
			break;
		}
		c = nextChar();
	}

	if (c != '<') return 0;

	length = state->parseBufferIndex - start;

	/* copy to null terminated buffer */
	tempString = (char *)malloc(length + 1);
	if (tempString == 0) {
		printf("IOCFUnserialize: can't alloc temp memory\n");
		goto error;
	}

	// copy out string in tempString
	// "&amp;" -> '&', "&lt;" -> '<', "&gt;" -> '>'

	i = j = 0;
	while (i < length) {
		c = state->parseBuffer[start + i++];
		if (c != '&') {
			tempString[j++] = c;
		} else {
			if ((i+3) > length) goto error;
			c = state->parseBuffer[start + i++];
			if (c == 'l') {
				if (state->parseBuffer[start + i++] != 't') goto error;
				if (state->parseBuffer[start + i++] != ';') goto error;
				tempString[j++] = '<';
				continue;
			}	
			if (c == 'g') {
				if (state->parseBuffer[start + i++] != 't') goto error;
				if (state->parseBuffer[start + i++] != ';') goto error;
				tempString[j++] = '>';
				continue;
			}	
			if ((i+3) > length) goto error;
			if (c == 'a') {
				if (state->parseBuffer[start + i++] != 'm') goto error;
				if (state->parseBuffer[start + i++] != 'p') goto error;
				if (state->parseBuffer[start + i++] != ';') goto error;
				tempString[j++] = '&';
				continue;
			}
			goto error;
		}	
	}
	tempString[j] = 0;

//	printf("string %s\n", tempString);

	return tempString;

error:
	if (tempString) free(tempString);
	return 0;
}

static long long
getNumber(parser_state_t *state)
{
	unsigned long long n = 0;
	int base = 10;
	bool negate = false;
	int c = currentChar();

	if (c == '0') {
		c = nextChar();
		if (c == 'x') {
			base = 16;
			c = nextChar();
		}
	}
	if (base == 10) {
		if (c == '-') {
			negate = true;
			c = nextChar();
		}
		while(isDigit(c)) {
			n = (n * base + c - '0');
			c = nextChar();
		}
		if (negate) {
			n = (unsigned long long)((long long)n * (long long)-1);
		}
	} else {
		while(isHexDigit(c)) {
			if (isDigit(c)) {
				n = (n * base + c - '0');
			} else {
				n = (n * base + 0xa + c - 'a');
			}
			c = nextChar();
		}
	}
//	printf("number 0x%x\n", (unsigned long)n);
	return n;
}

// taken from CFXMLParsing/CFPropertyList.c

static const signed char __CFPLDataDecodeTable[128] = {
    /* 000 */ -1, -1, -1, -1, -1, -1, -1, -1,
    /* 010 */ -1, -1, -1, -1, -1, -1, -1, -1,
    /* 020 */ -1, -1, -1, -1, -1, -1, -1, -1,
    /* 030 */ -1, -1, -1, -1, -1, -1, -1, -1,
    /* ' ' */ -1, -1, -1, -1, -1, -1, -1, -1,
    /* '(' */ -1, -1, -1, 62, -1, -1, -1, 63,
    /* '0' */ 52, 53, 54, 55, 56, 57, 58, 59,
    /* '8' */ 60, 61, -1, -1, -1,  0, -1, -1,
    /* '@' */ -1,  0,  1,  2,  3,  4,  5,  6,
    /* 'H' */  7,  8,  9, 10, 11, 12, 13, 14,
    /* 'P' */ 15, 16, 17, 18, 19, 20, 21, 22,
    /* 'X' */ 23, 24, 25, -1, -1, -1, -1, -1,
    /* '`' */ -1, 26, 27, 28, 29, 30, 31, 32,
    /* 'h' */ 33, 34, 35, 36, 37, 38, 39, 40,
    /* 'p' */ 41, 42, 43, 44, 45, 46, 47, 48,
    /* 'x' */ 49, 50, 51, -1, -1, -1, -1, -1
};

#define DATA_ALLOC_SIZE 4096

static void *
getCFEncodedData(parser_state_t *state, unsigned int *size)
{
    int numeq = 0, acc = 0, cntr = 0;
    int tmpbufpos = 0, tmpbuflen = 0;
    unsigned char *tmpbuf = (unsigned char *)malloc(DATA_ALLOC_SIZE);

    int c = currentChar();
    *size = 0;
	
    while (c != '<') {
        c &= 0x7f;
	if (c == 0) {
		free(tmpbuf);
		return 0;
	}
	if (c == '=') numeq++; else numeq = 0;
	if (c == '\n') state->lineNumber++;
        if (__CFPLDataDecodeTable[c] < 0) {
	    c = nextChar();
            continue;
	}
        cntr++;
        acc <<= 6;
        acc += __CFPLDataDecodeTable[c];
        if (0 == (cntr & 0x3)) {
            if (tmpbuflen <= tmpbufpos + 2) {
                tmpbuflen += DATA_ALLOC_SIZE;
		tmpbuf = (unsigned char *)realloc(tmpbuf, tmpbuflen);
            }
            tmpbuf[tmpbufpos++] = (acc >> 16) & 0xff;
            if (numeq < 2)
                tmpbuf[tmpbufpos++] = (acc >> 8) & 0xff;
            if (numeq < 1)
                tmpbuf[tmpbufpos++] = acc & 0xff;
        }
	c = nextChar();
    }
    *size = tmpbufpos;
    if (*size == 0) {
	free(tmpbuf);
	return 0;
    }
    return tmpbuf;
}

static int
yylex(YYSTYPE *lvalp, parser_state_t *state)
{
	int c, i;
	int tagType;
	char tag[TAG_MAX_LENGTH];
	int attributeCount;
	char attributes[TAG_MAX_ATTRIBUTES][TAG_MAX_LENGTH];
	char values[TAG_MAX_ATTRIBUTES][TAG_MAX_LENGTH];
	object_t *object;

 top:
	c = currentChar();

	/* skip white space  */
	if (isSpace(c)) while ((c = nextChar()) != 0 && isSpace(c)) {};

	/* keep track of line number, don't return \n's */
	if (c == '\n') {
		STATE->lineNumber++;
		(void)nextChar();
		goto top;
	}

	// end of the buffer?
	if (!c)	return 0;

	tagType = getTag(STATE, tag, &attributeCount, attributes, values);
	if (tagType == TAG_BAD) return SYNTAX_ERROR;
	if (tagType == TAG_IGNORE) goto top;

	// handle allocation and check for "ID" and "IDREF" tags up front
	*lvalp = object = newObject(STATE);
	object->idref = -1;
	for (i=0; i < attributeCount; i++) {
	    if (attributes[i][0] == 'I' && attributes[i][1] == 'D') {
		// check for idref's, note: we ignore the tag, for
		// this to work correctly, all idrefs must be unique
		// across the whole serialization
		if (attributes[i][2] == 'R' && attributes[i][3] == 'E' &&
		    attributes[i][4] == 'F' && !attributes[i][5]) {
		    if (tagType != TAG_EMPTY) return SYNTAX_ERROR;
		    object->idref = strtol(values[i], NULL, 0);
		    return IDREF;
		}
		// check for id's
		if (!attributes[i][2]) {
		    object->idref = strtol(values[i], NULL, 0);
		} else {
		    return SYNTAX_ERROR;
		}
	    }
	}

	switch (*tag) {
	case 'a':
		if (!strcmp(tag, "array")) {
			if (tagType == TAG_EMPTY) {
				object->elements = NULL;
				return ARRAY;
			}
			return (tagType == TAG_START) ? '(' : ')';
		}
		break;
	case 'd':
		if (!strcmp(tag, "dict")) {
			if (tagType == TAG_EMPTY) {
				object->elements = NULL;
				return DICTIONARY;
			}
			return (tagType == TAG_START) ? '{' : '}';
		}
		if (!strcmp(tag, "data")) {
			unsigned int size;
			if (tagType == TAG_EMPTY) {
				object->data = NULL;
				object->size = 0;
				return DATA;
			}
			object->data = getCFEncodedData(STATE, &size);
			object->size = size;
			if ((getTag(STATE, tag, &attributeCount, attributes, values) != TAG_END) || strcmp(tag, "data")) {
				return SYNTAX_ERROR;
			}
			return DATA;
		}
		break;
	case 'f':
		if (!strcmp(tag, "false")) {
			if (tagType == TAG_EMPTY) {
				object->number = 0;
				return BOOLEAN;
			}
		}
		break;
	case 'i':
		if (!strcmp(tag, "integer")) {
			object->size = 64;	// default
			for (i=0; i < attributeCount; i++) {
				if (!strcmp(attributes[i], "size")) {
					object->size = strtoul(values[i], NULL, 0);
				}
			}
			if (tagType == TAG_EMPTY) {
				object->number = 0;
				return NUMBER;
			}
			object->number = getNumber(STATE);
			if ((getTag(STATE, tag, &attributeCount, attributes, values) != TAG_END) || strcmp(tag, "integer")) {
				return SYNTAX_ERROR;
			}
			return NUMBER;
		}
		break;
	case 'k':
		if (!strcmp(tag, "key")) {
			if (tagType == TAG_EMPTY) return SYNTAX_ERROR;
			object->string = getString(STATE);
			if (!object->string) {
				return SYNTAX_ERROR;
			}
			if ((getTag(STATE, tag, &attributeCount, attributes, values) != TAG_END)
			   || strcmp(tag, "key")) {
				return SYNTAX_ERROR;
			}
			return KEY;
		}
		break;
	case 'p':
		if (!strcmp(tag, "plist")) {
			freeObject(STATE, object);
			goto top;
		}
		break;
	case 's':
		if (!strcmp(tag, "string")) {
			if (tagType == TAG_EMPTY) {
			    	object->string = (char *)malloc(1);
			    	object->string[0] = 0;
				return STRING;
			}
			object->string = getString(STATE);
			if (!object->string) {
				return SYNTAX_ERROR;
			}
			if ((getTag(STATE, tag, &attributeCount, attributes, values) != TAG_END)
			   || strcmp(tag, "string")) {
				return SYNTAX_ERROR;
			}
			return STRING;
		}
		if (!strcmp(tag, "set")) {
			if (tagType == TAG_EMPTY) {
				object->elements = NULL;
				return SET;;
			}
			if (tagType == TAG_START) {
				return '[';
			} else {
				return ']';
			}
		}
		break;
	case 't':
		if (!strcmp(tag, "true")) {
			if (tagType == TAG_EMPTY) {
				object->number = 1;
				return BOOLEAN;
			}
		}
		break;
	}

	return SYNTAX_ERROR;
}

// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#
// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#
// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#

// "java" like allocation, if this code hits a syntax error in the
// the middle of the parsed string we just bail with pointers hanging
// all over place, this code helps keeps it all together

//static int object_count = 0;

object_t *
newObject(parser_state_t *state)
{
	object_t *o;

	if (state->freeObjects) {
		o = state->freeObjects;
		state->freeObjects = state->freeObjects->next;
	} else {
		o = (object_t *)malloc(sizeof(object_t));
//		object_count++;
		memset(o, 0, sizeof(object_t));
		o->free = state->objects;
		state->objects = o;
	}
	
	return o;
}

void
freeObject(parser_state_t * state, object_t *o)
{
	o->next = state->freeObjects;
	state->freeObjects = o;	
}

void
cleanupObjects(parser_state_t *state)
{
	object_t *t, *o = state->objects;

	while (o) {
		if (o->object) {
//			printf("IOCFUnserialize: releasing object o=%x object=%x\n", (int)o, (int)o->object);
			CFRelease(o->object);
		}
		if (o->data) {
//			printf("IOCFUnserialize: freeing   object o=%x data=%x\n", (int)o, (int)o->data);
			free(o->data);
		}
		if (o->key) {
//			printf("IOCFUnserialize: releasing object o=%x key=%x\n", (int)o, (int)o->key);
			CFRelease(o->key);
		}
		if (o->string) {
//			printf("IOCFUnserialize: freeing   object o=%x string=%x\n", (int)o, (int)o->string);
			free(o->string);
		}

		t = o;
		o = o->free;
		free(t);
//		object_count--;
	}
//	printf("object_count = %d\n", object_count);
}

// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#
// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#
// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#

static void 
rememberObject(parser_state_t *state, intptr_t tag, CFTypeRef o)
{
//	printf("remember idref %d\n", tag);

	CFDictionarySetValue(state->tags, (void *) tag,  o);
}

static object_t *
retrieveObject(parser_state_t *state, intptr_t tag)
{
	CFTypeRef ref;
	object_t *o;

//	printf("retrieve idref '%d'\n", tag);

	ref = (CFTypeRef) CFDictionaryGetValue(state->tags, (void *) tag);
	if (!ref) return 0;

	o = newObject(state);
	o->object = ref;
	return o;
}

// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#
// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#
// !@$&)(^Q$&*^!$(*!@$_(^%_(*Q#$(_*&!$_(*&!$_(*&!#$(*!@&^!@#%!_!#

object_t *
buildDictionary(parser_state_t *state, object_t * header)
{
	object_t *o, *t;
	int count = 0;
	CFMutableDictionaryRef dict;

	// get count and reverse order
	o = header->elements;
	header->elements = 0;
	while (o) {
		count++;
		t = o;
		o = o->next;

		t->next = header->elements;
		header->elements = t;
	}

        dict = CFDictionaryCreateMutable(state->allocator, count,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
	if (header->idref >= 0) rememberObject(state, header->idref, dict);

	o = header->elements;
	while (o) {
		CFDictionarySetValue(dict, o->key, o->object);

		CFRelease(o->key); 
		CFRelease(o->object); 
		o->key = 0;
		o->object = 0;

		t = o;
		o = o->next;
		freeObject(state, t);
	}
	o = header;
	o->object = dict;
	return o;
};

object_t *
buildArray(parser_state_t *state, object_t * header)
{
	object_t *o, *t;
	int count = 0;
	CFMutableArrayRef array;

	// get count and reverse order
	o = header->elements;
	header->elements = 0;
	while (o) {
		count++;
		t = o;
		o = o->next;

		t->next = header->elements;
		header->elements = t;
	}

	array = CFArrayCreateMutable(state->allocator, count, &kCFTypeArrayCallBacks);
	if (header->idref >= 0) rememberObject(state, header->idref, array);

	o = header->elements;
	while (o) {
		CFArrayAppendValue(array, o->object);

		CFRelease(o->object);
		o->object = 0;

		t = o;
		o = o->next;
		freeObject(state, t);
	}
	o = header;
	o->object = array;
	return o;
};

object_t *
buildSet(parser_state_t *state, object_t *header)
{
	object_t *o, *t;
	int count = 0;
	CFMutableSetRef set;

	// get count and reverse order
	o = header->elements;
	header->elements = 0;
	while (o) {
		count++;
		t = o;
		o = o->next;

		t->next = header->elements;
		header->elements = t;
	}

	set = CFSetCreateMutable(state->allocator, count, &kCFTypeSetCallBacks);
	if (header->idref >= 0) rememberObject(state, header->idref, set);

	o = header->elements;
	while (o) {
		CFSetAddValue(set, o->object);

		CFRelease(o->object);
		o->object = 0;

		t = o;
		o = o->next;
		freeObject(state, t);
	}
	o = header;
	o->object = set;
	return o;
};

object_t *
buildString(parser_state_t *state, object_t *o)
{
	CFStringRef string;

	string = CFStringCreateWithCString(state->allocator, o->string,
					   kCFStringEncodingUTF8);
	if (!string) {
	    syslog(LOG_ERR, "FIXME: IOUnserialize has detected a string that is not valid UTF-8, \"%s\".", o->string);
	    string = CFStringCreateWithCString(state->allocator, o->string,
					   kCFStringEncodingMacRoman);
	}
	    
	if (o->idref >= 0) rememberObject(state, o->idref, string);

	free(o->string);
	o->string = 0;
	o->object = string;

	return o;
};

object_t *
buildData(parser_state_t *state, object_t *o)
{
	CFDataRef data;

	data = CFDataCreate(state->allocator, (const UInt8 *) o->data, o->size);
	if (o->idref >= 0) rememberObject(state, o->idref, data);

	if (o->size) free(o->data);
	o->data = 0;
	o->object = data;
	return o;
};

object_t *
buildNumber(parser_state_t *state, object_t *o)
{
	CFNumberRef 	number;
	CFNumberType 	numType;
	const UInt8 *	bytes;

	bytes = (const UInt8 *) &o->number;
	if (o->size <= 32) {
		numType = kCFNumberSInt32Type;
#if __BIG_ENDIAN__
		bytes += 4;
#endif
	} else {
		numType = kCFNumberSInt64Type;
	}

	number = CFNumberCreate(state->allocator, numType,
				(const void *) bytes);

	if (o->idref >= 0) rememberObject(state, o->idref, number);

	o->object = number;
	return o;
};

object_t *
buildBoolean(parser_state_t *state __unused, object_t *o)
{
	o->object = CFRetain((o->number == 0) ? kCFBooleanFalse : kCFBooleanTrue);
	return o;
};

CFTypeRef
IOCFUnserialize(const char	*buffer,
                CFAllocatorRef	allocator,
                CFOptionFlags	options,
                CFStringRef	*errorString)
{
	CFTypeRef object;
	parser_state_t *state;

	// just in case
	if (errorString) *errorString = NULL;

	if ((!buffer) || options) return 0;

	state = (parser_state_t *) malloc(sizeof(parser_state_t));

	if (!state) return 0;

	state->parseBuffer = buffer;
	state->parseBufferIndex = 0;
	state->lineNumber = 1;
	state->allocator = allocator;
	state->objects = 0;
	state->freeObjects = 0;
	state->tags = CFDictionaryCreateMutable(allocator, 0, 0, /* key callbacks */
						&kCFTypeDictionaryValueCallBacks);
	state->errorString = errorString;
	state->parsedObject = 0;

	(void)yyparse((void *)state);

	object = state->parsedObject;

	cleanupObjects(state);
	CFRelease(state->tags);
	free(state);

	return object;
}


//
//
//
//
//
//		 DO NOT EDIT IOCFUnserialize.tab.c
//
//			this means you!
//
//
//
//
//
