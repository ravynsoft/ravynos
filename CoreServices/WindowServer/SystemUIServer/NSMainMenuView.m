/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
   Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#import "NSMainMenuView.h"
#import "NSMenuWindow.h"
#import "NSSubmenuView.h"
#import <AppKit/NSFont.h>
#import <AppKit/NSGraphicsStyle.h>

enum {
	kNSMenuKeyboardNavigationNone,
	kNSMenuKeyboardNavigationUp,
	kNSMenuKeyboardNavigationDown,
	kNSMenuKeyboardNavigationLeft,
	kNSMenuKeyboardNavigationRight,
	kNSMenuKeyboardNavigationLetter
};

@implementation NSMainMenuView

+(NSFont *)menuFont {
    return [NSFont menuFontOfSize:15.0];
}

+(float)menuHeight {
    NSDictionary *attributes=[NSDictionary dictionaryWithObjectsAndKeys:
        [self menuFont], NSFontAttributeName, nil];
    float result=[@"Menu" sizeWithAttributes:attributes].height;

    result+=4;
    return result;
}

-initWithFrame:(NSRect)frame menu:(NSMenu *)menu {
    self = [super initWithFrame:frame];
    _menu=menu;
    _font=[[self class] menuFont];
    _selectedItemIndex=NSNotFound;
    [self sizeToFit];
    return self;
}

-(BOOL)isFlipped {
   return YES;
}

-(BOOL)isOpaque {
   return YES;
}

-(NSMenu *)menu {
   return _menu;
}

-(void)setMenu:(NSMenu *)menu {
   [super setMenu:menu];
   [self sizeToFit];
}

-(NSRect)titleRectForItem:(NSMenuItem *)item previousBorderRect:(NSRect)previousBorderRect
{
    NSRect result;
    NSSize titleSize = NSZeroSize;
    if([item hasSubmenu] && [[[item submenu] _name] isEqualToString:@"NSAppleMenu"])
        titleSize = [[self graphicsStyle] menuItemAttributedTextSize:[item attributedTitle]];
    else
        titleSize = [[self graphicsStyle] menuItemTextSize:[item title]];
    NSImage *img = [item image];
    if(img) {
        titleSize.width += [img size].width;
        if(titleSize.height < 22)
            titleSize.height = 22; // whee, magic numbers (height of menu bar)
    }

    result.origin = NSMakePoint(NSMaxX(previousBorderRect)+6,floor(([self bounds].size.height-titleSize.height)/2));
    result.size = titleSize;

    return result;
}

-(NSRect)borderRectFromTitleRect:(NSRect)titleRect {
    NSRect result=NSInsetRect(titleRect,-6,0);

    result.size.height=[self bounds].size.height;
    result.origin.y=0;

    return result;
}

-(void)sizeToFit {
    [self setFrameSize:NSMakeSize([self frame].size.width,[[self class] menuHeight])];
}

-(NSScreen *)_screenForPoint:(NSPoint)point {
   NSArray *screens=[NSScreen screens];
   int      i,count=[screens count];

   for(i=0;i<count;i++){
    NSScreen *check=[screens objectAtIndex:i];

    if(NSPointInRect(point,[check frame]))
     return check;
   }

   return [screens objectAtIndex:0];// should not happen
}

-(NSUInteger)selectedItemIndex {
   return _selectedItemIndex;
}

-(void)setSelectedItemIndex:(NSUInteger)itemIndex {
	if (_selectedItemIndex != itemIndex) {
		_selectedItemIndex=itemIndex;
		[self setNeedsDisplay:YES];
	}
}


-(NSArray *)itemArray {
   return [[self menu] itemArray];
}

-(NSArray *)visibleItemArray
{
   NSArray * items = [[self menu] itemArray];
   
   // Construct a new array of just the visible items
   if( _visibleArray == NULL )
      _visibleArray = [[NSMutableArray init] alloc];
	
   [_visibleArray removeAllObjects];
   
   int i;
   for( i = 0; i < [items count]; i++ )
   {
      NSMenuItem *item = [items objectAtIndex: i];
	  if( ![item isHidden] )
	     [_visibleArray addObject: item];
   }
   
   return _visibleArray;
}

#if 0
static void drawSunkenBorder(NSRect rect){
   NSRect   rects[5];
   NSColor *colors[5];

   rects[0]=rect;
   rects[0].size.width=1;
   colors[0]=[NSColor darkGrayColor];
   rects[1]=rect;
   rects[1].size.height=1;
   colors[1]=[NSColor darkGrayColor];
   rects[2]=rect;
   rects[2].origin.x=NSMaxX(rect)-1;
   rects[2].size.width=1;
   colors[2]=[NSColor whiteColor];
   rects[3]=rect;
   rects[3].origin.y=NSMaxY(rect)-1;
   rects[3].size.height=1;
   colors[3]=[NSColor whiteColor];
   rects[4]=NSInsetRect(rect,1,1);
   colors[4]=[NSColor controlColor];

   NSRectFillListWithColors(rects,colors,5);
}
#endif

-(NSImage *)overflowImage {
   return [NSImage imageNamed:@"NSMenuViewDoubleRightArrow"];
}

-(NSRect)overflowRect {
   NSRect   bounds=[self bounds];
   NSImage *image=[self overflowImage];
   NSSize   size=[image size];
   NSRect   rect=NSInsetRect(NSMakeRect(0,0,size.width,bounds.size.height),-3,0);

   rect.origin.x=NSMaxX(bounds)-rect.size.width;

   return rect;
}

-(void)drawRect:(NSRect)rect {
	NSRect        bounds=[self bounds];
	NSArray      *items=[[self menu] itemArray];
	int           i,count=[items count];
	NSRect        previousBorderRect=NSMakeRect(0,0,0,0);
	BOOL          overflow=NO;
	NSPoint       mouseLoc = [[NSApp currentEvent] locationInWindow];
	    
	mouseLoc = [self convertPoint:mouseLoc fromView:nil];
	
	[[self graphicsStyle] drawMenuBarBackgroundInRect:bounds];
	
	for(i=0;i<count;i++){
		NSMenuItem *item=[items objectAtIndex:i];
		NSString   *title=[item title];
		NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
		NSRect      borderRect=[self borderRectFromTitleRect:titleRect];
		
		[[self graphicsStyle] drawMenuBarItemBorderInRect:borderRect hover:(i==_selectedItemIndex)/*NSPointInRect(mouseLoc,borderRect)*/ selected:(i==_selectedItemIndex)];
		
		titleRect.origin.x = borderRect.origin.x + (NSWidth(borderRect) - NSWidth(titleRect)) / 2;
		titleRect.origin.y = borderRect.origin.y + (NSHeight(borderRect) - NSHeight(titleRect)) / 2;

                NSImage *img = [item image];
                if(img) {
                    NSPoint pt = titleRect.origin;
                    pt.y += ([img size].height);
                    [[item image] compositeToPoint:pt operation:NSCompositeSourceOver];
                }

                if([item hasSubmenu] && [[[item submenu] _name] isEqualToString:@"NSAppleMenu"]) 
                    [[self graphicsStyle] drawAttributedMenuItemText:[item attributedTitle]
                        inRect:titleRect enabled:YES selected:(i==_selectedItemIndex)];
                else
                    [[self graphicsStyle] drawMenuItemText:title inRect:titleRect
                        enabled:YES selected:(i==_selectedItemIndex)];
		
		previousBorderRect=borderRect;
		
		if(NSMaxX(borderRect)>NSMaxX(bounds)){
			overflow=YES;
			break;
		}
	}
	
	if(overflow){
		NSImage *image=[self overflowImage];
		NSSize   size=[image size];
		NSRect   rect=[self overflowRect];
		NSPoint  origin;
		NSRect   fill=rect;
		
		[[NSColor controlColor] setFill];
		fill.origin.x-=4;
		fill.size.width+=4;
		NSRectFill(fill);
		
#if 0
		if(_selectedItemIndex==count)
			drawSunkenBorder(rect);
#endif

		origin=rect.origin;
		origin.x+=3;
		origin.y+=floor((rect.size.height-size.height)/2);
		[image compositeToPoint:origin operation:NSCompositeSourceOver];
	}
}

-(unsigned)overflowIndex {
   NSRect        bounds=[self bounds];
   NSArray      *items=[[self menu] itemArray];
   unsigned      i,count=[items count];
   NSRect        previousBorderRect=NSMakeRect(0,0,0,0);

   for(i=0;i<count;i++){
    NSMenuItem *item=[items objectAtIndex:i];
    NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
    NSRect      borderRect=[self borderRectFromTitleRect:titleRect];

    if(NSMaxX(borderRect)>NSMaxX(bounds))
     return i;

    previousBorderRect=borderRect;
   }

   return NSNotFound;
}

-(unsigned)itemIndexAtPoint:(NSPoint)point {
   NSInteger    result=NSNotFound;
   NSRect        bounds=[self bounds];
   NSArray      *items=[[self menu] itemArray];
   unsigned      i,count=[items count];
   NSRect        previousBorderRect=NSMakeRect(0,0,0,0);
   BOOL          overflow=NO;

   for(i=0;i<count;i++){
    NSMenuItem *item=[items objectAtIndex:i];
    NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
    NSRect      borderRect=[self borderRectFromTitleRect:titleRect];

    if(NSMaxX(borderRect)>NSMaxX(bounds))
     overflow=YES;

    if(NSMouseInRect(point,borderRect,[self isFlipped]))
     result=i;

    previousBorderRect=borderRect;
   }

   if(overflow){
    if(NSMouseInRect(point,[self overflowRect],[self isFlipped]))
     return count;
   }

   return result;
}

-(void)positionBranchForSelectedItem:(NSMenuWindow *)branch screen:(NSScreen *)screen {
   NSRect        branchFrame=[branch frame];
   NSRect        screenVisible=[screen visibleFrame];
   NSArray      *items=[[self menu] itemArray];
   unsigned      i,count=[items count];
   NSRect        previousBorderRect=NSMakeRect(0,0,0,0);
   NSRect        itemRect=NSZeroRect;
   NSPoint       topLeft=NSZeroPoint;

   [[branch menuView] setWindow:branch]; // make sure something with ARC keeps a ref!

   if(_selectedItemIndex==count){
    itemRect=[self overflowRect];
    topLeft=NSMakePoint(itemRect.origin.x,NSMaxY(itemRect));
   } else {
    for(i=0;i<count;i++){
     NSMenuItem *item=[items objectAtIndex:i];
     NSRect      titleRect=[self titleRectForItem:item previousBorderRect:previousBorderRect];
     itemRect=[self borderRectFromTitleRect:titleRect];
 
     if(i==_selectedItemIndex){
      topLeft=NSMakePoint(itemRect.origin.x,NSMaxY(itemRect));
      break;
     }

     previousBorderRect=itemRect;
    }
   }

   topLeft=[self convertPoint:topLeft toView:nil];
   topLeft=[[self window] convertBaseToScreen:topLeft];

   if(topLeft.y-branchFrame.size.height<NSMinY(screenVisible)){
    topLeft=NSMakePoint(itemRect.origin.x,NSMinY(itemRect));

    topLeft=[self convertPoint:topLeft toView:nil];
    topLeft=[[self window] convertBaseToScreen:topLeft];

    topLeft.y+=branchFrame.size.height;
   }

   if(topLeft.x+branchFrame.size.width>NSMaxX(screenVisible))
    topLeft.x=NSMaxX(screenVisible)-branchFrame.size.width;
   if(topLeft.x<NSMinX(screenVisible))
    topLeft.x=NSMinX(screenVisible);

  [branch setFrameTopLeftPoint:topLeft];
}

-(SubmenuView *)viewAtSelectedIndexPositionOnScreen:(NSScreen *)screen {
   NSArray *items=[self visibleItemArray];

#if 0
   if(_selectedItemIndex==[items count]){
    NSMenuWindow *branch=[[NSMenuWindow alloc] initWithMenu:[self menu] overflowAtIndex:[self overflowIndex]];

    [self positionBranchForSelectedItem:branch screen:screen];

    [branch orderFront:nil];
    return [branch menuView];
   }
#endif
   if(_selectedItemIndex<[items count]){
    NSMenuItem *item=[items objectAtIndex:_selectedItemIndex];

    if([item hasSubmenu]){
     if([[[item submenu] itemArray] count]>0){
      NSMenuWindow *branch=[[NSMenuWindow alloc] initWithMenu:[item submenu]];

      [self positionBranchForSelectedItem:branch screen:screen];

      [branch orderFront:nil];
      return [branch menuView];
     }
    }
   }
   return nil;
}

-(NSMenuItem *)itemAtSelectedIndex {
   NSArray *items=[self visibleItemArray];

   if(_selectedItemIndex<[items count])
    return [items objectAtIndex:_selectedItemIndex];

   return nil;
}

const float kMenuInitialClickThreshold = .3f;
const float kMouseMovementThreshold = .001f;
#if 0
#define MENUDEBUG(...) NSLog(__VA_ARGS__)
#else
#define MENUDEBUG(...)
#endif

-(NSMenuItem *)trackForEvent:(NSEvent *)event {
	NSMenuItem *item=nil;
	
	enum {
		STATE_FIRSTMOUSEDOWN,
		STATE_MOUSEDOWN,
		STATE_MOUSEUP,
		STATE_EXIT
	} state=STATE_FIRSTMOUSEDOWN;
	
	// Get the menu management ball rolling
	NSPoint point=[event locationInWindow];
	NSPoint firstPoint=point;
	NSTimeInterval firstTimestamp = [event timestamp];
	
	// Cascading menus we manage will be pushed and popped on the viewStack
	NSMutableArray *viewStack=[NSMutableArray array];
	
	// Make sure we can put things back the way we found them
	BOOL oldAcceptsMouseMovedEvents = [[self window] acceptsMouseMovedEvents];
	
	// But while we're dealing with menus we want to track the mouse...
	[[self window] setAcceptsMouseMovedEvents:YES];
	
	// Make sure the menu contents are up to date
	[[self menu] update];
	
	// And we, of course, are first on the stack
	[viewStack addObject:self];
	
	int keyboardNavigationAction = kNSMenuKeyboardNavigationNone;
	
	BOOL cancelled = NO;
	
	MENUDEBUG(@"entering outer loop");
	
	// Start tracking the mouse movements and clicks
	do {
		int                count=[viewStack count];
		NSScreen          *screen=[self _screenForPoint:[[event window] convertBaseToScreen:point]];
		
		point=[[event window] convertBaseToScreen:point];
		
		// We've not pushed any views yet so the screen is where our window is
		if(count==1) {
			screen=[self _screenForPoint:point];
		}
                MENUDEBUG(@"INITIAL EVENT POINT IS %@ (count == %d, screen == %@)",
                        NSStringFromPoint(point), count, screen);
		
        if (keyboardNavigationAction == kNSMenuKeyboardNavigationNone) {
            // We're not current dealing with a keyboard event
            // Take a look at the visible menu stack (we're within a big loop so views can come and
            // go and the mouse can wander all over) deepest first
            while(--count>=0){
                
                // get the deepest one
                SubmenuView *checkView=[viewStack objectAtIndex:count];
                
                // And find out where the mouse is relative to it
                NSPoint     checkPoint=[[checkView window] convertScreenToBase:point];
                MENUDEBUG(@"1 CHECKPOINT IS %@", NSStringFromPoint(checkPoint));
                
                checkPoint=[checkView convertPoint:checkPoint fromView:nil];
                MENUDEBUG(@"2 CHECKPOINT IS %@", NSStringFromPoint(checkPoint));
                
                // If it's inside the menu view
                if(NSMouseInRect(checkPoint,[checkView bounds],[checkView isFlipped])){
                    
                    MENUDEBUG(@"found a menu: %@", checkView);
                    
                    // Which item is the cursor on top of?
                    unsigned itemIndex=[checkView itemIndexAtPoint:checkPoint];

                    MENUDEBUG(@"found an item index: %u", itemIndex);

                    // If it's not the currently selected item
                    if(itemIndex!=[checkView selectedItemIndex]){
                        SubmenuView *branch;
                        
                        // This looks like it's dealing with pushed cascading menu
                        // views that are no longer needed because the user has moved
                        // on - so pop them all off.
                        while (count+1<[viewStack count]) {
                            NSView* view = [viewStack lastObject];
                            MENUDEBUG(@"popping cascading view: %@", view);
                            [[view window] close];
                            [viewStack removeLastObject];
                        }
                        
                        // And now select the new item
                        MENUDEBUG(@"about to setSelectedItemIndex");
                        [checkView setSelectedItemIndex:itemIndex];
                        
                        // If it's got a cascading menu then push that on the stack
                        if((branch=[checkView viewAtSelectedIndexPositionOnScreen:screen])!=nil) {
                            MENUDEBUG(@"adding a new cascading view: %@", branch);
                            [viewStack addObject:branch];
                        }
                    }
                    // And bail out of the while loop - we're in the right place
                    break;
                } else {
                    // We've wandered off the menu so don't show anything selected if it's the deepest
                    // visible view
                    if (checkView == [viewStack lastObject]) {
                        MENUDEBUG(@"clearing selection in view: %@", checkView);
                        // The mouse is outside of the top menu - be sure no item is selected anymore
                        [checkView setSelectedItemIndex:NSNotFound];
                    }
                }
            }
            MENUDEBUG(@"exited --count>=0 loop");
            
            // Looks like we've popped everything so nothing can be selected
            if(count<0){
                MENUDEBUG(@"clearing all selection");
                [[viewStack lastObject] setSelectedItemIndex:NSNotFound];
            }
        }
		
		// Let's take a look at what's come in on the event queue
                MENUDEBUG(@"getting next window event for %@", [self window]);
		event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSMouseMovedMask|NSLeftMouseDraggedMask|NSKeyDownMask|NSAppKitDefinedMask];
		
		if (keyboardNavigationAction != kNSMenuKeyboardNavigationNone) {
			// We didn't enter the mouse handling loop that predecrements count - so do it here...
			count--;
		}
		// Reset the keyboard navigation state
		keyboardNavigationAction = kNSMenuKeyboardNavigationNone;
		
        // Sometimes we can get key events with no characters (if the user
        // has invoked an accelerator while the menu is open for example)
		if ([event type] == NSKeyDown && [[event characters] length] > 0) {

			NSString* chars = [event characters];
			unichar ch = [chars characterAtIndex: 0];
			switch (ch) {
				case NSUpArrowFunctionKey:
					keyboardNavigationAction = kNSMenuKeyboardNavigationUp;
					break;
				case NSDownArrowFunctionKey:
					keyboardNavigationAction = kNSMenuKeyboardNavigationDown;
					break;
				case NSLeftArrowFunctionKey:
					keyboardNavigationAction = kNSMenuKeyboardNavigationLeft;
					break;
				case NSRightArrowFunctionKey:
					keyboardNavigationAction = kNSMenuKeyboardNavigationRight;
					break;
					
				case '\r': // Return = select the current item and exit the loop
					MENUDEBUG(@"Selecting current item and exit");
					state = STATE_EXIT;
					break;
				case 27: // Escape = pop unless we're done then it's cancel
				{
					if ([viewStack count] > 1) {
						NSView* view = [viewStack lastObject];
						MENUDEBUG(@"popping cascading view: %@", view);
						[[view window] close];
						[viewStack removeLastObject];
					}	else {
						MENUDEBUG(@"Cancelling");
						cancelled = YES;
					}
				}
					break;
				default:
					keyboardNavigationAction = kNSMenuKeyboardNavigationLetter;
					break;
			}
			
			SubmenuView* activeMenuView = [viewStack lastObject];

			BOOL ignoreEnabledState = NO;
			if ([viewStack count] == 1 && [activeMenuView isKindOfClass: [NSMainMenuView class]]) {
				// For some reason main menu items are disabled - even though they work fine...
				ignoreEnabledState = YES;
				// we're navigating the top menu which has opposite semantics than a regular menu
				switch (keyboardNavigationAction) {
					case kNSMenuKeyboardNavigationDown:
						keyboardNavigationAction = kNSMenuKeyboardNavigationRight;
						break;
					case kNSMenuKeyboardNavigationUp:
						keyboardNavigationAction = kNSMenuKeyboardNavigationLeft;
						break;
					case kNSMenuKeyboardNavigationLeft:
						keyboardNavigationAction = kNSMenuKeyboardNavigationUp;
						break;
					case kNSMenuKeyboardNavigationRight:
						keyboardNavigationAction = kNSMenuKeyboardNavigationDown;
						break;
				}
			}
			
			switch (keyboardNavigationAction) {
				case kNSMenuKeyboardNavigationUp:
				{
					MENUDEBUG(@"Up...");
					
					unsigned oldIndex = [activeMenuView selectedItemIndex];
					NSArray *items = [activeMenuView itemArray];
					// Look for the next enabled item by search up and wrapping around the bottom
					unsigned newIndex = 0;
					if (oldIndex != NSNotFound) {
						newIndex = oldIndex == 0 ? [items count] - 1 : oldIndex - 1;
					}
					MENUDEBUG(@"oldIndex = %u", oldIndex);
					MENUDEBUG(@"newIndex = %u", newIndex);
					BOOL found = NO;
					while (!found && newIndex != oldIndex) {
						// Make sure we stop eventually
						if (oldIndex == NSNotFound) {
							oldIndex = 0;
						}
						// Try and find a new item to select
						NSMenuItem *item = [items objectAtIndex: newIndex];
						if ([item isSeparatorItem] == NO &&
							((ignoreEnabledState || [item isEnabled]) || [item hasSubmenu])) {
							MENUDEBUG(@"selecting item = %@", item);
							[activeMenuView setSelectedItemIndex: newIndex];
							found = YES;
						} else {
							MENUDEBUG(@"skipping item: %@", item);
							if (newIndex == 0) {
								newIndex = [items count] - 1;
							} else {
								newIndex --;
							}
						}
					}
				}
					break;
				case kNSMenuKeyboardNavigationDown:
				{
					MENUDEBUG(@"Down...");
					unsigned oldIndex = [activeMenuView selectedItemIndex];
					NSArray *items = [activeMenuView itemArray];
					// Look for the next enabled item by search down and wrapping around to the top
					unsigned newIndex = 0;
					if (oldIndex != NSNotFound) {
						newIndex = oldIndex == [items count] -1 ? 0 : oldIndex + 1;
					}
					
					MENUDEBUG(@"oldIndex = %u", oldIndex);
					MENUDEBUG(@"newIndex = %u", newIndex);
					BOOL found = NO;
					while (!found && newIndex != oldIndex) {
						// Make sure we stop eventually
						if (oldIndex == NSNotFound) {
							oldIndex = 0;
						}
						// Try and find a new item to select
						NSMenuItem *item = [items objectAtIndex: newIndex];
						if ([item isSeparatorItem] == NO &&
							((ignoreEnabledState || [item isEnabled]) || [item hasSubmenu])) {
							MENUDEBUG(@"selecting item: %u", item);
							[activeMenuView setSelectedItemIndex: newIndex];
							found = YES;
						} else {
							MENUDEBUG(@"skipping item: %@", item);
							if (newIndex == [items count] - 1) {
								newIndex = 0;
							} else {
								newIndex++;
							}
						}
					}
				}
					break;
				case kNSMenuKeyboardNavigationLeft:
					MENUDEBUG(@"Left...");
					if ([viewStack count] > 1) {
						NSView* view = [viewStack lastObject];
						MENUDEBUG(@"popping cascading view: %@", view);
						[[view window] close];
						[viewStack removeLastObject];
					}	
					break;
				case kNSMenuKeyboardNavigationRight:
                {
						MENUDEBUG(@"Right...");
					SubmenuView *branch = nil;
					// If there's a submenu at the current  selected index
					if((branch=[activeMenuView viewAtSelectedIndexPositionOnScreen:screen])!=nil) {
						MENUDEBUG(@"adding a new cascading view: %@", branch);
						[viewStack addObject:branch];
					} else {
						// We'll pop it - they're trying to navigate to the next menu most likely
						if ([viewStack count] > 1) {
							NSView* view = [viewStack lastObject];
							MENUDEBUG(@"popping cascading view: %@", view);
							[[view window] close];
							[viewStack removeLastObject];
						}
					}
                }
					break;
				case kNSMenuKeyboardNavigationLetter:
				{
					MENUDEBUG(@"Letter...");
					NSString *letterString = [[NSString stringWithCharacters: &ch length: 1] uppercaseString];
					unsigned oldIndex = [activeMenuView selectedItemIndex];
					NSArray *items = [activeMenuView itemArray];
					// Look for the next enabled item by search down and wrapping around to the top
					unsigned newIndex = 0;
					if (oldIndex != NSNotFound) {
						newIndex = oldIndex == [items count] -1 ? 0 : oldIndex + 1;
					}
					
					MENUDEBUG(@"oldIndex = %u", oldIndex);
					MENUDEBUG(@"newIndex = %u", newIndex);
					BOOL found = NO;
					while (!found && newIndex != oldIndex) {
						// Make sure we stop eventually
						if (oldIndex == NSNotFound) {
							oldIndex = 0;
						}
						// Try and find a new item to select
						NSMenuItem *item = [items objectAtIndex: newIndex];
						if ((ignoreEnabledState || [item isEnabled] == YES) || [item hasSubmenu] == YES) {
							NSRange range = [[item title] rangeOfString: letterString];
							if (range.location != NSNotFound) {
								MENUDEBUG(@"selecting item: %u", item);
								[activeMenuView setSelectedItemIndex: newIndex];
								found = YES;
							}
						}
						if (!found) {
							MENUDEBUG(@"skipping item: %@", item);
							if (newIndex == [items count] - 1) {
								newIndex = 0;
							} else {
								newIndex++;
							}
						}
					}
				}
					break;
			}
		}
		// We use this special AppKitDefined event to let the menu respond to the app deactivation - it *has*
		// to be passed through the event system, unfortunately
		if ([event type] == NSAppKitDefined) {
			if ([event subtype] == NSApplicationDeactivated) {
				MENUDEBUG(@"NSApplicationDeactivated");
				cancelled = YES;
			}
		}
		
		if (cancelled == NO && [event type] != NSAppKitDefined && [event type] != NSKeyDown) {
			
			// looks like we can keep rolling
			
			point=[event locationInWindow];
                        MENUDEBUG(@"1 SECOND POINT is now %@", NSStringFromPoint(point));
                        point = [[event window] convertScreenToBase:point];
                        MENUDEBUG(@"2 SECOND POINT is now %@", NSStringFromPoint(point));
                        point.y += [screen frame].size.height;
                        MENUDEBUG(@"3 SECOND POINT is now %@", NSStringFromPoint(point));

			// Don't test for "== 0." - we tend to receive some delta with some .000000... values while the mouse doesn't move
			BOOL mouseMoved = ([event type] != NSAppKitDefined) &&
								(fabs([event deltaX]) > kMouseMovementThreshold || fabs([event deltaY]) > kMouseMovementThreshold);
			
			SubmenuView * activeView = nil;

			// We may not have a menuview here - so be cautious - and we may have added a cascading menu
			// so lastObject is also not the right thing to look at - we need to look at the menuview found in
			// the preceeding block (if there was one found - the user could have moused somewhere else entirely
			// remember)
			if (count >= 0) {
				activeView = [viewStack objectAtIndex: count];
                                MENUDEBUG(@"activeView == %@ at index %d", activeView, count);
			}
			
                        MENUDEBUG(@"switch(%d)", state);
			switch(state){
				case STATE_FIRSTMOUSEDOWN:
					// Let's take a look at the item under the cursor (if there is one)
					item=[activeView itemAtSelectedIndex];
                                        MENUDEBUG(@"itemAtSelectedIndex == %@ for index %d", item, _selectedItemIndex);
					
					if([event type]==NSLeftMouseUp) {
						// The menu is really active after a mouse up (which means the menu will stay sticky)...
						// The timestamp is to avoid false clicks - make sure there's a delay so the user can
						if ([event timestamp] - firstTimestamp > kMenuInitialClickThreshold &&
							[viewStack count]==1 && [item isEnabled]) {
							MENUDEBUG(@"Handling selected item - exiting");
							state=STATE_EXIT;
						} else {
							MENUDEBUG(@"mouse up - continuing");
							state=STATE_MOUSEUP;
						}
					} else if([event type]==NSLeftMouseDown || mouseMoved) {
						// .. Or a mouse down (second click after the sticky menu) or a real move
						state=STATE_MOUSEDOWN;
					}
					break;
					
				default:
					item=[activeView itemAtSelectedIndex];
					if([event type]==NSLeftMouseUp){
						MENUDEBUG(@"mouseUp on item: %@", item);
						if(item == nil || ([viewStack count]<=2) || [item isEnabled]) {
							MENUDEBUG(@"mouse up - exiting because of many possible reasons...");
							state=STATE_EXIT;
						} else {
							MENUDEBUG(@"mouse up");
							state=STATE_MOUSEUP;
						}
					}
					break;
			}
                        point = [event locationInWindow];
                        MENUDEBUG(@"reset point!");
		}
	}while(cancelled == NO && state!=STATE_EXIT);
	
	MENUDEBUG(@"done with the event loop");
	
	// If we've got a menu still visible
	if(item == nil && [viewStack count]>0) {
		// Get the selected item
		item=[[viewStack lastObject] itemAtSelectedIndex];
		MENUDEBUG(@"got the selected item at the top most menu view: %@", item);
	}
	
	MENUDEBUG(@"removing the visible menu views");
	while([viewStack count]>1){
                NSView *v = (NSView *)[viewStack lastObject];
                NSWindow *w = [v window];
                MENUDEBUG(@"closing window %@ for view %@", w, v);
		[[(NSView *)[viewStack lastObject] window] close];
		[viewStack removeLastObject];
	}
	[viewStack removeLastObject];
	
	_selectedItemIndex=NSNotFound;
	[[self window] setAcceptsMouseMovedEvents:oldAcceptsMouseMovedEvents];
	[self setNeedsDisplay:YES];
	
	return ([item isEnabled])?item:(NSMenuItem *)nil;
}

-(void)mouseDown:(NSEvent *)event {
   BOOL        didAccept=[[self window] acceptsMouseMovedEvents];
   NSMenuItem *item;

   [[self window] setAcceptsMouseMovedEvents:YES];
   item=[self trackForEvent:event];
   [[self window] setAcceptsMouseMovedEvents:didAccept];
   
   if(item!=nil)
    [NSApp sendAction:[item action] to:[item target] from:item];
}

@end
