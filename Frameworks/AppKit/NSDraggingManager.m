/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSDraggingManager.h>
#import <AppKit/NSDisplay.h>
#import <AppKit/NSRaise.h>

@implementation NSDraggingManager

+(NSDraggingManager *)draggingManager {
   return [[NSDisplay currentDisplay] draggingManager];
}

-(void)registerWindow:(NSWindow *)window dragTypes:(NSArray *)types {
   NSInvalidAbstractInvocation();
}

-(void)unregisterWindow:(NSWindow *)window {
   NSInvalidAbstractInvocation();
}

-(id)localDraggingSource {
   NSInvalidAbstractInvocation();
   return nil;
}

// Location is expected to be in the same window coordinate system as the event
-(void)dragImage:(NSImage *)image at:(NSPoint)location offset:(NSSize)offset event:(NSEvent *)event pasteboard:(NSPasteboard *)pasteboard source:(id)source slideBack:(BOOL)slideBack {
   NSInvalidAbstractInvocation();
}

@end
