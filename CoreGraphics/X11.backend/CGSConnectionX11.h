/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CGSCONNECTIONX11_H
#define CGSCONNECTIONX11_H
#import <CoreGraphics/CGSConnection.h>
#import <X11/Xlib.h>
#import <CoreFoundation/CFRunLoop.h>
#import <CoreFoundation/CFSocket.h>

@interface CGSConnectionX11 : CGSConnection {
	Display *_display;
	// We use CFRunLoop directly, without going through any Foundation wrapper,
    // because Apple's Cocoa has none. Unlike Apple's Cocoa, we need to watch
    // over a Unix domain socket, not a Mach port.
    CFSocketRef _cfSocket;
    CFRunLoopSourceRef _source;

	// NOTE: A CGSScreen refers to a 'crtc' in X11 paralance, and *not* to an X11 screen.
	// In other words, it refers to a physical monitor.
	NSArray<CGSScreen*>* _screens;

	CGSKeyboardLayout* _keyboardLayout;
	int _keyboardLayoutGroup;

	int _xkbEventBase;
	int _xrrEventBase;
}

-(instancetype) initWithConnectionID:(CGSConnectionID)connId;
-(void) dealloc;
-(CGSWindow*) newWindow:(CGSRegionRef)region;

-(void) processPendingEvents;

+(BOOL) isAvailable;

@end

#endif

