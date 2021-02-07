/** Implementation for GSXML classes

   Copyright (C) 2000,2001,2002 Free Software Foundation, Inc.

   Written by: Michael Pakhantsov  <mishel@berest.dp.ua> on behalf of
   Brainstorm computer solutions.
   Date: Jule 2000

   Integration/updates/rewrites by: Richard Frith-Macdonald <rfm@gnu.org>
   Date: Sep2000,Dec2001/Jan2002

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

   <title>The XML and HTML parsing system</title>
   <chapter>
      <heading>The XML and HTML parsing system</heading>
      <p>
        The GNUstep XML parser is a collection Objective-C classes
        wrapping the C XML parsing library (libxml).
      </p>
      <p>
        The underlying C library handles high performance parsing, while
	the ObjectiveC classes provide ease of use/integration.
      </p>
   </chapter>
*/

#import "common.h"
#import "GNUstepBase/Unicode.h"

#ifndef	_XOPEN_SOURCE
#define	_XOPEN_SOURCE 600
#endif
#include <stdio.h>


#ifdef	HAVE_LIBXML

#import "GNUstepBase/GSObjCRuntime.h"
#import "GNUstepBase/NSDebug+GNUstepBase.h"
#import "GNUstepBase/NSObject+GNUstepBase.h"
#import "GNUstepBase/GSMime.h"
#import "GNUstepBase/GSXML.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSCalendarDate.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSFileHandle.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSString.h"
#import "Foundation/NSTimer.h"
#import "Foundation/NSTimeZone.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSURLHandle.h"
#import "Foundation/NSValue.h"

/* Avoid problems on systems where the xml headers use 'id'
 */
#define	id	GSXMLID

/* libxml headers */
#include <libxml/tree.h>
#include <libxml/entities.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/catalog.h>
#include <libxml/SAX2.h>
#include <libxml/HTMLparser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#ifdef HAVE_LIBXSLT
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif /* HAVE_LIBXSLT */

#undef	id


/*
 * optimization
 *
 */
static Class NSString_class;
static Class treeClass;
static id (*usImp)(id, SEL, const unsigned char*);
static SEL usSel;

static xmlExternalEntityLoader  originalLoader = NULL;

/*
 * Macro to cast results to correct type for libxml2
 */
#define	UTF8STRING(X)	((const unsigned char*)[X UTF8String])

inline static NSString*
UTF8Str(const unsigned char *bytes)
{
  // stringWithUTF8String: raises on OSX if you feed it with a NULL string.
  if (NULL == bytes) {
    return nil;
  }
  return (*usImp)(NSString_class, usSel, bytes);
}

inline static NSString*
UTF8StrLen(const unsigned char *bytes, unsigned length)
{
  NSString	*str;

  str = [[NSString_class alloc] initWithBytes: bytes
				       length: length
				     encoding: NSUTF8StringEncoding];
  return AUTORELEASE(str);
}

static BOOL cacheDone = NO;

#ifdef NeXT_Foundation_LIBRARY
@interface NSObject (MissingFromMacOSX)
+ (IMP) methodForSelector: (SEL)aSelector;
@end
#endif

static char * xml_strdup(const char *from)
{
  unsigned	len;
  char		*to;

  if (0 == from) from = "";
  len = strlen(from) + 1;
  to = malloc(len);
  memcpy(to, from, len);
  return to;
}

static void
setupCache()
{
  if (cacheDone == NO)
    {
      cacheDone = YES;
      xmlMemSetup(free, malloc, realloc, xml_strdup);
      xmlInitializeCatalog();
      xmlDefaultSAXHandlerInit();
      NSString_class = [NSString class];
      usSel = @selector(stringWithUTF8String:);
      usImp = (id (*)(id, SEL, const unsigned char*))
	[NSString_class methodForSelector: usSel];
      treeClass = [GSTreeSAXHandler class];
    }
}

static int xmlNSInputStreamReadCallback(void *context, char *buffer, int len)
{
  NSInputStream *stream = (NSInputStream *)context;
  return [stream read: (uint8_t *)buffer maxLength: len];
}

static int xmlNSInputStreamCloseCallback (void *context)
{
  NSInputStream *stream = (NSInputStream *)context;
  [stream close];
  return 0;
}

static xmlParserInputPtr
loadEntityFunction(const unsigned char *url, const unsigned char *eid,
  void *ctx);
static xmlParserInputPtr
resolveEntityFunction(void *ctx, const unsigned char *eid,
  const unsigned char *url);
static xmlEntityPtr
getEntityIgnoreExternal(void *ctx, const xmlChar *name);
static xmlEntityPtr
getEntityResolveExternal(void *ctx, const xmlChar *name);

@interface GSXPathObject(Private)
+ (id) _newWithNativePointer: (xmlXPathObject *)lib
                     context: (GSXPathContext *)context;
@end

@interface GSXMLDocument (GSPrivate)
- (id) _initFrom: (void*)data parent: (id)p ownsLib: (BOOL)f;
@end

@interface GSXMLNamespace (GSPrivate)
- (id) _initFrom: (void*)data parent: (id)p;
@end

@interface GSXMLNode (GSPrivate)
- (id) _initFrom: (void*)data parent: (id)p;
@end

@interface GSXMLParser (Private)
- (BOOL) _initLibXML;
- (NSMutableString*) _messages;
- (void) _parseChunk: (NSData*)data;
@end

@interface GSSAXHandler (Private)
- (BOOL) _initLibXML;
- (void) _setParser: (GSXMLParser*)value;
@end

/**
 * A class wrapping attributes of an XML element node.  Generally when
 * examining a GSXMLDocument, you need not concern yourself with
 * GSXMLAttribute objects as you will probably use the
 * [GSXMLNode-objectForKey:] method to return the string value of any
 * attribute you are interested in.
 */
@implementation GSXMLAttribute

static NSMapTable	*attrNames = 0;

+ (void) initialize
{
  if (self == [GSXMLAttribute class])
    {
      if (cacheDone == NO)
	setupCache();
      attrNames = NSCreateMapTable(NSIntegerMapKeyCallBacks,
	NSNonRetainedObjectMapValueCallBacks, 0);
      [[NSObject leakAt: &attrNames] release];
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_CDATA, (void*)@"XML_ATTRIBUTE_CDATA");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_ID, (void*)@"XML_ATTRIBUTE_ID");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_IDREF, (void*)@"XML_ATTRIBUTE_IDREF");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_IDREFS, (void*)@"XML_ATTRIBUTE_IDREFS");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_ENTITY, (void*)@"XML_ATTRIBUTE_ENTITY");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_ENTITIES, (void*)@"XML_ATTRIBUTE_ENTITIES");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_NMTOKEN, (void*)@"XML_ATTRIBUTE_NMTOKEN");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_NMTOKENS, (void*)@"XML_ATTRIBUTE_NMTOKENS");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_ENUMERATION, (void*)@"XML_ATTRIBUTE_ENUMERATION");
      NSMapInsert(attrNames,
	(void*)XML_ATTRIBUTE_NOTATION, (void*)@"XML_ATTRIBUTE_NOTATION");
    }
}

+ (NSInteger) typeFromDescription: (NSString*)desc
{
  NSMapEnumerator	enumerator;
  NSString		*val;
  void			*key;

  enumerator = NSEnumerateMapTable(attrNames);
  while (NSNextMapEnumeratorPair(&enumerator, &key, (void**)&val))
    {
      if ([desc isEqual: val] == YES)
	{
	  return (NSInteger)(intptr_t)key;
	}
    }
  return -1;
}

+ (NSString*) descriptionFromType: (NSInteger)type
{
  NSString	*desc = (NSString*)NSMapGet(attrNames, (void*)(intptr_t)type);

  return desc;
}

- (NSInteger) type
{
  return (NSInteger)((xmlAttrPtr)(lib))->atype;
}

- (NSString*) typeDescription
{
  NSString	*desc;

  desc = (NSString*)NSMapGet(attrNames, (void*)(intptr_t)[self type]);
  if (desc == nil)
    {
      desc = @"Unknown attribute type";
    }
  return desc;
}

/**
 * Return the next sibling node ... another GSXMLAttribute instance.
 */
- (GSXMLNode*) next
{
  if (((xmlAttrPtr)(lib))->next != NULL)
    {
      return AUTORELEASE([[GSXMLAttribute alloc]
        _initFrom: ((xmlAttrPtr)(lib))->next parent: self]);
    }
  else
    {
      return nil;
    }
}

/**
 * Return the previous sibling node ... another GSXMLAttribute instance.
 */
- (GSXMLNode*) previous
{
  if (((xmlAttrPtr)(lib))->prev != NULL)
    {
      return AUTORELEASE([[GSXMLAttribute alloc]
        _initFrom: ((xmlAttrPtr)(lib))->prev parent: self]);
    }
  else
    {
      return nil;
    }
}

/**
 * Returns the string value of the attribute.
 */
- (NSString*) value
{
  if (((xmlNodePtr)lib)->children != NULL
    && ((xmlNodePtr)lib)->children->content != NULL)
    {
      return UTF8Str(((xmlNodePtr)(lib))->children->content);
    }
  return nil;
}

@end

/**
 * A GSXML document wraps the document structure of the underlying
 * libxml library.
 */
@implementation GSXMLDocument

/**
 * Create a new document with the specified version.
 * <example>
 * id d = [GSXMLDocument documentWithVersion: @"1.0"];
 *
 * [d setRoot: [d makeNodeWithNamespace: nil name: @"plist" content: nil]];
 * [[d root] setObject: @"0.9" forKey: @"version"];
 * n1 = [[d root] makeChildWithNamespace: nil name: @"dict" content: nil];
 * [n1 makeComment: @" this is a comment "];
 * [n1 makePI: @"pi1" content: @"this is a process instruction"];
 * [n1 makeChildWithNamespace: nil name: @"key" content: @"Year Of Birth"];
 * [n1 makeChildWithNamespace: nil name: @"integer" content: @"65"];
 * [n1 makeChildWithNamespace: nil name: @"key" content: @"Pets Names"];
 * </example>
 */
+ (GSXMLDocument*) documentWithVersion: (NSString*)version
{
  void		*data = xmlNewDoc(UTF8STRING(version));
  GSXMLDocument	*document = nil;

  if (data == 0)
    {
      NSLog(@"Can't create GSXMLDocument object");
    }
  else
    {
      document = [GSXMLDocument alloc];
      document = [document _initFrom: data parent: nil ownsLib: YES];
    }
  return AUTORELEASE(document);
}

+ (void) initialize
{
  if (cacheDone == NO)
    setupCache();
}

- (id) copyWithZone: (NSZone*)z
{
  return RETAIN(self);
}

- (void) dealloc
{
  if (_ownsLib == YES && lib != NULL)
    {
      xmlFreeDoc(lib);
    }
  RELEASE(_parent);
  [super dealloc];
}

/**
 * Returns a string representation of the document (ie the XML)
 * or nil if the document does not have reasonable contents.
 */
- (NSString*) description
{
  NSString	*string = nil;
  xmlChar	*buf = NULL;
  int		length;

  xmlDocDumpFormatMemoryEnc(lib, &buf, &length, "utf-8", 1);

  if (buf != 0 && length > 0)
    {
      string = UTF8StrLen(buf, length);
      free(buf);
    }
  return string;
}

/**
 * Returns the name of the encoding for this document.
 */
- (NSString*) encoding
{
  return UTF8Str(((xmlDocPtr)(lib))->encoding);
}

- (NSUInteger) hash
{
  return (((NSUInteger)(intptr_t)lib) >> 3);
}

- (id) init
{
  NSLog(@"GSXMLDocument: calling -init is not legal");
  DESTROY(self);
  return nil;
}

/**
 * Returns a pointer to the raw libxml data used by this document.<br />
 * Only for use by libxml experts!
 */
- (void*) lib
{
  return lib;
}

/**
 * Creates a new node within the document.
 * <example>
 * GSXMLNode *n1, *n2;
 * GSXMLDocument *d;
 *
 * d = [GSXMLDocument documentWithVersion: @"1.0"];
 * [d setRoot: [d makeNodeWithNamespace: nil name: @"plist" content: nil]];
 * [[d root] setObject: @"0.9" forKey: @"version"];
 * n1 = [[d root] makeChildWithNamespace: nil name: @"dict" content: nil];
 * </example>
 */
- (GSXMLNode*) makeNodeWithNamespace: (GSXMLNamespace*)ns
				name: (NSString*)name
			     content: (NSString*)content
{
  GSXMLNode	*n = [GSXMLNode alloc];

  n = [n _initFrom:
    xmlNewDocNode(lib, [ns lib], UTF8STRING(name), UTF8STRING(content))
    parent: self];
  return AUTORELEASE(n);
}

/**
 * Returns the root node of the document.
 */
- (GSXMLNode*) root
{
  GSXMLNode	*n = [GSXMLNode alloc];

  n = [n _initFrom: xmlDocGetRootElement(lib) parent: self];
  return AUTORELEASE(n);
}

/**
 * Sets the root of the document.<br />
 * NB. The node must have been created as part of the receiving document
 * (eg. using the -makeNodeWithNamespace:name:content: method).
 */
- (GSXMLNode*) setRoot: (GSXMLNode*)node
{
  xmlNodePtr	nodeLib = (xmlNodePtr)[node lib];
  xmlNodePtr	selfLib = (xmlNodePtr)[self lib];

  if (node == nil)
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"Attempt to set root of document to nil"];
    }
  if (nodeLib->doc != selfLib->doc)
    {
      [NSException raise: NSInvalidArgumentException
      		  format: @"Attempt to set root to node from other document"];
    }
  xmlDocSetRootElement(lib, nodeLib);
  return node;
}

/**
 * Returns the version string for this document.
 */
- (NSString*) version
{
  return UTF8Str(((xmlDocPtr)(lib))->version);
}

/**
 * Uses the -description method to produce a string representation of
 * the document and writes that to filename.
 */
- (BOOL) writeToFile: (NSString*)filename atomically: (BOOL)useAuxilliaryFile
{
  NSString	*s = [self description];

  if (s == nil)
    {
      return NO;
    }
  return [s writeToFile: filename atomically: useAuxilliaryFile];
}

/**
 * Uses the -description method to produce a string representation of
 * the document and writes that to url.
 */
- (BOOL) writeToURL: (NSURL*)url atomically: (BOOL)useAuxilliaryFile
{
  NSString	*s = [self description];

  if (s == nil)
    {
      return NO;
    }
  return [s writeToURL: url atomically: useAuxilliaryFile];
}

@end

@implementation GSXMLDocument (GSPrivate)
/**
 * <init />
 * Initialise a new document object using raw libxml data.
 * The resulting document does not 'own' the data, and will not free it.
 */
- (id) _initFrom: (void*)data parent: (id)p ownsLib: (BOOL)f
{
  if (data == NULL)
    {
      NSLog(@"%@ - no data for initialization",
	NSStringFromClass([self class]));
      DESTROY(self);
      return nil;
    }
  lib = data;
  _ownsLib = f;
  ASSIGN(_parent, p);
  return self;
}
@end

/**
 * A GSXMLNamespace object wraps part of the document structure of
 * the underlying libxml library.
 */
@implementation GSXMLNamespace

static NSMapTable	*nsNames = 0;

/**
 * Return the string representation of the specified numeric type.
 */
+ (NSString*) descriptionFromType: (NSInteger)type
{
  NSString	*desc = (NSString*)NSMapGet(nsNames, (void*)(intptr_t)type);

  return desc;
}

+ (void) initialize
{
  if (self == [GSXMLNamespace class])
    {
      if (cacheDone == NO)
	setupCache();
      nsNames = NSCreateMapTable(NSIntegerMapKeyCallBacks,
	NSNonRetainedObjectMapValueCallBacks, 0);
      [[NSObject leakAt: &nsNames] release];
      NSMapInsert(nsNames,
	(void*)XML_LOCAL_NAMESPACE, (void*)@"XML_LOCAL_NAMESPACE");
    }
}

/**
 * Return the numeric constant value for the namespace
 * type named.  This method is inefficient, so the returned
 * value should be saved for re-use later.  The possible
 * values are -
 * <list>
 *   <item>XML_LOCAL_NAMESPACE</item>
 * </list>
 */
+ (NSInteger) typeFromDescription: (NSString*)desc
{
  NSMapEnumerator	enumerator;
  NSString		*val;
  void			*key;

  enumerator = NSEnumerateMapTable(nsNames);
  while (NSNextMapEnumeratorPair(&enumerator, &key, (void**)&val))
    {
      if ([desc isEqual: val] == YES)
	{
	  return (NSInteger)(intptr_t)key;
	}
    }
  return -1;
}

- (id) copyWithZone: (NSZone*)z
{
  return RETAIN(self);
}

- (void) dealloc
{
  RELEASE(_parent);
  [super dealloc];
}

- (NSUInteger) hash
{
  return (((NSUInteger)(intptr_t)lib) >> 3);
}

/**
 * Returns the namespace reference
 */
- (NSString*) href
{
  return UTF8Str(((xmlNsPtr)(lib))->href);
}

- (id) init
{
  NSLog(@"GSXMLNamespace: calling -init is not legal");
  DESTROY(self);
  return nil;
}

- (BOOL) isEqual: (id)other
{
  if ([other isKindOfClass: [self class]] == YES && [other lib] == lib)
    return YES;
  else
    return NO;
}

/**
 * Returns a pointer to the raw libxml data used by this document.<br />
 * Only for use by libxml experts!
 */
- (void*) lib
{
  return lib;
}

/**
 * return the next namespace.
 */
- (GSXMLNamespace*) next
{
  if (((xmlNsPtr)(lib))->next != NULL)
    {
      GSXMLNamespace	*ns = [GSXMLNamespace alloc];

      ns = [ns _initFrom: ((xmlNsPtr)(lib))->next parent: self];
      return AUTORELEASE(ns);
    }
  else
    {
      return nil;
    }
}

/**
 * Return the namespace prefix.
 */
- (NSString*) prefix
{
  return UTF8Str(((xmlNsPtr)(lib))->prefix);
}

/**
 * Return type of namespace
 */
- (NSInteger) type
{
  return (NSInteger)((xmlNsPtr)(lib))->type;
}

/**
 * Return string representation of the type of the namespace.
 */
- (NSString*) typeDescription
{
  NSString	*desc;

  desc = (NSString*)NSMapGet(nsNames, (void*)(intptr_t)[self type]);
  if (desc == nil)
    {
      desc = @"Unknown namespace type";
    }
  return desc;
}

@end


@implementation GSXMLNamespace (GSPrivate)
/**
 * Initialise a new namespace object using raw libxml data.
 * The resulting namespace does not 'own' the data, and will not free it.
 */
- (id) _initFrom: (void*)data parent: (id)p
{
  if (data == NULL)
    {
      NSLog(@"%@ - no data for initialization",
	NSStringFromClass([self class]));
      DESTROY(self);
      return nil;
    }
  lib = data;
  ASSIGN(_parent, p);
  return self;
}

@end

/**
 * A GSXMLNode object wraps part of the document structure of the
 * underlying libxml library.  It may have a parent, siblings, and children.
 */
@implementation GSXMLNode

static NSMapTable	*nodeNames = 0;

/**
 * Return the string constant value for the node type given.
 */
+ (NSString*) descriptionFromType: (NSInteger)type
{
  NSString	*desc = (NSString*)NSMapGet(nodeNames, (void*)(intptr_t)type);

  return desc;
}

+ (void) initialize
{
  if (self == [GSXMLNode class])
    {
      if (cacheDone == NO)
	setupCache();
      nodeNames = NSCreateMapTable(NSIntegerMapKeyCallBacks,
	NSNonRetainedObjectMapValueCallBacks, 0);
      [[NSObject leakAt: &nodeNames] release];
      NSMapInsert(nodeNames,
	(void*)XML_ELEMENT_NODE, (void*)@"XML_ELEMENT_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_ATTRIBUTE_NODE, (void*)@"XML_ATTRIBUTE_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_TEXT_NODE, (void*)@"XML_TEXT_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_CDATA_SECTION_NODE, (void*)@"XML_CDATA_SECTION_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_ENTITY_REF_NODE, (void*)@"XML_ENTITY_REF_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_ENTITY_NODE, (void*)@"XML_ENTITY_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_PI_NODE, (void*)@"XML_PI_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_COMMENT_NODE, (void*)@"XML_COMMENT_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_DOCUMENT_NODE, (void*)@"XML_DOCUMENT_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_DOCUMENT_TYPE_NODE, (void*)@"XML_DOCUMENT_TYPE_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_DOCUMENT_FRAG_NODE, (void*)@"XML_DOCUMENT_FRAG_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_NOTATION_NODE, (void*)@"XML_NOTATION_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_HTML_DOCUMENT_NODE, (void*)@"XML_HTML_DOCUMENT_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_DTD_NODE, (void*)@"XML_DTD_NODE");
      NSMapInsert(nodeNames,
	(void*)XML_ELEMENT_DECL, (void*)@"XML_ELEMENT_DECL");
      NSMapInsert(nodeNames,
	(void*)XML_ATTRIBUTE_DECL, (void*)@"XML_ATTRIBUTE_DECL");
      NSMapInsert(nodeNames,
	(void*)XML_ENTITY_DECL, (void*)@"XML_ENTITY_DECL");
    }
}

/**
 * <p>Converts a node type string to a numeric constant which can be compared
 * with the result of the -type method to determine what sort of node an
 * instance is.  Because this method is quite inefficient, you should cache
 * the numeric type returned and re-use the cached value.
 * </p>
 * The node type names are -
 * <list>
 *   <item>XML_ELEMENT_NODE</item>
 *   <item>XML_ATTRIBUTE_NODE</item>
 *   <item>XML_TEXT_NODE</item>
 *   <item>XML_CDATA_SECTION_NODE</item>
 *   <item>XML_ENTITY_REF_NODE</item>
 *   <item>XML_ENTITY_NODE</item>
 *   <item>XML_PI_NODE</item>
 *   <item>XML_COMMENT_NODE</item>
 *   <item>XML_DOCUMENT_NODE</item>
 *   <item>XML_DOCUMENT_TYPE_NODE</item>
 *   <item>XML_DOCUMENT_FRAG_NODE</item>
 *   <item>XML_NOTATION_NODE</item>
 *   <item>XML_HTML_DOCUMENT_NODE</item>
 *   <item>XML_DTD_NODE</item>
 *   <item>XML_ELEMENT_DECL</item>
 *   <item>XML_ATTRIBUTE_DECL</item>
 *   <item>XML_ENTITY_DECL</item>
 * </list>
 */
+ (NSInteger) typeFromDescription: (NSString*)desc
{
  NSMapEnumerator	enumerator;
  NSString		*val;
  void			*key;

  enumerator = NSEnumerateMapTable(nodeNames);
  while (NSNextMapEnumeratorPair(&enumerator, &key, (void**)&val))
    {
      if ([desc isEqual: val] == YES)
	{
	  return (NSInteger)(intptr_t)key;
	}
    }
  return -1;
}

/**
 * <p>
 *   Return attributes and values as a dictionary
 * </p>
 */
- (NSDictionary*) attributes
{
  xmlAttrPtr		prop;
  NSMutableDictionary	*d = [NSMutableDictionary dictionary];

  prop = ((xmlNodePtr)(lib))->properties;

  while (prop != NULL)
    {
      const void	*name = prop->name;
      NSString		*key = UTF8Str(name);
      NSString		*value = @"";
      xmlNodePtr	child = prop->children;

      while (child != NULL)
	{
	  const void	*content = child->content;

	  value = [value stringByAppendingString: UTF8Str(content)];
	  child = child->next;
	}
      [d setObject: value forKey: key];
      prop = prop->next;
  }

  return d;
}

- (id) copyWithZone: (NSZone*)z
{
  return RETAIN(self);
}

/**
 * Return node content as a string.  This should return meaningful
 * information for text nodes and for entity nodes containing only
 * text nodes.<br />
 * If entity substitution was not enabled during parsing, an
 * element containing text may actually contain both text nodes and
 * entity reference nodes, in this case you should not use this
 * method to get the content of the element, but should examine
 * the child nodes of the element individually and perform any
 * entity reference you need to do explicitly.<br />
 * NB. There are five standard entities which are automatically
 * substituted into the content text rather than appearing as
 * entity nodes in their own right.  These are '&lt;', '&gt;', '&apos;',
 * '&quot;' and '&amp;'.  If you with to receive content in which these
 * characters are represented by the original entity escapes, you need
 * to use the -escapedContent method.
 */
- (NSString*) content
{
  xmlNodePtr	ptr = (xmlNodePtr)lib;

  if (ptr == NULL)
    {
      return nil;
    }
  if (ptr->content != NULL)
    {
      return UTF8Str(ptr->content);
    }
  if ((NSInteger)ptr->type == XML_TEXT_NODE)
    {
      return @"";
    }
  else if ((NSInteger)ptr->type == XML_ELEMENT_NODE)
    {
      ptr = ptr->children;
      if (ptr != NULL)
	{
	  if (ptr->next == NULL)
	    {
	      if (ptr->content != NULL)
		{
		  return UTF8Str(ptr->content);
		}
	    }
	  else
	    {
	      NSMutableString	*m = [NSMutableString new];

	      while (ptr != NULL)
		{
		  if (ptr->content != NULL)
		    {
		      [m appendString: UTF8Str(ptr->content)];
		    }
		  ptr = ptr->next;
		}
	      return AUTORELEASE(m);
	    }
	}
    }
  return nil;
}

- (void) dealloc
{
  RELEASE(_parent);
  [super dealloc];
}

/**
 * Returns a string representation of the node and all its children
 * (ie the XML text) or nil if the node does not have reasonable contents.
 */
- (NSString*) description
{
  NSString		*string = nil;
  xmlOutputBufferPtr	buf;

  buf = xmlAllocOutputBuffer(0);
  if (buf != 0)
    {
      xmlNodeDumpOutput(buf,
	((xmlNodePtr)(lib))->doc,
	(xmlNodePtr)(lib),
	1,
	1,
	"utf-8");
      if (xmlOutputBufferFlush(buf) < 0)
	{
	  NSDebugMLog(@"Failed to flush XML description");
	}
#if LIBXML_VERSION < 20900
      string = UTF8StrLen(buf->buffer->content, buf->buffer->use);
#else
      string = UTF8StrLen(xmlBufContent(buf->buffer), xmlBufUse(buf->buffer));
#endif
      xmlOutputBufferClose(buf);
    }
  return string;
}

/**
 * Return the document in which this node exists.
 */
- (GSXMLDocument*) document
{
  if (((xmlNodePtr)(lib))->doc != NULL)
    {
      GSXMLDocument	*d = [GSXMLDocument alloc];

      d = [d _initFrom: ((xmlNodePtr)(lib))->doc parent: self ownsLib: NO];
      return AUTORELEASE(d);
    }
  else
    {
      return nil;
    }
}

/**
 * This performs the same function as the -content method, but retains
 * escaped character information (the standard five entities &amp;lt;,
 * &amp;gt;, &amp;apos;, &amp;quot;, and &amp;amp;) which are normally
 * replaced with their standard equivalents
 * (&lt;, &gt;, &apos;, &quot;, and &amp;).
 */
- (NSString*) escapedContent
{
  NSString		*str = [self content];

  return [str stringByEscapingXML];
}

/**
 * Return the first attribute in this node.
 */
- (GSXMLAttribute*) firstAttribute
{
  if (((xmlNodePtr)(lib))->properties != NULL)
    {
      return AUTORELEASE([[GSXMLAttribute alloc]
        _initFrom: ((xmlNodePtr)(lib))->properties parent: self]);
    }
  else
    {
      return nil;
    }
}

/**
 * Return the first child element of this node.  If you wish to step
 * through all children of the node (including non-element nodes)
 * you should use the -firstChild method instead.
 */
- (GSXMLNode*) firstChildElement
{
  xmlNodePtr	ptr = ((xmlNodePtr)lib)->children;

  while (ptr != NULL)
    {
      if (ptr->type == XML_ELEMENT_NODE)
	{
	  GSXMLNode	*n = [GSXMLNode alloc];

	  n = [n _initFrom: ptr parent: self];
	  return AUTORELEASE(n);
	}
      ptr = ptr->next;
    }
  return nil;
}

/**
 * Return the first child node of this node.
 * <example>
 *    - (GSXMLNode*) elementRecursive: (GSXMLNode*)node
 *    {
 *      while (node != nil)
 *        {
 *          if ([node type] == XML_ELEMENT_NODE)
 *            {
 *              return node;
 *            }
 *          if ([node firstChild] != nil)
 *            {
 *              node = [self elementRecursive: [node firstChild]];
 *            }
 *          else
 *            {
 *              node = [node next];
 *            }
 *        }
 *      return node;
 *    }
 *  </example>
 */
- (GSXMLNode*) firstChild
{
  if (((xmlNodePtr)(lib))->children != NULL)
    {
      GSXMLNode	*n = [GSXMLNode alloc];

      n = [n _initFrom: ((xmlNodePtr)(lib))->children parent: self];
      return AUTORELEASE(n);
    }
  else
    {
      return nil;
    }
}

- (NSUInteger) hash
{
  return (((NSUInteger)(intptr_t)lib) >> 3);
}

- (id) init
{
  NSLog(@"GSXMLNode: calling -init is not legal");
  DESTROY(self);
  return nil;
}

/**
 * Convenience method, equivalent to calling -type and comparing it
 * with the result of passing "XML_ELEMENT_NODE" to
 * +typeFromDescription: (but faster).
 */
- (BOOL) isElement
{
  if ((NSInteger)((xmlNodePtr)(lib))->type == XML_ELEMENT_NODE)
    {
      return YES;
    }
  return NO;
}

- (BOOL) isEqual: (id)other
{
  if ([other isKindOfClass: [self class]] == YES
    && [other lib] == lib)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/**
 * Convenience method, equivalent to calling -type and comparing it
 * with the result of passing "XML_TEXT_NODE" to
 * +typeFromDescription: (but faster).
 */
- (BOOL) isText
{
  if ((NSInteger)((xmlNodePtr)(lib))->type == XML_TEXT_NODE)
    {
      return YES;
    }
  return NO;
}

/**
 * Returns a pointer to the raw libxml data used by this document.<br />
 * Only for use by libxml experts!
 */
- (void*) lib
{
  return lib;
}

/**
 * Create and return an attribute (unless the named attribute already exists,
 * in which case we update them value of the existing attribute and return it.
 */
- (GSXMLAttribute*) makeAttributeWithName: (NSString*)name
				    value: (NSString*)value
{
  void	*l;

  l = xmlNewProp((xmlNodePtr)[self lib], UTF8STRING(name), UTF8STRING(value));
  return AUTORELEASE([[GSXMLAttribute alloc] _initFrom: l parent: self]);
}

/**
 * <p>
 *   Creation of a new child element, added at the end of
 *   parent children list.
 *   ns and content parameters are optional (may be nil).
 *   If content is non nil, a child list containing the
 *   TEXTs and ENTITY_REFs node will be created.
 *   Return previous node.
 * </p>
 * <example>
 *
 * GSXMLNode *n1, *n2;
 * GSXMLDocument *d, *d1;
 *
 * d = [GSXMLDocument documentWithVersion: @"1.0"];
 * [d setRoot: [d makeNodeWithNamespace: nil
 *                                 name: @"plist"
 *                              content: nil]];
 * [[d root] setObject: @"0.9" forKey: @"version"];
 * n1 = [[d root] makeChildWithNamespace: nil
 *                                  name: @"dict"
 *                               content: nil];
 * [n1 makeChildWithNamespace: nil name: @"key" content: @"Year Of Birth"];
 * [n1 makeChildWithNamespace: nil name: @"integer" content: @"65"];
 *
 * [n1 makeChildWithNamespace: nil name: @"key" content: @"Pets Names"];
 * [n1 makeChildWithNamespace: nil name: @"array" content: nil];
 *
 * </example>
 */
- (GSXMLNode*) makeChildWithNamespace: (GSXMLNamespace*)ns
				 name: (NSString*)name
			      content: (NSString*)content
{
  GSXMLNode	*n = [GSXMLNode alloc];

  n = [n _initFrom:
    xmlNewTextChild(lib, [ns lib], UTF8STRING(name), UTF8STRING(content))
    parent: self];
  return AUTORELEASE(n);
}

/**
 * Creation of a new text element, added at the end of
 * parent children list.
 * <example>
 * d = [GSXMLDocument documentWithVersion: @"1.0"];
 *
 * [d setRoot: [d makeNodeWithNamespace: nil name: @"plist" content: nil]];
 * [[d root] setObject: @"0.9" forKey: @"version"];
 * n1 = [[d root] makeChildWithNamespace: nil name: @"dict" content: nil];
 * [n1 makeText: @" this is a text "];
 * </example>
 */
- (GSXMLNode*) makeText: (NSString*)content
{
  GSXMLNode	*n = [GSXMLNode alloc];

  n = [n _initFrom:
    xmlAddChild((xmlNodePtr)lib, xmlNewText(UTF8STRING(content)))
    parent: self];
  return AUTORELEASE(n);
}

/**
 * Creation of a new comment element, added at the end of
 * parent children list.
 * <example>
 * d = [GSXMLDocument documentWithVersion: @"1.0"];
 *
 * [d setRoot: [d makeNodeWithNamespace: nil name: @"plist" content: nil]];
 * [[d root] setObject: @"0.9" forKey: @"version"];
 * n1 = [[d root] makeChildWithNamespace: nil name: @"dict" content: nil];
 * [n1 makeComment: @" this is a comment "];
 * </example>
 */
- (GSXMLNode*) makeComment: (NSString*)content
{
  GSXMLNode	*n = [GSXMLNode alloc];

  n = [n _initFrom:
    xmlAddChild((xmlNodePtr)lib, xmlNewComment(UTF8STRING(content)))
    parent: self];
  return AUTORELEASE(n);
}

/**
 * Create a namespace attached to this node.
 */
- (GSXMLNamespace*) makeNamespaceHref: (NSString*)href
			       prefix: (NSString*)prefix
{
  void	*data;

  data = xmlNewNs((xmlNodePtr)lib, UTF8STRING(href), UTF8STRING(prefix));
  if (data == NULL)
    {
      NSLog(@"Can't create GSXMLNamespace object");
      return nil;
    }
  return AUTORELEASE([[GSXMLNamespace alloc] _initFrom: data parent: self]);
}

/**
 * Creation of a new process instruction element,
 * added at the end of parent children list.
 * <example>
 * d = [GSXMLDocument documentWithVersion: @"1.0"];
 *
 * [d setRoot: [d makeNodeWithNamespace: nil name: @"plist" content: nil]];
 * [[d root] setObject: @"0.9" forKey: @"version"];
 * n1 = [[d root] makeChildWithNamespace: nil name: @"dict" content: nil];
 * [n1 makeComment: @" this is a comment "];
 * [n1 makePI: @"pi1" content: @"this is a process instruction"];
 * </example>
 */
- (GSXMLNode*) makePI: (NSString*)name content: (NSString*)content
{
  GSXMLNode	*n = [GSXMLNode alloc];

  n = [n _initFrom:
    xmlAddChild((xmlNodePtr)lib, xmlNewPI(UTF8STRING(name),
    UTF8STRING(content))) parent: self];
  return AUTORELEASE(n);
}

/**
 * Return the node-name
 */
- (NSString*) name
{
  if (lib != NULL && ((xmlNodePtr)lib)->name!=NULL)
    {
      return UTF8Str(((xmlNodePtr)lib)->name);
    }
  else
    {
      return nil;
    }
}

/**
 * Return the next node at this level.  This method can return any type
 * of node, and it may be more convenient to use the -nextElement node
 * if you are parsing a document where you wish to ignore non-element
 * nodes such as whitespace text separating elements.
 */
- (GSXMLNode*) next
{
  if (((xmlNodePtr)(lib))->next != NULL)
    {
      GSXMLNode	*n = [GSXMLNode alloc];

      n = [n _initFrom: ((xmlNodePtr)(lib))->next parent: _parent];
      return AUTORELEASE(n);
    }
  else
    {
      return nil;
    }
}

/**
 * Returns the next element node, skipping past any other node types
 * (such as text nodes).  If there is no element node to be returned,
 * this method returns nil.<br />
 * NB. This method is not available in java, as the method name conflicts
 * with that of java's Enumerator class.
 */
- (GSXMLNode*) nextElement
{
  xmlNodePtr	ptr = (xmlNodePtr)lib;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (ptr->type == XML_ELEMENT_NODE)
	{
	  GSXMLNode	*n = [GSXMLNode alloc];

	  n = [n _initFrom: ptr parent: _parent];
	  return AUTORELEASE(n);
	}
    }
  return nil;
}

/**
 * Return the namespace of the node.
 */
- (GSXMLNamespace*) namespace
{
  if (lib != NULL && ((xmlNodePtr)(lib))->ns != NULL)
    {
      GSXMLNamespace	*ns = [GSXMLNamespace alloc];

      ns = [ns _initFrom: ((xmlNodePtr)(lib))->ns parent: self];
      return AUTORELEASE(ns);
    }
  else
    {
      return nil;
    }
}

/**
 * Return namespace definitions for the node
 */
- (GSXMLNamespace*) namespaceDefinitions
{
  if (lib != NULL && ((xmlNodePtr)lib)->nsDef != NULL)
    {
      GSXMLNamespace	*ns = [GSXMLNamespace alloc];

      ns = [ns _initFrom: ((xmlNodePtr)(lib))->nsDef parent: self];
      return AUTORELEASE(ns);
    }
  else
    {
      return nil;
    }
}

/**
 * Return the attribute value for the specified key.
 */
- (NSString*) objectForKey: (NSString*)key
{
  NSString	*value = nil;
  const char    *str = 0;
  xmlAttrPtr	prop;

  prop = ((xmlNodePtr)(lib))->properties;
  while (prop != NULL)
    {
      const void	*name = prop->name;

      if (0 == str)
        {
          str = [key UTF8String];
        }

      if (strcmp(str, name) == 0)
        {
	  xmlNodePtr	child = prop->children;

	  while (child != NULL)
	    {
	      const void	*content = child->content;

	      if (value == nil)
		{
		  value = UTF8Str(content);
		}
	      else
		{
		  value = [value stringByAppendingString: UTF8Str(content)];
		}
	      child = child->next;
	    }
	  break;
	}
      prop = prop->next;
    }
  return value;
}

/**
 * Return the parent of this node.
 */
- (GSXMLNode*) parent
{
  if (((xmlNodePtr)(lib))->parent != NULL)
    {
      GSXMLNode	*n = [GSXMLNode alloc];

      n = [n _initFrom: ((xmlNodePtr)(lib))->parent parent: self];
      return AUTORELEASE(n);
    }
  else
    {
      return nil;
    }
}

/**
 * Return the previous node at this level.
 */
- (GSXMLNode*) previous
{
  if (((xmlNodePtr)(lib))->prev != NULL)
    {
      GSXMLNode	*n = [GSXMLNode alloc];

      n = [n _initFrom: ((xmlNodePtr)(lib))->prev parent: _parent];
      return AUTORELEASE(n);
    }
  else
    {
      return nil;
    }
}

/**
 * Return the previous element node at this level.<br />
 * NB. This method is not available in java, as the method name conflicts
 * with that of java's Enumerator class.
 */
- (GSXMLNode*) previousElement
{
  xmlNodePtr	ptr = (xmlNodePtr)lib;

  while (ptr->prev != NULL)
    {
      ptr = ptr->prev;
      if (ptr->type == XML_ELEMENT_NODE)
	{
	  GSXMLNode	*n = [GSXMLNode alloc];

	  n = [n _initFrom: ptr parent: _parent];
	  return AUTORELEASE(n);
	}
    }
  return nil;
}


/**
 * <p>
 *   Return attributes and values as a dictionary, but applies
 *   the specified selector to each key before adding the
 *   key and value to the dictionary.  The selector must be a
 *   method of NSString taking no arguments and returning an
 *   object suitable for use as a dictionary key.
 * </p>
 * <p>
 *   This method exists for the use of GSWeb ... it is probably
 *   not of much use elsewhere.
 * </p>
 */
- (NSMutableDictionary*) propertiesAsDictionaryWithKeyTransformationSel:
  (SEL)keyTransformSel
{
  xmlAttrPtr		prop;
  NSMutableDictionary	*d = [NSMutableDictionary dictionary];

  prop = ((xmlNodePtr)(lib))->properties;

  while (prop != NULL)
    {
      xmlNodePtr	child = prop->children;
      const void	*name = prop->name;
      NSString		*key = UTF8Str(name);
      NSString		*value = @"";

      if (keyTransformSel != 0)
	{
	  key = [key performSelector: keyTransformSel];
	}
      while (child != NULL)
	{
	  const void	*content = child->content;

	  value = [value stringByAppendingString: UTF8Str(content)];
	  child = child->next;
	}
      [d setObject: value forKey: key];
      prop = prop->next;
    }

  return d;
}

/**
 * Set (or reset) an attribute carried by a node.
 * <example>
 *   [n1 setObject: @"prop1" forKey: @"name1"];
 *   [n1 setObject: @"prop2" forKey: @"name2"];
 *   [n1 setObject: @"prop3" forKey: @"name3"];
 * </example>
 */
- (void) setObject: (NSString*)value forKey: (NSString*)key
{
  xmlSetProp(lib, UTF8STRING(key), UTF8STRING(value));
}

/**
 * Return node-type. The most efficient way of testing node types is to
 * use this method and compare the return value with a value you previously
 * obtained using the +typeFromDescription: method.
 */
- (NSInteger) type
{
  return (NSInteger)((xmlNodePtr)(lib))->type;
}

/**
 * Return node type as a string.
 */
- (NSString*) typeDescription
{
  NSString	*desc;

  desc = (NSString*)NSMapGet(nodeNames, (void*)(intptr_t)[self type]);
  if (desc == nil)
    {
      desc = @"Unknown node type";
    }
  return desc;
}

/**
 * Sets the namespace of the receiver to the value specified.<br />
 * Supplying a nil namespace removes any namespace previously set
 * or any namespace that the node inherited from a parent when it
 * was created.
 */
- (void) setNamespace: (GSXMLNamespace *)space
{
  xmlSetNs (lib, [space lib]);
}

@end

@implementation GSXMLNode (GSPrivate)
/**
 * Initialise from raw libxml data
 */
- (id) _initFrom: (void*)data parent: (id)p
{
  if (data == NULL)
    {
      NSLog(@"%@ - no data for initialization",
	NSStringFromClass([self class]));
      DESTROY(self);
      return nil;
    }
  lib = data;
  ASSIGN(_parent, p);
  return self;
}
@end

/**
 * <p>
 *   The XML parser object is the pivotal part of parsing an XML
 *   document - it will either build a tree representing the
 *   document (if initialized without a GSSAXHandler), or will
 *   cooperate with a GSSAXHandler object to provide parsing
 *   without the overhead of building a tree.
 * </p>
 * <p>
 *   The parser may be initialized with an input source (in which
 *   case it will expect to be asked to parse the entire input in
 *   a single operation), or without.  If it is initialised without
 *   an input source, incremental parsing can be done by feeding
 *   successive parts of the XML document into the parser as
 *   NSData objects.
 * </p>
 */
@implementation GSXMLParser

/* To override location for DTDs
 */
static NSString	*dtdPath = nil;

static NSString	*endMarker = @"At end of incremental parse";

+ (void) initialize
{
  static BOOL	beenHere = NO;

  if (beenHere == NO)
    {
      beenHere = YES;
      if (cacheDone == NO)
	setupCache();
      /* Replace the default external entity loader with our own one which
       * looks for GNUstep DTDs in the correct location.
       */
      if (NULL == originalLoader)
        {
          originalLoader = xmlGetExternalEntityLoader();
          xmlSetExternalEntityLoader(
            (xmlExternalEntityLoader)loadEntityFunction);
        }
    }
}

/**
 * <p>
 *   This method controls the loading of external entities into
 *   the system.  If it returns an empty string, the entity is not
 *   loaded.  If it returns a filename, the entity is loaded from
 *   that file.  If it returns nil, the default entity loading
 *   mechanism is used.
 * </p>
 * <p>
 *   The default entity loading mechanism is to construct a file
 *   name from the locationURL, by replacing all path separators
 *   with underscores, then attempt to locate that file in the DTDs
 *   resource directory of the main bundle, and all the standard
 *   system locations.
 * </p>
 * <p>
 *   As a special case, the default loader examines the publicID
 *   and if it is a GNUstep DTD, the loader constructs a special
 *   name from the ID (by replacing dots with underscores and
 *   spaces with hyphens) and looks for a file with that name
 *   and a '.dtd' extension in the GNUstep bundles.
 * </p>
 * <p>
 *   NB. This method will only be called if there is no SAX
 *   handler in use, or if the corresponding method in the
 *   SAX handler returns nil.
 * </p>
 */
+ (NSString*) loadEntity: (NSString*)publicId
		      at: (NSString*)location
{
  return nil;
}

/**
 * Creation of a new Parser (for incremental parsing)
 * by calling -initWithSAXHandler:
 */
+ (GSXMLParser*) parser
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: nil]);
}

/**
 * Creation of a new Parser by calling
 * -initWithSAXHandler:withContentsOfFile:
 * <example>
 * GSXMLParser       *p = [GSXMLParser parserWithContentsOfFile: @"macos.xml"];
 *
 * if ([p parse])
 *   {
 *     [[p document] dump];
 *   }
 * else
 *   {
 *     printf("error parse file\n");
 *   }
 * </example>
 */
+ (GSXMLParser*) parserWithContentsOfFile: (NSString*)path
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: nil
				   withContentsOfFile: path]);
}

/**
 * Creation of a new Parser by calling
 * -initWithSAXHandler:withContentsOfURL:
 */
+ (GSXMLParser*) parserWithContentsOfURL: (NSURL*)url
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: nil
				    withContentsOfURL: url]);
}

/**
 * Creation of a new Parser by calling
 * -initWithSAXHandler:withData:
 */
+ (GSXMLParser*) parserWithData: (NSData*)data
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: nil
					     withData: data]);
}

/**
 * <p>
 *   Creation of a new Parser by calling -initWithSAXHandler:
 * </p>
 * <p>
 *   If the handler object supplied is nil, the parser will build
 *   a tree representing the parsed file rather than attempting
 *   to get the handler to deal with the parsed elements and entities.
 * </p>
 */
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: handler]);
}

/**
 * Creation of a new Parser by calling
 * -initWithSAXHandler:withContentsOfFile:
 * <example>
 * NSAutoreleasePool *arp = [NSAutoreleasePool new];
 * GSSAXHandler *h = [GSDebugSAXHandler handler];
 * GSXMLParser  *p = [GSXMLParser parserWithSAXHandler: h
 *                                  withContentsOfFile: @"macos.xml"];
 * if ([p parse])
 *   {
 *      printf("ok\n");
 *   }
 * RELEASE(arp);
 * </example>
 */
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler
		   withContentsOfFile: (NSString*)path
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: handler
				   withContentsOfFile: path]);
}

/**
 * Creation of a new Parser by calling
 * -initWithSAXHandler:withContentsOfURL:
 */
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler
		    withContentsOfURL: (NSURL*)url
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: handler
				    withContentsOfURL: url]);
}

/**
 * Creation of a new Parser by calling
 * -initWithSAXHandler:withData:
 */
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler
			     withData: (NSData*)data
{
  return AUTORELEASE([[self alloc] initWithSAXHandler: handler
					     withData: data]);
}

/** Sets a directory in which to look for DTDs when resolving external
 * references.  Can be used whjen DTDs have not been installed in the
 * normal locatioons.
 */
+ (void) setDTDs: (NSString*)aPath
{
  ASSIGNCOPY(dtdPath, aPath);
}

/**
 * Return the name of the string encoding (for XML) to use for the
 * specified OpenStep encoding.
 */
+ (NSString*) xmlEncodingStringForStringEncoding: (NSStringEncoding)encoding
{
  NSString	*xmlEncodingString = nil;

  switch (encoding)
    {
      case NSUnicodeStringEncoding:
	NSLog(@"NSUnicodeStringEncoding not supported for XML");//??
	break;
      case NSNEXTSTEPStringEncoding:
	NSLog(@"NSNEXTSTEPStringEncoding not supported for XML");//??
	break;
      case NSJapaneseEUCStringEncoding:
	xmlEncodingString = @"EUC-JP";
	break;
      case NSShiftJISStringEncoding:
	xmlEncodingString = @"Shift-JIS";
	break;
      case NSISO2022JPStringEncoding:
	xmlEncodingString = @"ISO-2022-JP";
	break;
      case NSUTF8StringEncoding:
	xmlEncodingString = @"UTF-8";
	break;
      case NSWindowsCP1251StringEncoding:
	NSLog(@"NSWindowsCP1251StringEncoding not supported for XML");//??
	break;
      case NSWindowsCP1252StringEncoding:
	NSLog(@"NSWindowsCP1252StringEncoding not supported for XML");//??
	break;
      case NSWindowsCP1253StringEncoding:
	NSLog(@"NSWindowsCP1253StringEncoding not supported for XML");//??
	break;
      case NSWindowsCP1254StringEncoding:
	NSLog(@"NSWindowsCP1254StringEncoding not supported for XML");//??
	break;
      case NSWindowsCP1250StringEncoding:
	NSLog(@"NSWindowsCP1250StringEncoding not supported for XML");//??
	break;
      case NSISOLatin1StringEncoding:
	xmlEncodingString = @"ISO-8859-1";
	break;
      case NSISOLatin2StringEncoding:
	xmlEncodingString = @"ISO-8859-2";
	break;
      case NSSymbolStringEncoding:
	NSLog(@"NSSymbolStringEncoding not supported for XML");//??
	break;
      default:
	NSLog(@"Encoding not supported for XML");//??
	xmlEncodingString = nil;
	break;
    }
  return xmlEncodingString;
}

/**
 * If called by a SAX callback routine, this method will terminate
 * the parsiong process.
 */
- (void) abortParsing
{
  if (lib != NULL)
    {
      xmlParserCtxtPtr	ctxt = (xmlParserCtxtPtr)lib;

      // Stop SAX callbacks
      ctxt->disableSAX = 1;
      // Stop incoming data being parsed.
      ctxt->instate = XML_PARSER_EOF;
      // Pretend we are at end of file (nul byte).
      if (ctxt->input != NULL) ctxt->input->cur = (const unsigned char*)"";
    }
}

/**
 * If executed during a parse operation, returns the current column number.
 */
- (NSInteger) columnNumber
{
  return  xmlSAX2GetColumnNumber(lib);
}

- (void) dealloc
{
  RELEASE(messages);
  RELEASE(src);
  RELEASE(saxHandler);
  if (lib != NULL)
    {
      xmlFreeDoc(((xmlParserCtxtPtr)lib)->myDoc);
      xmlFreeParserCtxt(lib);
    }
  [super dealloc];
}

/**
 * Sets whether the document needs to be validated.
 */
- (BOOL) doValidityChecking: (BOOL)yesno
{
  BOOL	old;

  old = (((xmlParserCtxtPtr)lib)->validate) ? YES : NO;
  ((xmlParserCtxtPtr)lib)->validate = (yesno ? 1 : 0);
  return old;
}

/**
 * Return the document produced as a result of parsing data.
 */
- (GSXMLDocument*) document
{
  GSXMLDocument	*d = [GSXMLDocument alloc];

  d = [d _initFrom: ((xmlParserCtxtPtr)lib)->myDoc parent: self ownsLib: NO];
  return AUTORELEASE(d);
}

/**
 * Return error code for last parse operation.
 */
- (NSInteger) errNo
{
  return ((xmlParserCtxtPtr)lib)->errNo;
}

/**
 * Sets whether warnings are generated.
 */
- (BOOL) getWarnings: (BOOL)yesno
{
  BOOL	old = (((xmlParserCtxtPtr)lib)->vctxt.warning) ? YES : NO;

  if (yesno == YES)
    {
      ((xmlParserCtxtPtr)lib)->vctxt.warning = xmlParserValidityWarning;
    }
  else
    {
      ((xmlParserCtxtPtr)lib)->vctxt.warning = 0;
    }

  return old;
}

/**
 * Initialises by calling -initWithSAXHandler: with a nil argument.
 */
- (id) init
{
  return [self initWithSAXHandler: nil];
}

/** <init />
 * <p>
 *   Initialisation of a new Parser with SAX handler.
 * </p>
 * <p>
 *   If the handler object supplied is nil, the parser will use
 *   an instance of [GSTreeSAXHandler] to build a tree representing
 *   the parsed file.  This tree will then be available (via the -document
 *   method) as a [GSXMLDocument] on completion of parsing.
 * </p>
 * <p>
 *   The source for the parsing process is not specified - so
 *   parsing must be done incrementally by feeding data to the
 *   parser.
 * </p>
 */
- (id) initWithSAXHandler: (GSSAXHandler*)handler
{
  if (handler == nil)
    {
      saxHandler = [GSTreeSAXHandler new];
    }
  else if ([handler isKindOfClass: [GSSAXHandler class]] == YES)
    {
      saxHandler = RETAIN(handler);
    }
  else
    {
      NSLog(@"Bad GSSAXHandler object passed to GSXMLParser initialiser");
      DESTROY(self);
      return nil;
    }
  [saxHandler _setParser: self];
  if ([self _initLibXML] == NO)
    {
      DESTROY(self);
      return nil;
    }
  return self;
}

/**
 * <p>
 *   Initialisation of a new Parser with SAX handler (if not nil)
 *   by calling -initWithSAXHandler:
 * </p>
 * <p>
 *   Sets the input source for the parser to be the specified file -
 *   so parsing of the entire file will be performed rather than
 *   incremental parsing.
 * </p>
 */
- (id) initWithSAXHandler: (GSSAXHandler*)handler
       withContentsOfFile: (NSString*)path
{
  if (path == nil || [path isKindOfClass: NSString_class] == NO)
    {
      NSLog(@"Bad file path passed to initialize GSXMLParser");
      DESTROY(self);
      return nil;
    }
  src = [path copy];
  self = [self initWithSAXHandler: handler];
  return self;
}

/**
 * <p>
 *   Initialisation of a new Parser with SAX handler (if not nil)
 *   by calling -initWithSAXHandler:
 * </p>
 * <p>
 *   Sets the input source for the parser to be the specified URL -
 *   so parsing of the entire document will be performed rather than
 *   incremental parsing.
 * </p>
 */
- (id) initWithSAXHandler: (GSSAXHandler*)handler
	withContentsOfURL: (NSURL*)url
{
  if (url == nil || [url isKindOfClass: [NSURL class]] == NO)
    {
      NSLog(@"Bad NSURL passed to initialize GSXMLParser");
      DESTROY(self);
      return nil;
    }
  src = [url copy];
  self = [self initWithSAXHandler: handler];
  return self;
}

/**
 * <p>
 *   Initialisation of a new Parser with SAX handler (if not nil)
 *   by calling -initWithSAXHandler:
 * </p>
 * <p>
 *   Sets the input source for the parser to be the specified data
 *   object (which must contain an XML document), so parsing of the
 *   entire document will be performed rather than incremental parsing.
 * </p>
 */
- (id) initWithSAXHandler: (GSSAXHandler*)handler
		 withData: (NSData*)data
{
  if (nil == data)
    {
      NSLog(@"Nil NSData passed to initialize GSXMLParser");
      DESTROY(self);
      return nil;
    }
  if ([data isKindOfClass: [NSData class]] == NO)
    {
      NSLog(@"Non NSData passed to initialize GSXMLParser; %@", data);
      DESTROY(self);
      return nil;
    }
  src = [data copy];
  self = [self initWithSAXHandler: handler];
  return self;
}

/**
 * <p>
 *   Initialisation of a new Parser with SAX handler (if not nil)
 *   by calling -initWithSAXHandler:
 * </p>
 * <p>
 *   Sets the input source for the parser to be the specified input stream,
 *   so parsing of the entire document will be performed rather than
 *   incremental parsing.
 * </p>
 */
- (id) initWithSAXHandler: (GSSAXHandler*)handler
	 withInputStream: (NSInputStream*)stream
{
  if (stream == nil || [stream isKindOfClass: [NSInputStream class]] == NO)
    {
      NSLog(@"Bad NSInputStream passed to initialize GSXMLParser");
      DESTROY(self);
      return nil;
    }
  src = RETAIN(stream);
  self = [self initWithSAXHandler: handler];
  return self;
}

/**
 * Set and return the previous value for blank text nodes support.
 * ignorableWhitespace nodes are only generated when running
 * the parser in validating mode and when the current element
 * doesn't allow CDATA or mixed content.
 */
- (BOOL) keepBlanks: (BOOL)yesno
{
  BOOL	old;

  old = (((xmlParserCtxtPtr)lib)->keepBlanks) ? YES : NO;
  ((xmlParserCtxtPtr)lib)->keepBlanks = (yesno ? 1 : 0);
  return old;
}

/**
 * If executed during a parse operation, returns the current line number.
 */
- (NSInteger) lineNumber
{
  return  xmlSAX2GetLineNumber(lib);
}

/**
 * Returns the string into which warning and error messages are saved,
 * or nil if they are being written to stderr.
 */
- (NSString*) messages
{
  return messages;
}

/**
 * Parse source. Return YES if parsed as valid, otherwise NO.
 * If validation against a DTD is not enabled, the return value simply
 * indicates whether the xml was well formed.<br />
 * This method should be called once to parse the entire document.
 * <example>
 * GSXMLParser       *p = [GSXMLParser parserWithContentsOfFile:@"macos.xml"];
 *
 * if ([p parse])
 *   {
 *     [[p doc] dump];
 *   }
 * else
 *   {
 *     printf("error parse file\n");
 *   }
 * </example>
 */
- (BOOL) parse
{
  id	tmp;

  if (src == endMarker)
    {
      NSLog(@"GSXMLParser -parse called on object that is already parsed");
      return NO;
    }
  if (src == nil)
    {
      NSLog(@"GSXMLParser -parse called on object with no source");
      return NO;
    }

  if ([src isKindOfClass: [NSData class]]
      || [src isKindOfClass: [NSInputStream class]])
    {
    }
  else if ([src isKindOfClass: NSString_class])
    {
      NSData	*data = [NSData dataWithContentsOfFile: src];

      if (data == nil)
	{
	  NSLog(@"File to parse (%@) is not readable", src);
          return NO;
	}
      ASSIGN(src, data);
    }
  else if ([src isKindOfClass: [NSURL class]])
    {
      NSData	*data = [src resourceDataUsingCache: YES];

      if (data == nil)
	{
	  NSLog(@"URL to parse (%@) is not readable", src);
          return NO;
	}
      ASSIGN(src, data);
    }
  else
    {
       NSLog(@"Source for [-parse] must be NSString, NSData, NSInputStream, or"
         @" NSURL type");
       return NO;
    }

  tmp = RETAIN(src);
  ASSIGN(src, endMarker);
  if ([tmp isKindOfClass: [NSInputStream class]])
    {
      xmlParseDocument(lib);
    }
  else
    {
      [self _parseChunk: tmp];
      [self _parseChunk: nil];
    }
  RELEASE(tmp);

  if (((xmlParserCtxtPtr)lib)->wellFormed != 0
    && (0 == ((xmlParserCtxtPtr)lib)->validate
      || ((xmlParserCtxtPtr)lib)->valid != 0))
    {
      return YES;
    }
  return NO;
}

/**
 * <p>
 *   Pass data to the parser for incremental parsing.  This method
 *   should be called many times, with each call passing another
 *   block of data from the same document.  After the whole of the
 *   document has been parsed, the method should be called with
 *   an empty or nil data object to indicate end of parsing.
 *   On this final call, the return value indicates whether the
 *   document was valid or not.  If validation to a DTD is not enabled,
 *   the return value simply indicates whether the xml was well formed.
 * </p>
 * <example>
 * GSXMLParser       *p = [GSXMLParser parserWithSAXHandler: nil source: nil];
 *
 * while ((data = getMoreData()) != nil)
 *   {
 *     if ([p parse: data] == NO)
 *       {
 *         NSLog(@"parse error");
 *       }
 *   }
 *   // Do something with document parsed
 * [p parse: nil];  // Completed parsing of document.
 * </example>
 */
- (BOOL) parse: (NSData*)data
{
  if (src == endMarker)
    {
      NSLog(@"GSXMLParser -parse: called on object that is fully parsed");
      return NO;
    }
  if (src != nil)
    {
       NSLog(@"XMLParser -parse: called for parser not initialised with nil");
       return NO;
    }

  if (data == nil || [data length] == 0)
    {
      /*
       * At end of incremental parse.
       */
      if (lib != NULL)
	{
	  [self _parseChunk: nil];
	  src = endMarker;
          if (((xmlParserCtxtPtr)lib)->wellFormed != 0
            && (0 == ((xmlParserCtxtPtr)lib)->validate
              || ((xmlParserCtxtPtr)lib)->valid != 0))
            {
              return YES;
            }
          return NO;
	}
      else
	{
	  NSLog(@"GSXMLParser -parse: terminated with no data");
	  return NO;
	}
    }
  else
    {
      [self _parseChunk: data];
      return YES;
    }
}

/**
 * Return the public ID of the document being parsed.
 */
- (NSString*) publicID
{
  return UTF8Str(xmlSAX2GetPublicId(lib));
}

/**
 * Sets up (or removes) a mutable string to which error and warning
 * messages are saved.  Using an argument of NO will cause these messages
 * to be written to stderr (the default).<br />
 * NB. A SAX handler which overrides the error and warning logging
 * messages may stop this mechanism operating.
 */
- (void) saveMessages: (BOOL)yesno
{
  if (yesno == YES)
    {
      ASSIGN(messages, [NSMutableString stringWithCapacity: 256]);
    }
  else
    {
      DESTROY(messages);
    }
}

- (BOOL) resolveEntities: (BOOL)yesno
{
  BOOL	old;

  if (yesno) yesno = YES;
  if ((((xmlParserCtxtPtr)lib)->sax)->getEntity
    == (void*)getEntityIgnoreExternal)
    {
      old = NO;
    }
  else
    {
      old = YES;
    }
  if (YES == yesno)
    {
      (((xmlParserCtxtPtr)lib)->sax)->getEntity
        = (void*)getEntityResolveExternal;
    }
  else
    {
      (((xmlParserCtxtPtr)lib)->sax)->getEntity
        = (void*)getEntityIgnoreExternal;
    } 
  return old;
}

/**
 * Set and return the previous value for entity support.
 * Initially the parser always keeps entity references instead
 * of substituting entity values in the output.
 */
- (BOOL) substituteEntities: (BOOL)yesno
{
  BOOL	old;

  if (yesno) yesno = YES;
  old = (((xmlParserCtxtPtr)lib)->replaceEntities) ? YES : NO;
  if (old != yesno)
    {
      ((xmlParserCtxtPtr)lib)->replaceEntities = (yesno ? 1 : 0);
    }
  
  return old;
}

/**
 * Return the system ID of the document being parsed.
 */
- (NSString*) systemID
{
  return UTF8Str(xmlSAX2GetSystemId(lib));
}

/*
 * Private methods - internal use only.
 */

- (BOOL) _initLibXML
{
  const char	*file;

  if ([src isKindOfClass: NSString_class])
    {
      file = [src lossyCString];
    }
  else if ([src isKindOfClass: [NSURL class]])
    {
      file = [[src absoluteString] lossyCString];
    }
  else
    {
      file = ".";
    }

  if ([src isKindOfClass: [NSInputStream class]])
    {
      [(NSInputStream*)src open];
      lib = (void*)xmlCreateIOParserCtxt([saxHandler lib], NULL,
        xmlNSInputStreamReadCallback, xmlNSInputStreamCloseCallback,
        (void*)src, XML_CHAR_ENCODING_NONE);
    }
  else
    {
      lib = (void*)xmlCreatePushParserCtxt([saxHandler lib], NULL, 0, 0, file);
    }

  if (lib == NULL)
    {
      NSLog(@"Failed to create libxml parser context");
      return NO;
    }
  else
    {
      /*
       * Put saxHandler address in _private member, so we can retrieve
       * the GSSAXHandler to use in our SAX C Functions.
       */
      ((xmlParserCtxtPtr)lib)->_private = saxHandler;

      /*
       * Set the entity loading function for this parser to be our one.
       */
      ((xmlParserCtxtPtr)lib)->sax->resolveEntity = resolveEntityFunction;
    }
  return YES;
}

- (NSMutableString*) _messages
{
  return messages;
}

// nil data allowed
- (void) _parseChunk: (NSData*)data
{
  if (lib == NULL || ((xmlParserCtxtPtr)lib)->disableSAX != 0)
    {
      return;	// Parsing impossible or disabled.
    }
  xmlParseChunk(lib, [data bytes], [data length], data == nil);
}

@end

/**
 * The GSHTMLParser class is a simple subclass of GSXMLParser which should
 * parse reasonably well formed HTML documents. If you wish to parse XHTML
 * documents, you should use GSXMLParser ... the GSHTMLParser class is for
 * older 'legacy' documents.
 */
@implementation GSHTMLParser

- (BOOL) _initLibXML
{
  lib = (void*)htmlCreatePushParserCtxt([saxHandler lib], NULL, 0, 0, ".",
    XML_CHAR_ENCODING_NONE);
  if (lib == NULL)
    {
      NSLog(@"Failed to create libxml parser context");
      return NO;
    }
  else
    {
      /*
       * Put saxHandler address in _private member, so we can retrieve
       * the GSSAXHandler to use in our SAX C Functions.
       */
      ((htmlParserCtxtPtr)lib)->_private = saxHandler;
    }
  return YES;
}

- (void) _parseChunk: (NSData*)data
{
  htmlParseChunk(lib, [data bytes], [data length], data == nil);
}

@end

/**
 * <p>XML SAX Handler.</p>
 * <p>
 *   GSSAXHandler is a callback-based interface to the [GSXMLParser]
 *   which operates in a similar (though not identical) manner to
 *   SAX.
 * </p>
 * <p>
 *   Each GSSAXHandler object is associated with a GSXMLParser
 *   object.  As parsing progresses, the methods of the GSSAXHandler
 *   are invoked by the parser, so the handler is able to deal
 *   with the elements and entities being parsed.
 * </p>
 * <p>
 *   The callback methods in the GSSAXHandler class do nothing - it
 *   is intended that you subclass GSSAXHandler and override them.
 * </p>
 * <p>
 *   If you create a GSXMLParser passing nil as the GSSAXHandler,
 *   the parser will parse data to create a [GSXMLDocument] instance
 *   which you can then examine as a whole ... this is generally the
 *   preferred mechanism for parsing as it permits the parser to
 *   validate the parsed document againts a DTD, and your software
 *   can then examine the document secure in the knowledge that it
 *   contains the expected structure.  Use of a GSSAXHandler is
 *   preferred for very large documents with simple structure ...
 *   in which case incremental parsing is more efficient.
 * </p>
 */
@implementation GSSAXHandler

+ (void) initialize
{
  static BOOL	beenHere = NO;

  if (beenHere == NO)
    {
      beenHere = YES;
      if (cacheDone == NO)
        {
          setupCache();
        }
      /* Replace the default external entity loader with our own one which
       * looks for GNUstep DTDs in the correct location.
       */
      if (NULL == originalLoader)
        {
          originalLoader = xmlGetExternalEntityLoader();
          xmlSetExternalEntityLoader(
            (xmlExternalEntityLoader)loadEntityFunction);
        }
    }
}

/*
 * The context is a xmlParserCtxtPtr or htmlParserCtxtPtr.
 * Its _private member contains the address of our Sax Handler Object.
 * We can use a (xmlParserCtxtPtr) cast because xmlParserCtxt and
 * htmlParserCtxt are the same structure (and will remain, cf libxml author).
 */
#define	HANDLER	((GSSAXHandler*)(((xmlParserCtxtPtr)ctx)->_private))

static xmlEntityPtr
getEntityDefault(void *ctx, const xmlChar *name, BOOL resolve)
{
  xmlParserCtxtPtr      ctxt = (xmlParserCtxtPtr) ctx;
  xmlEntityPtr          ret = NULL;

  if (ctx != 0)
    {
      if (0 == ctxt->inSubset)
        {
          if ((ret = xmlGetPredefinedEntity(name)) != NULL)
            {
              return ret;
            }
        }
      if ((ctxt->myDoc != NULL) && (1 == ctxt->myDoc->standalone))
        {
          if (2 == ctxt->inSubset)
            {
              ctxt->myDoc->standalone = 0;
              ret = xmlGetDocEntity(ctxt->myDoc, name);
              ctxt->myDoc->standalone = 1;
            }
          else
            {
              ret = xmlGetDocEntity(ctxt->myDoc, name);
              if (NULL == ret)
                {
                  ctxt->myDoc->standalone = 0;
                  ret = xmlGetDocEntity(ctxt->myDoc, name);
                  if (ret != NULL)
                    {
                      ((((xmlParserCtxtPtr)ctxt)->sax)->fatalError)(ctxt,
                        "Entity(%s) document marked standalone"
                        " but requires external subset", name);
                      xmlStopParser(ctxt);
                    }
                  ctxt->myDoc->standalone = 1;
                }
            }
        }
      else
        {
          ret = xmlGetDocEntity(ctxt->myDoc, name);
        }
      if ((ret != NULL)
        && ((ctxt->validate) || (ctxt->replaceEntities))
        && (ret->children == NULL)
        && (ret->etype == XML_EXTERNAL_GENERAL_PARSED_ENTITY))
        {
          if (YES == resolve)
            {
              xmlNodePtr    children;
              int           val;

              /*
               * for validation purposes we really need to fetch and
               * parse the external entity
               */
              val = xmlParseCtxtExternalEntity(ctxt, ret->URI,
                ret->ExternalID, &children);
              if (val == 0)
                {
                  xmlAddChildList((xmlNodePtr) ret, children);
                }
              else
                {
                  ((((xmlParserCtxtPtr)ctxt)->sax)->fatalError)(ctxt,
                    "Failure to process entity %s\n", name);
                  xmlStopParser(ctxt);
                  ctxt->validate = 0;
                  return NULL;
                }
              ret->owner = 1;
              if (ret->checked == 0)
                {
                  ret->checked = 1;
                }
            }
        }
    }
  return ret;
}

static xmlEntityPtr
getEntityIgnoreExternal(void *ctx, const xmlChar *name)
{
  return getEntityDefault(ctx, name, NO);
}

static xmlEntityPtr
getEntityResolveExternal(void *ctx, const xmlChar *name)
{
  return getEntityDefault(ctx, name, YES);
}

/* WARNING ... as far as I can tell libxml2 never uses the resolveEntity
 * callback, so this function is never called via that route.
 * We therefore also set this as the global default entity loading
 * function (in [GSXMLParser+initialize] and [GSSAXHandler+initialize]).
 *
 * To implement the -resolveEntities method we must permit/deny any attempt
 * to load an entity (before the function to resolve is even called),
 * We therefore intercept the getEntity callback (using getEntityDefault()),
 * re-implementing some of the code inside libxml2 to avoid attempts to
 * load/parse external entities unless we have specifically enabled it.
 */
static xmlParserInputPtr
loadEntityFunction(const unsigned char *url,
  const unsigned char *eid,
  void *ctx)
{
  NSString			*file = nil;
  NSString			*entityId;
  NSString			*location;
  NSArray			*components;
  NSMutableString		*local;
  unsigned			count;
  unsigned			index;

  NSCAssert(ctx, @"No Context");
  if (url == NULL)
    return NULL;

  entityId = (eid != NULL) ? (id)UTF8Str(eid) : nil;
  location = UTF8Str(url);
  components = [location pathComponents];
  local = (NSMutableString *) [NSMutableString string];

  /*
   * Build a local filename by replacing path separator characters with
   * something else.
   */
  count = [components count];
  if (count > 0)
    {
      count--;
      for (index = 0; index < count; index++)
	{
	  [local appendString: [components objectAtIndex: index]];
	  [local appendString: @"_"];
	}
      [local appendString: [components objectAtIndex: index]];
    }
  /* Also replace ':' which isn't legal on some file systems */
  [local replaceOccurrencesOfString: @":" withString: @"+"
                            options: NSLiteralSearch
                            range: NSMakeRange(0, [local length])];

  if ([location rangeOfString: @"/DTDs/PropertyList"].length > 0)
    {
      file = [location substringFromIndex: 6];
      if ([[NSFileManager defaultManager] fileExistsAtPath: file] == NO)
        {
	  location = [NSBundle pathForLibraryResource: @"plist-0_9"
					       ofType: @"dtd"
					  inDirectory: @"DTDs"];
	  entityId = @"-//GNUstep//DTD plist 0.9//EN";
	  file = nil;
	}
    }

  if (file == nil && ((xmlParserCtxtPtr)ctx)->_private != NULL)
    {
      /*
       * Now ask the SAXHandler callback for the name of a local file
       */
      file = [HANDLER loadEntity: entityId at: location];
      if (file == nil)
	{
	  file = [GSXMLParser loadEntity: entityId at: location];
	}
    }

  if (file == nil)
    {
      /*
       * Special case - GNUstep DTDs - should be installed in the GNUstep
       * system bundle - so we look for them there.
       */
      if ([entityId hasPrefix: @"-//GNUstep//DTD "] == YES)
	{
	  NSCharacterSet	*ws = [NSCharacterSet whitespaceCharacterSet];
	  NSString		*found = nil;
	  NSMutableString	*name;
	  unsigned		len;
	  NSRange		r;

	  /*
	   * Extract the relevent DTD name
	   */
	  name = AUTORELEASE([entityId mutableCopy]);
	  r = NSMakeRange(0, 16);
	  [name deleteCharactersInRange: r];
	  len = [name length];
	  r = [name rangeOfString: @"/" options: NSLiteralSearch];
	  if (r.length > 0)
	    {
	      r.length = len - r.location;
	      [name deleteCharactersInRange: r];
	      len = [name length];
	    }

	  /*
	   * Convert dots to underscores.
	   */
	  r = [name rangeOfString: @"." options: NSLiteralSearch];
	  while (r.length > 0)
	    {
	      [name replaceCharactersInRange: r withString: @"_"];
	      r.location++;
	      r.length = len - r.location;
	      r = [name rangeOfString: @"."
			     options: NSLiteralSearch
			       range: r];
	    }

	  /*
	   * Convert whitespace to hyphens.
	   */
	  r = [name rangeOfCharacterFromSet: ws options: NSLiteralSearch];
	  while (r.length > 0)
	    {
	      [name replaceCharactersInRange: r withString: @"-"];
	      r.location++;
	      r.length = len - r.location;
	      r = [name rangeOfCharacterFromSet: ws
				       options: NSLiteralSearch
					 range: r];
	    }

	  if (dtdPath != nil)
	    {
	      found = [dtdPath stringByAppendingPathComponent: name];
	      found = [found stringByAppendingPathExtension: @"dtd"];
	      if (![[NSFileManager defaultManager] fileExistsAtPath: found])
		{
		  found = nil;
		}
	    }
	  if (found == nil)
	    {
#ifndef NeXT_Foundation_LIBRARY
	      found = [NSBundle pathForLibraryResource: name
						ofType: @"dtd"
					   inDirectory: @"DTDs"];
#else
	      found = [[NSBundle bundleForClass: [GSXMLNode class]]
		pathForResource: name
		ofType: @"dtd"
		inDirectory: @"DTDs"];
#endif
	    }
	  if (found == nil)
	    {
	      NSLog(@"unable to find GNUstep DTD - '%@' for '%s'", name, eid);
	    }
	  else
	    {
	      file = found;
	    }
	}

      /*
       * DTD not found - so we look for it in standard locations.
       */
      if (file == nil)
	{
	  if (dtdPath != nil)
	    {
	      file = [dtdPath stringByAppendingPathComponent: local];
	      if (![[NSFileManager defaultManager] fileExistsAtPath: local])
		{
		  file = nil;
		}
	    }
	  if (file == nil)
	    {
	      file = [[NSBundle mainBundle] pathForResource: local
						     ofType: @""
					        inDirectory: @"DTDs"];
	    }
	  if (file == nil)
	    {
#ifndef NeXT_Foundation_LIBRARY
	      file = [NSBundle pathForLibraryResource: local
					       ofType: @""
					  inDirectory: @"DTDs"];
#else
	      file = [[NSBundle bundleForClass: [GSXMLNode class]]
		pathForResource: local
		ofType: @""
		inDirectory: @"DTDs"];
#endif
	    }
	}
    }

  /*
   * If we found the DTD somewhere, add it to the catalog.
   */
  if ([file length] > 0)
    {
      NSURL *theURL = [NSURL fileURLWithPath: file];
      xmlCatalogAdd((const unsigned char*)"public", eid,
        UTF8STRING([theURL absoluteString]));
    }
    
  /* A local DTD will now be in the catalog: The builtin entity resolver can
   * take over.
   */
  return (*originalLoader)((const char*)url, (const char*)eid, ctx);
}

static xmlParserInputPtr
resolveEntityFunction(void *ctx,
  const unsigned char *eid, const unsigned char *url)
{
  return loadEntityFunction(url, eid, ctx);
}


#define	TREEFUN(NAME,ARGS) ((HANDLER->isHtmlHandler == YES) ? (*(htmlDefaultSAXHandler.NAME))ARGS : (*(xmlDefaultSAXHandler.NAME))ARGS)
#define	START(SELNAME, RET, ARGS) \
  static SEL sel; \
  static RET (*treeImp)ARGS = 0; \
  RET (*imp)ARGS; \
\
  NSCAssert(ctx,@"No Context"); \
\
  if (treeImp == 0) \
    { \
      sel = @selector(SELNAME); \
      treeImp = (RET (*)ARGS)[treeClass instanceMethodForSelector: sel];\
    } \
  imp = (RET (*)ARGS)[HANDLER methodForSelector: sel]


static void
startDocumentFunction(void *ctx)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER startDocument];
}

static void
endDocumentFunction(void *ctx)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER endDocument];
}

static int
isStandaloneFunction(void *ctx)
{
  NSCAssert(ctx,@"No Context");
  return [HANDLER isStandalone];
}

static int
hasInternalSubsetFunction(void *ctx)
{
  int	has;

  NSCAssert(ctx,@"No Context");
  has = [HANDLER hasInternalSubset];
  if (has < 0)
    {
      has = TREEFUN(hasInternalSubset, (ctx));
    }
  return has;
}

static int
hasExternalSubsetFunction(void *ctx)
{
  int	has;

  NSCAssert(ctx,@"No Context");
  has = [HANDLER hasExternalSubset];
  if (has < 0)
    {
      has = TREEFUN(hasExternalSubset, (ctx));
    }
  return has;
}

static void
internalSubsetFunction(void *ctx, const unsigned char *name,
  const xmlChar *ExternalID, const xmlChar *SystemID)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER internalSubset: UTF8Str(name)
	       externalID: UTF8Str(ExternalID)
		 systemID: UTF8Str(SystemID)];
}

static void
externalSubsetFunction(void *ctx, const unsigned char *name,
  const xmlChar *ExternalID, const xmlChar *SystemID)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER externalSubset: UTF8Str(name)
	       externalID: UTF8Str(ExternalID)
		 systemID: UTF8Str(SystemID)];
}

static xmlEntityPtr
getEntityFunction(void *ctx, const unsigned char *name)
{
  NSCAssert(ctx,@"No Context");
  return [HANDLER getEntity: UTF8Str(name)];
}

static xmlEntityPtr
getParameterEntityFunction(void *ctx, const unsigned char *name)
{
  NSCAssert(ctx,@"No Context");
  return [HANDLER getParameterEntity: UTF8Str(name)];
}

static void
entityDeclFunction(void *ctx, const unsigned char *name, int type,
  const unsigned char *publicId, const unsigned char *systemId,
  unsigned char *content)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER entityDecl: UTF8Str(name)
		 type: type
	       public: UTF8Str(publicId)
	       system: UTF8Str(systemId)
	      content: UTF8Str(content)];
}

static void
attributeDeclFunction(void *ctx, const unsigned char *elem,
  const unsigned char *name, int type, int def,
  const unsigned char *defaultValue, xmlEnumerationPtr tree)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER attributeDecl: UTF8Str(elem)
		    name: UTF8Str(name)
		    type: type
	    typeDefValue: def
	    defaultValue: UTF8Str(defaultValue)];
}

static void
elementDeclFunction(void *ctx, const unsigned char *name, int type,
  xmlElementContentPtr content)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER elementDecl: UTF8Str(name) type: type];
}

static void
notationDeclFunction(void *ctx, const unsigned char *name,
  const unsigned char *publicId, const unsigned char *systemId)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER notationDecl: UTF8Str(name)
		 public: UTF8Str(publicId)
		 system: UTF8Str(systemId)];
}

static void
unparsedEntityDeclFunction(void *ctx, const unsigned char *name,
  const unsigned char *publicId, const unsigned char *systemId,
  const unsigned char *notationName)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER unparsedEntityDecl: UTF8Str(name)
		       public: UTF8Str(publicId)
		       system: UTF8Str(systemId)
		 notationName: UTF8Str(notationName)];
}

static void
startElementFunction(void *ctx, const unsigned char *name,
  const unsigned char **atts)
{
  NSMutableDictionary *dict;

  NSCAssert(ctx,@"No Context");
  dict = [NSMutableDictionary dictionary];
  if (atts != NULL)
    {
      int i = 0;

      while (atts[i] != NULL)
	{
	  NSString		*key = UTF8Str(atts[i++]);
	  NSString		*obj;
	  const unsigned char	*val = atts[i++];

	  if (0 == val)
	    {
	      /* No value ... assume tis is a minimised attribute in html.
	       */
	      obj = key;
	    }
	  else
	    {
	      obj = UTF8Str(val);
	    }
	  [dict setObject: obj forKey: key];
	}
    }
  [HANDLER startElement: UTF8Str(name)
	     attributes: dict];
}

static void
endElementFunction(void *ctx, const unsigned char *name)
{
  [HANDLER endElement: UTF8Str(name)];
}

static void
startElementNsFunction(void *ctx, const unsigned char *name,
  const unsigned char *prefix, const unsigned char *href,
  int nb_namespaces, const unsigned char **namespaces,
  int nb_attributes, int nb_defaulted,
  const unsigned char **atts)
{
  NSMutableDictionary	*adict = nil;
  NSMutableDictionary	*ndict = nil;
  NSString		*elem;

  NSCAssert(ctx,@"No Context");
  elem = UTF8Str(name);
  if (atts != NULL)
    {
      int 	i;
      int	j;

      adict = [NSMutableDictionary dictionaryWithCapacity: nb_attributes];
      for (i = j = 0; i < nb_attributes; i++, j += 5)
	{
	  NSString	*key = UTF8Str(atts[j]);
          NSString      *obj = nil;
          // We need to append the namespace prefix
          if (atts[j+1] != NULL)
            {
              key =
               [[UTF8Str(atts[j+1]) stringByAppendingString: @":"]
                                      stringByAppendingString: key];
            }
	  obj = UTF8StrLen(atts[j+3], atts[j+4]-atts[j+3]);

	  [adict setObject: obj forKey: key];
	}
    }
  if (nb_namespaces > 0)
    {
      int       i;
      int       pos = 0;

      ndict = [NSMutableDictionary dictionaryWithCapacity: nb_namespaces];
      for (i = 0; i < nb_namespaces; i++)
        {
          NSString      *key;
          NSString      *obj;

          if (namespaces[pos] == 0)
            {
              key = @"";
            }
          else
            {
              key = UTF8Str(namespaces[pos]);
            }
          pos++;
          if (namespaces[pos] == 0)
            {
              obj = @"";
            }
          else
            {
              obj = UTF8Str(namespaces[pos]);
            }
          pos++;
          [ndict setObject: obj forKey: key];
        }
    }
  [HANDLER startElement: elem
		 prefix: UTF8Str(prefix)
		   href: UTF8Str(href)
	     attributes: adict
             namespaces: ndict];
}

static void
endElementNsFunction(void *ctx, const unsigned char *name,
  const unsigned char *prefix, const unsigned char *href)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER endElement: UTF8Str(name)
	       prefix: UTF8Str(prefix)
		 href: UTF8Str(href)];
}

static void
charactersFunction(void *ctx, const unsigned char *ch, int len)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER characters: UTF8StrLen(ch, len)];
}

static void
referenceFunction(void *ctx, const unsigned char *name)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER reference: UTF8Str(name)];
}

static void
ignorableWhitespaceFunction(void *ctx, const unsigned char *ch, int len)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER ignoreWhitespace: UTF8StrLen(ch, len)];
}

static void
processingInstructionFunction(void *ctx, const unsigned char *target,
  const char *data)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER processInstruction: UTF8Str(target)
			 data: UTF8Str((const unsigned char*)data)];
}

static void
cdataBlockFunction(void *ctx, const unsigned char *value, int len)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER cdataBlock: [NSData dataWithBytes: value length: len]];
}

static void
commentFunction(void *ctx, const unsigned char *value)
{
  NSCAssert(ctx,@"No Context");
  [HANDLER comment: UTF8Str(value)];
}

static void
warningFunction(void *ctx, const unsigned char *msg, ...)
{
  NSString	*estr;
  va_list	args;
  int		lineNumber = -1;
  int		colNumber = -1;

  va_start(args, msg);
  estr = [[[NSString alloc]
    initWithFormat: UTF8Str(msg) arguments: args] autorelease];
  va_end(args);

  NSCAssert(ctx,@"No Context");
  lineNumber = xmlSAX2GetLineNumber(ctx);
  colNumber = xmlSAX2GetColumnNumber(ctx);
  [HANDLER warning: estr
	 colNumber: colNumber
	lineNumber: lineNumber];
}

static void
errorFunction(void *ctx, const unsigned char *msg, ...)
{
  NSString	*estr;
  va_list	args;
  int		lineNumber = -1;
  int		colNumber = -1;

  va_start(args, msg);
  estr = [[[NSString alloc]
    initWithFormat: UTF8Str(msg) arguments: args] autorelease];
  va_end(args);
  NSCAssert(ctx,@"No Context");
  lineNumber = xmlSAX2GetLineNumber(ctx);
  colNumber = xmlSAX2GetColumnNumber(ctx);
  [HANDLER error: estr
       colNumber: colNumber
      lineNumber: lineNumber];
}

static void
fatalErrorFunction(void *ctx, const unsigned char *msg, ...)
{
  NSString	*estr;
  va_list	args;
  int		lineNumber = -1;
  int		colNumber = -1;

  va_start(args, msg);
  estr = [[[NSString alloc]
    initWithFormat: UTF8Str(msg) arguments: args] autorelease];
  va_end(args);
  NSCAssert(ctx, @"No Context");
  lineNumber = xmlSAX2GetLineNumber(ctx);
  colNumber = xmlSAX2GetColumnNumber(ctx);
  [HANDLER fatalError: estr
            colNumber: colNumber
           lineNumber: lineNumber];
}

#undef	HANDLER

/**
 * Create a new SAX handler.
 */
+ (GSSAXHandler*) handler
{
  return AUTORELEASE([[self alloc] init]);
}

- (id) init
{
  NSAssert(lib == 0, @"Already created lib");
  self = [super init];
  if (self != nil)
    {
      if ([self _initLibXML] == NO)
        {
          NSLog(@"GSSAXHandler: out of memory\n");
	  DESTROY(self);
	  return nil;
        }
    }
  return self;
}

/**
 * Returns a pointer to the raw libxml data used by this document.<br />
 * Only for use by libxml experts!
 */
- (void*) lib
{
  return lib;
}

/**
 * Return the parser object with which this handler is
 * associated.  This may occasionally be useful.
 */
- (GSXMLParser*) parser
{
  return parser;
}

- (void) dealloc
{
  if (lib != NULL)
    {
      free(lib);
    }
  [super dealloc];
}

/**
 * Called when the document starts being processed.
 */
- (void) startDocument
{
}

/**
 * Called when the document end has been detected.
 */
- (void) endDocument
{
}

/**
 * Called to detemrine if the document is standalone.
 */
- (NSInteger) isStandalone
{
  return 1;
}

/**
 * Called when an opening tag has been processed.
 */
- (void) startElement: (NSString*)elementName
	   attributes: (NSMutableDictionary*)elementAttributes
{
}

- (void) startElement: (NSString*)elementName
               prefix: (NSString*)prefix
		 href: (NSString*)href
	   attributes: (NSMutableDictionary*)elementAttributes
{
  [self startElement: elementName attributes: elementAttributes];
}

- (void) startElement: (NSString*)elementName
               prefix: (NSString*)prefix
		 href: (NSString*)href
	   attributes: (NSMutableDictionary*)elementAttributes
           namespaces: (NSMutableDictionary*)elementNamespaces
{
  [self startElement: elementName
              prefix: prefix
                href: href
          attributes: elementAttributes];
}

/**
 * Called when a closing tag has been processed.
 */
- (void) endElement: (NSString*)elementName
{
}

/**
 * Called when a closing tag has been processed.
 */
- (void) endElement: (NSString*)elementName
	     prefix: (NSString*)prefix
	       href: (NSString*)href
{
  [self endElement: elementName];
}

/**
 * Handle an attribute that has been read by the parser.
 */
- (void) attribute: (NSString*) name value: (NSString*)value
{
}

/**
 * Receiving some chars from the parser.
 */
- (void) characters: (NSString*) name
{
}

/**
 * Receiving some ignorable whitespaces from the parser.
 */
- (void) ignoreWhitespace: (NSString*) ch
{
}

/**
 * A processing instruction has been parsed.
 */
- (void) processInstruction: (NSString*)targetName data: (NSString*)PIdata
{
}

/**
 * A comment has been parsed.
 */
- (void) comment: (NSString*) value
{
}

/**
 * Called when a cdata block has been parsed.
 */
- (void) cdataBlock: (NSData*)value
{
}

/**
 * Called to return the filename from which an entity should be loaded.
 */
- (NSString*) loadEntity: (NSString*)publicId
		      at: (NSString*)location
{
  return nil;
}

/**
 * An old global namespace has been parsed.
 */
- (void) namespaceDecl: (NSString*)name
		  href: (NSString*)href
		prefix: (NSString*)prefix
{
}

/**
 * What to do when a notation declaration has been parsed.
 */
- (void) notationDecl: (NSString*)name
	       public: (NSString*)publicId
	       system: (NSString*)systemId
{
}

/**
 * An entity definition has been parsed.
 */
- (void) entityDecl: (NSString*)name
	       type: (NSInteger)type
	     public: (NSString*)publicId
	     system: (NSString*)systemId
	    content: (NSString*)content
{
}

/**
 * An attribute definition has been parsed.
 */
- (void) attributeDecl: (NSString*)nameElement
		  name: (NSString*)name
		  type: (NSInteger)type
	  typeDefValue: (NSInteger)defType
	  defaultValue: (NSString*)value
{
}

/**
 * An element definition has been parsed.
 */
- (void) elementDecl: (NSString*)name
		type: (NSInteger)type
{
}

/**
 * What to do when an unparsed entity declaration is parsed.
 */
- (void) unparsedEntityDecl: (NSString*)name
		     public: (NSString*)publicId
		     system: (NSString*)systemId
	       notationName: (NSString*)notation
{
}

/**
 * Called when an entity reference is detected.
 */
- (void) reference: (NSString*) name
{
}

/**
 * An old global namespace has been parsed.
 */
- (void) globalNamespace: (NSString*)name
		    href: (NSString*)href
		  prefix: (NSString*)prefix
{
}

/**
 * Called when a warning message needs to be output.
 */
- (void) warning: (NSString*)e
{
  GSPrintf(stderr, @"%@", e);
}

/**
 * Called when an error message needs to be output.
 */
- (void) error: (NSString*)e
{
  GSPrintf(stderr, @"%@", e);
}

/**
 * Called when a fatal error message needs to be output.
 */
- (void) fatalError: (NSString*)e
{
  GSPrintf(stderr, @"%@", e);
}

/**
 * Called when a warning message needs to be output.
 */
- (void) warning: (NSString*)e
       colNumber: (NSInteger)colNumber
      lineNumber: (NSInteger)lineNumber
{
  e = [NSString stringWithFormat:
    @"at line: %"PRIdPTR" column: %"PRIdPTR" ... %@",
    lineNumber, colNumber, e];
  [self warning: e];
}

/**
 * Called when an error message needs to be output.
 */
- (void) error: (NSString*)e
     colNumber: (NSInteger)colNumber
    lineNumber: (NSInteger)lineNumber
{
  e = [NSString stringWithFormat:
    @"at line: %"PRIdPTR" column: %"PRIdPTR" ... %@",
    lineNumber, colNumber, e];
  [self error: e];
}

/**
 * Called when a fatal error message needs to be output.
 */
- (void) fatalError: (NSString*)e
          colNumber: (NSInteger)colNumber
         lineNumber: (NSInteger)lineNumber
{
  e = [NSString stringWithFormat:
    @"at line: %"PRIdPTR" column: %"PRIdPTR" ... %@",
    lineNumber, colNumber, e];
  [self fatalError: e];
}

/**
 * Called to find out whether there is an internal subset.
 */
- (NSInteger) hasInternalSubset
{
  return 0;
}

/**
 * Called to find out whether there is an internal subset.
 */
- (BOOL) internalSubset: (NSString*)name
	     externalID: (NSString*)externalID
	       systemID: (NSString*)systemID
{
  return NO;
}

/**
 * Called to find out whether there is an external subset.
 */
- (NSInteger) hasExternalSubset
{
  return 0;
}

/**
 * Called to find out whether there is an external subset.
 */
- (BOOL) externalSubset: (NSString*)name
	     externalID: (NSString*)externalID
	       systemID: (NSString*)systemID
{
  return NO;
}

/**
 * get an entity by name
 */
- (void*) getEntity: (NSString*)name
{
  return 0;
}

/**
 * get a parameter entity by name
 */
- (void*) getParameterEntity: (NSString*)name
{
  return 0;
}

/*
 * Private methods - internal use only.
 */
- (BOOL) _initLibXML
{
  lib = (xmlSAXHandler*)malloc(sizeof(xmlSAXHandler));
  if (lib == NULL)
    {
      return NO;
    }
  else
    {
      memcpy(lib, &xmlDefaultSAXHandler, sizeof(xmlSAXHandler));

#define	LIB	((xmlSAXHandlerPtr)lib)
      /*
       * We must call xmlSAXVersion() BEFORE setting any functions as it
       * sets up default values and would trash our settings.
       */
      xmlSAXVersion(LIB, 2);	// Set SAX2
      LIB->startElementNs         = (void*) startElementNsFunction;
      LIB->endElementNs           = (void*) endElementNsFunction;
      LIB->startElement           = (void*) startElementFunction;
      LIB->endElement             = (void*) endElementFunction;
      LIB->internalSubset         = (void*) internalSubsetFunction;
      LIB->externalSubset         = (void*) externalSubsetFunction;
      LIB->isStandalone           = (void*) isStandaloneFunction;
      LIB->hasInternalSubset      = (void*) hasInternalSubsetFunction;
      LIB->hasExternalSubset      = (void*) hasExternalSubsetFunction;
      LIB->getEntity              = (void*) getEntityIgnoreExternal;
      LIB->entityDecl             = (void*) entityDeclFunction;
      LIB->notationDecl           = (void*) notationDeclFunction;
      LIB->attributeDecl          = (void*) attributeDeclFunction;
      LIB->elementDecl            = (void*) elementDeclFunction;
      LIB->unparsedEntityDecl     = (void*) unparsedEntityDeclFunction;
      LIB->startDocument          = (void*) startDocumentFunction;
      LIB->endDocument            = (void*) endDocumentFunction;
      LIB->reference              = (void*) referenceFunction;
      LIB->characters             = (void*) charactersFunction;
      LIB->ignorableWhitespace    = (void*) ignorableWhitespaceFunction;
      LIB->processingInstruction  = (void*) processingInstructionFunction;
      LIB->comment                = (void*) commentFunction;
      LIB->warning                = (void*) warningFunction;
      LIB->error                  = (void*) errorFunction;
      LIB->fatalError             = (void*) fatalErrorFunction;
      LIB->getParameterEntity     = (void*) getParameterEntityFunction;
      LIB->cdataBlock             = (void*) cdataBlockFunction;
      LIB->resolveEntity          = (void*) resolveEntityFunction;
#undef	LIB
      return YES;
    }
}

- (void) _setParser: (GSXMLParser*)value
{
  parser = value;
}
@end


/**
 * The default handler for parsing documents ... this will build a
 * GSXMLDocument for you. This handler may not currently be subclassed,
 * though that capability may be added at a later date.
 */
@implementation GSTreeSAXHandler

/**
 * Called when a warning message needs to be output.<br />
 * See [GSXMLParser-setErrors:] for the mechanism implemented by this.
 */
- (void) warning: (NSString*)e
{
  NSMutableString	*m = [parser _messages];

  if (m == nil)
    {
      GSPrintf(stderr, @"%@", e);
    }
  else
    {
      [m appendString: e];
    }
}

/**
 * Called when an error message needs to be output.<br />
 * See [GSXMLParser-setErrors:] for the mechanism implemented by this.
 */
- (void) error: (NSString*)e
{
  NSMutableString	*m = [parser _messages];

  if (m == nil)
    {
      GSPrintf(stderr, @"%@", e);
    }
  else
    {
      [m appendString: e];
    }
}

/**
 * Called when a fatal error message needs to be output.<br />
 * See [GSXMLParser-setErrors:] for the mechanism implemented by this.
 */
- (void) fatalError: (NSString*)e
{
  NSMutableString	*m = [parser _messages];

  if (m == nil)
    {
      GSPrintf(stderr, @"%@", e);
    }
  else
    {
      [m appendString: e];
    }
}


- (BOOL) _initLibXML
{
  lib = (xmlSAXHandler*)malloc(sizeof(xmlSAXHandler));
  if (lib == NULL)
    {
      return NO;
    }
  else
    {
      memcpy(lib, &xmlDefaultSAXHandler, sizeof(xmlSAXHandler));

#define	LIB	((xmlSAXHandlerPtr)lib)
#define	SETCB(NAME,SEL) if ([self methodForSelector: @selector(SEL)] != [treeClass instanceMethodForSelector: @selector(SEL)]) LIB->NAME = (void*)NAME ## Function
      /*
       * We must call xmlSAXVersion() BEFORE setting any functions as it
       * sets up default values and would trash our settings.
       */
      xmlSAXVersion(LIB, 2);	// Set SAX2
      SETCB(startElementNs, startElement:prefix:href:attributes:);
      SETCB(endElementNs, endElement:prefix:href:);
      SETCB(startElement, startElement:attributes:);
      SETCB(endElement, endElement:);
      SETCB(internalSubset, internalSubset:externalID:systemID:);
      SETCB(externalSubset, externalSubset:externalID:systemID:);
      SETCB(isStandalone, isStandalone);
      SETCB(hasInternalSubset, hasInternalSubset);
      SETCB(hasExternalSubset, hasExternalSubset);
      SETCB(getEntity, getEntity:);
      if (LIB->getEntity != getEntityFunction)
        {
          LIB->getEntity = getEntityIgnoreExternal;
        }
      SETCB(entityDecl, entityDecl:type:public:system:content:);
      SETCB(notationDecl, notationDecl:public:system:);
      SETCB(attributeDecl, attributeDecl:name:type:typeDefValue:defaultValue:);
      SETCB(elementDecl, elementDecl:type:);
      SETCB(unparsedEntityDecl, unparsedEntityDecl:public:system:notationName:);
      SETCB(startDocument, startDocument);
      SETCB(endDocument, endDocument);
      SETCB(reference, reference:);
      SETCB(characters, characters:);
      SETCB(ignorableWhitespace, ignoreWhitespace:);
      SETCB(processingInstruction, processInstruction:data:);
      SETCB(comment, comment:);
      SETCB(getParameterEntity, getParameterEntity:);
      SETCB(cdataBlock, cdataBlock:);

      LIB->warning                = (void*)warningFunction;
      LIB->error                  = (void*)errorFunction;
      LIB->fatalError             = (void*)fatalErrorFunction;

#undef	LIB
      return YES;
    }
}
@end



/**
 * You may create a subclass of this class to handle incremental parsing
 * of html documents ... this is provided for handling legacy documents,
 * as modern html documents should use xhtml, and for those you should
 * simply subclass [GSSAXHandler]
 */
@implementation GSHTMLSAXHandler
- (BOOL) _initLibXML
{
  isHtmlHandler = YES;
  lib = (xmlSAXHandler*)malloc(sizeof(htmlSAXHandler));
  if (lib == NULL)
    {
      return NO;
    }
  else
    {
      memcpy(lib, &htmlDefaultSAXHandler, sizeof(htmlSAXHandler));

#define	LIB	((htmlSAXHandlerPtr)lib)
      LIB->internalSubset         = (void*)internalSubsetFunction;
      LIB->externalSubset         = (void*)externalSubsetFunction;
      LIB->isStandalone           = (void*)isStandaloneFunction;
      LIB->hasInternalSubset      = (void*)hasInternalSubsetFunction;
      LIB->hasExternalSubset      = (void*)hasExternalSubsetFunction;
      LIB->getEntity              = (void*)getEntityFunction;
      LIB->entityDecl             = (void*)entityDeclFunction;
      LIB->notationDecl           = (void*)notationDeclFunction;
      LIB->attributeDecl          = (void*)attributeDeclFunction;
      LIB->elementDecl            = (void*)elementDeclFunction;
      LIB->unparsedEntityDecl     = (void*)unparsedEntityDeclFunction;
      LIB->startDocument          = (void*)startDocumentFunction;
      LIB->endDocument            = (void*)endDocumentFunction;
      LIB->startElement           = (void*)startElementFunction;
      LIB->endElement             = (void*)endElementFunction;
      LIB->reference              = (void*)referenceFunction;
      LIB->characters             = (void*)charactersFunction;
      LIB->ignorableWhitespace    = (void*)ignorableWhitespaceFunction;
      LIB->processingInstruction  = (void*)processingInstructionFunction;
      LIB->comment                = (void*)commentFunction;
      LIB->warning                = (void*)warningFunction;
      LIB->error                  = (void*)errorFunction;
      LIB->fatalError             = (void*)fatalErrorFunction;
      LIB->getParameterEntity     = (void*)getParameterEntityFunction;
      LIB->cdataBlock             = (void*)cdataBlockFunction;
#undef	LIB
      return YES;
    }
}
@end



/**
 * <p>You don't create GSXPathObject instances, instead the XPATH system
 * creates them and returns them as the result of the
 * [GSXPathContext-evaluateExpression:] method.
 * </p>
 */
@implementation GSXPathObject
- (id) init
{
  DESTROY(self);
  return nil;
}

/* Internal method.  */
- (id) _initWithNativePointer: (xmlXPathObject *)lib
		      context: (GSXPathContext *)context
{
  _lib = lib;
  /* We RETAIN our context because we might be holding references to nodes
   * which belong to the document, and we must make sure the document is
   * not freed before we are.  */
  ASSIGN (_context, context);
  return self;
}

/* This method is called by GSXPathContext when creating a
 * GSXPathObject to wrap the results of a query.  It assumes that lib
 * is a pointer created by xmlXPathEval (), and that we are now taking
 * on responsibility for freeing it.  It then examines lib, and
 * replaces itself with an object of the appropriate subclass.  */
+ (id) _newWithNativePointer: (xmlXPathObject *)lib
		     context: (GSXPathContext *)context
{
  switch (lib->type)
    {
      case XPATH_NODESET:
	return [[GSXPathNodeSet alloc] _initWithNativePointer: lib
						      context: context];
	break;
      case XPATH_BOOLEAN:
	return [[GSXPathBoolean alloc] _initWithNativePointer: lib
						      context: context];
	break;
      case XPATH_NUMBER:
	return [[GSXPathNumber alloc] _initWithNativePointer: lib
						     context: context];
	break;
      case XPATH_STRING:
	return [[GSXPathString alloc] _initWithNativePointer: lib
						     context: context];
	break;
      default:
	/* This includes:
	   case XPATH_UNDEFINED:
	   case XPATH_POINT:
	   case XPATH_RANGE:
	   case XPATH_LOCATIONSET:
	   case XPATH_USERS:
	   case XPATH_XSLT_TREE:
	*/
	return [[self alloc] _initWithNativePointer: lib  context: context];
    }
}

- (void) dealloc
{
  xmlXPathFreeObject (_lib);
  RELEASE (_context);
  [super dealloc];
}
@end

@implementation GSXPathBoolean
/**
 * Returns the the value of the receiver ... YES/NO, true/false.
 */
- (BOOL) booleanValue
{
  return ((xmlXPathObject*)_lib)->boolval;
}
- (NSString *) description
{
  return ([self booleanValue] ? @"true" : @"false");
}
@end

@implementation GSXPathNumber
/**
 * Returns the floating point (double) value of the receiver.
 */
- (double) doubleValue
{
  return ((xmlXPathObject*)_lib)->floatval;
}
- (NSString *) description
{
  return [NSString_class stringWithFormat: @"%f", [self doubleValue]];
}
@end

@implementation GSXPathString
/**
 * Returns the string value of the receiver.
 */
- (NSString *) stringValue
{
  xmlChar *string = ((xmlXPathObject*)_lib)->stringval;
  return [NSString_class stringWithUTF8String: (const char*)string];
}
- (NSString *) description
{
  return [NSString_class stringWithFormat: @"%@", [self stringValue]];
}
@end


/**
 * An XPATH node set is an ordered set of nodes returned as a result of
 * an expression. The order of the nodes in the set is the same as the
 * order in the xml document from which they were extracted.
 */
@implementation GSXPathNodeSet
/**
 * Returns the number of nodes in the receiver.
 */
- (NSUInteger) count
{
  if (xmlXPathNodeSetIsEmpty (((xmlXPathObject*)_lib)->nodesetval))
    {
      return 0;
    }
  return xmlXPathNodeSetGetLength (((xmlXPathObject*)_lib)->nodesetval);
}

/**
 * Deprecated
 */
- (NSUInteger) length
{
  if (xmlXPathNodeSetIsEmpty (((xmlXPathObject*)_lib)->nodesetval))
    {
      return 0;
    }

  return xmlXPathNodeSetGetLength (((xmlXPathObject*)_lib)->nodesetval);
}

/**
 * Returns the node from the receiver at the specified index, or nil
 * if no such node exists.
 */
- (GSXMLNode *) nodeAtIndex: (NSUInteger)index
{
  if (xmlXPathNodeSetIsEmpty (((xmlXPathObject*)_lib)->nodesetval))
    {
      return nil;
    }
  else
    {
      xmlNode	*node;
      GSXMLNode *n;

      node = xmlXPathNodeSetItem (((xmlXPathObject*)_lib)->nodesetval,
	(NSInteger)index);
      n = [GSXMLNode alloc];
      n = [n _initFrom: node  parent: self];
      return AUTORELEASE(n);
    }
}
- (NSString *) description
{
  return [NSString_class stringWithFormat:
    @"NodeSet (count %"PRIuPTR")", [self count]];
}
@end


/**
 * <p>Use of the GSXPathContext class is simple ... you just need to
 * look up xpath to learn the syntax of xpath expressions, then you
 * can apply those expressions to a context to retrieve data from a
 * document.
 * </p>
 * <example>
 * GSXMLParser       *p = [GSXMLParser parserWithContentsOfFile: @"xp.xml"];
 *
 * if ([p parse])
 *   {
 *     GSXMLDocument *d = [p document];
 *     GSXPathContext *c = [[GSXPathContext alloc] initWithDocument: document];
 *     GSXPathString *result = [c evaluateExpression: @"string(/body/text())"];
 *
 *     GSPrintf(stdout, @"Got %@", [result stringValue]);
 *   }
 * else
 *   {
 *     GSPrintf(stderr, "error parsing file\n");
 *   }
 * </example>
 */
@implementation GSXPathContext
/**
 * Initialises the receiver as an xpath parser for the supplied document.
 */
- (id) initWithDocument: (GSXMLDocument *)d
{
  ASSIGN (_document, d);
  _lib = xmlXPathNewContext ([_document lib]);
  ((xmlXPathContext*)_lib)->node = xmlDocGetRootElement ([_document lib]);

  return self;
}

/**
 * Evaluates the supplied expression and returns the resulting node or
 * node set.  If the expression is invalid, returns nil.
 */
- (GSXPathObject *) evaluateExpression: (NSString *)XPathExpression
{
  xmlXPathCompExpr *comp;
  xmlXPathObject   *res;
  GSXPathObject *result;

  comp = xmlXPathCompile (UTF8STRING(XPathExpression));
  if (comp == NULL)
    {
      /* Maybe an exception would be better ? */
      return nil;
    }

  res = xmlXPathCompiledEval (comp, ((xmlXPathContext*)_lib));
  if (res == NULL)
    {
      result = nil;
    }
  else
    {
      result = [GSXPathObject _newWithNativePointer: res  context: self];
      IF_NO_GC ([result autorelease];)
    }
  xmlXPathFreeCompExpr (comp);

  return result;
}

- (BOOL) registerNamespaceWithPrefix: (NSString *)prefix
				href: (NSString *)href
{
  if (xmlXPathRegisterNs (_lib, UTF8STRING(prefix), UTF8STRING(href)) != 0)
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (void) dealloc
{
  xmlXPathFreeContext (_lib);
  RELEASE (_document);
  [super dealloc];
}
@end



/*
 * need this to make the linker happy on Windows
 */
@interface GSXMLDummy : NSObject
@end
@implementation GSXMLDummy
@end


@implementation GSXMLNode (Deprecated)
- (GSXMLNode*) childElement { static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); } return [self firstChildElement]; }
- (GSXMLNode*) children { static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); } return [self firstChild]; }
- (GSXMLDocument*) doc { static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); } return [self document]; }
- (GSXMLNamespace*) ns { static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); } return [self namespace]; }
- (GSXMLNamespace*) nsDefs { static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); } return [self namespaceDefinitions]; }
- (GSXMLNode*) prev { static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); } return [self previous]; }
- (NSMutableDictionary*) propertiesAsDictionary
{
static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); }
  return [self propertiesAsDictionaryWithKeyTransformationSel: NULL];
}

@end
@implementation GSXMLParser (Deprecated)
- (GSXMLDocument*) doc { static BOOL warned = NO; if (warned == NO) { warned = YES; NSLog(@"WARNING, use of deprecated method ... [%@ -%@]", NSStringFromClass([self class]), NSStringFromSelector(_cmd)); } return [self document]; }
@end

@implementation GSXMLDocument (XSLT)
#ifdef HAVE_LIBXSLT
/**
 * Performs an XSLT transformation on the specified file using the
 * stylesheet provided.<br />
 *
 * Returns an autoreleased GSXMLDocument containing the transformed
 * XML, or nil on failure.
 */
+ (GSXMLDocument*) xsltTransformFile: (NSString*)xmlFile
                          stylesheet: (NSString*)xsltStylesheet
{
  return [GSXMLDocument xsltTransformFile: xmlFile
                               stylesheet: xsltStylesheet
			           params: nil];
}

/**
 * Performs an XSLT transformation on the specified file using the
 * stylesheet and parameters provided. See the libxslt documentation
 * for details of the supported parameters.<br />
 *
 * Returns an autoreleased GSXMLDocument containing the transformed
 * XML, or nil on failure.
 */
+ (GSXMLDocument*) xsltTransformFile: (NSString*)xmlFile
                          stylesheet: (NSString*)xsltStylesheet
		              params: (NSDictionary*)params
{
  GSXMLDocument	*newdoc;

  NS_DURING
    {
      NSData	*xml;
      NSData	*ss;

      xml = [NSData dataWithContentsOfFile: xmlFile];
      ss = [NSData dataWithContentsOfFile: xsltStylesheet];
      if (xml == nil || ss == nil)
	{
	  newdoc = nil;
	}
      else
	{
	  newdoc = [GSXMLDocument xsltTransformXml: xml
					stylesheet: ss
					    params: params];
	}
    }
  NS_HANDLER
    {
      newdoc = nil;
    }
  NS_ENDHANDLER

  return newdoc;
}
/**
 * Performs an XSLT transformation on the specified file using the
 * stylesheet provided.<br />
 *
 * Returns an autoreleased GSXMLDocument containing the transformed
 * XML, or nil on failure.
 */
+ (GSXMLDocument*) xsltTransformXml: (NSData*)xmlData
                         stylesheet: (NSData*)xsltStylesheet
{
  return [GSXMLDocument xsltTransformXml: xmlData
                              stylesheet: xsltStylesheet
			          params: nil];
}
			
/**
 * Performs an XSLT transformation on the specified file using the
 * stylesheet and parameters provided.See the libxslt documentation
 * for details of the supported parameters.<br />
 *
 * Returns an autoreleased GSXMLDocument containing the transformed
 * XML, or nil on failure.
 */
+ (GSXMLDocument*) xsltTransformXml: (NSData*)xmlData
                         stylesheet: (NSData*)xsltStylesheet
		             params: (NSDictionary*)params
{
  GSXMLDocument	*newdoc;

  NS_DURING
    {
      GSXMLParser	*xmlParser;
      GSXMLDocument	*xml;
      GSXMLParser	*ssParser;
      GSXMLDocument	*ss;

      newdoc = nil;
      xmlParser = [GSXMLParser parserWithData: xmlData];
      if ([xmlParser parse] == YES)
	{
	  xml = [xmlParser document];
	  ssParser = [GSXMLParser parserWithData: xsltStylesheet];
	  if ([ssParser parse] == YES)
	    {
	      ss = [ssParser document];
	      newdoc = [xml xsltTransform: ss params: params];
	    }
	}
    }
  NS_HANDLER
    {
      newdoc = nil;
    }
  NS_ENDHANDLER

  return newdoc;
}

/**
 * Performs an XSLT transformation on the current document using the
 * supplied stylesheet.<br />
 *
 * Returns an autoreleased GSXMLDocument containing the transformed
 * XML, or nil on failure.
 */
- (GSXMLDocument*) xsltTransform: (GSXMLDocument*)xsltStylesheet
{
  return [self xsltTransform: xsltStylesheet params: nil];
}

/**
 * Performs an XSLT transformation on the current document using the
 * supplied stylesheet and paramaters (params may be nil).
 * See the libxslt documentation for details of the supported parameters.<br />
 *
 * Returns an autoreleased GSXMLDocument containing the transformed
 * XML, or nil on failure.
 */
- (GSXMLDocument*) xsltTransform: (GSXMLDocument*)xsltStylesheet
                          params: (NSDictionary*)params
{
  GSXMLDocument		*newdoc = nil;

  NS_DURING
    {
      xsltStylesheetPtr ss = NULL;
      xmlDocPtr		ssXml = (xmlDocPtr)[xsltStylesheet lib];
      int		pSize = params == nil ? 1 : ([params count] * 2) + 1;
      int		pNum = 0;
      const char	*parameters[pSize];

      if (params != nil)
	{
	  NSEnumerator	*keys = [params keyEnumerator];
	  if (keys != nil)
	    {
	      NSString	*key = [keys nextObject];
	      while (key != nil)
		{
		  NSString	*value = [params objectForKey: key];
		  parameters[pNum++] = [key cString];
		  parameters[pNum++] = [value cString];
		  key = [keys nextObject];
		}
	    }
	}
      parameters[pNum] = NULL;

      ss = xsltParseStylesheetDoc(ssXml);
      if (xsltStylesheet != NULL)
	{
	  xmlDocPtr	res = NULL;

	  res = xsltApplyStylesheet(ss, lib, parameters);
	  if (res != NULL)
	    {
	      newdoc = [GSXMLDocument alloc];
	      newdoc = [newdoc _initFrom: res
				  parent: self
				 ownsLib: YES];
	      IF_NO_GC([newdoc autorelease];)
	    }
	}
      /*
       * N.B. We don't want to call xsltFreeStylesheet() to free the
       * stylesheet xmlDocPtr because that will destroy the lib which
       * is owned by the xsltStylesheet object.
       */
      xsltCleanupGlobals();
    }
  NS_HANDLER
    {
      newdoc=  nil;
    }
  NS_ENDHANDLER
  return newdoc;
}
#else /* HAVE_LIBXSLT */
+ (GSXMLDocument*) xsltTransformFile: (NSString*)xmlFile
                          stylesheet: (NSString*)xsltStylesheet
		              params: (NSDictionary*)params
{
  NSLog(@"libxslt is not available");
  return nil;
}
			
+ (GSXMLDocument*) xsltTransformFile: (NSString*)xmlFile
                          stylesheet: (NSString*)xsltStylesheet
{
  NSLog(@"libxslt is not available");
  return nil;
}
			
+ (GSXMLDocument*) xsltTransformXml: (NSData*)xmlData
                         stylesheet: (NSData*)xsltStylesheet
		             params: (NSDictionary*)params
{
  NSLog(@"libxslt is not available");
  return nil;
}

+ (GSXMLDocument*) xsltTransformXml: (NSData*)xmlData
                         stylesheet: (NSData*)xsltStylesheet
{
  NSLog(@"libxslt is not available");
  return nil;
}
			
- (GSXMLDocument*) xsltTransform: (GSXMLDocument*)xsltStylesheet
                          params: (NSDictionary*)params
{
  NSLog(@"libxslt is not available");
  return nil;
}

- (GSXMLDocument*) xsltTransform: (GSXMLDocument*)xsltStylesheet
{
  NSLog(@"libxslt is not available");
  return nil;
}
#endif /* HAVE_LIBXSLT */
@end

#else

#ifndef NeXT_Foundation_LIBRARY
#import	"Foundation/NSCoder.h"
#import	"Foundation/NSInvocation.h"
#endif

/*
 * Build dummy implementations of the classes if libxml is not available
 */
GS_EXPORT_CLASS
@interface GSXMLDummy : NSObject
@end

GS_EXPORT_CLASS
@interface GSXMLDocument : GSXMLDummy
@end

GS_EXPORT_CLASS
@interface GSXMLNamespace : GSXMLDummy
@end

GS_EXPORT_CLASS
@interface GSXMLNode : GSXMLDummy
@end

GS_EXPORT_CLASS
@interface GSSAXHandler : GSXMLDummy
@end

GS_EXPORT_CLASS
@interface GSXMLParser : GSXMLDummy
@end

GS_EXPORT_CLASS
@interface GSXMLAttribute : GSXMLNode
@end

@implementation GSXMLDummy
+ (id) allocWithZone: (NSZone*)z
{
  NSLog(@"Not built with libxml ... %@ unusable in %@",
    NSStringFromClass(self), NSStringFromSelector(_cmd));
  return nil;
}
+ (void) forwardInvocation: (NSInvocation*)anInvocation
{
  NSLog(@"Not built with libxml ... %@ unusable in %@",
    NSStringFromClass([self class]),
    NSStringFromSelector([anInvocation selector]));
  return;
}
- (id) init
{
  NSLog(@"Not built with libxml ... %@ unusable in %@",
    NSStringFromClass([self class]), NSStringFromSelector(_cmd));
  DESTROY(self);
  return nil;
}
- (id) initWithCoder: (NSCoder*)aCoder
{
  NSLog(@"Not built with libxml ... %@ unusable in %@",
    NSStringFromClass([self class]), NSStringFromSelector(_cmd));
  DESTROY(self);
  return nil;
}
@end

GS_EXPORT_CLASS
@implementation GSXMLDocument
@end

GS_EXPORT_CLASS
@implementation GSXMLNamespace
@end

GS_EXPORT_CLASS
@implementation GSXMLNode
@end

GS_EXPORT_CLASS
@implementation GSSAXHandler
@end

GS_EXPORT_CLASS
@implementation GSXMLParser
@end

GS_EXPORT_CLASS
@implementation GSXMLAttribute
@end

#endif



@implementation	NSString (GSXML)
- (NSString*) stringByEscapingXML
{
  unsigned	length = [self length];
  unsigned	output = 0;
  unichar	*from;
  unsigned	i = 0;
  BOOL		escape = NO;

  from = NSZoneMalloc (NSDefaultMallocZone(), sizeof(unichar) * length);
  [self getCharacters: from];

  for (i = 0; i < length; i++)
    {
      unichar	c = from[i];

      if ((c >= 0x20 && c <= 0xd7ff)
	|| c == 0x9 || c == 0xd || c == 0xa
	|| (c >= 0xe000 && c <= 0xfffd))
	{
	  switch (c)
	    {
	      case '"':
	      case '\'':
		output += 6;
		escape = YES;
	        break;

	      case '&':
		output += 5;
		escape = YES;
	        break;

	      case '<':
	      case '>':
		output += 4;
		escape = YES;
	        break;

	      default:
		/*
		 * For non-ascii characters, we can use &#nnnn; escapes
		 */
		if (c > 127)
		  {
		    output += 5;
		    while (c >= 1000)
		      {
			output++;
			c /= 10;
		      }
		    escape = YES;
		  }
		output++;
		break;
	    }
	}
      else
	{
	  escape = YES;	// Need to remove bad characters
	}
    }

  if (escape == YES)
    {
      unichar	*to;
      unsigned	j = 0;

      to = NSZoneMalloc (NSDefaultMallocZone(), sizeof(unichar) * output);

      for (i = 0; i < length; i++)
	{
	  unichar	c = from[i];

	  if ((c >= 0x20 && c <= 0xd7ff)
	    || c == 0x9 || c == 0xd || c == 0xa
	    || (c >= 0xe000 && c <= 0xfffd))
	    {
	      switch (c)
		{
		  case '"':
		    to[j++] = '&';
		    to[j++] = 'q';
		    to[j++] = 'u';
		    to[j++] = 'o';
		    to[j++] = 't';
		    to[j++] = ';';
		    break;

		  case '\'':
		    to[j++] = '&';
		    to[j++] = 'a';
		    to[j++] = 'p';
		    to[j++] = 'o';
		    to[j++] = 's';
		    to[j++] = ';';
		    break;

		  case '&':
		    to[j++] = '&';
		    to[j++] = 'a';
		    to[j++] = 'm';
		    to[j++] = 'p';
		    to[j++] = ';';
		    break;

		  case '<':
		    to[j++] = '&';
		    to[j++] = 'l';
		    to[j++] = 't';
		    to[j++] = ';';
		    break;

		  case '>':
		    to[j++] = '&';
		    to[j++] = 'g';
		    to[j++] = 't';
		    to[j++] = ';';
		    break;

		  default:
		    if (c > 127)
		      {
			char	buf[12];
			char	*ptr = buf;

			to[j++] = '&';
			to[j++] = '#';
			snprintf(buf, sizeof(buf), "%u", c);
			while (*ptr != '\0')
			  {
			    to[j++] = *ptr++;
			  }
			to[j++] = ';';
		      }
		    else
		      {
			to[j++] = c;
		      }
		    break;
		}
	    }
	}
      self = [[NSString alloc] initWithCharacters: to length: output];
      NSZoneFree (NSDefaultMallocZone (), to);
      IF_NO_GC([self autorelease];)
    }
  else
    {
      self = AUTORELEASE([self copyWithZone: NSDefaultMallocZone()]);
    }
  NSZoneFree (NSDefaultMallocZone (), from);
  return self;
}

- (NSString*) stringByUnescapingXML
{
  unsigned		length = [self length];
  NSRange		r = NSMakeRange(0, length);

  r = [self rangeOfString: @"&" options: NSLiteralSearch range: r];
  if (r.length > 0)
    {
      NSMutableString	*m = [self mutableCopy];

      while (r.length > 0)
	{
	  NSRange	e;
	  unsigned	s0 = NSMaxRange(r);

	  e = [m rangeOfString: @";"
		       options: NSLiteralSearch
			 range: NSMakeRange(s0, length - s0)];
	  if (e.length > 0)
	    {
	      unsigned	s1 = NSMaxRange(e);
	      NSString	*s = [m substringWithRange: NSMakeRange(s0, s1 - s0)];

	      if ([s hasPrefix: @"&#"] == YES)
		{
		  unichar	u;

		  if ([s hasPrefix: @"&#x"] || [s hasPrefix: @"&#X"])
		    {
		      unsigned	val = 0;

		      s = [s substringFromIndex: 3];
		      sscanf([s UTF8String], "%x", &val);
		      u = val;
		    }
		  else if ([s hasPrefix: @"&#0x"] || [s hasPrefix: @"&#0X"])
		    {
		      unsigned	val = 0;

		      s = [s substringFromIndex: 4];
		      sscanf([s UTF8String], "%x", &val);
		      u = val;
		    }
		  else
		    {
		      s = [s substringFromIndex: 2];
		      u = [s intValue];
		    }
		  if (u == 0)
		    {
		      u = ' ';
		    }
		  s = [[NSString alloc] initWithCharacters: &u length: 1];
		  s = AUTORELEASE(s);
		}
	      else if ([s isEqualToString: @"amp"])
		{
		  s = @"&";
		}
	      else if ([s isEqualToString: @"apos"])
		{
		  s = @"'";
		}
	      else if ([s isEqualToString: @"quot"])
		{
		  s = @"\"";
		}
	      else if ([s isEqualToString: @"lt"])
		{
		  s = @"<";
		}
	      else if ([s isEqualToString: @"gt"])
		{
		  s = @">";
		}
	      else
		{
		  // Unknown escape ... don't change.
		  s = [NSString stringWithFormat: @"&%@;", s];
		}


	      [m replaceCharactersInRange: NSMakeRange(s0, s1 - s0)
			       withString: s];
	      r.length = [s length];
	      length += r.length - (s1 - s0);
	      r.location = NSMaxRange(r);
	      r.length = length - r.location;
	      r = [m rangeOfString: @"&" options: NSLiteralSearch range: r];
	    }
	  else
	    {
	      r.length = 0;
	    }
	}
      self = AUTORELEASE(m);
    }
  else
    {
      self = AUTORELEASE([self copyWithZone: NSDefaultMallocZone()]);
    }

  return self;
}
@end



#ifdef	HAVE_LIBXML

/*
 * Categories on other classes which are required for XMLRPC
 */
@interface	NSArray (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc;
@end

@interface	NSData (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc;
@end

@interface	NSDate (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc;
@end

@interface	NSDictionary (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc;
@end

@interface	NSObject (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc;
@end

@interface	NSNumber (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc;
@end

@interface	NSString (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc;
@end




/*
 * A little code to handle indentation.
 */
static NSString	*indentations[] = {
  @"  ",
  @"    ",
  @"      ",
  @"\t",
  @"\t  ",
  @"\t    ",
  @"\t      ",
  @"\t\t",
  @"\t\t  ",
  @"\t\t    ",
  @"\t\t      ",
  @"\t\t\t",
  @"\t\t\t  ",
  @"\t\t\t    ",
  @"\t\t\t      ",
  @"\t\t\t\t"
};
static void indentation(unsigned level, NSMutableString *str)
{
  if (level > 0)
    {
      if (level >= sizeof(indentations)/sizeof(*indentations))
	{
	  level = sizeof(indentations)/sizeof(*indentations) - 1;
	}
      [str appendString: indentations[level]];
    }
}

#define	INDENT(I)	if (compact == NO) indentation(I, str)
#define	NL		if (compact == NO) [str appendString: @"\n"]



/*
 * Implementation of categories to output objects for XMLRPC
 */

@implementation	NSArray (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc
{
  unsigned 		i;
  unsigned		c = [self count];
  BOOL			compact = [rpc compact];
  
  INDENT(indent++);
  [str appendString: @"<array>"];
  NL;
  INDENT(indent++);
  [str appendString: @"<data>"];
  NL;
  for (i = 0; i < c; i++)
    {
      id	value = [self objectAtIndex: i];

      INDENT(indent++);
      [str appendString: @"<value>"];
      NL;
      [value appendToXMLRPC: str indent: indent for: rpc];
      NL;
      INDENT(--indent);
      [str appendString: @"</value>"];
      NL;
    }
  INDENT(--indent);
  [str appendString: @"</data>"];
  NL;
  INDENT(--indent);
  [str appendString: @"</array>"];
}
@end

@implementation	NSData (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc
{
  NSData	*d;
  NSString	*s;

  d = [GSMimeDocument encodeBase64: self];
  s = [[NSString alloc] initWithData: d encoding: NSASCIIStringEncoding];
  [str appendString: @"<base64>"];
  [str appendString: s];
  [str appendString: @"</base64>"];
  RELEASE(s);
}
@end

@implementation	NSDate (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc
{
  NSString		*s;

  s = [self descriptionWithCalendarFormat: @"%Y%m%dT%H:%M:%S"
				 timeZone: [rpc timeZone]
				   locale: nil];
  [str appendString: @"<dateTime.iso8601>"];
  [str appendString: s];
  [str appendString: @"</dateTime.iso8601>"];
}
@end

@implementation	NSDictionary (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc
{
  NSEnumerator	*kEnum = [self keyEnumerator];
  NSString	*key;
  BOOL		compact = [rpc compact];

  INDENT(indent++);
  [str appendString: @"<struct>"];
  NL;
  while ((key = [kEnum nextObject]))
    {
      id	value = [self objectForKey: key];

      INDENT(indent++);
      [str appendString: @"<member>"];
      NL;
      INDENT(indent);
      [str appendString: @"<name>"];
      [str appendString: [[key description] stringByEscapingXML]];
      [str appendString: @"</name>"];
      NL;
      INDENT(indent++);
      [str appendString: @"<value>"];
      NL;
      [value appendToXMLRPC: str indent: indent-- for: rpc];
      NL;
      INDENT(indent--);
      [str appendString: @"</value>"];
      NL;
      INDENT(indent);
      [str appendString: @"</member>"];
      NL;
    }
  INDENT(--indent);
  [str appendString: @"</struct>"];
}
@end

@implementation	NSNumber (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc
{
  const char	*t = [self objCType];
  BOOL		compact = [rpc compact];

  INDENT(indent);
  if (strchr("cCsSiIlLqQ", *t) != 0)
    {
      int64_t	i = [self longLongValue];

      if ((i & 0xffffffff) != i)
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"Can't encode %"PRId64" as i4", i];
	}
      if ((i == 0 || i == 1) && (*t == 'c' || *t == 'C'))
        {
	  if (i == 0)
	    {
	      [str appendString: @"<boolean>0</boolean>"];
	    }
	  else
	    {
	      [str appendString: @"<boolean>1</boolean>"];
	    }
	}
      else
	{
	  [str appendFormat: @"<i4>%"PRId32"</i4>", (int32_t)i];
	}
    }
  else
    {
      [str appendFormat: @"<double>%f</double>", [self doubleValue]];
    }
}
@end

@implementation	NSObject (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc
{
  [[self description] appendToXMLRPC: str indent: indent for: rpc];
}
@end

@implementation	NSString (GSXMLRPC)
- (void) appendToXMLRPC: (NSMutableString*)str
		 indent: (NSUInteger)indent
		    for: (GSXMLRPC*)rpc
{
  BOOL	compact = [rpc compact];

  if (compact == YES)
    {
      [str appendString: [self stringByEscapingXML]];
    }
  else
    {
      INDENT(indent);
      [str appendFormat: @"<string>%@</string>", [self stringByEscapingXML]];
    }
}
@end



/*
 * Convert incoming XMLRPC value to a normal Objective-C object.
 */
@interface	GSXMLRPC (Private)
- (id) _parseValue: (GSXMLNode*)node;
@end

@implementation	GSXMLRPC (Private)
- (id) _parseValue: (GSXMLNode*)node
{
  NSString	*name = [node name];
  NSString	*str;

  if ([name isEqualToString: @"value"])
    {
      GSXMLNode	*type = [node firstChildElement];

      /*
       * A value with no type element is just a string.
       */
      if (type == nil)
	{
	  name = @"string";
	}
      else
	{
	  node = type;
	  name = [node name];
	}
    }

  if ([name length] == 0)
    {
      return nil;
    }

  if ([name isEqualToString: @"i4"] || [name isEqualToString: @"int"])
    {
      str = [node content];
      if (str == nil)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"missing %@ value", name];
	}
      return [NSNumber numberWithInt: [str intValue]];
    }

  if ([name isEqualToString: @"string"])
    {
      str = [node content];
      if (str == nil)
	{
	  str = @"";
	}
      return str;
    }

  if ([name isEqualToString: @"boolean"])
    {
      char	c;

      str = [node content];
      if (str == nil)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"missing %@ value", name];
	}
      c = [str intValue];
      return [NSNumber numberWithBool: c == 0 ? NO : YES];
    }

  if ([name isEqualToString: @"double"])
    {
      str = [node content];
      if (str == nil)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"missing %@ value", name];
	}
      return [NSNumber numberWithDouble: [str doubleValue]];
    }

  if ([name isEqualToString: @"base64"])
    {
      NSData	*d;

      str = [node content];
      if (str == nil)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"missing %@ value", name];
	}
      d = [str dataUsingEncoding: NSASCIIStringEncoding];
      return [GSMimeDocument decodeBase64: d];
    }

  if ([name isEqualToString: @"dateTime.iso8601"])
    {
      NSCalendarDate	*d;
      const char	*s;
      int		year;
      int		month;
      int		day;
      int		hour;
      int		minute;
      int		second;

      str = [node content];
      if (str == nil)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"missing %@ value", name];
	}
      s = [str UTF8String];
      if (sscanf(s, "%04d%02d%02dT%02d:%02d:%02d",
        &year, &month, &day, &hour, &minute, &second) != 6)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"bad date/time format '%@'", str];
	}
      d = [[NSCalendarDate alloc] initWithYear: year
					 month: month
					   day: day
					  hour: hour
					minute: minute
					second: second 
				      timeZone: tz]; 
      return AUTORELEASE(d);
    }

  if ([name isEqualToString: @"array"])
    {
      NSMutableArray		*arr = [NSMutableArray array];

      node = [node firstChildElement];
      while (node != nil && [[node name] isEqualToString: @"data"] == NO)
        {
	  node = [node nextElement];
	}
      if ([[node name] isEqualToString: @"data"] == YES)
        {
	  node = [node firstChildElement];
	  while (node != nil)
	    {
	      if ([[node name] isEqualToString: @"value"] == YES)
	        {
		  id	v;

		  v = [self _parseValue: node];
		  if (v != nil)
		    {
		      [arr addObject: v];
		    }
		}
	      node = [node nextElement];
	    }
	}
      return arr;
    }

  if ([name isEqualToString: @"struct"])
    {
      NSMutableDictionary	*dict = [NSMutableDictionary dictionary];

      node = [node firstChildElement];
      while (node != nil)
        {
	  if ([[node name] isEqualToString: @"member"] == YES)
	    {
	      GSXMLNode	*member = [node firstChildElement];
	      NSString	*key = nil;
	      id	val = nil;

	      while (member != nil)
	        {
		  if ([[member name] isEqualToString: @"name"] == YES)
		    {
		      key = [member content];
		    }
		  else if ([[member name] isEqualToString: @"value"] == YES)
		    {
		      val = [self _parseValue: member];
		    }
		  if (key != nil && val != nil)
		    {
		      [dict setObject: val forKey: key];
		      break;
		    }
		  member = [member nextElement];
		}
	    }
	  node = [node nextElement];
	}
      return dict;
    }

  [NSException raise: NSInvalidArgumentException
	      format: @"Unknown value type: %@", name];
  return nil;
}
@end



/*
 * And now, the actual GSXMLRPC class.
 */
@implementation	GSXMLRPC

- (NSData*) buildMethod: (NSString*)method 
	         params: (NSArray*)params
{
  return [[self buildMethodCall: method params: params] dataUsingEncoding:
    NSUTF8StringEncoding];
}

- (NSString*) buildMethodCall: (NSString*)method 
                       params: (NSArray*)params
{
  NSMutableString	*str = [NSMutableString stringWithCapacity: 1024];
  unsigned		c = [params count];
  unsigned		i;
  
  if ([method length] == 0)
    {
      return nil;
    }
  else
    {
      static NSCharacterSet	*illegal = nil;
      NSRange			r;

      if (illegal == nil)
	{
	  NSMutableCharacterSet	*tmp = [NSMutableCharacterSet new];

	  [tmp addCharactersInRange: NSMakeRange('0', 10)];
	  [tmp addCharactersInRange: NSMakeRange('a', 26)];
	  [tmp addCharactersInRange: NSMakeRange('A', 26)];
	  [tmp addCharactersInString: @"_.:/"];
	  [tmp invert];
	  illegal = [tmp copy];
	  RELEASE(tmp);
	}
      r = [method rangeOfCharacterFromSet: illegal];
      if (r.length > 0)
	{
	  return nil;	// Bad method name.
	}
    }
  [str appendString: @"<?xml version=\"1.0\"?>\n"];
  [str appendString: @"<methodCall>"];
  NL;
  INDENT(1);
  [str appendFormat: @"<methodName>%@</methodName>",
    [method stringByEscapingXML]];
  NL;
  if (c > 0)
    {
      INDENT(1);
      [str appendString: @"<params>"];
      NL;
      for (i = 0; i < c; i++)
      	{
	  INDENT(2);
	  [str appendString: @"<param>"];
	  NL;
	  INDENT(3);
	  [str appendString: @"<value>"];
	  NL;
	  [[params objectAtIndex: i] appendToXMLRPC: str indent: 3 for: self];
	  NL;
	  INDENT(3);
	  [str appendString: @"</value>"];
	  NL;
	  INDENT(2);
	  [str appendString: @"</param>"];
	  NL;
	}
      INDENT(1);
      [str appendString: @"</params>"];
      NL;
    }
  [str appendString: @"</methodCall>"];
  NL;
  return str;
}

- (NSString*) buildResponseWithFaultCode: (NSInteger)code andString: (NSString*)s
{
  NSMutableString	*str = [NSMutableString stringWithCapacity: 1024];
  NSDictionary		*fault;

  fault = [NSDictionary dictionaryWithObjectsAndKeys:
    [NSNumber numberWithInt: code], @"faultCode",
    s, @"faultString",
    nil];

  [str appendString: @"<?xml version=\"1.0\"?>\n"];
  [str appendString: @"<methodResponse>"];
  NL;
  INDENT(1);
  [str appendString: @"<fault>"];
  NL;
  INDENT(2);
  [str appendString: @"<value>"];
  NL;
  [fault appendToXMLRPC: str indent: 3 for: self];
  NL;
  INDENT(2);
  [str appendString: @"</value>"];
  NL;
  INDENT(1);
  [str appendString: @"</fault>"];
  NL;
  [str appendString: @"</methodResponse>"];
  NL;
  return str;
}

- (NSString*) buildResponseWithParams: (NSArray*)params
{
  NSMutableString	*str = [NSMutableString stringWithCapacity: 1024];
  unsigned		c = [params count];
  unsigned		i;
  
  [str appendString: @"<?xml version=\"1.0\"?>\n"];
  [str appendString: @"<methodResponse>"];
  NL;
  INDENT(1);
  [str appendString: @"<params>"];
  NL;
  for (i = 0; i < c; i++)
    {
      INDENT(2);
      [str appendString: @"<param>"];
      NL;
      INDENT(3);
      [str appendString: @"<value>"];
      NL;
      [[params objectAtIndex: i] appendToXMLRPC: str indent: 3 for: self];
      NL;
      INDENT(3);
      [str appendString: @"</value>"];
      NL;
      INDENT(2);
      [str appendString: @"</param>"];
      NL;
    }
  INDENT(1);
  [str appendString: @"</params>"];
  NL;
  [str appendString: @"</methodResponse>"];
  NL;
  return str;
}

- (BOOL) compact
{
  return compact;
}

- (void) dealloc
{
  RELEASE(tz);
  if (timer != nil)
    {
      [self timeout: nil];	// Treat as immediate timeout.
    }
#ifdef GNUSTEP
  [handle removeClient: self];
#endif
  DESTROY(result);
#ifdef GNUSTEP
  DESTROY(handle);
#else
  if (connection)
    {
      [connection release];
    }
  [response release];
  [connectionURL release];
#endif
  [super dealloc];
}

- (id) delegate
{
  return delegate;
}

- (id) initWithURL: (NSString*)url
{
  return [self initWithURL: url certificate: nil privateKey: nil password: nil];
}

- (id) initWithURL: (NSString*)url
       certificate: (NSString*)cert
        privateKey: (NSString*)pKey
	  password: (NSString*)pwd
{
  if (url != nil)
    {
      NS_DURING
	{
#ifdef GNUSTEP
	  NSURL	*u = [NSURL URLWithString: url];
        
	  handle = RETAIN([u URLHandleUsingCache: NO]);
	  if (cert != nil && pKey != nil && pwd != nil)
	    {
	      [handle writeProperty: cert forKey: GSTLSCertificateFile];
	      [handle writeProperty: pKey forKey: GSTLSCertificateKeyFile];
	      [handle writeProperty: pwd forKey: GSTLSCertificateKeyPassword];
	    }
#else
	  connectionURL = [url copy];
	  connection = nil;
	  response = [[NSMutableData alloc] init];
#endif
	}
      NS_HANDLER
	{
	  DESTROY(self);
	}
      NS_ENDHANDLER
    }
  return self;
}

- (id) init
{
  return [self initWithURL: nil certificate: nil privateKey: nil password: nil];
}

- (id) makeMethodCall: (NSString*)method 
	       params: (NSArray*)params
	      timeout: (NSInteger)seconds
{
  NS_DURING
    {
      if ([self sendMethodCall: method params: params timeout: seconds] == YES)
	{
	  NSDate	*when = AUTORELEASE(RETAIN([timer fireDate]));

	  while (timer != nil)
	    {
	      [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
				       beforeDate: when];
	    }
	}
    }
  NS_HANDLER
    {
      ASSIGN(result, [localException description]);
    }
  NS_ENDHANDLER

  return result;  
}

- (NSString*) parseMethod: (NSData*)request
		   params: (NSMutableArray*)params
{
  GSXPathContext	*ctx = nil;
  GSXPathNodeSet	*ns = nil;
  GSXMLParser		*parser = nil;
  NSString		*method;
  
  [params removeAllObjects];

  NS_DURING
    {
      GSXMLDocument	*doc = nil;

      parser = [GSXMLParser parserWithData: request];
      [parser substituteEntities: YES];
      [parser saveMessages: YES];
      if ([parser parse] == YES)
        {
	  doc = [parser document];
	  ctx = AUTORELEASE([[GSXPathContext alloc] initWithDocument: doc]);
	}
    }
  NS_HANDLER
    {
      ctx = nil;
    }
  NS_ENDHANDLER
  if (ctx == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Bad Request: parse failed (%@)", [parser messages]];
    }
  
  ns = (GSXPathNodeSet*)[ctx evaluateExpression: @"//methodCall/methodName"];
  if ([ns count] != 1)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Badly formatted methodCall"];
    }
  method = [[ns nodeAtIndex: 0] content];
	 
  ns = (GSXPathNodeSet*)[ctx evaluateExpression: 
    @"//methodCall/params/param/value"];

  NS_DURING
    {
      int	i;

      for (i = 0; i < [ns count]; i++)
	{
	  GSXMLNode	*node = [ns nodeAtIndex: i];
	  id		value = [self _parseValue: node];

	  if (value != nil)
	    {
	      [params addObject: value];
	    }
	}
    }
  NS_HANDLER
    {
      [params removeAllObjects];
      [localException raise];
    }
  NS_ENDHANDLER

  return method;
}

- (NSDictionary*) parseResponse: (NSData*)resp
			 params: (NSMutableArray*)params
{
  GSXPathContext	*ctx = nil;
  GSXPathNodeSet	*ns = nil;
  GSXMLParser		*parser = nil;
  id			fault = nil;

  [params removeAllObjects];

  NS_DURING
    {
      GSXMLDocument	*doc = nil;

      parser = [GSXMLParser parserWithData: resp];
      [parser substituteEntities: YES];
      [parser saveMessages: YES];
      if ([parser parse] == YES)
	{
	  doc = [parser document];
	  ctx = AUTORELEASE([[GSXPathContext alloc] initWithDocument: doc]);
	}
    }
  NS_HANDLER
    {
      ctx = nil;
    }
  NS_ENDHANDLER
  if (ctx == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Bad Request: parse failed (%@)", [parser messages]];
    }

  ns = (GSXPathNodeSet*)[ctx evaluateExpression: 
    @"//methodResponse/params/param/value"];

  NS_DURING
    {
      int	i;

      if ([ns count] > 0)
	{
	  for (i = 0; i < [ns count]; i++)
	    {
	      GSXMLNode	*node = [ns nodeAtIndex: i];
	      id	value = [self _parseValue: node];

	      if (value != nil)
		{
		  [params addObject: value];
		}
	    }
	}
      else
	{
	  ns = (GSXPathNodeSet*)[ctx evaluateExpression: 
	    @"//methodResponse/fault/value/struct"];
	  if ([ns count] > 0)
	    {
	      fault = [self _parseValue: [ns nodeAtIndex: 0]];
	    }
	}
    }
  NS_HANDLER
    {
      [params removeAllObjects];
      [localException raise];
    }
  NS_ENDHANDLER

  return fault;
}

- (id) result
{
  if (timer == nil)
    {
      return result;
    }
  else
    {
      return nil;
    }
}

- (BOOL) sendMethodCall: (NSString*)method 
		 params: (NSArray*)params
		timeout: (NSInteger)seconds
{
  NSData	*data;

  ASSIGN(result, @"unable to send");

#ifdef GNUSTEP
  if (handle == nil)
    {
      return NO;	// Not initialised to send.
    }
#endif
  if (timer != nil)
    {
      return NO;	// Send already in progress.
    }
  data = [self buildMethod: method params: params];
  if (data == nil)
    {
      return NO;
    }

  timer = [NSTimer scheduledTimerWithTimeInterval: seconds
					   target: self
					 selector: @selector(timeout:)
					 userInfo: nil
					  repeats: NO];

#ifdef GNUSTEP
  [handle addClient: self];
  [handle writeProperty: @"POST" forKey: GSHTTPPropertyMethodKey];
  [handle writeProperty: @"GSXMLRPC/1.0.0" forKey: @"User-Agent"];
  [handle writeProperty: @"text/xml" forKey: @"Content-Type"];
  [handle writeData: data];
  [handle loadInBackground];
#else
  {
    NSMutableURLRequest *request;

    request = [NSMutableURLRequest alloc];
    request = [request initWithURL: [NSURL URLWithString: connectionURL]];
    [request setCachePolicy: NSURLRequestReloadIgnoringCacheData];
    [request setHTTPMethod: @"POST"];  
    [request setValue: @"GSXMLRPC/1.0.0" forHTTPHeaderField: @"User-Agent"];
    [request setValue: @"text/xml" forHTTPHeaderField: @"Content-Type"];
    [request setHTTPBody: data];
  
    connection = [NSURLConnection alloc];
    connection = [connection initWithRequest: request delegate: self];
    [request release];
  }
#endif
  return YES;
}

- (int) setDebug: (int)flag
{
#ifdef GNUSTEP
  if ([handle respondsToSelector: _cmd] == YES)
    {
      return [(id)handle setDebug: flag];
    }
#endif
  return NO;
}

- (void) setCompact: (BOOL)flag
{
  compact = flag;
}

- (void) setDelegate: (id)aDelegate
{
  delegate = aDelegate;
}

- (void) setTimeZone: (NSTimeZone*)timeZone
{
  ASSIGN(tz, timeZone);
}

- (void) timeout: (NSTimer*)t
{
  [timer invalidate];
  timer = nil;
#ifdef GNUSTEP
  [handle cancelLoadInBackground];
#else
  [connection cancel];
#endif
}

- (NSTimeZone*) timeZone
{
  if (tz == nil)
    {
      tz = RETAIN([NSTimeZone timeZoneForSecondsFromGMT: 0]);
    }
  return tz;
}

#ifdef GNUSTEP

- (void) URLHandle: (NSURLHandle*)sender
  resourceDataDidBecomeAvailable: (NSData*)newData
{
  // Not interesting
}

- (void) URLHandle: (NSURLHandle*)sender
  resourceDidFailLoadingWithReason: (NSString*)reason
{
  ASSIGN(result, reason);
  [timer invalidate];
  timer = nil;
#ifdef GNUSTEP
  [handle removeClient: self];
#endif
  if ([delegate respondsToSelector: @selector(completedXMLRPC:)])
    {
      [delegate completedXMLRPC: self];
    }
}

- (void) URLHandleResourceDidBeginLoading: (NSURLHandle*)sender
{
  // Not interesting
}

- (void) URLHandleResourceDidCancelLoading: (NSURLHandle*)sender
{
  NSString	*str;

  [timer invalidate];
  timer = nil;
#ifdef GNUSTEP
  [handle removeClient: self];
#endif
  str = [handle propertyForKeyIfAvailable: NSHTTPPropertyStatusCodeKey];
  if (str == nil)
    {
      str = @"timeout";
    }
  else
    {
      str = [NSString stringWithFormat: @"HTTP status %@", str];
    }
  ASSIGN(result, str);
  if ([delegate respondsToSelector: @selector(completedXMLRPC:)])
    {
      [delegate completedXMLRPC: self];
    }
}

- (void) URLHandleResourceDidFinishLoading: (NSURLHandle*)sender
{
  NSMutableArray	*params = [NSMutableArray array];
  id			fault = nil;
  int			code;

  code = [[handle propertyForKey: NSHTTPPropertyStatusCodeKey] intValue];

  if (code == 200)
    {
      NSData	*resp = [handle availableResourceData];

      NS_DURING
	{
	  fault = [self parseResponse: resp params: params];
	}
      NS_HANDLER
	{
	  fault = [localException reason];
	}
      NS_ENDHANDLER
    }
  else
    {
      fault = [NSString stringWithFormat: @"HTTP status %03d", code];
    }
  if (fault == nil)
    {
      ASSIGN(result, params);
    }
  else
    {
      ASSIGN(result, fault);
    }

  [timer invalidate];
  timer = nil;
#ifdef GNUSTEP
  [handle removeClient: self];
#endif

  if ([delegate respondsToSelector: @selector(completedXMLRPC:)])
    {
      [delegate completedXMLRPC: self];
    }
}

#else /* !GNUSTEP */

- (void) connection: (NSURLConnection*)connection
didCancelAuthenticationChallenge: (NSURLAuthenticationChallenge*)challenge
{
  /* DO NOTHING */
}

- (void) connection: (NSURLConnection*)connection
   didFailWithError: (NSError*)error
{
  ASSIGN(result, [error localizedDescription]);
  [timer invalidate];
  timer = nil;
  if ([delegate respondsToSelector: @selector(completedXMLRPC:)])
    {
      [delegate completedXMLRPC: self];
    }    
}

- (void) connection: (NSURLConnection*)connection
didReceiveAuthenticationChallenge: (NSURLAuthenticationChallenge*)challenge 
{
}

- (void) connection: (NSURLConnection*)connection didReceiveData: (NSData*)data 
{
  [response appendData: data];
}

- (void) connection: (NSURLConnection*)connection
 didReceiveResponse: (NSURLResponse*)response 
{
  /* DO NOTHING */
}

- (NSCachedURLResponse*) connection: (NSURLConnection*)connection
		  willCacheResponse: (NSCachedURLResponse*)cachedResponse
{
  return nil;
}

- (NSURLRequest*) connection: (NSURLConnection*)connection
	     willSendRequest: (NSURLRequest*)request
	    redirectResponse: (NSURLResponse*)redirectResponse 
{
  return nil;
}

-(void) connectionDidFinishLoading: (NSURLConnection*)connection 
{
  NSMutableArray	*params = [NSMutableArray array];
  id			fault = nil;

  NS_DURING
    {
      fault = [self parseResponse: response params:   params];
    }
  NS_HANDLER
    {
      fault = [localException reason];
    }
  NS_ENDHANDLER
    
  if (fault == nil)
    {
      ASSIGNCOPY(result, params);
    }
  else
    {
      ASSIGNCOPY(result, fault);
    }
    
  [timer invalidate];
  timer = nil;
    
  if ([delegate respondsToSelector: @selector(completedXMLRPC:)])
    {
      [delegate completedXMLRPC: self];
    }        
}

#endif

@end

@implementation	GSXMLRPC (Delegate)
- (void) completedXMLRPC: (GSXMLRPC*)sender
{
}
@end

#endif	/* HAVE_LIBXML */

