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

#pragma once
#import <AppKit/NSMainMenuView.h>
#import <AppKit/NSMenu.h>
#import <AppKit/WLWindow.h>
#import <mach/mach.h>

#import "Label.h"

#define menuBarHeight 22
#define menuBarVPad 2
#define menuBarHPad 16
#define SERVICE_NAME "com.ravynos.WindowServer"

extern const NSString *PrefsWallpaperPathKey;
extern const NSString *PrefsDateFormatStringKey;
extern const NSString *WLOutputDidResizeNotification;
extern const NSString *WLMenuDidUpdateNotification;

@interface NSWindow(WLWindow_private)
- (WLWindow *)platformWindow;
- (void)setExclusiveZone:(uint32_t)pixels;
@end


// the clock
@interface ClockView: Label {
    NSDateFormatter *dateFormatter;
    NSString *dateFormat;
    NSDictionary *attributes;
}

- (ClockView *)initWithFrame:(NSRect)frame;
- (void)update:(NSWindow *)window;
@end

// system and application menu titles view
@interface MenuView: NSView {
    NSMainMenuView *logoMenuView;
    NSMenu *sysMenu;
    NSMenu *recentItemsMenu;
    NSMainMenuView *appMenuView;
    NSWindow *aboutWindow;
    int _maxRecentItems;
}

- (MenuView *)initWithFrame:(NSRect)frame;
- (void)setWindow:(NSWindow *)window;
- (void)setMenu:(NSMenu *)menu;
- (void)aboutThisComputer:(id)sender;
- (void)performSleep:(id)sender;
- (void)performRestart:(id)sender;
- (void)performShutDown:(id)sender;
- (void)launchSystemPreferences:(id)sender;
- (void)addRecentItem:(NSURL *)itemURL;
- (void)launchRecentItem:(id)sender;
@end

// menu extras container
@interface ExtrasView: NSView {
}
@end

// the global top bar
@interface MenuBarWindow: NSWindow {
    MenuView *menuView;
    ExtrasView *extrasView;
    ClockView *clockView;
    NSMutableDictionary *menuDict;
    mach_port_t _menuPort;
}

- (MenuBarWindow *)initWithFrame:(NSRect)frame forOutput:(NSNumber *)outputKey;
- (void)setPort:(mach_port_t)port forMenu:(NSMenu *)menu;
- (void)removePortForMenu:(NSMenu *)menu;
- (mach_port_t)portForMenu:(NSMenu *)menu;
- (NSMenu *)menuForPID:(unsigned int)pid;
- (void)setMenu:(NSMenu *)menu forPID:(unsigned int)pid;
- (void)removeMenuForPID:(unsigned int)pid;
- (BOOL)activateMenuForPID:(unsigned int)pid;
- (void)addRecentItem:(NSURL *)itemURL;
@end

// desktop interface controller
@interface AppDelegate: NSObject {
    mach_port_name_t _servicePort;
    MenuBarWindow *menuBar;
}

- (void)receiveMachMessage;
- (void)screenDidResize:(NSNotification *)note;
@end

