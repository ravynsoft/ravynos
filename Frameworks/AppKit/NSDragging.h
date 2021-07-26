/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

@class NSPasteboard, NSWindow, NSImage;

typedef unsigned NSDragOperation;

enum {
    NSDragOperationNone = 0x00,
    NSDragOperationCopy = 0x01,
    NSDragOperationLink = 0x02,
    NSDragOperationGeneric = 0x04,
    NSDragOperationPrivate = 0x08,
    NSDragOperationMove = 0x10,
    NSDragOperationDelete = 0x20,
    NSDragOperationEvery = UINT_MAX
};

@protocol NSDraggingInfo
- (NSPasteboard *)draggingPasteboard;
- (NSDragOperation)draggingSourceOperationMask;
- (NSPoint)draggingLocation;
- (NSImage *)draggedImage;
- (NSPoint)draggedImageLocation;
- (NSWindow *)draggingDestinationWindow;
- (id)draggingSource;
- (void)slideDraggedImageTo:(NSPoint)point;
- (int)draggingSequenceNumber;
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)destination;
@end

@interface NSObject (NSDragging_destination)
- (BOOL)prepareForDragOperation:(id<NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender;
- (void)concludeDragOperation:(id<NSDraggingInfo>)sender;

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender;
- (void)draggingExited:(id<NSDraggingInfo>)sender;
- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender;

- (BOOL)wantsPeriodicDraggingUpdates;
@end

@interface NSObject (NSDragging_source)
- (BOOL)ignoreModifierKeysWhileDragging;
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)flag;
- (void)draggedImage:(NSImage *)image beganAt:(NSPoint)point;
- (void)draggedImage:(NSImage *)image movedTo:(NSPoint)point;
- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)point operation:(NSDragOperation)operation;
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)destination;
@end
