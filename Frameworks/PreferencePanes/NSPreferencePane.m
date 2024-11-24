/*
 * ravynOS Preference Panes
 *
 * Copyright (C) 2024 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import "NSPreferencePane.h"
#import <AppKit/NSNib.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSHelpManager.h>

NSString * const NSPreferencePrefPaneIsAvailableNotification = @"NSPreferencePrefPaneIsAvailableNotification";
NSString * const NSPreferencePaneDoUnselectNotification = @"NSPreferencePaneDoUnselectNotification";
NSString * const NSPreferencePaneCancelUnselectNotification = @"NSPreferencePaneCancelUnselectNotification";
NSString * const NSPreferencePaneSwitchToPaneNotification = @"NSPreferencePaneSwitchToPaneNotification";
NSString * const NSPreferencePaneUpdateHelpMenuNotification = @"NSPreferencePaneUpdateHelpMenuNotification";

NSString * const NSPrefPaneHelpMenuInfoPListKey = @"NSPrefPaneHelpAnchors";
NSString * const NSPrefPaneHelpMenuTitleKey = @"title";
NSString * const NSPrefPaneHelpMenuAnchorKey = @"anchor";

#define kNSPrefPaneHelpMenuInfoPListKey NSPrefPaneHelpMenuInfoPListKey
#define kNSPrefPaneHelpMenuTitleKey NSPrefPaneHelpMenuTitleKey
#define kNSPrefPaneHelpMenuAnchorKey NSPrefPaneHelpMenuAnchorKey

@implementation NSPreferencePane

-(id)initWithBundle:(NSBundle *)bundle {
    self = [super init];
    _bundle = bundle;
    _mainView = nil;
    _mainNibName = [bundle objectForInfoDictionaryKey:@"NSMainNibFile"];
    if(_mainNibName == nil)
        _mainNibName = @"Main";
    _selected = NO;
    _shouldUnselect = NSUnselectNow;
    _autoSaveTextFields = YES;
    _helpItemsDict = nil;
    _help = nil;
    return self;
}

-(NSView *)loadMainView {
    // FIXME: this loadNibNamed variant is deprecated from 10.8+
    // should use -loadNibNamed:owner:topLevelObjects instead
    NSDictionary *nameTable = [NSDictionary dictionaryWithObject:self
                                                          forKey:NSNibOwner];
    if([_bundle loadNibFile:_mainNibName
          externalNameTable:nameTable
                   withZone:NSDefaultMallocZone()]) {
        [self assignMainView];
        [self mainViewDidLoad];
    }
    return _mainView;
}

-(void)assignMainView {
    _mainView = [_window contentView];
    _window = nil;
}

-(void)mainViewDidLoad {
}

-(void)willSelect {
}

-(void)didSelect {
}

-(void)willUnselect {
}

-(void)didUnselect {
}

-(void)replyToShouldUnselect:(BOOL)shouldUnselect {
    if(shouldUnselect)
        _shouldUnselect = NSUnselectNow;
}


-(void)updateHelpMenuWithArray:(NSArray *)inArrayOfMenuItems {
    if(_help == nil) // _help is set by Sys Prefs on load
        return;

    [_help removeAllItems];
    _helpItemsDict = nil;
    _helpItemsDict = [NSMutableDictionary new];

    for(int i = 0; i < [inArrayOfMenuItems count]; ++i) {
        NSDictionary *dict = [inArrayOfMenuItems objectAtIndex:i];
        NSString *title = [dict objectForKey:kNSPrefPaneHelpMenuTitleKey];
        NSString *anchor = [dict objectForKey:kNSPrefPaneHelpMenuAnchorKey];
        [_helpItemsDict setObject:anchor forKey:title];
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:title
                                                      action:@selector(_helpMenuAction:)
                                               keyEquivalent:@""];
        [item setTarget:self];
        [_help addItem:item];
    }
}

// Private stuff

-(void)_helpMenuAction:(NSMenuItem *)item {
    NSString *anchor = [_helpItemsDict objectForKey:[item title]];
    NSString *book = [_bundle objectForInfoDictionaryKey:@"CFBundleHelpBookName"];
    if(anchor && book)
        [[NSHelpManager sharedHelpManager] openHelpAnchor:anchor inBook:book];
}

-(void)setHelpMenu:(NSMenu *)help {
    _help = help;
}

@end

