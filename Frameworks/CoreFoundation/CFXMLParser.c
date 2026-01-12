/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFXMLParser.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: David Smith
*/

#include <CoreFoundation/CFXMLParser.h>
#include <CoreFoundation/CFNumber.h>
#include "CFXMLInputStream.h"
#include "CFUniChar.h" 
#include "CFInternal.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

struct __CFXMLParser {
    CFRuntimeBase _cfBase;

    _CFXMLInputStream input;
    
    void **stack;
    void **top;
    UInt32 capacity;

    struct __CFXMLNode *node;  // Our private node; we use it to report back information
    CFMutableDictionaryRef argDict;
    CFMutableArrayRef argArray;

    UInt32 options;
    CFXMLParserCallBacks callBacks;
    CFXMLParserContext context;

    CFXMLParserStatusCode status;
    CFStringRef errorString;
};

static CFStringRef __CFXMLParserCopyDescription(CFTypeRef cf) {
    const struct __CFXMLParser *parser = (const struct __CFXMLParser *)cf;
    return CFStringCreateWithFormat(CFGetAllocator(cf), NULL, CFSTR("<CFXMLParser %p>"), parser);
}

static void __CFXMLParserDeallocate(CFTypeRef cf) {
    struct __CFXMLParser *parser = (struct __CFXMLParser *)cf;
    CFAllocatorRef alloc = CFGetAllocator(parser);
    _freeInputStream(&(parser->input));
    if (parser->argDict) CFRelease(parser->argDict);
    if (parser->argArray) CFRelease(parser->argArray);
    if (parser->errorString) CFRelease(parser->errorString);
    if (parser->node) CFRelease(parser->node);
    CFAllocatorDeallocate(alloc, parser->stack);
    if (parser->context.info && parser->context.release) {
        parser->context.release(parser->context.info);
    }
}

static CFTypeID __kCFXMLParserTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFXMLParserClass = {
    0,
    "CFXMLParser",
    NULL,      // init
    NULL,      // copy
    __CFXMLParserDeallocate,
    NULL,
    NULL,
    NULL,      // 
    __CFXMLParserCopyDescription
};

CFTypeID CFXMLParserGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFXMLParserTypeID = _CFRuntimeRegisterClass(&__CFXMLParserClass); });
    return __kCFXMLParserTypeID;
}

void CFXMLParserGetContext(CFXMLParserRef parser, CFXMLParserContext *context) {
    CFAssert1(parser != NULL, __kCFLogAssertion, "%s(): NULL parser not permitted", __PRETTY_FUNCTION__);
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    if (context) {
	context->version = parser->context.version;
	context->info = parser->context.info;
	context->retain = parser->context.retain;
	context->release = parser->context.release;
	context->copyDescription = parser->context.copyDescription;
	UNFAULT_CALLBACK(context->retain);
	UNFAULT_CALLBACK(context->release);
	UNFAULT_CALLBACK(context->copyDescription);
    }
}

void CFXMLParserGetCallBacks(CFXMLParserRef parser, CFXMLParserCallBacks *callBacks) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    if (callBacks) {
        callBacks->version = parser->callBacks.version;
        callBacks->createXMLStructure = parser->callBacks.createXMLStructure;
        callBacks->addChild = parser->callBacks.addChild;
        callBacks->endXMLStructure = parser->callBacks.endXMLStructure;
        callBacks->resolveExternalEntity = parser->callBacks.resolveExternalEntity;
        callBacks->handleError = parser->callBacks.handleError;
	UNFAULT_CALLBACK(callBacks->createXMLStructure);
	UNFAULT_CALLBACK(callBacks->addChild);
	UNFAULT_CALLBACK(callBacks->endXMLStructure);
	UNFAULT_CALLBACK(callBacks->resolveExternalEntity);
	UNFAULT_CALLBACK(callBacks->handleError);
    }
}

CFURLRef CFXMLParserGetSourceURL(CFXMLParserRef parser) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    return parser->input.url;
}

/* Returns the character index or line number of the current parse location */
CFIndex CFXMLParserGetLocation(CFXMLParserRef parser) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    return _inputStreamCurrentLocation(&parser->input);
}

CFIndex CFXMLParserGetLineNumber(CFXMLParserRef parser) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    return _inputStreamCurrentLine(&parser->input);
}

/* Returns the top-most object returned by the createXMLStructure callback */
void *CFXMLParserGetDocument(CFXMLParserRef parser) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    if (parser->capacity > 0)
        return parser->stack[0];
    else
        return NULL;
}

CFXMLParserStatusCode CFXMLParserGetStatusCode(CFXMLParserRef parser) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    return parser->status;
}

CFStringRef CFXMLParserCopyErrorDescription(CFXMLParserRef parser) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    return (CFStringRef)CFRetain(parser->errorString);
}

void CFXMLParserAbort(CFXMLParserRef parser, CFXMLParserStatusCode errorCode, CFStringRef errorDescription) {
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    CFAssert1(errorCode > 0, __kCFLogAssertion, "%s(): errorCode must be greater than zero", __PRETTY_FUNCTION__);
    CFAssert1(errorDescription != NULL, __kCFLogAssertion, "%s(): errorDescription may not be NULL", __PRETTY_FUNCTION__);
    __CFGenericValidateType(errorDescription, CFStringGetTypeID());

    parser->status = errorCode;
    if (parser->errorString) CFRelease(parser->errorString);
    parser->errorString = (CFStringRef)CFStringCreateCopy(kCFAllocatorSystemDefault, errorDescription);
}


static Boolean parseXML(CFXMLParserRef parser);
static Boolean parseComment(CFXMLParserRef parser, Boolean report);
static Boolean parseProcessingInstruction(CFXMLParserRef parser, Boolean report);
static Boolean parseInlineDTD(CFXMLParserRef parser);
static Boolean parseDTD(CFXMLParserRef parser);
static Boolean parsePhysicalEntityReference(CFXMLParserRef parser);
static Boolean parseCDSect(CFXMLParserRef parser);
static Boolean parseEntityReference(CFXMLParserRef parser, Boolean report);
static Boolean parsePCData(CFXMLParserRef parser);
static Boolean parseWhitespace(CFXMLParserRef parser);
static Boolean parseAttributeListDeclaration(CFXMLParserRef parser);
static Boolean parseNotationDeclaration(CFXMLParserRef parser);
static Boolean parseElementDeclaration(CFXMLParserRef parser);
static Boolean parseEntityDeclaration(CFXMLParserRef parser);
static Boolean parseExternalID(CFXMLParserRef parser, Boolean alsoAcceptPublicID, CFXMLExternalID *extID);
static Boolean parseCloseTag(CFXMLParserRef parser, CFStringRef tag);
static Boolean parseTagContent(CFXMLParserRef parser);
static Boolean parseTag(CFXMLParserRef parser);
static Boolean parseAttributes(CFXMLParserRef parser);
static Boolean parseAttributeValue(CFXMLParserRef parser, CFMutableStringRef str);

// Utilities; may need to make these accessible to the property list parser to avoid code duplication
static void _CFReportError(CFXMLParserRef parser, CFXMLParserStatusCode errNum, const char *str);
static Boolean reportNewLeaf(CFXMLParserRef parser); // Assumes parser->node has been set and is ready to go
static void pushXMLNode(CFXMLParserRef parser, void *node);

static CFXMLParserRef __CFXMLParserInit(CFAllocatorRef alloc, CFURLRef dataSource, CFOptionFlags options, CFDataRef xmlData, CFIndex version, CFXMLParserCallBacks *callBacks, CFXMLParserContext *context) {
    struct __CFXMLParser *parser = (struct __CFXMLParser *)_CFRuntimeCreateInstance(alloc, CFXMLParserGetTypeID(), sizeof(struct __CFXMLParser) - sizeof(CFRuntimeBase), NULL);
    struct __CFXMLNode *node = (struct __CFXMLNode *)_CFRuntimeCreateInstance(alloc, CFXMLNodeGetTypeID(), sizeof(struct __CFXMLNode) - sizeof(CFRuntimeBase), NULL);
    UniChar *buf;
    if (parser && node) {
        alloc = CFGetAllocator(parser);
        _initializeInputStream(&(parser->input), alloc, dataSource, xmlData);
        parser->top = parser->stack;
        parser->stack = NULL;
        parser->capacity = 0;

        buf = (UniChar *)CFAllocatorAllocate(alloc, 128*sizeof(UniChar), 0);
        parser->node = node;
        parser->node->dataString = CFStringCreateMutableWithExternalCharactersNoCopy(alloc, buf, 0, 128, alloc);
        parser->node->additionalData = NULL;
        parser->node->version = version;
        parser->argDict = NULL; // don't create these until necessary
        parser->argArray = NULL;

        parser->options = options;
        parser->callBacks = *callBacks;

        FAULT_CALLBACK((void **)&(parser->callBacks.createXMLStructure));
        FAULT_CALLBACK((void **)&(parser->callBacks.addChild));
        FAULT_CALLBACK((void **)&(parser->callBacks.endXMLStructure));
        FAULT_CALLBACK((void **)&(parser->callBacks.resolveExternalEntity));
        FAULT_CALLBACK((void **)&(parser->callBacks.handleError));
        
        if (context) {
            parser->context = *context;
            if (parser->context.info && parser->context.retain) {
                parser->context.retain(parser->context.info);
            }
        } else {
            parser->context.version = 0;
            parser->context.info = NULL;
            parser->context.retain = NULL;
            parser->context.release = NULL;
            parser->context.copyDescription = NULL;
        }
        parser->status = kCFXMLStatusParseNotBegun;
        parser->errorString = NULL;
    } else {
        if (parser) CFRelease(parser);
        if (node) CFRelease(node);
        parser = NULL;
    }
    return parser;
}

CFXMLParserRef CFXMLParserCreate(CFAllocatorRef allocator, CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes, CFXMLParserCallBacks *callBacks, CFXMLParserContext *context) {
    CFAssert1(xmlData != NULL, __kCFLogAssertion, "%s(): NULL data not permitted", __PRETTY_FUNCTION__);
    __CFGenericValidateType(xmlData, CFDataGetTypeID());
    CFAssert1(dataSource == NULL || CFGetTypeID(dataSource) == CFURLGetTypeID(), __kCFLogAssertion, "%s(): dataSource is not a valid CFURL", __PRETTY_FUNCTION__);
    CFAssert1(callBacks != NULL && callBacks->createXMLStructure != NULL && callBacks->addChild != NULL && callBacks->endXMLStructure != NULL, __kCFLogAssertion, "%s(): callbacks createXMLStructure, addChild, and endXMLStructure must all be non-NULL", __PRETTY_FUNCTION__);
    CFAssert2(versionOfNodes <= 1, __kCFLogAssertion, "%s(): version number %d is higher than supported by CFXMLParser", __PRETTY_FUNCTION__, versionOfNodes);
    CFAssert1(versionOfNodes != 0, __kCFLogAssertion, "%s(): version number 0 is no longer supported by CFXMLParser", __PRETTY_FUNCTION__);
    return __CFXMLParserInit(allocator, dataSource, parseOptions, xmlData, versionOfNodes, callBacks, context);
}

CFXMLParserRef CFXMLParserCreateWithDataFromURL(CFAllocatorRef allocator, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes, CFXMLParserCallBacks *callBacks, CFXMLParserContext *context) {
    CFAssert1(dataSource == NULL || CFGetTypeID(dataSource) == CFURLGetTypeID(), __kCFLogAssertion, "%s(): dataSource is not a valid CFURL", __PRETTY_FUNCTION__);
    CFAssert1(callBacks != NULL && callBacks->createXMLStructure != NULL && callBacks->addChild != NULL && callBacks->endXMLStructure != NULL, __kCFLogAssertion, "%s(): callbacks createXMLStructure, addChild, and endXMLStructure must all be non-NULL", __PRETTY_FUNCTION__);
    CFAssert2(versionOfNodes <= 1, __kCFLogAssertion, "%s(): version number %d is higher than supported by CFXMLParser", __PRETTY_FUNCTION__, versionOfNodes);
    CFAssert1(versionOfNodes != 0, __kCFLogAssertion, "%s(): version number 0 is no longer supported by CFXMLParser", __PRETTY_FUNCTION__);

    return __CFXMLParserInit(allocator, dataSource, parseOptions, NULL, versionOfNodes, callBacks, context);
}

Boolean CFXMLParserParse(CFXMLParserRef parser) {
    CFXMLDocumentInfo docData;
    __CFGenericValidateType(parser, CFXMLParserGetTypeID());
    if (parser->status != kCFXMLStatusParseNotBegun) return false;
    parser->status = kCFXMLStatusParseInProgress;

    if (!_openInputStream(&parser->input)) {
        if (!parser->input.data) {
            // couldn't load URL
            parser->status = kCFXMLErrorNoData;
            parser->errorString = CFStringCreateWithFormat(CFGetAllocator(parser), NULL, CFSTR("No data found at %@"), CFURLGetString(parser->input.url));
        } else {
            // couldn't figure out the encoding
            CFAssert(parser->input.encoding == kCFStringEncodingInvalidId, __kCFLogAssertion, "CFXMLParser internal error: input stream could not be opened");
            parser->status = kCFXMLErrorUnknownEncoding;
            parser->errorString = CFStringCreateWithCString(CFGetAllocator(parser), "Encountered unknown encoding", kCFStringEncodingASCII);
        }
        if (parser->callBacks.handleError) {
            INVOKE_CALLBACK3(parser->callBacks.handleError, parser, parser->status, parser->context.info);
        }
        return false;
    }
    
    // Create the document
    parser->stack = (void **)CFAllocatorAllocate(CFGetAllocator(parser), 16 * sizeof(void *), 0);
    parser->capacity = 16;
    parser->node->dataTypeID = kCFXMLNodeTypeDocument;
    docData.encoding = _inputStreamGetEncoding(&parser->input);
    docData.sourceURL = parser->input.url;
    parser->node->additionalData = &docData;
    parser->stack[0] = (void *)INVOKE_CALLBACK3(parser->callBacks.createXMLStructure, parser, parser->node, parser->context.info);
    parser->top = parser->stack;
    parser->node->additionalData = NULL;

    // Client may have called CFXMLParserAbort() during any callback, so we must always check to see if we have an error status after a callback
    if (parser->status != kCFXMLStatusParseInProgress) {
        _CFReportError(parser, parser->status, NULL);
        return false;
    }
    return parseXML(parser);
}

/* The next several functions are all intended to parse past a particular XML structure.  They expect parser->curr to be set to the first content character of their structure (e.g. parseXMLComment expects parser->curr to be set just past "<!--").  They parse to the end of their structure, calling any necessary callbacks along the way, and advancing parser->curr as they go.  They either return void (not possible for the parse to fail) or they return a Boolean (success/failure).  The calling routines are expected to catch returned Booleans and fail immediately if false is returned. */

// [3]  S ::= (#x20 | #x9 | #xD | #xA)+ 
static Boolean parseWhitespace(CFXMLParserRef parser) {
    CFIndex len;
    Boolean report = !(parser->options & kCFXMLParserSkipWhitespace);
    len = _inputStreamSkipWhitespace(&parser->input, report ? (CFMutableStringRef)(parser->node->dataString) : NULL);
    if (report && len) {
        parser->node->dataTypeID = kCFXMLNodeTypeWhitespace;
        parser->node->additionalData = NULL;
        return reportNewLeaf(parser);
    } else {
        return true;
    }
}

// parser should be just past "<!--"
static Boolean parseComment(CFXMLParserRef parser, Boolean report) {
    const UniChar dashes[2] = {'-', '-'};
    UniChar ch;
    report = report && (!(parser->options & kCFXMLParserSkipMetaData));
    if (!_inputStreamScanToCharacters(&parser->input, dashes, 2, report ? (CFMutableStringRef)(parser->node->dataString) : NULL) || !_inputStreamGetCharacter(&parser->input, &ch)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF,"Found unexpected EOF while parsing comment");
        return false;
    } else if (ch != '>') {
        _CFReportError(parser, kCFXMLErrorMalformedComment, "Found \"--\" within a comment");
        return false;
    } else if (report) {
        parser->node->dataTypeID = kCFXMLNodeTypeComment;
        parser->node->additionalData = NULL;
        return reportNewLeaf(parser);
    } else {
        return true;
    }
}

/* 
[16] PI ::= '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
[17] PITarget ::= Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
 */
// parser should be set to the first character after "<?"
static Boolean parseProcessingInstruction(CFXMLParserRef parser, Boolean report) {
    const UniChar piTermination[2] = {'?', '>'};
    CFMutableStringRef str;
    CFStringRef name;
    
    if (!_inputStreamScanXMLName(&parser->input, false, &name)) {
        _CFReportError(parser, kCFXMLErrorMalformedProcessingInstruction, "Found malformed processing instruction");
        return false;
    }
    _inputStreamSkipWhitespace(&parser->input, NULL);
    str = (report && *parser->top) ? CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser)) : NULL;
    if (!_inputStreamScanToCharacters(&parser->input, piTermination, 2, str)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing processing instruction");
        if (str) CFRelease(str);
        return false;
    }

    if (str) {
        CFXMLProcessingInstructionInfo data;
        Boolean result;
        CFStringRef tmp = parser->node->dataString;
        parser->node->dataTypeID = kCFXMLNodeTypeProcessingInstruction;
        parser->node->dataString = name;
        data.dataString = str;
        parser->node->additionalData = &data;
        result = reportNewLeaf(parser);
        parser->node->additionalData = NULL;
        parser->node->dataString = tmp;
        CFRelease(str);
        return result;
    } else {
        return true;
    }
}

/*
 [28] doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | PEReference | S)* ']' S?)? '>'
*/
static const UniChar _DoctypeOpening[7] = {'D', 'O', 'C', 'T', 'Y', 'P', 'E'};
// first character should be immediately after the "<!"
static Boolean parseDTD(CFXMLParserRef parser) {
    UniChar ch;
    Boolean success, hasExtID = false;
    CFXMLDocumentTypeInfo docData = {{NULL, NULL}};
    void *dtdStructure = NULL;
    CFStringRef name;
    
    // First pass "DOCTYPE"
    success = _inputStreamMatchString(&parser->input, _DoctypeOpening, 7);
    success = success && _inputStreamSkipWhitespace(&parser->input, NULL) != 0;
    success = success && _inputStreamScanXMLName(&parser->input, false, &name);
    if (success) {
        _inputStreamSkipWhitespace(&parser->input, NULL);
        success = _inputStreamPeekCharacter(&parser->input, &ch);
    } else {
        // didn't make it past "DOCTYPE" successfully.
        _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found malformed DTD");
        return false;
    }
    if (success && ch != '[' && ch != '>') {
        // ExternalID
        hasExtID = true;
        success = parseExternalID(parser, false, &(docData.externalID));
        if (success)  {
            _inputStreamSkipWhitespace(&parser->input, NULL);
            success = _inputStreamPeekCharacter(&parser->input, &ch);
        }
    }

    if (!(parser->options & kCFXMLParserSkipMetaData) && *(parser->top)) {
        CFStringRef tmp = parser->node->dataString;
        parser->node->dataTypeID = kCFXMLNodeTypeDocumentType;
        parser->node->dataString = name;
        parser->node->additionalData = &docData;
        dtdStructure = (void *)INVOKE_CALLBACK3(parser->callBacks.createXMLStructure, parser, parser->node, parser->context.info);
        if (dtdStructure && parser->status == kCFXMLStatusParseInProgress) {
            INVOKE_CALLBACK4(parser->callBacks.addChild, parser, *parser->top, dtdStructure, parser->context.info);
        }
        parser->node->additionalData = NULL;
        parser->node->dataString = tmp;
        if (parser->status != kCFXMLStatusParseInProgress) {
            // callback called CFXMLParserAbort()
            _CFReportError(parser, parser->status, NULL);
            return false;
        }
    } else {
        dtdStructure = NULL;
    }
    if (docData.externalID.publicID) CFRelease(docData.externalID.publicID);
    if (docData.externalID.systemID) CFRelease(docData.externalID.systemID);
    pushXMLNode(parser, dtdStructure);

    if (success && ch == '[')  {
        // inline DTD
        _inputStreamGetCharacter(&parser->input, &ch);
        if (!parseInlineDTD(parser)) return false;
        _inputStreamSkipWhitespace(&parser->input, NULL);
        success = _inputStreamGetCharacter(&parser->input, &ch) && ch == '>';
    } else if (success && ch == '>') {
        // End of the DTD
        _inputStreamGetCharacter(&parser->input, &ch);
    }
    if (!success) {
        if (_inputStreamAtEOF(&parser->input)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing DTD");
        } else {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found malformed DTD");
        }
        return false;
    }

    parser->top --; // Remove dtdStructure from the stack

    if (success && dtdStructure) {
        INVOKE_CALLBACK3(parser->callBacks.endXMLStructure, parser, dtdStructure, parser->context.info);
        if (parser->status != kCFXMLStatusParseInProgress) {
            _CFReportError(parser, parser->status, NULL);
            return false;
        }
    }
    return true;
}

/*
 [69] PEReference ::= '%' Name ';'
*/
static Boolean parsePhysicalEntityReference(CFXMLParserRef parser) {
    UniChar ch;
    CFStringRef name;
    if (!_inputStreamScanXMLName(&parser->input, false, &name)) {
        _CFReportError(parser, kCFXMLErrorMalformedName, "Found malformed name while parsing physical entity reference");
        return false;
    } else if (!_inputStreamGetCharacter(&parser->input, &ch)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing physical entity reference");
        return false;
    } else if (ch != ';') {
        _CFReportError(parser, kCFXMLErrorMalformedName, "Found malformed name while parsing physical entity reference");
        return false;
    } else if (!(parser->options & kCFXMLParserSkipMetaData) && *(parser->top)) {
        CFXMLEntityReferenceInfo myData;
        Boolean result;
        CFStringRef tmp = parser->node->dataString;
        parser->node->dataTypeID = kCFXMLNodeTypeEntityReference;
        parser->node->dataString = name;
        myData.entityType = kCFXMLEntityTypeParameter;
        parser->node->additionalData = &myData;
        result = reportNewLeaf(parser);
        parser->node->additionalData = NULL;
        parser->node->dataString = tmp;
        return result;
    } else {
        return true;
    }
}

/*
 [54] AttType ::= StringType | TokenizedType | EnumeratedType
 [55] StringType ::= 'CDATA'
 [56] TokenizedType ::= 'ID' | 'IDREF'| 'IDREFS'| 'ENTITY'| 'ENTITIES'| 'NMTOKEN'| 'NMTOKENS'
 [57] EnumeratedType ::= NotationType | Enumeration
 [58] NotationType ::= 'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'
 [59] Enumeration ::= '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')'
*/
static Boolean parseEnumeration(CFXMLParserRef parser, Boolean useNMTokens) {
    UniChar ch;
    Boolean done = false;
    if (!_inputStreamGetCharacter(&parser->input, &ch)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
        return false;
    } else if (ch != '(') {
        _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
        return false;
    }
    _inputStreamSkipWhitespace(&parser->input, NULL);
    if (!_inputStreamScanXMLName(&parser->input, useNMTokens, NULL)) {
        _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
        return false;
    }
    while (!done) {
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamGetCharacter(&parser->input, &ch)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
            return false;
        } else if (ch == ')') {
            done = true;
        } else if (ch == '|') {
            _inputStreamSkipWhitespace(&parser->input, NULL);
            if (!_inputStreamScanXMLName(&parser->input, useNMTokens, NULL)) {
                _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
                return false;
            }
        } else {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
            return false;
        }
    }
    return true;
}

static Boolean parseAttributeType(CFXMLParserRef parser, CFMutableStringRef str) {
    Boolean success = false;
    static const UniChar attTypeStrings[6][8] = {
    {'C', 'D', 'A', 'T', 'A', '\0', '\0', '\0'},
    {'I', 'D', 'R', 'E', 'F', 'S', '\0', '\0'},
    {'E', 'N', 'T', 'I', 'T', 'Y', '\0', '\0'},
    {'E', 'N', 'T', 'I', 'T', 'I', 'E', 'S'},
    {'N', 'M', 'T', 'O', 'K', 'E', 'N', 'S'},
    {'N', 'O', 'T', 'A', 'T', 'I', 'O', 'N'} };
    if (str) _inputStreamSetMark(&parser->input);
    if (_inputStreamMatchString(&parser->input, attTypeStrings[0], 5) ||
        _inputStreamMatchString(&parser->input, attTypeStrings[1], 6) ||
        _inputStreamMatchString(&parser->input, attTypeStrings[1], 5) ||
        _inputStreamMatchString(&parser->input, attTypeStrings[1], 2) ||
        _inputStreamMatchString(&parser->input, attTypeStrings[2], 6) ||
        _inputStreamMatchString(&parser->input, attTypeStrings[3], 8) ||
        _inputStreamMatchString(&parser->input, attTypeStrings[4], 8) ||
        _inputStreamMatchString(&parser->input, attTypeStrings[4], 7)) {
        success = true;
    } else if (_inputStreamMatchString(&parser->input, attTypeStrings[5], 8)) {
        // Notation
        if (_inputStreamSkipWhitespace(&parser->input, NULL) == 0) {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
            success = false;
        } else  {
            success = parseEnumeration(parser, false);
        }
    } else {
        success = parseEnumeration(parser, true);
    }
    if (str) {
        if (success) {
            _inputStreamGetCharactersFromMark(&parser->input, str);
        }
        _inputStreamClearMark(&parser->input);
    }
    return success;
}

/*  [60] DefaultDecl ::= '#REQUIRED' | '#IMPLIED' | (('#FIXED' S)? AttValue) */
static Boolean parseAttributeDefaultDeclaration(CFXMLParserRef parser, CFMutableStringRef str) {
    const UniChar strings[3][8] = {
    {'R', 'E', 'Q', 'U', 'I', 'R', 'E', 'D'},
    {'I', 'M', 'P', 'L', 'I', 'E', 'D', '\0'},
    {'F', 'I', 'X', 'E', 'D', '\0', '\0', '\0'}};
    UniChar ch;
    Boolean success;
    if (str) _inputStreamSetMark(&parser->input);
    if (!_inputStreamGetCharacter(&parser->input, &ch)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
        success = false;
    } else if (ch == '#') {
        if (_inputStreamMatchString(&parser->input, strings[0], 8) ||
            _inputStreamMatchString(&parser->input, strings[1], 7)) {
            success = true;
        } else if (!_inputStreamMatchString(&parser->input, strings[2], 5) || _inputStreamSkipWhitespace(&parser->input, NULL) == 0) {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
            success = false;
        } else {
            // we fall through if "#FIXED" was matched, and at least one whitespace character was stripped.
            success = parseAttributeValue(parser, NULL);
        }
    } else {
        _inputStreamReturnCharacter(&parser->input, ch);
        success = parseAttributeValue(parser, NULL);
    }
    if (str) {
        if (success) {
            _inputStreamGetCharactersFromMark(&parser->input, str);
        }
        _inputStreamClearMark(&parser->input);
    }
    return success;
}

/*
 [52] AttlistDecl ::= '<!ATTLIST' S Name AttDef* S? '>'
 [53] AttDef ::= S Name S AttType S DefaultDecl
*/
static Boolean parseAttributeListDeclaration(CFXMLParserRef parser) {
    const UniChar attList[7] = {'A', 'T', 'T', 'L', 'I', 'S', 'T'};
    CFXMLAttributeListDeclarationInfo attListData;
    CFXMLAttributeDeclarationInfo attributeArray[8], *attributes=attributeArray;
    CFIndex capacity = 8;
    UniChar ch;
    Boolean success = true;
    CFStringRef name;
    if (!_inputStreamMatchString(&parser->input, attList, 7) ||
        _inputStreamSkipWhitespace(&parser->input, NULL) == 0 ||
        !_inputStreamScanXMLName(&parser->input, false, &name)) {
        _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
        return false;
    }
    attListData.numberOfAttributes = 0;
    if (!(*parser->top) || (parser->options & kCFXMLParserSkipMetaData)) {
        // Use this to mark that we don't need to collect attribute information to report to the client.  Ultimately, we may want to collect this for our own use (for validation, for instance), but for now, the only reason we would create it would be for the client.  -- REW, 2/9/2000
        attributes = NULL;
    }
    while (_inputStreamPeekCharacter(&parser->input, &ch) && ch != '>' && _inputStreamSkipWhitespace(&parser->input, NULL) != 0) {
        CFXMLAttributeDeclarationInfo *attribute = NULL;
        if (_inputStreamPeekCharacter(&parser->input, &ch) && ch == '>')
            break;
        if (attributes) {
            if (capacity == attListData.numberOfAttributes) {
                capacity = 2*capacity;
                if (attributes != attributeArray) {
                    attributes = (CFXMLAttributeDeclarationInfo *)CFAllocatorReallocate(CFGetAllocator(parser), attributes, capacity * sizeof(CFXMLAttributeDeclarationInfo), 0);
                } else {
                    attributes = (CFXMLAttributeDeclarationInfo *)CFAllocatorAllocate(CFGetAllocator(parser), capacity * sizeof(CFXMLAttributeDeclarationInfo), 0);
                }
            }
            attribute = &(attributes[attListData.numberOfAttributes]);
            // Much better if we can somehow create these strings immutable - then if the client (or we ourselves) has to copy them, they will end up multiply-retained, rather than having a new alloc and data copy performed.  -- REW, 2/9/2000
            attribute->typeString = CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser));
            attribute->defaultString = CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser));
        }
        if (!_inputStreamScanXMLName(&parser->input, false, &(attribute->attributeName)) || (_inputStreamSkipWhitespace(&parser->input, NULL) == 0)) {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
            success = false;
            break;
        }
        if (!parseAttributeType(parser, attribute ? (CFMutableStringRef)attribute->typeString : NULL)) {
            success = false;
            break;
        }
        if (_inputStreamSkipWhitespace(&parser->input, NULL) == 0) {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
            success = false;
            break;
        }
        if (!parseAttributeDefaultDeclaration(parser, attribute ? (CFMutableStringRef)attribute->defaultString : NULL)) {
            success = false;
            break;
        }
        attListData.numberOfAttributes ++;
    }
    if (success) {
        if (!_inputStreamGetCharacter(&parser->input, &ch)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
            success = false;
        } else if (ch != '>') {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
            success = false;
        } else if (attributes) {
            CFStringRef tmp = parser->node->dataString;
            parser->node->dataTypeID = kCFXMLNodeTypeAttributeListDeclaration;
            parser->node->dataString = name;
            attListData.attributes = attributes;
            parser->node->additionalData = (void *)&attListData;
            success = reportNewLeaf(parser);
            parser->node->additionalData = NULL;
            parser->node->dataString = tmp;
        }
    }
    if (attributes) {
        // Free up all that memory
        CFIndex idx;
        for (idx = 0; idx < attListData.numberOfAttributes; idx ++) {
            // Do not release attributeName here; it's a uniqued string from scanXMLName
            CFRelease(attributes[idx].typeString);
            CFRelease(attributes[idx].defaultString);
        }
        if (attributes != attributeArray) {
            CFAllocatorDeallocate(CFGetAllocator(parser), attributes);
        }
    }
    return success;
}

CF_INLINE Boolean parseSystemLiteral(CFXMLParserRef parser, CFXMLExternalID *extID) {
    Boolean success;
    if (extID) {
        CFMutableStringRef urlStr = CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser));
        if (_inputStreamScanQuotedString(&parser->input, urlStr)) {
            success = true;
            extID->systemID = CFURLCreateWithString(CFGetAllocator(parser), urlStr, parser->input.url);
        } else {
            extID->systemID = NULL;
            success = false;
        }
        CFRelease(urlStr);
    } else {
        success = _inputStreamScanQuotedString(&parser->input, NULL);
    }
    return success;
}

/*
 [75] ExternalID ::= 'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral
 [83] PublicID ::= 'PUBLIC' S PubidLiteral
 [12] PubidLiteral ::= '"' PubidChar* '"' | "'" (PubidChar - "'")* "'"
 [13] PubidChar ::= #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
 [11] SystemLiteral ::= ('"' [^"]* '"') | ("'" [^']* "'") 
*/
// This does NOT report errors itself; caller can check to see if parser->input is at EOF to determine whether the formatting failed or unexpected EOF occurred.  -- REW, 2/2/2000
static Boolean parseExternalID(CFXMLParserRef parser, Boolean alsoAcceptPublicID, CFXMLExternalID *extID) {
    const UniChar publicString[6] = {'P', 'U', 'B', 'L', 'I', 'C'};
    const UniChar systemString[6] = {'S', 'Y', 'S', 'T', 'E', 'M'};
    Boolean success;
    if (extID) {
        extID->systemID = NULL;
        extID->publicID = NULL;
    }
    if (_inputStreamMatchString(&parser->input, publicString, 6)) {
        success = _inputStreamSkipWhitespace(&parser->input, NULL) != 0;
        if (extID) {
            extID->publicID = CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser));
            success = success && _inputStreamScanQuotedString(&parser->input, (CFMutableStringRef)extID->publicID);
        } else {
            success = success && _inputStreamScanQuotedString(&parser->input, NULL);
        }
        if (success) {
            UniChar ch;
            if (alsoAcceptPublicID) {
                _inputStreamSetMark(&parser->input); // In case we need to roll back the parser
            }
            if (_inputStreamSkipWhitespace(&parser->input, NULL) == 0
                || !_inputStreamPeekCharacter(&parser->input, &ch)
                || (ch != '\'' && ch != '\"')
                || !parseSystemLiteral(parser, extID)) {
                success = alsoAcceptPublicID;
                if (alsoAcceptPublicID) {
                    _inputStreamBackUpToMark(&parser->input);
                }
            } else {
                success = true;
            }
            if (alsoAcceptPublicID) {
                _inputStreamClearMark(&parser->input);
            }
        }
    } else if (_inputStreamMatchString(&parser->input, systemString, 6)) {
        success = _inputStreamSkipWhitespace(&parser->input, NULL) != 0 && parseSystemLiteral(parser, extID);
    } else {
        success = false;
    }
    return success;
}

/*
 [82] NotationDecl ::= '<!NOTATION' S Name S (ExternalID |  PublicID) S? '>'
*/
static Boolean parseNotationDeclaration(CFXMLParserRef parser) {
    static UniChar notationString[8] = {'N', 'O', 'T', 'A', 'T', 'I', 'O', 'N'};
    Boolean report = *(parser->top) && !(parser->options & kCFXMLParserSkipMetaData);
    CFXMLNotationInfo notationData = {{NULL, NULL}};
    CFStringRef name;
    Boolean success =
        _inputStreamMatchString(&parser->input, notationString, 8) &&
        _inputStreamSkipWhitespace(&parser->input, NULL) != 0 &&
        _inputStreamScanXMLName(&parser->input, false, report ? &name : NULL) &&
        _inputStreamSkipWhitespace(&parser->input, NULL) != 0 &&
        parseExternalID(parser, true, report ? &(notationData.externalID) : NULL);

    if (success) {
        UniChar ch;
        _inputStreamSkipWhitespace(&parser->input, NULL);
        success = (_inputStreamGetCharacter(&parser->input, &ch) && ch == '>');
    }
    if (!success) {
        if (_inputStreamAtEOF(&parser->input)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
        } else {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
        }
    } else if (report) {
        CFStringRef tmp = parser->node->dataString;
        parser->node->dataTypeID = kCFXMLNodeTypeNotation;
        parser->node->dataString = name;
        parser->node->additionalData = &notationData;
        success = reportNewLeaf(parser);
        parser->node->additionalData = NULL;
        parser->node->dataString = tmp;
    }
    if (notationData.externalID.systemID) CFRelease(notationData.externalID.systemID);
    if (notationData.externalID.publicID) CFRelease(notationData.externalID.publicID);
    return success;
}

/*
 [48] cp ::= (Name | choice | seq) ('?' | '*' | '+')?
 [49] choice ::= '(' S? cp ( S? '|' S? cp )* S? ')'
 [50] seq ::= '(' S? cp ( S? ',' S? cp )* S? ')'
*/
static Boolean parseChoiceOrSequence(CFXMLParserRef parser, Boolean pastParen) {
    UniChar ch, separator;
    if (!pastParen) {
        if (!_inputStreamGetCharacter(&parser->input, &ch) || ch != '(') return false;
        _inputStreamSkipWhitespace(&parser->input, NULL);
    }
    if (!_inputStreamPeekCharacter(&parser->input, &ch)) return false;

    /* Now scanning cp, production [48] */
    if (ch == '(') {
        if (!parseChoiceOrSequence(parser, false)) return false;
    } else {
        if (!_inputStreamScanXMLName(&parser->input, false, NULL)) return false;
    }
    if (!_inputStreamPeekCharacter(&parser->input, &ch)) return false;
    if (ch == '?' || ch == '*' || ch == '+') _inputStreamGetCharacter(&parser->input, &ch);

    /* Now past cp */
    _inputStreamSkipWhitespace(&parser->input, NULL);
    if (!_inputStreamGetCharacter(&parser->input, &ch)) return false;
    if (ch == ')') return true;
    if (ch != '|' && ch != ',') return false;
    separator = ch;
    while (ch == separator) {
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamPeekCharacter(&parser->input, &ch)) return false;
        if (ch != '(') {
            if (!_inputStreamScanXMLName(&parser->input, false, NULL)) return false;
        } else if (!parseChoiceOrSequence(parser, false)) {
            return false;
        }
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamGetCharacter(&parser->input, &ch)) return false;
    }
    return ch == ')';
}

/*
 [51] Mixed ::= '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*' | '(' S? '#PCDATA' S? ')'
*/
static Boolean parseMixedElementContent(CFXMLParserRef parser) {
    static const UniChar pcdataString[7] = {'#', 'P', 'C', 'D', 'A', 'T', 'A'};
    UniChar ch;
    if (!_inputStreamMatchString(&parser->input, pcdataString, 7)) return false;
    _inputStreamSkipWhitespace(&parser->input, NULL);
    if (!_inputStreamGetCharacter(&parser->input, &ch) && (ch == ')' || ch == '|')) return false;
    if (ch == ')') return true;

    while (ch == '|') {
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamScanXMLName(&parser->input, false, NULL)) return false;
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamGetCharacter(&parser->input, &ch)) return false;
    }
    if (ch != ')') return false;
    if (!_inputStreamGetCharacter(&parser->input, &ch) || ch != '*') return false;
    return true;
}

/*
 [46] contentspec ::= 'EMPTY' | 'ANY' | Mixed | children
 [47] children ::= (choice | seq) ('?' | '*' | '+')?
 */
static Boolean parseElementContentSpec(CFXMLParserRef parser) {
    static const UniChar eltContentEmpty[5] = {'E', 'M', 'P', 'T', 'Y'};
    static const UniChar eltContentAny[3] = {'A', 'N', 'Y'};
    UniChar ch;
    if (_inputStreamMatchString(&parser->input, eltContentEmpty, 5) || _inputStreamMatchString(&parser->input, eltContentAny, 3)) {
        return true;
    } else if (!_inputStreamPeekCharacter(&parser->input, &ch) || ch != '(') {
        return false;
    } else {
        // We want to know if we have a Mixed per production [51].  If we don't, we will need to back up and call the parseChoiceOrSequence function.  So we set the mark now.  -- REW, 2/10/2000
        _inputStreamGetCharacter(&parser->input, &ch);
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamPeekCharacter(&parser->input, &ch)) return false;
        if (ch == '#') {
            // Mixed
            return parseMixedElementContent(parser);
        } else {
            if (parseChoiceOrSequence(parser, true)) {
                if (_inputStreamPeekCharacter(&parser->input, &ch) && (ch == '*' || ch == '?' || ch == '+')) {
                    _inputStreamGetCharacter(&parser->input, &ch);
                }
                return true;
            } else {
                return false;
            }
        }
    }
}

/*
 [45] elementdecl ::= '<!ELEMENT' S Name S contentspec S? '>'
 */
static Boolean parseElementDeclaration(CFXMLParserRef parser) {
    Boolean report = *(parser->top) && !(parser->options & kCFXMLParserSkipMetaData);
    Boolean success;
    static const UniChar eltChars[7] = {'E', 'L', 'E', 'M', 'E', 'N', 'T'};
    UniChar ch = '>';
    CFMutableStringRef contentDesc = NULL;
    CFStringRef name;
    success = _inputStreamMatchString(&parser->input, eltChars, 7)
        && _inputStreamSkipWhitespace(&parser->input, NULL) != 0
        && _inputStreamScanXMLName(&parser->input, false, report ? &name : NULL)
        && _inputStreamSkipWhitespace(&parser->input, NULL) != 0;
    if (success) {
        if (report) _inputStreamSetMark(&parser->input);
        success = parseElementContentSpec(parser);
        if (success && report) {
            contentDesc = CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser));
            _inputStreamGetCharactersFromMark(&parser->input, contentDesc);
        }
        if (report) _inputStreamClearMark(&parser->input);
        if (success) _inputStreamSkipWhitespace(&parser->input, NULL);
        success = success && _inputStreamMatchString(&parser->input, &ch, 1);
    }
    if (!success) {
        if (_inputStreamAtEOF(&parser->input)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
        } else {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
        }
    } else if (report) {
        CFXMLElementTypeDeclarationInfo eltData;
        CFStringRef tmp = parser->node->dataString;
        parser->node->dataTypeID = kCFXMLNodeTypeElementTypeDeclaration;
        parser->node->dataString = name;
        eltData.contentDescription = contentDesc;
        parser->node->additionalData = &eltData;
        success = reportNewLeaf(parser);
        parser->node->additionalData = NULL;
        parser->node->dataString = tmp;
    }
    if (contentDesc) CFRelease(contentDesc);
    return success;
}

/*
 [70] EntityDecl ::= GEDecl | PEDecl
 [71] GEDecl ::= '<!ENTITY' S Name S EntityDef S? '>'
 [72] PEDecl ::= '<!ENTITY' S '%' S Name S PEDef S? '>'
 [73] EntityDef ::= EntityValue | (ExternalID NDataDecl?)
 [74] PEDef ::= EntityValue | ExternalID
 [76] NDataDecl ::= S 'NDATA' S Name
 [9]  EntityValue ::= '"' ([^%&"] | PEReference | Reference)* '"' |  "'" ([^%&'] | PEReference | Reference)* "'"
*/
static Boolean parseEntityDeclaration(CFXMLParserRef parser) {
    const UniChar entityStr[6] = {'E', 'N', 'T', 'I', 'T', 'Y'};
    UniChar ch;
    Boolean isPEDecl = false;
    CFXMLEntityInfo entityData;
    CFStringRef name;
    Boolean report = *(parser->top) && !(parser->options & kCFXMLParserSkipMetaData);
    Boolean success =
        _inputStreamMatchString(&parser->input, entityStr, 6) &&
        (_inputStreamSkipWhitespace(&parser->input, NULL) != 0) &&
        _inputStreamPeekCharacter(&parser->input, &ch);

    entityData.replacementText = NULL;
    entityData.entityID.publicID = NULL;
    entityData.entityID.systemID = NULL;
    entityData.notationName = NULL;
    // We will set entityType immediately before reporting

    if (success && ch == '%') {
        _inputStreamGetCharacter(&parser->input, &ch);
        success = _inputStreamSkipWhitespace(&parser->input, NULL) != 0;
        isPEDecl = true;
    }
    success = success && _inputStreamScanXMLName(&parser->input, false, report ? &name : NULL) && (_inputStreamSkipWhitespace(&parser->input, NULL) != 0) && _inputStreamPeekCharacter(&parser->input, &ch);
    if (success && (ch == '\"' || ch == '\'')) {
        // EntityValue
        // This is not quite correct - the string scanned cannot contain '%' or '&' unless it's as part of a valid entity reference -- REW, 2/2/2000
        if (report) {
            entityData.replacementText = CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser));
            success = _inputStreamScanQuotedString(&parser->input, (CFMutableStringRef)entityData.replacementText);
        } else {
            success = _inputStreamScanQuotedString(&parser->input, NULL);
        }
    } else if (success) {
        // ExternalID
        success = parseExternalID(parser, false, report ? &(entityData.entityID) : NULL);
        if (success && !isPEDecl && _inputStreamSkipWhitespace(&parser->input, NULL) != 0) {
            // There could be an option NDataDecl
            // Don't we need to set entityData.notationName? -- REW, 3/6/2000
            const UniChar nDataStr[5] = {'N', 'D', 'A', 'T', 'A'};
            if (_inputStreamMatchString(&parser->input, nDataStr, 5)) {
                success = (_inputStreamSkipWhitespace(&parser->input, NULL) != 0) && _inputStreamScanXMLName(&parser->input, false, NULL);
            }
        }
    }
    if (success) {
        _inputStreamSkipWhitespace(&parser->input, NULL);
        success = _inputStreamGetCharacter(&parser->input, &ch) && ch == '>';
    }
    if (!success) {
        if (_inputStreamAtEOF(&parser->input)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
        } else {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
        }
    } else {
        CFStringRef tmp = parser->node->dataString;
        if (isPEDecl) entityData.entityType = kCFXMLEntityTypeParameter;
        else if (entityData.replacementText) entityData.entityType = kCFXMLEntityTypeParsedInternal;
        else if (!entityData.notationName) entityData.entityType = kCFXMLEntityTypeParsedExternal;
        else entityData.entityType = kCFXMLEntityTypeUnparsed;
        parser->node->dataTypeID = kCFXMLNodeTypeEntity;
        parser->node->dataString = name;
        parser->node->additionalData = &entityData;
        success = reportNewLeaf(parser);
        parser->node->additionalData = NULL;
        parser->node->dataString = tmp;
        if (entityData.replacementText) CFRelease(entityData.replacementText);
    }
    if (entityData.entityID.publicID) CFRelease(entityData.entityID.publicID);
    if (entityData.entityID.systemID) CFRelease(entityData.entityID.systemID);
    return success;
}

/*
 [28] doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | PEReference | S)* ']' S?)? '>'
 [29] markupdecl ::= elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment 
*/ 
// First character should be just past '['
static Boolean parseInlineDTD(CFXMLParserRef parser) {
    Boolean success = true;
    while (success && !_inputStreamAtEOF(&parser->input)) {
        UniChar ch;

        parseWhitespace(parser);
        if (!_inputStreamGetCharacter(&parser->input, &ch)) break;
        if (ch == '%') {
            // PEReference
            success = parsePhysicalEntityReference(parser);
        } else if (ch == '<') {
            // markupdecl
            if (!_inputStreamGetCharacter(&parser->input, &ch)) {
                _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
                return false;
            }
            if (ch == '?') {
                // Processing Instruction
                success = parseProcessingInstruction(parser, true); // We can safely pass true here, because *parser->top will be NULL if kCFXMLParserSkipMetaData is true
            } else if (ch == '!') {
                UniChar dashes[2] = {'-', '-'};
                if (_inputStreamMatchString(&parser->input, dashes, 2)) {
                    // Comment
                    success = parseComment(parser, true);
                } else {
                    // elementdecl | AttListDecl | EntityDecl | NotationDecl
                    if (!_inputStreamPeekCharacter(&parser->input, &ch)) {
                        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
                        return false;
                    } else if (ch == 'A') {
                        // AttListDecl
                        success = parseAttributeListDeclaration(parser);
                    } else if (ch == 'N') {
                        success = parseNotationDeclaration(parser);
                    } else if (ch == 'E') {
                        // elementdecl | EntityDecl
                        _inputStreamGetCharacter(&parser->input, &ch);
                        if (!_inputStreamPeekCharacter(&parser->input, &ch)) {
                            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
                            return false;
                        }
                        _inputStreamReturnCharacter(&parser->input, 'E');
                        if (ch == 'L') {
                            success = parseElementDeclaration(parser);
                        } else if (ch == 'N') {
                            success = parseEntityDeclaration(parser);
                        } else {
                            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
                            return false;
                        }
                    } else {
                        _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
                        return false;
                    }                        
                }
            } else {
                _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
                return false;
            }
        } else if (ch == ']') {
            return true;
        } else {
            _CFReportError(parser, kCFXMLErrorMalformedDTD, "Found unexpected character while parsing inline DTD");
            return false;
        }
    }
    if (success) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Found unexpected EOF while parsing inline DTD");
    }
    return false;
}

/*
[43] content ::= (element | CharData | Reference | CDSect | PI | Comment)*
 */
static Boolean parseTagContent(CFXMLParserRef parser) {
    while (!_inputStreamAtEOF(&parser->input)) {
        UniChar ch;
        CFIndex numWhitespaceCharacters;
        
        _inputStreamSetMark(&parser->input);
        numWhitespaceCharacters = _inputStreamSkipWhitespace(&parser->input, NULL);
        // Don't report the whitespace yet; if the first thing we see is character data, we put the whitespace back and report it as part of the character data.
        if (!_inputStreamGetCharacter(&parser->input, &ch)) break;  // break == report unexpected EOF

        if (ch != '<' && ch != '&') { // CharData
            // Back off the whitespace; we'll report it with the PCData
            _inputStreamBackUpToMark(&parser->input);
            _inputStreamClearMark(&parser->input);
             if (!parsePCData(parser)) return false;
             if(_inputStreamComposingErrorOccurred(&parser->input)) {
                 _CFReportError(parser, kCFXMLErrorEncodingConversionFailure, "Encountered string encoding error");
                 return false;
             }
             continue;
        }

        // element | Reference | CDSect | PI | Comment
        // We can safely report any whitespace now
        if (!(parser->options & kCFXMLParserSkipWhitespace) && numWhitespaceCharacters != 0 && *(parser->top)) {
            _inputStreamReturnCharacter(&parser->input, ch);
            _inputStreamGetCharactersFromMark(&parser->input, (CFMutableStringRef)(parser->node->dataString));
            parser->node->dataTypeID = kCFXMLNodeTypeWhitespace;
            parser->node->additionalData = NULL;
            if (!reportNewLeaf(parser)) return false;
            _inputStreamGetCharacter(&parser->input, &ch);
        }
        _inputStreamClearMark(&parser->input);
        
        if (ch == '&') {
            // Reference; for the time being, we don't worry about processing these; just report them as Entity references
            if (!parseEntityReference(parser, true)) return false;
            continue;
        }

        // ch == '<'; element | CDSect | PI | Comment
        if (!_inputStreamPeekCharacter(&parser->input, &ch)) break;
        if (ch == '?') { // PI
            _inputStreamGetCharacter(&parser->input, &ch);
            if (!parseProcessingInstruction(parser, true))
                return false;
        } else if (ch == '/') { // end tag; we're passing outside of content's production
            _inputStreamReturnCharacter(&parser->input, '<'); // Back off to the '<'
            return true;
        } else if (ch != '!') { // element
            if (!parseTag(parser))  return false;
        } else {
            // Comment | CDSect
            UniChar dashes[3] = {'!', '-', '-'};
            if (_inputStreamMatchString(&parser->input, dashes, 3)) {
                // Comment
                if (!parseComment(parser, true)) return false;
            } else {
                // Should have a CDSect; back off the "<!" and call parseCDSect
                _inputStreamReturnCharacter(&parser->input, '<');
                if (!parseCDSect(parser)) return false;
            }
        }
    }

    if(_inputStreamComposingErrorOccurred(&parser->input)) {
        _CFReportError(parser, kCFXMLErrorEncodingConversionFailure, "Encountered string encoding error");
        return false;
    }
    // Only way to get here is if premature EOF was found
//#warning CF:Include the tag name here
    _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing tag content");
    return false;
}

static Boolean parseCDSect(CFXMLParserRef parser) {
    const UniChar _CDSectOpening[9] = {'<', '!', '[', 'C', 'D', 'A', 'T', 'A', '['};
    const UniChar _CDSectClose[3] = {']', ']', '>'};
    if (!_inputStreamMatchString(&parser->input, _CDSectOpening, 9)) {
        _CFReportError(parser, kCFXMLErrorMalformedCDSect, "Encountered bad prefix to a presumed CDATA section");
        return false;
    }
    if (!_inputStreamScanToCharacters(&parser->input, _CDSectClose, 3, (CFMutableStringRef)(parser->node->dataString))) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing CDATA section");
        return false;
    }

    parser->node->dataTypeID = kCFXMLNodeTypeCDATASection;
    parser->node->additionalData = NULL;
    return reportNewLeaf(parser);
}

/*
 [66] CharRef ::= '&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';'
*/
static Boolean validateCharacterReference(CFStringRef str) {
    Boolean isHex;
    CFIndex idx, len = CFStringGetLength(str);
    if (len < 2) return false;
    if (CFStringGetCharacterAtIndex(str, 0) != '#') return false;
    if (CFStringGetCharacterAtIndex(str, 1) == 'x') {
        isHex = true;
        idx = 2;
        if (len == 2) return false;
    } else {
        isHex = false;
        idx = 1;
    }

    while (idx < len) {
        UniChar ch;
        ch = CFStringGetCharacterAtIndex(str, idx);
        idx ++;
        if (!(ch <= '9' && ch >= '0') &&
            !(isHex && ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))) {
            break;
        }
    }
    return (idx == len);
}

/*
 [67] Reference ::= EntityRef | CharRef
 [68] EntityRef ::= '&' Name ';'
*/
static Boolean parseEntityReference(CFXMLParserRef parser, Boolean report) {
    UniChar ch;
    CFXMLEntityReferenceInfo entData;
    CFStringRef name = NULL;
    if (!_inputStreamPeekCharacter(&parser->input, &ch)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing EntityReference");
        return false;
    }
    if (ch == '#') {
        ch = ';';
        if (!_inputStreamScanToCharacters(&parser->input, &ch, 1, (CFMutableStringRef)parser->node->dataString)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing EntityReference");
            return false;
        } else if (!validateCharacterReference(parser->node->dataString)) {
            _CFReportError(parser, kCFXMLErrorMalformedCharacterReference, "Encountered illegal character while parsing character reference");
            return false;
        }
        entData.entityType = kCFXMLEntityTypeCharacter;
        name = parser->node->dataString;
    } else if (!_inputStreamScanXMLName(&parser->input, false, report ? &name : NULL) || !_inputStreamGetCharacter(&parser->input, &ch) || ch != ';') {
        if (_inputStreamAtEOF(&parser->input)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing EntityReference");
            return false;
        } else {
            _CFReportError(parser, kCFXMLErrorMalformedName, "Encountered malformed name while parsing EntityReference");
            return false;
        }
    } else {
        entData.entityType = kCFXMLEntityTypeParsedInternal;
    }
    if (report) {
        CFStringRef tmp = parser->node->dataString;
        Boolean success;
        parser->node->dataTypeID = kCFXMLNodeTypeEntityReference;
        parser->node->dataString = name;
        parser->node->additionalData = &entData;
        success = reportNewLeaf(parser);
        parser->node->additionalData = NULL;
        parser->node->dataString = tmp;
        return success;
    } else {
        return true;
    }
}

#if 0
// Kept from old entity reference parsing....
{
    switch (*(parser->curr)) {
        case 'l':  // "lt"
            if (len >= 3 && *(parser->curr+1) == 't' && *(parser->curr+2) == ';') {
                ch = '<';
                parser->curr += 3;
                break;
            }
            parser->errorString = CFStringCreateWithFormat(parser->allocator, NULL, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(parser));
            return;
        case 'g': // "gt"
            if (len >= 3 && *(parser->curr+1) == 't' && *(parser->curr+2) == ';') {
                ch = '>';
                parser->curr += 3;
                break;
            }
            parser->errorString = CFStringCreateWithFormat(parser->allocator, NULL, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(parser));
            return;
        case 'a': // "apos" or "amp"
            if (len < 4) {   // Not enough characters for either conversion
                parser->errorString = CFStringCreateWithCString(parser->allocator, "Encountered unexpected EOF", kCFStringEncodingASCII);
                return;
            }
            if (*(parser->curr+1) == 'm') {
                // "amp"
                if (*(parser->curr+2) == 'p' && *(parser->curr+3) == ';') {
                    ch = '&';
                    parser->curr += 4;
                    break;
                }
            } else if (*(parser->curr+1) == 'p') {
                // "apos"
                if (len > 4 && *(parser->curr+2) == 'o' && *(parser->curr+3) == 's' && *(parser->curr+4) == ';') {
                    ch = '\'';
                    parser->curr += 5;
                    break;
                }
            }
            parser->errorString = CFStringCreateWithFormat(parser->allocator, NULL, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(parser));
            return;
        case 'q':  // "quote"
            if (len >= 6 && *(parser->curr+1) == 'u' && *(parser->curr+2) == 'o' && *(parser->curr+3) == 't' && *(parser->curr+4) == 'e' && *(parser->curr+5) == ';') {
                ch = '\"';
                parser->curr += 6;
                break;
            }
            parser->errorString = CFStringCreateWithFormat(parser->allocator, NULL, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(parser));
            return;
        case '#':
        {
            UniChar num = 0;
            Boolean isHex = false;
            if ( len < 4) {  // Not enough characters to make it all fit!  Need at least "&#d;"
                parser->errorString = CFStringCreateWithCString(parser->allocator, "Encountered unexpected EOF", kCFStringEncodingASCII);
                return;
            }
            parser->curr ++;
            if (*(parser->curr) == 'x') {
                isHex = true;
                parser->curr ++;
            }
            while (parser->curr < parser->end) {
                ch = *(parser->curr);
                if (ch == ';') {
                    CFStringAppendCharacters(string, &num, 1);
                    parser->curr ++;
                    return;
                }
                if (!isHex) num = num*10;
                else num = num << 4;
                if (ch <= '9' && ch >= '0') {
                    num += (ch - '0');
                } else if (!isHex) {
                    parser->errorString = CFStringCreateWithFormat(parser->allocator, NULL, CFSTR("Encountered unexpected character %c at line %d"), ch, lineNumber(parser));
                    return;
                } else if (ch >= 'a' && ch <= 'f') {
                    num += 10 + (ch - 'a');
                } else if (ch >= 'A' && ch <= 'F') {
                    num += 10 + (ch - 'A');
                } else {
                    parser->errorString = CFStringCreateWithFormat(parser->allocator, NULL, CFSTR("Encountered unexpected character %c at line %d"), ch, lineNumber(parser));
                    return;                    
                }
            }
            parser->errorString = CFStringCreateWithCString(parser->allocator, "Encountered unexpected EOF", kCFStringEncodingASCII);
            return;
        }
        default:
            parser->errorString = CFStringCreateWithFormat(parser->allocator, NULL, CFSTR("Encountered unknown ampersand-escape sequence at line %d"), lineNumber(parser));
            return;
    }
    CFStringAppendCharacters(string, &ch, 1);
}
#endif

/*
[14] CharData ::= [^<&]* - ([^<&]* ']]>' [^<&]*)
*/
static Boolean parsePCData(CFXMLParserRef parser) {
    UniChar ch;
    Boolean done = false;
    _inputStreamSetMark(&parser->input);
    while (!done && _inputStreamGetCharacter(&parser->input, &ch)) {
        switch (ch) {
            case '<': 
            case '&':
                _inputStreamReturnCharacter(&parser->input, ch);
                done = true;
                break;
            case ']':
            {
                const UniChar endSequence[2] = {']', '>'};
                if (_inputStreamMatchString(&parser->input, endSequence, 2)) {
                    _CFReportError(parser, kCFXMLErrorMalformedParsedCharacterData, "Encountered \"]]>\" in parsed character data");
                    _inputStreamClearMark(&parser->input);
                    return false;
                }
                break;
            }
            default:
                ;
        }
    }
    _inputStreamGetCharactersFromMark(&parser->input, (CFMutableStringRef)(parser->node->dataString));
    _inputStreamClearMark(&parser->input);
    parser->node->dataTypeID = kCFXMLNodeTypeText;
    parser->node->additionalData = NULL;
    return reportNewLeaf(parser);
}

/*
[42] ETag ::= '</' Name S? '>'
 */
static Boolean parseCloseTag(CFXMLParserRef parser, CFStringRef tag) {
    const UniChar beginEndTag[2] = {'<', '/'};
    Boolean unexpectedEOF = false, mismatch = false;
    CFStringRef closeTag;

    // We can get away with testing pointer equality between tag & closeTag because scanXMLName guarantees the strings it returns are unique.
    if (_inputStreamMatchString(&parser->input, beginEndTag, 2) && _inputStreamScanXMLName(&parser->input, false, &closeTag) && closeTag == tag) {
        
        UniChar ch;
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamGetCharacter(&parser->input, &ch)) {
            unexpectedEOF = true;
        } else if (ch != '>') {
            mismatch = true;
        }
    } else if (_inputStreamAtEOF(&parser->input)) {
        unexpectedEOF = true;
    } else {
        mismatch = true;
    }
        
    if (unexpectedEOF || mismatch) {
        if (unexpectedEOF) {
            parser->errorString = CFStringCreateWithFormat(CFGetAllocator(parser), NULL, CFSTR("Encountered unexpected EOF while parsing close tag for <%@>"), tag);
            parser->status = kCFXMLErrorUnexpectedEOF;
            if(parser->callBacks.handleError) INVOKE_CALLBACK3(parser->callBacks.handleError, parser, kCFXMLErrorUnexpectedEOF, parser->context.info);
        } else {
            parser->errorString = CFStringCreateWithFormat(CFGetAllocator(parser), NULL, CFSTR("Encountered malformed close tag for <%@>"), tag);
            parser->status = kCFXMLErrorMalformedCloseTag;
            if(parser->callBacks.handleError) INVOKE_CALLBACK3(parser->callBacks.handleError, parser, kCFXMLErrorMalformedCloseTag, parser->context.info);
        }
        return false;
    }
    return true;
}

/*
 [39] element ::= EmptyElementTag | STag content ETag
 [40] STag ::= '<' Name (S Attribute)* S? '>'
 [44] EmptyElemTag ::= '<' Name (S Attribute)* S? '/>'
*/
static Boolean parseTag(CFXMLParserRef parser) {
    UniChar ch;
    void *tag;
    CFXMLElementInfo data;
    Boolean success = true;
    CFStringRef tagName;

    if (!_inputStreamScanXMLName(&parser->input, false, &tagName)) {
        _CFReportError(parser, kCFXMLErrorMalformedStartTag, "Encountered malformed start tag");
        return false;
    }

    _inputStreamSkipWhitespace(&parser->input, NULL);
    
    if (!parseAttributes(parser)) return false; // parsed directly into parser->argDict ; parseAttributes consumes any trailing whitespace
    data.attributes = parser->argDict;
    data.attributeOrder = parser->argArray;
    if (!_inputStreamGetCharacter(&parser->input, &ch)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF");
        return false;
    }
    if (ch == '/') {
        data.isEmpty = true;
        if (!_inputStreamGetCharacter(&parser->input, &ch)) {
            _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF");
            return false;
        }
    } else {
        data.isEmpty = false;
    }
    if (ch != '>') {
        _CFReportError(parser, kCFXMLErrorMalformedStartTag, "Encountered malformed start tag");
        return false;
    }

    if (*parser->top || parser->top == parser->stack) {
        CFStringRef oldStr = parser->node->dataString;
	parser->node->dataTypeID = kCFXMLNodeTypeElement;
        parser->node->dataString = tagName;
	parser->node->additionalData = &data;
        tag = (void *)INVOKE_CALLBACK3(parser->callBacks.createXMLStructure, parser, parser->node, parser->context.info);
        if (tag && parser->status == kCFXMLStatusParseInProgress) {
            INVOKE_CALLBACK4(parser->callBacks.addChild, parser, *parser->top, tag, parser->context.info);
        }
	parser->node->additionalData = NULL;
        parser->node->dataString = oldStr;
        if (parser->status != kCFXMLStatusParseInProgress) {
            // callback called CFXMLParserAbort()
            _CFReportError(parser, parser->status, NULL);
            return false;
        }
    } else {
        tag = NULL;
    }

    pushXMLNode(parser, tag);
    if (!data.isEmpty) {
        success =  parseTagContent(parser);
        if (success) {
            success = parseCloseTag(parser, tagName);
        }
    }
    parser->top --;

    if (success && tag) {
        INVOKE_CALLBACK3(parser->callBacks.endXMLStructure, parser, tag, parser->context.info);
        if (parser->status != kCFXMLStatusParseInProgress) {
            _CFReportError(parser, parser->status, NULL);
            return false;
        }
    }
    return success;
}

/*
 [10] AttValue ::= '"' ([^<&"] | Reference)* '"' |  "'" ([^<&'] | Reference)* "'"
 [67] Reference ::= EntityRef | CharRef
 [68] EntityRef ::= '&' Name ';'
 */
// For the moment, we don't worry about references in the attribute values.
static Boolean parseAttributeValue(CFXMLParserRef parser, CFMutableStringRef str) {
    UniChar quote, ch;
    Boolean success = _inputStreamGetCharacter(&parser->input, &quote);
    if (!success || (quote != '\'' && quote != '\"')) return false;
    if (str) _inputStreamSetMark(&parser->input);
    while (_inputStreamGetCharacter(&parser->input, &ch) && ch != quote) {
        switch (ch) {
            case '<': success = false; break;
            case '&':
                if (!parseEntityReference(parser, false)) {
                    success = false;
                    break;
                }
            default:
                ;
        }
    }
    
    if (success && _inputStreamAtEOF(&parser->input)) {
        success = false;
    }
    if (str) {
        if (success) {
            _inputStreamReturnCharacter(&parser->input, quote);
            _inputStreamGetCharactersFromMark(&parser->input, str);
            _inputStreamGetCharacter(&parser->input, &ch);
        }
        _inputStreamClearMark(&parser->input);
    }
    return success;
}

/*
 [40] STag ::= '<' Name (S Attribute)* S? '>'
 [41] Attribute ::= Name Eq AttValue
 [25] Eq ::= S? '=' S?
*/

// Expects parser->curr to be at the first content character; will consume the trailing whitespace.  
Boolean parseAttributes(CFXMLParserRef parser) {
    UniChar ch;
    CFMutableDictionaryRef dict;
    CFMutableArrayRef array;
    Boolean failure = false;
    if (_inputStreamPeekCharacter(&parser->input, &ch) == '>') {
        if (parser->argDict) {
            CFDictionaryRemoveAllValues(parser->argDict);
            CFArrayRemoveAllValues(parser->argArray);
        }
        return true;  // No attributes; let caller deal with it
    }
    if (!parser->argDict) {
        parser->argDict = CFDictionaryCreateMutable(CFGetAllocator(parser), 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        parser->argArray = CFArrayCreateMutable(CFGetAllocator(parser), 0, &kCFTypeArrayCallBacks);
    } else {
        CFDictionaryRemoveAllValues(parser->argDict);
        CFArrayRemoveAllValues(parser->argArray);
    }
    dict = parser->argDict;
    array = parser->argArray; 
    while (!failure && _inputStreamPeekCharacter(&parser->input, &ch) && ch != '>' && ch != '/') {
        CFStringRef key;
        CFMutableStringRef value;
        if (!_inputStreamScanXMLName(&parser->input, false, &key)) {
            failure = true;
            break;
        }
        if (CFArrayGetFirstIndexOfValue(array, CFRangeMake(0, CFArrayGetCount(array)), key) != kCFNotFound) {
                _CFReportError(parser, kCFXMLErrorMalformedStartTag, "Found repeated attribute");
                return false;
        }
        _inputStreamSkipWhitespace(&parser->input, NULL);
        if (!_inputStreamGetCharacter(&parser->input, &ch) || ch != '=') { 
            failure = true;
            break;
        }
        _inputStreamSkipWhitespace(&parser->input, NULL);
        value = CFStringCreateMutableWithExternalCharactersNoCopy(CFGetAllocator(parser), NULL, 0, 0, CFGetAllocator(parser));
        if (!parseAttributeValue(parser, value)) {
            CFRelease(value);
            failure = true;
            break;
        }
        CFArrayAppendValue(array, key);
        CFDictionarySetValue(dict, key, value);
        CFRelease(value);
        _inputStreamSkipWhitespace(&parser->input, NULL);
    }
    if (failure) {
//#warning CF:Include tag name in this error report
        _CFReportError(parser, kCFXMLErrorMalformedStartTag, "Found illegal character while parsing element tag");
        return false;
    } else if (_inputStreamAtEOF(&parser->input)) {
        _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing element attributes");
        return false;
    } else {
        return true;
    }
}

/*
 [1]  document ::= prolog element Misc*
 [22] prolog ::= XMLDecl? Misc* (doctypedecl Misc*)?
 [27] Misc ::= Comment | PI | S
 [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>' 

 We treat XMLDecl as a plain old PI, since PI is part of Misc.  This changes the prolog and document productions to
 [22-1] prolog ::= Misc* (doctypedecl Misc*)?
 [1-1] document ::= Misc* (doctypedecl Misc*)? element Misc*

 NOTE: This function assumes parser->stack has a valid top.  I.e. the document pointer has already been created!
*/
static Boolean parseXML(CFXMLParserRef parser) {
    Boolean success = true, sawDTD = false, sawElement = false;
    UniChar ch;
    while (success && _inputStreamPeekCharacter(&parser->input, &ch)) {
        switch (ch) {
            case ' ':
            case '\n':
            case '\t':
            case '\r':
                success = parseWhitespace(parser);
                break;
            case '<':
                _inputStreamGetCharacter(&parser->input, &ch);
                if (!_inputStreamGetCharacter(&parser->input, &ch)) {
                    _CFReportError(parser, kCFXMLErrorUnexpectedEOF, "Encountered unexpected EOF while parsing top-level document");
                    return false;
                }
                if (ch == '!') {
                    // Comment or DTD
                    UniChar dashes[2] = {'-', '-'};
                    if (_inputStreamMatchString(&parser->input, dashes, 2)) {
                        // Comment
                        success = parseComment(parser, true);
                    } else {
                        // Should be DTD
                        if (sawDTD) {
                            _CFReportError(parser, kCFXMLErrorMalformedDocument, "Encountered a second DTD");
                            return false;
                        }
                        success = parseDTD(parser);
                        if (success) sawDTD = true;
                    }
                } else if (ch == '?') {
                    // Processing instruction
                    success = parseProcessingInstruction(parser, true);
                } else {
                    // Tag or malformed
                    if (sawElement) {
                        _CFReportError(parser, kCFXMLErrorMalformedDocument, "Encountered second top-level element");
                        return false;
                    }
                    _inputStreamReturnCharacter(&parser->input, ch);
                    success = parseTag(parser);
                    if (success) sawElement = true;
                }
                break;
            default: {
                parser->status = kCFXMLErrorMalformedDocument;
                parser->errorString = ch < 256 ?
                    CFStringCreateWithFormat(CFGetAllocator(parser), NULL, CFSTR("Encountered unexpected character 0x%x (\'%c\') at top-level"), ch, ch) :
                    CFStringCreateWithFormat(CFGetAllocator(parser), NULL, CFSTR("Encountered unexpected Unicode character 0x%x at top-level"), ch);

                if (parser->callBacks.handleError) {
                    INVOKE_CALLBACK3(parser->callBacks.handleError, parser, parser->status, parser->context.info);
                }
                return false;
            }
        }
    }
    
    if (!success) return false;
    if (!sawElement) {
        _CFReportError(parser, kCFXMLErrorElementlessDocument, "No element found in document");
        return false;
    }
    return true;
}

static void _CFReportError(CFXMLParserRef parser, CFXMLParserStatusCode errNum, const char *str) {
    if (str) {
        parser->status = errNum;
        parser->errorString = CFStringCreateWithCString(CFGetAllocator(parser), str, kCFStringEncodingASCII);
    }
    if (parser->callBacks.handleError) {
        INVOKE_CALLBACK3(parser->callBacks.handleError, parser, errNum, parser->context.info);
    }
}

// Assumes parser->node has been set and is ready to go
static Boolean reportNewLeaf(CFXMLParserRef parser) {
    void *xmlStruct;
    if (*(parser->top) == NULL) return true;

    xmlStruct = (void *)INVOKE_CALLBACK3(parser->callBacks.createXMLStructure, parser, parser->node, parser->context.info);
    if (xmlStruct && parser->status == kCFXMLStatusParseInProgress) {
        INVOKE_CALLBACK4(parser->callBacks.addChild, parser, *(parser->top), xmlStruct, parser->context.info);
        if (parser->status == kCFXMLStatusParseInProgress) INVOKE_CALLBACK3(parser->callBacks.endXMLStructure, parser, xmlStruct, parser->context.info);
    }
    if (parser->status != kCFXMLStatusParseInProgress) {
        _CFReportError(parser, parser->status, NULL);
        return false;
    }
    return true;
}

static void pushXMLNode(CFXMLParserRef parser, void *node) {
    parser->top ++;
    if ((unsigned)(parser->top - parser->stack) == parser->capacity) {
        parser->stack = (void **)CFAllocatorReallocate(CFGetAllocator(parser), parser->stack, 2 * parser->capacity * sizeof(void *), 0);
        parser->top = parser->stack + parser->capacity;
        parser->capacity = 2*parser->capacity;
    }
    *(parser->top) = node;
}

/**************************/
/* Parsing to a CFXMLTree */
/**************************/

static void *_XMLTreeCreateXMLStructure(CFXMLParserRef parser, CFXMLNodeRef node, void *context) {
    CFXMLNodeRef myNode = CFXMLNodeCreateCopy(CFGetAllocator(parser), node);
    CFXMLTreeRef tree = CFXMLTreeCreateWithNode(CFGetAllocator(parser), myNode);
    CFRelease(myNode);
    return (void *)tree;
}

static void _XMLTreeAddChild(CFXMLParserRef parser, void *parent, void *child, void *context) {
    CFTreeAppendChild((CFTreeRef)parent, (CFTreeRef)child);
}

static void _XMLTreeEndXMLStructure(CFXMLParserRef parser, void *xmlType, void *context) {
    CFXMLTreeRef node = (CFXMLTreeRef)xmlType;
    if (CFTreeGetParent(node))
        CFRelease((CFXMLTreeRef)xmlType);
}

CFXMLTreeRef CFXMLTreeCreateWithDataFromURL(CFAllocatorRef allocator, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex version) {
    CFXMLParserRef parser;
    CFXMLParserCallBacks callbacks;
    CFXMLTreeRef result;

    CFAssert1(dataSource == NULL || CFGetTypeID(dataSource) == CFURLGetTypeID(), __kCFLogAssertion, "%s(): dataSource is not a valid CFURL", __PRETTY_FUNCTION__);

    callbacks.createXMLStructure = _XMLTreeCreateXMLStructure;
    callbacks.addChild = _XMLTreeAddChild;
    callbacks.endXMLStructure = _XMLTreeEndXMLStructure;
    callbacks.resolveExternalEntity = NULL;
    callbacks.handleError = NULL;
    parser = CFXMLParserCreateWithDataFromURL(allocator, dataSource, parseOptions, version, &callbacks, NULL);

    if (CFXMLParserParse(parser)) {
        result = (CFXMLTreeRef)CFXMLParserGetDocument(parser);
    } else {
        result = (CFXMLTreeRef)CFXMLParserGetDocument(parser);
        if (result) CFRelease(result);
        result = NULL;
    }
    CFRelease(parser);
    return result;
}

CFXMLTreeRef CFXMLTreeCreateFromData(CFAllocatorRef allocator, CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex parserVersion) {
    return CFXMLTreeCreateFromDataWithError(allocator, xmlData, dataSource, parseOptions, parserVersion, NULL);
}

CONST_STRING_DECL(kCFXMLTreeErrorDescription, "kCFXMLTreeErrorDescription");
CONST_STRING_DECL(kCFXMLTreeErrorLineNumber, "kCFXMLTreeErrorLineNumber");
CONST_STRING_DECL(kCFXMLTreeErrorLocation, "kCFXMLTreeErrorLocation");
CONST_STRING_DECL(kCFXMLTreeErrorStatusCode, "kCFXMLTreeErrorStatusCode");

CFXMLTreeRef CFXMLTreeCreateFromDataWithError(CFAllocatorRef allocator, CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex parserVersion, CFDictionaryRef *errorDict) {
    CFXMLParserRef parser;
    CFXMLParserCallBacks callbacks;
    CFXMLTreeRef result;

    __CFGenericValidateType(xmlData, CFDataGetTypeID());
    CFAssert1(dataSource == NULL || CFGetTypeID(dataSource) == CFURLGetTypeID(), __kCFLogAssertion, "%s(): dataSource is not a valid CFURL", __PRETTY_FUNCTION__);

    callbacks.createXMLStructure = _XMLTreeCreateXMLStructure;
    callbacks.addChild = _XMLTreeAddChild;
    callbacks.endXMLStructure = _XMLTreeEndXMLStructure;
    callbacks.resolveExternalEntity = NULL;
    callbacks.handleError = NULL;
    parser = CFXMLParserCreate(allocator, xmlData, dataSource, parseOptions, parserVersion, &callbacks, NULL);

    if (CFXMLParserParse(parser)) {
        result = (CFXMLTreeRef)CFXMLParserGetDocument(parser);
    } else {
        if (errorDict) {	// collect the error dictionary
            *errorDict = CFDictionaryCreateMutable(allocator, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            if (*errorDict) {
                CFIndex rawnum;
                CFNumberRef cfnum;
                CFStringRef errstring;
                
                rawnum = CFXMLParserGetLocation(parser);
                cfnum = CFNumberCreate(allocator, kCFNumberSInt32Type, &rawnum);
                if(cfnum) {
                    CFDictionaryAddValue((CFMutableDictionaryRef)*errorDict, kCFXMLTreeErrorLocation, cfnum);
                    CFRelease(cfnum);                    
                }
                
                rawnum = CFXMLParserGetLineNumber(parser);
                cfnum = CFNumberCreate(allocator, kCFNumberSInt32Type, &rawnum);
                if(cfnum) {
                    CFDictionaryAddValue((CFMutableDictionaryRef)*errorDict, kCFXMLTreeErrorLineNumber, cfnum);
                    CFRelease(cfnum);                    
                }

                rawnum = CFXMLParserGetStatusCode(parser);
                cfnum = CFNumberCreate(allocator, kCFNumberSInt32Type, &rawnum);
                if(cfnum) {
                    CFDictionaryAddValue((CFMutableDictionaryRef)*errorDict, kCFXMLTreeErrorStatusCode, cfnum);
                    CFRelease(cfnum);                    
                }

                errstring = CFXMLParserCopyErrorDescription(parser);
                if(errstring) {
                    CFDictionaryAddValue((CFMutableDictionaryRef)*errorDict, kCFXMLTreeErrorDescription, errstring);
                    CFRelease(errstring);                    
                }
            }
        }
        result = (CFXMLTreeRef)CFXMLParserGetDocument(parser);
        if (result) CFRelease(result);
        result = NULL;
    }
    CFRelease(parser);
    return result;
}

/*
 At the very least we need to do <, >, &, ", and '. In addition, we'll have to do everything else in the string.
 We should also be handling items that are up over certain values correctly.
 */
CFStringRef CFXMLCreateStringByEscapingEntities(CFAllocatorRef allocator, CFStringRef string, CFDictionaryRef entitiesDictionary) {
    CFAssert1(string != NULL, __kCFLogAssertion, "%s(): NULL string not permitted.", __PRETTY_FUNCTION__);
    CFMutableStringRef newString = CFStringCreateMutable(allocator, 0); // unbounded mutable string
    CFMutableCharacterSetRef startChars = CFCharacterSetCreateMutable(allocator);

    CFStringInlineBuffer inlineBuf;
    CFIndex idx = 0;
    CFIndex mark = idx;
    CFIndex stringLength = CFStringGetLength(string);
    UniChar uc;

    CFCharacterSetAddCharactersInString(startChars, CFSTR("&<>'\""));

    CFStringInitInlineBuffer(string, &inlineBuf, CFRangeMake(0, stringLength));
    for(idx = 0; idx < stringLength; idx++) {
        uc = CFStringGetCharacterFromInlineBuffer(&inlineBuf, idx);
        if(CFCharacterSetIsCharacterMember(startChars, uc)) {
            CFStringRef previousSubstring = CFStringCreateWithSubstring(allocator, string, CFRangeMake(mark, idx - mark));
            CFStringAppend(newString, previousSubstring);
            CFRelease(previousSubstring);
            switch(uc) {
                case '&':
                    CFStringAppend(newString, CFSTR("&amp;"));
                    break;
                case '<':
                    CFStringAppend(newString, CFSTR("&lt;"));
                    break;
                case '>':
                    CFStringAppend(newString, CFSTR("&gt;"));
                    break;
                case '\'':
                    CFStringAppend(newString, CFSTR("&apos;"));
                    break;
                case '"':
                    CFStringAppend(newString, CFSTR("&quot;"));
                    break;
            }
            mark = idx + 1;
        }
    }
    // Copy the remainder to the output string before returning.
    CFStringRef remainder = CFStringCreateWithSubstring(allocator, string, CFRangeMake(mark, idx - mark));
    if (NULL != remainder) {
        CFStringAppend(newString, remainder);
        CFRelease(remainder);
    }
    
    CFRelease(startChars);
    return newString;
}

CFStringRef CFXMLCreateStringByUnescapingEntities(CFAllocatorRef allocator, CFStringRef string, CFDictionaryRef entitiesDictionary) {
    CFAssert1(string != NULL, __kCFLogAssertion, "%s(): NULL string not permitted.", __PRETTY_FUNCTION__);

    CFStringInlineBuffer inlineBuf; /* use this for fast traversal of the string in question */
    CFStringRef sub;
    CFIndex lastChunkStart, length = CFStringGetLength(string);
    CFIndex i, entityStart;
    UniChar uc;
    UInt32 entity;
    int base;
    CFMutableDictionaryRef fullReplDict = entitiesDictionary ? CFDictionaryCreateMutableCopy(allocator, 0, entitiesDictionary) : CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(fullReplDict, (const void *)CFSTR("amp"), (const void *)CFSTR("&"));
    CFDictionaryAddValue(fullReplDict, (const void *)CFSTR("quot"), (const void *)CFSTR("\""));
    CFDictionaryAddValue(fullReplDict, (const void *)CFSTR("lt"), (const void *)CFSTR("<"));
    CFDictionaryAddValue(fullReplDict, (const void *)CFSTR("gt"), (const void *)CFSTR(">"));
    CFDictionaryAddValue(fullReplDict, (const void *)CFSTR("apos"), (const void *)CFSTR("'"));

    CFStringInitInlineBuffer(string, &inlineBuf, CFRangeMake(0, length - 1));
    CFMutableStringRef newString = CFStringCreateMutable(allocator, 0);

    lastChunkStart = 0;
    // Scan through the string in its entirety
    for(i = 0; i < length; ) {
        uc = CFStringGetCharacterFromInlineBuffer(&inlineBuf, i); i++;	// grab the next character and move i.

        if(uc == '&') {
            entityStart = i - 1;
            entity = 0xFFFF;	// set this to a not-Unicode character as sentinel
                             // we've hit the beginning of an entity. Copy everything from lastChunkStart to this point.
            if(lastChunkStart < i - 1) {
                sub = CFStringCreateWithSubstring(allocator, string, CFRangeMake(lastChunkStart, (i - 1) - lastChunkStart));
                CFStringAppend(newString, sub);
                CFRelease(sub);
            }

            uc = CFStringGetCharacterFromInlineBuffer(&inlineBuf, i); i++;	// grab the next character and move i.
                                                                           // Now we can process the entity reference itself
            if(uc == '#') {	// this is a numeric entity.
                base = 10;
                entity = 0;
                uc = CFStringGetCharacterFromInlineBuffer(&inlineBuf, i); i++;

                if(uc == 'x') {	// only lowercase x allowed. Translating numeric entity as hexadecimal.
                    base = 16;
                    uc = CFStringGetCharacterFromInlineBuffer(&inlineBuf, i); i++;
                }

                // process the provided digits 'til we're finished
                while(true) {
                    if (uc >= '0' && uc <= '9')
                        entity = entity * base + (uc-'0');
                    else if (uc >= 'a' && uc <= 'f' && base == 16)
                        entity = entity * base + (uc-'a'+10);
                    else if (uc >= 'A' && uc <= 'F' && base == 16)
                        entity = entity * base + (uc-'A'+10);
                    else break;

                    if (i < length) {
                        uc = CFStringGetCharacterFromInlineBuffer(&inlineBuf, i); i++;
                    }
                    else
                        break;
                }
            }

            // Scan to the end of the entity
            while(uc != ';' && i < length) {
                uc = CFStringGetCharacterFromInlineBuffer(&inlineBuf, i); i++;
            }

            if(0xFFFF != entity) { // it was numeric, and translated.
                // Now, output the result fo the entity
                if(entity >= 0x10000) {
                    UniChar characters[2] = { ((entity - 0x10000) >> 10) + 0xD800, ((entity - 0x10000) & 0x3ff) + 0xDC00 };
                    CFStringAppendCharacters(newString, characters, 2);
                } else {
                    UniChar character = entity;
                    CFStringAppendCharacters(newString, &character, 1);
                }
            } else {	// it wasn't numeric.
                sub = CFStringCreateWithSubstring(allocator, string, CFRangeMake(entityStart + 1, (i - entityStart - 2))); // This trims off the & and ; from the string, so we can use it against the dictionary itself.
                CFStringRef replacementString = (CFStringRef)CFDictionaryGetValue(fullReplDict, sub);
                if(replacementString) {
                    CFStringAppend(newString, replacementString);
                } else {
                    CFRelease(sub); // let the old substring go, since we didn't find it in the dictionary
                    sub =  CFStringCreateWithSubstring(allocator, string, CFRangeMake(entityStart, (i - entityStart))); // create a new one, including the & and ;
                    CFStringAppend(newString, sub); // ...and append that.
                }
                CFRelease(sub); // in either case, release the most-recent "sub"
            }

            // move the lastChunkStart to the beginning of the next chunk.
            lastChunkStart = i;
        }
    }
    if(lastChunkStart < length) { // we've come out of the loop, let's get the rest of the string and tack it on.
        sub = CFStringCreateWithSubstring(allocator, string, CFRangeMake(lastChunkStart, i - lastChunkStart));
        CFStringAppend(newString, sub);
        CFRelease(sub);
    }

    CFRelease(fullReplDict);

    return newString;
}

#pragma GCC diagnostic pop
