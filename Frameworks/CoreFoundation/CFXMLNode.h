/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFURL.h>

typedef struct CFXMLNode *CFXMLNodeRef;

enum {
    kCFXMLNodeCurrentVersion = 1,
};

typedef enum {
    kCFXMLNodeTypeDocument = 1,
    kCFXMLNodeTypeElement = 2,
    kCFXMLNodeTypeAttribute = 3,
    kCFXMLNodeTypeProcessingInstruction = 4,
    kCFXMLNodeTypeComment = 5,
    kCFXMLNodeTypeText = 6,
    kCFXMLNodeTypeCDATASection = 7,
    kCFXMLNodeTypeDocumentFragment = 8,
    kCFXMLNodeTypeEntity = 9,
    kCFXMLNodeTypeEntityReference = 10,
    kCFXMLNodeTypeDocumentType = 11,
    kCFXMLNodeTypeWhitespace = 12,
    kCFXMLNodeTypeNotation = 13,
    kCFXMLNodeTypeElementTypeDeclaration = 14,
    kCFXMLNodeTypeAttributeListDeclaration = 15,
} CFXMLNodeTypeCode;

typedef enum {
    kCFXMLEntityTypeParameter = 0,
    kCFXMLEntityTypeParsedInternal = 1,
    kCFXMLEntityTypeParsedExternal = 2,
    kCFXMLEntityTypeUnparsed = 3,
    kCFXMLEntityTypeCharacter = 4,
} CFXMLEntityTypeCode;

typedef struct {
    CFURLRef sourceURL;
    CFStringEncoding encoding;
} CFXMLDocumentInfo;

typedef struct {
    CFDictionaryRef attributes;
    CFArrayRef attributeOrder;
    Boolean isEmpty;
} CFXMLElementInfo;

typedef struct {
    CFURLRef systemID;
    CFStringRef publicID;
} CFXMLExternalID;

typedef struct {
    CFXMLExternalID externalID;
} CFXMLNotationInfo;

typedef struct {
    CFStringRef dataString;
} CFXMLProcessingInstructionInfo;

typedef struct {
    CFStringRef attributeName;
    CFStringRef typeString;
    CFStringRef defaultString;
} CFXMLAttributeDeclarationInfo;

typedef struct {
    CFIndex numberOfAttributes;
    CFXMLAttributeDeclarationInfo *attributes;
} CFXMLAttributeListDeclarationInfo;

typedef struct {
    CFXMLExternalID externalID;
} CFXMLDocumentTypeInfo;

typedef struct {
    CFStringRef contentDescription;
} CFXMLElementTypeDeclarationInfo;

typedef struct {
    CFXMLEntityTypeCode entityType;
    CFStringRef replacementText;
    CFXMLExternalID entityID;
    CFStringRef notationName;
} CFXMLEntityInfo;

typedef struct {
    CFXMLEntityTypeCode entityType;
} CFXMLEntityReferenceInfo;

COREFOUNDATION_EXPORT CFTypeID CFXMLNodeGetTypeID(void);

COREFOUNDATION_EXPORT CFXMLNodeRef CFXMLNodeCreate(CFAllocatorRef allocator, CFXMLNodeTypeCode typeCode, CFStringRef string, const void *infoPtr, CFIndex version);

COREFOUNDATION_EXPORT CFXMLNodeRef CFXMLNodeCreateCopy(CFAllocatorRef allocator, CFXMLNodeRef self);

COREFOUNDATION_EXPORT CFXMLNodeTypeCode CFXMLNodeGetTypeCode(CFXMLNodeRef self);
COREFOUNDATION_EXPORT CFStringRef CFXMLNodeGetString(CFXMLNodeRef self);
COREFOUNDATION_EXPORT const void *CFXMLNodeGetInfoPtr(CFXMLNodeRef self);
COREFOUNDATION_EXPORT CFIndex CFXMLNodeGetVersion(CFXMLNodeRef self);
