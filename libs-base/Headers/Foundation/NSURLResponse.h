/* Interface for NSURLResponse for GNUstep
   Copyright (C) 2006 Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <frm@gnu.org>
   Date: 2006
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */ 

#ifndef __NSURLResponse_h_GNUSTEP_BASE_INCLUDE
#define __NSURLResponse_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif


@class NSDictionary;
@class NSString;
@class NSURL;

#define NSURLResponseUnknownLength ((long long)-1)

/**
 * The response to an NSURLRequest
 */
GS_EXPORT_CLASS
@interface NSURLResponse :  NSObject <NSCoding, NSCopying>
{
#if	GS_EXPOSE(NSURLResponse)
  void *_NSURLResponseInternal;
#endif
}

/**
 * Returns the expected content length of the receiver or -1 if
 * there is no idea of what the content length might be.<br />
 * This value is advisory, not a definitive length.
 */
- (long long) expectedContentLength;

/**
 * Initialises the receiver with the URL, MIMEType, expected length and
 * text encoding name provided.
 */
- (id) initWithURL: (NSURL *)URL
  MIMEType: (NSString *)MIMEType
  expectedContentLength: (NSInteger)length
  textEncodingName: (NSString *)name;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST)
/**
 * Initialises the receiver with the URL, statusCode, HTTPVersion, and
 * headerFields provided.
 */
- (id) initWithURL: (NSURL*)URL
	statusCode: (NSInteger)statusCode
       HTTPVersion: (NSString*)HTTPVersion
      headerFields: (NSDictionary*)headerFields;
#endif

/**
 * Returns the receiver's MIME type.
 */
- (NSString *) MIMEType;

/**
 * Returns a suggested file name for storing the response data, with
 * suggested names being found in the following order:<br />
 * <list>
 *   <item>content-disposition header</item>
 *   <item>last path component of URL</item>
 *   <item>host name from URL</item>
 *   <item>'unknown'</item>
 * </list>
 * If possible, an extension based on the MIME type of the response
 * is also appended.<br />
 * The result should always be a valid file name.
 */
- (NSString *) suggestedFilename;

/**
 * Returns the name of the character set used where response data is text
 */
- (NSString *) textEncodingName;

/**
 * Returns the receiver's URL.
 */
- (NSURL *) URL;

@end


/**
 * HTTP specific additions to an NSURLResponse
 */
GS_EXPORT_CLASS
@interface NSHTTPURLResponse :  NSURLResponse

/**
 * Returns a string representation of a status code.
 */
+ (NSString *) localizedStringForStatusCode: (NSInteger)statusCode;

/**
 * Returns a dictionary containing all the HTTP header fields.
 */
- (NSDictionary *) allHeaderFields;

/**
 * Returns the HTTP status code for the response.
 */
- (NSInteger) statusCode;

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif
