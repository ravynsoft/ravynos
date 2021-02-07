/* Implementation for NSXMLDTDNode for GNUStep
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

#import "common.h"

#if defined(HAVE_LIBXML)

#define GSInternal	NSXMLDTDNodeInternal

#import	"NSXMLPrivate.h"
#import "GSInternal.h"
GS_PRIVATE_INTERNAL(NSXMLDTDNode)

@implementation NSXMLDTDNode

- (void) dealloc
{
  if (GS_EXISTS_INTERNAL)
    {
    }
  [super dealloc];
}

- (NSXMLDTDNodeKind) DTDKind
{
  return internal->DTDKind;
}

- (void) _createInternal
{
  GS_CREATE_INTERNAL(NSXMLDTDNode);
}

- (id) initWithKind: (NSXMLNodeKind)theKind options: (NSUInteger)theOptions
{
  if (NSXMLEntityDeclarationKind == theKind
    || NSXMLElementDeclarationKind == theKind
    || NSXMLNotationDeclarationKind == theKind)
    {
      return [super initWithKind: theKind options: theOptions];
    }
  else
    {
      [self release];
      // This cast is here to keep clang quite that expects an init* method to 
      // return an object of the same class, which is not true here.
      return (NSXMLDTDNode*)[[NSXMLNode alloc] initWithKind: theKind
                                                    options: theOptions];
    }
}

- (id) initWithXMLString: (NSString*)string
{
  NSXMLDTDNode *result = nil;
  NSError *error;
  NSXMLDocument *tempDoc = 
    [[NSXMLDocument alloc] initWithXMLString: string
                                     options: 0
                                       error: &error];
  if (tempDoc != nil)
    {
      result = (NSXMLDTDNode*)RETAIN([tempDoc childAtIndex: 0]);
      [result detach]; // detach from document.
    }
  [tempDoc release];
  [self release];

  return result;
}

- (BOOL) isExternal
{
  if ([self systemID])
    {
      return YES;
    }
  return NO;
}

- (NSString*) notationName
{
  return StringFromXMLStringPtr(internal->node.entity->name);
}

- (NSString*) publicID
{
 return StringFromXMLStringPtr(internal->node.entity->ExternalID);
}

- (void) setDTDKind: (NSXMLDTDNodeKind)nodeKind
{
  internal->DTDKind = nodeKind;
}

- (void) setNotationName: (NSString*)notationName
{
  internal->node.entity->name = XMLSTRING(notationName);
}

- (void) setPublicID: (NSString*)publicID
{
  internal->node.entity->ExternalID = XMLSTRING(publicID);
}

- (void) setSystemID: (NSString*)systemID
{
  internal->node.entity->ExternalID = XMLSTRING(systemID);
}

- (NSString*) systemID
{
  return StringFromXMLStringPtr(internal->node.entity->SystemID);
}

@end

#endif	/* HAVE_LIBXML */
