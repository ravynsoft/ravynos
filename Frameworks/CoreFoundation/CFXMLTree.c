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

/*	CFXMLTree.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: David Smith
*/

#include "CFInternal.h"
#include <CoreFoundation/CFXMLParser.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

/*************/
/* CFXMLTree */
/*************/

/* Creates a childless node from desc */
CFXMLTreeRef CFXMLTreeCreateWithNode(CFAllocatorRef allocator, CFXMLNodeRef node) {
    CFTreeContext treeCtxt;
    treeCtxt.version = 0;
    treeCtxt.info = (void *)node;
    treeCtxt.retain = CFRetain;
    treeCtxt.release = CFRelease;
    treeCtxt.copyDescription = CFCopyDescription;
    return CFTreeCreate(allocator, &treeCtxt);
}

CFXMLNodeRef CFXMLTreeGetNode(CFXMLTreeRef xmlNode) {
    CFTreeContext treeContext;
    treeContext.version = 0;
    CFTreeGetContext(xmlNode, &treeContext);
    return (CFXMLNodeRef)treeContext.info;
}

// We will probably ultimately want to export this under some public API name
CF_PRIVATE Boolean CFXMLTreeEqual(CFXMLTreeRef xmlTree1, CFXMLTreeRef xmlTree2) {
    CFXMLNodeRef node1, node2;
    CFXMLTreeRef child1, child2;
    if (CFTreeGetChildCount(xmlTree1) != CFTreeGetChildCount(xmlTree2)) return false;
    node1 = CFXMLTreeGetNode(xmlTree1);
    node2 = CFXMLTreeGetNode(xmlTree2);
    if (!CFEqual(node1, node2)) return false;
    for (child1 = CFTreeGetFirstChild(xmlTree1), child2 = CFTreeGetFirstChild(xmlTree2); child1 && child2; child1 = CFTreeGetNextSibling(child1), child2 = CFTreeGetNextSibling(child2)) {
        if (!CFXMLTreeEqual(child1, child2)) return false;
    }
    return true;
}

static void _CFAppendXML(CFMutableStringRef str, CFXMLTreeRef tree);
static void _CFAppendXMLProlog(CFMutableStringRef str, const CFXMLTreeRef node);
static void _CFAppendXMLEpilog(CFMutableStringRef str, const CFXMLTreeRef node);

CFDataRef CFXMLTreeCreateXMLData(CFAllocatorRef allocator, CFXMLTreeRef xmlTree) {
    CFMutableStringRef xmlStr; 
    CFDataRef result;
    CFStringEncoding encoding;

    __CFGenericValidateType(xmlTree, CFTreeGetTypeID());
    
    xmlStr = CFStringCreateMutable(allocator, 0);
    _CFAppendXML(xmlStr, xmlTree);
    if (CFXMLNodeGetTypeCode(CFXMLTreeGetNode(xmlTree)) == kCFXMLNodeTypeDocument) {
        const CFXMLDocumentInfo *docData = (CFXMLDocumentInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(xmlTree));
        encoding = docData ? docData->encoding : kCFStringEncodingUTF8;
    } else {
        encoding = kCFStringEncodingUTF8;
    }
    result = CFStringCreateExternalRepresentation(allocator, xmlStr, encoding, 0);
    CFRelease(xmlStr);
    return result;
}

static void _CFAppendXML(CFMutableStringRef str, CFXMLTreeRef tree) {
    CFXMLTreeRef child;
    _CFAppendXMLProlog(str, tree);
    for (child = CFTreeGetFirstChild(tree); child; child = CFTreeGetNextSibling(child)) {
        _CFAppendXML(str, child);
    }
    _CFAppendXMLEpilog(str, tree);
}

CF_PRIVATE void appendQuotedString(CFMutableStringRef str, CFStringRef strToQuote) {
    char quoteChar = CFStringFindWithOptions(strToQuote, CFSTR("\""), CFRangeMake(0, CFStringGetLength(strToQuote)), 0, NULL) ? '\'' : '\"';
    CFStringAppendFormat(str, NULL, CFSTR("%c%@%c"), quoteChar, strToQuote, quoteChar);
}

static void appendExternalID(CFMutableStringRef str, CFXMLExternalID *extID) {
    if (extID->publicID) {
        CFStringAppendCString(str, " PUBLIC ", kCFStringEncodingASCII);
        appendQuotedString(str, extID->publicID);
        if (extID->systemID) {
            // Technically, for externalIDs, systemID must not be NULL.  However, by testing for a NULL systemID, we can use this to emit publicIDs, too.  REW, 2/15/2000
            CFStringAppendCString(str, " ", kCFStringEncodingASCII);
            appendQuotedString(str, CFURLGetString(extID->systemID));
        }
    } else if (extID->systemID) {
        CFStringAppendCString(str, " SYSTEM ", kCFStringEncodingASCII);
        appendQuotedString(str, CFURLGetString(extID->systemID));
    } else {
        // Should never get here
    }
}

static void appendElementProlog(CFMutableStringRef str, CFXMLTreeRef tree) {
    const CFXMLElementInfo *data = (CFXMLElementInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree));
    CFStringAppendFormat(str, NULL, CFSTR("<%@"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
    if (data->attributeOrder) {
        CFIndex i, c = CFArrayGetCount(data->attributeOrder);
        for (i = 0; i < c; i ++) {
            CFStringRef attr = (CFStringRef)CFArrayGetValueAtIndex(data->attributeOrder, i);
            CFStringRef value = (CFStringRef)CFDictionaryGetValue(data->attributes, attr);
            CFStringAppendFormat(str, NULL, CFSTR(" %@="), attr);
            appendQuotedString(str, value);
        }
    }
    if (data->isEmpty) {
        CFStringAppendCString(str, "/>", kCFStringEncodingASCII);
    } else {
        CFStringAppendCString(str, ">", kCFStringEncodingASCII);
    }
}

/* Although named "prolog", for leafs of the tree, this is the only XML generation function called.  This is why Comments, Processing Instructions, etc. generate their XML during this function.  REW, 2/11/2000 */
static void _CFAppendXMLProlog(CFMutableStringRef str, const CFXMLTreeRef tree) {
    switch (CFXMLNodeGetTypeCode(CFXMLTreeGetNode(tree))) {
        case kCFXMLNodeTypeDocument:
            break;
        case kCFXMLNodeTypeElement:
            appendElementProlog(str, tree);
            break;
        case kCFXMLNodeTypeAttribute:
            // Should never be encountered
            break;
        case kCFXMLNodeTypeProcessingInstruction: {
            CFXMLProcessingInstructionInfo *data = (CFXMLProcessingInstructionInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree));
            if (data->dataString) {
                CFStringAppendFormat(str, NULL, CFSTR("<?%@ %@?>"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)), data->dataString);
            } else {
                CFStringAppendFormat(str, NULL, CFSTR("<?%@?>"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            }
            break;
        }
        case kCFXMLNodeTypeComment:
            CFStringAppendFormat(str, NULL, CFSTR("<!--%@-->"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            break;
        case kCFXMLNodeTypeText:
            CFStringAppend(str, CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            break;
        case kCFXMLNodeTypeCDATASection:
            CFStringAppendFormat(str, NULL, CFSTR("<![CDATA[%@]]>"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            break;
        case kCFXMLNodeTypeDocumentFragment:
            break;
        case kCFXMLNodeTypeEntity: {
            CFXMLEntityInfo *data = (CFXMLEntityInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree));
            CFStringAppendCString(str, "<!ENTITY ", kCFStringEncodingASCII);
            if (data->entityType == kCFXMLEntityTypeParameter) {
                CFStringAppend(str, CFSTR("% "));
            }
            CFStringAppend(str, CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            CFStringAppend(str, CFSTR(" "));
            if (data->replacementText) {
                appendQuotedString(str, data->replacementText);
                CFStringAppendCString(str, ">", kCFStringEncodingASCII);
            } else {
                appendExternalID(str, &(data->entityID));
                if (data->notationName) {
                    CFStringAppendFormat(str, NULL, CFSTR(" NDATA %@"), data->notationName);
                }
                CFStringAppendCString(str, ">", kCFStringEncodingASCII);
            }
                break;
        }
        case kCFXMLNodeTypeEntityReference:
        {
            CFXMLEntityTypeCode entityType = ((CFXMLEntityReferenceInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree)))->entityType;
            if (entityType == kCFXMLEntityTypeParameter) {
                CFStringAppendFormat(str, NULL, CFSTR("%%%@;"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            } else {
                CFStringAppendFormat(str, NULL, CFSTR("&%@;"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            }
            break;
        }
        case kCFXMLNodeTypeDocumentType:
            CFStringAppendCString(str, "<!DOCTYPE ", kCFStringEncodingASCII);
            CFStringAppend(str, CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            if (CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree))) {
                CFXMLExternalID *extID = &((CFXMLDocumentTypeInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree)))->externalID;
                appendExternalID(str, extID);
            }
                CFStringAppendCString(str, " [", kCFStringEncodingASCII);
            break;
        case kCFXMLNodeTypeWhitespace:
            CFStringAppend(str, CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            break;
        case kCFXMLNodeTypeNotation: {
            CFXMLNotationInfo *data = (CFXMLNotationInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree));
            CFStringAppendFormat(str, NULL, CFSTR("<!NOTATION %@ "), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            appendExternalID(str, &(data->externalID));
                CFStringAppendCString(str, ">", kCFStringEncodingASCII);
            break;
        }
        case kCFXMLNodeTypeElementTypeDeclaration:
            CFStringAppendFormat(str, NULL, CFSTR("<!ELEMENT %@ %@>"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)), ((CFXMLElementTypeDeclarationInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree)))->contentDescription);
            break;
        case kCFXMLNodeTypeAttributeListDeclaration: {
            CFXMLAttributeListDeclarationInfo *attListData = (CFXMLAttributeListDeclarationInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree));
            CFIndex idx;
            CFStringAppendCString(str, "<!ATTLIST ", kCFStringEncodingASCII);
            CFStringAppend(str, CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
            for (idx = 0; idx < attListData->numberOfAttributes; idx ++) {
                CFXMLAttributeDeclarationInfo *attr = &(attListData->attributes[idx]);
                CFStringAppendFormat(str, NULL, CFSTR("\n\t%@ %@ %@"), attr->attributeName, attr->typeString, attr->defaultString);
            }
            CFStringAppendCString(str, ">", kCFStringEncodingASCII);
            break;
        }
        default:
            CFAssert1(false, __kCFLogAssertion, "Encountered unexpected XMLDataTypeID %d", CFXMLNodeGetTypeCode(CFXMLTreeGetNode(tree)));
    }
}

static void _CFAppendXMLEpilog(CFMutableStringRef str, CFXMLTreeRef tree) {
    CFXMLNodeTypeCode typeID = CFXMLNodeGetTypeCode(CFXMLTreeGetNode(tree));
    if (typeID == kCFXMLNodeTypeElement) {
        if (((CFXMLElementInfo *)CFXMLNodeGetInfoPtr(CFXMLTreeGetNode(tree)))->isEmpty) return;
        CFStringAppendFormat(str, NULL, CFSTR("</%@>"), CFXMLNodeGetString(CFXMLTreeGetNode(tree)));
    } else if (typeID == kCFXMLNodeTypeDocumentType) {
        CFIndex len = CFStringGetLength(str);
        if (CFStringHasSuffix(str, CFSTR(" ["))) {
            // There were no in-line DTD elements
            CFStringDelete(str, CFRangeMake(len-2, 2));
        } else {
            CFStringAppendCString(str, "]", kCFStringEncodingASCII);
        }
        CFStringAppendCString(str, ">", kCFStringEncodingASCII);
    }
}

#pragma GCC diagnostic pop
