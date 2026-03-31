/*
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#import "_CNDomainBrowser.h"
#import "CNDomainBrowserPathUtils.h"
#include <dns_sd.h>

const NSString *    _CNSubDomainKey_defaultFlag         = @"defaultFlag";
const NSString *    _CNSubDomainKey_subPath             = @"subPath";
const NSString *    _CNSubDomainKey_reverseDomainPath   = @"reverseDomainPath";

@interface _CNDomainBrowser ()

@property (assign) DNSServiceRef                    browseDomainR;
@property (strong) NSMutableDictionary *            browseDomainD;

@property (weak)   id<_CNDomainBrowserDelegate>    delegate;

@end

@implementation _CNDomainBrowser

static void enumReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *replyDomain, void *context);

- (instancetype)initWithDelegate:(id<_CNDomainBrowserDelegate>)delegate
{
    if (self = [super init])
    {
        _delegate = delegate;
        [self _commonInit];
    }
    return(self);
}

- (void)_commonInit
{
    self.browseDomainD = [NSMutableDictionary dictionary];
    self.callbackQueue = dispatch_get_main_queue();
}

- (void)dealloc
{
    [self stopBrowser];
}

- (void)startBrowser
{
    if (!_browseDomainR)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            dispatch_queue_t queue = dispatch_queue_create("DNSServiceEnumerateDomains", DISPATCH_QUEUE_PRIORITY_DEFAULT);
            dispatch_set_context(queue, (void *)CFBridgingRetain(self));
            dispatch_set_finalizer_f(queue, finalizer);
            
            DNSServiceRef ref;
            DNSServiceErrorType error;
            if ((error = DNSServiceEnumerateDomains(&ref, self->_browseRegistration ? kDNSServiceFlagsRegistrationDomains : kDNSServiceFlagsBrowseDomains, 0, enumReply, (__bridge void *)self)) != 0)
                NSLog(@"DNSServiceEnumerateDomains failed err: %ld", error);
            else
            {
                self->_browseDomainR = ref;
                (void)DNSServiceSetDispatchQueue(self->_browseDomainR, queue);
            }
        });
    }
}

- (void)stopBrowser
{    
    if (_browseDomainR)
    {
        DNSServiceRefDeallocate(_browseDomainR);
        _browseDomainR = nil;
    }
}

- (BOOL)foundInstanceInMoreThanLocalDomain
{
    BOOL result = YES;
    
    if( self.browseDomainD.count )
    {
        for( NSDictionary *next in [self.browseDomainD allValues] )
        {
            if( [next[_CNSubDomainKey_reverseDomainPath][0] isEqual: @"local"] )            continue;
            else
            {
                result = YES;
                break;
            }
        }
    }
    
    return( result );
}

- (NSArray *)defaultDomainPath
{
    NSArray * revDomainArray = nil;
    
    NSArray *defaults = [[self.browseDomainD allValues] filteredArrayUsingPredicate: [NSPredicate predicateWithFormat: @"(%K == %@)", _CNSubDomainKey_defaultFlag, @YES]];
    if (defaults.count)     revDomainArray = defaults[0][_CNSubDomainKey_reverseDomainPath];
    if (!revDomainArray)	revDomainArray = [NSArray arrayWithObject: @"local"];	//	If no defaults found
    
    return(revDomainArray);
}

- (NSArray *)flattenedDNSDomains
{
    return([self.browseDomainD allKeys]);
}

- (NSArray *)subDomainsAtDomainPath:(NSArray *)domainPath
{
    NSMutableDictionary * subs = [NSMutableDictionary dictionary];
    for (NSDictionary * next in [self.browseDomainD allValues])
    {
        NSArray * bdomain = next[_CNSubDomainKey_reverseDomainPath];
        if (bdomain.count > domainPath.count)
        {
            BOOL	match = YES;
            for (NSUInteger i = 0 ; i < domainPath.count ; i++)
            {
                if (![bdomain[i] isEqualToString: domainPath[i]])	{ match = NO;	break; }
            }
            if (match)
            {
                NSString * key = bdomain[domainPath.count];
                [subs setObject: @{ _CNSubDomainKey_subPath: key, _CNSubDomainKey_defaultFlag: next[_CNSubDomainKey_defaultFlag] } forKey: key];
            }
        }
    }
    return([subs allValues]);
}

- (void) reloadBrowser
{
    if ([_delegate respondsToSelector: @selector(bonjourBrowserDomainUpdate:)])
    {
        dispatch_async(self.callbackQueue, ^{
            [self->_delegate bonjourBrowserDomainUpdate: [self defaultDomainPath]];
        });
    }
}

- (BOOL)isBrowsing
{
    return(_browseDomainR != nil);
}

#pragma mark - Dispatch

static void finalizer(void * context)
{
    _CNDomainBrowser *self = (__bridge _CNDomainBrowser *)context;
//    NSLog(@"finalizer: %@", self);
    (void)CFBridgingRelease((__bridge void *)self);
}

#pragma mark - Commands

#pragma mark - Static Callbacks

static void enumReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
               const char *replyDomain, void *context)
{
	(void)sdRef;
	(void)interfaceIndex;
	(void)errorCode;
	
    if (!*replyDomain) return;
    
    _CNDomainBrowser *self = (__bridge _CNDomainBrowser *)context;
    NSString *key = [NSString stringWithUTF8String: replyDomain];
    
    if (self.ignoreLocal && [key isEqualToString: @"local."])               goto exit;
    if (self.ignoreBTMM && [key hasSuffix: @".members.btmm.icloud.com."])   goto exit;
    
    if (!(flags & kDNSServiceFlagsAdd))
    {
        [self.browseDomainD removeObjectForKey:key];
    }
    else
    {
        NSArray * pathArray = DNSDomainToDomainPath(key);
        [self.browseDomainD setObject: @{ _CNSubDomainKey_reverseDomainPath: pathArray,
                                          _CNSubDomainKey_defaultFlag: (flags & kDNSServiceFlagsDefault) ? @YES : @NO }
                               forKey: key];
    }

exit:
    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        [self reloadBrowser];
    }
}

@end
