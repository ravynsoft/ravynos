/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSPopUpWindow.h>
#import <AppKit/NSPopUpView.h>

@implementation NSPopUpWindow

-initWithFrame:(NSRect)frame {
   [self initWithContentRect:frame styleMask:NSBorderlessWindowMask|WLWindowPopUp backing:NSBackingStoreBuffered defer:NO];
   [self setLevel:NSPopUpMenuWindowLevel];
   _releaseWhenClosed=YES;

   _view=[[NSPopUpView alloc] initWithFrame:NSMakeRect(0,0,frame.size.width,frame.size.height)];
   [[self contentView] addSubview:_view];

   return self;
}

-(void)dealloc {
   [_view release];
   [super dealloc];
}

-(void)setParent:(id)window {
    [_platformWindow setParent:window];
}

-(void)setMenu:(NSMenu *)menu {
   [_view setMenu:menu];
}

-(void)setFont:(NSFont *)font {
   [_view setFont:font];
}

-(void)setPullsDown:(BOOL)pullsDown {
   [_view setPullsDown:pullsDown];
}

-(void)selectItemAtIndex:(int)index {
   [_view selectItemAtIndex:index];
}

-(int)runTrackingWithEvent:(NSEvent *)event {
    NSSize size = [_view sizeForContents];
    NSRect selectedRect = [_view rectForSelectedItem];
    NSRect frame = [self frame];

    frame.size=size;
    frame.origin.y-=(size.height-selectedRect.origin.y)-selectedRect.size.height;
    [self setFrame:frame display:NO];

    [_view setFrameSize:size];
    [_view setFrameOrigin:NSMakePoint(0,0)];

    [_view setNeedsDisplay: YES];

    [self orderFront:nil];
    return [_view runTrackingWithEvent:event];
}

@end
