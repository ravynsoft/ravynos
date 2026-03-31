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

#import <Cocoa/Cocoa.h>

@protocol CNDomainBrowserViewDelegate;

IB_DESIGNABLE

@interface CNDomainBrowserView : NSView

@property (nonatomic) IBInspectable BOOL                                        browseRegistration;
@property (nonatomic) IBInspectable BOOL                                        ignoreLocal;
@property (nonatomic) IBInspectable BOOL                                        ignoreBTMM;
@property (weak)      IBOutlet		id<CNDomainBrowserViewDelegate>             delegate;

@property (readonly)                NSString *                                  selectedDNSDomain;
@property (readonly)                NSString *                                  defaultDNSDomain;
@property (readonly)                NSArray *                                   flattenedDNSDomains;

@property (readonly)                BOOL                                        isBrowsing;

- (void)startBrowse;
- (void)stopBrowse;

- (CGFloat)minimumHeight;
- (void)showSelectedRow;
- (BOOL)foundInstanceInMoreThanLocalDomain;

@end

@protocol CNDomainBrowserViewDelegate <NSObject>

@optional

- (void)domainBrowserDomainSelected:(NSString *)domain;
- (void)domainBrowserDomainUpdate:(NSString *)defaultDomain;

@end

@interface CNBonjourDomainCell : NSCell
@end

@interface CNBonjourDomainView : NSView

@property(strong, nonatomic)   NSString *	domain;

@end
