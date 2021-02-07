/* Interface for NSXMLDTDNode for GNUStep
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

#ifndef __NSXMLDTDNode_h_GNUSTEP_BASE_INCLUDE
#define __NSXMLDTDNode_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSXMLNode.h>

#if	defined(__cplusplus)
extern "C" {
#endif

/**
 * The kind of DTD node.
 */
enum {
  NSXMLEntityGeneralKind = 1,
  NSXMLEntityParsedKind,
  NSXMLEntityUnparsedKind,
  NSXMLEntityParameterKind,
  NSXMLEntityPredefined,
  NSXMLAttributeCDATAKind,
  NSXMLAttributeIDKind,
  NSXMLAttributeIDRefKind,
  NSXMLAttributeIDRefsKind,
  NSXMLAttributeEntityKind,
  NSXMLAttributeEntitiesKind,
  NSXMLAttributeNMTokenKind,
  NSXMLAttributeNMTokensKind,
  NSXMLAttributeEnumerationKind,
  NSXMLAttributeNotationKind,
  NSXMLElementDeclarationUndefinedKind,
  NSXMLElementDeclarationEmptyKind,
  NSXMLElementDeclarationAnyKind,
  NSXMLElementDeclarationMixedKind,
  NSXMLElementDeclarationElementKind
};
typedef NSUInteger NSXMLDTDNodeKind;

/**
 * Represents the nodes whose types are present only in DTDs.<br />
 * Object values for the different nodes are:
 * <deflist>
 * <term>Entity declaration</term>
 * <desc>The string that that entity resolves to eg "&lt;"</desc>
 * <term>Attribute declaration</term>
 * <desc>The default value, if any</desc>
 * <term>Element declaration</term>
 * <desc>The validation string</desc>
 * <term>Notation declaration</term>
 * <desc>nil</desc>
 * </deflist>
 */
GS_EXPORT_CLASS
@interface NSXMLDTDNode : NSXMLNode
{
#if     GS_NONFRAGILE
#  if	defined(GS_NSXMLDTDNode_IVARS)
@public GS_NSXMLDTDNode_IVARS;
#  endif
#endif
  /* The pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available is inherited from
   * NSXMLNode.  See Source/GSInternal.h for details.
   */
}

/**
 * Returns what kind of DTD node this is.
 */
- (NSXMLDTDNodeKind) DTDKind;

/** <init />
 * Initialises the receiver based on the contents of the supplied XML.
 */
- (id) initWithXMLString: (NSString*)string;

/**
 * Returns YES if the system id is set, NO otherwise.<br />
 * Is valid only for entities and notations.
 */
- (BOOL) isExternal;

/**
 * Returns the notation name.
 */
- (NSString*) notationName;

/**
 * Returns the public id.
 */
- (NSString*) publicID;

/**
 * Sets what kind of DTD node this is.
 */
- (void) setDTDKind: (NSXMLDTDNodeKind)nodeKind;

/**
 * Sets the notation name if the receiver is an entity.
 */
- (void) setNotationName: (NSString*)notationName;

/**
 * Sets the public id of this node.<br />
 * This identifier should be in the default catalog or in a location
 * given by the XML_CATALOG_FILES environment variable.<br />
 * When the public id is set the system id must also be set.<br />
 * This is valid only for entities and notations.
 */
- (void) setPublicID: (NSString*)publicID;

/**
 * Sets the system ID ... a URL referring to the DTD document.
 */
- (void) setSystemID: (NSString*)systemID;

/**
 * Returns the system ID.
 */
- (NSString*) systemID;

@end


#if	defined(__cplusplus)
}
#endif

#endif /*__NSXMLDTDNode_h_GNUSTEP_BASE_INCLUDE */
