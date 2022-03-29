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

@implementation MenuView
- init {
    NSRect frame = [[NSScreen mainScreen] visibleFrame];

    self = [super initWithFrame:NSMakeRect(0, 0, frame.size.width/2, menuBarHeight)];
    NSString *ravyn = [[NSBundle mainBundle] pathForResource:@"ravynos-mark-64" ofType:@"png"];
    NSImage *logo = [[NSImage alloc] initWithContentsOfFile:ravyn];
    logoView = [[NSImageView alloc] initWithFrame:NSMakeRect(menuBarHPad,menuBarVPad,16,16)];
    [logoView setImage:logo];
    [self addSubview:logoView];
    [self setNeedsDisplay:YES];
    return self;
}

- (void)setMenu:(NSMenu *)menu {
    NSFontManager *fontmgr = [NSFontManager sharedFontManager];
    NSDictionary *attributesBold = [NSDictionary dictionaryWithObject:[fontmgr convertFont:
        [NSFont systemFontOfSize:14] toHaveTrait:NSBoldFontMask] forKey:NSFontAttributeName];
    NSDictionary *attributes = [NSDictionary dictionaryWithObject:[NSFont
        systemFontOfSize:14] forKey:NSFontAttributeName];

    NSArray *items = [menu itemArray];
    int x = menuBarHPad*3;

    for(int i = 0; i < [items count]; ++i) {
        NSMenuItem *item = [items objectAtIndex:i];
        if([item title] == nil || [item isHidden])
            continue;

        NSAttributedString *title = [[NSAttributedString alloc] 
            initWithString:[item title] attributes:(i == 0) ?
            attributesBold : attributes];
        NSSize size = [title size];

        NSPopUpButton *b = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(x,
            menuBarVPad, size.width + menuBarHPad, menuBarHeight - menuBarVPad)
            pullsDown:YES];
        [b setMenu:[item submenu]];
        [b addItemWithTitle:[item title]];
        [b setAttributedTitle:title];
        [b setBordered:NO];
        [b setBezeled:NO];
        [[b cell] setArrowPosition:NSPopUpNoArrow];
        [self addSubview:b];
        x += size.width + menuBarHPad;
    }
    [self setNeedsDisplay:YES];
}

@end

