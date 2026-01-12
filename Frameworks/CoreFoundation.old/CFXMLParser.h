/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFXMLNode.h>

typedef struct CFXMLParser *CFXMLParserRef;

typedef enum {
    kCFXMLStatusParseNotBegun = -2,
    kCFXMLStatusParseInProgress = -1,
    kCFXMLStatusParseSuccessful = 0,
    kCFXMLErrorUnexpectedEOF = 1,
    kCFXMLErrorUnknownEncoding = 2,
    kCFXMLErrorEncodingConversionFailure = 3,
    kCFXMLErrorMalformedProcessingInstruction = 4,
    kCFXMLErrorMalformedDTD = 5,
    kCFXMLErrorMalformedName = 6,
    kCFXMLErrorMalformedCDSect = 7,
    kCFXMLErrorMalformedCloseTag = 8,
    kCFXMLErrorMalformedStartTag = 9,
    kCFXMLErrorMalformedDocument = 10,
    kCFXMLErrorElementlessDocument = 11,
    kCFXMLErrorMalformedComment = 12,
    kCFXMLErrorMalformedCharacterReference = 13,
    kCFXMLErrorMalformedParsedCharacterData = 14,
    kCFXMLErrorNoData = 15,
} CFXMLParserStatusCode;

typedef enum {
    kCFXMLParserNoOptions = 0,
    kCFXMLParserValidateDocument = (1 << 0),
    kCFXMLParserSkipMetaData = (1 << 1),
    kCFXMLParserReplacePhysicalEntities = (1 << 2),
    kCFXMLParserSkipWhitespace = (1 << 3),
    kCFXMLParserResolveExternalEntities = (1 << 4),
    kCFXMLParserAddImpliedAttributes = (1 << 5),
    kCFXMLParserAllOptions = 0xFFFFFF,
} CFXMLParserOptions;

typedef void *(*CFXMLParserCreateXMLStructureCallBack)(CFXMLParserRef self, CFXMLNodeRef node, void *info);
typedef void (*CFXMLParserAddChildCallBack)(CFXMLParserRef self, void *parent, void *child, void *info);
typedef void (*CFXMLParserEndXMLStructureCallBack)(CFXMLParserRef self, void *type, void *info);
typedef CFDataRef (*CFXMLParserResolveExternalEntityCallBack)(CFXMLParserRef self, CFXMLExternalID *externalID, void *info);
typedef Boolean (*CFXMLParserHandleErrorCallBack)(CFXMLParserRef self, CFXMLParserStatusCode statusCode, void *info);

typedef struct {
    CFIndex version;
    CFXMLParserCreateXMLStructureCallBack createXMLStructure;
    CFXMLParserAddChildCallBack addChild;
    CFXMLParserEndXMLStructureCallBack endXMLStructure;
    CFXMLParserResolveExternalEntityCallBack resolveExternalEntity;
    CFXMLParserHandleErrorCallBack handleError;
} CFXMLParserCallBacks;

typedef CFAllocatorRetainCallBack CFXMLParserRetainCallBack;
typedef CFAllocatorReleaseCallBack CFXMLParserReleaseCallBack;
typedef CFAllocatorCopyDescriptionCallBack CFXMLParserCopyDescriptionCallBack;

typedef struct CFXMLParserContext {
    CFIndex version;
    void *info;
    CFXMLParserRetainCallBack retain;
    CFXMLParserReleaseCallBack release;
    CFXMLParserCopyDescriptionCallBack copyDescription;
} CFXMLParserContext;

COREFOUNDATION_EXPORT CFTypeID CFXMLParserGetTypeID();

COREFOUNDATION_EXPORT CFXMLParserRef CFXMLParserCreate(CFAllocatorRef allocator, CFDataRef data, CFURLRef url, CFOptionFlags options, CFIndex nodeVersion, CFXMLParserCallBacks *callbacks, CFXMLParserContext *context);
COREFOUNDATION_EXPORT CFXMLParserRef CFXMLParserCreateWithDataFromURL(CFAllocatorRef allocator, CFURLRef url, CFOptionFlags options, CFIndex nodeVersion, CFXMLParserCallBacks *callbacks, CFXMLParserContext *context);

COREFOUNDATION_EXPORT void CFXMLParserAbort(CFXMLParserRef self, CFXMLParserStatusCode statusCode, CFStringRef errorDescription);
COREFOUNDATION_EXPORT CFStringRef CFXMLParserCopyErrorDescription(CFXMLParserRef self);
COREFOUNDATION_EXPORT void CFXMLParserGetCallBacks(CFXMLParserRef self, CFXMLParserCallBacks *callbacks);
COREFOUNDATION_EXPORT void CFXMLParserGetContext(CFXMLParserRef self, CFXMLParserContext *context);
COREFOUNDATION_EXPORT void *CFXMLParserGetDocument(CFXMLParserRef self);
COREFOUNDATION_EXPORT CFIndex CFXMLParserGetLineNumber(CFXMLParserRef self);
COREFOUNDATION_EXPORT CFIndex CFXMLParserGetLocation(CFXMLParserRef self);
COREFOUNDATION_EXPORT CFURLRef CFXMLParserGetSourceURL(CFXMLParserRef self);
COREFOUNDATION_EXPORT CFXMLParserStatusCode CFXMLParserGetStatusCode(CFXMLParserRef self);

Boolean CFXMLParserParse(CFXMLParserRef self);
