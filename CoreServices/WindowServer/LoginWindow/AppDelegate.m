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
- (AppDelegate *)init {
    desktops = [NSMutableDictionary new];
    return self;
}

- (void)screenDidResize:(NSNotification *)note {
    NSMutableDictionary *dict = (NSMutableDictionary *)[note userInfo];
    NSNumber *key = [dict objectForKey:@"WLOutputXDGOutput"];

    if(key == nil) {
        NSLog(@"ERROR: screenDidResize for null output key");
        return;
    }

    NSRect frame = NSZeroRect;
    frame.size = NSSizeFromString([dict objectForKey:@"WLOutputSize"]);
    frame.origin = NSPointFromString([dict objectForKey:@"WLOutputPosition"]);

    DesktopWindow *desktop = [[DesktopWindow alloc] initWithFrame:frame forOutput:key];
    [desktops setObject:desktop forKey:key];
    [desktop setDelegate:self];
    [desktop makeKeyAndOrderFront:nil];
}

- (void)updateBackground {
    NSArray *values = [desktops allValues];
    for(int i = 0; i < [values count]; ++i)
        [[values objectAtIndex:i] updateBackground];
}

@end

