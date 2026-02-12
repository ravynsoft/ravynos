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

#import <Foundation/Foundation.h>
#import <dispatch/queue.h>

extern const NSString *    _CNSubDomainKey_defaultFlag;
extern const NSString *    _CNSubDomainKey_subPath;

@protocol _CNDomainBrowserDelegate;

@interface _CNDomainBrowser : NSObject

@property (nonatomic) BOOL              browseRegistration;
@property (nonatomic) BOOL              ignoreLocal;
@property (nonatomic) BOOL              ignoreBTMM;
@property (strong)    dispatch_queue_t  callbackQueue;
@property (readonly)  BOOL              isBrowsing;

- (instancetype)initWithDelegate:(id<_CNDomainBrowserDelegate>)delegate;
- (void)startBrowser;
- (void)stopBrowser;

- (BOOL)foundInstanceInMoreThanLocalDomain;

@property (readonly) NSArray *          defaultDomainPath;
@property (readonly) NSArray *          flattenedDNSDomains;

- (NSArray *)subDomainsAtDomainPath:(NSArray *)domainPath;

@end

@protocol _CNDomainBrowserDelegate <NSObject>

- (void)bonjourBrowserDomainUpdate:(NSArray *)defaultDomainPath;

@end
