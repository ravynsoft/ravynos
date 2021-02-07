/** Interface for MIME parsing classes

   Copyright (C) 2000-2016 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald  <rfm@gnu.org>

   Date: October 2000
   
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

   AutogsdocSource: Additions/GSMime.m
*/

#ifndef __GSMime_h_GNUSTEP_BASE_INCLUDE
#define __GSMime_h_GNUSTEP_BASE_INCLUDE
#import <GNUstepBase/GSVersionMacros.h>

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

#ifdef NeXT_Foundation_LIBRARY
#import <Foundation/Foundation.h>
#else
#import	<Foundation/NSObject.h>
#import	<Foundation/NSString.h>
#import	<Foundation/NSMapTable.h>
#endif

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSArray;
@class	NSMutableArray;
@class	NSData;
@class	NSMutableData;
@class	NSDictionary;
@class	NSMutableDictionary;
@class	NSScanner;

/*
 * A trivial class for mantaining state while decoding/encoding data.
 * Each encoding type requires its own subclass.
 */
GS_EXPORT_CLASS
@interface	GSMimeCodingContext : NSObject
{
  BOOL		atEnd;	/* Flag to say that data has ended.	*/
}
- (BOOL) atEnd;
- (BOOL) decodeData: (const void*)sData
             length: (NSUInteger)length
	   intoData: (NSMutableData*)dData;
- (void) setAtEnd: (BOOL)flag;
@end

GS_EXPORT_CLASS
@interface      GSMimeHeader : NSObject <NSCopying>
{
#if	GS_EXPOSE(GSMimeHeader)
  NSString              *name;
  NSString              *lower;
  NSString              *value;
  NSMutableDictionary   *objects;
  NSMutableDictionary	*params;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}
+ (GSMimeHeader*) headerWithName: (NSString*)n
                           value: (NSString*)v
                      parameters: (NSDictionary*)p;
+ (NSString*) makeQuoted: (NSString*)v always: (BOOL)flag;
+ (NSString*) makeToken: (NSString*)t preservingCase: (BOOL)preserve;
+ (NSString*) makeToken: (NSString*)t;
- (id) copyWithZone: (NSZone*)z;
/** How big this header might be when represented as raw MIME data.
 */
- (NSUInteger) estimatedSize;
- (NSString*) fullValue;
- (id) initWithName: (NSString*)n
	      value: (NSString*)v;
- (id) initWithName: (NSString*)n
	      value: (NSString*)v
	 parameters: (NSDictionary*)p;
- (NSString*) name;
- (NSString*) namePreservingCase: (BOOL)preserve;
- (id) objectForKey: (NSString*)k;
- (NSDictionary*) objects;
- (NSString*) parameterForKey: (NSString*)k;
- (NSDictionary*) parameters;
- (NSDictionary*) parametersPreservingCase: (BOOL)preserve;
- (NSMutableData*) rawMimeData;
- (NSMutableData*) rawMimeDataPreservingCase: (BOOL)preserve;
- (NSMutableData*) rawMimeDataPreservingCase: (BOOL)preserve
                                    foldedAt: (NSUInteger)fold;
- (void) rawMimeDataPreservingCase: (BOOL)preserve
                          foldedAt: (NSUInteger)fold
                                to: (NSMutableData*)md;
- (void) setObject: (id)o  forKey: (NSString*)k;
- (void) setParameter: (NSString*)v forKey: (NSString*)k;
- (void) setParameters: (NSDictionary*)d;
- (void) setValue: (NSString*)s;
- (NSString*) text;
- (NSString*) value;
@end

GS_EXPORT_CLASS
@interface	GSMimeDocument : NSObject <NSCopying>
{
#if	GS_EXPOSE(GSMimeDocument)
  NSMutableArray	*headers;
  id			content;
#endif
#if	!GS_NONFRAGILE
  void			*_unused;
#endif
}

+ (NSString*) charsetFromEncoding: (NSStringEncoding)enc;

/**
 * Decode the source data from base64 encoding and return the result.<br />
 * The source data is expected to be ASCII text and may be multiple
 * lines or a line of any length (decoding is very tolerant).
 */
+ (NSData*) decodeBase64: (NSData*)source;
+ (NSString*) decodeBase64String: (NSString*)source;
+ (GSMimeDocument*) documentWithContent: (id)newContent
				   type: (NSString*)type
				   name: (NSString*)name;
/**
 * Encode the source data to base64 encoding and return the result.<br />
 * The resulting data is ASCII text and contains only the base64 encoded
 * values with no line breaks or extraneous data.  This is base64 encoded
 * data in it's general format as mandated in RFC 3548.  If the data is
 * to be used as part of a MIME document body, line breaks must be
 * introduced at 76 byte intervals (GSMime does this when automatically
 * encoding data for you).  If the data is to be used in a PEM document
 * line breaks must be introduced at 74 byte intervals.
 */
+ (NSData*) encodeBase64: (NSData*)source;
+ (NSString*) encodeBase64String: (NSString*)source;
+ (NSStringEncoding) encodingFromCharset: (NSString*)charset;

- (void) addContent: (id)newContent;
- (void) addHeader: (GSMimeHeader*)info;
- (GSMimeHeader*) addHeader: (NSString*)name
                      value: (NSString*)value
		 parameters: (NSDictionary*)parameters;
- (NSArray*) allHeaders;
- (id) content;
- (id) contentByID: (NSString*)key;
- (id) contentByLocation: (NSString*)key;
- (id) contentByName: (NSString*)key;
- (id) copyWithZone: (NSZone*)z;
- (NSString*) contentFile;
- (NSString*) contentID;
- (NSString*) contentLocation;
- (NSString*) contentName;
- (NSString*) contentSubtype;
- (NSString*) contentType;
- (NSArray*) contentsByName: (NSString*)key;
- (void) convertTo7BitSafe;
- (void) convertToBase64;       // DEPRECATED ... use convertTo7BitSafe
- (void) convertToBinary;
- (NSData*) convertToData;
- (NSString*) convertToText;
- (void) deleteContent: (GSMimeDocument*)aPart;
- (void) deleteHeader: (GSMimeHeader*)aHeader;
- (void) deleteHeaderNamed: (NSString*)name;
/** How big this document might be when represented as raw MIME data.
 */
- (NSUInteger) estimatedSize;
- (GSMimeHeader*) headerNamed: (NSString*)name;
- (NSArray*) headersNamed: (NSString*)name;
- (NSString*) makeBoundary;
- (GSMimeHeader*) makeContentID;
- (GSMimeHeader*) makeHeader: (NSString*)name
                       value: (NSString*)value
		  parameters: (NSDictionary*)parameters;
- (GSMimeHeader*) makeMessageID;
- (NSMutableData*) rawMimeData;
- (NSMutableData*) rawMimeData: (BOOL)isOuter;
- (NSMutableData*) rawMimeData: (BOOL)isOuter foldedAt: (NSUInteger)fold;
- (void) setContent: (id)newContent;
- (void) setContent: (id)newContent
	       type: (NSString*)type;
- (void) setContent: (id)newContent
	       type: (NSString*)type
	       name: (NSString*)name;
- (void) setContentType: (NSString*)newType;
- (void) setHeader: (GSMimeHeader*)info;
- (GSMimeHeader*) setHeader: (NSString*)name
                      value: (NSString*)value
		 parameters: (NSDictionary*)parameters;

@end

GS_EXPORT_CLASS
@interface	GSMimeParser : NSObject
{
#if	GS_EXPOSE(GSMimeParser)
  NSMutableData		*data;
  unsigned char		*bytes;
  unsigned		dataEnd;
  unsigned		sectionStart;
  unsigned		lineStart;
  unsigned		lineEnd;
  unsigned		input;
  /* During header parsing, we use this field to count white space we are
   * expecting to have after an encoded word.
   * During bnody parsing, we use the field to count expected content bytes.
   */
  unsigned		expect;
  unsigned		rawBodyLength;
  struct {
    unsigned int	inBody:1;
    unsigned int	isHttp:1;
    unsigned int	complete:1;
    unsigned int	hadErrors:1;
    unsigned int	buggyQuotes:1;
    unsigned int	wantEndOfLine:1;
    unsigned int	excessData:1;
    unsigned int	headersOnly:1;
    unsigned int        encodedWord:1;
  } flags;
  NSData		*boundary;	// Also overloaded to hold excess
  GSMimeDocument	*document;
  GSMimeParser		*child;
  GSMimeCodingContext	*context;
  NSStringEncoding	_defaultEncoding;
#endif
#if	!GS_NONFRAGILE
  void			*_unused;
#endif
}

+ (GSMimeDocument*) documentFromData: (NSData*)mimeData;
+ (GSMimeParser*) mimeParser;

- (GSMimeCodingContext*) contextFor: (GSMimeHeader*)info;
- (NSMutableData*) data;
- (BOOL) decodeData: (NSData*)sData
	  fromRange: (NSRange)aRange
	   intoData: (NSMutableData*)dData
	withContext: (GSMimeCodingContext*)con;
- (NSData*) excess;
- (void) expectNoHeaders;
- (BOOL) isComplete;
- (BOOL) isHttp;
- (BOOL) isInBody;
- (BOOL) isInHeaders;
- (GSMimeDocument*) mimeDocument;
- (BOOL) parse: (NSData*)d;
/** Parses headers from the supplied data returning YES if more data is
 * needed before the end of thge headers are reached.<br />
 * If body is not NULL and the end of the headers were reached leaving
 * some unused data, that remaining data is returned.<br />
 * NB. The returned data is a reference to part of the original memory
 * buffer provided in d, so you must copy it if you intend to use it after
 * modifying or deallocating the original data.
 */
- (BOOL) parseHeaders: (NSData*)d remaining: (NSData**)body;
- (BOOL) parseHeader: (NSString*)aHeader;
- (BOOL) scanHeaderBody: (NSScanner*)scanner into: (GSMimeHeader*)info;
- (NSString*) scanName: (NSScanner*)scanner;
- (BOOL) scanPastSpace: (NSScanner*)scanner;
- (NSString*) scanSpecial: (NSScanner*)scanner;
- (NSString*) scanToken: (NSScanner*)scanner;
- (void) setBuggyQuotes: (BOOL)flag;
- (void) setDefaultCharset: (NSString*)aName;
- (void) setHeadersOnly;
- (void) setIsHttp;
@end

/** Instances of the GSMimeSerializer class are used to serialise
 * GSMimeDocument objects to NSMutableData objects, producing data
 * in a form suitable for sending as an Email over the SMTP protocol
 * or in other forms.
 */
GS_EXPORT_CLASS
@interface GSMimeSerializer : NSObject <NSCopying>
{
  NSUInteger    foldAt;         /** Fold long lines at this position */
  BOOL          use8bit;        /** Output does not need to be 7bit-safe */
  NSString      *dataEncoding;  /** To make 8bit data 7bit-safe */
  NSString      *textEncoding;  /** To make 8bit text 7bit-safe */
}

/** Returns an autorelease GSMimeSerializer configured for transfer
 * over binary safe protocols with unliumited line lenth).
 */
+ (GSMimeSerializer*) binarySerializer;

/** Returns an autorelease GSMimeSerializer configured for Email
 * to be sent as 7bit data over SMTP.
 */
+ (GSMimeSerializer*) smtp7bitSerializer;

/** Returns a copy of the receiver.
 */
- (id) copyWithZone: (NSZone*)aZone;

/** Returns the default content transfer encoding used when 8bit data needs
 * to be made 7bit safe.  This is base64 by default.
 */
- (NSString*) dataEncoding;

/** Encodes the document and returns the resulting raw mime data.
 */
- (NSMutableData*) encodeDocument: (GSMimeDocument*)document;

/** Appends a document part to the supplied data object.
 */
- (void) encodePart: (GSMimeDocument*)document to: (NSMutableData*)md;

/** Returns the maximum line length (excluding the trailing CRLF) to which
 * we will encode data. See also the -setFoldAt: method.
 */ 
- (NSUInteger) foldAt;

/** This method allows you to control the position at which lines in
 * headers and the body data are wrapped.<br />
 * RFC 2822 says that the absolute maximum (except for 'binary' content
 * transfer encoding) is 998 (excluding CRLF), but the recommended
 * maximum is 78 so we use that by default.<br />
 * Setting any ridiculously short value (less than 20) or an excessively
 * long value (greater than the 998 character limit supported by SMTP)
 * actually sets a value of zero, meaning that there is no limit.
 */
- (void) setFoldAt: (NSUInteger)position;

/** Sets the content transfer encoding used when 8bit data needs to be sent
 * in a 7bit safe form.<br />
 * Setting a nil/empty encoding reverts to the default (base64).<br />
 * Setting an unknown/inapplicable encoding raises an exception.
 */
- (void) setDataEncoding: (NSString*)encoding;

/** Sets the content transfer encoding used when 8bit text needs to be sent
 * in a 7bit safe form.<br />
 * Setting a nil/empty encoding reverts to the default (quoted-printable).<br />
 * Setting an unknown/inapplicable encoding raises an exception.
 */
- (void) setTextEncoding: (NSString*)encoding;

/** Sets whether we will allow 8bit data in the output.<br />
 * The default is NO (because 8bit data breaks some mail transfer agents).
 */
- (void) setUse8bit: (BOOL)aFlag;

/** Returns the default content transfer encoding used when 8bit text needs
 * to be made 7bit safe.  This is quoted-printable by default.
 */
- (NSString*) textEncoding;

/** Returns YES is we will allow 8bit data in the output, NO if we encode
 * everything in a 7bit safe form.
 */
- (BOOL) use8bit;

@end



/** The error domain for the GSMime system.
 */
GS_EXPORT NSString* const GSMimeErrorDomain;

/** The error codes used in the GSMimeErrorDomain
 */
typedef enum {
  GSMimeSMTPAbort,
  GSMimeSMTPTimeout,
  GSMimeSMTPCommsEnd,
  GSMimeSMTPCommsError,
  GSMimeSMTPServerResponse
} GSMimeErrorCode;

@class	NSError;
@class	NSStream;
@class	NSTimer;

/** The GSMimeSMTPClient class provides the ability to send EMails
 * ([GSMimeDocument] instances) via an SMTP server.
 */
GS_EXPORT_CLASS
@interface	GSMimeSMTPClient : NSObject
{
#if	GS_NONFRAGILE
#  if	defined(GS_GSMimeSMTPClient_IVARS)
@public
GS_GSMimeSMTPClient_IVARS;
#  endif
#else
@private id _internal;
#endif
}

/** Shut down any message send in progress and abort any queued messages.
 */
- (void) abort;

/** Returns the current delegate.
 */
- (id) delegate;

/** Tries to flush any queued messages to the SMTP server, completing by the
 * specified limit date.<br />
 * If limit is nil then a date in the distant future is used.<br />
 * If the queue is emptied in time, this method returns YES, otherwise it
 * returns NO.
 */ 
- (BOOL) flush: (NSDate*)limit;

/** Returns the last error encountered, or nil if there is none recorded.
 */
- (NSError*) lastError;

/** Returns the number of messages currently in the queue.
 */ 
- (NSUInteger) queueSize;

/** Add the message to the queue of emails to be sent by the receiver.
 */
- (void) send: (GSMimeDocument*)message;

/** Add the message to the queue of emails to be sent by the receiver.<br />
 * Also adds an envelope ID string to be used to uniquely identify the
 * message for delivery receipting purposes.<br />
 * For this to work, the SMTP gateway being used must support the SMTP
 * service extension for delivery status notification (RFC 3460).
 */
- (void) send: (GSMimeDocument*)message envelopeID: (NSString*)envid;

/** Set the delegate to receive callback methods indicating when a message
 * is sent, failed, or removed from the queue unsent.
 */
- (void) setDelegate: (id)d;

/** Set the host for the SMTP server.  If this is not set (or is set to nil)
 * then the GSMimeSMTPClientHost user default is used.  If the host is nil
 * or an empty string then 'localhost' is used.
 */
- (void) setHostname: (NSString*)s;

/** Set the host for the SMTP client to identify itsself to the server.
 * If this is not set (or is set to nil) then the GSMimeSMTPClientIdentity
 * user default is used.  If the identity is nil or an empty string then
 * a name of the current host is use.
 */
- (void) setIdentity: (NSString*)s;

/** Sets the maximum number of messages which may remain in the queue.
 * If this is exceeded then any unsuccessful send attempt results in
 * excess queued messages discarded as unsent.<br />
 * The method returns the previous setting.
 */
- (NSUInteger) setMaximum: (NSUInteger)m;
 
/** Set the originator for any emails sent by the SMTP client.<br />
 * This overrides the value in the 'from' header of an email.<br />
 * If this is not set (or is set to nil) then the GSMimeSMTPClientOriginator
 * user default is used.  If the originator is nil or an empty string then
 * the value in the 'from' header of the email is used.
 */
- (void) setOriginator: (NSString*)s;

/** Set the port for the SMTP server.  If this is not set (or is set to nil)
 * then the GSMimeSMTPClientPort user default is used.  If the port is not an
 * integer in the 1-65535 range, then '25' (the default SMTP port) is used.
 */
- (void) setPort: (NSString*)s;

/** Set the username for authentication to the SMTP server.
 * If this is not set (or is set to nil) then the GSMimeSMTPClientUsername
 * user default is used.  If the username is nil or an empty string then
 * authentication is not attempted.
 */
- (void) setUsername: (NSString*)s;

/** returns the receivers current state.
 */
- (int) state;

/** Returns a string describing the receiver's current state
 */
- (NSString*) stateDesc;

@end

/** Informal protocol for delegates of the GSMimeSMTPClient class.
 * The default implementations of these methods do nothing.
 */
@interface	NSObject (GSMimeSMTPClient)
- (void) smtpClient: (GSMimeSMTPClient*)client
	 mimeFailed: (GSMimeDocument*)doc;	/* Failed to send */
- (void) smtpClient: (GSMimeSMTPClient*)client
	 mimeSent: (GSMimeDocument*)doc;	/* Sent successfully */
- (void) smtpClient: (GSMimeSMTPClient*)client
	 mimeUnsent: (GSMimeDocument*)doc;	/* Aborted (not sent) */
@end


#if	defined(__cplusplus)
}
#endif

#endif	/* OS_API_VERSION(GS_API_NONE,GS_API_NONE) */

#endif	/* __GSMime_h_GNUSTEP_BASE_INCLUDE */
