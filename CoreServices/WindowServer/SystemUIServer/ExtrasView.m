/*
 * Copyright (C) 2022-2023 Zoe Knox <zoe@pixin.net>
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

@implementation ExtrasView
- initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    [self setNeedsDisplay:YES];
    statusItems = [NSMutableArray new];
    return self;
}

- (void)addStatusItem:(NSStatusItem *)item pid:(unsigned int)pid {
    NSImageView *icon = [[NSImageView alloc]
	initWithFrame:NSMakeRect(0, 2, menuBarHeight - 4, menuBarHeight - 4)];
    [icon setImageScaling:NSImageScaleProportionallyUpOrDown];
    [icon setImage:[item image]];
    [[icon image] setScalesWhenResized:YES];
    [icon setToolTip:item.toolTip];

    NSMutableDictionary *props = [NSMutableDictionary
	dictionaryWithObjects:@[item, [NSNumber numberWithInt:pid], icon]
		      forKeys:@[@"NSStatusItem", @"ProcessID", @"NSView"]];

    if(kill(pid, 0) != 0 && errno == ESRCH)
        return; // pid has already exited

    [statusItems addObject:props];
    [self renderItems];
}

// render the array from right to left, so element 0 is closest
// to the clock
- (void)renderItems {
    [self setSubviews:nil];
    float height = menuBarHeight - 4;
    NSPoint itemPos = NSMakePoint(_frame.size.width - menuBarHeight - 12, 2);

    for(int i = 0; i < [statusItems count]; ++i) {
	NSMutableDictionary *props = [statusItems objectAtIndex:i];
	NSView *icon = [props objectForKey:@"NSView"];
        NSStatusItem *item = [props objectForKey:@"NSStatusItem"];
	[icon setFrameOrigin:itemPos];
	[self addSubview:icon];
	itemPos.x -= (menuBarHeight + 12);
    }
    [self setNeedsDisplay:YES];
}

- (void)removeStatusItemsForPID:(unsigned int)pid {
    for(int i = 0; i < [statusItems count]; ++i) {
	NSMutableDictionary *d = [statusItems objectAtIndex:i];
	if([[d objectForKey:@"ProcessID"] intValue] == pid) {
	    [statusItems removeObjectAtIndex:i];
	}
    }
    [self renderItems];
}

@end

