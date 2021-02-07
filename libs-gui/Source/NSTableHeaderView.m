/** <title>NSTableHeaderView</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 1999
   First actual coding.

   Author: Nicola Pero <nicola@brainstorm.co.uk>
   Date: August 2000, Semptember 2000
   Selection and resizing of Columns.

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

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/NSTableHeaderCell.h"
#import "AppKit/NSTableHeaderView.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSGraphics.h"
#import "GSGuiPrivate.h"
#import "GNUstepGUI/GSTheme.h"

/*
 * Number of pixels in either direction that will be counted as a hit 
 * on the column border and trigger a column resize.
 */
#define mouse_sensitivity 4

@interface NSTableView (GNUstepPrivate)
- (void) _userResizedTableColumn: (NSInteger)index
                           width: (CGFloat)width;
- (CGFloat *) _columnOrigins;
- (void) _mouseDownInHeaderOfTableColumn: (NSTableColumn *)tc;
- (void) _clickTableColumn: (NSTableColumn *)tc;
@end

@implementation NSTableHeaderView

/*
 *
 * Class methods
 *
 */
+ (void) initialize
{
  if (self == [NSTableColumn class])
    [self setVersion: 1];
}

/*
 *
 * Instance methods
 *
 */

/*
 * Initializes an instance
 */

// TODO: Remove this method, if not really needed
- (id)initWithFrame:(NSRect)frameRect
{
  self = [super initWithFrame: frameRect];
  if (self == nil)
      return nil;

  _tableView = nil;
  _resizedColumn = -1;
  return self;
}

- (BOOL) isFlipped
{
  return YES;
}

/*
 * Setting the table view 
 */
- (void)setTableView: (NSTableView*)aTableView
{
  // We do not RETAIN aTableView but aTableView is supposed 
  // to RETAIN us.
  _tableView = aTableView;

}
- (NSTableView*)tableView
{
  return _tableView;
}
/*
 * Checking altered columns 
 */
- (NSInteger) draggedColumn
{
  // TODO
  return -1;
}
- (CGFloat) draggedDistance
{
  // TODO
  return -1;
}
- (NSInteger) resizedColumn
{
  return _resizedColumn;
}
/*
 * Utility methods 
 */
- (NSInteger) columnAtPoint: (NSPoint)aPoint
{
  if (_tableView == nil)
    return -1;

  /* Ask to the tableview, which is caching geometry info */
  aPoint = [self convertPoint: aPoint toView: _tableView];
  aPoint.y = [_tableView bounds].origin.y;
  return [_tableView columnAtPoint: aPoint];
}

- (NSRect)headerRectOfColumn: (NSInteger)columnIndex
{
  NSRect rect;

  if (_tableView == nil)
    return NSZeroRect;

  /* Ask to the tableview, which is caching geometry info */
  rect = [_tableView rectOfColumn: columnIndex];
  rect = [self convertRect: rect fromView: _tableView];
  rect.origin.y = _bounds.origin.y;
  rect.size.height = _bounds.size.height;
  
  return rect;
}

/*
 * Overidden Methods
 */
- (void)drawRect: (NSRect)aRect
{
  [[GSTheme theme] drawTableHeaderRect: aRect
		   inView: self];
}

- (void) resetCursorRects
{
  if ([[self tableView] allowsColumnResizing])
    {
      const NSRect visibleRect = [self visibleRect];
      NSInteger i;
      const NSInteger count = [[[self tableView] tableColumns] count];

      for (i = 0; i < (count - 1) && (count > 0); i++)
	{
	  NSRect resizeRect = [self headerRectOfColumn: i];
	  resizeRect.origin.x = NSMaxX(resizeRect) - mouse_sensitivity;
	  resizeRect.size.width = 2 * mouse_sensitivity;
	  resizeRect = NSIntersectionRect(resizeRect, visibleRect);

	  if (!NSEqualRects(NSZeroRect, resizeRect))
	    {
	      [self addCursorRect: resizeRect cursor: [NSCursor resizeLeftRightCursor]];
	    }
	}
    }
}

/**
 * In -mouseDown we intercept the mouse event to handle the
 * colum resize and rearrangement. Resizing or moving columns
 * will do a live resize/move of the columns by default. Users can revert to
 * a "ghost" resize/move indicator by doing:
 * defaults write NSGlobalDomain GSUseGhostResize YES
 */
- (void) mouseDown: (NSEvent*)event
{
  NSPoint location = [event locationInWindow];
  NSInteger clickCount;
  NSInteger columnIndex;
  NSTableColumn *currentColumn;

  clickCount = [event clickCount];

  /*  
  if (clickCount > 2)
    {
      return;
    }
  */  

  location = [self convertPoint: location fromView: nil];
  columnIndex = [self columnAtPoint: location];
  
  if (columnIndex == -1)
    {
      return;  
    }
  currentColumn = [[_tableView tableColumns]
                    objectAtIndex: columnIndex];


  if (clickCount == 2)
    {
      [_tableView _sendDoubleActionForColumn: columnIndex];
      //      return;
    }

  //  if (clickCount == 1)
    {
      NSRect rect = [self headerRectOfColumn: columnIndex];

      /* Safety check */
      if (_resizedColumn != -1)
        {
          NSLog(@"Bug: starting resizing of column while already resizing!");
          _resizedColumn = -1;
        }
      
      if ([_tableView allowsColumnResizing])
        {
          /* Start resizing if the mouse is down on the bounds of a column. */
          if (location.x >= NSMaxX(rect) - mouse_sensitivity)
            {
              if (columnIndex < [_tableView numberOfColumns])
                {
                  _resizedColumn = columnIndex;
                }
              else
                {
                  NSLog(@"Bug: Trying to resize column past the end of the table.");
                }
            }
          else if (location.x <= NSMinX(rect) + mouse_sensitivity) 
            {
              if (columnIndex > 0)
                {
                  _resizedColumn = columnIndex - 1;
                }
            }
        }

      /* Resizing */
      if (_resizedColumn != -1)
        {
          CGFloat p;
          NSEvent *e;
          BOOL lit;
          NSUInteger eventMask;
          BOOL liveResize;

          /* Width of the highlighted area. */
          const float divWidth = 4;
          /* Coordinates of visible part of table */
          CGFloat minVisCoord = NSMinX([self visibleRect]);
          CGFloat maxVisCoord = NSMaxX([self visibleRect]);
          
          NSPoint unconverted = [event locationInWindow];
          NSArray *columns = [_tableView tableColumns];
          /* Column on the left of resizing bound */
          NSTableColumn *column = [columns objectAtIndex: _resizedColumn];
          const CGFloat columnMinX = NSMinX([self headerRectOfColumn: _resizedColumn]);
          const CGFloat columnMinWidth = [column minWidth];
          const CGFloat columnMaxWidth = [column maxWidth];
          CGFloat newColumnWidth = [column width];
          CGFloat newColumnMaxX;
          NSRect oldHighlightRect;
          NSRect highlightRect = [self visibleRect];
          highlightRect.size.width = divWidth;
          
          /* Mouse position */
          /* YES if some highlighting was done and needs to be undone */
          lit = NO;
          eventMask = NSLeftMouseUpMask | NSLeftMouseDraggedMask | NSPeriodicMask;
          liveResize = ![[NSUserDefaults standardUserDefaults] boolForKey: @"GSUseGhostResize"];
          
          if ([column isResizable] == NO)
            {
              _resizedColumn = -1;
              return;
            }

          /* Do we need to check that we already fit into this area ? 
             We should */

	  [[NSCursor resizeLeftRightCursor] push];

          if (!liveResize)
            {
              oldHighlightRect = NSZeroRect;
              [self lockFocus];
              [[NSColor lightGrayColor] set];
            }
            
          [[NSRunLoop currentRunLoop] limitDateForMode: NSEventTrackingRunLoopMode];
          
          [NSEvent startPeriodicEventsAfterDelay: 0.05 withPeriod: 0.05];
          e = [NSApp nextEventMatchingMask: eventMask
                     untilDate: [NSDate distantFuture]
                     inMode: NSEventTrackingRunLoopMode
                     dequeue: YES];

          while ([e type] != NSLeftMouseUp)
            {
              if ([e type] != NSPeriodic)
                {
                  unconverted = [e locationInWindow];
                }
                  
              p = [self convertPoint: unconverted fromView: nil].x;        
              minVisCoord = NSMinX([self visibleRect]);
              maxVisCoord = NSMaxX([self visibleRect]);
                
              /* newColumnWidth is always positive, since we always resize
                 the column to the left of the mouse pointer */
              newColumnWidth = (p - columnMinX);
              newColumnWidth = MAX(MIN(newColumnWidth, columnMaxWidth), columnMinWidth);
              newColumnMaxX = columnMinX + newColumnWidth;
              
              if (liveResize && [column width] != newColumnWidth)
                {
                  [_tableView _userResizedTableColumn: _resizedColumn
                              width: newColumnWidth];
                }
              else if (!liveResize)
                {
                  highlightRect.origin.x = newColumnMaxX;

                  /* Only draw the divider if it is not the same as the currently
                     drawn one, and lies within the header view area */
                  if (!NSEqualRects(oldHighlightRect,highlightRect) && 
                       highlightRect.origin.x > [self visibleRect].origin.x &&
                       highlightRect.origin.x + divWidth < [self visibleRect].origin.x + [self visibleRect].size.width)
                    {
                      if (lit)
                        {
                          NSHighlightRect(oldHighlightRect);
                        }
                      NSHighlightRect(highlightRect);
                      [_window flushWindow];                         
                      lit = YES;
                      oldHighlightRect = highlightRect;
                    }
                }

              /* Scroll the tableview, if needed, so the user's desired new 
                 column edge position lies at the edge of the visible part of 
                 the table */
              if ((p > maxVisCoord && newColumnMaxX > maxVisCoord)
                || (p < minVisCoord && newColumnMaxX < minVisCoord))
                {
                  NSRect tvRect = [_tableView visibleRect];   
              
                  if (!liveResize && lit)
                    {
                      NSHighlightRect(oldHighlightRect);
                      lit = NO;
                      [_window flushWindow]; 
                    }
                  
                  if (p > maxVisCoord) /* resizing to the right */
                    tvRect.origin.x = newColumnMaxX - tvRect.size.width;
                  else                /* resizing to the left */
                    tvRect.origin.x = newColumnMaxX;
                  
                  [_tableView scrollPoint: tvRect.origin];
                }
                    
              e = [NSApp nextEventMatchingMask: eventMask
                         untilDate: [NSDate distantFuture]
                         inMode: NSEventTrackingRunLoopMode
                         dequeue: YES];
            }
          [NSEvent stopPeriodicEvents];

	  [NSCursor pop];

          if (!liveResize)
            {
              if (lit)
                {
                  NSHighlightRect(oldHighlightRect);
                  [_window flushWindow];
                }
              [self unlockFocus];
              
              /* The following tiles the table.  We use a private method 
                 which avoids tiling the table twice. */
              if ([column width] != newColumnWidth)
                {
                  [_tableView _userResizedTableColumn: _resizedColumn
                              width: newColumnWidth];
                }
            }

          /* Clean up */
          _resizedColumn = -1;
          return;
        }

      /* We are not resizing
         Let's launch a mouseDownInHeaderOfTableColumn message
      */
      {
        NSRect rect = [self headerRectOfColumn: columnIndex];
        [_tableView _mouseDownInHeaderOfTableColumn: 
                      [[_tableView tableColumns] 
                        objectAtIndex: columnIndex]];
        rect.origin.y++;
        rect.size.height--;
        [[currentColumn headerCell] setHighlighted: YES];

        [self lockFocus];
        [[currentColumn headerCell]
          highlight: YES
          withFrame: rect
          inView: self];
        [self unlockFocus];
        [_window flushWindow];
      }


      /* Dragging */
      /* Wait for mouse dragged events. 
         If mouse is dragged, move the column.
         If mouse is not dragged but released, select/deselect the column. */
      if ([_tableView allowsColumnReordering])
        {
          NSInteger i = columnIndex;
          NSInteger j = columnIndex;
          CGFloat minCoord; 
          CGFloat maxCoord; 
          CGFloat minVisCoord;
          CGFloat maxVisCoord;
          CGFloat *_cO;
          CGFloat *_cO_minus1;
          NSInteger numberOfColumns = [_tableView numberOfColumns];
          NSUInteger eventMask = (NSLeftMouseUpMask 
                                    | NSLeftMouseDraggedMask 
                                    | NSPeriodicMask);
          NSUInteger modifiers = [event modifierFlags];
          NSEvent *e;
          NSDate *distantFuture = [NSDate distantFuture];
          NSRect visibleRect = [self visibleRect];
          NSRect tvRect;
          NSRect highlightRect = NSZeroRect, oldRect = NSZeroRect;
          BOOL outside = NO;
          BOOL lit = NO;
          BOOL liveResize = ![[NSUserDefaults standardUserDefaults] boolForKey: @"GSUseGhostResize"];
          
          BOOL mouseDragged = NO;
          CGFloat p;
          NSPoint unconverted;
          minVisCoord = NSMinX (visibleRect);
          maxVisCoord = NSMaxX (visibleRect);
          {
            NSRect bounds = [self bounds];
            minCoord = NSMinX(bounds);
            maxCoord = NSMaxX(bounds);
          }
          {
            CGFloat *_c = [_tableView _columnOrigins];
            _cO_minus1 = malloc((numberOfColumns + 3) * sizeof(CGFloat));
            _cO = _cO_minus1 + 1;
            memcpy(_cO, _c, numberOfColumns * sizeof(CGFloat));
            _cO[numberOfColumns] = maxCoord;
            _cO[numberOfColumns + 1] = maxCoord;
            _cO[-1] = minCoord;
          }

          highlightRect.size.height = NSHeight (visibleRect);
          highlightRect.origin.y = NSMinY (visibleRect);

          if (!liveResize)
            {
              [self lockFocus];
              [[NSColor lightGrayColor] set];
            }
          [NSEvent startPeriodicEventsAfterDelay: 0.05
                   withPeriod: 0.05];
          e = [NSApp nextEventMatchingMask: eventMask 
                     untilDate: distantFuture
                     inMode: NSEventTrackingRunLoopMode 
                     dequeue: YES];

          while ([e type] != NSLeftMouseUp)
            {
              switch ([e type])
                {
                case NSLeftMouseDragged:
                  unconverted = [e locationInWindow];
                  p = [self convertPoint: unconverted fromView: nil].x;
                  if (mouseDragged == NO)
                    {
                      [self setNeedsDisplay:YES];
                    }
                  mouseDragged = YES;
                  if (p < minVisCoord || p > maxVisCoord)
                    {
                      outside = YES;
                    }
                  else
                    {
                      outside = NO;
                      i = j;
                      if (p > (_cO[i] + _cO[i+1]) / 2)
                        {
                          while (p > (_cO[i] + _cO[i+1]) / 2)
                            i++;
                        }
                      else if (p < (_cO[i] + _cO[i-1]) / 2)
                        {
                          while (p < (_cO[i] + _cO[i-1]) / 2)
                            i--;
                        }
                      if (!liveResize)
                        {  
                          if (i != columnIndex
                              && i != columnIndex + 1)
                            {
                              j = i;
                              highlightRect.size.height = NSHeight (visibleRect);
                              highlightRect.origin.y = NSMinY (visibleRect);
                              highlightRect.size.width = 7;
                              if (i == numberOfColumns)
                                {
                                  highlightRect.origin.x = _cO[i] - 3;
                                }
                              else if (i == 0)
                                {
                                  highlightRect.origin.x = _cO[i] - 3;
                                }
                              else
                                {
                                  highlightRect.origin.x = _cO[i] - 3;
                                }
                              if (!NSEqualRects(highlightRect, oldRect))
                                {
                                  if (lit)
                                    NSHighlightRect(oldRect);
                                  NSHighlightRect(highlightRect);
                                  [_window flushWindow];
                                }
                              else if (!lit)
                                {
                                  NSHighlightRect(highlightRect);
                                  [_window flushWindow];
                                }
                              oldRect = highlightRect;
                              lit = YES;
                            }
                          else
                            {
                              i = columnIndex;
                              highlightRect.size.height = NSHeight (visibleRect);
                              highlightRect.origin.y = NSMinY (visibleRect);
                              highlightRect.origin.x = _cO[columnIndex];
                              highlightRect.size.width = 
                                _cO[columnIndex + 1] - _cO[columnIndex];
                            
                              if (!NSEqualRects(highlightRect, oldRect))
                                {
                                  if (lit)
                                    NSHighlightRect(oldRect);
                                  //  NSHighlightRect(highlightRect);
                                  [_window flushWindow];
                                }
                              else if (!lit)
                                {
                                  //  NSHighlightRect(highlightRect);
                                  // [_window flushWindow];
                                }
                              // oldRect = highlightRect;
                              oldRect = NSZeroRect;
                              lit = NO; //lit = YES;
                            }
                         }
                       else if (liveResize)
                         {
                           if (i > columnIndex)
                             i--;
                           if (i != columnIndex)
                             {
                               [_tableView moveColumn: columnIndex
                                           toColumn: i];
                             }  
                           columnIndex = i;
                         }
                    }
                  break;
                case NSPeriodic:
                  if (outside == YES)
                    {
                      if (!liveResize)
                        {
                          if (lit)
                            {
                              NSHighlightRect(oldRect);
                              [_window flushWindow];
                              lit = NO;
                              oldRect = NSZeroRect;
                            }
                        }
                      p = [self convertPoint: unconverted
                                fromView: nil].x;
                      tvRect = [_tableView visibleRect];
                      if (p > maxVisCoord)
                        {
                          if (p > maxCoord)
                            tvRect.origin.x += (p - maxVisCoord)/8;
                          else
                            tvRect.origin.x += (p - maxVisCoord)/2;
                        }
                      else if (p < minVisCoord)
                        {
                          if (p < minCoord)
                            tvRect.origin.x += (p - minVisCoord)/8;
                          else
                            tvRect.origin.x += (p - minVisCoord)/2;
                        }
                      else // TODO remove this condition
                        {
                          NSLog(@"not outside !");
                        }
                      [_tableView scrollPoint: tvRect.origin];
                      visibleRect = [self visibleRect];
                      minVisCoord = NSMinX (visibleRect);
                      maxVisCoord = NSMaxX (visibleRect);
                    }
                  break;
                default:
                  break;
                }
              e = [NSApp nextEventMatchingMask: eventMask 
                         untilDate: distantFuture
                         inMode: NSEventTrackingRunLoopMode 
                         dequeue: YES]; 
            }
            
          if (!liveResize)
            {
              if (lit)
                {
                  NSHighlightRect(highlightRect);
                  [_window flushWindow];
                }
              [self unlockFocus];
            }

          [NSEvent stopPeriodicEvents];       
            
          if (mouseDragged == NO)
            {
              [_tableView _selectColumn: columnIndex modifiers: modifiers];
              [_tableView _clickTableColumn: currentColumn];

              [self setNeedsDisplay: YES];;
            }
          else // mouseDragged == YES
            {
              {
                NSRect rect = [self headerRectOfColumn: columnIndex];
                [_tableView _mouseDownInHeaderOfTableColumn: 
                              [[_tableView tableColumns] 
                                objectAtIndex: columnIndex]];
                rect.origin.y++;
                rect.size.height--;
                [[currentColumn headerCell]
                  setHighlighted: NO];
                
                [self lockFocus];
                [[currentColumn headerCell] 
                  highlight: NO
                  withFrame: rect
                  inView: self];
                [self unlockFocus];
                [_window flushWindow];
              }
              if (i > columnIndex)
                i--;
              if (i != columnIndex)
                {
                  [_tableView moveColumn: columnIndex
                              toColumn: i];
                }
            }
          free(_cO_minus1);
          return;
        }
      else
        {
          NSRect cellFrame = [self headerRectOfColumn: columnIndex];
          NSApplication *theApp = [NSApplication sharedApplication];
          NSUInteger modifiers = [event modifierFlags];
          NSPoint location = [event locationInWindow];
          NSPoint point = [self convertPoint: location fromView: nil];

          if (![self mouse: point inRect: cellFrame])
            {
              NSLog(@"not in frame, what's happening ?");
              return;
            }

          event = [theApp nextEventMatchingMask: NSLeftMouseUpMask
                          untilDate: [NSDate distantFuture]
                          inMode: NSEventTrackingRunLoopMode
                          dequeue: NO];
          

          location = [event locationInWindow];
          
          point = [self convertPoint: location fromView: nil];
          
          if (![self mouse: point inRect: cellFrame])
            {
              NSDebugLLog(@"NSCell", 
                          @"tableheaderview point not in cell frame\n");
              {
                NSRect rect = [self headerRectOfColumn: columnIndex];
                [_tableView _mouseDownInHeaderOfTableColumn: 
                              [[_tableView tableColumns] 
                                objectAtIndex: columnIndex]];
                rect.origin.y++;
                rect.size.height--;
                [[currentColumn headerCell]
                  setHighlighted: NO];
                
                [self lockFocus];
                [[currentColumn headerCell] 
                  highlight: NO
                  withFrame: rect
                  inView: self];
                [self unlockFocus];
                [_window flushWindow];
              }

            }
          else
            {
              [_tableView _selectColumn: columnIndex modifiers: modifiers];
              [_tableView _clickTableColumn: currentColumn];

              [self setNeedsDisplay: YES];
              /*              
              if ([_tableView highlightedTableColumn] != currentColumn)
                {
                  NSRect rect = [self headerRectOfColumn: columnIndex];
                  
                  // [_tableView _mouseDownInHeaderOfTableColumn: 
                  // [[_tableView tableColumns] 
                  // objectAtIndex: columnIndex]];

                  rect.origin.y++;
                  rect.size.height--;
                  NSLog(@"highlight");
                  [[currentColumn headerCell] setHighlighted: NO];
                  
                  [[currentColumn headerCell] 
                    highlight: NO
                    withFrame: rect
                    inView: self];
                  [_window flushWindow];
                }
              */
            }
        }
    }
}

/*
 * Encoding/Decoding
 */

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  /* Nothing else to encode in NSTableHeaderView:
       - _tableView is set by the parent NSTableView
       - _resizedColumn is reset on decoding anyway
     */
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (self == nil)
      return nil;

  // NOTE: some xib's can have unintuitive load orders where
  // the above -initWithCoder: call causes the receiver's assocaited
  // table view to be loaded, which calls -[self setTableView:].
  // So at this point, _tableView might already have been set,
  // so we must not set it to nil here.

  _resizedColumn = -1;

  return self;
}

@end

