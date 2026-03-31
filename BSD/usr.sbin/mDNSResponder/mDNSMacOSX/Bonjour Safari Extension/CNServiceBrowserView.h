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

#import <Cocoa/Cocoa.h>

@protocol CNServiceBrowserViewDelegate;

IB_DESIGNABLE

@interface CNServiceBrowserView : NSView

@property (strong) IBInspectable	NSArray *							serviceTypes;
@property (strong) IBInspectable	NSDictionary *						localizedServiceTypesDictionary;
@property (weak)   IBOutlet			id<CNServiceBrowserViewDelegate>	delegate;

- (void)newServiceBrowse:(NSArray *)domainPath;

@end

@protocol CNServiceBrowserViewDelegate <NSObject>

@optional

- (void)bonjourServiceSelected:(NSString *)service type:(NSString *)type atDomain:(NSString *)domain;
- (void)doubleAction:(NSURL *)url;

@end

