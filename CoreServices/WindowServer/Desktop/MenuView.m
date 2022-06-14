/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
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
#import "AboutWindow.h"

@interface NSMenu(private)
-(NSString *)_name;
@end

@interface NSMainMenuView(private)
-(void)setWindow:(NSWindow *)window;
@end

@implementation MenuView
- init {
    NSRect frame = [[NSScreen mainScreen] visibleFrame];

    self = [super initWithFrame:NSMakeRect(0, 0, frame.size.width/2, menuBarHeight)];
    aboutWindow = nil;
    _maxRecentItems = 12; // FIXME: read from preferences

    sysMenu = [NSMenu new];
    [sysMenu setDelegate:self];
    [sysMenu setAutoenablesItems:YES];

    recentItemsMenu = [NSMenu new];
    [recentItemsMenu setDelegate:self];
    [recentItemsMenu setAutoenablesItems:YES];
    NSMenuItem *test = [recentItemsMenu addItemWithTitle:@"Test Item" 
        action:@selector(launchRecentItem:)
        keyEquivalent:@""];
    [test setTarget:self];

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
    [[sysMenu addItemWithTitle:@"Log Out" action:NULL keyEquivalent:@""] setTarget:self];

    NSString *ravyn = [[NSBundle mainBundle] pathForResource:@"ravynos-mark-64" ofType:@"png"];
    NSImage *logo = [[NSImage alloc] initWithContentsOfFile:ravyn];
    [logo setScalesWhenResized:YES];
    [logo setSize:NSMakeSize(16,16)];
    NSMenu *logoMenu = [NSMenu new];
    NSMenuItem *logoItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [logoItem setImage:logo];
    [logoItem setSubmenu:sysMenu];
    [logoMenu addItem:logoItem];

    NSRect rect = NSMakeRect(menuBarHPad, 0, 32, menuBarHeight);
    NSMainMenuView *mv = [[NSMainMenuView alloc] initWithFrame:rect menu:logoMenu];
    [self addSubview:mv];

    [self setNeedsDisplay:YES];
    return self;
}

- (BOOL)isFlipped {
    return YES;
}

- (void)setMenu:(NSMenu *)menu {
    NSMenuItem *item = [menu itemAtIndex:0];
    if([item hasSubmenu] && [[[item submenu] _name] isEqualToString:@"NSAppleMenu"]) {
        NSFontManager *fontmgr = [NSFontManager sharedFontManager];
        NSDictionary *attr = [NSDictionary dictionaryWithObject:[fontmgr convertFont:
            [NSFont menuFontOfSize:15] toHaveTrait:NSBoldFontMask] forKey:NSFontAttributeName];
        [item setAttributedTitle:[[NSAttributedString alloc] initWithString:[item title]
            attributes:attr]];
    }

    NSRect rect = NSMakeRect(menuBarHPad*4, 0, _frame.size.width, menuBarHeight);
    appMenuView = [[NSMainMenuView alloc] initWithFrame:rect menu:menu];
    [appMenuView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
    [self addSubview:appMenuView];
    [appMenuView setWindow:[self window]];

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

static void _performShutDown(int mode) {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"shutdown" ofType:@""];
    pid_t helper;
    char *argv[3], *envp[1];
    char *modestr;
    asprintf(&modestr, "%d", mode);
    argv[0] = [[path lastPathComponent] UTF8String];
    argv[1] = modestr;
    argv[2] = NULL;
    envp[0] = NULL;
    if(!path || posix_spawn(&helper, [path UTF8String], NULL, NULL, argv, envp) != 0)
        NSLog(@"performShutDown: error occurred"); // FIXME: error handling
    free(modestr);
}

- (void)performSleep:(id)sender {
    _performShutDown(0);
}

- (void)performRestart:(id)sender {
    int rc = NSRunAlertPanel(@"Confirm Restart", 
        @"Are you sure you want to restart your computer?",
        @"Restart", @"Cancel", nil);
    if(rc == 1) {
        _performShutDown(1);
    }
}

- (void)performShutDown:(id)sender {
    int rc = NSRunAlertPanel(@"Confirm Shut Down", 
        @"Are you sure you want to shut down your computer?",
        @"Shut Down", @"Cancel", nil);
    if(rc == 1) {
        _performShutDown(2);
    }
}

- (void)launchSystemPreferences:(id)sender {
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
    NSLog(@"force quit");
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

