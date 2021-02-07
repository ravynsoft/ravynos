/* Implementation for NSXMLNode for GNUStep
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

#define GSInternal	NSXMLNodeInternal

#import "Foundation/NSCharacterSet.h"
#import "NSXMLPrivate.h"
#import "GSInternal.h"
GS_PRIVATE_INTERNAL(NSXMLNode)

void
cleanup_namespaces(xmlNodePtr node, xmlNsPtr ns)
{
  if ((node == NULL) || (ns == NULL))
    return;

  if ((node->type == XML_ATTRIBUTE_NODE) ||
      (node->type == XML_ELEMENT_NODE))
    {
      xmlNsPtr ns1 = node->ns;
      
      if (ns1 == ns)
        {
          return;
        }

      // Either both the same or one NULL and the other the same
      if (ns1 != NULL &&
          (((ns1->href == NULL) &&
            (xmlStrcmp(ns1->prefix, ns->prefix) == 0)) ||
           /*
           ((ns1->prefix == NULL) &&
            (xmlStrcmp(ns1->href, ns->href) == 0)) ||
           */
           ((xmlStrcmp(ns1->prefix, ns->prefix) == 0) &&
            (xmlStrcmp(ns1->href, ns->href) == 0))))
        {
          //xmlFreeNs(ns1);
          xmlSetNs(node, ns);
        }
 
      cleanup_namespaces(node->children, ns);
      cleanup_namespaces(node->next, ns);
      if (node->type == XML_ELEMENT_NODE)
        {
          cleanup_namespaces((xmlNodePtr)node->properties, ns);
        }
    }
}

void
ensure_oldNs(xmlNodePtr node)
{
  if (node->doc == NULL)
    {
      // Create a private document for this node
      xmlDocPtr tmp = xmlNewDoc((xmlChar *)"1.0");
      
#if LIBXML_VERSION >= 20620
      xmlDOMWrapAdoptNode(NULL, NULL, node, tmp, NULL, 0);
#else
      xmlSetTreeDoc(node, tmp);
#endif
    }
  if (node->doc->oldNs == NULL)
    {
      xmlNsPtr ns = (xmlNsPtr) xmlMalloc(sizeof(xmlNs));
      memset(ns, 0, sizeof(xmlNs));
      ns->type = XML_LOCAL_NAMESPACE;
      ns->href = xmlStrdup(XML_XML_NAMESPACE);
      ns->prefix = xmlStrdup((const xmlChar *)"xml");
      node->doc->oldNs = ns;
    }
}

static int
countAttributes(xmlNodePtr node)
{
  int count = 0;
  xmlAttrPtr attr = node->properties;

  while (attr) 
    {
      count++;
      attr = attr->next;
    }

  return count;
}

static BOOL
isEqualAttr(const xmlAttrPtr attrA, const xmlAttrPtr attrB)
{
  xmlChar	*contentA;
  xmlChar	*contentB;
  const xmlChar	*nameA;
  const xmlChar	*nameB;

  /* what has to be the same for two attributes to be equal --
   * just their values??
   */
  if (attrB == attrA)
    {
      return YES;
    }

  if (attrA == NULL || attrB == NULL)
    {
      return NO;
    }

  nameA = attrA->name;
  nameB = attrB->name;

  if (xmlStrcmp(nameA, nameB) == 0)
    {
      // get the content...
      contentA = xmlNodeGetContent((const xmlNodePtr)attrA);
      contentB = xmlNodeGetContent((const xmlNodePtr)attrB);

      if (xmlStrcmp(contentA, contentB) == 0)
	{
          xmlFree(contentA);
          xmlFree(contentB);
	  return YES;
	}
      xmlFree(contentA);
      xmlFree(contentB);
      return NO;
    }
  
  return NO;
}

static xmlAttrPtr
findAttrWithName(xmlNodePtr node, const xmlChar* targetName)
{
  xmlAttrPtr attr = node->properties;

  // find an attr in node with the given name, and return it, else NULL
  while ((attr != NULL) && xmlStrcmp(attr->name, targetName) != 0) 
    {
      attr = attr->next;
    }

  return attr;
}

static BOOL
isEqualAttributes(xmlNodePtr nodeA, xmlNodePtr nodeB)
{
  xmlAttrPtr attrA = NULL;

  if (countAttributes(nodeA) != countAttributes(nodeB))
    return NO;
  
  attrA = nodeA->properties;
  while (attrA)
    {
      xmlAttrPtr attrB = findAttrWithName(nodeB, attrA->name);
      if (!isEqualAttr(attrA, attrB))
	{
	  return NO;
	}
      attrA = attrA->next;
    }

  return YES;
}

static BOOL
isEqualNode(xmlNodePtr nodeA, xmlNodePtr nodeB)
{
  if (nodeA == nodeB)
    return YES;

  if (nodeA->type != nodeB->type)
    return NO;
  
  if (nodeA->type == XML_NAMESPACE_DECL) 
    {
      xmlNsPtr nsA = (xmlNsPtr)nodeA;
      xmlNsPtr nsB = (xmlNsPtr)nodeB;
      
      if (xmlStrcmp(nsA->href, nsB->href) != 0)
	{
	  return NO;
	}
      if (xmlStrcmp(nsA->prefix, nsB->prefix) != 0)
	{
	  return NO;
	}
      
      return YES;
    }

  if (xmlStrcmp(nodeA->name, nodeB->name) != 0)
    return NO;
  
  if (nodeA->type == XML_ELEMENT_NODE) 
    {
      xmlChar *contentA = NULL;
      xmlChar *contentB = NULL;

      if (!isEqualAttributes(nodeA, nodeB))
	{
	  return NO;
	}

      // Get the value of any text node underneath the current element.
      contentA = xmlNodeGetContent((const xmlNodePtr)nodeA);
      contentB = xmlNodeGetContent((const xmlNodePtr)nodeB);
      if (xmlStrcmp(contentA, contentB) != 0)
	{
          xmlFree(contentA);
          xmlFree(contentB);
	  return NO;
	}
      xmlFree(contentA);
      xmlFree(contentB);
    }
  if (1) 
    {
      xmlChar *contentA = NULL;
      xmlChar *contentB = NULL;

      // FIXME: Handle more node types
      if (!isEqualAttributes(nodeA, nodeB))
	{
	  return NO;
	}

      // Get the value of any text node underneath the current element.
      contentA = xmlNodeGetContent((const xmlNodePtr)nodeA);
      contentB = xmlNodeGetContent((const xmlNodePtr)nodeB);
      if (xmlStrcmp(contentA, contentB) != 0)
	{
          xmlFree(contentA);
          xmlFree(contentB);
	  return NO;
	}
      xmlFree(contentA);
      xmlFree(contentB);
    }
  
  return YES;
}

static BOOL
isEqualTree(xmlNodePtr nodeA, xmlNodePtr nodeB)
{
  xmlNodePtr childA;
  xmlNodePtr childB;

  if (nodeA == nodeB)
    {
      return YES;
    }
  
  if (nodeA == NULL || nodeB == NULL)
    {
      return NO;
    }
  
  if (!isEqualNode(nodeA, nodeB))
    {
      return NO;
    }
  
  if (nodeA->type == XML_NAMESPACE_DECL) 
    {
      return YES;
    }

  // Check children
  childA = nodeA->children;
  childB = nodeB->children;
  while (isEqualTree(childA, childB))
    {
      if (childA == NULL)
        {
          return YES;
        }
      else
        {
          childA = childA->next;
          childB = childB->next;
        }
    }
  
  return NO;
}

/* FIXME ... the libxml2 data structure representing a namespace has a
 * completely different layout from that of almost all other nodes, so
 * the generic xmlNode code won't work and we need to check the type
 * in every method we use!
 */
@implementation NSXMLNode (Private)
- (void *) _node
{
  return internal->node.node;
}

- (void) _setNode: (void *)_anode
{
  DESTROY(internal->subNodes);
  internal->node.node = _anode;
  if (internal->node.node != NULL)
    {
      if (internal->node.node->type == XML_NAMESPACE_DECL)
        {
          ((xmlNsPtr)(internal->node.node))->_private = self;
        }
      else
        {
          internal->node.node->_private = self;
        }
    }
}

+ (NSXMLNode *) _objectForNode: (xmlNodePtr)node
{
  NSXMLNode *result = nil;
  
  if (node)
    {
      if (node->type == XML_NAMESPACE_DECL)
        {
          result = ((xmlNs *)node)->_private;
        }
      else
        {
          result = node->_private;
        }
      if (result == nil)
	{
          Class cls;
          NSXMLNodeKind kind;
          xmlElementType type = node->type;
          xmlDoc *docNode;
          NSXMLDocument *doc = nil;
          
	  switch (type)
	    {
	      case XML_DOCUMENT_NODE:
              case XML_HTML_DOCUMENT_NODE:
		cls = [NSXMLDocument class];
		kind = NSXMLDocumentKind;
		break;
	      case XML_ELEMENT_NODE: 
		cls = [NSXMLElement class];
		kind = NSXMLElementKind;
		break;
	      case XML_DTD_NODE:
		cls = [NSXMLDTD class];
		kind = NSXMLDTDKind;
		break;
	      case XML_ATTRIBUTE_DECL: 
		cls = [NSXMLDTDNode class];
		kind = NSXMLAttributeDeclarationKind;
		break;
	      case XML_ELEMENT_DECL: 
		cls = [NSXMLDTDNode class];
		kind = NSXMLElementDeclarationKind;
		break;
	      case XML_ENTITY_DECL: 
		cls = [NSXMLDTDNode class];
		kind = NSXMLEntityDeclarationKind;
		break;
	      case XML_NOTATION_NODE: 
		cls = [NSXMLDTDNode class];
		kind = NSXMLNotationDeclarationKind;
		break;
	      case XML_ATTRIBUTE_NODE: 
		cls = [NSXMLNode class];
		kind = NSXMLAttributeKind;
		break;
	      case XML_CDATA_SECTION_NODE:
		cls = [NSXMLNode class];
		kind = NSXMLTextKind;
                // FIXME: Should set option
		break;
	      case XML_COMMENT_NODE: 
		cls = [NSXMLNode class];
		kind = NSXMLCommentKind;
		break;
	      case XML_NAMESPACE_DECL: 
		cls = [NSXMLNode class];
		kind = NSXMLNamespaceKind;
		break;
	      case XML_PI_NODE: 
		cls = [NSXMLNode class];
		kind = NSXMLProcessingInstructionKind;
		break;
	      case XML_TEXT_NODE: 
		cls = [NSXMLNode class];
		kind = NSXMLTextKind;
		break;
	      default: 
		NSLog(@"ERROR: _objectForNode: called with a node of type %d",
		  type);
		return nil;
		break;
	    }
          if (node->type == XML_NAMESPACE_DECL)
            {
              docNode = NULL;
            }
          else
            {
              docNode = node->doc;
            }

          if ((docNode != NULL) && ((xmlNodePtr)docNode != node) &&
              (NULL != docNode->children))
            {
              doc = (NSXMLDocument*)[self _objectForNode: (xmlNodePtr)docNode];
              if (doc != nil)
                {
                  cls = [[doc class] replacementClassForClass: cls];
                }
            }

          result = [[cls alloc] _initWithNode: node kind: kind];
	  AUTORELEASE(result);
          if (node->type == XML_NAMESPACE_DECL)
            {
              [doc _addSubNode: result];
            }
          else
            {
              if (node->parent)
                {
                  NSXMLNode *parent = [self _objectForNode: node->parent];
                  [parent _addSubNode: result];
                }
            }
	}
    }
  
  return result;
}

- (void) _addSubNode: (NSXMLNode *)subNode
{
  if (!internal->subNodes)
    internal->subNodes = [[NSMutableArray alloc] init];
  if ([internal->subNodes indexOfObjectIdenticalTo: subNode] == NSNotFound)
    {
      [internal->subNodes addObject: subNode];
    }
}

- (void) _removeSubNode: (NSXMLNode *)subNode
{
  // retain temporarily so we can safely remove from our subNodes list first
  AUTORELEASE(RETAIN(subNode));
  [internal->subNodes removeObjectIdenticalTo: subNode];
}

- (void) _createInternal
{
  GS_CREATE_INTERNAL(NSXMLNode);
}

- (id) _initWithNode: (xmlNodePtr)theNode kind: (NSXMLNodeKind)theKind
{
  if ((self = [super init]))
    {
      [self _createInternal];
      [self _setNode: theNode];
      internal->kind = theKind;
    }
  return self;
}

- (xmlNodePtr) _childNodeAtIndex: (NSUInteger)index
{
  NSUInteger count = 0;
  xmlNodePtr theNode = internal->node.node;
  xmlNodePtr children;

  if ((theNode->type == XML_NAMESPACE_DECL) ||
      (theNode->type == XML_ATTRIBUTE_NODE))
    {
      return NULL;
    }

  children = theNode->children;
  if (!children)
    return NULL; // the Cocoa docs say it returns nil if there are no children

  while (children != NULL && count++ < index)
    {
      children = children->next;
    }

  if (count < index)
    [NSException raise: NSRangeException format: @"child index too large"];

  return children;
}

- (void) _insertChild: (NSXMLNode*)child atIndex: (NSUInteger)index
{
  /* this private method provides the common insertion
   * implementation used by NSXMLElement and NSXMLDocument
   */
  
  // Get all of the nodes...
  xmlNodePtr parentNode = internal->node.node; // we are the parent
  xmlNodePtr childNode = (xmlNodePtr)[child _node];
  xmlNodePtr curNode = [self _childNodeAtIndex: index];
  BOOL mergeTextNodes = NO; // is there a defined option for this?

  if (childNode->type == XML_NAMESPACE_DECL)
    {
      // FIXME
      return;
    }
  else 
    {
      xmlDocPtr tmp = childNode->doc;

      if (tmp)
        {
          // Try to resolve half defined namespaces
          xmlNsPtr ns = tmp->oldNs;
          xmlNsPtr last = NULL;

          ensure_oldNs(parentNode);

          while (ns != NULL)
            {
              BOOL resolved = NO;

              if (ns->href == NULL)
                {
                  xmlNsPtr ns1;

                  ns1 = xmlSearchNs(parentNode->doc, parentNode, ns->prefix);
                  if (ns1 != NULL)
                    {
                      cleanup_namespaces(childNode, ns1);
                      resolved = YES;
                    }
                }
              /*
              else if (ns->prefix == NULL)
                {
                  xmlNsPtr ns1;

                  ns1
                    = xmlSearchNsByHref(parentNode->doc, parentNode, ns->href);
                  if (ns1 != NULL)
                    {
                      cleanup_namespaces(childNode, ns1);
                      resolved = YES;
                    }
                }
              */
              if (!resolved)
                {
                  xmlNsPtr cur = ns;
                  xmlNsPtr oldNs1;

                  // Need to transfer the namespace to the new tree
                  // Unlink in old
                  if (last == NULL)
                    {
                      tmp->oldNs = NULL;
                    }
                  else
                    {
                      last->next = ns->next;
                    }

                  ns = ns->next;
                  cur->next = NULL;
                  
                  // Insert in new
                  oldNs1 = parentNode->doc->oldNs;
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
              else
                {
                  last = ns;
                  ns = ns->next;
                }
            }

#if LIBXML_VERSION >= 20620
          xmlDOMWrapAdoptNode(NULL, childNode->doc, childNode, 
                              parentNode->doc, parentNode, 0);
#else
          xmlSetTreeDoc(childNode, parentNode->doc);
#endif
          xmlFreeDoc(tmp);
        }
    }

  if (mergeTextNodes
    || ((childNode->type != XML_TEXT_NODE)
      && (parentNode->type != XML_TEXT_NODE)))
    {
      // this uses the built-in libxml functions which merge adjacent text nodes
      xmlNodePtr addedNode = NULL;

      if (curNode == NULL)
        {
          addedNode = xmlAddChild(parentNode, childNode);
        }
      else
        {
          addedNode = xmlAddPrevSibling(curNode, childNode);
        }
      if (addedNode != childNode)
        {
          if (addedNode != NULL)
            {
              // The node was freed while merging
              [child _invalidate];
            }
          // Don't add as subnode
          return;
        }
    }
  else
    {
      /* here we avoid merging adjacent text nodes by linking
       * the new node in "by hand"
       */
      childNode->parent = parentNode;
      xmlSetTreeDoc(childNode, parentNode->doc);

      if (curNode)
	{
	  // insert childNode before an existing node curNode
	  xmlNodePtr prevNode = curNode->prev;
	  curNode->prev = childNode;
	  childNode->next = curNode;
	  if (prevNode)
	    {
	      childNode->prev = prevNode;
	      prevNode->next = childNode;
	    }
	  else
	    {
	      /* in this case, this is the new "first child",
	       * so update our parent to point to it
	       */
	      parentNode->children = childNode;
	    }
	}
      else
	{
	  // not inserting before an existing node... add as new "last child"
	  xmlNodePtr formerLastChild = parentNode->last;
	  if (formerLastChild)
	    {
	      formerLastChild->next = childNode;
	      childNode->prev = formerLastChild;
	      parentNode->last = childNode;
	    }
	  else
	    {
	      // no former children -- this is the first
	      parentNode->children = childNode;
	      parentNode->last = childNode;
	    }
	}
    }

  [self _addSubNode: child];
}

- (void) _invalidate
{
  internal->kind = NSXMLInvalidKind;
  [self _setNode: NULL];
}

@end

static void
clearPrivatePointers(xmlNodePtr aNode)
{
  if (!aNode)
    return;

  if (aNode->type == XML_NAMESPACE_DECL)
    {
      xmlNsPtr ns = (xmlNsPtr)aNode;

      ns->_private = NULL;
      clearPrivatePointers((xmlNodePtr)(ns->next));
      return;
    }

  aNode->_private = NULL;
  clearPrivatePointers(aNode->children);
  clearPrivatePointers(aNode->next);
  if (aNode->type == XML_ELEMENT_NODE)
    {
      clearPrivatePointers((xmlNodePtr)(aNode->properties));
      clearPrivatePointers((xmlNodePtr)(aNode->nsDef));
    }
  if (aNode->type == XML_ELEMENT_DECL)
    {
      xmlElementPtr elem = (xmlElementPtr)aNode;
      clearPrivatePointers((xmlNodePtr)(elem->attributes));
    }
  if (aNode->type == XML_DOCUMENT_NODE)
    {
      xmlDocPtr doc = (xmlDocPtr)aNode;
      clearPrivatePointers((xmlNodePtr)(doc->intSubset));
    }
  // FIXME: Handle more node types
}

static NSArray *
execute_xpath(xmlNodePtr node, NSString *xpath_exp, NSDictionary *constants,
              BOOL nodesOnly, NSError **error)
{
  xmlDocPtr doc = node->doc;
  NSMutableArray *result = nil;
  const xmlChar* xpathExpr = XMLSTRING(xpath_exp); 
  xmlXPathContextPtr xpathCtx =  NULL; 
  xmlXPathObjectPtr xpathObj = NULL; 
  xmlNodePtr rootNode = NULL;

  if (error != NULL)
    {
      *error = NULL;
    }

  if (doc == NULL)
    {
      // FIXME: Create temporary document
      return nil;
    }

  assert(xpathExpr);
  
  /* Create xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if (!xpathCtx) 
    {
      NSLog(@"Error: unable to create new XPath context.");
      return nil;
    }
    
  // provide a context for relative paths
  xpathCtx->node = node;

  /* Register namespaces from root node (if any) */
  rootNode = xmlDocGetRootElement(doc);
  if (rootNode != NULL)
    {
      xmlNsPtr ns = rootNode->nsDef;

      while (ns != NULL)
        {
          xmlXPathRegisterNs(xpathCtx, ns->prefix, ns->href);
          ns = ns->next;
        }
    }

  // Add constants
  if (constants != nil)
    {
      NSEnumerator *keyEnum = [constants keyEnumerator];
      NSString *key;

      while ((key = [keyEnum nextObject]) != nil)
        {
          id obj = [constants objectForKey: key];
          const xmlChar *name = XMLSTRING(key);
          
          // FIXME: Add more conversions
          if ([obj isKindOfClass: [NSString class]])
            {
              xmlXPathObjectPtr value = xmlXPathNewString(XMLSTRING(obj));
              
              xmlXPathRegisterVariable(xpathCtx, name, value);
            }
        }
    }

  /* Evaluate xpath expression */
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if (xpathObj == NULL) 
    {
      NSLog(@"Error: unable to evaluate xpath expression \"%s\"", xpathExpr);
      xmlXPathFreeContext(xpathCtx);
      return nil;
    }
  
  /* results */
  if (xpathObj->type == XPATH_NODESET)
    {
      xmlNodeSetPtr nodeset = NULL;

      nodeset = xpathObj->nodesetval;
      if (!xmlXPathNodeSetIsEmpty(nodeset))
        {
          int i = 0; 

          /* Collect results */
          result = [NSMutableArray arrayWithCapacity: nodeset->nodeNr];
          for (i = 0; i < nodeset->nodeNr; i++)
            {
              id obj = nil;
              xmlNodePtr cur = NULL;

              cur = nodeset->nodeTab[i];
              obj = [NSXMLNode _objectForNode: cur];
              if (obj)
                {
                  [result addObject: obj];
                }
            } 
        }
    }
  else if (!nodesOnly)
    {
      switch (xpathObj->type)
        {
        case XPATH_STRING:
          result = [NSMutableArray arrayWithObject:
                                     StringFromXMLStringPtr(xpathObj->stringval)];
          break;
          // FIXME: Handle other types
        default:
          break;
        }
    }

  /* Cleanup */
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx); 

  return result;
}

@implementation NSXMLNode

+ (void) initialize
{
  if (self == [NSXMLNode class])
    {
      xmlCheckVersion(LIBXML_VERSION);

      // Protect against libxml2 not being correctly set up on Windows.
      // See: http://www.linuxquestions.org/questions/programming-9/%5Bsolved%5Dusing-libxml2-on-mingw-xmlfree-crashes-839802/
      if (!xmlFree)
        {
          xmlMemGet(&xmlFree, &xmlMalloc, &xmlRealloc, NULL);
        }

      // Switch off default
      xmlKeepBlanksDefault(0);
    }
}

+ (id) attributeWithName: (NSString*)name
	     stringValue: (NSString*)stringValue
{
  NSXMLNode *n;

  n = [[self alloc] initWithKind: NSXMLAttributeKind];
  [n setStringValue: stringValue];
  [n setName: name];
  
  return AUTORELEASE(n);
}

+ (id) attributeWithName: (NSString*)name
		     URI: (NSString*)URI
	     stringValue: (NSString*)stringValue
{
  NSXMLNode *n;
  
  n = [[self alloc] initWithKind: NSXMLAttributeKind];
  [n setURI: URI];
  [n setStringValue: stringValue];
  [n setName: name];
  
  return AUTORELEASE(n);
}

+ (id) commentWithStringValue: (NSString*)stringValue
{
  NSXMLNode *n;

  n = [[self alloc] initWithKind: NSXMLCommentKind];
  [n setStringValue: stringValue];

  return AUTORELEASE(n);
}

+ (id) DTDNodeWithXMLString: (NSString*)string
{
  NSXMLNode *n;

  n = [[NSXMLDTDNode alloc] initWithXMLString: string];

  return AUTORELEASE(n);
}

+ (id) document
{
  NSXMLNode *n;

  n = [[NSXMLDocument alloc] initWithKind: NSXMLDocumentKind];
  return AUTORELEASE(n);
}

+ (id) documentWithRootElement: (NSXMLElement*)element
{
  NSXMLDocument	*d;

  d = [[NSXMLDocument alloc] initWithRootElement: element];
  return AUTORELEASE(d);
}

+ (id) elementWithName: (NSString*)name
{
  NSXMLNode *n;

  n = [[NSXMLElement alloc] initWithName: name];
  return AUTORELEASE(n);
}

+ (id) elementWithName: (NSString*)name
	      children: (NSArray*)children
	    attributes: (NSArray*)attributes
{
  NSXMLElement *e = [self elementWithName: name];

  [e insertChildren: children atIndex: 0];
  [e setAttributes: attributes];
  return e;
}

+ (id) elementWithName: (NSString*)name
		   URI: (NSString*)URI
{
  NSXMLNode *n;

  n = [[NSXMLElement alloc] initWithName: name URI: URI];
  return AUTORELEASE(n);
}

+ (id) elementWithName: (NSString*)name
	   stringValue: (NSString*)string
{
  NSXMLElement *e;
  
  e = [[NSXMLElement alloc] initWithName: name stringValue: string];
  return AUTORELEASE(e);
}

+ (NSString*) localNameForName: (NSString*)name
{
  const xmlChar *xmlName = XMLSTRING(name); 
  xmlChar *prefix = NULL;
  xmlChar *localName;
  NSString *result = name;

  if (NULL == xmlName)
    return nil;

  localName = xmlSplitQName2(xmlName, &prefix);
  if (NULL != localName)
    {
      result = StringFromXMLStringPtr(localName);
      xmlFree(localName);
      xmlFree(prefix);
    }

  return result;
}

+ (id) namespaceWithName: (NSString*)name
	     stringValue: (NSString*)stringValue
{
  NSXMLNode *n;

  n = [[self alloc] initWithKind: NSXMLNamespaceKind];
  [n setName: name];
  [n setStringValue: stringValue];
  return AUTORELEASE(n);
}

+ (NSXMLNode*) predefinedNamespaceForPrefix: (NSString*)name
{
  // FIXME: We should cache these instances
  if ([name isEqualToString: @"xml"])
    {
      return [self namespaceWithName: @"xml"
        stringValue: @"http: //www.w3.org/XML/1998/namespace"];
    }
  if ([name isEqualToString: @"xs"])
    {
      return [self namespaceWithName: @"xs"
        stringValue: @"http: //www.w3.org/2001/XMLSchema"];
    }
  if ([name isEqualToString: @"xsi"])
    {
      return [self namespaceWithName: @"xsi"
        stringValue: @"http: //www.w3.org/2001/XMLSchema-instance"];
    }
  if ([name isEqualToString: @"fn"])
    {
      return [self namespaceWithName: @"fn"
        stringValue: @"http: //www.w3.org/2003/11/xpath-functions"];
    }
  if ([name isEqualToString: @"local"])
    {
      return [self namespaceWithName: @"local"
        stringValue: @"http: //www.w3.org/2003/11/xpath-local-functions"];
    }
  
  return nil;
}

+ (NSString*) prefixForName: (NSString*)name
{
  const xmlChar *xmlName = XMLSTRING(name); 
  xmlChar *localName;
  xmlChar *prefix = NULL;
  NSString *result = @"";

  if (NULL == xmlName)
    {
      return result;
    }

  localName = xmlSplitQName2(xmlName, &prefix);
  if (NULL != prefix)
    {
      result = StringFromXMLStringPtr(prefix);
      xmlFree(localName);
      xmlFree(prefix);
    }

  return result;
}

+ (id) processingInstructionWithName: (NSString*)name
			 stringValue: (NSString*)stringValue
{
  NSXMLNode *n;

  n = [[self alloc] initWithKind: NSXMLProcessingInstructionKind];
  [n setStringValue: stringValue];
  [n setName: name];
  return AUTORELEASE(n);
}

+ (id) textWithStringValue: (NSString*)stringValue
{
  NSXMLNode *n;

  n = [[self alloc] initWithKind: NSXMLTextKind];
  [n setStringValue: stringValue];
  return AUTORELEASE(n);
}

- (NSString*) canonicalXMLStringPreservingComments: (BOOL)comments
{
  // FIXME ... generate from libxml
  return [self XMLStringWithOptions: NSXMLNodePreserveWhitespace];
}

- (NSXMLNode*) childAtIndex: (NSUInteger)index
{
  xmlNodePtr childNode = [self _childNodeAtIndex: index];
  return [NSXMLNode _objectForNode: childNode];
}

- (NSUInteger) childCount
{
  NSUInteger count = 0;
  xmlNodePtr children = NULL;
  xmlNodePtr theNode = internal->node.node;

  if (!theNode)
    {
      return 0;
    }

  if ((theNode->type == XML_NAMESPACE_DECL) ||
      (theNode->type == XML_ATTRIBUTE_NODE))
    {
      return 0;
    }

  for (children = theNode->children; children; children = children->next)
    {
      count++;
    }

  return count;
}

- (NSArray*) children
{
  NSMutableArray *childrenArray = nil;

  if (NSXMLInvalidKind == internal->kind)
    {
      return nil;
    }
  else
    {
      xmlNodePtr children = NULL;
      xmlNodePtr theNode = internal->node.node;
      
      if ((theNode == NULL) ||
          (theNode->type == XML_NAMESPACE_DECL) ||
          (theNode->type == XML_ATTRIBUTE_NODE) ||
          (theNode->children == NULL))
	{
	  return nil;
	}

      childrenArray = [NSMutableArray array];
      for (children = theNode->children; children; children = children->next)
	{
	  NSXMLNode *n = [NSXMLNode _objectForNode: children];
	  [childrenArray addObject: n];
	}
    }
  return childrenArray;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSXMLNode *c = [[self class] allocWithZone: zone];
  xmlNodePtr newNode = xmlCopyNode([self _node], 1); // make a deep copy
  clearPrivatePointers(newNode);

  c = [c _initWithNode: newNode kind: internal->kind];

  GSIVar(c, options) = internal->options;
  if (nil != internal->objectValue)
    {
      /*
        Only copy the objectValue when externally set.
        The problem here are nodes created by parsing XML.
        There the stringValue may be set, but the objectValue isn't.
        This should rather be solved by creating a suitable objectValue,
        when the node gets instantiated.
      */
      [c setObjectValue: internal->objectValue];
    }
  [c setURI: [self URI]];
//  [c setName: [self name]];
//  [c setStringValue: [self stringValue]];

  return c;
}

- (NSString*) description
{
/* OSX simply uses the XML string value of a node as its description.
  return [NSString stringWithFormat:@"<%@ %@ %d>%@",
    NSStringFromClass([self class]),
    [self name], [self kind], [self XMLString]];
*/
  return [self XMLString];
}

- (void) dealloc
{
  if (GS_EXISTS_INTERNAL)
    {
      xmlNodePtr theNode = internal->node.node;
      NSArray *theSubNodes = [internal->subNodes copy];
      NSEnumerator *enumerator = [theSubNodes objectEnumerator];
      NSXMLNode *subNode;

      while ((subNode = [enumerator nextObject]) != nil)
        {
          [subNode detach];
        }
      RELEASE(theSubNodes);

      RELEASE(internal->objectValue);
      RELEASE(internal->subNodes);
      if (theNode)
        {
          if (theNode->type == XML_NAMESPACE_DECL)
            {
              ((xmlNsPtr)theNode)->_private = NULL;
              // FIXME: Not sure when to free the node here,
              // the same namespace node might be referenced
              // from other places.
              xmlFreeNode(theNode);
            }
          else
            {
              theNode->_private = NULL;
              if (theNode->parent == NULL)
                {
                  // the top level node frees the entire tree
                  if (theNode->type == XML_DOCUMENT_NODE)
                    {
                      xmlFreeDoc((xmlDocPtr)theNode);
                    }
                  else if (theNode->type == XML_ENTITY_DECL && 
                           ((xmlEntityPtr)theNode)->etype == XML_INTERNAL_PREDEFINED_ENTITY)
                    {
                      // Don't free internal entity nodes
                    }
                  else
                    {
                      xmlDocPtr tmp = theNode->doc;

                      xmlFreeNode(theNode);
                      // Free the private document we allocated in detach
                      if (tmp)
                        {
                          xmlFreeDoc(tmp);
                        }
                    }
                }
            }
        }
      GS_DESTROY_INTERNAL(NSXMLNode);
    }
  [super dealloc];
}

- (void) detach
{
  xmlNodePtr theNode = internal->node.node;

  if (theNode)
    {
      NSXMLNode *parent = [self parent];

      if (theNode->type == XML_NAMESPACE_DECL)
        {
          // FIXME
        }
      else
        {
          if (theNode->doc)
            {
              /* Create a private document and move the node over.
               * This is needed so that the strings of the nodes subtree
               * get stored in the dictionary of this new document.
               */
              // FIXME: Should flag this doc so it wont get returned in
              // the method rootDocument
              xmlDocPtr tmp = xmlNewDoc((xmlChar *)"1.0");

#if LIBXML_VERSION >= 20620
              xmlDOMWrapAdoptNode(NULL, theNode->doc, theNode, tmp, NULL, 0);
#else
              xmlSetTreeDoc(theNode, tmp);
#endif
            }
          else
            {
              // separate our node from its parent and siblings
              xmlUnlinkNode(theNode);
            }
        }

      if (parent)
	{
	  [parent _removeSubNode: self];
	}
    }
}

- (NSUInteger) hash
{
  return [[self name] hash];
}

- (NSUInteger) index
{
  xmlNodePtr theNode = internal->node.node;
  int count = 0;

  if (theNode->type == XML_NAMESPACE_DECL)
    {
      return 0;
    }

  while ((theNode = theNode->prev))
    {
      count++; // count our earlier sibling nodes
    }

  return count;
}

- (id) init
{
  return [self initWithKind: NSXMLInvalidKind];
}

- (id) initWithKind: (NSXMLNodeKind)theKind
{
  return [self initWithKind: theKind options: 0];
}

- (id) initWithKind: (NSXMLNodeKind)theKind options: (NSUInteger)theOptions
{
  Class theSubclass = [NSXMLNode class];
  void *theNode = NULL;

  /*
   * We find the correct subclass for specific node kinds: 
   */
  switch (theKind)
    {
      case NSXMLDocumentKind: 
	theSubclass = [NSXMLDocument class];
	break;
	  
      case NSXMLInvalidKind: 
	theSubclass = [NSXMLNode class];
	break; 

      case NSXMLElementKind: 
	theSubclass = [NSXMLElement class];
	break;
	
      case NSXMLDTDKind: 
	theSubclass = [NSXMLDTD class];
	break;
	
      case NSXMLElementDeclarationKind: 
      case NSXMLEntityDeclarationKind: 
      case NSXMLNotationDeclarationKind: 
	theSubclass = [NSXMLDTDNode class];
	break;

      case NSXMLNamespaceKind: 
	theSubclass = [NSXMLNode class];
	break;

      case NSXMLAttributeKind: 
      case NSXMLCommentKind: 
      case NSXMLProcessingInstructionKind: 
      case NSXMLTextKind: 
	break;

      case NSXMLAttributeDeclarationKind: 
	RELEASE(self);
	return nil;
	
      default: 
	theKind = NSXMLInvalidKind;
	theSubclass = [NSXMLNode class];
	break;
    }

  /*
   * Check whether we are already initializing an instance of the given
   * subclass. If we are not, release ourselves and allocate a subclass
   * instance instead.
   */
  if (NO == [self isKindOfClass: theSubclass])
    {
      RELEASE(self);
      return [[theSubclass alloc] initWithKind: theKind
				       options: theOptions];
    }

  /* If we are initializing for the correct class, we can actually perform
   * initializations: 
   */
  switch (theKind)
    {
      case NSXMLDocumentKind: 
	theNode = xmlNewDoc((xmlChar *)"1.0");
	break;
	  
      case NSXMLInvalidKind: 
      case NSXMLElementKind: 
	theNode = xmlNewNode(NULL,(xmlChar *)"");
	break;
	
      case NSXMLDTDKind: 
	theNode = xmlNewDtd(NULL, (xmlChar *)"", (xmlChar *)"",(xmlChar *)"");
	break;
	
      case NSXMLElementDeclarationKind: 
        {
          xmlElementPtr ret;
          ret = (xmlElementPtr) xmlMalloc(sizeof(xmlElement));
          memset(ret, 0, sizeof(xmlElement));
          ret->type = XML_ELEMENT_DECL;
          ret->name = xmlStrdup((xmlChar *)"");
          theNode = ret;
          break;
        }

      case NSXMLEntityDeclarationKind: 
#if LIBXML_VERSION >= 20700
        theNode = xmlNewEntity(NULL, (xmlChar *)"", 0, (xmlChar *)"", 
                               (xmlChar *)"", (xmlChar *)"");
#else
        {
          xmlEntityPtr ret;
          ret = (xmlEntityPtr) xmlMalloc(sizeof(xmlEntity));
          memset(ret, 0, sizeof(xmlEntity));
          ret->type = XML_ENTITY_DECL;
          ret->name = xmlStrdup((xmlChar *)"");
          ret->ExternalID = xmlStrdup((xmlChar *)"");
          ret->SystemID = xmlStrdup((xmlChar *)"");
          ret->content = xmlStrdup((xmlChar *)"");
          theNode = ret;
        }
#endif
        break;

      case NSXMLNotationDeclarationKind:
        // FIXME
	theNode = xmlNewNode(NULL, (xmlChar *)"");
	break;

      case NSXMLProcessingInstructionKind: 
	theNode = xmlNewPI((xmlChar *)"", (xmlChar *)"");
	break;

      case NSXMLCommentKind: 
	theNode = xmlNewComment((xmlChar *)"");
	break;

      case NSXMLTextKind: 
	theNode = xmlNewText((xmlChar *)"");
	break;

      case NSXMLNamespaceKind: 
	theNode = xmlNewNs(NULL,(xmlChar *)"",(xmlChar *)"");
	break;

      case NSXMLAttributeKind: 
	theNode = xmlNewProp(NULL,(xmlChar *)"",(xmlChar *)"");
	break;

      default: 
	break;
    }

  if (nil == (self = [self _initWithNode: theNode kind: theKind]))
    {
      return nil;
    }

  internal->options = theOptions;
  return self;
}

- (BOOL) isEqual: (id)other
{
  if ([self kind] != [other kind])
    {
      return NO;
    }
  /*
  NSLog(@"self %@ other %@", self, other);
  NSLog(@"s sV '%@' oV '%@', other sV '%@' oV '%@'", [self stringValue], [self objectValue],
        [other stringValue], [other objectValue]);
  */
  return isEqualTree(internal->node.node, (xmlNodePtr)[other _node]);
}

- (NSXMLNodeKind) kind
{
  return internal->kind;
}

- (NSUInteger) level
{
  NSXMLNode *parent = [self parent];
  
  if (nil == parent)
    {
      return 0;
    }
  else
    {
      return [parent level] + 1;
    }
}

- (NSString*) localName
{
  xmlNodePtr theNode = internal->node.node;

  if (NSXMLInvalidKind == internal->kind)
    {
      return nil;
    }

  if (theNode->type == XML_NAMESPACE_DECL)
    {
      return StringFromXMLStringPtr(((xmlNs *)theNode)->prefix);
    }
  else
    {
      return StringFromXMLStringPtr(theNode->name);
    }
}

- (NSString*) name
{
  NSString *localName = [self localName];

  if (nil != localName)
    {
      NSString *prefix = [self prefix];

      if ((nil != prefix) && [prefix length] > 0)
        {
          return [NSString stringWithFormat: @"%@:%@", prefix, localName];
        }
    }

  return localName;
}

- (NSXMLNode*) _nodeFollowingInNaturalDirection: (BOOL)forward
{
  NSXMLNode *ancestor = self;
  NSXMLNode *candidate = nil;
  NSXMLNodeKind theKind;

  if (forward)
    {
      /* Node walking is a depth-first thingy. Hence, we consider children first: */
      if (0 != [self childCount])
        {
          candidate = [[self children] objectAtIndex: 0];
        }
    }

  /* If there are no children, we move on to siblings: */
  /* If there are no siblings left for the receiver, we recurse down to the root
   * of the tree until we find an ancestor with further siblings: */
  while ((nil == candidate) && (nil != ancestor))
    {
      if (forward)
	{
	  candidate = [ancestor nextSibling];
	}
      else
	{
	  candidate = [ancestor previousSibling];
	}
      ancestor = [ancestor parent];
    }

  /* No children, no next siblings, no next siblings for any ancestor: We are
   * the last node */
  if (nil == candidate)
    {
      return nil;
    }

  if (!forward)
    {
      /* Node walking is a depth-first thingy. Hence, we consider children first: */
      while (0 != [candidate childCount])
        {
          NSUInteger theIndex = [candidate childCount] - 1;
          candidate = [[candidate children] objectAtIndex: theIndex];
        }
    }

  /* Sanity check: Namespace and attribute nodes are skipped: */
  theKind = [candidate kind];
  if ((NSXMLAttributeKind == theKind) || (NSXMLNamespaceKind == theKind))
    {
      return [candidate _nodeFollowingInNaturalDirection: forward];
    }
  return candidate;
}

- (NSXMLNode*) nextNode
{
  return [self _nodeFollowingInNaturalDirection: YES];
}

- (NSXMLNode*) nextSibling
{
  xmlNodePtr theNode = internal->node.node;

  if (NULL == theNode)
    {
      return nil;
    }
  if (XML_NAMESPACE_DECL == theNode->type)
    {
      return nil;
    }
  return [NSXMLNode _objectForNode: theNode->next];
}

- (id) objectValue
{
  return internal->objectValue;
}

- (NSXMLNode*) parent
{
  xmlNodePtr parent = NULL;
  xmlNodePtr theNode = internal->node.node;

  if (NULL == theNode)
    {
      return nil;
    }
  if (XML_NAMESPACE_DECL == theNode->type)
    {
      return nil;
    }

  parent = theNode->parent;
  return [NSXMLNode _objectForNode: parent];
}

- (NSString*) prefix
{
  xmlNodePtr theNode = internal->node.node;

  if (NULL == theNode)
    {
      return nil;
    }
  if (XML_NAMESPACE_DECL == theNode->type)
    {
      return @"";
    }
  if (XML_ELEMENT_NODE != theNode->type)
    {
      return @"";
    }
  if (theNode->ns == NULL)
    {
      return @"";
    }

  return StringFromXMLStringPtr(theNode->ns->prefix);
}

- (NSXMLNode*) previousNode
{
  return [self _nodeFollowingInNaturalDirection: NO];
}

- (NSXMLNode*) previousSibling
{
  xmlNodePtr theNode = internal->node.node;

  if (NULL == theNode)
    {
      return nil;
    }
  if (XML_NAMESPACE_DECL == theNode->type)
    {
      return nil;
    }
  return [NSXMLNode _objectForNode: theNode->prev];
}

- (NSXMLDocument*) rootDocument
{
  xmlNodePtr theNode = internal->node.node;

  if (NULL == theNode)
    {
      return nil;
    }
  if (XML_NAMESPACE_DECL == theNode->type)
    {
      return nil;
    }
  if ((NULL == theNode->doc) || (NULL == theNode->doc->children))
    {
      // This is a standalone node, we still may have a private document,
      // but we don't want to return this.
      return nil;
    }

  return
    (NSXMLDocument *)[NSXMLNode _objectForNode: (xmlNodePtr)(theNode->doc)];
}

- (NSString*) stringValue
{
  xmlNodePtr theNode = internal->node.node;
  xmlChar *content = xmlNodeGetContent(theNode);
  NSString *result = nil;

  if (NULL != content)
    {
      result = StringFromXMLStringPtr(content);
      xmlFree(content);
    }
  else
    {
      result = @"";
    }

  return result;
}

- (void) setObjectValue: (id)value
{
  NSString *stringValue;

  // FIXME: Use correct formatter here
  stringValue = [value description];
  [self setStringValue: stringValue];

  ASSIGN(internal->objectValue, value);
}

- (void) setName: (NSString *)name
{
  xmlNodePtr theNode = internal->node.node;

  if (NSXMLInvalidKind == internal->kind)
    {
      return;
    }
  
  if (theNode->type == XML_NAMESPACE_DECL)
    {
      xmlNsPtr ns = (xmlNsPtr)theNode;

      if (ns->prefix != NULL)
        {
          xmlFree((xmlChar *)ns->prefix);
        }
      ns->prefix = XMLStringCopy(name);
    }
  else
    {
      const xmlChar *xmlName = XMLSTRING(name); 
      xmlChar *prefix = NULL;
      xmlChar *localName;
      
      if (NULL == xmlName)
        {
          xmlNodeSetName(theNode, (const xmlChar *)"");
          return;
        }
      
      localName = xmlSplitQName2(xmlName, &prefix);
      if (prefix != NULL)
        {
          if ((theNode->type == XML_ATTRIBUTE_NODE) ||
              (theNode->type == XML_ELEMENT_NODE))
            {
              if ((theNode->ns != NULL && theNode->ns->prefix == NULL))
                {
                  theNode->ns->prefix = xmlStrdup(prefix);
                }
              else
                {
                  xmlNsPtr ns;

                  // Set namespace
                  ns = xmlSearchNs(theNode->doc, theNode, prefix);
                  if (ns)
                    {
                      xmlSetNs(theNode, ns);
                    }
                  else
                    {
                      xmlNsPtr oldNs;

                      ensure_oldNs(theNode);

                      // Fake the name space and fix it later
                      // This function is private, so re reimplemt it.
                      //ns = xmlDOMWrapStoreNs(node->doc, NULL, prefix);
                      oldNs = theNode->doc->oldNs;
                      while (oldNs)
                        {
                          if (oldNs->prefix != NULL && xmlStrEqual(oldNs->prefix, prefix))
                            {
                              ns = oldNs;
                              break;
                            }
                          if (oldNs->next == NULL)
                            {
                              ns = xmlNewNs(NULL, NULL, prefix);
                              oldNs->next = ns;
                              break;
                            }
                          oldNs = oldNs->next;
                        }
                      xmlSetNs(theNode, ns);
                    }
                }
            }
          
          xmlNodeSetName(theNode, localName);
          xmlFree(localName);
          xmlFree(prefix);
        }
      else
        {
          xmlNodeSetName(theNode, xmlName);
        }
    }
}

- (void) setStringValue: (NSString*)string
{
  [self setStringValue: string resolvingEntities: NO];
}

- (void) setStringValue: (NSString*)string resolvingEntities: (BOOL)resolve
{
  xmlNodePtr theNode = internal->node.node;

  if (theNode->type == XML_NAMESPACE_DECL)
    {
      xmlNsPtr ns = (xmlNsPtr)theNode;
      if (ns->href != NULL)
        {
          xmlFree((xmlChar *)ns->href);
        }
      ns->href = XMLStringCopy(string);
    }
  else
    {
      // Remove all child nodes except attributes
      if (internal->subNodes)
        {
          NSArray *theSubNodes = [internal->subNodes copy];
          NSEnumerator *enumerator = [theSubNodes objectEnumerator];
          NSXMLNode *subNode;
          
          while ((subNode = [enumerator nextObject]) != nil)
            {
              if ([subNode kind] != NSXMLAttributeKind)
                {
                  [subNode detach];
                }
            }
          RELEASE(theSubNodes);
        }

      if (resolve == NO)
        {
          xmlNodeSetContent(theNode, XMLSTRING(string));
        }
      else
        {
          // need to actually resolve entities...
          // is this the right functionality?? xmlEncodeSpecialChars()
          xmlChar *newstr = xmlEncodeEntitiesReentrant(theNode->doc, XMLSTRING(string));
          xmlNodeSetContent(theNode, newstr);
          xmlMemFree(newstr);
        }
    }
  ASSIGN(internal->objectValue, string);
}

- (void) setURI: (NSString*)URI
{
  xmlNodePtr theNode = internal->node.node;

  if (NSXMLInvalidKind == internal->kind)
    {
      return;
    }
  if ((theNode->type == XML_ATTRIBUTE_NODE) ||
      (theNode->type == XML_ELEMENT_NODE))
    {
      const xmlChar *uri = XMLSTRING(URI);
      xmlNsPtr ns;
      
      if (uri == NULL)
        {
          theNode->ns = NULL;
          return;
        }

      ns = xmlSearchNsByHref(theNode->doc, theNode, uri);
      if (ns == NULL)
        {
          if ((theNode->ns != NULL && theNode->ns->href == NULL))
            {
              theNode->ns->href = xmlStrdup(uri);
              return;
            }
          /*
          if (theNode->type == XML_ELEMENT_NODE)
            {
              //ns = xmlNewNs(theNode, uri, (const xmlChar *)"");
              ns = xmlNewNs(theNode, uri, NULL);
            }
          else
          */
            {
              xmlNsPtr oldNs;
              
              ensure_oldNs(theNode);
              
              // Fake the name space and fix it later
              // This function is private, so re reimplemt it.
              //ns = xmlDOMWrapStoreNs(node->doc, NULL, prefix);
              oldNs = theNode->doc->oldNs;
              while (oldNs)
                {
                  if (oldNs->href != NULL && xmlStrEqual(oldNs->href, uri))
                    {
                      ns = oldNs;
                      break;
                    }
                  if (oldNs->next == NULL)
                  {
                    ns = xmlNewNs(NULL, uri, NULL);
                    oldNs->next = ns;
                    break;
                  }
                  oldNs = oldNs->next;
                }
            }
        }
      
      xmlSetNs(theNode, ns);
    }
}

- (NSString*) URI
{
  xmlNodePtr theNode = internal->node.node;

  if (NSXMLInvalidKind == internal->kind)
    {
      return nil;
    }
  if ((theNode->type == XML_ATTRIBUTE_NODE) ||
      (theNode->type == XML_ELEMENT_NODE))
    {
      xmlNsPtr ns = theNode->ns;
      
      if ((ns != NULL) && (ns->href != NULL))
        {
          return StringFromXMLStringPtr(ns->href);
        }
    }

  return nil;
}

- (NSString*) XMLString
{
  return [self XMLStringWithOptions: NSXMLNodeOptionsNone];
}

- (NSString*) XMLStringWithOptions: (NSUInteger)theOptions
{
  NSString     *string = nil;
  xmlBufferPtr buffer;
  int error = 0;
  int xmlOptions = 0;

  buffer = xmlBufferCreate();
  if (buffer == NULL)
    {
      // FIXME: xmlGetLastError()
      return nil;
    }

  // XML_SAVE_XHTML XML_SAVE_AS_HTML XML_SAVE_NO_DECL XML_SAVE_NO_XHTML
#if LIBXML_VERSION >= 20702
  xmlOptions |= XML_SAVE_AS_XML;
#endif
#if LIBXML_VERSION >= 20708
  if (theOptions & NSXMLNodePreserveWhitespace)
    {
      xmlOptions |= XML_SAVE_WSNONSIG;
    }
#endif
#if LIBXML_VERSION >= 20622
  //NSXMLNodeExpandEmptyElement is the default
  if ((theOptions & NSXMLNodeCompactEmptyElement) == 0)
    {
      xmlOptions |= XML_SAVE_NO_EMPTY;
    }
#endif
#if LIBXML_VERSION >= 20617
  if (theOptions & NSXMLNodePrettyPrint)
    {
      xmlOptions |= XML_SAVE_FORMAT;
    }
#endif
  
#if LIBXML_VERSION >= 20623
  {
    xmlSaveCtxtPtr ctxt;

    ctxt = xmlSaveToBuffer(buffer, "utf-8", xmlOptions);
    xmlSaveTree(ctxt, internal->node.node);
    error = xmlSaveClose(ctxt);
  }
#else
  {
    xmlDocPtr doc = NULL;

    if (internal->node.node->type != XML_NAMESPACE_DECL)
      {
        doc = internal->node.node->doc;
      }
    error = xmlNodeDump(buffer, doc, internal->node.node, 1, 1);
  }
#endif
  if (-1 == error)
    {
      xmlBufferFree(buffer);
      return nil;
    }

  string = StringFromXMLString(buffer->content, buffer->use);
  xmlBufferFree(buffer);

  if ([self kind] == NSXMLTextKind)
    {
      return string;
    }
  else
    {
      return [string stringByTrimmingCharactersInSet: 
                       [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    }
}

- (NSString*) XPath
{
  xmlNodePtr theNode = internal->node.node;
  return StringFromXMLStringPtr(xmlGetNodePath(theNode));
}

- (NSArray*) nodesForXPath: (NSString*)anxpath error: (NSError**)error
{
  xmlNodePtr theNode = internal->node.node;

  if (NSXMLInvalidKind == internal->kind)
    {
      return nil;
    }
  if (theNode->type == XML_NAMESPACE_DECL)
    {
      return nil;
    }

  return execute_xpath(theNode, anxpath, nil, YES, error);
}

 - (NSArray*) objectsForXQuery: (NSString*)xquery
		     constants: (NSDictionary*)constants
		         error: (NSError**)error
{
  return [self notImplemented: _cmd];
}

- (NSArray*) objectsForXQuery: (NSString*)xquery error: (NSError**)error
{
  return [self objectsForXQuery: xquery
                      constants: nil
                          error: error];
}
@end

#else

#import "Foundation/NSException.h"
#import "Foundation/NSXMLNode.h"

@implementation NSXMLNode

+ (id) attributeWithName: (NSString*)name
	     stringValue: (NSString*)stringValue
{
  return nil;
}

+ (id) attributeWithName: (NSString*)name
		     URI: (NSString*)URI
	     stringValue: (NSString*)stringValue
{
  return nil;
}

+ (id) commentWithStringValue: (NSString*)stringValue
{
  return nil;
}

+ (id) DTDNodeWithXMLString: (NSString*)string
{
  return nil;
}

+ (id) document
{
  return nil;
}

+ (id) documentWithRootElement: (NSXMLElement*)element
{
  return nil;
}

+ (id) elementWithName: (NSString*)name
{
  return nil;
}

+ (id) elementWithName: (NSString*)name
	      children: (NSArray*)children
	    attributes: (NSArray*)attributes
{
  return nil;
}

+ (id) elementWithName: (NSString*)name
		   URI: (NSString*)URI
{
  return nil;
}

+ (id) elementWithName: (NSString*)name
	   stringValue: (NSString*)string
{
  return nil;
}

+ (NSString*) localNameForName: (NSString*)name
{
  return nil;
}

+ (id) namespaceWithName: (NSString*)name
	     stringValue: (NSString*)stringValue
{
  return nil;
}

+ (NSXMLNode*) predefinedNamespaceForPrefix: (NSString*)name
{
  return nil;
}

+ (NSString*) prefixForName: (NSString*)name
{
  return nil;
}

+ (id) processingInstructionWithName: (NSString*)name
			 stringValue: (NSString*)stringValue
{
  return nil;
}

+ (id) textWithStringValue: (NSString*)stringValue
{
  return nil;
}

- (NSString*) canonicalXMLStringPreservingComments: (BOOL)comments
{
  return nil;
}

- (NSXMLNode*) childAtIndex: (NSUInteger)index
{
  return nil;
}

- (NSUInteger) childCount
{
  return 0;
}

- (NSArray*) children
{
  return nil;
}

- (id) copyWithZone: (NSZone*)zone
{
  return nil;
}

- (void) detach
{
}

- (NSUInteger) index
{
  return 0;
}

- (id) init
{
  NSString      *className = NSStringFromClass([self class]);

  DESTROY(self);
  [NSException raise: NSGenericException
              format: @"%@ - no libxml2 at configure time", className];
  return nil;
}

- (id) initWithKind: (NSXMLNodeKind)theKind
{
  return [self init];
}

- (id) initWithKind: (NSXMLNodeKind)theKind options: (NSUInteger)theOptions
{
  return [self init];
}

- (NSXMLNodeKind) kind
{
  return 0;
}

- (NSUInteger) level
{
  return 0;
}

- (NSString*) localName
{
  return nil;
}

- (NSString*) name
{
  return nil;
}

- (NSXMLNode*) nextNode
{
  return nil;
}

- (NSXMLNode*) nextSibling
{
  return nil;
}

- (id) objectValue
{
  return nil;
}

- (NSXMLNode*) parent
{
  return nil;
}

- (NSString*) prefix
{
  return nil;
}

- (NSXMLNode*) previousNode
{
  return nil;
}

- (NSXMLNode*) previousSibling
{
  return nil;
}

- (NSXMLDocument*) rootDocument
{
  return nil;
}

- (NSString*) stringValue
{
  return nil;
}

- (void) setObjectValue: (id)value
{
}

- (void) setName: (NSString *)name
{
}

- (void) setStringValue: (NSString*)string
{
}

- (void) setStringValue: (NSString*)string resolvingEntities: (BOOL)resolve
{
}

- (void) setURI: (NSString*)URI
{
}

- (NSString*) URI
{
  return nil;
}

- (NSString*) XMLString
{
  return nil;
}

- (NSString*) XMLStringWithOptions: (NSUInteger)theOptions
{
  return nil;
}

- (NSString*) XPath
{
  return nil;
}

- (NSArray*) nodesForXPath: (NSString*)anxpath error: (NSError**)error
{
  return nil;
}

 - (NSArray*) objectsForXQuery: (NSString*)xquery
		     constants: (NSDictionary*)constants
		         error: (NSError**)error
{
  return nil;
}

- (NSArray*) objectsForXQuery: (NSString*)xquery error: (NSError**)error
{
  return nil;
}
@end

#endif	/* HAVE_LIBXML */
