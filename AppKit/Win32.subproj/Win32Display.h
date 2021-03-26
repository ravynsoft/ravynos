/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/NSDisplay.h>
#import <AppKit/NSEvent.h>

#import <windows.h>

@class NSEvent, NSColor, Win32EventInputSource, O2Context_gdi;

@interface Win32Display : NSDisplay {
    Win32EventInputSource *_eventInputSource;

    NSPasteboard *_generalPasteboard;
    NSMutableDictionary *_pasteboards;

    NSMutableDictionary *_nameToColor;

    id _cursor;
    int _cursorDisplayCount;
    NSMutableDictionary *_cursorCache;
    HCURSOR _lastCursor;

    int _clickCount;
    DWORD _lastTickCount;
    LPARAM _lastPosition;
    NSPoint _pastLocation;

    NSMutableString *_ignoringModifiersString;
    unsigned short _keyCode;
    BOOL _isKeypad;
}

+ (Win32Display *)currentDisplay;

- (NSArray *)screens;

- (NSPasteboard *)pasteboardWithName:(NSString *)name;

- (NSDraggingManager *)draggingManager;

/** Some Win32 fonts are not playing nice when auto-detected - Arial in particular */
- (void)forceLoadOfFontsAtPaths:(NSArray *)paths;

- (void)invalidateSystemColors;
- (NSColor *)colorWithName:(NSString *)colorName;

- (void)hideCursor;
- (void)unhideCursor;

// Arrow, IBeam, HorizontalResize, VerticalResize
- (id)cursorWithName:(NSString *)name;
- (void)setCursor:(id)cursor;

- (void)stopWaitCursor;
- (void)startWaitCursor;

- (BOOL)postMSG:(MSG)msg keyboardState:(BYTE *)keyboardState;

- (void)beep;

@end
