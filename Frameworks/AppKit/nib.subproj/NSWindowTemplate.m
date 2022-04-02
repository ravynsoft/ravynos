/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSWindowTemplate.h"
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSMainMenuView.h>
#import <AppKit/NSWindow-Private.h>

@interface NSWindow(private)
+(BOOL)hasMainMenuForStyleMask:(NSUInteger)styleMask;
@end

@implementation NSWindowTemplate

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _maxSize=[keyed decodeSizeForKey:@"NSMaxSize"];
    _minSize=[keyed decodeSizeForKey:@"NSMinSize"];
    _screenRect=[keyed decodeRectForKey:@"NSScreenRect"]; // screen created on
    _viewClass=[[keyed decodeObjectForKey:@"NSViewClass"] retain];
    _wtFlags=[keyed decodeIntForKey:@"NSWTFlags"];
    _windowBacking=[keyed decodeIntForKey:@"NSWindowBacking"];
    _windowClass=[[keyed decodeObjectForKey:@"NSWindowClass"] retain];
    _windowRect=[keyed decodeRectForKey:@"NSWindowRect"];
    _windowStyleMask=[keyed decodeIntForKey:@"NSWindowStyleMask"];
    _windowTitle=[[keyed decodeObjectForKey:@"NSWindowTitle"] retain];
    _windowView=[[keyed decodeObjectForKey:@"NSWindowView"] retain];
	_windowAutosave=[[keyed decodeObjectForKey:@"NSFrameAutosaveName"] retain];

    if ([NSScreen mainScreen])
      _windowRect.origin.y -= _screenRect.size.height - [[NSScreen mainScreen] frame].size.height;
#ifdef MENUS_IN_WINDOW
    if ([NSClassFromString(_windowClass) hasMainMenuForStyleMask:_windowStyleMask])
      _windowRect.origin.y -= [NSMainMenuView menuHeight];   // compensation for the additional menu bar
#endif
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return self;
}

-(void)dealloc {
   [_viewClass release];
   [_windowClass release];
   [_windowTitle release];
   [_windowView release];
   [_windowAutosave release];
   [super dealloc];
}

-awakeAfterUsingCoder:(NSCoder *)coder {
   NSWindow *result;
   Class     class;
   BOOL      defer;

   if((class=NSClassFromString(_windowClass))==Nil){
    [NSException raise:NSInvalidArgumentException format:@"Unable to locate NSWindow class %@, using NSWindow",_windowClass];
    class=[NSWindow class];
   }
   defer=(_wtFlags&0x20000000)?YES:NO;
   result=[[class alloc] initWithContentRect:_windowRect styleMask:_windowStyleMask backing:_windowBacking defer:defer];
   [result setMinSize:_minSize];
   [result setMaxSize:_maxSize];
   [result setOneShot:(_wtFlags&0x10000000)?YES:NO];
   [result setReleasedWhenClosed:(_wtFlags&0x40000000)?NO:YES];
   [result setHidesOnDeactivate:(_wtFlags&0x80000000)?YES:NO];
   [result setTitle:_windowTitle];
   
   [result setContentView:_windowView];
   [_windowView setAutoresizesSubviews:YES];
   [_windowView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
   
   if([_viewClass isKindOfClass:[NSToolbar class]]) {
      [result setToolbar:_viewClass];
   }

   if([_windowAutosave length]>0)
    [result _setFrameAutosaveNameNoIO:_windowAutosave];

   [self release];
   return result;
}

@end
