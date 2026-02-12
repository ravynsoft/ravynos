/*
 *
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "SafariExtensionHandler.h"
#import "SafariExtensionViewController.h"

//#define SHOW_BROWSE_COUNT 1

#if SHOW_BROWSE_COUNT
#include <dns_sd.h>

static void browseReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype, const char *replyDomain, void *context );
#endif

@interface SafariExtensionHandler ()

@property (strong) NSMutableDictionary *    instanceD;
#if SHOW_BROWSE_COUNT
@property (strong) dispatch_queue_t         instanceBrowseQ;
@property (assign) DNSServiceRef            instanceRef;    //  Never released!!!
#endif

@end

@implementation SafariExtensionHandler

- (instancetype)init
{
    if( self = [super init] )
    {
#if SHOW_BROWSE_COUNT
        self.instanceD = [NSMutableDictionary dictionary];
        [self startInstanceBrowse];
#endif
    }
    return( self );
}

- (void)validateToolbarItemInWindow:(SFSafariWindow *)window validationHandler:(void (^)(BOOL enabled, NSString *badgeText))validationHandler {
    // This method will be called whenever some state changes in the passed in window. You should use this as a chance to enable or disable your toolbar item and set badge text.
    (void)window;    // Unused
    validationHandler(YES, _instanceD.count ? [NSNumber numberWithInteger: _instanceD.count].stringValue : @"");
}

- (SFSafariExtensionViewController *)popoverViewController {
    return [SafariExtensionViewController sharedController];
}

#if SHOW_BROWSE_COUNT
- (void)startInstanceBrowse
{
    if( !_instanceBrowseQ )
    {
        self.instanceBrowseQ = dispatch_queue_create( "DNSAllServiceBrowse", DISPATCH_QUEUE_PRIORITY_DEFAULT );
        dispatch_set_context( _instanceBrowseQ, (void *)CFBridgingRetain( self ) );
        dispatch_set_finalizer_f( _instanceBrowseQ, finalizer );
    }
    
    dispatch_sync( _instanceBrowseQ, ^{
        [_instanceD removeAllObjects];
    });
    
    DNSServiceErrorType error;
    if( (error = DNSServiceBrowse( &_instanceRef, 0/*no flags*/, 0, @"_http._tcp".UTF8String, "", browseReply, (__bridge void *)self )) != 0 )
        NSLog(@"DNSServiceBrowse failed error: %ld", error);
    
    if( !error )
    {
        error = DNSServiceSetDispatchQueue( _instanceRef, _instanceBrowseQ );
        if( error ) NSLog( @"DNSServiceSetDispatchQueue error: %d", error );
    }
}

#pragma mark - Dispatch

static void finalizer( void * context )
{
    SafariExtensionHandler *self = (__bridge SafariExtensionHandler *)context;
    NSLog( @"finalizer: %@", self );
    (void)CFBridgingRelease( (__bridge void *)self );
}

#pragma mark - DNS callbacks

static void browseReply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *serviceName, const char *regtype, const char *replyDomain, void *context )
{
    (void)sdRef;            //    Unused
    (void)interfaceIndex;   //    Unused
    (void)errorCode;        //    Unused
    SafariExtensionHandler *self = (__bridge SafariExtensionHandler *)context;
    char fullNameBuffer[kDNSServiceMaxDomainName];
    if( DNSServiceConstructFullName( fullNameBuffer, serviceName, regtype, replyDomain ) == kDNSServiceErr_NoError )
    {
        NSString *fullName = @(fullNameBuffer);
        
        if( flags & kDNSServiceFlagsAdd )
        {
            [self.instanceD setObject: fullName
                               forKey: fullName];
        }
        else
        {
            [self.instanceD removeObjectForKey: fullName];
        }
    }
}
#endif

@end
