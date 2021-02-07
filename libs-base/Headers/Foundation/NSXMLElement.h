/* Interface for NSXMLElement for GNUStep
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

#ifndef __NSXMLElement_h_GNUSTEP_BASE_INCLUDE
#define __NSXMLElement_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSXMLNode.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSDictionary, NSEnumerator, NSMutableArray, NSMutableDictionary;

/**
 * Represents an XML element.<br />
 */
GS_EXPORT_CLASS
@interface NSXMLElement : NSXMLNode
{
#if     GS_NONFRAGILE
#  if	defined(GS_NSXMLElement_IVARS)
@public GS_NSXMLElement_IVARS;
#  endif
#endif
  /* The pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available is inherited from
   * NSXMLNode.  See Source/GSInternal.h for details.
   */
}

/**
 * Initialises the receiver with the given name.
 */
- (id) initWithName: (NSString*)name;

/**
 * Initialises the receiver with the given name and namespace URI.
 */
- (id) initWithName: (NSString*)name URI: (NSString*)URI;

/**
 * Initialises the receiver as a text node with the given name and content.
 */
- (id) initWithName: (NSString*)name stringValue: (NSString*)string;

/**
 * Initialises the receiver by parsing the XML string supplied.
 */
- (id) initWithXMLString: (NSString*)string error: (NSError**)error;

/**
 * Searches for and returns all child elements which match name.
 */
- (NSArray*) elementsForName: (NSString*)name;

/**
 * Searches for and returns all child elements which match localName
 * and the specified URI.
 */
- (NSArray*) elementsForLocalName: (NSString*)localName URI: (NSString*)URI;

/**
 * Adds the supplied attribute to the receiver (ignoring if it has a duplicate
 * name).
 */
- (void) addAttribute: (NSXMLNode*)attribute;

/**
 * Removes the named attribute.
 */
- (void) removeAttributeForName: (NSString*)name;

/**
 * Sets the attributes of the receiver, ignoring all but the first of any
 * duplicates.
 */
- (void) setAttributes: (NSArray*)attributes;

/**
 * Sets attributes from the supplied dictionary.<br />
 * DEPRECATED ... use -setAttributesWithDictionary: instead.
 */
- (void) setAttributesAsDictionary: (NSDictionary*)attributes;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
/**
 * Sets attributes from the supplied dictionary.
 */
- (void) setAttributesWithDictionary: (NSDictionary*)attributes;
#endif

/**
 * Returns the receiver's attributes.
 */
- (NSArray*) attributes;

/**
 * Returns the named attribute.
 */
- (NSXMLNode*) attributeForName: (NSString*)name;

/**
 * Returns the attribute matching localName and URI.
 */
- (NSXMLNode*) attributeForLocalName: (NSString*)localName
                                  URI: (NSString*)URI;

/**
 * Adds a namespace unless the name is a duplicate.
 */
- (void) addNamespace: (NSXMLNode*)aNamespace;

/**
 * Removes a named namespace.
 */
- (void) removeNamespaceForPrefix: (NSString*)name;

/**
 * Sets the namespaces for the receiver, ignoring all but the first
 * of any duplicates.
 */
- (void) setNamespaces: (NSArray*)namespaces;

/**
 * Returns the namespaces of the receiver.
 */
- (NSArray*) namespaces;

/**
 * Returns the namespace for the specified prefix in the receiver.
 */
- (NSXMLNode*) namespaceForPrefix: (NSString*)name;

/**
 * Returns the namespace found by searching the chain of namespaces.
 */
- (NSXMLNode*) resolveNamespaceForName: (NSString*)name;

/**
 * Returns the URI by searching the chain of namespaces.
 */
- (NSString*) resolvePrefixForNamespaceURI: (NSString*)namespaceURI;

/**
 * Inerts a child node.
 */
- (void) insertChild: (NSXMLNode*)child atIndex: (NSUInteger)index;

/**
 * Inserts a number of children.
 */
- (void) insertChildren: (NSArray*)children atIndex: (NSUInteger)index;

/**
 * Removes a child node.
 */
- (void) removeChildAtIndex: (NSUInteger)index;

/**
 * Replaces all existing child nodes with those from the array.
 */
- (void) setChildren: (NSArray*)children;

/**
 * Adds a child after existing children.
 */
- (void) addChild: (NSXMLNode*)child;

/**
 * Replaces the child at the specified index.
 */
- (void) replaceChildAtIndex: (NSUInteger)index withNode: (NSXMLNode*)theNode;

/**
 * Merges adjacent text nodes. If a node's value is the empty string,
 * and preserve is NO, it is removed.<br />
 * This should be called with a value of NO before using
 * XQuery or XPath.
 */
- (void) normalizeAdjacentTextNodesPreservingCDATA: (BOOL)preserve;

@end

#if	defined(__cplusplus)
}
#endif

#endif /*__NSXMLElement_h_GNUSTEP_BASE_INCLUDE */
