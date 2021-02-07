/* Interface for NSXMLDocument for GNUStep
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

#ifndef __NSXMLDocument_h_GNUSTEP_BASE_INCLUDE
#define __NSXMLDocument_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSXMLNode.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData;
@class NSXMLDTD;
@class NSMutableArray;

/*
 * Input options
 * NSXMLNodeOptionsNone
 * NSXMLNodePreserveAll
 * NSXMLNodePreserveNamespaceOrder
 * NSXMLNodePreserveAttributeOrder
 * NSXMLNodePreserveEntities
 * NSXMLNodePreservePrefixes
 * NSXMLNodePreserveCDATA
 * NSXMLNodePreserveEmptyElements
 * NSXMLNodePreserveQuotes
 * NSXMLNodePreserveWhitespace
 * NSXMLDocumentTidyHTML
 * NSXMLDocumentTidyXML
 * NSXMLDocumentValidate
 *
 * Output options
 * NSXMLNodePrettyPrint
 * NSXMLDocumentIncludeContentTypeDeclaration
 */

enum {
  NSXMLDocumentXMLKind = 0,       /** Default type */
  NSXMLDocumentXHTMLKind,         /** HTML found */
  NSXMLDocumentHTMLKind,          /** Output no close tag for empty elem*/
  NSXMLDocumentTextKind           /** Output string value of doc */
};
/**
 * Define what type of document this is.
 */
typedef NSUInteger NSXMLDocumentContentKind;

/**
 * An XMLDocument encapsulates an entire document.
 * This must contain a single element node.
 */
GS_EXPORT_CLASS
@interface NSXMLDocument : NSXMLNode
{
#if     GS_NONFRAGILE
#  if	defined(GS_NSXMLDocument_IVARS)
@public GS_NSXMLDocument_IVARS;
#  endif
#endif
  /* The pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available is inherited from
   * NSXMLNode.  See Source/GSInternal.h for details.
   */
}

+ (Class) replacementClassForClass: (Class)cls;

/**
 * Returns the IANA character encoding, or nil if none is set.
 */
- (NSString*) characterEncoding;

/**
 * Returns the kind of document.
 */
- (NSXMLDocumentContentKind) documentContentKind;

/**
 * Returns the DTD set for the receiver.
 */
- (NSXMLDTD*) DTD;

/**
 * Initialise using the data downloaded from the spplied url.
 */
- (id) initWithContentsOfURL: (NSURL*)url
                     options: (NSUInteger)mask
                       error: (NSError**)error;

/** <init />
 * Returns a document created from data.<br />
 * Parse errors are returned in error.
 */
- (id) initWithData: (NSData*)data
            options: (NSUInteger)mask
              error: (NSError**)error;

/**
 * Returns a document with a single child, the root element.
 */
- (id) initWithRootElement: (NSXMLElement*)element;

/**
 * Initialises the receiver by creating a document from XML (or HTML
 * if the HTMLTidy option is set).
 * Parse errors are returned in the error argument.
*/
- (id) initWithXMLString: (NSString*)string
                 options: (NSUInteger)mask
                   error: (NSError**)error;

/**
 * Returns NO if the receiver depends upon an external DTD, otherwise
 * returns YES.
 */
- (BOOL) isStandalone;

/**
 * Returns the document MIME type..
 */
- (NSString*) MIMEType;

/**
 * Returns the root object of the receiver.
 */
- (NSXMLElement*) rootElement; 

/**
 * Sets the character encoding to an IANA characterset type.
 */
- (void) setCharacterEncoding: (NSString*)encoding;

/**
 * Sets the kind of document.
 */
- (void) setDocumentContentKind: (NSXMLDocumentContentKind)theContentKind;

/**
 * Sets the DTD of the receiver.  If this is set then the DTD will be
 * output when the document is.
 */
- (void) setDTD: (NSXMLDTD*)documentTypeDeclaration;

/**
 * Sets the document MIME type (usually text/xml).
 */
- (void) setMIMEType: (NSString*)theMIMEType;

/**
 * Sets the root object of the receiver, removing any children which
 * were previously set.
 */
- (void) setRootElement: (NSXMLNode*)root;

/**
 * Sets whether the receiver is a document which requires an external DTD.<br />
 * If this is set then the standalone declaration will appear if the document is
 * output.
 */
- (void) setStandalone: (BOOL)standalone;

/**
 * Sets the XML version<br />
 * Permitted values ar '1.0' or '1,1'
 */
- (void) setVersion: (NSString*)version;

/**
 * Returns the XML version or nil if none is set.
 */
- (NSString*) version;


/**
 * Inserts child at index.
 */
- (void) insertChild: (NSXMLNode*)child atIndex: (NSUInteger)index;

/**
 * Inserts a number of children at the index.
 */
- (void) insertChildren: (NSArray*)children atIndex: (NSUInteger)index;

/**
 * Removes the child at the index.
 */
- (void) removeChildAtIndex: (NSUInteger)index;

/**
 * Replaces all existing child nodes with the ones in the array.
 */
- (void) setChildren: (NSArray*)children;

/**
 * Adds child after existing children.
 */
- (void) addChild: (NSXMLNode*)child;

/**
 * Replacs the child at the specified index.
 */
- (void) replaceChildAtIndex: (NSUInteger)index withNode: (NSXMLNode*)theNode;

/**
 * Outputs XML data using -XMLDataWithOptions: with NSXMLNodeOptionsNone.
 */
- (NSData*) XMLData;

/**
 * Outputs the reciever encoded using the specified options.
 */
- (NSData*) XMLDataWithOptions: (NSUInteger)theOptions;

/**
 * Returns a new document created by applying xslt (with a set of
 * key/value pairs) to the receiver.
 */
- (id) objectByApplyingXSLT: (NSData*)xslt
                  arguments: (NSDictionary*)arguments
                      error: (NSError**)error;

/**
 * Returns a new document created by applying xslt (with a set of
 * key/value pairs) to the receiver.
 */
- (id) objectByApplyingXSLTString: (NSString*)xslt
                        arguments: (NSDictionary*)arguments
                            error: (NSError**)error;

/**
 * Downloads XSLT from xsltURL, and then returns a new document created
 * by applying it (with a set of key/value pairs) to the receiver.
 */
- (id) objectByApplyingXSLTAtURL: (NSURL*)xsltURL
                       arguments: (NSDictionary*)arguments
                           error: (NSError**)error;

/* Validate the receiver according to its DTD.
 */
- (BOOL) validateAndReturnError: (NSError**)error;

@end

#if	defined(__cplusplus)
}
#endif

#endif /*__NSXMLDocument_h_GNUSTEP_BASE_INCLUDE */
