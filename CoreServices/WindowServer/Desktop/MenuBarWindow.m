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
- init {
    NSRect frame = [[NSScreen mainScreen] visibleFrame];

    self = [super initWithFrame:NSMakeRect(0, frame.size.height - menuBarHeight, 
        frame.size.width, menuBarHeight)];

    clockView = [ClockView new];
    menuView = [MenuView new];
    extrasView = [ExtrasView new];
    menuDict = [NSMutableDictionary new];
    _menuPort = MACH_PORT_NULL;

    [self addSubview:menuView];
    [self addSubview:extrasView];
    [self addSubview:clockView];
    [menuView setWindow:[self window]];

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
    NSMenu *menu = [menuDict objectForKey:[NSNumber numberWithInt:pid]];
    mach_port_t port = [self portForMenu:menu];
    if(menu /*&& port != MACH_PORT_NULL*/) {
        [menuView setMenu:menu];
        _menuPort = port;
        return YES;
    }
    return NO;
}

@end

