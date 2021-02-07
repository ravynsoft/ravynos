/** Interface for XML parsing classes

   Copyright (C) 2000-2005 Free Software Foundation, Inc.

   Written by:  Michael Pakhantsov  <mishel@berest.dp.ua> on behalf of
   Brainstorm computer solutions.

   Date: Jule 2000

   Integrated by Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: September 2000
   GSXPath by Nicola Pero <nicola@brainstorm.co.uk>

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

   AutogsdocSource: Additions/GSXML.m

*/

#ifndef __GSXML_h_GNUSTEP_BASE_INCLUDE
#define __GSXML_h_GNUSTEP_BASE_INCLUDE
#import <GNUstepBase/GSVersionMacros.h>

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

#ifndef NeXT_Foundation_LIBRARY
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSStream.h>
#else
#import <Foundation/Foundation.h>
#endif

#if	defined(__cplusplus)
extern "C" {
#endif

@class GSXMLAttribute;
@class GSXMLDocument;
@class GSXMLNamespace;
@class GSXMLNode;
@class GSSAXHandler;

/**
 * Convenience methods for managing XML escape sequences in an NSString.
 */
@interface	NSString (GSXML)
/**
 * Deals with standard XML internal entities.<br />
 * Converts the five XML special characters in the receiver ('&gt;', '&lt;',
 * '&amp;', '&apos;' and '&quot;') to their escaped equivalents, and return
 * the escaped string.<br />
 * Also converts non-ascii characters to the corresponding numeric
 * entity escape sequences.<br />
 * You should perform any non-standard entity substitution you require
 * <em>after</em> you have called this method.
 */
- (NSString*) stringByEscapingXML;
/**
 * Deals with standard XML internal entities.<br />
 * Converts the five XML escape sequences ('&amp;gt;', '&amp;lt;', '&amp;amp;',
 * '&amp;apos;' and '&amp;quot;') to the unicode characters they represent,
 * and returns the unescaped string.<br />
 * Also converts numeric entity escape sequences to the corresponding unicode
 * characters.<br />
 * You should perform any non-standard entity substitution you require
 * <em>before</em> you have called this method.
 */
- (NSString*) stringByUnescapingXML;
@end

GS_EXPORT_CLASS
@interface GSXMLDocument : NSObject <NSCopying>
{
  void	*lib;	// pointer to xmllib pointer of xmlDoc struct
  BOOL	_ownsLib;
  id	_parent;
}
+ (GSXMLDocument*) documentWithVersion: (NSString*)version;

- (NSString*) description;
- (NSString*) encoding;

- (void*) lib;

- (GSXMLNode*) makeNodeWithNamespace: (GSXMLNamespace*)ns
				name: (NSString*)name
			     content: (NSString*)content;

- (GSXMLNode*) root;
- (GSXMLNode*) setRoot: (GSXMLNode*)node;

- (NSString*) version;

- (BOOL) writeToFile: (NSString*)filename atomically: (BOOL)useAuxilliaryFile;
- (BOOL) writeToURL: (NSURL*)url atomically: (BOOL)useAuxilliaryFile;

@end



GS_EXPORT_CLASS
@interface GSXMLNamespace : NSObject <NSCopying>
{
  void	*lib;          /* pointer to struct xmlNs in the gnome xmllib */
  id	_parent;
}

+ (NSString*) descriptionFromType: (NSInteger)type;
+ (NSInteger) typeFromDescription: (NSString*)desc;

- (NSString*) href;

- (void*) lib;
- (GSXMLNamespace*) next;
- (NSString*) prefix;
- (NSInteger) type;
- (NSString*) typeDescription;

@end

/* XML Node */

GS_EXPORT_CLASS
@interface GSXMLNode : NSObject <NSCopying>
{
  void  *lib;      /* pointer to struct xmlNode from libxml */
  id	_parent;
}

+ (NSString*) descriptionFromType: (NSInteger)type;
+ (NSInteger) typeFromDescription: (NSString*)desc;

- (NSDictionary*) attributes;
- (NSString*) content;
- (NSString*) description;
- (GSXMLDocument*) document;
- (NSString*) escapedContent;
- (GSXMLAttribute*) firstAttribute;
- (GSXMLNode*) firstChild;
- (GSXMLNode*) firstChildElement;
- (BOOL) isElement;
- (BOOL) isText;
- (void*) lib;
- (GSXMLAttribute*) makeAttributeWithName: (NSString*)name
				    value: (NSString*)value;
- (GSXMLNode*) makeChildWithNamespace: (GSXMLNamespace*)ns
				 name: (NSString*)name
			      content: (NSString*)content;
- (GSXMLNode*) makeComment: (NSString*)content;
- (GSXMLNamespace*) makeNamespaceHref: (NSString*)href
			       prefix: (NSString*)prefix;
- (GSXMLNode*) makePI: (NSString*)name
	      content: (NSString*)content;
- (GSXMLNode*) makeText: (NSString*)content;
- (NSString*) name;
- (GSXMLNamespace*) namespace;
- (GSXMLNamespace*) namespaceDefinitions;
- (GSXMLNode*) next;
- (GSXMLNode*) nextElement;
- (NSString*) objectForKey: (NSString*)key;
- (GSXMLNode*) parent;
- (GSXMLNode*) previous;
- (GSXMLNode*) previousElement;
- (NSMutableDictionary*) propertiesAsDictionaryWithKeyTransformationSel:
  (SEL)keyTransformSel;
- (void) setObject: (NSString*)value forKey:(NSString*)key;
- (NSInteger) type;
- (NSString*) typeDescription;
- (void) setNamespace: (GSXMLNamespace *)space;

@end

GS_EXPORT_CLASS
@interface GSXMLAttribute : GSXMLNode
- (NSString*) value;
@end

GS_EXPORT_CLASS
@interface GSXMLParser : NSObject
{
   id			src;		/* source for parsing	*/
   void			*lib;		/* parser context	*/
   GSSAXHandler		*saxHandler;	/* handler for parsing	*/
   NSMutableString	*messages;	/* append messages here	*/
}
+ (NSString*) loadEntity: (NSString*)publicId at: (NSString*)location;
+ (GSXMLParser*) parser;
+ (GSXMLParser*) parserWithContentsOfFile: (NSString*)path;
+ (GSXMLParser*) parserWithContentsOfURL: (NSURL*)url;
+ (GSXMLParser*) parserWithData: (NSData*)data;
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler;
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler
		   withContentsOfFile: (NSString*)path;
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler
		    withContentsOfURL: (NSURL*)url;
+ (GSXMLParser*) parserWithSAXHandler: (GSSAXHandler*)handler
			     withData: (NSData*)data;
+ (void) setDTDs: (NSString*)aPath;
+ (NSString*) xmlEncodingStringForStringEncoding: (NSStringEncoding)encoding;

- (void) abortParsing;
- (NSInteger) columnNumber;
- (GSXMLDocument*) document;
- (BOOL) doValidityChecking: (BOOL)yesno;
- (NSInteger) errNo;
- (BOOL) getWarnings: (BOOL)yesno;
- (id) initWithSAXHandler: (GSSAXHandler*)handler;
- (id) initWithSAXHandler: (GSSAXHandler*)handler
       withContentsOfFile: (NSString*)path;
- (id) initWithSAXHandler: (GSSAXHandler*)handler
	withContentsOfURL: (NSURL*)url;
- (id) initWithSAXHandler: (GSSAXHandler*)handler
		 withData: (NSData*)data;
- (id) initWithSAXHandler: (GSSAXHandler*)handler
	 withInputStream: (NSInputStream*)stream;

- (BOOL) keepBlanks: (BOOL)yesno;
- (NSInteger) lineNumber;
- (NSString*) messages;
- (BOOL) parse;
- (BOOL) parse: (NSData*)data;
- (NSString*) publicID;
- (void) saveMessages: (BOOL)yesno;
- (BOOL) resolveEntities: (BOOL)yesno;
- (BOOL) substituteEntities: (BOOL)yesno;
- (NSString*) systemID;

@end

GS_EXPORT_CLASS
@interface GSHTMLParser : GSXMLParser
{
}
@end

GS_EXPORT_CLASS
@interface GSSAXHandler : NSObject
{
  void		*lib;	// xmlSAXHandlerPtr
  GSXMLParser	*parser;
@protected
  BOOL		isHtmlHandler;
}
+ (GSSAXHandler*) handler;
- (void*) lib;
- (GSXMLParser*) parser;

/** <override-dummy /> */
- (void) attribute: (NSString*)name
	     value: (NSString*)value;
/** <override-dummy /> */
- (void) attributeDecl: (NSString*)nameElement
                  name: (NSString*)name
                  type: (NSInteger)type
          typeDefValue: (NSInteger)defType
          defaultValue: (NSString*)value;
/** <override-dummy /> */
- (void) characters: (NSString*)name;
/** <override-dummy /> */
- (void) cdataBlock: (NSData*)value;
/** <override-dummy /> */
- (void) comment: (NSString*) value;
/** <override-dummy /> */
- (void) elementDecl: (NSString*)name
		type: (NSInteger)type;
/** <override-dummy /> */
- (void) endDocument;
/** <override-dummy /> */
- (void) endElement: (NSString*)elementName;
/** <override-dummy /> */
- (void) endElement: (NSString*)elementName
             prefix: (NSString*)prefix
	       href: (NSString*)href;
/** <override-dummy /> */
- (void) entityDecl: (NSString*)name
               type: (NSInteger)type
             public: (NSString*)publicId
             system: (NSString*)systemId
            content: (NSString*)content;
- (void) error: (NSString*)e;
- (void) error: (NSString*)e
     colNumber: (NSInteger)colNumber
    lineNumber: (NSInteger)lineNumber;
- (BOOL) externalSubset: (NSString*)name
             externalID: (NSString*)externalID
               systemID: (NSString*)systemID;
- (void) fatalError: (NSString*)e;
- (void) fatalError: (NSString*)e
          colNumber: (NSInteger)colNumber
         lineNumber: (NSInteger)lineNumber;
- (void*) getEntity: (NSString*)name;
- (void*) getParameterEntity: (NSString*)name;
/** <override-dummy /> */
- (void) globalNamespace: (NSString*)name
		    href: (NSString*)href
		  prefix: (NSString*)prefix;
- (NSInteger) hasExternalSubset;
- (NSInteger) hasInternalSubset;
/** <override-dummy /> */
- (void) ignoreWhitespace: (NSString*)ch;
- (BOOL) internalSubset: (NSString*)name
             externalID: (NSString*)externalID
               systemID: (NSString*)systemID;
- (NSInteger) isStandalone;
- (NSString*) loadEntity: (NSString*)publicId
		      at: (NSString*)location;
/** <override-dummy /> */
- (void) namespaceDecl: (NSString*)name
		  href: (NSString*)href
		prefix: (NSString*)prefix;
/** <override-dummy /> */
- (void) notationDecl: (NSString*)name
	       public: (NSString*)publicId
	       system: (NSString*)systemId;
/** <override-dummy /> */
- (void) processInstruction: (NSString*)targetName
		       data: (NSString*)PIdata;
/** <override-dummy /> */
- (void) reference: (NSString*)name;
/** <override-dummy /> */
- (void) startDocument;
/** <override-dummy /> */
- (void) startElement: (NSString*)elementName
           attributes: (NSMutableDictionary*)elementAttributes;
/** <override-dummy /> */
- (void) startElement: (NSString*)elementName
	       prefix: (NSString*)prefix
		 href: (NSString*)href
           attributes: (NSMutableDictionary*)elementAttributes
           namespaces: (NSMutableDictionary*)elementNamespaces;
/** <override-dummy /> */
- (void) unparsedEntityDecl: (NSString*)name
		     public: (NSString*)publicId
		     system: (NSString*)systemId
	       notationName: (NSString*)notation;
- (void) warning: (NSString*)e;
- (void) warning: (NSString*)e
       colNumber: (NSInteger)colNumber
      lineNumber: (NSInteger)lineNumber;

@end

GS_EXPORT_CLASS
@interface GSTreeSAXHandler : GSSAXHandler
@end

GS_EXPORT_CLASS
@interface GSHTMLSAXHandler : GSSAXHandler
@end

@class GSXPathObject;

/*
 * Using this library class is trivial.  Get your GSXMLDocument.  Create
 * a GSXPathContext for it.
 *
 * GSXPathContext *p = [[GSXPathContext alloc] initWithDocument: document];
 *
 * Then, you can use it to evaluate XPath expressions:
 *
 * GSXPathString *result = [p evaluateExpression: @"string(/body/text())"];
 * NSLog (@"Got %@", [result stringValue]);
 *
 * If the XML document contains namespaces, you first need to register them
 * with the parser by using registerNamespaceWithPrefix:href:, as in:
 *
 * GSXPathContext *p = [[GSXPathContext alloc] initWithDocument: document];
 * if ([p registerNamespaceWithPrefix: @"a"  href="http://www.gnustep.org/demo"] == NO)
 *  {
 *    // Error registering namespace, do something about it.
 *  }
 *
 * and then you can use the namespace prefix in your expressions, as in
 *
 * GSXPathString *result = [p evaluateExpression: @"string(/a:body/text())"];
 * NSLog (@"Got %@", [result stringValue]);
 *
 */
GS_EXPORT_CLASS
@interface GSXPathContext : NSObject
{
  void		*_lib;		// xmlXPathContext
  GSXMLDocument *_document;
}
- (id) initWithDocument: (GSXMLDocument*)d;
- (GSXPathObject*) evaluateExpression: (NSString*)XPathExpression;

/*
 * Registers a new namespace.  Return YES if succesful, NO if not.
 */
- (BOOL) registerNamespaceWithPrefix: (NSString *)prefix
                                href: (NSString *)href;
@end

/** XPath queries return a GSXPathObject.  GSXPathObject in itself is
 * an abstract class; there are four types of completely different
 * GSXPathObject types, listed below.  I'm afraid you need to check
 * the returned type of each GSXPath query to make sure it's what you
 * meant it to be.
 */
GS_EXPORT_CLASS
@interface GSXPathObject : NSObject
{
  void		*_lib;		// xmlXPathObject
  GSXPathContext *_context;
}
@end

/**
 * For XPath queries returning true/false.
 */
GS_EXPORT_CLASS
@interface GSXPathBoolean : GSXPathObject
- (BOOL) booleanValue;
@end

/**
 * For XPath queries returning a number.
 */
GS_EXPORT_CLASS
@interface GSXPathNumber : GSXPathObject
- (double) doubleValue;
@end

/**
 * For XPath queries returning a string.
 */
GS_EXPORT_CLASS
@interface GSXPathString : GSXPathObject
- (NSString *) stringValue;
@end

/**
 * For XPath queries returning a node set.
 */
GS_EXPORT_CLASS
@interface GSXPathNodeSet : GSXPathObject
- (NSUInteger) count;
- (NSUInteger) length;

/** Please note that index starts from 0.  */
- (GSXMLNode *) nodeAtIndex: (NSUInteger)index;
@end

@interface GSXMLDocument (XSLT)
+ (GSXMLDocument*) xsltTransformFile: (NSString*)xmlFile
                          stylesheet: (NSString*)xsltStylesheet
		              params: (NSDictionary*)params;
			
+ (GSXMLDocument*) xsltTransformFile: (NSString*)xmlFile
                          stylesheet: (NSString*)xsltStylesheet;
			
+ (GSXMLDocument*) xsltTransformXml: (NSData*)xmlData
                         stylesheet: (NSData*)xsltStylesheet
		             params: (NSDictionary*)params;
			
+ (GSXMLDocument*) xsltTransformXml: (NSData*)xmlData
                         stylesheet: (NSData*)xsltStylesheet;
			
- (GSXMLDocument*) xsltTransform: (GSXMLDocument*)xsltStylesheet
                          params: (NSDictionary*)params;
			
- (GSXMLDocument*) xsltTransform: (GSXMLDocument*)xsltStylesheet;
@end



#import	<Foundation/NSURLHandle.h>
#import	<Foundation/NSURLConnection.h>

@class	NSArray;
@class	NSDictionary;
@class	NSTimer;
@class	GSXMLNode;
@class	GSXMLRPC;

/**
 * <p>The GSXMLRPC class provides methods for constructing and parsing
 * XMLRPC method call and response documents ... so that calls may
 * be constructed of standard objects.
 * </p>
 * <p>The correspondence between XMLRPC values and Objective-C objects
 * is as follows -
 * </p>
 * <list>
 *   <item><strong>i4</strong> (or <em>int</em>) is an [NSNumber] other
 *   than a real/float or boolean.</item>
 *   <item><strong>boolean</strong> is an [NSNumber] created as a BOOL.</item>
 *   <item><strong>string</strong> is an [NSString] object.</item>
 *   <item><strong>double</strong> is an [NSNumber] created as a float or
 *   double.</item>
 *   <item><strong>dateTime.iso8601</strong> is an [NSDate] object.</item>
 *   <item><strong>base64</strong> is an [NSData] object.</item>
 *   <item><strong>array</strong> is an [NSArray] object.</item>
 *   <item><strong>struct</strong> is an [NSDictionary] object.</item>
 * </list>
 * <p>If you attempt to use any other type of object in the construction
 * of an XMLRPC document, the [NSObject-description] method of that
 * object will be used to create a striong, and the resulting object
 * will be encoded as an XMLRPC <em>string</em> element.
 * </p>
 * <p>In particular, the names of members in a <em>struct</em> must be strings,
 * so if you provide an [NSDictionary] object to represent a <em>struct</em>
 * the keys of the dictionary will be converted to strings if necessary.
 * </p>
 * <p>The class also provides a method for making a synchronous XMLRPC
 * method call (with timeout), or an asynchronous call in which the
 * call completion is handled by a delegate.
 * </p>
 * <p>You may also use the class to implement an XMLRPC server, by calling
 * the -parseMethod:params: method to parse the data POSTed to your server,
 * and -buildResponseWithParams: (or -buildResponseWithFaultCode:andString:)
 * to produce the data to be sent back to the client.
 * </p>
 * <p>In order to simply make a synchronous XMLRPC call to a server, all
 * you need to do is write code like:
 * </p>
 * <example>
 *   GSXMLRPC	*server = [[GSXMLRPC alloc] initWithURL: @"http://server/path"];
 *   id		result = [server makeMethodCall: name params: p timeout: 30];
 * </example>
 * <p>Saying that you want to call the specified method ('name') on  the server,
 * passing the parameters ('p') and with a 30 second timeout.<br />
 * If there is a network or http-level error or a timeout, the result will be
 * an error string, otherwise it will be an array (on success) or a dictionary
 * containing the fault details.
 * </p>
 */
@interface	GSXMLRPC : NSObject <NSURLHandleClient>
{
@private
#if	defined(GNUSTEP)
  NSURLHandle		*handle;
  NSString		*connectionURL GS_UNUSED_IVAR;
  NSURLConnection	*connection GS_UNUSED_IVAR;
  NSMutableData		*response GS_UNUSED_IVAR;
#else
  id			handle GS_UNUSED_IVAR;
  NSString		*connectionURL;
  NSURLConnection	*connection;
  NSMutableData		*response;
#endif
  NSTimer		*timer;
  id			result;
  id			delegate;	// Not retained.
  NSTimeZone		*tz;
  BOOL			compact;
}

/**
 * Given a method name and an array of parameters, this method constructs
 * the XML document for the corresponding XMLRPC call and returns the
 * document as an NSData object containing UTF-8 text.<br />
 * The params array may be empty or nil if there are no parameters to be
 * passed.<br />
 * The method returns nil if passed an invalid method name (a method name
 * may contain any of the ascii alphanumeric characters and underscore,
 * fullstop, colon, or slash).<br />
 * This method is used internally when sending an XMLRPC method call to
 * a remote system, but you can also call it yourself.
 */
- (NSData*) buildMethod: (NSString*)method 
	         params: (NSArray*)params;

/**
 * Given a method name and an array of parameters, this method constructs
 * the XML document for the corresponding XMLRPC call and returns the
 * document as a string.<br />
 * The params array may be empty or nil if there are no parameters to be
 * passed.<br />
 * The method returns nil if passed an invalid method name (a method name
 * may contain any of the ascii alphanumeric characters and underscore,
 * fullstop, colon, or slash).<br />
 */
- (NSString*) buildMethodCall: (NSString*)method 
                       params: (NSArray*)params;

/**
 * Constructs an XML document for an XMLRPC fault response with the
 * specified code and string.  The resulting document is returned
 * as a string.<br />
 * This method is intended for use by applications acting as XMLRPC servers.
 */
- (NSString*) buildResponseWithFaultCode: (NSInteger)code
                               andString: (NSString*)s;

/**
 * Builds an XMLRPC response with the specified array of parameters and
 * returns the document as a string.<br />
 * The params array may be empty or nil if there are no parameters to be
 * returned (an empty params element will be created).<br />
 * This method is intended for use by applications acting as XMLRPC servers.
 */
- (NSString*) buildResponseWithParams: (NSArray*)params;

/**
 * Return the value set by a prior call to -setCompact: (or NO ... the default).
 */
- (BOOL) compact;

/**
 * Returns the delegate previously set by the -setDelegate: method.<br />
 * The delegate handles completion of asynchronous method calls to the
 * URL specified when the receiver was initialised (if any).
 */
- (id) delegate;

/**
 * Initialise the receiver to make XMLRPC calls to the specified URL.<br />
 * This method just calls -initWithURL:certificate:privateKey:password:
 * with nil arguments for the SSL credentials.
 */
- (id) initWithURL: (NSString*)url;

/** <init />
 * Initialise the receiver to make XMLRPC calls to the specified url
 * and (optionally) with the specified SSL parameters.<br />
 * The url argument may be nil, in which case the receiver will be
 * unable to make XMLRPC calls, but can be used to parse incoming
 * requests and build responses.<br />
 * If the SSL credentials are non-nil, connections to the remote server
 * will be authenticated using the supplied certificate so that the
 * remote system knows who is contacting it.
 */
- (id) initWithURL: (NSString*)url
       certificate: (NSString*)cert
        privateKey: (NSString*)pKey
	  password: (NSString*)pwd;

/**
 * Calls -sendMethodCall:params:timeout: and waits for the response.<br />
 * Returns the response parameters (an array),
 * the response fault (a dictionary),
 * or a failure reason (a string).
 */
- (id) makeMethodCall: (NSString*)method
	       params: (NSArray*)params
	      timeout: (NSInteger)seconds;

/**
 * Parses XML data containing an XMLRPC method call.<br />
 * Returns the name of the method call.<br />
 * Empties, and then places the method parameters (if any)
 * in the params argument.<br />
 * NB. Any containers (arrays or dictionaries) in the parsed parameters
 * will be mutable, so you can modify this data structure as you like.<br />
 * Raises an exception if parsing fails.<br />
 * This method is intended for the use of XMLRPC server applications.
 */
- (NSString*) parseMethod: (NSData*)request
		   params: (NSMutableArray*)params;

/**
 * Parses XML data containing an XMLRPC method response.<br />
 * Returns nil for success, the fault dictionary on failure.<br />
 * Places the response parameters (if any) in the params argument.<br />
 * NB. Any containers (arrays or dictionaries) in the parsed parameters
 * will be mutable, so you can modify this data structure as you like.<br />
 * Raises an exception if parsing fails.<br />
 * Used internally when making a method call to a remote server.
 */
- (NSDictionary*) parseResponse: (NSData*)resp
			 params: (NSMutableArray*)params;

/**
 * Returns the result of the last method call, or nil if there has been
 * no method call or one is in progress.<br />
 * The result may be one of -
 * <list>
 *   <item>A mutable array ... the parameters of a success response.</item>
 *   <item>A dictionary ... containing a fault response.</item>
 *   <item>A string ... describing a low-level failure (eg. timeout).</item>
 * </list>
 * NB. Any containers (arrays or dictionaries) in the parsed parameters
 * of a success response will be mutable, so you can modify this data
 * structure as you like.
 */
- (id) result;

/**
 * Send an asynchronous XMLRPC method call with the specified timeout.<br />
 * A delegate should have been set to handle the result of this call,
 * but if one was not set the state of the asynchronous call may be polled
 * by calling the -result method, which will return nil as long as the
 * call has not completed.<br />
 * The call may be cancelled by calling the -timeout: method<br />
 * This method returns YES if the call was started,
 * NO if it could not be started
 * (eg because another call is in progress or because of bad arguments).<br />
 * NB. For the asynchronous operation to proceed, the current [NSRunLoop]
 * must be run.
 */
- (BOOL) sendMethodCall: (NSString*)method
		 params: (NSArray*)params
		timeout: (NSInteger)seconds;

/**
 * Specify whether to generate compact XML (omit indentation and other white
 * space and omit &lt;string&gt; element markup).<br />
 * Compact representation saves some space (can be important when sent over
 * slow/low bandwidth connections), but sacrifices readability.
 */
- (void) setCompact: (BOOL)flag;

/**
 * Specify whether to perform debug trace on I/O<br />
 * Return the previous value
 */
- (int) setDebug: (int)flag;

/**
 * Sets the delegate object which will receive callbacks when an XMLRPC
 * call completes.<br />
 * NB. this delegate is <em>not</em> retained, and should be removed
 * before it is deallocated (call -setDelegate: again with a nil argument
 * to remove the delegate).
 */
- (void) setDelegate: (id)aDelegate;

/**
 * Sets the time zone for use when sending/receiving date/time values.<br />
 * The XMLRPC specification says that timezone is server dependent so you
 * will need to set it according to the server you are connecting to.<br />
 * If this is not set, UCT is assumed.
 */
- (void) setTimeZone: (NSTimeZone*)timeZone;

/**
 * Handles timeouts, passing information to delegate ... you don't need to
 * call this method, but you <em>may</em> call it in order to cancel an
 * asynchronous request as if it had timed out.
 */
- (void) timeout: (NSTimer*)t;

/**
 * Return the time zone currently set.
 */
- (NSTimeZone*) timeZone;

#ifdef GNUSTEP
/** <override-dummy />
 * Allows GSXMLRPC to act as a client of NSURLHandle. Internal use only. */
- (void) URLHandle: (NSURLHandle*)sender
  resourceDataDidBecomeAvailable: (NSData*)newData;
/** <override-dummy />
 * Allows GSXMLRPC to act as a client of NSURLHandle. Internal use only. */
- (void) URLHandle: (NSURLHandle*)sender
  resourceDidFailLoadingWithReason: (NSString*)reason;
/** <override-dummy />
 * Allows GSXMLRPC to act as a client of NSURLHandle. Internal use only. */
- (void) URLHandleResourceDidBeginLoading: (NSURLHandle*)sender;
/** <override-dummy />
 * Allows GSXMLRPC to act as a client of NSURLHandle. Internal use only. */
- (void) URLHandleResourceDidCancelLoading: (NSURLHandle*)sender;
/** <override-dummy />
 * Allows GSXMLRPC to act as a client of NSURLHandle. Internal use only. */
- (void) URLHandleResourceDidFinishLoading: (NSURLHandle*)sender;
#endif

@end

/**
 * Delegates should implement this method in order to be informed of
 * the success or failure of an XMLRPC method call which was initiated
 * by the -sendMethodCall:params:timeout: method.<br />
 */
@interface	GSXMLRPC (Delegate)

/** <override-dummy />
 * Called by the sender when an XMLRPC method call completes (either success
 * or failure). 
 * The delegate may then call the -result method to retrieve the result of
 * the method call from the sender.
 */
- (void) completedXMLRPC: (GSXMLRPC*)sender;
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* OS_API_VERSION(GS_API_NONE,GS_API_NONE) */

#endif /* __GSXML_h_GNUSTEP_BASE_INCLUDE */

