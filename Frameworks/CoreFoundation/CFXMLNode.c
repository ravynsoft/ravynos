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

/*	CFXMLNode.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: David Smith
*/

#include <CoreFoundation/CFXMLNode.h>
#include <CoreFoundation/CFPropertyList.h>
#include "CFInternal.h"
#include "CFXMLInputStream.h"

CF_INLINE Boolean _nullSafeCFEqual(CFTypeRef cf1, CFTypeRef cf2) {
    if (cf1 && !cf2) return false;
    if (cf2 && !cf1) return false;
    if (cf1) return CFEqual(cf1, cf2);
    return true;
}

static Boolean externalIDEqual(CFXMLExternalID *ext1, CFXMLExternalID *ext2) {
    return _nullSafeCFEqual(ext1->systemID, ext2->systemID) && _nullSafeCFEqual(ext1->publicID, ext2->publicID);
}

static Boolean __CFXMLNodeEqual(CFTypeRef  cf1, CFTypeRef  cf2) {
    CFXMLNodeRef desc1 = (CFXMLNodeRef)cf1, desc2 = (CFXMLNodeRef)cf2;
    if (desc1 == desc2) return true;
    if (!desc1 || !desc2) return false;
    if (desc1->dataTypeID != desc2->dataTypeID) return false;
    if ((desc1->dataString && !desc2->dataString) || (!desc1->dataString && desc2->dataString)) return false;
    if (desc1->dataString && !CFEqual(desc1->dataString, desc2->dataString)) return false;
    if ((desc1->additionalData && !desc2->additionalData) || (!desc1->additionalData && desc2->additionalData)) return false;
    if (!desc1->additionalData) return true;
    switch (desc1->dataTypeID) {
        case kCFXMLNodeTypeDocument:{
            CFURLRef url1, url2;
            url1 = ((CFXMLDocumentInfo *)desc1->additionalData)->sourceURL;
            url2 = ((CFXMLDocumentInfo *)desc2->additionalData)->sourceURL;
            return _nullSafeCFEqual(url1, url2);
        }
        case kCFXMLNodeTypeElement: {
            CFXMLElementInfo *elt1, *elt2;
            elt1 = (CFXMLElementInfo *)desc1->additionalData;
            elt2 = (CFXMLElementInfo *)desc2->additionalData;
            if (elt1->isEmpty != elt2->isEmpty) return false;
            if (elt1->attributes == elt2->attributes) return true;
            if (!elt1->attributes) return (CFDictionaryGetCount(elt2->attributes) == 0);
            if (!elt2->attributes) return (CFDictionaryGetCount(elt1->attributes) == 0);
            return CFEqual(elt1->attributes, elt2->attributes);
        }
        case kCFXMLNodeTypeProcessingInstruction: {
            CFStringRef str1, str2;
            str1 = ((CFXMLProcessingInstructionInfo *)desc1->additionalData)->dataString;
            str2 = ((CFXMLProcessingInstructionInfo *)desc2->additionalData)->dataString;
            return _nullSafeCFEqual(str1, str2);
        }
        case kCFXMLNodeTypeEntity: {
            CFXMLEntityInfo *data1, *data2;
            data1 = (CFXMLEntityInfo *)desc1->additionalData;
            data2 = (CFXMLEntityInfo *)desc2->additionalData;
            if (data1->entityType != data2->entityType) return false;
            if (!_nullSafeCFEqual(data1->replacementText, data2->replacementText)) return false;
            if (!_nullSafeCFEqual(data1->notationName, data2->notationName)) return false;
            return externalIDEqual(&data1->entityID, &data2->entityID);
        }
        case kCFXMLNodeTypeEntityReference: {
            return ((CFXMLEntityReferenceInfo *)(desc1->additionalData))->entityType == ((CFXMLEntityReferenceInfo *)(desc2->additionalData))->entityType;
        }
        case kCFXMLNodeTypeNotation: {
            CFXMLNotationInfo *data1, *data2;
            data1 = (CFXMLNotationInfo *)desc1->additionalData;
            data2 = (CFXMLNotationInfo *)desc2->additionalData;
            return externalIDEqual(&(data1->externalID), &(data2->externalID));
        }
        case kCFXMLNodeTypeDocumentType: {
            CFXMLDocumentTypeInfo *data1, *data2;
            data1 = (CFXMLDocumentTypeInfo *)desc1->additionalData;
            data2 = (CFXMLDocumentTypeInfo *)desc2->additionalData;
            return externalIDEqual(&(data1->externalID), &(data2->externalID));
        }
        case kCFXMLNodeTypeElementTypeDeclaration: {
            CFXMLElementTypeDeclarationInfo *d1 = (CFXMLElementTypeDeclarationInfo *)desc1->additionalData;
            CFXMLElementTypeDeclarationInfo *d2 = (CFXMLElementTypeDeclarationInfo *)desc2->additionalData;
            return _nullSafeCFEqual(d1->contentDescription, d2->contentDescription);
        }
        case kCFXMLNodeTypeAttributeListDeclaration: {
            CFXMLAttributeListDeclarationInfo *attList1 = (CFXMLAttributeListDeclarationInfo *)desc1->additionalData;
            CFXMLAttributeListDeclarationInfo *attList2 = (CFXMLAttributeListDeclarationInfo *)desc2->additionalData;
            CFIndex idx;
            if (attList1->numberOfAttributes != attList2->numberOfAttributes) return false;
            for (idx = 0; idx < attList1->numberOfAttributes; idx ++) {
                CFXMLAttributeDeclarationInfo *attr1 = &(attList1->attributes[idx]);
                CFXMLAttributeDeclarationInfo *attr2 = &(attList2->attributes[idx]);
                if (!_nullSafeCFEqual(attr1->attributeName, attr2->attributeName)) return false;
                if (!_nullSafeCFEqual(attr1->typeString, attr2->typeString)) return false;
                if (!_nullSafeCFEqual(attr1->defaultString, attr2->defaultString)) return false;
            }
            return true;
        }
        default:
            return false;
    }
    return true;
}

static CFHashCode __CFXMLNodeHash(CFTypeRef  cf) {
    CFXMLNodeRef node = (CFXMLNodeRef)cf;
    if (node->dataString) {
        return CFHash(node->dataString);
    }
    if (node->dataTypeID == kCFXMLNodeTypeDocument) {
        CFURLRef url = ((CFXMLDocumentInfo *)node->additionalData)->sourceURL;
        return url ? CFHash(url) : (CFHashCode)cf;
    } else {
        CFAssert2(false, __kCFLogAssertion, "%s(): Saw unexpected XML type code %d", __PRETTY_FUNCTION__, node->dataTypeID);
        return CFHash(cf);
    }
}

static CFStringRef __CFXMLNodeCopyDescription(CFTypeRef  cf) {
    struct __CFXMLNode *node = (struct __CFXMLNode *)cf;
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("CFXMLNode %p>{typeID = %ld, string = %@}"), cf, (unsigned long)node->dataTypeID, node->dataString);
}

static void __CFXMLNodeDeallocate(CFTypeRef  cf) {
    struct __CFXMLNode *node = (struct __CFXMLNode *)cf;
    if (node->dataString) CFRelease(node->dataString);
    if (node->additionalData) {
        switch (node->dataTypeID) {
            case kCFXMLNodeTypeDocument:
                if (((CFXMLDocumentInfo *)node->additionalData)->sourceURL) {
                    CFRelease(((CFXMLDocumentInfo *)node->additionalData)->sourceURL);
                }
                break;
            case kCFXMLNodeTypeElement:
                if (((CFXMLElementInfo *)node->additionalData)->attributes) {
                    CFRelease(((CFXMLElementInfo *)node->additionalData)->attributes);
                    CFRelease(((CFXMLElementInfo *)node->additionalData)->attributeOrder);
                }
                break;
            case kCFXMLNodeTypeProcessingInstruction:
                if (((CFXMLProcessingInstructionInfo *)node->additionalData)->dataString) {
                    CFRelease(((CFXMLProcessingInstructionInfo *)node->additionalData)->dataString);
                }
                break;
            case kCFXMLNodeTypeEntity:
            {
                CFXMLEntityInfo *data = (CFXMLEntityInfo *)node->additionalData;
                if (data->replacementText) CFRelease(data->replacementText);
                if (data->entityID.systemID) CFRelease(data->entityID.systemID);
                if (data->entityID.publicID) CFRelease(data->entityID.publicID);
                if (data->notationName) CFRelease(data->notationName);
                break;
            }
            case kCFXMLNodeTypeEntityReference:
            {
                // Do nothing; additionalData has no structure of its own, with dependent pieces to release.  -- REW, 2/11/2000
                break;
            }
            case kCFXMLNodeTypeDocumentType:
            case kCFXMLNodeTypeNotation:
                // We get away with this because CFXMLNotationInfo and CFXMLDocumentTypeInfo have identical formats
            {
                CFXMLNotationInfo *data = (CFXMLNotationInfo *)node->additionalData;
                if (data->externalID.systemID) CFRelease(data->externalID.systemID);
                if (data->externalID.publicID) CFRelease(data->externalID.publicID);
                break;
            }
            case kCFXMLNodeTypeElementTypeDeclaration:
                if (((CFXMLElementTypeDeclarationInfo *)node->additionalData)->contentDescription) {
                    CFRelease(((CFXMLElementTypeDeclarationInfo *)node->additionalData)->contentDescription);
                }
                break;
            case kCFXMLNodeTypeAttributeListDeclaration:
            {
                CFXMLAttributeListDeclarationInfo *data = (CFXMLAttributeListDeclarationInfo *)node->additionalData;
                CFIndex idx;
                for (idx = 0; idx < data->numberOfAttributes; idx ++) {
                    CFRelease(data->attributes[idx].attributeName);
                    CFRelease(data->attributes[idx].typeString);
                    CFRelease(data->attributes[idx].defaultString);
                }
                CFAllocatorDeallocate(CFGetAllocator(node), data->attributes);
                break;
            }
            default:
                CFAssert1(false, __kCFLogAssertion, "%s(): Encountered unexpected typeID %d (additionalData should be empty)", node->dataTypeID);
        }
    }
}

static CFTypeID __kCFXMLNodeTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFXMLNodeClass = {
    0,
    "CFXMLNode",
    NULL,      // init
    NULL,      // copy
    __CFXMLNodeDeallocate,
    __CFXMLNodeEqual,
    __CFXMLNodeHash,
    NULL,      // 
    __CFXMLNodeCopyDescription
};

CFTypeID CFXMLNodeGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFXMLNodeTypeID = _CFRuntimeRegisterClass(&__CFXMLNodeClass); });
    return __kCFXMLNodeTypeID;
}

CFXMLNodeRef CFXMLNodeCreateCopy(CFAllocatorRef alloc, CFXMLNodeRef origNode) {
    return CFXMLNodeCreate(alloc, origNode->dataTypeID, origNode->dataString, origNode->additionalData, origNode->version);
}

static void _copyAddlDataForType(CFAllocatorRef alloc, CFXMLNodeTypeCode xmlType, const void *src, void *dest) {
    switch(xmlType) {
        case kCFXMLNodeTypeDocument: {
            CFXMLDocumentInfo *srcData = (CFXMLDocumentInfo *)src;
            CFXMLDocumentInfo *destData = (CFXMLDocumentInfo *)dest;
            destData->sourceURL = srcData->sourceURL ? (CFURLRef)CFRetain(srcData->sourceURL) : NULL;
            destData->encoding = srcData->encoding;
            break;
        }
        case kCFXMLNodeTypeElement: {
            CFXMLElementInfo *srcData = (CFXMLElementInfo *)src;
            CFXMLElementInfo *destData = (CFXMLElementInfo *)dest;
            if (srcData->attributes && CFDictionaryGetCount(srcData->attributes) != 0) {
                destData->attributes =  (CFDictionaryRef)CFPropertyListCreateDeepCopy(alloc, srcData->attributes, kCFPropertyListImmutable);
                destData->attributeOrder = (CFArrayRef)CFPropertyListCreateDeepCopy(alloc, srcData->attributeOrder, kCFPropertyListImmutable);
            } else {
                destData->attributes = NULL;
                destData->attributeOrder = NULL;
            }
            destData->isEmpty = srcData->isEmpty;
            break;
        }
        case kCFXMLNodeTypeProcessingInstruction: {
            CFXMLProcessingInstructionInfo *srcData = (CFXMLProcessingInstructionInfo *)src;
            CFXMLProcessingInstructionInfo *destData = (CFXMLProcessingInstructionInfo *)dest;
            destData->dataString = srcData->dataString ? (CFStringRef)CFStringCreateCopy(alloc, srcData->dataString) : NULL;
            break;
        }
        case kCFXMLNodeTypeEntity:
        {
            CFXMLEntityInfo *sourceData = (CFXMLEntityInfo *)src;
            CFXMLEntityInfo *destData = (CFXMLEntityInfo *)dest;
            destData->entityType = sourceData->entityType;
            destData->replacementText = sourceData->replacementText ? (CFStringRef)CFStringCreateCopy(alloc, sourceData->replacementText) : NULL;
            destData->entityID.systemID = sourceData->entityID.systemID ? (CFURLRef)CFRetain(sourceData->entityID.systemID) : NULL;
            destData->entityID.publicID = sourceData->entityID.publicID ? (CFStringRef)CFStringCreateCopy(alloc, sourceData->entityID.publicID) : NULL;
            destData->notationName = sourceData->notationName ? (CFStringRef)CFStringCreateCopy(alloc, sourceData->notationName) : NULL;
            break;
        }
        case kCFXMLNodeTypeEntityReference:
        {
            CFXMLEntityReferenceInfo *srcData = (CFXMLEntityReferenceInfo *)src;
            CFXMLEntityReferenceInfo *destData = (CFXMLEntityReferenceInfo *)dest;
            destData->entityType = srcData->entityType;
            break;
        }
        case kCFXMLNodeTypeDocumentType:
        case kCFXMLNodeTypeNotation:
        {
            // We can get away with this because the structures of CFXMLNotationInfo and CFXMLDocumentTypeInfo match.  -- REW, 3/8/2000
            CFXMLNotationInfo *srcData = (CFXMLNotationInfo *)src;
            CFXMLNotationInfo *destData = (CFXMLNotationInfo *)dest;
            destData->externalID.systemID = srcData->externalID.systemID ? (CFURLRef)CFRetain(srcData->externalID.systemID) : NULL;
            destData->externalID.publicID = srcData->externalID.publicID ? (CFStringRef)CFStringCreateCopy(alloc, srcData->externalID.publicID) : NULL;
            break;
        }
        case kCFXMLNodeTypeElementTypeDeclaration: {
            CFXMLElementTypeDeclarationInfo *srcData = (CFXMLElementTypeDeclarationInfo *)src;
            CFXMLElementTypeDeclarationInfo *destData = (CFXMLElementTypeDeclarationInfo *)dest;
            destData->contentDescription = srcData->contentDescription ? (CFStringRef)CFStringCreateCopy(alloc, srcData->contentDescription) : NULL;
            break;
        }
        case kCFXMLNodeTypeAttributeListDeclaration:
        {
            CFXMLAttributeListDeclarationInfo *sourceData = (CFXMLAttributeListDeclarationInfo *)src;
            CFXMLAttributeListDeclarationInfo *destData = (CFXMLAttributeListDeclarationInfo *)dest;
            CFIndex idx;
            destData->numberOfAttributes = sourceData->numberOfAttributes;
            destData->attributes = sourceData->numberOfAttributes ? (CFXMLAttributeDeclarationInfo *)CFAllocatorAllocate(alloc, sizeof(CFXMLAttributeDeclarationInfo)*sourceData->numberOfAttributes, 0) : NULL;
            for (idx = 0; idx < sourceData->numberOfAttributes; idx ++) {
                CFXMLAttributeDeclarationInfo sourceAttr = sourceData->attributes[idx];
                CFXMLAttributeDeclarationInfo *destAttr = &(destData->attributes[idx]);
                destAttr->attributeName = (CFStringRef)CFStringCreateCopy(alloc, sourceAttr.attributeName);
                destAttr->typeString = (CFStringRef)CFStringCreateCopy(alloc, sourceAttr.typeString);
                destAttr->defaultString = (CFStringRef)CFStringCreateCopy(alloc, sourceAttr.defaultString);
            }
            break;
        }
        default:
            CFAssert2(false, __kCFLogAssertion, "%s(): Encountered unexpected typeID %d (additionalData should be empty)", __PRETTY_FUNCTION__, xmlType);
    }
}

// Designated initializer; all node creation (except the wonky one created by the parser) should happen here.
CFXMLNodeRef CFXMLNodeCreate(CFAllocatorRef alloc, CFXMLNodeTypeCode xmlType, CFStringRef dataString, const void *additionalData, CFIndex version) {
    struct __CFXMLNode *node;
    UInt32 extraSize;
    // Add assertions checking xmlType against the presence/absence of additionalData & dataString
    switch (xmlType) {
        case kCFXMLNodeTypeDocument: extraSize = sizeof(CFXMLDocumentInfo); break;
        case kCFXMLNodeTypeElement: extraSize = sizeof(CFXMLElementInfo); break;
        case kCFXMLNodeTypeProcessingInstruction: extraSize = sizeof(CFXMLProcessingInstructionInfo); break;
        case kCFXMLNodeTypeEntity: extraSize = sizeof(CFXMLEntityInfo); break;
        case kCFXMLNodeTypeEntityReference: extraSize = sizeof(CFXMLEntityReferenceInfo); break;
        case kCFXMLNodeTypeDocumentType: extraSize = sizeof(CFXMLDocumentTypeInfo); break;
        case kCFXMLNodeTypeNotation: extraSize = sizeof(CFXMLNotationInfo); break;
        case kCFXMLNodeTypeElementTypeDeclaration: extraSize = sizeof(CFXMLElementTypeDeclarationInfo); break;
        case kCFXMLNodeTypeAttributeListDeclaration: extraSize = sizeof(CFXMLAttributeListDeclarationInfo); break;
        default: extraSize = 0;
    }

    node = (struct __CFXMLNode *)_CFRuntimeCreateInstance(alloc, CFXMLNodeGetTypeID(), sizeof(struct __CFXMLNode) + extraSize - sizeof(CFRuntimeBase), NULL);
    if (node) {
        alloc = CFGetAllocator(node);
        node->version = version;
        node->dataTypeID = xmlType;
        node->dataString = dataString ? (CFStringRef)CFStringCreateCopy(alloc, dataString) : NULL;
        if (extraSize != 0) {
            node->additionalData = (void *)((uint8_t *)node + sizeof(struct __CFXMLNode));
            _copyAddlDataForType(alloc, xmlType, additionalData, node->additionalData);
        } else {
            node->additionalData = NULL;
        }
    }
    return node;
}

CFXMLNodeTypeCode CFXMLNodeGetTypeCode(CFXMLNodeRef node) {
    return node->dataTypeID;
}

CFStringRef CFXMLNodeGetString(CFXMLNodeRef node) {
    return node->dataString;
}

const void *CFXMLNodeGetInfoPtr(CFXMLNodeRef node) {
    return node->additionalData;
}

CFIndex CFXMLNodeGetVersion(CFXMLNodeRef node) {
    return node->version;
}


