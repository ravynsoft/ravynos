//
//  NSStatusItem (Private).h
//  AppKit
//
//  Created by Rhys Cox on 17/02/2012.
//  Copyright (c) 2012 BP Software. All rights reserved.
//

#import <Foundation/NSObject.h>
#import <CoreFoundation/CFBase.h>
#ifdef Win32Assert
#import <AppKit.h/NSWindow.h>
#import <AppKit.h/Win32Window.h>
#endif

#define NSVariableStatusItemLength (-1)
#define NSSquareStatusItemLength (-2)

#import <AppKit/NSStatusBar.h>

@interface NSStatusBar (Private)
#ifdef WIN32
- (void)_trayNotificationForID:(int)aTrayID event:(int)anEvent;
- (Win32Window *)fakeWindow;
#endif
@end
