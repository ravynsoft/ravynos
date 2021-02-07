/* Inmplementation for NSXMLDTD for GNUStep
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

#define GSInternal	NSXMLDTDInternal

#import "NSXMLPrivate.h"
#import "GSInternal.h"
GS_PRIVATE_INTERNAL(NSXMLDTD)

@implementation NSXMLDTD

+ (NSXMLDTDNode*) predefinedEntityDeclarationForName: (NSString*)name
{
  xmlEntityPtr node = xmlGetPredefinedEntity(XMLSTRING(name));
  return (NSXMLDTDNode*)[self _objectForNode: (xmlNodePtr)node];
}

- (void) dealloc
{
  if (GS_EXISTS_INTERNAL)
    {
    }
  [super dealloc];
}

- (void) addChild: (NSXMLNode*)child
{
  [self insertChild: child atIndex: [self childCount]];
}

- (NSXMLDTDNode*) attributeDeclarationForName: (NSString*)name
                                  elementName: (NSString*)elementName
{
  xmlDtdPtr theNode = internal->node.dtd;
  xmlNodePtr children = NULL;
  const xmlChar *xmlName = XMLSTRING(name);
  const xmlChar *xmlElementName = XMLSTRING(elementName);

  if ((theNode == NULL) ||
      (theNode->children == NULL))
    {
      return nil;
    }
     
 for (children = theNode->children; children; children = children->next)
   {
     if (children->type == XML_ATTRIBUTE_DECL)
       {
         xmlAttributePtr attr = (xmlAttributePtr)children;

         if ((xmlStrcmp(attr->name, xmlName) == 0) &&
             (xmlStrcmp(attr->elem, xmlElementName) == 0))
           {
             return (NSXMLDTDNode*)[NSXMLNode _objectForNode: children];
           }
       }
   }

  return nil;
}

- (NSXMLDTDNode*) elementDeclarationForName: (NSString*)name
{
  xmlDtdPtr theNode = internal->node.dtd;
  xmlNodePtr children = NULL;
  const xmlChar *xmlName = XMLSTRING(name);

  if ((theNode == NULL) ||
      (theNode->children == NULL))
    {
      return nil;
    }
     
 for (children = theNode->children; children; children = children->next)
   {
     if (children->type == XML_ELEMENT_DECL)
       {
         xmlElementPtr elem = (xmlElementPtr)children;

         if (xmlStrcmp(elem->name, xmlName) == 0)
           {
             return (NSXMLDTDNode*)[NSXMLNode _objectForNode: children];
           }
       }
   }

  return nil;
}

- (NSXMLDTDNode*) entityDeclarationForName: (NSString*)name
{
  //xmlGetEntityFromDtd 
  xmlDtdPtr theNode = internal->node.dtd;
  xmlNodePtr children = NULL;
  const xmlChar *xmlName = XMLSTRING(name);

  if ((theNode == NULL) ||
      (theNode->children == NULL))
    {
      return nil;
    }
     
 for (children = theNode->children; children; children = children->next)
   {
     if (children->type == XML_ENTITY_DECL)
       {
         xmlEntityPtr entity = (xmlEntityPtr)children;

         if (xmlStrcmp(entity->name, xmlName) == 0)
           {
             return (NSXMLDTDNode*)[NSXMLNode _objectForNode: children];
           }
       }
   }

  return nil;
}

- (void) _createInternal
{
  GS_CREATE_INTERNAL(NSXMLDTD);
}

- (id) init
{
  return [self initWithKind: NSXMLDTDKind options: 0];
}

- (id) initWithContentsOfURL: (NSURL*)url
                     options: (NSUInteger)mask
                       error: (NSError**)error
{
  NSData	*data;
  NSXMLDTD	*doc;

  data = [NSData dataWithContentsOfURL: url];
  doc = [self initWithData: data options: mask error: error];
  [doc setURI: [url absoluteString]];
  return doc;
}

- (id) initWithData: (NSData*)data
            options: (NSUInteger)mask
              error: (NSError**)error
{
  NSXMLDocument *tempDoc = 
    [[NSXMLDocument alloc] initWithData: data
                                options: mask
                                  error: error];
  if (tempDoc != nil)
    {
      NSArray *children = [tempDoc children];
      NSEnumerator *enumerator = [children objectEnumerator];
      NSXMLNode *child;

      self = [self initWithKind: NSXMLDTDKind options: mask];
      
      while ((child = [enumerator nextObject]) != nil)
        {
          [child detach]; // detach from document.
          [self addChild: child];
        }
      [tempDoc release];
    }

  return self;
}

- (id) initWithKind: (NSXMLNodeKind)theKind options: (NSUInteger)theOptions
{
  if (NSXMLDTDKind == theKind)
    {
      return [super initWithKind: theKind options: theOptions];
    }
  else
    {
      [self release];
      // This cast is here to keep clang quite that expects an init* method to 
      // return an object of the same class, which is not true here.
      return (NSXMLDTD*)[[NSXMLNode alloc] initWithKind: theKind
                                                options: theOptions];
    }
}

- (void) insertChild: (NSXMLNode*)child atIndex: (NSUInteger)index
{
  NSXMLNodeKind	theKind = [child kind];
  NSUInteger childCount = [self childCount];

  // Check to make sure this is a valid addition...
  NSAssert(nil != child, NSInvalidArgumentException);
  NSAssert(index <= childCount, NSInvalidArgumentException);
  NSAssert(nil == [child parent], NSInvalidArgumentException);
  NSAssert(NSXMLAttributeKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLDTDKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLDocumentKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLElementKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLInvalidKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLNamespaceKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLTextKind != theKind, NSInvalidArgumentException);

  [self _insertChild: child atIndex: index];
}

- (void) insertChildren: (NSArray*)children atIndex: (NSUInteger)index
{
  NSEnumerator	*enumerator = [children objectEnumerator];
  NSXMLNode	*child;

  while ((child = [enumerator nextObject]) != nil)
    {
      [self insertChild: child atIndex: index++];
    }
}

- (NSXMLDTDNode*) notationDeclarationForName: (NSString*)name
{
  xmlDtdPtr theNode = internal->node.dtd;
  xmlNodePtr children = NULL;
  const xmlChar *xmlName = XMLSTRING(name);

  if ((theNode == NULL) ||
      (theNode->children == NULL))
    {
      return nil;
    }
     
 for (children = theNode->children; children; children = children->next)
   {
     if (children->type == XML_NOTATION_NODE)
       {
         if (xmlStrcmp(children->name, xmlName) == 0)
           {
             return (NSXMLDTDNode*)[NSXMLNode _objectForNode: children];
           }
       }
   }

  return nil;
}

- (NSString*) publicID
{
  xmlDtd *theNode = internal->node.dtd;

  return StringFromXMLStringPtr(theNode->ExternalID); 
}

- (void) removeChildAtIndex: (NSUInteger)index
{
  NSXMLNode *child;

  if (index >= [self childCount])
    {
      [NSException raise: NSRangeException
                  format: @"index too large"];
    }

  child = [self childAtIndex: index];
  [child detach];
}

- (void) replaceChildAtIndex: (NSUInteger)index withNode: (NSXMLNode*)theNode
{
  [self insertChild: theNode atIndex: index];
  [self removeChildAtIndex: index + 1];
}

- (void) setChildren: (NSArray*)children
{
  NSUInteger count = [self childCount];

  while (count-- > 0)
    {
      [self removeChildAtIndex: count];
    }

  [self insertChildren: children atIndex: 0];
}

- (void) setPublicID: (NSString*)publicID
{
  xmlDtd *theNode = internal->node.dtd;

  theNode->ExternalID = XMLStringCopy(publicID); 
}

- (void) setSystemID: (NSString*)systemID
{
  xmlDtd *theNode = internal->node.dtd;

  theNode->SystemID = XMLStringCopy(systemID); 
}

- (NSString*) systemID
{
  xmlDtd *theNode = internal->node.dtd;

  return StringFromXMLStringPtr(theNode->SystemID); 
}

@end

#endif	/* HAVE_LIBXML */
