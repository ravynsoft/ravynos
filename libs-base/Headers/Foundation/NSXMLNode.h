/* Interface for NSXMLNode for GNUStep
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

#ifndef __NSXMLNode_h_GNUSTEP_BASE_INCLUDE
#define __NSXMLNode_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSXMLNodeOptions.h>

#if	defined(__cplusplus)
extern "C" {
#endif


@class NSArray;
@class NSDictionary;
@class NSError;
@class NSString;
@class NSURL;
@class NSXMLDocument;
@class NSXMLElement;

enum {
  NSXMLInvalidKind = 0,
  NSXMLDocumentKind,
  NSXMLElementKind,
  NSXMLAttributeKind,
  NSXMLNamespaceKind,
  NSXMLProcessingInstructionKind,
  NSXMLCommentKind,
  NSXMLTextKind,
  NSXMLDTDKind,
  NSXMLEntityDeclarationKind,
  NSXMLAttributeDeclarationKind,
  NSXMLElementDeclarationKind,
  NSXMLNotationDeclarationKind
};
typedef NSUInteger NSXMLNodeKind;

// initWithKind options
//  NSXMLNodeOptionsNone
//  NSXMLNodePreserveAll
//  NSXMLNodePreserveNamespaceOrder
//  NSXMLNodePreserveAttributeOrder
//  NSXMLNodePreserveEntities
//  NSXMLNodePreservePrefixes
//  NSXMLNodeIsCDATA
//  NSXMLNodeExpandEmptyElement
//  NSXMLNodeCompactEmptyElement
//  NSXMLNodeUseSingleQuotes
//  NSXMLNodeUseDoubleQuotes

// Output options
//  NSXMLNodePrettyPrint

/* This next class declaration is private for internal use by the base
 * library developers ... it is present simply to make the compiler
 * produce symbol information to be used when debugging.
 */
#if	defined(GSInternal)
@class GSInternal;
#endif

/**
 * The most primitive unit in an XML document.
 */
GS_EXPORT_CLASS
@interface NSXMLNode : NSObject <NSCopying>
{
#if     GS_NONFRAGILE
#  if	defined(GS_NSXMLNode_IVARS)
@public GS_NSXMLNode_IVARS;
#  endif
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   * In this case, when compiled the variable is a public class pointer
   * available so that the NSXMLNode subclasses within the base library
   * can share the same variables, but when viewed from outside the
   * base library itsself, it's an opaque pointer which must not be used.
   */
#  if	defined(GSInternal)
  @public GSInternal *_internal GS_UNUSED_IVAR;
#  else
  @private id _internal GS_UNUSED_IVAR;
#  endif
#endif
}

/**
 * Creates and returns an attribute node with the specified name and value.
 */
+ (id) attributeWithName: (NSString*)name stringValue: (NSString*)stringValue;

/**
 * Creates and returns an attribute node with the specified
 * (fully qualified) name, namespace URI, and value.
 */
+ (id) attributeWithName: (NSString*)name
                     URI: (NSString*)URI
             stringValue: (NSString*)stringValue;

/**
 * Creates and returns a comment node with the supplied text.
 */
+ (id) commentWithStringValue: (NSString*)stringValue;

/** Creates and returns an empty document ... convenience method.
 */
+ (id) document;

/**
 * Creates and returns a new document intialised with the supplied root
 * node.
 */
+ (id) documentWithRootElement: (NSXMLElement*)element;

/**
 * Creates and returns a DTD node from the supplied string.
 * The node might describe element, attribute, entity, or notation.
 */
+ (id) DTDNodeWithXMLString: (NSString*)string;

/**
 * Creates and returns a node node with the specified name.
 */
+ (id) elementWithName: (NSString*)name;

/**
 * Creates and returns a node containing the supplied child nodes and with
 * the attribute nodes from the supplied array.
 */
+ (id) elementWithName: (NSString*)name
              children: (NSArray*)children
            attributes: (NSArray*)attributes;

/**
 * Creates and returns a node with the supplied (fully qualified name)
 * an namespace URI.
 */
+ (id) elementWithName: (NSString*)name URI: (NSString*)URI;

/**
 * Creates and returns a node with the pecified name and with a single
 * text node (containing string) as its content.
 */
+ (id) elementWithName: (NSString*)name stringValue: (NSString*)string;

/** Strips any leading namespace prefix from name and returns the result.
 */
+ (NSString*) localNameForName: (NSString*)name;

/**
 * Creates and returns a node mapping the supplkied namespace name to
 * the string value (URI).
 */
+ (id) namespaceWithName: (NSString*)name stringValue: (NSString*)stringValue;

/** Returns the namespace node corresponding to a predefined namespace names
 * (one of xml, xs, or xsi)
 */
+ (NSXMLNode*) predefinedNamespaceForPrefix: (NSString*)name;

/** Returns the namespace prefix of name.
 */
+ (NSString*) prefixForName: (NSString*)name;

/**
 * Creates and return a processinc insruction node with th specified name
 * and textual value.
 */
+ (id) processingInstructionWithName: (NSString*)name
                         stringValue: (NSString*)stringValue;

/**
 * Creates and returns a simple text node.
 */
+ (id) textWithStringValue: (NSString*)stringValue;


/** Returns the canonical form (see http://www.w3.org/TR/xml-c14n) of the
 * receiveri as long as the NSXMLNodePreserveWhitespace has been set.
 * If the option as not been set, return the same without white space
 * preserved.
 */
- (NSString*) canonicalXMLStringPreservingComments: (BOOL)comments;

/** Returns the child node if the receiver at the specified index.
 */
- (NSXMLNode*) childAtIndex: (NSUInteger)index;

/** Returns the number of immediate child nodes of the receiver.<br />
 * This method is more efficient than getting the array of children
 * and counting it.
 */
- (NSUInteger) childCount;

/** Returns n array containing the immediate child nodes of the receiver.
 */
- (NSArray*) children;

/** Detaches the receiver from its parent node.
 */
- (void) detach;

/** Return the index of the receiver within its parent node.
 */
- (NSUInteger) index;

/** Initialises the receiver as a specific kind of node.<br />
 * Calls -initWithKind:options: using NSXMLNodeOptionsNone and
 * returns the result.
 */
- (id) initWithKind: (NSXMLNodeKind)theKind;

/** <init />
 * Initialises the receiver as the specified kind of node and with the
 * specified options.
 */
- (id) initWithKind: (NSXMLNodeKind)theKind options: (NSUInteger)theOptions;

/** Return the level of the receiver within the tree of nodes.<br />
 *  A document or a node which is not inside another is at level 0.
 */
- (NSUInteger) level;

/** Returns the name of the receiver without any namespace prefix.
 */
- (NSString*) localName;

/**
 * Returns the kind of node represented by the receiver.
 */
- (NSXMLNodeKind) kind;

/** Returns the name of the receiver.
 */
- (NSString*) name;

/** Returns the next node in the tree.
 */
- (NSXMLNode*) nextNode;

/** Returns the next sibling of the receiver
 */
- (NSXMLNode*) nextSibling;

/** Returns the nodes resulting from applying xpath to the receiver.<br />
 * Before using xpath on a tree, you must call the
 * -normalizeAdjacentTextNodesPreservingCDATA: with NO as the argument,
 * in order to avoid problems where the xpath syntax cannot cope with
 * multiple text nodes.
 */
- (NSArray*) nodesForXPath: (NSString*)anxpath error: (NSError**)error;

/** Returns the data resulting from calling the query on the receiver.<br />
 * The same as objectsForXQuery:constants:error: without the constants.
 */
- (NSArray*) objectsForXQuery: (NSString*)xquery error: (NSError**)error;

/** Returns the data resulting from calling the query on the receiver.<br />
 * Before using xpath on a tree, you must call the
 * -normalizeAdjacentTextNodesPreservingCDATA: with NO as the argument,
 * in order to avoid problems where the xpath syntax cannot cope with
 * multiple text nodes.<br />
 * The value of constants is a dictionary of constants declared
 * to be "external" in the query.<br />
 * The resulting array amy contain array, data, date, number, string,
 * and URL objects as well as nodes.
 */
- (NSArray*) objectsForXQuery: (NSString*)xquery
                    constants: (NSDictionary*)constants
                        error: (NSError**)error;

/**
 * Returns the object value of the receiver (as set using -setObjectValue:)
 * or nil if there is none.
 */
- (id) objectValue;

/**
 * Returns the parent node of the receiver or nil if the receiver is not
 * within another node.
 */
- (NSXMLNode*) parent;

/** Returns the namepsace prefix for the receiver or nil if there is no
 * namespace prefix.
 */
- (NSString*) prefix;

/** Returns the previous node in the tree ... stepping through the tree
 * backwards.
 */
- (NSXMLNode*) previousNode;

/* Returns the previous sibling of the receiver.
 */
- (NSXMLNode*) previousSibling;

/** Returns the document containing the receiver, or nil if the receiver
 * does not lie within a document.
 */
- (NSXMLDocument*) rootDocument;

/** Sets the name of the receiver.
 */
- (void) setName: (NSString*)name;

/** Sets the content of the receiver, removing any existing children
 * (including comments and processing instructions).<br />
 * For an element node, this sets text contetn within the node.
 */
- (void) setObjectValue: (id)value;

/**
 * Sets the content of the receiver to be the supplied string value ...
 * which involves removing existing content, comments,
 * and processing instructions.<br />
 * If the receiver is an element node, this creates a text node containing
 * the string value as the sole child of the receiver.
 */
- (void) setStringValue: (NSString*)string;

/**
 * Sets the string content of the receiver.<br />
 * if the resolve flag is YES then any entities which can be resolved are
 * replaced by the resolved versions.
 */
- (void) setStringValue: (NSString*)string resolvingEntities: (BOOL)resolve;

/** Sets the URI of the receiver.
 */
- (void) setURI: (NSString*)URI;

/** Returns the string value of the receiver.  For kinds of node which have
 * direct string content, this simply returns that content.  For elements
 * this recursively traverses the children of the receiver appending the
 * text of each child to a single string result. 
 */
- (NSString*) stringValue;

/** Returns the URI of the receiver.
 */
- (NSString*) URI;

/** Returns the XPath string to access the receiver with the document.
 */
- (NSString*) XPath;

/**
 * Returns the text of the receiver as XML (ie in the form it would have
 * in an XML document).
 */
- (NSString*) XMLString;

/**
 * Returns the text of the receiver as XML (ie in the form it would have
 * in an XML document), with the specified options controlling it.
 */
- (NSString*) XMLStringWithOptions: (NSUInteger)theOptions;

@end

#if	defined(__cplusplus)
}
#endif

#endif /*__NSXMLNode_h_GNUSTEP_BASE_INCLUDE */
