/*
   GSDragView - Generic Drag and Drop code.

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: May 2004

   Based on X11 specific code from:
   Created by: Wim Oudshoorn <woudshoo@xs4all.nl>
   Date: Nov 2001
   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1998

   This file is part of the GNU Objective C User Interface Library.

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

#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSThread.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"

#import "GNUstepGUI/GSDisplayServer.h"
#import "GNUstepGUI/GSDragView.h"
#include <math.h>

/* Size of the dragged window */
#define	DWZ	48

#define SLIDE_TIME_STEP   .02   /* in seconds */
#define SLIDE_NR_OF_STEPS 20  

@interface GSRawWindow : NSWindow
@end

@interface NSApplication (GNUstepPrivate)
- (void) _postAndSendEvent: (NSEvent *)anEvent;
@end

@interface NSCursor (BackendPrivate)
- (void *)_cid;
- (void) _setCid: (void *)val;
@end

@interface GSDragView (Private)
- (void) _setupWindowFor: (NSImage*)anImage
	   mousePosition: (NSPoint)mPoint
	   imagePosition: (NSPoint)iPoint;
- (void) _clearupWindow;
- (BOOL) _updateOperationMask: (NSEvent*) theEvent;
- (void) _setCursor;
- (void) _sendLocalEvent: (GSAppKitSubtype)subtype
	  action: (NSDragOperation)action
	position: (NSPoint)eventLocation
       timestamp: (NSTimeInterval)time
	toWindow: (NSWindow*)dWindow;
- (void) _handleDrag: (NSEvent*)theEvent slidePoint: (NSPoint)slidePoint;
- (void) _handleEventDuringDragging: (NSEvent *)theEvent;
- (void) _updateAndMoveImageToCorrectPosition;
- (void) _moveDraggedImageToNewPosition;
- (void) _slideDraggedImageTo: (NSPoint)screenPoint
	numberOfSteps: (int) steps
		delay: (float) delay
               waitAfterSlide: (BOOL) waitFlag;
@end

@implementation GSRawWindow

- (BOOL) canBecomeMainWindow
{
  return NO;
}

- (BOOL) canBecomeKeyWindow
{
  return NO;
}

- (void) _initDefaults
{
  [super _initDefaults];
  [self setReleasedWhenClosed: NO];
  [self setExcludedFromWindowsMenu: YES];
}

- (void) orderWindow: (NSWindowOrderingMode)place relativeTo: (NSInteger)otherWin
{
  [super orderWindow: place relativeTo: otherWin];
  [self setLevel: NSPopUpMenuWindowLevel];
}

@end


@implementation GSDragView

static	GSDragView *sharedDragView = nil;

+ (id) sharedDragView
{
  if (sharedDragView == nil)
    {
      sharedDragView = [GSDragView new];
    }
  return sharedDragView;
}

+ (Class) windowClass
{
  return [GSRawWindow class];
}

- (id) init
{
  self = [super init];
  if (self != nil)
    {
      NSRect winRect = {{0, 0}, {DWZ, DWZ}};
      NSWindow *sharedDragWindow = [[[object_getClass(self) windowClass] alloc]
                                     initWithContentRect: winRect
                                               styleMask: NSBorderlessWindowMask
                                                 backing: NSBackingStoreNonretained
                                                   defer: NO];

      dragCell = [[NSCell alloc] initImageCell: nil];
      [dragCell setBordered: NO];
      
      [sharedDragWindow setContentView: self];
      [sharedDragWindow setBackgroundColor: [NSColor clearColor]];
      // Kept alive by the window
      RELEASE(self);
    }

  return self;
}

- (void) dealloc
{
  [super dealloc];
}

/* NSDraggingInfo protocol */
- (NSWindow*) draggingDestinationWindow
{
  return destWindow;
}

- (NSPoint) draggingLocation
{
  return dragPoint;
}

- (NSPasteboard*) draggingPasteboard
{
  return dragPasteboard;
}

- (NSInteger) draggingSequenceNumber
{
  return dragSequence;
}

- (id) draggingSource
{
  return dragSource;
}

- (NSDragOperation) draggingSourceOperationMask
{
  // Mix in possible modifiers
  return dragMask & operationMask;
}

- (NSImage*) draggedImage
{
  if (dragSource)
    return [dragCell image];
  else
    return nil;
}

- (NSPoint) draggedImageLocation
{
  NSPoint loc = dragPoint;

  if (dragSource)
    {
      loc.x -= offset.width;
      loc.y -= offset.height;
    }

  return loc;
}

- (NSArray *) namesOfPromisedFilesDroppedAtDestination: (NSURL *)dropDestination
{
  if ([dragSource respondsToSelector:
                    @selector(namesOfPromisedFilesDroppedAtDestination:)])
    {
      return [dragSource namesOfPromisedFilesDroppedAtDestination: 
                           dropDestination];
    }
  else
    {
      return nil;
    }
}

- (BOOL) isDragging
{
  return isDragging;
}

- (void) drawRect: (NSRect)rect
{
  [dragCell drawWithFrame: [self frame] inView: self];
}

- (void) dragImage: (NSImage*)anImage
		at: (NSPoint)screenLocation
	    offset: (NSSize)initialOffset
	     event: (NSEvent*)event
	pasteboard: (NSPasteboard*)pboard
	    source: (id)sourceObject
	 slideBack: (BOOL)slideFlag
{
  NSPoint	eventPoint;
  NSPoint	imagePoint;

  ASSIGN(dragPasteboard, pboard);
  ASSIGN(dragSource, sourceObject);
  dragSequence = [event timestamp];
  slideBack = slideFlag;

  // Unset the target window  
  targetWindowRef = 0;
  targetMask = NSDragOperationEvery;
  destExternal = NO;

  NSDebugLLog(@"NSDragging", @"Start drag with %@", [pboard types]);

  /*
   * The position of the mouse is the event location  plus any offset
   * provided.  We convert this from window coordinates to screen
   * coordinates.
   */
  eventPoint = [event locationInWindow];
  eventPoint = [[event window] convertBaseToScreen: eventPoint];
  eventPoint.x += initialOffset.width;
  eventPoint.y += initialOffset.height;

  /*
   * Adjust image location to match the mose adjustment.
   */
  imagePoint = screenLocation;
  imagePoint.x += initialOffset.width;
  imagePoint.y += initialOffset.height;

  [self _setupWindowFor: anImage
	  mousePosition: eventPoint
	  imagePosition: imagePoint];

  isDragging = YES;
  [self _handleDrag: event slidePoint: screenLocation];
  isDragging = NO;
  DESTROY(dragSource);
  DESTROY(dragPasteboard);
}

- (void) slideDraggedImageTo:  (NSPoint)point
{
  float distx = point.x - dragPosition.x;
  float disty = point.y - dragPosition.y;
  float dist = sqrt((distx * distx) + (disty * disty));
  NSSize imgSize = [[dragCell image] size];
  float imgDist = sqrt((imgSize.width * imgSize.width) + 
		       (imgSize.height * imgSize.height));
  int steps = (int)(dist/imgDist);

  /*
   * Convert point from coordinates of image to coordinates of mouse
   * cursor for internal use.
   */
  point.x += offset.width;
  point.y += offset.height;
/*  [self _slideDraggedImageTo: point 
	       numberOfSteps: SLIDE_NR_OF_STEPS 
	               delay: SLIDE_TIME_STEP
	      waitAfterSlide: YES];*/
  [self _slideDraggedImageTo: point 
	       numberOfSteps: steps
	               delay: SLIDE_TIME_STEP
	      waitAfterSlide: YES];
}

/* 
   Called by NSWindow. Sends drag events to external sources
 */
- (void) postDragEvent: (NSEvent *)theEvent
{
  if ([theEvent subtype] == GSAppKitDraggingStatus)
    {
      NSDragOperation action = [theEvent data2];

      if (destExternal)
        {

        }
      else
        {	 
          if (action != targetMask)
            {
              targetMask = action;
              [self _setCursor];
            }
        }
    }
}

- (void) sendExternalEvent: (GSAppKitSubtype)subtype
                    action: (NSDragOperation)action
                  position: (NSPoint)eventLocation
                 timestamp: (NSTimeInterval)time
                  toWindow: (int)dWindowNumber
{
}

/*
  Return the window that lies below the cursor and accepts drag and drop.
  In mouseWindowRef the OS reference for this window is returned, this is even 
  set, if there is a native window, but no GNUstep window at this location.
 */
- (NSWindow*) windowAcceptingDnDunder: (NSPoint)mouseLocation
                            windowRef: (int*)mouseWindowRef
{
  NSInteger win;

  *mouseWindowRef = 0;
  win = [GSServerForWindow(_window) findWindowAt: mouseLocation
				       windowRef: mouseWindowRef
				       excluding: [_window windowNumber]];

  return GSWindowWithNumber(win);
}

@end

@implementation GSDragView (Private)

/*
  Method to initialize the dragview before it is put on the screen.
  It only initializes the instance variables that have to do with
  moving the image over the screen and variables that are used
  to keep track where we are.

  So it is typically used just before the dragview is actually displayed.

  Post conditions:
  - dragCell is initialized with the image to drag.
  - all instance variables pertaining to moving the window are initialized
 */
- (void) _setupWindowFor: (NSImage*)anImage
           mousePosition: (NSPoint)mPoint
           imagePosition: (NSPoint)iPoint
{
  NSSize imageSize;

  if (anImage == nil)
    {
      anImage = [NSImage imageNamed: @"common_Close"];
    }
  imageSize = [anImage size];
  [dragCell setImage: anImage];
  
  /* setup the coordinates, used for moving the view around */
  dragPosition = mPoint;
  newPosition = mPoint;
  offset.width = mPoint.x - iPoint.x;
  offset.height = mPoint.y - iPoint.y;

  [_window setFrame:
    NSMakeRect (iPoint.x, iPoint.y, imageSize.width, imageSize.height)
            display: NO];

  // Only display the image
  [GSServerForWindow(_window) restrictWindow: [_window windowNumber]
                                     toImage: [dragCell image]];

  [_window orderFront: nil];
}

- (void) _clearupWindow
{
  [_window setFrame: NSZeroRect display: NO];
  [_window orderOut: nil];
}

/*
  updates the operationMask by examining modifier keys
  pressed during -theEvent-.

  If the current value of operationMask == NSDragOperationIgnoresModifiers
  it will return immediately without updating the operationMask
  
  This method will return YES if the operationMask
  is changed, NO if it is still the same.
*/
- (BOOL) _updateOperationMask: (NSEvent*) theEvent
{
  NSUInteger mod = [theEvent modifierFlags];
  NSDragOperation oldOperationMask = operationMask;

  if (operationMask == NSDragOperationIgnoresModifiers)
    {
      return NO;
    }
  
  if (mod & NSControlKeyMask)
    {
      operationMask = NSDragOperationLink;
    }
  else if (mod & NSAlternateKeyMask)
    {
      operationMask = NSDragOperationCopy;
    }
  else if (mod & NSCommandKeyMask)
    {
      operationMask = NSDragOperationGeneric;
    }
  else
    {
      operationMask = NSDragOperationEvery;
    }

  return (operationMask != oldOperationMask);
}

/**
  _setCursor examines the state of the dragging and update
  the cursor accordingly.  It will not save the current cursor,
  if you want to keep the original you have to save it yourself.

  The code recogines 4 cursors:

  - NONE - when the source does not allow dragging
  - COPY - when the current operation is ONLY Copy
  - LINK - when the current operation is ONLY Link
  - GENERIC - all other cases

  And two colors

  - GREEN - when the target accepts the drop
  - BLACK - when the target does not accept the drop

  Note that the code to figure out which of the 4 cursor to use
  depends on the fact that

  {NSDragOperationNone, NSDragOperationCopy, NSDragOperationLink} = {0, 1, 2}
*/
- (void) _setCursor
{
  NSCursor *newCursor;
  NSDragOperation mask;

  mask = dragMask & operationMask;

  if (targetWindowRef != 0)
    mask &= targetMask;

  NSDebugLLog (@"NSDragging",
               @"drag, operation, target mask = (%x, %x, %x), dnd aware = %d\n",
               (unsigned int)dragMask, (unsigned int)operationMask,
               (unsigned int)targetMask, (targetWindowRef != 0));
  
  switch (mask)
    {
    case NSDragOperationNone:
      newCursor = [NSCursor operationNotAllowedCursor];
      break;
    case NSDragOperationGeneric:
    case NSDragOperationCopy:
      newCursor = [NSCursor dragCopyCursor];
      break;
    case NSDragOperationLink:
      newCursor = [NSCursor dragLinkCursor];
      break;
    case NSDragOperationDelete:
      newCursor = [NSCursor disappearingItemCursor];
      break;
    default:
      // NSDragOperationEvery, NSDragOperationPrivate
      if (targetWindowRef != 0)
        {
          newCursor = [NSCursor greenArrowCursor];
        }
      else
        {
          newCursor = [NSCursor arrowCursor];
        }
      
      break;
    }

  [newCursor set];
}

- (void) _sendLocalEvent: (GSAppKitSubtype)subtype
		  action: (NSDragOperation)action
	        position: (NSPoint)eventLocation
	       timestamp: (NSTimeInterval)time
	        toWindow: (NSWindow*)dWindow
{
  NSEvent *e;
  NSGraphicsContext *context = GSCurrentContext();
  // FIXME: Should store this once
  NSInteger dragWindowRef = (NSInteger)(intptr_t)[GSServerForWindow(_window) windowDevice: [_window windowNumber]];

  eventLocation = [dWindow convertScreenToBase: eventLocation];
  e = [NSEvent otherEventWithType: NSAppKitDefined
	                 location: eventLocation
	            modifierFlags: 0
	                timestamp: time
	             windowNumber: [dWindow windowNumber]
	                  context: context
	                  subtype: subtype
	                    data1: dragWindowRef
	                    data2: action];
  [NSApp _postAndSendEvent: e];
}

/*
  The dragging support works by hijacking the NSApp event loop.

  - this function loops until the dragging operation is finished
    and consumes all NSEvents during the drag operation.

  - It sets up periodic events.  The drawing and communication
    with DraggingSource and DraggingTarget is handled in the
    periodic event code.  The use of periodic events is purely
    a performance improvement.  If no periodic events are used
    the system can not process them all on time.
    At least on a 333Mhz laptop, using fairly simple
    DraggingTarget code.

  PROBLEMS:

  - No autoreleasePools are created.  So long drag operations can consume
    memory

  - It seems that sometimes a periodic event get lost.
*/
- (void) _handleDrag: (NSEvent*)theEvent slidePoint: (NSPoint)slidePoint
{
  // Caching some often used values. These values do not
  // change in this method.
  // Use eWindow for coordination transformation
  NSWindow	*eWindow = [theEvent window];
  NSDate	*theDistantFuture = [NSDate distantFuture];
  NSUInteger	eventMask = NSLeftMouseDownMask | NSLeftMouseUpMask
    | NSLeftMouseDraggedMask | NSMouseMovedMask
    | NSPeriodicMask | NSAppKitDefinedMask | NSFlagsChangedMask;
  NSPoint       startPoint;
  // Storing values, to restore after we have finished.
  NSCursor      *cursorBeforeDrag = [NSCursor currentCursor];
  BOOL deposited;

  startPoint = [eWindow convertBaseToScreen: [theEvent locationInWindow]];
  startPoint.x -= offset.width;
  startPoint.y -= offset.height;
  NSDebugLLog(@"NSDragging", @"Drag window origin %@\n", NSStringFromPoint(startPoint));

  // Notify the source that dragging has started
  if ([dragSource respondsToSelector:
      @selector(draggedImage:beganAt:)])
    {
      [dragSource draggedImage: [self draggedImage]
		       beganAt: startPoint];
    }

  // --- Setup up the masks for the drag operation ---------------------
  if ([dragSource respondsToSelector:
    @selector(ignoreModifierKeysWhileDragging)]
    && [dragSource ignoreModifierKeysWhileDragging])
    {
      operationMask = NSDragOperationIgnoresModifiers;
    }
  else
    {
      operationMask = 0;
      [self _updateOperationMask: theEvent];
    }


  if ([dragSource respondsToSelector:
                    @selector(draggingSourceOperationMaskForLocal:)])
    {
      dragMask = [dragSource draggingSourceOperationMaskForLocal: !destExternal];
    }
  else
    {
      dragMask = NSDragOperationCopy | NSDragOperationLink |
        NSDragOperationGeneric | NSDragOperationPrivate;
    }
  
  // --- Setup the event loop ------------------------------------------
  [self _updateAndMoveImageToCorrectPosition];
  [NSEvent startPeriodicEventsAfterDelay: 0.02 withPeriod: 0.03];

  // --- Loop that handles all events during drag operation -----------
  while ([theEvent type] != NSLeftMouseUp)
    {
      [self _handleEventDuringDragging: theEvent];

      theEvent = [NSApp nextEventMatchingMask: eventMask
				    untilDate: theDistantFuture
				       inMode: NSEventTrackingRunLoopMode
				      dequeue: YES];
    }

  // --- Event loop for drag operation stopped ------------------------
  [NSEvent stopPeriodicEvents];
  [self _updateAndMoveImageToCorrectPosition];

  NSDebugLLog(@"NSDragging", @"dnd ending %d\n", targetWindowRef);

  // --- Deposit the drop ----------------------------------------------
  if ((targetWindowRef != 0)
    && ((targetMask & dragMask & operationMask) != NSDragOperationNone))
    {
      /* FIXME:
       * We remove the dragged image from the screen before 
       * sending the dnd drop event to the destination.
       * This code should actually be rewritten, because
       * the depositing of the drop consist of three steps
       *  - prepareForDragOperation
       *  - performDragOperation
       *  - concludeDragOperation.
       * The dragged image should be removed from the screen
       * between the prepare and the perform operation.
       * The three steps are now executed in the NSWindow class
       * and the NSWindow class does not have access to
       * the image.
       */
      [self _clearupWindow];
      [cursorBeforeDrag set];
      NSDebugLLog(@"NSDragging", @"sending dnd drop\n");
      if (!destExternal)
        {
          [self _sendLocalEvent: GSAppKitDraggingDrop
                         action: 0
                       position: dragPosition
		                  timestamp: [theEvent timestamp]
                       toWindow: destWindow];
        }
      else
        {
          [self sendExternalEvent: GSAppKitDraggingDrop
                           action: 0
		                     position: dragPosition
		                    timestamp: [theEvent timestamp]
		                     toWindow: targetWindowRef];
        }
      deposited = YES;
    }
  else
    {
      if (slideBack)
        {
          [self slideDraggedImageTo: slidePoint];
        }
      [self _clearupWindow];
      [cursorBeforeDrag set];
      deposited = NO;
    }

  if ([dragSource respondsToSelector:
                      @selector(draggedImage:endedAt:operation:)])
     {
       NSPoint point;
           
       point = [theEvent locationInWindow];
       // Convert from mouse cursor coordinate to image coordinate
       point.x -= offset.width;
       point.y -= offset.height;
       point = [[theEvent window] convertBaseToScreen: point];
       [dragSource draggedImage: [self draggedImage]
                        endedAt: point
                      operation: targetMask & dragMask & operationMask];
     }
   else if ([dragSource respondsToSelector:
                            @selector(draggedImage:endedAt:deposited:)])
    {
      NSPoint point;
          
      point = [theEvent locationInWindow];
      // Convert from mouse cursor coordinate to image coordinate
      point.x -= offset.width;
      point.y -= offset.height;
      point = [[theEvent window] convertBaseToScreen: point];
      [dragSource draggedImage: [self draggedImage]
                       endedAt: point
                     deposited: deposited];
    }
}

/*
 * Handle the events for the event loop during drag and drop
 */
- (void) _handleEventDuringDragging: (NSEvent *)theEvent
{
  switch ([theEvent type])
    {
    case  NSAppKitDefined:
      {
        GSAppKitSubtype	sub = [theEvent subtype];
        
        switch (sub)
        {
        case GSAppKitWindowMoved:
        case GSAppKitWindowResized:
        case GSAppKitRegionExposed:
          /*
           * Keep window up-to-date with its current position.
           */
          [NSApp sendEvent: theEvent];
          break;
          
        case GSAppKitDraggingStatus:
          NSDebugLLog(@"NSDragging", @"got GSAppKitDraggingStatus\n");
          if ((int)[theEvent data1] == targetWindowRef)
            {
              NSDragOperation newTargetMask = (NSDragOperation)[theEvent data2];

              if (newTargetMask != targetMask)
                {
                  targetMask = newTargetMask;
                  [self _setCursor];
                }
            }
          break;
          
        case GSAppKitDraggingFinished:
          NSLog(@"Internal: got GSAppKitDraggingFinished out of seq");
          break;
          
        case GSAppKitWindowFocusIn:
        case GSAppKitWindowFocusOut:
        case GSAppKitWindowLeave:
        case GSAppKitWindowEnter:
          break;

        default:
          NSDebugLLog(@"NSDragging", @"dropped NSAppKitDefined (%d) event", sub);
          break;
        }
      }
      break;
      
    case NSMouseMoved:
    case NSLeftMouseDragged:
    case NSLeftMouseDown:
    case NSLeftMouseUp:
      newPosition = [[theEvent window] convertBaseToScreen:
                       [theEvent locationInWindow]];
      break;
    case NSFlagsChanged:
      if ([self _updateOperationMask: theEvent])
        {
          // If flags change, send update to allow
          // destination to take note.
          if (destWindow)
            {
              [self _sendLocalEvent: GSAppKitDraggingUpdate
                             action: dragMask & operationMask
                           position: newPosition
                          timestamp: [theEvent timestamp]
                           toWindow: destWindow];
            }
          else
            {
              [self sendExternalEvent: GSAppKitDraggingUpdate
                               action: dragMask & operationMask
                             position: newPosition
                            timestamp: [theEvent timestamp]
                             toWindow: targetWindowRef];
            }
          [self _setCursor];
        }
      break;
    case NSPeriodic:
      newPosition = [NSEvent mouseLocation];
      if (newPosition.x != dragPosition.x || newPosition.y != dragPosition.y) 
        {
          [self _updateAndMoveImageToCorrectPosition];
        }
      else if (destWindow)
        {
	  [self _sendLocalEvent: GSAppKitDraggingUpdate
                         action: dragMask & operationMask
                       position: newPosition
                      timestamp: [theEvent timestamp]
                       toWindow: destWindow];
	}
      else
        {
          [self sendExternalEvent: GSAppKitDraggingUpdate
                           action: dragMask & operationMask
                         position: newPosition
                        timestamp: [theEvent timestamp]
                         toWindow: targetWindowRef];
        }
      break;
    default:
      NSLog(@"Internal: dropped event (%d) during dragging", (int)[theEvent type]);
    }
}
  
/*
 * This method will move the drag image and update all associated data
 */
- (void) _updateAndMoveImageToCorrectPosition
{
  //--- Store old values -----------------------------------------------------
  NSWindow *oldDestWindow = destWindow;
  BOOL oldDestExternal = destExternal;
  int mouseWindowRef; 
  BOOL changeCursor = NO;
 
  //--- Move drag image to the new position -----------------------------------
  [self _moveDraggedImageToNewPosition];

  if ([dragSource respondsToSelector:
              @selector(draggedImage:movedTo:)])
    {
      [dragSource draggedImage: [self draggedImage] movedTo: dragPosition];
    }

  //--- Determine target window ---------------------------------------------
  destWindow = [self windowAcceptingDnDunder: dragPosition
                                   windowRef: &mouseWindowRef];

  // If we are not hovering above a window that we own
  // we are dragging to an external application.
  destExternal = (mouseWindowRef != 0) && (destWindow == nil);
            
  if (destWindow != nil)
    {
      dragPoint = [destWindow convertScreenToBase: dragPosition];
    }
            
  NSDebugLLog(@"NSDragging", @"mouse window %d (%@) at %@\n",
    mouseWindowRef, destWindow, NSStringFromPoint(dragPosition));
            
  //--- send exit message if necessary -------------------------------------
  if ((mouseWindowRef != targetWindowRef) && targetWindowRef)
    {
      /* If we change windows and the old window is dnd aware, we send an
         dnd exit */
      NSDebugLLog(@"NSDragging", @"sending dnd exit\n");
                
      if (oldDestWindow != nil)   
        {
          [self _sendLocalEvent: GSAppKitDraggingExit
                         action: dragMask & operationMask
                       position: NSZeroPoint
                      timestamp: dragSequence
                       toWindow: oldDestWindow];
        }  
      else
        {  
          [self sendExternalEvent: GSAppKitDraggingExit
                           action: dragMask & operationMask
                         position: NSZeroPoint
                        timestamp: dragSequence
                         toWindow: targetWindowRef];
        }
    }

  //  Reset drag mask when we switch from external to internal or back
  if (oldDestExternal != destExternal)
    {
      NSDragOperation newMask;

      if ([dragSource respondsToSelector:
                        @selector(draggingSourceOperationMaskForLocal:)])
        {
          newMask = [dragSource draggingSourceOperationMaskForLocal: !destExternal];
        }
      else
        {
          newMask = NSDragOperationCopy | NSDragOperationLink |
            NSDragOperationGeneric | NSDragOperationPrivate;
        }

      if (newMask != dragMask)
        {
          dragMask = newMask;
          changeCursor = YES;
        }
    }

  if (mouseWindowRef == targetWindowRef && targetWindowRef)  
    { 
      // same window, sending update
      NSDebugLLog(@"NSDragging", @"sending dnd pos\n");

      // FIXME: We should only send this when the destination wantsPeriodicDraggingUpdates
      if (destWindow != nil)
        {
          [self _sendLocalEvent: GSAppKitDraggingUpdate
                         action: dragMask & operationMask
                       position: dragPosition
                      timestamp: dragSequence
                       toWindow: destWindow];
        }
      else 
        {
          [self sendExternalEvent: GSAppKitDraggingUpdate 
                           action: dragMask & operationMask
                         position: dragPosition
                        timestamp: dragSequence
                         toWindow: targetWindowRef];
        }
    }
  else if (mouseWindowRef != 0)
    {
      // FIXME: We might force the cursor update here, if the
      // target wants to change the cursor.
      NSDebugLLog(@"NSDragging", @"sending dnd enter/pos\n");
      
      if (destWindow != nil)
        {
          [self _sendLocalEvent: GSAppKitDraggingEnter
                         action: dragMask
                       position: dragPosition
                      timestamp: dragSequence
                       toWindow: destWindow];
        }
      else
        {
          [self sendExternalEvent: GSAppKitDraggingEnter
                           action: dragMask
                         position: dragPosition
                        timestamp: dragSequence
                         toWindow: mouseWindowRef];
        }
    }

  if (targetWindowRef != mouseWindowRef)
    {
      targetWindowRef = mouseWindowRef;
      changeCursor = YES;
    }
  
  if (changeCursor)
    {
      [self _setCursor];
    }
}

/*
 * Move the dragged image immediately to the position indicated by
 * the instance variable newPosition.
 *
 * In doing so it will update the dragPosition instance variables.
 */
- (void) _moveDraggedImageToNewPosition
{
  dragPosition = newPosition;
  [GSServerForWindow(_window) movewindow:
    NSMakePoint(newPosition.x - offset.width, newPosition.y - offset.height) 
    : [_window windowNumber]];
}


/*
 * NB. screenPoint here is the position of the mouse cursor.
 */
- (void) _slideDraggedImageTo: (NSPoint)screenPoint
                numberOfSteps: (int)steps
			delay: (float)delay
               waitAfterSlide: (BOOL)waitFlag
{
  /* If we do not need multiple redrawing, just move the image immediately
   * to its desired spot.
   */
  if (steps < 2)
    {
      newPosition = screenPoint;
      [self _moveDraggedImageToNewPosition];
    }
  else
    {
      [NSEvent startPeriodicEventsAfterDelay: delay withPeriod: delay];

      // Use the event loop to redraw the image repeatedly.
      // Using the event loop to allow the application to process
      // expose events.  
      while (steps)
        {
          NSEvent *theEvent = [NSApp nextEventMatchingMask: NSPeriodicMask
                                     untilDate: [NSDate distantFuture]
                                     inMode: NSEventTrackingRunLoopMode
                                     dequeue: YES];
          
          if ([theEvent type] != NSPeriodic)
            {
              NSDebugLLog (@"NSDragging", 
			   @"Unexpected event type: %d during slide",
                           (int)[theEvent type]);
            }
          newPosition.x = (screenPoint.x + ((float) steps - 1.0) 
			   * dragPosition.x) / ((float) steps);
          newPosition.y = (screenPoint.y + ((float) steps - 1.0) 
			   * dragPosition.y) / ((float) steps);

          [self _moveDraggedImageToNewPosition];
          steps--;
        }
      [NSEvent stopPeriodicEvents];
    }

  if (waitFlag)
    {
      [NSThread sleepUntilDate: 
	[NSDate dateWithTimeIntervalSinceNow: delay * 2.0]];
    }
}

@end
