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

#import "SafariExtensionViewController.h"
#import "CNDomainBrowserPathUtils.h"

@interface SafariExtensionViewController ()

@end

@implementation SafariExtensionViewController

+ (SafariExtensionViewController *)sharedController {
    static SafariExtensionViewController *sharedController = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedController = [[SafariExtensionViewController alloc] init];
    });
    return sharedController;
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    [_domainBrowserView startBrowse];
    [_mainSplitView adjustSubviews];
}

- (void) viewWillDisappear
{
    [super viewWillDisappear];
    [_domainBrowserView stopBrowse];
}

#pragma mark - BServiceBrowser Delegates

- (void)bonjourServiceSelected:(NSString *)service type:(NSString *)type atDomain:(NSString *)domain
{
    (void)service;  // unused
    (void)type;     // unused
    (void)domain;   // unused
}

- (void)doubleAction:(NSURL *)url
{
    if (url)
    {
        [SFSafariApplication getActiveWindowWithCompletionHandler:^(SFSafariWindow * _Nullable activeWindow) {
            [activeWindow openTabWithURL: url makeActiveIfPossible: YES completionHandler:^(SFSafariTab * _Nullable tab) {
                (void)tab;         // Unused
            }];
        }];
        [self dismissPopover];
    }
}

#pragma mark - BonjourBrowser Delegates

- (void)domainBrowserDomainUpdate:(NSString *)defaultDomain
{
    [_serviceBrowserView newServiceBrowse: DNSDomainToDomainPath(defaultDomain)];
    if( !_domainBrowserView.selectedDNSDomain.length )
    {
        [_mainSplitView setPosition: [_mainSplitView maxPossiblePositionOfDividerAtIndex: 0] ofDividerAtIndex: 0];
    }
    else
    {
        [_mainSplitView adjustSubviews];
        [_domainBrowserView showSelectedRow];
    }
}

- (void)domainBrowserDomainSelected:(NSString *)domain
{
    [_serviceBrowserView newServiceBrowse: DNSDomainToDomainPath(domain)];
}

#pragma mark - SplitView Delegate

- (CGFloat)splitView:(NSSplitView *)splitView constrainSplitPosition:(CGFloat)proposedPosition ofSubviewAt:(NSInteger)dividerIndex
{
    (void)splitView; // Unused
#define TOP_MIN        40
    CGFloat    pos;
    CGFloat bottomMinHeight = [_domainBrowserView minimumHeight];
    
    if( proposedPosition < TOP_MIN )
    {
        pos = TOP_MIN;
    }
    else if( proposedPosition < [_mainSplitView maxPossiblePositionOfDividerAtIndex: dividerIndex] - bottomMinHeight )
    {
        pos = proposedPosition;
    }
    else
    {
        pos = [_mainSplitView maxPossiblePositionOfDividerAtIndex: dividerIndex] - bottomMinHeight;
    }
    
    //    Make sure selected rows stay in view
    [_domainBrowserView showSelectedRow];
    
    return( pos );
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex
{
    (void)splitView;       // Unused
    (void)dividerIndex;    // Unused
    return( ![_domainBrowserView foundInstanceInMoreThanLocalDomain] );
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view
{
    (void)splitView;    // Unused
    (void)view;         // Unused
    return YES;         // Having this override seems to make some non-wanted resizes to not occur
}

@end
