/* Implementation for NSXMLElement for GNUStep
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

#define GSInternal	NSXMLElementInternal

#import "NSXMLPrivate.h"
#import "GSInternal.h"
GS_PRIVATE_INTERNAL(NSXMLElement)

extern void cleanup_namespaces(xmlNodePtr node, xmlNsPtr ns);
extern void ensure_oldNs(xmlNodePtr node);

@implementation NSXMLElement

- (void) dealloc
{
  if (GS_EXISTS_INTERNAL && internal != nil)
    {
      /*
      NSArray *subNodes = [internal->subNodes copy];
      NSEnumerator *enumerator = [subNodes objectEnumerator];
      NSXMLNode *subNode;

      while ((subNode = [enumerator nextObject]) != nil)
        {
          if ([subNode kind] == NSXMLNamespaceKind)
            {
              [self removeNamespaceForPrefix: [subNode name]];
            }
        }
      */
    }

  [super dealloc];
}

- (void) _createInternal
{
  GS_CREATE_INTERNAL(NSXMLElement);
}

- (id) init
{
  return [self initWithKind: NSXMLElementKind options: 0];
}

- (id) initWithKind: (NSXMLNodeKind)theKind options: (NSUInteger)theOptions
{
  if (NSXMLElementKind == theKind)
    {
      return [super initWithKind: theKind options: theOptions];
    }
  else
    {
      [self release];
      // This cast is here to keep clang quite that expects an init* method to 
      // return an object of the same class, which is not true here.
      return (NSXMLElement*)[[NSXMLNode alloc] initWithKind: theKind
                                                    options: theOptions];
    }
}

- (id) initWithName: (NSString*)name
{
  return [self initWithName: name URI: nil];
}

- (id) initWithName: (NSString*)name URI: (NSString*)URI
{
  if ((self = [self initWithKind: NSXMLElementKind]) != nil)
    {
      [self setName: name];
      // Without this check this could unset a namespace set via the name
      if (URI != nil)
        {
          [self setURI: URI];
        }
    }
  return self;
}

- (id) initWithName: (NSString*)name stringValue: (NSString*)string
{
  if ((self = [self initWithName: name URI: nil]) != nil)
    {
      NSXMLNode *t;

      t = [[NSXMLNode alloc] initWithKind: NSXMLTextKind];
      [t setStringValue: string];
      [self addChild: t];
      [t release];
    }
  return self;
}

- (id) initWithXMLString: (NSString*)string 
		   error: (NSError**)error
{
  NSXMLElement *result = nil;
  NSXMLDocument *tempDoc = 
    [[NSXMLDocument alloc] initWithXMLString: string
                                     options: 0
                                       error: error];
  if (tempDoc != nil)
    {
      result = RETAIN([tempDoc rootElement]);
      [result detach]; // detach from document.
    }
  [tempDoc release];
  [self release];

  return result;
}

- (id) objectValue
{
  if (internal->objectValue == nil)
    {
      return @"";
    }
  return internal->objectValue;
}

- (NSArray*) elementsForName: (NSString*)name
{
  NSString *prefix = [[self class] prefixForName: name];

  if ((nil != prefix) && [prefix length] > 0)
    {
      NSXMLNode *ns = [self namespaceForPrefix: prefix];

      if (nil != ns)
        {
          NSString *localName = [[self class] localNameForName: name];

          // Namespace nodes have the URI as their stringValue
          return [self elementsForLocalName: localName URI: [ns stringValue]];
        }
    }

    {
      NSMutableArray *results = [NSMutableArray arrayWithCapacity: 10];
      xmlNodePtr cur = NULL;
      const xmlChar *xmlName = XMLSTRING(name);
      
      for (cur = internal->node.node->children; cur != NULL; cur = cur->next)
        {
          if (cur->type == XML_ELEMENT_NODE)
            {
              // no namespace or default namespace
              if ((xmlStrcmp(xmlName, cur->name) == 0) &&
                  ((cur->ns == NULL) || (cur->ns->prefix == NULL) ||
                   (xmlStrcmp(cur->ns->prefix, (const xmlChar*)"") == 0)))
                {
                  NSXMLNode *theNode = [NSXMLNode _objectForNode: cur];
                  [results addObject: theNode];
                }
            }
        }
  
      return results;
    }
}

- (NSArray*) elementsForLocalName: (NSString*)localName URI: (NSString*)URI
{
  NSMutableArray *results = [NSMutableArray arrayWithCapacity: 10];
  xmlNodePtr cur = NULL;
  const xmlChar *href = XMLSTRING(URI);
  const xmlChar *xmlName = XMLSTRING(localName);
  xmlNsPtr parentNS = xmlSearchNsByHref(internal->node.node->doc, internal->node.node, href);

  for (cur = internal->node.node->children; cur != NULL; cur = cur->next)
    {
      if (cur->type == XML_ELEMENT_NODE)
        {
          if (xmlStrcmp(xmlName, cur->name) == 0)
            {
              xmlNsPtr childNS = parentNS;

              if (cur->nsDef != NULL)
                {
                  childNS = xmlSearchNsByHref(internal->node.node->doc, cur, href);
                }

              
              if (((childNS != NULL) && 
                   ((cur->ns == childNS) ||
                    ((cur->ns == NULL) &&
                     (xmlStrcmp(childNS->prefix, (const xmlChar*)"") == 0)))) ||
                  ((cur->ns != NULL) && (xmlStrcmp(cur->ns->href, href) == 0)))
                {
                  NSXMLNode *theNode = [NSXMLNode _objectForNode: cur];
                  [results addObject: theNode];
                }
            }
        }
    }
  
  return results;
}

- (void) addAttribute: (NSXMLNode*)attribute
{
  xmlNodePtr theNode = internal->node.node;
  xmlAttrPtr attr = (xmlAttrPtr)[attribute _node];
  xmlAttrPtr oldAttr;

  if (nil != [attribute parent])
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"Tried to add attribute to multiple parents."];
    }

  if (attr->ns != NULL)
    {
      xmlNsPtr ns = attr->ns;
      xmlDocPtr tmp = attr->doc;
      BOOL resolved = NO;
    
      if (ns->href == NULL)
        {
          xmlNsPtr newNs = xmlSearchNs(theNode->doc, theNode, ns->prefix);
          
          if (newNs != NULL)
            {
              ns = newNs;
              attr->ns = ns;
              resolved = YES;
            }
        }
      else //if (ns->prefix == NULL)
        {
          xmlNsPtr newNs = xmlSearchNsByHref(theNode->doc, theNode, ns->href);
          
          if (newNs != NULL)
            {
              ns = newNs;
              attr->ns = ns;
              resolved = YES;
            }
        }
      
      if (!resolved && (tmp != NULL))
        {
          xmlNsPtr cur = tmp->oldNs;
          xmlNsPtr last = NULL;
          xmlNsPtr oldNs1;

          // Need to transfer the namespace to the new tree
          // Unlink in old
          while (cur)
            {
              if (cur == ns)
                {
                  if (last == NULL)
                    {
                      tmp->oldNs = NULL;
                    }
                  else
                    {
                      last->next = cur->next;
                    }
                  cur->next = NULL;
                  break;
                }
              last = cur;
              cur = cur->next;
            }

          // Insert in new
          ensure_oldNs(theNode);
          oldNs1 = theNode->doc->oldNs;
          while (oldNs1)
            {
              if (oldNs1->next == NULL)
                {
                  oldNs1->next = cur;
                  break;
                }
              oldNs1 = oldNs1->next;
            }
        }

#if LIBXML_VERSION >= 20620
      xmlDOMWrapAdoptNode(NULL, attr->doc, (xmlNodePtr)attr, 
                          theNode->doc, theNode, 0);
#else
      xmlSetTreeDoc((xmlNodePtr)attr, theNode->doc);
#endif
      xmlFreeDoc(tmp);

      oldAttr = xmlHasNsProp(theNode, attr->name, ns->href);
    }
  else
    {
      oldAttr = xmlHasProp(theNode, attr->name);
    }

  if (NULL != oldAttr)
    {
      /*
       * As per Cocoa documentation, we only add the attribute if it's not
       * already set. xmlHasProp() also looks at the DTD for default attributes
       * and we need  to make sure that we only bail out here on #FIXED
       * attributes.
       */

      // Do not replace plain attributes.
      if (XML_ATTRIBUTE_NODE == oldAttr->type)
	{
	  return;
	}
      else if (XML_ATTRIBUTE_DECL == oldAttr->type)
	{
	  // If the attribute is from a DTD, do not replace it if it's #FIXED
	  xmlAttributePtr attrDecl = (xmlAttributePtr)oldAttr;
	  if (XML_ATTRIBUTE_FIXED == attrDecl->def)
	    {
	      return;
	    }
	}
    }
  xmlAddChild(theNode, (xmlNodePtr)attr);
  [self _addSubNode: attribute];
}

- (void) removeAttributeForName: (NSString*)name
{
  NSXMLNode *attrNode = [self attributeForName: name];

  [attrNode detach];
}

- (void) setAttributes: (NSArray*)attributes
{
  NSEnumerator	*enumerator = [attributes objectEnumerator];
  NSXMLNode	*attribute;

  // FIXME: Remove all previous attributes
  while ((attribute = [enumerator nextObject]) != nil)
    {
      [self addAttribute: attribute];
    }
}

- (void) setAttributesAsDictionary: (NSDictionary*)attributes
{
  [self setAttributesWithDictionary: attributes];
}

- (void) setAttributesWithDictionary: (NSDictionary*)attributes
{
  NSEnumerator	*en = [attributes keyEnumerator];
  NSString	*key;

  // FIXME: Remove all previous attributes
  while ((key = [en nextObject]) != nil)
    {
      NSString	*val = [[attributes objectForKey: key] stringValue];
      NSXMLNode	*attribute = [NSXMLNode attributeWithName: key
					      stringValue: val];
      [self addAttribute: attribute];
    }
}

- (NSArray*) attributes
{
  NSMutableArray *attributes = [NSMutableArray array];
  xmlNodePtr theNode = internal->node.node;
  xmlAttrPtr attributeNode = theNode->properties;

  while (attributeNode)
    {
      NSXMLNode *attribute;

      attribute = [NSXMLNode _objectForNode: (xmlNodePtr)attributeNode];
      [attributes addObject: attribute];
      attributeNode = attributeNode->next;
    }
  return attributes;
}

- (NSXMLNode*) attributeForName: (NSString*)name
{
  NSString *prefix = [[self class] prefixForName: name];

  if ((nil != prefix) && [prefix length] > 0)
    {
      NSXMLNode *ns = [self namespaceForPrefix: prefix];

      if (nil != ns)
        {
          NSString *localName = [[self class] localNameForName: name];

          // Namespace nodes have the URI as their stringValue
          return [self attributeForLocalName: localName URI: [ns stringValue]];
        }
    }

  {
    NSXMLNode *result = nil;
    xmlNodePtr theNode = internal->node.node;
    xmlAttrPtr attributeNode = xmlHasProp(theNode, XMLSTRING(name));
    
    if (NULL != attributeNode)
      {
        result = [NSXMLNode _objectForNode: (xmlNodePtr)attributeNode];
      }
    
    return result;
  }
}

- (NSXMLNode*) attributeForLocalName: (NSString*)localName
                                 URI: (NSString*)URI
{
  NSXMLNode *result = nil;
  xmlNodePtr theNode = internal->node.node;
  xmlAttrPtr attributeNode = xmlHasNsProp(theNode, XMLSTRING(localName),
                                          XMLSTRING(URI));
  
  if (NULL != attributeNode)
    {
      result = [NSXMLNode _objectForNode: (xmlNodePtr)attributeNode];
    }
    
    return result;
}

- (void) addNamespace: (NSXMLNode*)aNamespace
{
  xmlNsPtr ns = xmlCopyNamespace((xmlNsPtr)[aNamespace _node]);
  xmlNodePtr theNode = internal->node.node;
  const xmlChar *prefix = ns->prefix;

  if (theNode->nsDef == NULL)
    {
      theNode->nsDef = ns;
    }
  else
    {
      xmlNsPtr cur = theNode->nsDef;
      xmlNsPtr last = NULL;
      
      while (cur != NULL)
        {
          if ((prefix != NULL) &&
              (cur->prefix != NULL) &&
              (xmlStrcmp(prefix, cur->prefix) == 0))
            {
              break;
            }
          if (cur->next == NULL)
            {
              cur->next = ns;
              return;
            }
          last = cur;
	  cur = cur->next;
	}

      // Found the same prefix
      if (cur->href == NULL)
        {
          // This was a fake namespace we added
          if (theNode->ns == cur)
            {
              theNode->ns = ns;
            }
          if (last == NULL)
            {
              theNode->nsDef = ns;
            }
          else
            {
              last->next = ns;
            }
          ns->next = cur->next;
          cur->next = NULL;
        }
    }

  // Are we setting a default namespace?
  if ((theNode->ns == NULL) && (xmlStrcmp(prefix, (const xmlChar*)"") == 0))
    {
      theNode->ns = ns;
    }

  // Need to replace fake namespaces in subnodes
  cleanup_namespaces(theNode, ns);
}

- (void) removeNamespaceForPrefix: (NSString*)name
{
  xmlNodePtr theNode = internal->node.node;

  if (theNode->nsDef != NULL)
    {
      xmlNsPtr cur = theNode->nsDef;
      xmlNsPtr last = NULL;
      const xmlChar *prefix = XMLSTRING(name);
      
      while (cur != NULL)
        {
          if ((cur->prefix != NULL) && 
              (xmlStrcmp(prefix, cur->prefix) == 0))
            {
              if (last == NULL)
                {
                  internal->node.node->nsDef = cur->next;
                }
              else
                {
                  last->next = cur->next;
                }
              cur->next = NULL;
              if (theNode->ns == cur)
                {
                  theNode->ns = NULL;
                }
              xmlFreeNs(cur);
              return;
            }
          last = cur;
	  cur = cur->next;
	}
    }
}

- (void) setNamespaces: (NSArray*)namespaces
{
  NSEnumerator *en = [namespaces objectEnumerator];
  NSXMLNode *namespace = nil;

  // Remove old namespaces
  xmlFreeNsList(internal->node.node->nsDef);
  internal->node.node->nsDef = NULL;

  // Add new ones
  while ((namespace = (NSXMLNode *)[en nextObject]) != nil)
    {
      [self addNamespace: namespace];
    }
}

- (NSArray*) namespaces
{
  // FIXME: Should we use xmlGetNsList()?
  NSMutableArray *result = nil;
  xmlNsPtr ns = internal->node.node->nsDef;

  if (ns)
    {
      xmlNsPtr cur = NULL;

      result = [NSMutableArray array];
      for (cur = ns; cur != NULL; cur = cur->next)
	{
	  [result addObject: [NSXMLNode _objectForNode:
                                          (xmlNodePtr)xmlCopyNamespace(cur)]];
	}
    }

  return result;
}

- (NSXMLNode*) namespaceForPrefix: (NSString*)name
{
  if (name != nil)
    {
      const xmlChar *prefix = XMLSTRING(name);
      xmlNodePtr theNode = internal->node.node;
      xmlNsPtr ns;
      
      ns = xmlSearchNs(theNode->doc, theNode, prefix);
      if ((ns == NULL) && ([name length] == 0))
        {
          prefix = NULL;
          ns = xmlSearchNs(theNode->doc, theNode, prefix);
        }

      if (ns != NULL)
        {
          return [NSXMLNode _objectForNode: (xmlNodePtr)xmlCopyNamespace(ns)];
        }
    }

  return nil;
}

- (NSXMLNode*) resolveNamespaceForName: (NSString*)name
{
  NSString *prefix = [[self class] prefixForName: name];

  // Return the default namespace for an empty prefix
  if (nil != prefix)
    {
      return [self namespaceForPrefix: prefix];
    }

  return nil;
}

- (NSString*) resolvePrefixForNamespaceURI: (NSString*)namespaceURI
{
  const xmlChar *uri = XMLSTRING(namespaceURI);
  xmlNsPtr ns = xmlSearchNsByHref(internal->node.node->doc, internal->node.node, uri);

  if (ns)
    {
      return StringFromXMLStringPtr(ns->prefix);
    }

  return nil;
}

- (void) insertChild: (NSXMLNode*)child atIndex: (NSUInteger)index
{
  NSXMLNodeKind	theKind = [child kind];
  NSUInteger childCount = [self childCount];

  // Check to make sure this is a valid addition...
  NSAssert(nil != child, NSInvalidArgumentException);
  NSAssert(index <= childCount, NSInvalidArgumentException);
  NSAssert(NSXMLAttributeKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLDTDKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLDocumentKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLElementDeclarationKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLEntityDeclarationKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLInvalidKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLNamespaceKind != theKind, NSInvalidArgumentException);
  NSAssert(NSXMLNotationDeclarationKind != theKind, NSInvalidArgumentException);

/* On OSX we get NSInternalInconsistencyException if we try to add an element
 * which is already a child of some other parent.  So presumably we shouldn't
 * be auto-removing...
 * 
 *  if (nil != [child parent])
 *    {
 *      [child detach];
 *    }
 */
  NSAssert(nil == [child parent], NSInternalInconsistencyException);

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

- (void) setChildren: (NSArray*)children
{
  NSUInteger count = [self childCount];

  while (count-- > 0)
    {
      [self removeChildAtIndex: count];
    }

  [self insertChildren: children atIndex: 0];
}

- (void) addChild: (NSXMLNode*)child
{
  [self insertChild: child atIndex: [self childCount]];
}

- (void) replaceChildAtIndex: (NSUInteger)index withNode: (NSXMLNode*)theNode
{
  [self insertChild: theNode atIndex: index];
  [self removeChildAtIndex: index + 1];
}

static void
joinTextNodes(xmlNodePtr nodeA, xmlNodePtr nodeB, NSMutableArray *nodesToDelete)
{
  NSXMLNode *objA = (nodeA->_private);
  NSXMLNode *objB = (nodeB->_private);

  xmlTextMerge(nodeA, nodeB); // merge nodeB into nodeA

  if (objA != nil) // objA gets the merged node
    {
      if (objB != nil) // objB is now invalid
	{
	  /* set it to be invalid and make sure it's not
	   * pointing to a freed node
	   */
	  [objB _invalidate];
	  [nodesToDelete addObject: objB];
	}
    }
  else if (objB != nil) // there is no objA -- objB gets the merged node
    {
      [objB _setNode: nodeA]; // nodeA is the remaining (merged) node
    }
}

- (void) normalizeAdjacentTextNodesPreservingCDATA: (BOOL)preserve
{
  NSEnumerator *subEnum = [internal->subNodes objectEnumerator];
  NSXMLNode *subNode = nil;
  NSMutableArray *nodesToDelete = [NSMutableArray array];

  while ((subNode = [subEnum nextObject]))
    {
      xmlNodePtr theNode = [subNode _node];
      xmlNodePtr prev = theNode->prev;
      xmlNodePtr next = theNode->next;

      if (theNode->type == XML_ELEMENT_NODE)
	{
	  [(NSXMLElement *)subNode
	    normalizeAdjacentTextNodesPreservingCDATA:preserve];
	}
      else if (theNode->type == XML_TEXT_NODE
	|| (theNode->type == XML_CDATA_SECTION_NODE && !preserve))
	{
	  if (next && (next->type == XML_TEXT_NODE
	    || (next->type == XML_CDATA_SECTION_NODE && !preserve)))
	    {
	      //combine node & node->next
	      joinTextNodes(theNode, theNode->next, nodesToDelete);
	    }
	  if (prev && (prev->type == XML_TEXT_NODE
	    || (prev->type == XML_CDATA_SECTION_NODE && !preserve)))
	    {
	      /* combine node->prev & node
	       * join the text of both nodes
	       * assign the joined text to the earlier of the two
	       * nodes that has an ObjC object
	       * unlink the other node
	       * delete the other node's object (maybe add it to a
	       * list of nodes to delete when we're done? --
	       * or just set its node to null, and then remove it
	       * from our subNodes when we're done iterating it)
	       * (or maybe we need to turn it into an NSInvalidNode too??)
	       */
	      joinTextNodes(theNode->prev, theNode, nodesToDelete);
	    }

	}
    }
  if ([nodesToDelete count] > 0)
    {
      subEnum = [nodesToDelete objectEnumerator];
      while ((subNode = [subEnum nextObject]))
	{
	  [self _removeSubNode: subNode];
	}
    }
}

@end

#endif	/* HAVE_LIBXML */
