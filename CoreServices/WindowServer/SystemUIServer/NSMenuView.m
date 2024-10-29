/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSMenuView.h"
#import "NSMenuWindow.h"
#import "NSMainMenuView.h"
#import <AppKit/NSRaise.h>

enum {
	kNSMenuKeyboardNavigationNone,
	kNSMenuKeyboardNavigationUp,
	kNSMenuKeyboardNavigationDown,
	kNSMenuKeyboardNavigationLeft,
	kNSMenuKeyboardNavigationRight,
	kNSMenuKeyboardNavigationLetter
};

@implementation NSMenuView

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

-(NSMenuItem *)itemAtSelectedIndex {
   NSArray *items=[self visibleItemArray];

   if(_selectedItemIndex<[items count])
    return [items objectAtIndex:_selectedItemIndex];

   return nil;
}

-(NSMenuView *)viewAtSelectedIndexPositionOnScreen:(NSScreen *)screen {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)rightMouseDown:(NSEvent *)event {
   // do nothing
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

#if 1
#define MENUDEBUG(...) NSLog(__VA_ARGS__)
#else
#define MENUDEBUG(...)
#endif

// This threshold is not really applicable with regular
// menus - but in the event that the popup and regular menu
// logic is merged the behaviour as been replicated here.
// If a user clicks and releases on a menu it should remain
// visible. If a user clicks and holds for a period and then releases
// the current item should be reselected. This threshold is the dividing
// line between those two behaviours.
const float kMenuInitialClickThreshold = .3f;
const float kMouseMovementThreshold = .001f;

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
	
	//[event retain];
	
	int keyboardNavigationAction = kNSMenuKeyboardNavigationNone;
	
	BOOL cancelled = NO;
	
	MENUDEBUG(@"entering outer loop");
	
	// Start tracking the mouse movements and clicks
	do {
		// Lots of objects are going to come and go as we track the mouse
		// so a tactical autorelease pool keeps a lid on things
		NSAutoreleasePool *pool=[NSAutoreleasePool new];
		int                count=[viewStack count];
		NSScreen          *screen=[self _screenForPoint:[[event window] convertBaseToScreen:point]];
		
		point=[[event window] convertBaseToScreen:point];
                MENUDEBUG(@"INITIAL EVENT POINT IS %@", NSStringFromPoint(point));
		
		// We've not pushed any views yet so the screen is where our window is
		if(count==1) {
			screen=[self _screenForPoint:point];
		}
		
        if (keyboardNavigationAction == kNSMenuKeyboardNavigationNone) {
            // We're not current dealing with a keyboard event
            // Take a look at the visible menu stack (we're within a big loop so views can come and
            // go and the mouse can wander all over) deepest first
            while(--count>=0){
                
                // get the deepest one
                NSMenuView *checkView=[viewStack objectAtIndex:count];
                
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
                        NSMenuView *branch;
                        
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
                        MENUDEBUG(@"pushing new branch with viewAtSelectedIndexPositionOnScreen:%@", screen);
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
		
		//[event release];
		
		// Let's take a look at what's come in on the event queue
                MENUDEBUG(@"getting next window event for %@", [self window]);
		event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSMouseMovedMask|NSLeftMouseDraggedMask|NSKeyDownMask|NSAppKitDefinedMask];
		//[event retain];
		
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
			
			NSMenuView* activeMenuView = [viewStack lastObject];

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
					NSMenuView *branch = nil;
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
			
			NSMenuView* activeView = nil;

			// We may not have a menuview here - so be cautious - and we may have added a cascading menu
			// so lastObject is also not the right thing to look at - we need to look at the menuview found in
			// the preceeding block (if there was one found - the user could have moused somewhere else entirely
			// remember)
			if (count >= 0) {
				activeView = [viewStack objectAtIndex: count];
			}
			
			switch(state){
				case STATE_FIRSTMOUSEDOWN:
					// Let's take a look at the item under the cursor (if there is one)
					item=[activeView itemAtSelectedIndex];
					
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
		//[pool release];
	}while(cancelled == NO && state!=STATE_EXIT);
	//[event release];
	
	MENUDEBUG(@"done with the event loop");
	
	// If we've got a menu still visible
	if(item == nil && [viewStack count]>0) {
		// Get the selected item
		item=[[viewStack lastObject] itemAtSelectedIndex];
		MENUDEBUG(@"got the selected item at the top most menu view: %@", item);
	}
	
	MENUDEBUG(@"removing the visible menu views");
	while([viewStack count]>1){
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
