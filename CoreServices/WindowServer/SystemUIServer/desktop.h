/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
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
#import <mach/mach.h>

#import "Label.h"

#define menuBarHeight 22
#define menuBarVPad 2
#define menuBarHPad 16

extern const NSString *PrefsWallpaperPathKey;
extern const NSString *PrefsDateFormatStringKey;

// the clock
@interface ClockView: Label {
    NSDateFormatter *dateFormatter;
    NSString *dateFormat;
    NSDictionary *attributes;
}

- (ClockView *)initWithFrame:(NSRect)frame;
- (NSString *)currentDateValue;
- (void)notifyTick:(id)arg;
- (NSSize)size;
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
    NSMutableArray *statusItems;
}

- (void)addStatusItem:(NSStatusItem *)item pid:(unsigned int)pid;
- (void)removeStatusItemsForPID:(unsigned int)pid;
- (void)renderItems;
@end

// the global top bar
@interface MenuBarWindow: NSWindow {
    MenuView *menuView;
    ExtrasView *extrasView;
    ClockView *clockView;
    NSMutableDictionary *menuDict;
    NSString *activeApp;
}

- (MenuBarWindow *)initWithFrame:(NSRect)frame;
- (void)setPort:(mach_port_t)port forApp:(NSString *)bundleID;
- (void)removePortForApp:(NSString *)bundleID;
- (NSMenu *)menuForApp:(NSString *)bundleID;
- (void)setMenu:(NSMenu *)menu forApp:(NSString *)bundleID;
- (void)removeMenuForApp:(NSString *)bundleID;
- (BOOL)activateApp:(NSString *)bundleID;
- (void)addRecentItem:(NSURL *)itemURL;
- (void)addStatusItem:(NSStatusItem *)item pid:(unsigned int)pid;
- (void)removeStatusItemsForPID:(unsigned int)pid;
@end

// desktop interface controller
@interface AppDelegate: NSObject {
    mach_port_name_t _servicePort;
    int _kq;
    MenuBarWindow *menuBar;
}

- (void)receiveMachMessage;
- (void)processKernelQueue;
- (void)createWindows:(NSNotification *)note;
@end

