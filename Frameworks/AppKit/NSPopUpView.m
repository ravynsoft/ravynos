/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSPopUpView.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSGraphicsStyle.h>

enum {
    KEYBOARD_INACTIVE,
    KEYBOARD_ACTIVE,
    KEYBOARD_OK,
    KEYBOARD_CANCEL
};

// TODO : NSPopUpView should at least inherit from NSMenuView - we have plenty of common code here

#define ITEM_MARGIN 2

@implementation NSPopUpView


// If the user clicks and holds on a popup menu and then releases
// the app should just dismiss the popup and reselect the current value.
// If the user clicks and immediately releases the popup menu should remain
// on screen. This threshold is the dividing line between those two behaviours.
static const float kMenuInitialClickThreshold = .3f;

#define MIN_TITLE_KEY_GAP 8
#define WINDOW_BORDER_THICKNESS 3
#define IMAGE_TITLE_GAP 8

// Note: moved these above init to avoid compiler warnings
-(NSDictionary *)itemAttributes {
    return [NSDictionary dictionaryWithObjectsAndKeys:
            _font,NSFontAttributeName,
            nil];
}

-(NSArray *)visibleItemArray
{
    NSArray * items = [[self menu] itemArray];
    
    NSMutableArray *visibleArray = [[[NSMutableArray init] alloc] autorelease];
	
    for(NSMenuItem *item in items) {
        if( ![item isHidden]) {
            [visibleArray addObject: item];
        }
    }
    
    return visibleArray;
}

-(NSSize)sizeForMenuItemImage:(NSMenuItem *)item {
	NSSize result=NSZeroSize;
	
	if([item image]!=nil)
		result=[[item image] size];
	
	return result;
}

-(NSSize)sizeForMenuItems:(NSArray *)items {
	NSSize   result=NSZeroSize;
	float    maxTitleWidth = 0.0f;
	BOOL     anItemHasAnImage = NO;
	float    maxKeyWidth = 0.0f;
	float    totalHeight = WINDOW_BORDER_THICKNESS; // border. Magic constants that may not be right for Win7 vs XP
	NSSize   gutterSize = [[self graphicsStyle] menuItemGutterSize];
	NSSize   rightArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
	unsigned i,count=[items count];
	NSRect   rects[count];
    
    BOOL useCustomFont = _font != nil;
    if (useCustomFont) {
        // If we have the default font, then really use the default menu one instead of forcing it
        if ([_font isEqual:[NSFont fontWithName:@"Nimbus Sans-Regular" size:12.]]) {
            useCustomFont = NO;
        }
    }

	for (i = 0;i<count;i++)
	{
		NSMenuItem *item = [items objectAtIndex:i];
        float height =  0.;
        float      titleAndIconWidth = self.bounds.size.width;
		if ([item isSeparatorItem])
		{
            height = [[self graphicsStyle] menuItemSeparatorSize].height;
		}
		else
		{
			NSSize     size = NSZeroSize;
			
			size = [self sizeForMenuItemImage:item];
			height = MAX(height,size.height);
			if ((titleAndIconWidth = size.width) > 0.0f)
				anItemHasAnImage = YES;
            
            if ([item attributedTitle]) {
                size = [[self graphicsStyle] menuItemAttributedTextSize:[item attributedTitle]];
            } else {
                if (useCustomFont) {
                    NSDictionary *attributes = [self itemAttributes];
                    NSAttributedString *attributedTitle = [[[NSAttributedString alloc] initWithString:[item title] attributes:attributes] autorelease];
                    size = [[self graphicsStyle] menuItemAttributedTextSize:attributedTitle];
                } else {
                    size = [[self graphicsStyle] menuItemTextSize:[item title]];
                }
            }
			titleAndIconWidth += size.width;
			maxTitleWidth = MAX(maxTitleWidth,titleAndIconWidth);
			height = MAX(height,size.height);
			
			if ([[item keyEquivalent] length] != 0)
			{
				size = [[self graphicsStyle] menuItemTextSize:[item _keyEquivalentDescription]];
				maxKeyWidth=MAX(maxKeyWidth,size.width);
				height = MAX(height,size.height);
			}
			height = MAX(height,gutterSize.height);
			height = MAX(height,rightArrowSize.height);
		}
        rects[i] = NSMakeRect(0, totalHeight, titleAndIconWidth, height);
        totalHeight += height;
	}
	
	result.height = totalHeight;
	result.width = gutterSize.width;
	result.width += [[self graphicsStyle] menuItemGutterGap];
	if (anItemHasAnImage)
		result.width += IMAGE_TITLE_GAP;
	result.width += maxTitleWidth;
	if (maxKeyWidth > 0.0f)
	{
		result.width += MIN_TITLE_KEY_GAP;
		result.width += maxKeyWidth;
	}
	result.width += rightArrowSize.width;
	
	// Add the left+right and bottom borders
	result.width += WINDOW_BORDER_THICKNESS*2;
	result.height += WINDOW_BORDER_THICKNESS;

    // Normalize the items widths to the pulldown or menu width,
    // whichever is wider
	result.width = MAX(_cellSize.width, result.width);
	
    if (_cachedItemRects == nil && [items isEqual:[self visibleItemArray]]) {
        // Build our cached item rects
        _cachedItemRects = [[NSMutableArray arrayWithCapacity: count] retain];
        for (int i = 0; i < count; ++i) {
            rects[i].size.width = result.width;
            [_cachedItemRects addObject:[NSValue valueWithRect:rects[i]]];
        }
    }
	return result;
}

-initWithFrame:(NSRect)frame {
    [super initWithFrame:frame];
    _cellSize=frame.size;
    _font=[[NSFont messageFontOfSize:12] retain];
    _selectedIndex=-1;
    _pullsDown=NO;
    
    NSSize sz = [@"ABCxyzgjX" sizeWithAttributes: [self itemAttributes] ];
    _cellSize.height = sz.height + ITEM_MARGIN + ITEM_MARGIN;
    
    return self;
}

-(void)dealloc {
	[_cachedItemRects release];
    [_font release];
    [super dealloc];
}

-(BOOL)isFlipped {
    return YES;
}

-(void)setFont:(NSFont *)font {
    [_font autorelease];
    _font=[font retain];
}

-(BOOL)pullsDown {
    return _pullsDown;
}

-(void)setPullsDown:(BOOL)pullsDown {
    _pullsDown=pullsDown;
}

-(void)selectItemAtIndex:(NSInteger)index {
    _selectedIndex=index;
    _initialSelectedIndex=index;
}

-(NSSize)sizeForContents {
    return [self sizeForMenuItems:[self visibleItemArray]];
}

- (void)_buildCachedItemRects
{
    // Force a build of the cached item rects
    [self sizeForMenuItems:[self visibleItemArray]];
}

// For attributed strings - precalcing the the item rects makes performance much faster.
- (NSArray*)_cachedItemRects
{
	if (_cachedItemRects == nil) {
		[self _buildCachedItemRects];
	}
	return _cachedItemRects;
}

-(NSRect)rectForItemAtIndex:(NSInteger)index {
	NSRect result = [[[self _cachedItemRects] objectAtIndex: index] rectValue];
    // Be sure we cover the full view width
    result.size.width = _cellSize.width; //self.bounds.size.width;
    return result;
}

-(NSRect)rectForSelectedItem {
    NSRect r;
    if(_pullsDown) {
        r= NSZeroRect;
    } else if(_selectedIndex==-1) {
        r= [self rectForItemAtIndex:0];
    } else {
        r= [self rectForItemAtIndex:_selectedIndex];
    }
    return r;
}

static NSRect boundsToTitleAreaRect(NSRect rect){
    return NSInsetRect(rect, WINDOW_BORDER_THICKNESS, WINDOW_BORDER_THICKNESS);
}

-(void)drawRect:(NSRect)rect
{
	NSRect   bounds = [self bounds];
	NSRect   itemArea=boundsToTitleAreaRect(bounds);
	NSArray *items=[self visibleItemArray];
	unsigned i,count=[items count];
	NSPoint  origin=itemArea.origin;
	
    BOOL useCustomFont = _font != nil;
    if (useCustomFont) {
        // If we have the default font, then really use the default menu one instead of forcing it
        if ([_font isEqual:[NSFont fontWithName:@"Nimbus Sans-Regular" size:12.]]) {
            useCustomFont = NO;
        }
    }
    
	[[self graphicsStyle] drawMenuWindowBackgroundInRect:self.bounds];
	
	for(i=0;i<count;i++)
	{
		NSMenuItem *item=[items objectAtIndex:i];
		NSRect itemRect = [self rectForItemAtIndex:i];
        if (NSIntersectsRect(rect, itemRect) == NO) {
            continue;
        }
        itemArea = itemRect;
        origin = itemRect.origin;
        
		if ([item isSeparatorItem])
		{
			NSRect separatorRect = NSMakeRect(origin.x,origin.y,NSWidth(itemArea),[[self graphicsStyle] menuItemSeparatorSize].height);
			
			[[self graphicsStyle] drawMenuSeparatorInRect:separatorRect];
			
			origin.y += NSHeight(separatorRect);
		}
		else
		{
#define CENTER_PART_RECT_VERTICALLY(partSize)                          \
{                                                                      \
NSSize __partSize = (partSize);                                    \
__partSize.width = ceilf(__partSize.width); \
partRect.origin.y = origin.y + (itemHeight - __partSize.height) / 2; \
partRect.size.height = __partSize.height;                            \
partRect.size.width = __partSize.width;                              \
}
			NSImage      *image = [item image];
			BOOL         selected = (i ==_selectedIndex) ? YES : NO;
			float        itemHeight = itemRect.size.height;
			NSRect       partRect;
			NSSize       partSize;
			BOOL         showsEnabled = ([item isEnabled] || [item hasSubmenu]);
			
			partRect = NSMakeRect(origin.x,origin.y,itemArea.size.width,itemHeight);
            
			if (selected)
				[[self graphicsStyle] drawMenuSelectionInRect:partRect enabled:showsEnabled];
            
			// Draw the gutter and checkmark (if any)
			CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemGutterSize]);
			if ([item state] || _initialSelectedIndex == i)
			{
				[[self graphicsStyle] drawMenuGutterInRect:partRect];
				[[self graphicsStyle] drawMenuCheckmarkInRect:partRect enabled:showsEnabled selected:selected];
			}
			
			partRect.origin.x += [[self graphicsStyle] menuItemGutterGap];
			
			// Draw the image
			if (image)
			{
				NSRect imageRect;
				
				partRect.origin.x += partRect.size.width;
				CENTER_PART_RECT_VERTICALLY([image size]);
				
                CGContextRef ctx=[[NSGraphicsContext currentContext] graphicsPort];
                CGContextSaveGState(ctx);
                CGContextTranslateCTM(ctx,partRect.origin.x,partRect.origin.y);
                if([self isFlipped]) {
                    CGContextTranslateCTM(ctx,0,partRect.size.height);
                    CGContextScaleCTM(ctx,1,-1);
                }
                NSRect drawingRect = partRect;
                drawingRect.origin = NSZeroPoint;
                [[self graphicsStyle] drawButtonImage:image inRect:drawingRect enabled:showsEnabled mixed:NO];
                CGContextRestoreGState(ctx);
                
				partRect.origin.x += IMAGE_TITLE_GAP;
			}
			
			// Draw the title
			partRect.origin.x += partRect.size.width;
            
			NSAttributedString     *atitle = [item attributedTitle];
			if (atitle != nil && [atitle length] > 0) {
				CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemAttributedTextSize:atitle]);
#if WINDOWS
                // On Windows, when using the AGG graphics context, enabling font smoothing switches to using AGG for text drawing,
                // instead of the native Win32 API.
                // In case we're using a lot of different fonts, like for a styled for menu, that's leading to much faster drawing
                // thanks to fonts caching done by this context.
                // However, drawing is more smoothed that when traditional Win32 text rendering, leading to a different look than
                // standard Win32 menus - so we'll do that only if we're just drawing "normal" text, without any special attributes
                // It would probably be better to add font caching to the regular Win32 drawing context
                CGContextSetShouldSmoothFonts((CGContextRef)[[NSGraphicsContext currentContext] graphicsPort], YES);
#endif
				[[self graphicsStyle] drawAttributedMenuItemText:atitle inRect:partRect enabled:showsEnabled selected:selected];
			} else {
#if WINDOWS
                // On Windows, when using the AGG graphics context, enabling font smoothing switches to using AGG for text drawing,
                // instead of the native Win32 API.
                // In case we're using a lot of different fonts, like for a styled for menu, that's leading to much faster drawing
                // thanks to fonts caching done by this context.
                // However, drawing is more smoothed that when traditional Win32 text rendering, leading to a different look than
                // standard Win32 menus - so we'll do that only if we're just drawing "normal" text, without any special attributes
                // It would probably be better to add font caching to the regular Win32 drawing context
                CGContextSetShouldSmoothFonts((CGContextRef)[[NSGraphicsContext currentContext] graphicsPort], NO);
#endif
                NSString     *title = [item title];
                if (useCustomFont) {
                    NSDictionary *attributes = [self itemAttributes];
                    NSAttributedString *attributedTitle = [[[NSAttributedString alloc] initWithString:title attributes:attributes] autorelease];
                    CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemAttributedTextSize:attributedTitle]);
                    [[self graphicsStyle] drawAttributedMenuItemText:attributedTitle inRect:partRect enabled:showsEnabled selected:selected];
                } else {
                    CENTER_PART_RECT_VERTICALLY([[self graphicsStyle] menuItemTextSize:title]);
                    [[self graphicsStyle] drawMenuItemText:title inRect:partRect enabled:showsEnabled selected:selected];
                }
			}
			
			// Draw the key equivalent
			if ([[item keyEquivalent] length] != 0)
			{
				NSString *keyString = [item _keyEquivalentDescription];
				NSSize   branchArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
				NSSize   keyEquivalentSize = [[self graphicsStyle] menuItemTextSize:keyString];
				
				partRect.origin.x = origin.x + NSWidth(itemArea) - branchArrowSize.width - keyEquivalentSize.width;
				CENTER_PART_RECT_VERTICALLY(keyEquivalentSize);
				[[self graphicsStyle] drawMenuItemText:keyString inRect:partRect enabled:showsEnabled selected:selected];
			}
			
			// Draw the submenu arrow
			if([item hasSubmenu])
			{
				NSSize branchArrowSize = [[self graphicsStyle] menuItemBranchArrowSize];
				partRect.origin.x = origin.x + NSWidth(itemArea) - branchArrowSize.width;
				partRect.size.width = branchArrowSize.width;
				CENTER_PART_RECT_VERTICALLY(branchArrowSize);
				[[self graphicsStyle] drawMenuBranchArrowInRect:partRect enabled:showsEnabled selected:selected];
			}
			
			origin.y += itemHeight;
		}
	}
}
/*
 * Returns NSNotFound if the point is outside of the menu
 * Returns -1 if the point is inside the menu but on a disabled or separator item
 * Returns a usable index if the point is on a selectable item
 */
-(NSInteger)itemIndexForPoint:(NSPoint)point {
	NSInteger result;
	NSArray * items=[self visibleItemArray];
	int i, count = [items count];
	
	point.y-=2;
	
	if( point.y < 0 ) {
		return NSNotFound;
	}
	
	for( i = 0; i < count; i++ )
	{
		if( [[items objectAtIndex: i] isHidden ] )
			continue;
		
		NSRect itemRect = [self rectForItemAtIndex: i];
		if (NSPointInRect( point, itemRect)) {
			if (([[items objectAtIndex: i] isEnabled] == NO) ||
				([[items objectAtIndex: i] isSeparatorItem])) {
				return -1;
			}
			return i;
		}
	}
	return NSNotFound;
}


-(void)rightMouseDown:(NSEvent *)event {
    // do nothing
}

- (void)updateSelectedIndex:(int)index
{
    if(_selectedIndex!=index){
        NSInteger previous=_selectedIndex;
        _selectedIndex=index;
        if (previous != _selectedIndex) {
            if (previous != -1) {
                NSRect itemRect = [self rectForItemAtIndex: previous];
                [self setNeedsDisplayInRect: itemRect];
            }
            if (_selectedIndex != -1) {
                NSRect itemRect = [self rectForItemAtIndex: _selectedIndex];
                [self setNeedsDisplayInRect: itemRect];
            }
        }
    }
}
/*
 * This method may return NSNotFound when the view positioned outside the initial tracking area due to preferredEdge settings and the user clicks the mouse.
 * The NSPopUpButtonCell code deals with it. It might make sense for this to return the previous value.
 */
-(NSInteger)runTrackingWithEvent:(NSEvent *)event {
	enum {
        STATE_FIRSTMOUSEDOWN,
        STATE_MOUSEDOWN,
        STATE_MOUSEUP,
        STATE_EXIT
    } state=STATE_FIRSTMOUSEDOWN;
    NSPoint firstLocation,point=[event locationInWindow];
    NSInteger initialSelectedIndex = _selectedIndex;
    firstLocation = point;
    
    // Make sure we get mouse moved events, too, so we can respond apporpiately to
    // click-click actions as well as of click-and-drag
    BOOL oldAcceptsMouseMovedEvents = [[self window] acceptsMouseMovedEvents];
    [[self window] setAcceptsMouseMovedEvents:YES];
    
    // point comes in on controls window
    point=[[event window] convertBaseToScreen:point];
    point=[[self window] convertScreenToBase:point];
    point=[self convertPoint:point fromView:nil];
    
	// Make sure we know if the user clicks away from the app in the middle of this
	BOOL cancelled = NO;
	
    NSRect   screenVisible = NSInsetRect([[[self window] screen] visibleFrame],0,4);

    [NSEvent startPeriodicEventsAfterDelay:.0 withPeriod:.02];
    BOOL mouseMoved = NO;

    NSTimeInterval firstTimestamp = 0.;
    do {
        NSInteger index=[self itemIndexForPoint:point];
        
        /*
         If the popup is activated programmatically with performClick: index may be NSNotFound because the mouse starts out
         outside the view. We don't change _selectedIndex in this case.
         */
        if(index!= NSNotFound && _keyboardUIState == KEYBOARD_INACTIVE){
            [self updateSelectedIndex:index];
        }
        
        event=[[self window] nextEventMatchingMask:NSPeriodicMask|NSLeftMouseUpMask|NSMouseMovedMask|NSLeftMouseDraggedMask|NSKeyDownMask|NSAppKitDefinedMask];
        if (firstTimestamp == 0.) {
            // Note: we don't do that using the first event, because in case the menu takes a long of time to display, the next event
            // we get will be actually after our "long click" threshold
            firstTimestamp = [event timestamp];
        }
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
        point=[NSEvent mouseLocation];
        if (mouseMoved == NO) {
            mouseMoved = ABS(point.x-firstLocation.x) > 2. || ABS(point.y-firstLocation.y) > 2.;
        }


        if (NSPointInRect(point,[[self window] frame])) {
            if (!NSPointInRect(point,screenVisible)){
//              The point is inside the menu, and on the top or bottom border of the screen - let's autoscroll
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
        } else {
            [self updateSelectedIndex:-1];
        }
        // Convert the global point to view coordinates
        point=[[self window] convertScreenToBase:point];
        point=[self convertPoint:point fromView:nil];

        if ([event type] == NSPeriodic) {
            // Periodic events are just there for autoscroll so we're done with this one
            continue;
        }
        
        switch(state){
            case STATE_FIRSTMOUSEDOWN:
                if([event type]==NSLeftMouseUp) {
                    if(mouseMoved || [event timestamp] - firstTimestamp > kMenuInitialClickThreshold) {
                        // Long click - accept the selection
                        state=STATE_EXIT;
                    } else {
                        // Short click - let's keep the menu sticky
                        state=STATE_MOUSEUP;
                    }
                } else {
                    if (mouseMoved) {
                        state=STATE_MOUSEDOWN;   
                    }
                }
                break;
                
            default:
                if([event type]==NSLeftMouseUp) {
                    // If the user clicked outside of the window - then they want
                    // to dismiss it without changing anything
                    NSPoint winPoint=[event locationInWindow];
                    winPoint=[[event window] convertBaseToScreen:winPoint];
                    if (NSPointInRect(winPoint,[[self window] frame]) == NO) {
                        [self updateSelectedIndex:-1];
                    }
                    state=STATE_EXIT;
                }
                break;
        }
    }while(cancelled == NO && state!=STATE_EXIT);
    [NSEvent stopPeriodicEvents];
    
    [[self window] setAcceptsMouseMovedEvents: oldAcceptsMouseMovedEvents];

    _keyboardUIState = KEYBOARD_INACTIVE;
    
	return (_selectedIndex == -1) ? NSNotFound : _selectedIndex;
}

- (void)keyDown:(NSEvent *)event {
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (void)moveUp:(id)sender {
    NSInteger previous = _selectedIndex;
    
	// Find the previous visible item
	NSArray *items = [self visibleItemArray];
	if( _selectedIndex == -1 )
		_selectedIndex = [items count];
    
	do
	{
		_selectedIndex--;
	} while( (int)_selectedIndex >= 0 && ( [[items objectAtIndex: _selectedIndex] isHidden] ||
                                          [[items objectAtIndex: _selectedIndex] isSeparatorItem] ) );
	
    if ((int)_selectedIndex < 0)
        _selectedIndex = previous;
    
    [self setNeedsDisplay:YES];
}

- (void)moveDown:(id)sender {
    NSInteger previous = _selectedIndex;
    
	// Find the next visible item
	NSArray *items = [self visibleItemArray];
	NSInteger searchIndex = _selectedIndex;
    
	do
	{
		searchIndex++;
	} while( searchIndex < [items count] && ( [[items objectAtIndex: searchIndex] isHidden] ||
                                             [[items objectAtIndex: searchIndex] isSeparatorItem] )  );
    
	if (searchIndex >= [items count]) {
        _selectedIndex = previous;
	} else {
		_selectedIndex = searchIndex;
	}
	
    [self setNeedsDisplay:YES];
}

- (void)cancel:(id)sender {
    _keyboardUIState = KEYBOARD_CANCEL;
}

- (void)insertNewline:(id)sender {
    _keyboardUIState = KEYBOARD_OK;
}

- (void)insertText:(id)aString {
	
	// We're intercepting insertText: so we can do menu navigation by letter
	unichar ch = [aString characterAtIndex: 0];
	NSString *letterString = [[NSString stringWithCharacters: &ch length: 1] uppercaseString];
    
    NSInteger oldIndex = _selectedIndex;
    
	NSArray *items = [self visibleItemArray];
	NSInteger newIndex = _selectedIndex;
	
	// Set to the next item in the array or the start if we're at the end or there's no selection
	if (oldIndex == NSNotFound || oldIndex == [items count] - 1) {
		newIndex = 0;
	} else {
		newIndex = oldIndex + 1;
	}
	
	// Find the next visible item that has a title with an uppercase letter matching what the user
	// entered (who knows what this means in Japan...)
	BOOL found = NO;
	while (!found && newIndex != oldIndex) {
		// Make sure we stop eventually
		if (oldIndex == NSNotFound || oldIndex == -1) {
			oldIndex = 0;
		}
		// Try and find a new item to select
		NSMenuItem *item = [items objectAtIndex: newIndex];
		if ([item isEnabled] == YES && [item isSeparatorItem] == NO) {
			NSRange range = [[item title] rangeOfString: letterString];
			if (range.location != NSNotFound) {
				_selectedIndex =  newIndex;
				found = YES;
			}
		}
		if (!found) {
			if (newIndex == [items count] - 1) {
				newIndex = 0;
			} else {
				newIndex++;
			}
		}
	}
	if (newIndex != oldIndex) {
		[self setNeedsDisplay:YES];
	}
}
@end
