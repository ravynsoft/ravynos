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
#import "desktop.h"

@implementation AppDelegate
- (void)screenDidResize:(NSNotification *)note {
    NSRect frame = [[NSScreen mainScreen] visibleFrame];

    background = [DesktopWindow new];
    [background setDelegate:self];
//    [background makeKeyAndOrderFront:nil];

    menuBar = [MenuBarWindow new];
    [menuBar setDelegate:self];
    [menuBar makeKeyAndOrderFront:nil];
}

- (void)updateBackground {
    [background updateBackground];
}

- (void)menuDidUpdate:(NSNotification *)note {
    NSMenu *menu = (NSMenu *)[note userInfo];
    [menuBar setMenu:menu forPID:0]; // FIXME: add pid and menu to dict
    if(![menuBar activateMenuForPID:0]) // FIXME: don't activ8 right away
        NSLog(@"could not activate menus!");
}

@end

