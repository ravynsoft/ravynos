/* Interface for NSXMLNodeOptions for GNUStep
   Copyright (C) 2008 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Created: September 2008

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef __NSXMLNodeOptions_h_GNUSTEP_BASE_INCLUDE
#define __NSXMLNodeOptions_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObjCRuntime.h>

#if	defined(__cplusplus)
extern "C" {
#endif

/** This enumeration is a bitmask specifying input and output options
 * for nnode.
 * <deflist>
 * <term>NSXMLNodeIsCDATA</term>
 * <desc>Indicates that a text node is CDATA</desc>
 * <term>NSXMLNodeExpandEmptyElement</term>
 * <desc>The element should be expanded when empty,
 * eg &lt;elementname&gt;&lt;/elementname&gt;.
 * For some reason MacOS-X specifies this as the default setting.</desc>
 * <term>NSXMLNodeCompactEmptyElement</term>
 * <desc>This element should be compact when empty,
 * eg &lt;elementname/&gt;</desc>
 * <term>NSXMLNodeUseSingleQuotes</term>
 * <desc>Use single quotes on this attribute or namespace</desc>
 * <term>NSXMLNodeUseDoubleQuotes</term>
 * <desc>Use double quotes on this attribute or namespace.
 * MacOS-X specifies this as the default setting.</desc>
 * <term>NSXMLNodeOptionsNone</term>
 * <desc>Use the default options for this node</desc>
 * <term>NSXMLNodePreserveAll</term>
 * <desc>Turn all preservation options on</desc>
 * <term>NSXMLNodePreserveNamespaceOrder</term>
 * <desc>Preserve the order of namespaces</desc>
 * <term>NSXMLNodePreserveAttributeOrder</term>
 * <desc>Preserve the order of attributes</desc>
 * <term>NSXMLNodePreserveEntities</term>
 * <desc>Entities should not be resolved on output</desc>
 * <term>NSXMLNodePreservePrefixes</term>
 * <desc>Prefixes should not be chosen based on closest URI definition</desc>
 * <term>NSXMLNodePreserveCDATA</term>
 * <desc>CDATA should be preserved</desc>
 * <term>NSXMLNodePreserveEmptyElements</term>
 * <desc>Remember whether an empty element was expanded or compact</desc>
 * <term>NSXMLNodePreserveQuotes</term>
 * <desc>Remember whether an attribute used single or double quotes</desc>
 * <term>NSXMLNodePreserveWhitespace</term>
 * <desc>Preserve non-content whitespace</desc>
 * <term>NSXMLNodePreserveDTD</term>
 * <desc>Preserve the DTD until it is modified</desc>
 * <term>NSXMLDocumentTidyHTML</term>
 * <desc>Try to change HTML into valid XHTML</desc>
 * <term>NSXMLDocumentTidyXML</term>
 * <desc>Try to change malformed XML into valid XML</desc>
 * <term>NSXMLDocumentValidate</term>
 * <desc>Validate this document against its DTD</desc>
 * <term>NSXMLNodePrettyPrint</term>
 * <desc>Output this node in a readable format</desc>
 * <term>NSXMLDocumentIncludeContentTypeDeclaration</term>
 * <desc>Include a content type declaration for HTML or XHTML</desc>
 */

enum {
    NSXMLNodeOptionsNone = 0,
    
    NSXMLNodeIsCDATA = 1 << 0,
    NSXMLNodeExpandEmptyElement = 1 << 1,
    NSXMLNodeCompactEmptyElement =  1 << 2,
    NSXMLNodeUseSingleQuotes = 1 << 3,
    NSXMLNodeUseDoubleQuotes = 1 << 4,
    
    NSXMLDocumentTidyHTML = 1 << 9,
    NSXMLDocumentTidyXML = 1 << 10,
    
    NSXMLDocumentValidate = 1 << 13,

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST) 
    NSXMLNodeLoadExternalEntitiesAlways = 1 << 14,
    NSXMLNodeLoadExternalEntitiesSameOriginOnly = 1 << 15,
#endif

    NSXMLDocumentXInclude = 1 << 16,
    
    NSXMLNodePrettyPrint = 1 << 17,
    NSXMLDocumentIncludeContentTypeDeclaration = 1 << 18,

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST) 
    NSXMLNodeLoadExternalEntitiesNever = 1 << 19,
#endif
    
    NSXMLNodePreserveNamespaceOrder = 1 << 20,
    NSXMLNodePreserveAttributeOrder = 1 << 21,
    NSXMLNodePreserveEntities = 1 << 22,
    NSXMLNodePreservePrefixes = 1 << 23,
    NSXMLNodePreserveCDATA = 1 << 24,
    NSXMLNodePreserveWhitespace = 1 << 25,
    NSXMLNodePreserveDTD = 1 << 26,
    NSXMLNodePreserveCharacterReferences = 1 << 27,    
    NSXMLNodePreserveEmptyElements = (NSXMLNodeCompactEmptyElement
      | NSXMLNodeExpandEmptyElement),
    NSXMLNodePreserveQuotes = (NSXMLNodeUseDoubleQuotes
      | NSXMLNodeUseSingleQuotes),
    NSXMLNodePreserveAll = ( NSXMLNodePreserveAttributeOrder
      | NSXMLNodePreserveCDATA
      | NSXMLNodePreserveCharacterReferences
      | NSXMLNodePreserveDTD
      | NSXMLNodePreserveEmptyElements
      | NSXMLNodePreserveEntities
      | NSXMLNodePreserveNamespaceOrder
      | NSXMLNodePreservePrefixes
      | NSXMLNodePreserveQuotes
      | NSXMLNodePreserveWhitespace
      | 0xFFF00000) // high 12 bits
};

#if	defined(__cplusplus)
}
#endif

#endif /*__NSXMLNodeOptions_h_GNUSTEP_BASE_INCLUDE */
