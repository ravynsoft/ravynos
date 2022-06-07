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

    NSMenuItem *item;
    sysMenu = [NSMenu new];
    [sysMenu setDelegate:self];
    [sysMenu setAutoenablesItems:YES];
    [[sysMenu addItemWithTitle:@"About This Computer" action:@selector(aboutThisComputer:) 
        keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"System Preferences..." action:NULL keyEquivalent:@""]
        setTarget:self];
    [[sysMenu addItemWithTitle:@"Software Store..." action:NULL keyEquivalent:@""] setEnabled:NO];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Recent Items" action:NULL keyEquivalent:@""] setTarget:self];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Force Quit..." action:NULL keyEquivalent:@""] setTarget:self];
    [sysMenu addItem:[NSMenuItem separatorItem]];
    [[sysMenu addItemWithTitle:@"Sleep" action:NULL keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Restart..." action:NULL keyEquivalent:@""] setTarget:self];
    [[sysMenu addItemWithTitle:@"Shut Down..." action:NULL keyEquivalent:@""] setTarget:self];
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

    NSRect rect = NSMakeRect(menuBarHPad, 0, 64, menuBarHeight);
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

    NSRect rect = NSMakeRect(menuBarHPad*3, 0, _frame.size.width, menuBarHeight);
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

/* NSWindow delegate */
- (BOOL)windowShouldClose:(NSWindow *)window {
    return YES;
}

- (void)windowWillClose:(NSNotification *)note {
    if([[note object] isEqual:aboutWindow])
        aboutWindow = nil;
}

@end

