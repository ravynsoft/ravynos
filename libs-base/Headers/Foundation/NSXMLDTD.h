/* Interface for NSXMLDTD for GNUStep
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

#ifndef __NSXMLDTD_h_GNUSTEP_BASE_INCLUDE
#define __NSXMLDTD_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSXMLNode.h>

#if	defined(__cplusplus)
extern "C" {
#endif


@class NSData, NSMutableDictionary;
@class NSXMLDTDNode;

/**
 * Encapsulates document type definition data.
 */
GS_EXPORT_CLASS
@interface NSXMLDTD : NSXMLNode
{
#if     GS_NONFRAGILE
#  if	defined(GS_NSXMLDTD_IVARS)
@public GS_NSXMLDTD_IVARS;
#  endif
#endif
  /* The pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available is inherited from
   * NSXMLNode.  See Source/GSInternal.h for details.
   */
}

/**
 * Adds a child after the existing children.
 */
- (void) addChild: (NSXMLNode*)child;

/**
 * Returns the attribute declaration named.
 */
- (NSXMLDTDNode*) attributeDeclarationForName: (NSString*)name
                                   elementName: (NSString*)elementName;

/**
 * Returns the element declaration named.
 */
- (NSXMLDTDNode*) elementDeclarationForName: (NSString*)name;

/**
 * Returns the entity declaration named.
 */
- (NSXMLDTDNode*) entityDeclarationForName: (NSString*)name;

/** Load data from URL and initialise the receiver with the contents.
 */
- (id) initWithContentsOfURL: (NSURL*)url
                     options: (NSUInteger)mask
                       error: (NSError**)error;

/** <init />
 */
- (id) initWithData: (NSData*)data
            options: (NSUInteger)mask
              error: (NSError**)error;

/**
 * Inserts a child node at the specified index in the document.
 */
- (void) insertChild: (NSXMLNode*)child atIndex: (NSUInteger)index;

/**
 * Inserts a number of child nodes at the specified index.
 */
- (void) insertChildren: (NSArray*)children atIndex: (NSUInteger)index;

/**
 * Returns the notation declaration named.
 */
- (NSXMLDTDNode*) notationDeclarationForName: (NSString*)name;

/**
 * Returns the predefined entity declaration matching named.
 */
+ (NSXMLDTDNode*) predefinedEntityDeclarationForName: (NSString*)name;

/**
 * Returns the public ID set for the document.
 */
- (NSString*) publicID;

/**
 * Remove the indexed child node.
 */
- (void) removeChildAtIndex: (NSUInteger)index;

/**
 * Replaces the child at index with another child.
 */
- (void) replaceChildAtIndex: (NSUInteger)index withNode: (NSXMLNode*)theNode;

/**
 * Replaces all existing children with the child nodes in the array.
 */
- (void) setChildren: (NSArray*)children;

/**
 * Sets the public id of this document.<br />
 * This identifier should be in the default catalog or in a location
 * given by the XML_CATALOG_FILES environment variable.<br />
 * You should also set the systemID when you set this.
 */
- (void) setPublicID: (NSString*)publicID;

/**
 * Sets the system ID ... a URL referring to the DTD document.
 */
- (void) setSystemID: (NSString*)systemID;

/**
 * Returns the system ID
 */
- (NSString*) systemID;

@end

#if	defined(__cplusplus)
}
#endif

#endif /*__NSXMLDTD_GNUSTEP_BASE_INCLUDE */
