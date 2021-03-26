/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/Win32DraggingManager.h>
#import <AppKit/Win32IDropTargetServer.h>
#import <AppKit/Win32IDropSourceServer.h>
#import <AppKit/Win32IDataObjectServer.h>

#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSDragging.h>

@implementation Win32DraggingManager

+(void)initialize {
   if(self==[Win32DraggingManager class])
    OleInitialize(NULL);
}

-init {
   _dropTargets=[NSMutableArray new];

   return self;
}

-(void)registerWindow:(NSWindow *)window dragTypes:(NSArray *)dragTypes {
   Win32IDropTargetServer *target=[[[Win32IDropTargetServer alloc] initWithWindow:(Win32Window *)[window platformWindow]] autorelease];

   if(target==nil) // RegisterDragDrop fails sometimes, unresolved.
    return;
    
   [_dropTargets addObject:target];
}

-(void)unregisterWindow:(NSWindow *)window {
   int count=[_dropTargets count];

   while(--count>=0){
    Win32IDropTargetServer *target=[_dropTargets objectAtIndex:count];

    if([target window]==(Win32Window *)[window platformWindow]){
     [target invalidate];
     [_dropTargets removeObjectAtIndex:count];
    }
   }
}

-(id)localDraggingSource {
   return _localDraggingSource;
}

static unsigned sourceOperationForSource(id source,BOOL isLocal){
   if([source respondsToSelector:@selector(draggingSourceOperationMaskForLocal:)])
    return [source draggingSourceOperationMaskForLocal:NO];
   
   return NSDragOperationCopy|NSDragOperationLink|NSDragOperationGeneric|NSDragOperationPrivate;
}

-(void)dragImage:(NSImage *)image at:(NSPoint)location offset:(NSSize)offset event:(NSEvent *)event pasteboard:(NSPasteboard *)pasteboard source:(id)source slideBack:(BOOL)slideBack {
   Win32IDataObjectServer *dataServer=[[Win32IDataObjectServer alloc] initWithPasteboard:(Win32Pasteboard *)pasteboard];
   Win32IDropSourceServer *dropSource=[[Win32IDropSourceServer alloc] initAsIDropSource];
   unsigned                sourceOperation=sourceOperationForSource(source,NO);
   unsigned                targetOperation;
   DWORD                   sourceEffect=Win32DropEffectFromDragOperation(sourceOperation);
   DWORD                   targetEffect=0;
   HRESULT                 error;

	[dropSource startDragImage:image at:location offset:offset event:event];
   _localDraggingSource=source;

   if([source respondsToSelector:@selector(draggedImage:beganAt:)])
    [source draggedImage:image beganAt:location];

   error=DoDragDrop([dataServer iUknown],[dropSource iUknown],sourceEffect,&targetEffect);  
   if(error!=DRAGDROP_S_DROP)
    NSLog(@"DoDragDrop failed with %d",error);

   if([source respondsToSelector:@selector(draggedImage:endedAt:operation:)]){
    targetOperation=Win32DragOperationFromDropEffect(targetEffect);
    [source draggedImage:image endedAt:location operation:targetOperation];
   }

   _localDraggingSource=nil;

   [dataServer release];
   [dropSource release];
}

@end

unsigned Win32DragOperationFromDropEffect(DWORD dropEffect) {
   unsigned result=0;

   if(dropEffect&DROPEFFECT_COPY)
    result|=NSDragOperationCopy;
   if(dropEffect&DROPEFFECT_MOVE)
    result|=NSDragOperationMove;
   if(dropEffect&DROPEFFECT_LINK)
    result|=NSDragOperationLink;

   return result;
}

DWORD Win32DropEffectFromDragOperation(unsigned dragOperation) {
 DWORD result=0;

   if(dragOperation&NSDragOperationCopy)
    result|=DROPEFFECT_COPY;
    if(dragOperation&NSDragOperationMove)
    result|=DROPEFFECT_MOVE;
   if(dragOperation&NSDragOperationLink)
    result|=DROPEFFECT_LINK;

   return result;
}
