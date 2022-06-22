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

@implementation MenuBarWindow
- initWithFrame:(NSRect)frame forOutput:(NSNumber *)outputKey {
    NSArray *screens = [NSScreen screens];
    NSScreen *output = nil;
    BOOL priDisplay = NO;

    for(int i = 0; i < [screens count]; ++i) {
        NSScreen *s = [screens objectAtIndex:i];
        if([s key] == outputKey) {
            output = s;
            if(i == 0)
                priDisplay = YES;
            break;
        }
    }

    if(!priDisplay)
        return nil;

    NSRect visibleFrame = [output visibleFrame];
    visibleFrame.origin = NSZeroPoint;

    frame = NSMakeRect(0, NSMaxY(visibleFrame) - menuBarHeight, NSMaxX(visibleFrame), menuBarHeight);
    self = [super initWithContentRect:frame
        styleMask:NSBorderlessWindowMask|WLWindowLayerAnchorTop
            |WLWindowLayerAnchorLeft|WLWindowLayerAnchorRight
        backing:NSBackingStoreBuffered defer:NO screen:output];

    [self setMovableByWindowBackground:NO];

    clockView = [[ClockView alloc] initWithFrame:frame];
    menuView = [[MenuView alloc] initWithFrame:frame];
    extrasView = [ExtrasView new];
    menuDict = [NSMutableDictionary new];
    _menuPort = MACH_PORT_NULL;

    [_contentView addSubview:menuView];
    [_contentView addSubview:extrasView];
    [_contentView addSubview:clockView];
    [menuView setWindow:self];

    [_contentView setAutoresizingMask:0];
    
    return self;
}

- (void)setPort:(mach_port_t)port forMenu:(NSMenu *)menu {
    [menuDict setObject:[NSNumber numberWithInt:port] forKey:menu];
}

- (void)removePortForMenu:(NSMenu *)menu {
    mach_port_t port = [self portForMenu:menu];
    if(port != MACH_PORT_NULL)
        mach_port_deallocate(mach_task_self(), port);
    [menuDict removeObjectForKey:menu];
}

- (mach_port_t)portForMenu:(NSMenu *)menu {
    NSNumber *numPort = [menuDict objectForKey:menu];
    if(!numPort)
        return MACH_PORT_NULL;
    return [numPort intValue];
}

- (NSMenu *)menuForPID:(unsigned int)pid {
    return [menuDict objectForKey:[NSNumber numberWithInt:pid]];
}

- (void)setMenu:(NSMenu *)menu forPID:(unsigned int)pid {
    [menuDict setObject:menu forKey:[NSNumber numberWithInt:pid]];
}

- (void)removeMenuForPID:(unsigned int)pid {
    NSNumber *key = [NSNumber numberWithInt:pid];
    NSMenu *menu = [menuDict objectForKey:key];
    if(menu) {
        [self removePortForMenu:menu];
        [menuDict removeObjectForKey:key];
    }
}

- (BOOL)activateMenuForPID:(unsigned int)pid {
    [[self platformWindow] setExclusiveZone:menuBarHeight];
    NSMenu *menu = [menuDict objectForKey:[NSNumber numberWithInt:pid]];
    mach_port_t port = [self portForMenu:menu];
    if(menu /*&& port != MACH_PORT_NULL*/) {
        [menuView setMenu:menu];
        _menuPort = port;
        return YES;
    }
    return NO;
}

- (void)addRecentItem:(NSURL *)itemURL {
    [menuView addRecentItem:itemURL];
}

@end

