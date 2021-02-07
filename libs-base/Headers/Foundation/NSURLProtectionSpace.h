/* Interface for NSURLProtectionSpace for GNUstep
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

#ifndef __NSURLProtectionSpace_h_GNUSTEP_BASE_INCLUDE
#define __NSURLProtectionSpace_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString;

extern NSString * const NSURLProtectionSpaceFTPProxy;	/** An FTP proxy */
extern NSString * const NSURLProtectionSpaceHTTPProxy;	/** An HTTP proxy */
extern NSString * const NSURLProtectionSpaceHTTPSProxy;	/** An HTTPS proxy */
extern NSString * const NSURLProtectionSpaceSOCKSProxy;	/** A SOCKS proxy */

/** Default authentication (Basic) */
extern NSString * const NSURLAuthenticationMethodDefault;

/** HTML form authentication */
extern NSString * const NSURLAuthenticationMethodHTMLForm;

/** HTTP Basic authentication */
extern NSString * const NSURLAuthenticationMethodHTTPBasic;

/** HTTP Digest authentication */
extern NSString * const NSURLAuthenticationMethodHTTPDigest;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)
extern NSString * const NSURLAuthenticationMethodNTLM;
extern NSString * const NSURLAuthenticationMethodNegotiate;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)
extern NSString * const NSURLAuthenticationMethodClientCertificate;
extern NSString * const NSURLAuthenticationMethodServerTrust;
#endif

/**
 * Class to encapsulate a protection space ... where authentication is
 * required.
 */
GS_EXPORT_CLASS
@interface NSURLProtectionSpace : NSObject <NSCopying>
{
#if	GS_EXPOSE(NSURLProtectionSpace)
  void *_NSURLProtectionSpaceInternal;
#endif
}

/**
 * Returns the authentication method used for this protection space.
 */
- (NSString *) authenticationMethod;

/**
 * Returns the host (or proxy host) set in the receiver.
 */
- (NSString *) host;

/**
 * Initialises the receiver with host, port, and protocol identifying the
 * protection space.  For some protocols the realm identifies a space
 * within the host, for others it may be nil.
 */
- (id) initWithHost: (NSString *)host
	       port: (NSInteger)port
	   protocol: (NSString *)protocol
	      realm: (NSString *)realm
authenticationMethod: (NSString *)authenticationMethod;

/**
 * This is like -initWithHost:port:protocol:realm:authenticationMethod:
 * except that it uses a proxy host and proxy type rather than an actual
 * host and a protocol.
 */
- (id) initWithProxyHost: (NSString *)host
		    port: (NSInteger)port
		    type: (NSString *)type
		   realm: (NSString *)realm
    authenticationMethod: (NSString *)authenticationMethod;

/**
 * Returns a flag to indicate whether this protection space is on a proxy
 * server or not.
 */
- (BOOL) isProxy;

/**
 * Returns the port set for this receiver or zero if none was set.
 */
- (NSInteger) port;

/**
 * Returns the protocol of the receiver or nil if it is a proxy.
 */
- (NSString *) protocol;

/**
 * Returns the proxy type set for the receiver or nil if it's not a proxy.
 */
- (NSString *) proxyType;

/**
 * Returns the realm (or nil) which was set in the receiver upon initialisation.
 */
- (NSString *) realm;

/**
 * Returns a flag to indicate whether the password for this protection space
 * will be sent over a secure mechanism.
 */
- (BOOL) receivesCredentialSecurely;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST) && GS_API_VERSION( 11300,GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, copy) NSArray *distinguishedNames;
#else
- (NSArray *) distinguishedNames;
#endif
#endif

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif
