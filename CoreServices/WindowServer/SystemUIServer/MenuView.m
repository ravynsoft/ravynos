/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
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

#import <AppKit/AppKit.h>
#import <Foundation/NSPlatform.h>
#import "desktop.h"
#import "../common.h"
#import "AboutWindow.h"

extern const char **environ;
extern int exitCode;

@implementation MenuView
- initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];

    aboutWindow = nil;
    _maxRecentItems = 12; // FIXME: read from preferences

    sysMenu = [NSMenu new];
    [sysMenu setDelegate:self];
    [sysMenu setAutoenablesItems:YES];

    recentItemsMenu = [NSMenu new];
    [recentItemsMenu setDelegate:self];
    [recentItemsMenu setAutoenablesItems:YES];

    [[sysMenu addItemWithTitle:@"About This Computer" action:@selector(aboutThisComputer:) 
        keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"System Preferences..." 
        action:@selector(launchSystemPreferences:) keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Software Store..." action:NULL keyEquivalent:@""] setEnabled:NO];
    [sysMenu addItem:[NSMenuItem separatorItem]];

    NSMenuItem *item = [sysMenu addItemWithTitle:@"Recent Items" action:NULL
        keyEquivalent:@""];
    [item setTarget:self];
    [item setSubmenu:recentItemsMenu];

    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Force Quit..." action:@selector(forceQuit:)
        keyEquivalent:@""] setTarget:self];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Sleep" action:@selector(performSleep:) keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Restart..." action:@selector(performRestart:) keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Shut Down..." action:@selector(performShutDown:) keyEquivalent:@""] setTarget:self];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Lock Screen" action:NULL keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Log Out" action:@selector(performLogout:) keyEquivalent:@""] setTarget:self];

    NSString *ravyn = [[NSBundle mainBundle] pathForResource:@"ravynos-mark-64" ofType:@"png"];
    NSImage *logo = [[NSImage alloc] initWithContentsOfFile:ravyn];
    [logo setScalesWhenResized:YES];
    [logo setSize:NSMakeSize(16,16)];
    NSMenu *logoMenu = [NSMenu new];
    NSMenuItem *logoItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [logoItem setImage:logo];
    [logoItem setSubmenu:sysMenu];
    [logoMenu addItem:logoItem];

    NSRect rect = NSMakeRect(menuBarHPad, 0, _frame.size.width, menuBarHeight);
    appMenuView = [[NSMainMenuView alloc] initWithFrame:rect menu:logoMenu];
    [appMenuView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [self addSubview:appMenuView];

    [self setNeedsDisplay:YES];
    return self;
}

- (BOOL)isFlipped {
    return YES;
}

- (void)setMenu:(NSMenu *)menu {
    // Clear any old app menus, but leave our system menu icon
    NSMenu *fullMenu = [appMenuView menu];
    while([fullMenu numberOfItems] > 1)
        [fullMenu removeItemAtIndex:1];

    // Now add the new app's menus if there are any
    if(menu != nil) {
        NSMenuItem *item = [menu itemAtIndex:0];
        if([item hasSubmenu] && [[[item submenu] _name] isEqualToString:@"NSAppleMenu"]) {
            NSFontManager *fontmgr = [NSFontManager sharedFontManager];
            NSDictionary *attr = [NSDictionary dictionaryWithObject:[fontmgr convertFont:
                [NSFont menuFontOfSize:15] toHaveTrait:NSBoldFontMask] forKey:NSFontAttributeName];
            [item setAttributedTitle:[[NSAttributedString alloc] initWithString:[item title]
                attributes:attr]];
        }
        for(int i = 0; i < [menu numberOfItems]; ++i)
            [fullMenu addItem:[menu itemAtIndex:i]];
    }
    [self setNeedsDisplay:YES];
}

- (void)aboutThisComputer:(id)sender {
    if(aboutWindow) {
        [aboutWindow orderFront:nil];
        return;
    }

    aboutWindow = [AboutWindow new];
    [aboutWindow setDelegate:self];
    [aboutWindow makeKeyAndOrderFront:nil];
}

- (void)performSleep:(id)sender {
    NSLog(@"not implemented yet");
}

- (void)performRestart:(id)sender {
    int rc = NSRunAlertPanel(@"Confirm Restart", 
        @"Are you sure you want to restart your computer?",
        @"Restart", @"Cancel", nil);
    if(rc == 1) {
        exitCode = EXIT_RESTART;
        [NSApp stop:self];
    }
}

- (void)performShutDown:(id)sender {
    int rc = NSRunAlertPanel(@"Confirm Shut Down", 
        @"Are you sure you want to shut down your computer?",
        @"Shut Down", @"Cancel", nil);
    if(rc == 1) {
        exitCode = EXIT_SHUTDOWN;
        [NSApp stop:self];
    }
}

- (void)performLogout:(id)sender {
    int rc = NSRunAlertPanel(@"Confirm Log Out",
            @"Are you sure you want to quit all applications and log out now?",
            @"Log Out", @"Cancel", nil);
    if(rc == 1) {
        exitCode = EXIT_LOGOUT;
        [NSApp stop:self];
    }
}

- (void)launchSystemPreferences:(id)sender {
    NSLog(@"not implemented yet");
    NSURL *sysprefs = [NSURL fileURLWithPath:
        @"/System/Library/CoreServices/System Preferences.app"];
    OSStatus res = LSOpenCFURLRef((__bridge CFURLRef)sysprefs, NULL);
    if(res != 0) {
        NSLog(@"LSOpenCFURLRef: error %d", res);
        // FIXME: error handling
    }
}

- (void)addRecentItem:(NSURL *)itemURL {
    NSMenuItem *item;

    // if this item is already on the menu, move it to the top
    int index = [recentItemsMenu indexOfItemWithRepresentedObject:itemURL];
    if(index >= 0) {
        item = [recentItemsMenu itemAtIndex:index];
        [recentItemsMenu removeItem:item];
        [recentItemsMenu insertItem:item atIndex:0];
        return;
    }

    // otherwise add a new entry for this item and shift others down
    item = [[NSMenuItem alloc] initWithTitle:[itemURL lastPathComponent]
        action:@selector(launchRecentItem:) keyEquivalent:@""];
    [item setRepresentedObject:itemURL];
    [item setTarget:self];
    [recentItemsMenu insertItem:item atIndex:0];

    // keep the list at our desired size by dropping off oldest entry
    int count = [[recentItemsMenu itemArray] count];
    if(count > _maxRecentItems)
        [recentItemsMenu removeItemAtIndex:count];
}

- (void)launchRecentItem:(id)sender {
    NSMenuItem *item = (NSMenuItem *)sender;
    NSURL *itemURL = [item representedObject];
    NSLog(@"opening recent item %@ url %@", item, itemURL);
    LSOpenCFURLRef((__bridge CFURLRef)itemURL, NULL);
}

- (void)forceQuit:(id)sender {
    NSLog(@"force quit (not implemented yet)");
}

/* NSWindow delegate */
- (BOOL)windowShouldClose:(NSWindow *)window {
    return YES;
}

- (void)windowWillClose:(NSNotification *)note {
    if([[note object] isEqual:aboutWindow])
        aboutWindow = nil;
}

@end

