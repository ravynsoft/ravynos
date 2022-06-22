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
#import <mach/mach.h>
#import "Label.h"

extern const NSString *WLOutputDidResizeNotification;
extern int fd;

@class WLWindow;
@interface NSWindow(WLWindow_private)
- (WLWindow *)platformWindow;
- (void)setKeyboardInteractivity:(uint32_t)keyboardStyle;
@end

@interface DesktopWindow: NSWindow {
    NSImageView *view;
    BOOL _priDisplay; // primary display has the login box
}

- (DesktopWindow *)initWithFrame:(NSRect)frame forOutput:(NSNumber *)outputKey;
- (id)platformWindow;
- (void)updateBackground;
- (BOOL)isPrimaryDisplay;
- (void)screenDidResize:(NSNotification *)note;
- (void)updateBackground;
@end

@interface LoginBox: NSView {
    NSTextField *userField;
    NSSecureTextField *passField;
}

- initWithDesktopWindow:(DesktopWindow *)window;
- (void)checkFields:(id)sender;
@end


@interface AppDelegate: NSObject {
    NSMutableDictionary *desktops;
}

- (void)screenDidResize:(NSNotification *)note;
- (void)updateBackground;
@end
