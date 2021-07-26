/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/Win32DragSession.h>
#import <AppKit/Win32Window.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSDraggingManager.h>
#import <AppKit/NSRaise.h>

@implementation Win32DragSession

-initWithWindow:(Win32Window *)window pasteboard:(NSPasteboard *)pasteboard {
   _window=[window retain];
   _pasteboard=[pasteboard retain];
   _source=[[NSDraggingManager draggingManager] localDraggingSource]; 
   return self;
}

-(void)dealloc {
   [_window release];
   [_pasteboard release];
   [super dealloc];
}

-(void)setSourceOperationMask:(unsigned)mask {
   _sourceOperationMask=mask;
}

-(void)setScreenLocation:(NSPoint)point {
   _screenLocation=point;
}

-(id)receiver {
   return _receiver;
}

-(void)setReceiver:(id)object {
   _receiver=object;
}

-(BOOL)pasteboardTypesIntersectTypes:(NSArray *)check {
   return ([[[self draggingPasteboard] types] firstObjectCommonWithArray:check]!=nil)?YES:NO;
}

-(NSWindow *)draggingDestinationWindow {
   return [_window delegate];
}

-(unsigned)draggingSourceOperationMask {
   return _sourceOperationMask;
}

-(NSPoint)draggingLocation {
   return [[self draggingDestinationWindow] convertScreenToBase:_screenLocation];
}

-(NSPoint)draggedImageLocation {
   return _imageLocation;
}

-(NSImage *)draggedImage {
   return _image;
}

-(NSPasteboard *)draggingPasteboard {
   return _pasteboard;
}

-(id)draggingSource {
   return _source;
}

-(int)draggingSequenceNumber {
   return _sequenceNumber;
}

-(void)slideDraggedImageTo:(NSPoint)screenPoint {
   NSUnimplementedMethod();
}

-(NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)destination {
   NSUnimplementedMethod();
   return nil;
}

@end
