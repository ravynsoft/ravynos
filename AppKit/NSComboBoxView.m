/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Keyboard movement - David Young <daver@geeks.org>
// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSComboBoxView.h>
#import <AppKit/NSStringDrawer.h>

// same keyboard UI code from the pop-up menu.. or at least it's very similar
enum {
    KEYBOARD_INACTIVE,
    KEYBOARD_ACTIVE,
    KEYBOARD_OK,
    KEYBOARD_CANCEL
};

@implementation NSComboBoxView

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   _cellSize=frame.size;
   _font=[[NSFont messageFontOfSize:12] retain];
   return self;
}

-(void)dealloc {
   [_objects release];
   [_font release];
   [super dealloc];
}

-(BOOL)isFlipped {
   return YES;
}

-(void)setObjectArray:(NSArray *)objects {
   [_objects autorelease];
   _objects=[objects retain];
}

-(void)setFont:(NSFont *)font {
   [_font autorelease];
   _font=[font retain];
}

-(void)setSelectedIndex:(int)index {
   _selectedIndex = index;
}

-(NSDictionary *)itemAttributes {
   return [NSDictionary dictionaryWithObjectsAndKeys:
    _font,NSFontAttributeName,
    nil];
}

-(NSDictionary *)selectedItemAttributes {
   return [NSDictionary dictionaryWithObjectsAndKeys:
    _font,NSFontAttributeName,
    [NSColor selectedTextColor],NSForegroundColorAttributeName,
    nil];
}

-(NSSize)sizeForContents {
   int           count=[_objects count];
   NSSize        result=NSMakeSize([self bounds].size.width,0);

   result.height+=count*_cellSize.height;

   return result;
}

-(unsigned)itemIndexForPoint:(NSPoint)point {
   unsigned result;

   result=floor(point.y/_cellSize.height);

   if(result>=[_objects count])
    result=NSNotFound;

   return result;
}

-(NSRect)rectForItemAtIndex:(unsigned)index {
   NSRect result=NSMakeRect(0,0,_cellSize.width,_cellSize.height);

   result.origin.y+=index*_cellSize.height;

   return result;
}

-(void)drawItemAtIndex:(unsigned)index {
   id            item=[_objects objectAtIndex:index];
   NSDictionary *attributes;

   if(index==_selectedIndex)
    attributes=[self selectedItemAttributes];
   else
    attributes=[self itemAttributes];

   {
    NSString *string=[item description];
    NSSize    size=[string sizeWithAttributes:attributes];
    NSRect    itemRect=[self rectForItemAtIndex:index];

    if(index==_selectedIndex){
     [[NSColor selectedTextBackgroundColor] setFill];
     NSRectFill(itemRect);
    }
    else {
     [[NSColor textBackgroundColor] setFill];
     NSRectFill(itemRect);
    }

    itemRect=NSInsetRect(itemRect,1,1);
    itemRect.origin.y+=floor((_cellSize.height-size.height)/2);
    itemRect.size.height=size.height;
     
    [string _clipAndDrawInRect:itemRect withAttributes:attributes];
   }
}

-(void)drawRect:(NSRect)rect {
   int i,count=[_objects count];

   [[NSColor textBackgroundColor] setFill];
   NSRectFill([self bounds]);

   for(i=0;i<count;i++){
    [self drawItemAtIndex:i];
   }
}

-(void)rightMouseDown:(NSEvent *)event {
   // do nothing
}

#if 0
-(int)runTrackingWithEvent:(NSEvent *)event {
   enum {
    STATE_FIRSTMOUSEDOWN,
    STATE_MOUSEDOWN,
    STATE_MOUSEUP,
    STATE_EXIT
   } state=STATE_FIRSTMOUSEDOWN;
   NSPoint firstPoint,point=[event locationInWindow];

// point comes in on controls window
   point=[[event window] convertBaseToScreen:point];
   point=[[self window] convertScreenToBase:point];
   point=[self convertPoint:point fromView:nil];
   firstPoint=point;

   [self lockFocus];
   [self drawRect:[self bounds]];

   _selectedIndex=NSNotFound;

   do {
    unsigned index=[self itemIndexForPoint:point];

    if(_selectedIndex!=index){
     unsigned previous=_selectedIndex;

      _selectedIndex=index;

     if(previous!=NSNotFound)
      [self drawItemAtIndex:previous];

     if(_selectedIndex!=NSNotFound)
      [self drawItemAtIndex:_selectedIndex];
    }
    [[self window] flushWindow];

    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|
                          NSLeftMouseDraggedMask];
    point=[self convertPoint:[event locationInWindow] fromView:nil];

    switch(state){
     case STATE_FIRSTMOUSEDOWN:
      if(NSEqualPoints(firstPoint,point)){
       if([event type]==NSLeftMouseUp)
        state=STATE_MOUSEUP;
      }
      else
       state=STATE_MOUSEDOWN;      
      break;

     default:
      if([event type]==NSLeftMouseUp)
       state=STATE_EXIT;
      break;
    }

   }while(state!=STATE_EXIT);

   [self unlockFocus];

   return _selectedIndex;
}
#endif

// DWY: trying to do keyboard navigation with as little impact on the rest of the code as possible...
-(int)runTrackingWithEvent:(NSEvent *)event {
   enum {
    STATE_FIRSTMOUSEDOWN,
    STATE_MOUSEDOWN,
    STATE_MOUSEUP,
    STATE_EXIT
   } state=STATE_FIRSTMOUSEDOWN;
   NSPoint firstLocation,point=[event locationInWindow];
   unsigned initialSelectedIndex = _selectedIndex;


// point comes in on controls window
   point=[[event window] convertBaseToScreen:point];
   point=[[self window] convertScreenToBase:point];
   point=[self convertPoint:point fromView:nil];
   firstLocation=point;

   [self lockFocus];
   [self drawRect:[self bounds]];

	// Make sure we know if the user clicks away from the app in the middle of this
	BOOL cancelled = NO;
    	
   do {
    unsigned index=[self itemIndexForPoint:point];
    NSRect   screenVisible;

    if(index!=NSNotFound && _keyboardUIState == KEYBOARD_INACTIVE){
     if(_selectedIndex!=index){
      unsigned previous=_selectedIndex;

      _selectedIndex=index;

      if(previous!=NSNotFound)
       [self drawItemAtIndex:previous];

      [self drawItemAtIndex:_selectedIndex];
     }
    }
    [[self window] flushWindow];

    event=[[self window] nextEventMatchingMask:NSLeftMouseDownMask|NSLeftMouseUpMask|NSLeftMouseDraggedMask|NSKeyDownMask];
    if ([event type] == NSKeyDown) {
        [self interpretKeyEvents:[NSArray arrayWithObject:event]];
        switch (_keyboardUIState) {
            case KEYBOARD_INACTIVE:
                _keyboardUIState = KEYBOARD_ACTIVE;
                continue;

            case KEYBOARD_ACTIVE:
                break;
                
            case KEYBOARD_CANCEL:
                _selectedIndex = initialSelectedIndex;
            case KEYBOARD_OK:
                state=STATE_EXIT;
                break;
        }
    }
    else
        _keyboardUIState = KEYBOARD_INACTIVE;

	   if ([event type] == NSAppKitDefined) {
		   if ([event subtype] == NSApplicationDeactivated) {
			   cancelled = YES;
		   }
	   }
    point=[event locationInWindow];
    point=[[event window] convertBaseToScreen:point];
    screenVisible=NSInsetRect([[[self window] screen] visibleFrame],4,4);
    if(NSPointInRect(point,[[self window] frame]) && !NSPointInRect(point,screenVisible)){
     NSPoint origin=[[self window] frame].origin;
     BOOL    change=NO;

     if(point.y<NSMinY(screenVisible)){
      origin.y+=_cellSize.height;
      change=YES;
     }

     if(point.y>NSMaxY(screenVisible)){
      origin.y-=_cellSize.height;
      change=YES;
     }

     if(change)
      [[self window] setFrameOrigin:origin];
    }

    point=[self convertPoint:[event locationInWindow] fromView:nil];

    switch(state){
     case STATE_FIRSTMOUSEDOWN:
      if(NSEqualPoints(firstLocation,point)){
       if([event type]==NSLeftMouseUp)
        state=STATE_MOUSEUP;
      }
      else
       state=STATE_MOUSEDOWN;      
      break;

     default:
      if([event type]==NSLeftMouseUp)
       state=STATE_EXIT;
      break;
    }

   }while(cancelled == NO && state!=STATE_EXIT);

   [self unlockFocus];

   _keyboardUIState = KEYBOARD_INACTIVE;
   
   return _selectedIndex;
}

- (void)keyDown:(NSEvent *)event {
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (void)moveUp:(id)sender {
    int previous = _selectedIndex;
    
    _selectedIndex--;
    if ((int)_selectedIndex < 0)
        _selectedIndex = 0;

    if (previous != NSNotFound)
        [self drawItemAtIndex:previous];

    [self drawItemAtIndex:_selectedIndex];
}

- (void)moveDown:(id)sender {
    int previous = _selectedIndex;
    
    _selectedIndex++;
    if (_selectedIndex >= [_objects count])
        _selectedIndex = [_objects count]-1;

    if (previous != NSNotFound)
        [self drawItemAtIndex:previous];

    [self drawItemAtIndex:_selectedIndex];
}

- (void)cancel:(id)sender {
    _keyboardUIState = KEYBOARD_CANCEL;
}

- (void)insertNewline:(id)sender {
    _keyboardUIState = KEYBOARD_OK;
}


@end
