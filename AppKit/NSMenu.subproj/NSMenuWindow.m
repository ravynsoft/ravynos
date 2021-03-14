/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSMenuWindow.h>
#import <AppKit/NSSubmenuView.h>
#import <AppKit/NSOverflowMenuView.h>

@implementation NSMenuWindow

-initWithMenu:(NSMenu *)menu {
   NSSubmenuView *view=[[NSSubmenuView alloc] initWithMenu:menu];
   NSRect         contentRect=[view frame];

   [self initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
   [self setLevel:NSSubmenuWindowLevel];

   _releaseWhenClosed=YES;

   _view=view;

   [[self contentView] addSubview:_view];
	[self setAcceptsMouseMovedEvents:YES];
   return self;
}

-initWithMenu:(NSMenu *)menu overflowAtIndex:(unsigned)overflowIndex {
   NSOverflowMenuView *view=[[NSOverflowMenuView alloc] initWithMenu:menu overflowAtIndex:overflowIndex];
   NSRect              contentRect=[view frame];

   [self initWithContentRect: contentRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
   [self setLevel:NSSubmenuWindowLevel];

   _releaseWhenClosed=YES;

   _view=view;

   [[self contentView] addSubview:_view];
	[self setAcceptsMouseMovedEvents:YES];

   return self;
}

-(void)dealloc {
   [_view release];
   [super dealloc];
}

-(NSMenuView *)menuView {
   return _view;
}

@end
