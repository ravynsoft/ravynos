//
//  NSStatusItem+Private.h
//  AppKit
//
//  Created by Rhys Cox on 17/02/2012.
//  Copyright (c) 2012 BP Software. All rights reserved.
//  ravynOS integration Copyright (C) 2023 Zoe Knox
//
#import <Foundation/Foundation.h>
#import <CoreFoundation/CFBase.h>
#ifdef WIN32
#import <windows.h>
#endif
@class NSImage, NSStatusItem, NSMenuItem;
@interface NSStatusItem (Private)
#ifdef WIN32
- (void)_createTrayIcon;
- (void)_setHICONFromImage:(NSImage *)image;
- (void)_removeTrayIcon;
- (int)trayIconID;
- (void)_processWin32Event:(int)event;
- (void)_w32loadMenuItem:(NSMenuItem *)item withIdentifier:(int)i intoMenu:(HMENU)menu;
HWND _eventhWnd;
NOTIFYICONDATA niData;
#endif
- (void)_showContextMenu;
@end
