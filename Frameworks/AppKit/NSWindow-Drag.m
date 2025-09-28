/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSWindow-Drag.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSThemeFrame.h>

@interface NSView(NSView_drag)
-(NSArray *)_dragHitTest:(NSPoint)point;
@end

@implementation NSView(NSView_Drag)
-(NSArray *)_dragHitTest:(NSPoint)point {

   point=[self convertPoint:point fromView:[self superview]];

   if(!NSMouseInRect(point,[self visibleRect],[self isFlipped]))
    return nil;
   else {
    NSMutableArray *result=[NSMutableArray array];
    NSArray        *subviews=[self subviews];
    int             count=[subviews count];

    while(--count>=0){ // front to back
     NSView  *check=[subviews objectAtIndex:count];
     NSArray *hit=[check _dragHitTest:point];

     [result addObjectsFromArray:hit];
    }
    [result addObject:self];
    return result;
   }
}

@end

@implementation NSWindow(NSWindow_drag)

-(id)_receiverForDragSession:(id <NSDraggingInfo>)session {
   NSArray *types=[[session draggingPasteboard] types];
   NSPoint  point=[session draggingLocation];
   NSArray *hitArray=[[self _backgroundView] _dragHitTest:point];
   int      i,count=[hitArray count];
   id       hit=nil;

   for(i=0;i<count;i++){
    hit=[hitArray objectAtIndex:i];

    if([types firstObjectCommonWithArray:[hit _draggedTypes]]!=nil)
     return hit;
   }

   return ([types firstObjectCommonWithArray:[self _draggedTypes]]!=nil)?(id)self:nil;
}

@end
