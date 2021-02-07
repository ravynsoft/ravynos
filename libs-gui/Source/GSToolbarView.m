/*
   <Title>GSToolbarView.m</title>

   <abstract>The toolbar view class.</abstract>
   
   Copyright (C) 2004-2020 Free Software Foundation, Inc.

   Author:  Quentin Mathe <qmathe@club-internet.fr>
   Date: January 2004
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSString.h>
#import "AppKit/NSButton.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSDragging.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSPasteboard.h"
// It contains GSMovableToolbarItemPboardType declaration
#import "AppKit/NSToolbarItem.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"

#import "GNUstepGUI/GSTheme.h"
#import "GNUstepGUI/GSToolbarView.h"

#import "NSToolbarFrameworkPrivate.h"

typedef enum {
  ToolbarViewDefaultHeight = 62,
  ToolbarViewRegularHeight = 62,
  ToolbarViewSmallHeight = 52
} ToolbarViewHeight;

// Borrow this from  NSToolbarItem.m
static const int InsetItemViewX = 10;

static const int ClippedItemsViewWidth = 28;

static NSUInteger draggedItemIndex = NSNotFound;

/*
 * Toolbar related code
 */

@interface GSToolbarButton
- (NSToolbarItem *) toolbarItem;
@end

@interface GSToolbarBackView
- (NSToolbarItem *) toolbarItem;
@end

@interface GSToolbarClippedItemsButton : NSButton
{
  NSToolbar *_toolbar;
}

- (id) init;

// Accessors 
- (NSMenu *) overflowMenu; 
/* This method cannot be called "menu" otherwise it would override NSResponder
   method with the same name. */

- (void) layout;
- (void) setToolbar: (NSToolbar *)toolbar; 
@end

@implementation GSToolbarClippedItemsButton
- (id) init
{
  NSImage *image = [NSImage imageNamed: @"common_ToolbarClippedItemsMark"];
  NSRect dummyRect = NSMakeRect(0, 0, ClippedItemsViewWidth, 100);
  // The correct height will be set by the layout method
  
  if ((self = [super initWithFrame: dummyRect]) != nil) 
    {
      [self setBordered: NO];
      [[self cell] setHighlightsBy: NSChangeGrayCellMask 
        | NSChangeBackgroundCellMask];
      [self setAutoresizingMask: NSViewNotSizable];
      [self setImagePosition: NSImageOnly];
      [image setScalesWhenResized: YES];
      // [image setSize: NSMakeSize(20, 20)];
      [self setImage: image];
      return self;
    }
  return nil;
}

/* 
 * Not really used, it is here to be used by the developer who want to adjust
 * easily a toolbar view attached to a toolbar which is not bind to a window.
 */
- (void) layout 
{
  NSSize layoutSize = NSMakeSize([self frame].size.width, 
    [[_toolbar _toolbarView] _heightFromLayout]);

  [self setFrameSize: layoutSize];
}

- (void) mouseDown: (NSEvent *)event 
{
  NSMenu *clippedItemsMenu = [self menuForEvent: event];
   
  [super highlight: YES];
   
  if (clippedItemsMenu != nil)
    {
      [NSMenu popUpContextMenu: clippedItemsMenu withEvent: event 
              forView: self];
    }
    
  [super highlight: NO];
}

- (NSMenu *) menuForEvent: (NSEvent *)event 
{
  if ([event type] == NSLeftMouseDown)
    {
      return [self overflowMenu];
    }
  return nil;
}

- (NSMenu *) overflowMenu 
{
  /* This method cannot be called "menu" otherwise it would
     override NSResponder method with the same name. */
  NSMenu *menu = [[NSMenu alloc] initWithTitle: @""];
  NSEnumerator *e;
  id item;
  NSArray *visibleItems;
  
  visibleItems = [_toolbar visibleItems];

  e = [[_toolbar items] objectEnumerator];
  while ((item = [e nextObject]) != nil)
    {
      if (![visibleItems containsObject: item])
        {
          id menuItem;
      
          menuItem = [item menuFormRepresentation];
          if (menuItem == nil)
            menuItem = [item _defaultMenuFormRepresentation];
            
          if (menuItem != nil)
            {
              [item validate];
              [menu addItem: menuItem];
            }
        }
    }

  return AUTORELEASE(menu);
}

// Accessors

- (void) setToolbar: (NSToolbar *)toolbar
{
  // Don't do an ASSIGN here, the toolbar view retains us.
  _toolbar = toolbar;
}
@end

// ---

// Implementation GSToolbarView

@implementation GSToolbarView
+ (void) initialize
{
  if (self == [GSToolbarView class])
    {
    }
}

- (id) initWithFrame: (NSRect)frame
{
  if ((self = [super initWithFrame: frame]) == nil)
    {
      return nil;
    }
    
  _heightFromLayout = ToolbarViewDefaultHeight;
  [self setFrame: NSMakeRect(frame.origin.x, frame.origin.y, 
                             frame.size.width, _heightFromLayout)];
        
  _clipView = [[NSClipView alloc] initWithFrame: 
                                      NSMakeRect(0, 0, frame.size.width, 
                                                 _heightFromLayout)];
  [_clipView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  [_clipView setDrawsBackground: NO];
  [self addSubview: _clipView];
  // Adjust the clip view frame
  [self setBorderMask: GSToolbarViewTopBorder | GSToolbarViewBottomBorder 
        | GSToolbarViewRightBorder | GSToolbarViewLeftBorder]; 
  
  _clippedItemsMark = [[GSToolbarClippedItemsButton alloc] init];
  
  [self registerForDraggedTypes: 
            [NSArray arrayWithObject: GSMovableToolbarItemPboardType]];
  
  return self;
}

- (void) dealloc
{
  //NSLog(@"Toolbar view dealloc");
  
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  
  RELEASE(_clippedItemsMark);
  RELEASE(_clipView);

  [super dealloc];
}

// Dragging related methods

+ (NSUInteger) draggedItemIndex
{
  return draggedItemIndex;
}

+ (void) setDraggedItemIndex:(NSUInteger)sourceIndex
{
  draggedItemIndex = sourceIndex;
}

- (int) _insertionIndexAtPoint: (NSPoint)location
{
  NSUInteger index;
  NSArray *visibleBackViews = [self _visibleBackViews];

  location = [_clipView convertPoint:location fromView:nil];
  if (draggedItemIndex == NSNotFound)
    {
      //simply locate the nearest location between existing items
      for (index = 0; index < [visibleBackViews count]; index++)
        {
          NSRect itemRect = [[visibleBackViews objectAtIndex:index] frame];
          if (location.x < (itemRect.origin.x + (itemRect.size.width/2)))
            {
              NSLog(@"At location %lu", (unsigned long)index);
              return index;
            }
        }
      return [visibleBackViews count];
    }
  else
    {
      // don't return a different index unless drag has crossed the midpoint of its neighbor
      NSRect itemRect;
      BOOL draggingLeft = YES;
      if (draggedItemIndex < [visibleBackViews count])
        {
          itemRect = [[visibleBackViews objectAtIndex:draggedItemIndex] frame];
          draggingLeft = (location.x < (itemRect.origin.x + (itemRect.size.width/2)));
        }
      if (draggingLeft)
        {
          // dragging to the left of dragged item's current location
          for (index=0; index < draggedItemIndex; index++)
            {
              itemRect = [[visibleBackViews objectAtIndex:index] frame];
              if (location.x < (itemRect.origin.x + (itemRect.size.width/2)))
                {
                  return index;
                }
            }
        }
      else
        {
          // dragging to the right of current location
          // Never called for [visibleBackViews count] == 0
          for (index=[visibleBackViews count]-1; index > draggedItemIndex; index--)
            {
              itemRect = [[visibleBackViews objectAtIndex:index] frame];
              if (location.x > (itemRect.origin.x + (itemRect.size.width/2)))
                {
                  return index;
                }
            }
        }
      return draggedItemIndex;
    }
}

#define OUTSIDE_INDEX (NSNotFound - 1)

- (NSDragOperation) updateItemWhileDragging:(id <NSDraggingInfo>)info exited:(BOOL)exited
{
  NSToolbarItem *item = [[info draggingSource] toolbarItem];
  NSString *identifier = [item itemIdentifier];
  NSToolbar *toolbar = [self toolbar];
  NSArray *allowedItemIdentifiers = [toolbar _allowedItemIdentifiers];
  int newIndex; 
    
  // don't accept any dragging if the customization palette isn't running for this toolbar
  if (![toolbar customizationPaletteIsRunning] || ![allowedItemIdentifiers containsObject: identifier])
    {
      return NSDragOperationNone;
    }
	
  if (draggedItemIndex == NSNotFound) // initialize the index for this drag session
    {
      // if duplicate items aren't allowed, see if we already have such an item
      if (![item allowsDuplicatesInToolbar])
        {
          NSArray *items = [toolbar items];
          NSUInteger index;
          for (index=0; index<[items count]; index++)
            {
              NSToolbarItem *anItem = [items objectAtIndex:index];
              if ([[anItem itemIdentifier] isEqual:identifier])
                {
                  draggedItemIndex = index; // drag the existing item
                  break;
                }
            }
        }
    }	
  else if (draggedItemIndex == OUTSIDE_INDEX)
    {
      // re-entering after being dragged off -- treat as unknown location
      draggedItemIndex = NSNotFound;
    }

  newIndex = [self _insertionIndexAtPoint: [info draggingLocation]]; 
  
  if (draggedItemIndex != NSNotFound)
    {
      // existing item being dragged -- either move or remove it
      if (exited)
        {
          [toolbar _removeItemAtIndex:draggedItemIndex broadcast:YES];
          draggedItemIndex = OUTSIDE_INDEX; // no longer in our items
        }
      else
        {
          if (newIndex != draggedItemIndex)
            {
              [toolbar _moveItemFromIndex: draggedItemIndex toIndex: newIndex broadcast: YES];
              draggedItemIndex = newIndex;
            }
        }
    }
  else if (!exited)
    {
      // new item being dragged in -- add it
      [toolbar _insertItemWithItemIdentifier: identifier 
          atIndex: newIndex
          broadcast: YES];	
      draggedItemIndex = newIndex;
    }
  return NSDragOperationGeneric;
}

- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>)info
{
  return [self updateItemWhileDragging: info exited: NO];
}

- (NSDragOperation) draggingUpdated: (id <NSDraggingInfo>)info
{
  return [self updateItemWhileDragging: info exited: NO];
}

- (void) draggingEnded: (id <NSDraggingInfo>)info
{
  draggedItemIndex = NSNotFound;
}

- (void) draggingExited: (id <NSDraggingInfo>)info
{
  [self updateItemWhileDragging: info exited: YES];
}

- (BOOL) prepareForDragOperation: (id <NSDraggingInfo>)info
{
  return YES;
}

- (BOOL) performDragOperation: (id <NSDraggingInfo>)info
{
  NSToolbar *toolbar = [self toolbar];

  [self updateItemWhileDragging: info exited: NO];
  
  draggedItemIndex = NSNotFound;
  
  // save the configuration...
  [toolbar _saveConfig];

  return YES;
}

- (void) concludeDragOperation: (id <NSDraggingInfo>)info
{
  // Nothing to do currently
}

// More overrided methods

- (void) drawRect: (NSRect)aRect
{
  [[GSTheme theme] drawToolbarRect: aRect
                   frame: [self frame]
                   borderMask: _borderMask];
}

- (BOOL) isOpaque
{
  if ([[[GSTheme theme] toolbarBackgroundColor] alphaComponent] < 1.0)
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (void) windowDidResize: (NSNotification *)notification
{ 
  if ([self superview] == nil) 
    return;
  
  [self _reload];
}

- (void) viewWillMoveToSuperview: (NSView *)newSuperview
{ 
  [super viewWillMoveToSuperview: newSuperview];
  
  [_toolbar _toolbarViewWillMoveToSuperview: newSuperview]; 
  // Allow to update the validation system which is window specific 
}

- (void) viewDidMoveToWindow
{ 
  NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
  
  /* NSView method called when a view is moved to a window (NSView has a
     variable _window). */
  [super viewDidMoveToWindow]; 
  
  [nc removeObserver: self name: NSWindowDidResizeNotification object: nil];
  [nc addObserver: self selector: @selector(windowDidResize:) 
                            name: NSWindowDidResizeNotification 
                          object: _window];
}

// Accessors

- (unsigned int) borderMask
{
  return _borderMask;
}

- (NSToolbar *) toolbar
{
  return _toolbar;
}

- (void) setBorderMask: (unsigned int)borderMask
{
  NSRect toolbarViewFrame = [self frame];
  NSRect rect = NSMakeRect(0, 0, toolbarViewFrame.size.width, 
                           toolbarViewFrame.size.height);
  
  _borderMask = borderMask;
  
  // Take in account the border
  if (_borderMask & GSToolbarViewBottomBorder)
    {
      rect = NSMakeRect(rect.origin.x, ++rect.origin.y, rect.size.width, 
                        --rect.size.height);
    }

  if (_borderMask & GSToolbarViewTopBorder)
    {
      rect = NSMakeRect(rect.origin.x, rect.origin.y, rect.size.width, 
                        --rect.size.height); 
    }
    
  if (_borderMask & GSToolbarViewLeftBorder)
    {
      rect = NSMakeRect(++rect.origin.x, rect.origin.y, --rect.size.width, 
                        rect.size.height);
    }
    
  if (_borderMask & GSToolbarViewRightBorder)
    {
      rect = NSMakeRect(rect.origin.x, rect.origin.y, --rect.size.width, 
                        rect.size.height);
    }
    
  [_clipView setFrame: rect];
}

- (void) setToolbar: (NSToolbar *)toolbar 
{
  if (_toolbar == toolbar)
    return;

  _toolbar = toolbar;

  [_clippedItemsMark setToolbar: _toolbar];
  // Load the toolbar in the toolbar view
  [self _reload];
}

// Private methods

- (void) _handleBackViewsFrame
{
  CGFloat x = 0;
  CGFloat newHeight = 0;
  NSArray *subviews = [_clipView subviews];
  NSEnumerator *e = [[_toolbar items] objectEnumerator];
  NSToolbarItem *item;
  
  while ((item = [e nextObject]) != nil) 
    {
      NSView *itemBackView;
      NSRect itemBackViewFrame;

      itemBackView = [item _backView];
      if ([subviews containsObject: itemBackView] == NO
        || [item _isModified] 
        || [item _isFlexibleSpace])
        {
          // When a label is changed, _isModified returns YES to let us known we
          // must recalculate the text length and then the size for the edited
          // item back view
          [item _layout];
        }
      
      itemBackViewFrame = [itemBackView frame];
      [itemBackView setFrame: NSMakeRect(x, itemBackViewFrame.origin.y, 
        itemBackViewFrame.size.width, itemBackViewFrame.size.height)];
        
      x += [itemBackView frame].size.width;
      
      if (itemBackViewFrame.size.height > newHeight)
        newHeight = itemBackViewFrame.size.height;
    }
    
  if (newHeight > 0)
    _heightFromLayout = newHeight;
}

- (void) _takeInAccountFlexibleSpaces
{
  NSArray *items = [_toolbar items];
  NSEnumerator *e;
  NSToolbarItem *item;
  NSView *backView, *view;
  CGFloat lengthAvailable;
  BOOL mustAdjustNext = NO;
  CGFloat x = 0, visibleItemsMinWidth = 0, backViewsWidth = 0;
  NSMutableArray *variableWidthItems = [NSMutableArray array];
  unsigned flexibleItemsCount = 0, maxWidthItemsCount = 0;
  CGFloat spacePerFlexItem, extraSpace = 0;
  CGFloat toolbarWidth = [self frame].size.width;
  NSUInteger i, n;
  NSMutableArray *visibleItems = [NSMutableArray array];
  static const int FlexItemWeight = 4; // non-space flexible item counts as much as 4 flexible spaces

  n = [items count];
  if (n == 0)
    return; 
  
  // First determine which items can fit in toolbar if all are at their minimum width.
  // We'd like to show as many items as possible. These are our visibleItems.
  for (i=0; i < n; i++) 
    {
      item = [items objectAtIndex:i];
      backView = [item _backView];
      view = [item view];
      if (view != nil)
        backViewsWidth += [item minSize].width + 2*InsetItemViewX;
      else
        backViewsWidth += [backView frame].size.width;

      if ((backViewsWidth + ClippedItemsViewWidth <= toolbarWidth)
        || (i == n - 1 && backViewsWidth <= toolbarWidth))
        {
          visibleItemsMinWidth = backViewsWidth;
          [visibleItems addObject:item];
        }
      else
        {
          break;
        }
    }
  // next, figure out how much additional space there is for expanding flexible items
  lengthAvailable = toolbarWidth - visibleItemsMinWidth;
  if ([visibleItems count] < n)
    lengthAvailable -= ClippedItemsViewWidth;

  if (lengthAvailable < 1)
    return;
 
  // We want to divide available space evenly among all flexible items, but some items may
  // reach their maximum width, making more space available for the other items.
  // To do this, first we count the flexible items, gathering a list of those that may
  // have a maximum width.
  // To match observed behavior on Cocoa (which is NOT as documented!) we allocate only 1/4
  // as much space to flexible spaces as we do to other flexible items.
  e = [visibleItems objectEnumerator];
  while ((item = [e nextObject]) != nil) 
    {
      if ([item _isFlexibleSpace])
        {
          flexibleItemsCount++;
        }
      else
        {
          CGFloat minWidth = [item minSize].width;
          CGFloat maxWidth = [item maxSize].width;
          if (minWidth < maxWidth)
            {
              [variableWidthItems addObject:item];
              flexibleItemsCount += FlexItemWeight; // gets FlexItemWeight times the weight of a flexible space
            }
        }
    }
  if (flexibleItemsCount == 0)
    return;
    
  // Now go through any variableWidthItems to see if the available space per item would
  // cause any of them to exceed their maximum width, and calculate the extra space available
  spacePerFlexItem = MAX(lengthAvailable / flexibleItemsCount, 0);
  e = [variableWidthItems objectEnumerator];
  while ((item = [e nextObject]) != nil)
    {
      CGFloat minWidth = [item minSize].width;
      CGFloat maxWidth = [item maxSize].width;
      if (maxWidth-minWidth < spacePerFlexItem * FlexItemWeight)
        {
          extraSpace += spacePerFlexItem * FlexItemWeight - (maxWidth-minWidth); // give back unneeded space
          maxWidthItemsCount += FlexItemWeight;
        }
    }
  // Recalculate spacePerFlexItem (unless all flexible items are going to their max width)
  if (flexibleItemsCount > maxWidthItemsCount)
    spacePerFlexItem += extraSpace / (flexibleItemsCount-maxWidthItemsCount);
  
  // Finally, go through all items, adjusting their width and positioning them as needed
  e = [items objectEnumerator];
  while ((item = [e nextObject]) != nil)
  {
    backView = [item _backView];
    if ([item _isFlexibleSpace])
      {
        NSRect backViewFrame = [backView frame];
	NSRect newFrameRect = NSMakeRect(x, backViewFrame.origin.y,
					 spacePerFlexItem,
					 backViewFrame.size.height);
	[backView setFrame: [self centerScanRect:newFrameRect]];
        mustAdjustNext = YES;
      }
    else if ([variableWidthItems indexOfObjectIdenticalTo:item] != NSNotFound)
      {
        NSRect backViewFrame = [backView frame];
        CGFloat maxFlex = [item maxSize].width - [item minSize].width;
        CGFloat flexAmount = MIN(maxFlex, spacePerFlexItem * FlexItemWeight);
        CGFloat newWidth = [item minSize].width + flexAmount + 2 * InsetItemViewX;
	NSRect newFrameRect = NSMakeRect(x, backViewFrame.origin.y,
					 newWidth, 
					 backViewFrame.size.height);
	[backView setFrame: [self centerScanRect: newFrameRect]];
        mustAdjustNext = YES;
      }
    else if (mustAdjustNext)
      {
        NSRect backViewFrame = [backView frame];
	NSRect newFrameRect = NSMakeRect(x, backViewFrame.origin.y,
					 backViewFrame.size.width,
					 backViewFrame.size.height);
	[backView setFrame: [self centerScanRect: newFrameRect]];
      }
    view = [item view];
    if (view != nil)
      {
        NSRect viewFrame = [view frame];
        // Subtract InsetItemViewX
        viewFrame.size.width = [backView frame].size.width - 2 * InsetItemViewX;
        viewFrame.origin.x = InsetItemViewX;
        [view setFrame: viewFrame];
      }
    x += [backView frame].size.width;
  }
}

- (void) _handleViewsVisibility
{
  NSArray *backViews;
  NSArray *subviews;
  NSEnumerator *e;
  NSView *backView;
  
  /* The back views which are associated with each toolbar item (the toolbar
     items doesn't reflect the toolbar view content) */
  backViews = [[_toolbar items] valueForKey: @"_backView"];

  // We remove each back view associated with a removed toolbar item
  e = [[_clipView subviews] objectEnumerator];
  while ((backView = [e nextObject]) != nil) 
    {
      if ([backViews containsObject: backView] == NO)
        {
          if ([backView superview] != nil) 
            [backView removeFromSuperview];
        }
    }
      
  // We add each backView associated with an added toolbar item
  subviews = [_clipView subviews];
  e = [backViews objectEnumerator];
  while ((backView = [e nextObject]) != nil) 
  {
    if ([subviews containsObject: backView] == NO)
      {
        [_clipView addSubview: backView];
      }
  }
}

- (void) _manageClipView
{
  NSRect clipViewFrame = [_clipView frame];
  NSUInteger count = [[_toolbar items] count];
  // Retrieve the back views which should be visible now that the resize
  // process has been taken in account
  NSArray *visibleBackViews = [self _visibleBackViews];

  if ([visibleBackViews count] < count)
    {
      NSView *lastVisibleBackView = [visibleBackViews lastObject];
      float width = 0;
      
      // Resize the clip view
      if (lastVisibleBackView != nil)
        width = NSMaxX([lastVisibleBackView frame]);  
      [_clipView setFrame: NSMakeRect(clipViewFrame.origin.x,
                                      clipViewFrame.origin.y, 
                                      width,
                                      clipViewFrame.size.height)]; 
        
      // Adjust the clipped items mark frame handling   
      [_clippedItemsMark layout];

      // We get the new _clipView frame      
      clipViewFrame = [_clipView frame];
      [_clippedItemsMark setFrameOrigin: NSMakePoint(
        [self frame].size.width - ClippedItemsViewWidth, clipViewFrame.origin.y)];
        
      if ([_clippedItemsMark superview] == nil)       
        [self addSubview: _clippedItemsMark];  
      
    }
  else if (([_clippedItemsMark superview] != nil) 
    && ([visibleBackViews count] == count)) 
    {      
      [_clippedItemsMark removeFromSuperview];
      
      [_clipView setFrame: NSMakeRect(clipViewFrame.origin.x, 
                                      clipViewFrame.origin.y, 
                                      [self frame].size.width, 
                                      clipViewFrame.size.height)]; 
    }
}

- (void) _reload
{
  // First, we resize
  [self _handleBackViewsFrame];
  [self _takeInAccountFlexibleSpaces];
  
  [self _handleViewsVisibility]; 
  /* We manage the clipped items view in the case it should become visible or
     invisible */
  [self _manageClipView];

  [self setNeedsDisplay: YES];
}

// Accessors private methods

- (CGFloat) _heightFromLayout
{    
  CGFloat height = _heightFromLayout;
  
  if (_borderMask & GSToolbarViewBottomBorder)
    {
      height++;
    }

  if (_borderMask & GSToolbarViewTopBorder)
    {
      height++; 
    }
      
  return height;
}

/*
 * Will return the visible (not clipped) back views in the toolbar view even
 * when the toolbar is not visible.
 * May be should be renamed _notClippedBackViews method.
 */
- (NSArray *) _visibleBackViews 
{
  NSArray *items = [_toolbar items];
  NSView *backView, *view;
  NSUInteger i, n;
  float backViewsWidth = 0, toolbarWidth = [self frame].size.width;

  NSMutableArray *visibleBackViews = [NSMutableArray array];

  n = [items count];
  for (i = 0; i < n; i++)
    {
      NSToolbarItem *item = [items objectAtIndex:i];
      backView = [item _backView];
      view = [item view];
      if (view != nil)
        backViewsWidth += [item minSize].width + 2*InsetItemViewX;
      else
        backViewsWidth += [backView frame].size.width;

      if ((backViewsWidth + ClippedItemsViewWidth <= toolbarWidth)
        || (i == n - 1 && backViewsWidth <= toolbarWidth))
        {
          [visibleBackViews addObject: backView];
        }     
    }
  
  return visibleBackViews;
}

- (NSColor *) standardBackgroundColor
{
  NSLog(@"Use of deprecated method %@", NSStringFromSelector(_cmd));
  return nil;
}

- (BOOL) _usesStandardBackgroundColor
{
  NSLog(@"Use of deprecated method %@", NSStringFromSelector(_cmd));
  return NO;
}

- (void) _setUsesStandardBackgroundColor: (BOOL)standard
{
  NSLog(@"Use of deprecated method %@", NSStringFromSelector(_cmd));
}

- (NSMenu *) menuForEvent: (NSEvent *)event 
{
  NSMenu *menu = [[NSMenu alloc] initWithTitle: @""];
  id <NSMenuItem> customize = [menu insertItemWithTitle: _(@"Customize Toolbar")
                                                 action: @selector(runCustomizationPalette:)
                                          keyEquivalent: @""
                                                atIndex: 0];
  [customize setTarget: _toolbar];
  return AUTORELEASE(menu);
}

@end
