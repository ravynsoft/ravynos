/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32IDropTargetServer.h>
#import <AppKit/Win32DropPasteboard.h>
#import <AppKit/Win32DraggingManager.h>
#import <AppKit/Win32DragSession.h>

#import <AppKit/Win32Window-Drag.h>

@implementation Win32IDropTargetServer

-initWithWindow:(Win32Window *)window {
   HRESULT result;

   [super initAsIDropTarget];
   _window=window;

   if((result=RegisterDragDrop([_window windowHandle],[self iUknown]))!=S_OK){
    NSLog(@"RegisterDragDrop failed with %d, handle=%p",result,[_window windowHandle]);
    [super dealloc];
    return nil;
   }
   
   return self;
}

-(Win32Window *)window {
   return _window;
}

-(void)invalidate {
   HRESULT result;

   if((result=RevokeDragDrop([_window windowHandle]))!=S_OK)
    NSLog(@"RevokeDragDrop failed with %d",result);
}

-(HRESULT)DragEnter:(IDataObject *)dataObject:(DWORD)grfKeyState:(POINTL)pt:(DWORD *)dropEffect {
   NSPasteboard *pasteboard=[[[Win32DropPasteboard alloc] initWithIDataObject:dataObject] autorelease];

   [_session release];

   _session=[[Win32DragSession alloc] initWithWindow:_window pasteboard:pasteboard];
   [_session setSourceOperationMask:Win32DragOperationFromDropEffect(*dropEffect)];
   [_session setScreenLocation:[_window convertPOINTLToBase:pt]];

   [_window dragEnter:_session];

   *dropEffect=Win32DropEffectFromDragOperation([_session draggingSourceOperationMask]);

   return S_OK;
}

-(HRESULT)DragOver:(DWORD)grfKeyState:(POINTL)pt:(DWORD *)dropEffect {

   [_session setSourceOperationMask:Win32DragOperationFromDropEffect(*dropEffect)];
   [_session setScreenLocation:[_window convertPOINTLToBase:pt]];

   [_window dragOver:_session];

   *dropEffect=Win32DropEffectFromDragOperation([_session draggingSourceOperationMask]);

   return S_OK;
}

-(HRESULT)DragLeave {

   [_window dragLeave:_session];

   [_session release];
   _session=nil;
   return S_OK;
}

-(HRESULT)Drop:(IDataObject *)dataObject:(DWORD)grfKeyState:(POINTL)pt:(DWORD *)dropEffect {

   [_session setSourceOperationMask:Win32DragOperationFromDropEffect(*dropEffect)];
   [_session setScreenLocation:[_window convertPOINTLToBase:pt]];

   [_window drop:_session];

   *dropEffect=Win32DropEffectFromDragOperation([_session draggingSourceOperationMask]);

   [_session release];
   _session=nil;
   return S_OK;
}


@end
