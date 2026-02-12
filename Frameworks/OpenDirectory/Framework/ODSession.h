/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*!
    @const      ODSessionProxyAddress
    @abstract   the address to connect to via proxy, used when making the options dictionary
    @discussion the address to connect to via proxy, used when making the options dictionary
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyAddress;

/*!
    @const      ODSessionProxyPort
    @abstract   the port to connect to via proxy, used when making the options dictionary
    @discussion the port to connect to via proxy, used when making the options dictionary.  This parameter
                is optional and should not be passed normally.
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyPort;

/*!
    @const      ODSessionProxyUsername
    @abstract   the username to connect with via proxy, used when making the options dictionary
    @discussion the username to connect with via proxy, used when making the options dictionary
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyUsername;

/*!
    @const      ODSessionProxyPassword
    @abstract   the password to connect with via proxy, used when making the options dictionary
    @discussion the password to connect with via proxy, used when making the options dictionary
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyPassword;

/*!
    @class       ODSession
    @abstract    Class for working with OpenDirectory sessions.
    @discussion  Class for working with OpenDirectory sessions.
*/
@interface ODSession : NSObject {
	@private
	void *_internal;
}

/*!
    @method     defaultSession
    @abstract   Returns a shared instance of a local ODSession
    @discussion Returns a shared instance of a local ODSession.  This can be used for most situations unless
                more control is needed over the session.
*/
+ (id)defaultSession;

/*!
    @method     sessionWithOptions:error:
    @abstract   Creates an autoreleased instance of ODSession directed over Proxy to another host
    @discussion Creates an autoreleased instance of ODSession directed over Proxy to another host.  nil
                can be passed for no options. outError is optional parameter, nil can be passed if error
                details are not needed.  Options include:

                If proxy is required then a dictionary with keys should be:
                            Key                             Value
                    ODSessionProxyAddress        NSString(hostname or IP)
                    ODSessionProxyPort           NSNumber(IP port, should not be set as it will default)
                    ODSessionProxyUsername       NSString(username)
                    ODSessionProxyPassword       NSString(password)
*/
+ (id)sessionWithOptions:(NSDictionary *)inOptions error:(NSError **)outError;

/*!
    @method     initWithOptions:error:
    @abstract   Creates an instance of ODSession directed over Proxy to another host
    @discussion Creates an instance of ODSession directed over Proxy to another host.  nil can be
                passed for no options. outError is optional parameter, nil can be passed if error
                details are not needed. Options include:
     
                If proxy is required then a dictionary with keys should be:
                            Key                             Value
                    ODSessionProxyAddress        NSString(hostname or IP)
                    ODSessionProxyPort           NSNumber(IP port, should not be set as it will default)
                    ODSessionProxyUsername       NSString(username)
                    ODSessionProxyPassword       NSString(password)
*/
- (id)initWithOptions:(NSDictionary *)inOptions error:(NSError **)outError;

/*!
    @method     nodeNamesAndReturnError:
    @abstract   Returns the node names that are registered on this ODSession
    @discussion Returns the node names that are registered on this ODSession.  outError can be nil if
                error details are not needed.
*/
- (NSArray *)nodeNamesAndReturnError:(NSError **)outError;

@end
