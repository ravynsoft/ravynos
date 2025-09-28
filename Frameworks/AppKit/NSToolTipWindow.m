/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/NSToolTipWindow.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSThemeFrame.h>
#import <AppKit/NSTrackingArea.h>

// Note: This file contains a few minor adjustments to get it pixel-accurate on Win32.
//       Those occurences are marked with "Should...".
#define TEXTFIELD_MARGIN     3.0

@implementation NSToolTipWindow

+ (NSToolTipWindow *)sharedToolTipWindow
{
    static NSToolTipWindow *singleton = nil;
    
    if (singleton == nil){
        singleton = [[NSToolTipWindow alloc] initWithContentRect:NSMakeRect(0, 0, 20, 20) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
        [singleton setLevel:NSPopUpMenuWindowLevel];
    }
    
    return singleton;
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)styleMask backing:(unsigned)backing defer:(BOOL)defer
{
    [super initWithContentRect:contentRect styleMask:styleMask backing:backing defer:defer];
    _textField = [[NSTextField alloc] initWithFrame:contentRect]; // Will be adjusted later.
    [_textField setFrameOrigin:NSMakePoint(2., -3.)]; // Should be (0., 0.).
    [_textField setFont:[NSFont toolTipsFontOfSize:0.]];
    [_textField setEditable:NO];
    [_textField setBordered:NO];
    [_textField setBezeled:NO];
    [self setBackgroundColor:[NSColor colorWithDeviceRed:1. green:1. blue:.88 alpha:1.]];
    [_backgroundView setWindowBorderType:NSWindowToolTipBorderType];
    [[self contentView] addSubview:_textField];

    _trackingArea = nil;
    _sizeAdjusted = NO;

    return self;
}

- (NSString *)toolTip
{
    return [_textField stringValue];
}

- (void)setToolTip:(NSString *)toolTip
{
    [_textField setStringValue:toolTip];
    _sizeAdjusted = NO;
}

- (NSTrackingArea *)_trackingArea {
    return _trackingArea;
}

- (void)_setTrackingArea:(NSTrackingArea *)trackingArea {
    _trackingArea=trackingArea;
}

// NSWindow override.
-(void)orderFront:sender
{
    if (_sizeAdjusted == NO) {
        NSSize messageSize = NSZeroSize;
        NSRect windowFrame = [self frame];

        messageSize = [[NSScreen mainScreen] visibleFrame].size;
        if([[NSUserDefaults standardUserDefaults] boolForKey:@"NSToolTipAutoWrappingDisabled"]==NO)
         messageSize.width /= 4.;
        else
         messageSize.width = NSStringDrawerLargeDimension;

        messageSize=[[NSStringDrawer sharedStringDrawer] sizeOfString:[_textField stringValue] withAttributes:[NSDictionary dictionaryWithObjectsAndKeys:[NSFont toolTipsFontOfSize:0.], NSFontAttributeName, nil] inSize:messageSize];
        messageSize.width += TEXTFIELD_MARGIN * 2;
        messageSize.width += 2.; // Shouldn't be neccessary.
        messageSize.height += TEXTFIELD_MARGIN * 2;

        [_textField setFrameSize:messageSize];
		
        windowFrame.origin = [NSEvent mouseLocation];
        windowFrame.origin.x += 10.;
        windowFrame.origin.y += 10.;
        windowFrame.size = messageSize;
        windowFrame.size.height -= 1.; // Shouldn't be neccessary.
        [self setFrame:windowFrame display:YES];
        
        _sizeAdjusted = YES;
    }

    [super orderFront:sender];
}

@end
