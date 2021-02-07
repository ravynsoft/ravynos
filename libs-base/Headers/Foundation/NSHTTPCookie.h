/* Interface for NSHTTPCookie for GNUstep
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

#ifndef __NSHTTPCookie_h_GNUSTEP_BASE_INCLUDE
#define __NSHTTPCookie_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray;
@class NSDate;
@class NSDictionary;
@class NSString;
@class NSURL;

extern NSString * const NSHTTPCookieComment; /** Obtain text of the comment */
extern NSString * const NSHTTPCookieCommentURL; /** Obtain the comment URL */
extern NSString * const NSHTTPCookieDiscard; /** Obtain the sessions discard setting */
extern NSString * const NSHTTPCookieDomain; /** Obtain cookie domain */
extern NSString * const NSHTTPCookieExpires; /** Obtain cookie expiry date */
extern NSString * const NSHTTPCookieMaximumAge; /** Obtain maximum age (expiry) */
extern NSString * const NSHTTPCookieName; /** Obtain name of cookie */
extern NSString * const NSHTTPCookieOriginURL; /** Obtain cookie origin URL */
extern NSString * const NSHTTPCookiePath; /** Obtain cookie path */
extern NSString * const NSHTTPCookiePort; /** Obtain cookie ports */
extern NSString * const NSHTTPCookieSecure; /** Obtain cookie security */
extern NSString * const NSHTTPCookieValue; /** Obtain value of cookie */
extern NSString * const NSHTTPCookieVersion; /** Obtain cookie version */


/**
 *  An instance of the NSHTTPCookie class is a single, immutable http cookie.
 *  It can be initialised with properties from a dictionary and has accessor
 *  methods to obtain the cookie values.<br />
 *  The class supports unversioned cookies (sometimes referred to as version 0)
 *  as originally produced by netscape, as well as more recent standardised
 *  and versioned cookies.
 */
GS_EXPORT_CLASS
@interface NSHTTPCookie :  NSObject
{
#if	GS_EXPOSE(NSHTTPCookie)
@private
  void	*_NSHTTPCookieInternal;
#endif
}

/**
 * Allocates and returns an autoreleasd instance using -initWithProperties:
 * to initialise it from properties.
 */
+ (id) cookieWithProperties: (NSDictionary *)properties;

/**
 * Returns an array of cookies parsed from the headerFields and URL
 * (assuming that the headerFields came from a response to a request
 * sent to the URL).<br />
 * The headerFields dictionary must contain at least all the headers
 * relevant to cookie setting ... other headers are ignored.
 */
+ (NSArray *) cookiesWithResponseHeaderFields: (NSDictionary *)headerFields
				       forURL: (NSURL *)URL;

/**
 * Returns a dictionary of header fields that can be used to add the
 * specified cookies to a request.
 */
+ (NSDictionary *) requestHeaderFieldsWithCookies: (NSArray *)cookies;

/**
 * Returns a string which may be used to describe the cookie to the
 * user, or nil if no comment is set.
 */
- (NSString *) comment;

/**
 * Returns a URL where the user can find out about the cookie, or nil
 * if no comment URL is set.
 */
- (NSURL *) commentURL;

/**
 * Returns the domain to which the cookie should be sent.<br />
 * If there is a leading dot then subdomains should also receive the
 * cookie as specified in RFC 2965.
 */
- (NSString *) domain;

/**
 * Returns the expiry date of the receiver or nil if there is no
 * such date.
 */
- (NSDate *) expiresDate;

/** <init />
 *  Initialises the receiver with a dictionary of properties.<br />
 *  Unrecognised keys are ignored.<br />
 *  Returns nil if a required key is missing or if an illegal
 *  value is specified for a key.
 *  <deflist>
 *    <term>NSHTTPCookieComment</term>
 *    <desc>
 *      The [NSString] comment for the cookie (if any).<br />
 *      This is nil by default and for unversioned cookies.
 *    </desc>
 *    <term>NSHTTPCookieCommentURL</term>
 *    <desc>
 *      The [NSString] or [NSURL] URL to get the comment for the cookie.<br />
 *      This is nil by default and for unversioned cookies.
 *    </desc>
 *    <term>NSHTTPCookieDomain</term>
 *    <desc>
 *      The [NSString] specified the domain to which the cookie applies.<br />
 *      This is extracted from NSHTTPCookieOriginURL if not specified.
 *    </desc>
 *    <term>NSHTTPCookieDiscard</term>
 *    <desc>
 *      A [NSString] (either TRUE or FALSE) saying whether the cookie
 *      is to be discarded when the session ends.<br />
 *      Defaults to FALSE except for versioned cookies where
 *      NSHTTPCookieMaximumAge is unspecified.
 *    </desc>
 *    <term>NSHTTPCookieExpires</term>
 *    <desc>
 *      The [NSDate] or [NSString] (format Wdy, DD-Mon-YYYY HH:MM:SS GMT)
 *      specifying when an unversioned cookie expires and ignored for
 *      versioned cookies.
 *    </desc>
 *    <term>NSHTTPCookieMaximumAge</term>
 *    <desc>
 *      An [NSString] containing an integer value specifying the longest time
 *      (in seconds) for which the cookie is valid.<br />
 *      This defaults to zero and is only meaningful for versioned cookies.
 *    </desc>
 *    <term>NSHTTPCookieName</term>
 *    <desc>
 *      An [NSString] ... obvious ... no default value.
 *    </desc>
 *    <term>NSHTTPCookieOriginURL</term>
 *    <desc>
 *      An [NSString] or [NSURL] specifying the URL which set the cookie.<br />
 *      Must be supplied if NSHTTPCookieDomain is not.
 *    </desc>
 *    <term>NSHTTPCookiePath</term>
 *    <desc>
 *      An [NSString] specifying the path from the cookie.<br />
 *      If unspecified this value is determined from NSHTTPCookieOriginURL
 *      or defaults to '/'.
 *    </desc>
 *    <term>NSHTTPCookiePort</term>
 *    <desc>
 *      An [NSString] containing a comma separated list of integer port
 *      numbers.  This is valid for versioned cookies and defaults to
 *      an empty string.
 *    </desc>
 *    <term>NSHTTPCookieSecure</term>
 *    <desc>
 *      An [NSString] saying whether the cookie may be sent over
 *      insecure connections.<br />
 *      The default is FALSE meaning that it may be sent insecurely.
 *    </desc>
 *    <term>NSHTTPCookieValue</term>
 *    <desc>
 *      An [NSString] containing the whole value of the cooke.<br />
 *      This parameter <strong>must</strong> be provided.
 *    </desc>
 *    <term>NSHTTPCookieVersion</term>
 *    <desc>
 *      An [NSString] specifying the cookie version ... for an
 *      unversioned cookie (the default) this is '0'.<br />
 *      Also supports version '1'.
 *    </desc>
 *  </deflist>
 */
- (id) initWithProperties: (NSDictionary *)properties;

/**
 * Returns whether the receiver should only be sent over
 * secure connections.
 */
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, getter=isSecure) BOOL secure;
#else
- (BOOL) isSecure;
#endif

/**
 * Returns whether the receiver should be destroyed at the end of the
 * session.
 */
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, getter=isSessionOnly) BOOL sessionOnly;
#else
- (BOOL) isSessionOnly;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, getter=isHTTPOnly) BOOL HTTPOnly;
#else
- (BOOL) isHTTPOnly;
#endif
#endif

/**
 * Returns the name of the receiver.
 */
- (NSString *) name;

/**
 * Returns the URL path within the cookie's domain for which
 * this cookie must be sent.
 */
- (NSString *) path;

/**
 * Returns the list of ports to which the receiver should be sent,
 * or nil if the cookie can be used for any port.
 */
- (NSArray *) portList;

/**
 * Returns a dictionary representation of the receiver which could be
 * used as the argument for -initWithProperties: to recreate a copy
 * of the receiver.
 */
- (NSDictionary *) properties;

/**
 * Returns the value of the receiver.
 */
- (NSString *) value;

/**
 * Returns 0 for an unversioned Netscape style cookie or a
 * positive integer for a versioned cookie.
 */
- (NSUInteger) version;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* 100200 */

#endif	/* __NSHTTPCookie_h_GNUSTEP_BASE_INCLUDE */
