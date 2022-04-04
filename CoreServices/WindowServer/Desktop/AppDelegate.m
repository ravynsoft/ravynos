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

    desktop = [DesktopWindow new];
    [desktop setDelegate:self];
    [desktop makeKeyAndOrderFront:nil];
    menuBar = [desktop menuBar];
}

- (void)updateBackground {
    [desktop updateBackground];
}

/* Recursively set all menu targets and delegates to our proxy */
-(void)_menuEnumerateAndChange:(NSMenu *)menu {
    NSArray *items = [menu itemArray];
    [menu setDelegate:self];
    for(int i = 0; i < [items count]; ++i) {
        NSMenuItem *item = [items objectAtIndex:i];
        if([item isSeparatorItem] || [item isHidden] || ![item isEnabled])
            continue;
        [item setTarget:self];
        [item setAction:@selector(dump:)];
        if([item hasSubmenu])
            [self _menuEnumerateAndChange:[item submenu]];
    }
}

- (void)menuDidUpdate:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    pid_t pid = [[dict objectForKey:@"ProcessID"] intValue];
    NSMenu *mainMenu = [dict objectForKey:@"MainMenu"];
    [self _menuEnumerateAndChange:mainMenu];
    [menuBar setMenu:mainMenu forPID:pid];
    if(![menuBar activateMenuForPID:pid]) // FIXME: don't activ8 right away
        NSLog(@"could not activate menus!");
}

- (void)dump:(id)object {
    NSLog(@"DUMP: %@", object);
}
@end

