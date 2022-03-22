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

#define menuBarHeight 22
#define menuBarVPad 2
#define menuBarHPad 16

// the clock
@interface ClockView: NSTextView {
    NSDateFormatter *dateFormatter;
    NSString *dateFormat;
    NSTimer *updateTimer;
    NSDictionary *attributes;
}

- (ClockView *)init;
- (void)update:(NSTimer *)timer;
@end

// system and application menu titles view
@interface MenuView: NSView {
    NSImageView *logoView;
    NSView *appMenuView;
}

- (MenuView *)init;
@end

// menu extras container
@interface ExtrasView: NSView {
}
@end

// desktop wallpaper and context menu
@interface DesktopWindow: NSWindow {
    NSImageView *view;
}

- (DesktopWindow *)init;
@end

// the global top bar
@interface MenuBarWindow: NSWindow {
    MenuView *menuView;
    ExtrasView *extrasView;
    ClockView *clockView;
}

- (MenuBarWindow *)init;
@end

// desktop interface controller
@interface AppDelegate: NSObject {
    MenuBarWindow *menuBar;
    DesktopWindow *background;
}

-(void)screenDidResize:(NSNotification *)note;
@end


