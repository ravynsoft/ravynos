/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSCachedImageRep.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSGraphicsContext.h>
#import <CoreGraphics/CGWindow.h>
#import <AppKit/NSWindow-Private.h>

@implementation NSCachedImageRep

-initWithWindow:(NSWindow *)window rect:(NSRect)rect {
   _size=rect.size;
   _window=[window retain];
   _origin=rect.origin;

// this is a little broken, the windows get resized to larger size when on-screen
	if([[NSUserDefaults standardUserDefaults] boolForKey:@"NSShowAllWindows"])
    [_window orderFront:nil];
   return self;
}

-initWithSize:(NSSize)size depth:(NSWindowDepth)windowDepth separate:(BOOL)separateWindow alpha:(BOOL)hasAlpha {
// FIXME: Implement shared caches, but also update NSImage to handle caching in them

	int styleMask = NSBorderlessWindowMask;
	if([[NSUserDefaults standardUserDefaults] boolForKey:@"NSShowAllWindows"] == NO) {
		styleMask |= NSAppKitPrivateWindow;
	}
	NSWindow *window=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,size.width,size.height) styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];

   NSDictionary *entries=[NSDictionary dictionaryWithObject:@"Onyx" forKey:@"CGContext"];
   
   [[window platformWindow] addEntriesToDeviceDictionary:entries];
   
   [self initWithWindow:window rect:NSMakeRect(0,0,size.width,size.height)];
   
   [window release];
   
   return self;
}

-(void)dealloc {
   [_window release];
   [super dealloc];
}

-(NSWindow *)window {
   return _window;
}

-(NSRect)rect {
   return NSMakeRect(_origin.x,_origin.y,_size.width,_size.height);
}

-(BOOL)drawAtPoint:(NSPoint)point {
   NSRect     rect={point,_size};
   CGImageRef imageRef=CGBitmapContextCreateImage([[_window graphicsContext] graphicsPort]);
   
   CGContextDrawImage(NSCurrentGraphicsPort(),rect,imageRef);
   
   CGImageRelease(imageRef);
   
   return YES;
}

-(BOOL)drawInRect:(NSRect)rect {
   CGImageRef imageRef=CGBitmapContextCreateImage([[_window graphicsContext] graphicsPort]);
   
   CGContextDrawImage(NSCurrentGraphicsPort(),rect,imageRef);
   
   CGImageRelease(imageRef);
   
   return YES;
}

- (void)draw
{
	[self drawInRect:[self rect]];
}
@end
