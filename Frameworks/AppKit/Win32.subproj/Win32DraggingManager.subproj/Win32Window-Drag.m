/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32Window-Drag.h>
#import <AppKit/Win32DragSession.h>
#import <AppKit/Win32Pasteboard.h>
#import <AppKit/NSWindow-Drag.h>

@implementation Win32Window(Win32Window_drag)

- (id)receiverWithSession:(Win32DragSession *)session
{
    NSWindow *window=[self delegate];
    id receiver=[window _receiverForDragSession:session];
    id sessionReceiver=[session receiver];
    if (receiver!=sessionReceiver && receiver && sessionReceiver) {
        // If receiver is a subview if [session receiver] and it hasn't registered
        // for this type of drop, continue using [session receiver] as the target
        // (as Cocoa on Mac does)
        if ([receiver isKindOfClass:[NSView class]] &&
            [sessionReceiver isKindOfClass:[NSView class]]) {

            if ([receiver isDescendantOf:sessionReceiver] &&
                [session pasteboardTypesIntersectTypes:[sessionReceiver _draggedTypes]] &&
                ![sessionReceiver pasteboardTypesIntersectTypes:[receiver _draggedTypes]]) {

                receiver = [session receiver];
            }
        }
    }
    return receiver;
}

-(void)dragEnter:(Win32DragSession *)session {
   id receiver=[self receiverWithSession:session];
   NSDragOperation operation;

   [session setReceiver:receiver];
   operation=[receiver draggingEntered:session];
   [session setSourceOperationMask:operation];
}

-(void)dragOver:(Win32DragSession *)session {
   id receiver=[self receiverWithSession:session];
   NSDragOperation operation;

   if(receiver==[session receiver]){
    operation=[receiver draggingUpdated:session];
   }
   else {
    [[session receiver] draggingExited:session];

    [session setReceiver:receiver];
    operation=[receiver draggingEntered:session];
   }
   [session setSourceOperationMask:operation];
}

-(void)dragLeave:(Win32DragSession *)session {
   [[session receiver] draggingExited:session];
}

-(void)drop:(Win32DragSession *)session {
   id receiver=[self receiverWithSession:session];

   if(receiver!=[session receiver]){
    [[session receiver] draggingExited:session];
    [session setReceiver:receiver];
   }

   [receiver prepareForDragOperation:session];
   [receiver performDragOperation:session];
   [receiver concludeDragOperation:session];
}

@end
