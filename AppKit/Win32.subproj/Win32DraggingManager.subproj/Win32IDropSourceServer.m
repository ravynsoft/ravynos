/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32IDropSourceServer.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSPanel.h>
#import <AppKit/Win32Window.h>
#import <AppKit/NSDragView.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSDraggingManager.h>

@implementation Win32IDropSourceServer

-(void)dealloc {
   [_image release];
   [_window release];
   [super dealloc];
}

-(void)startDragImage:(NSImage *)image at:(NSPoint)location offset:(NSSize)offset event:(NSEvent *)event
{
	_image=[image retain];
	
	if(_image!=nil){
		NSSize  size=[image size];
		NSView *view=[[[NSDragView alloc] initWithImage:image] autorelease];
		
		_window=[[NSPanel alloc] initWithContentRect:NSMakeRect(0,0,size.width,size.height)
										   styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
		[_window setHasShadow:NO];
		[_window setOpaque:NO];
		[_window setBackgroundColor:[NSColor clearColor]];
		
		[_window setLevel:NSPopUpMenuWindowLevel];
		[[_window contentView] addSubview:view];
		
		[_window _setVisible:YES];
		[(Win32Window *)[_window platformWindow] showWindowWithoutActivation];
		[(Win32Window *)[_window platformWindow] bringToTop];

		// We don't want the window used to display the drag image to be itself a drop target...
		[(Win32Window *)[_window platformWindow] makeTransparent];
		
		// Set the offset between the start location and the mouse location in the event
		NSPoint eventLocation = [event locationInWindow];
		_offset.width = eventLocation.x - location.x; 
		_offset.height = eventLocation.y - location.y; 
	}
}

-(HRESULT)QueryContinueDrag:(BOOL)escapePressed:(DWORD)keyState {
#if 0
   NSMutableString *state=[NSMutableString string];

   if(keyState&MK_CONTROL)
    [state appendString:@"MK_CONTROL "];
   if(keyState&MK_SHIFT)
    [state appendString:@"MK_SHIFT "];
   if(keyState&MK_ALT)
    [state appendString:@"MK_ALT "];
   if(keyState&MK_LBUTTON)
    [state appendString:@"MK_LBUTTON "];
   if(keyState&MK_MBUTTON)
    [state appendString:@"MK_MBUTTON "];
   if(keyState&MK_RBUTTON)
    [state appendString:@"MK_RBUTTON "];

   NSLog(@"state=[%@]",state);
#endif

   NSPoint point=[(Win32Window *)[_window platformWindow] mouseLocationOutsideOfEventStream];
	point.x -= _offset.width;
	point.y -= _offset.height;
	
   [_window setFrameOrigin:point];

   if(escapePressed)
    return DRAGDROP_S_CANCEL;

   if(!(keyState&MK_LBUTTON))
    return DRAGDROP_S_DROP;

   return S_OK;
}

-(HRESULT)GiveFeedback:(DWORD)dwEffect {
	NSPoint point=[(Win32Window *)[_window platformWindow] mouseLocationOutsideOfEventStream];
	point.x -= _offset.width;
	point.y -= _offset.height;
	
	[_window setFrameOrigin:point];
	
   [[NSApp windows] makeObjectsPerformSelector:@selector(displayIfNeeded)];

   return DRAGDROP_S_USEDEFAULTCURSORS;
}

@end
