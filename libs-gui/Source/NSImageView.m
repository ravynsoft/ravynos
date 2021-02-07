/** <title>NSImageView</title>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: January 1998
   Updated by: Jonathan Gapen <jagapen@smithlab.chem.wisc.edu>
   Date: May 1999
   
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

#import "AppKit/NSDragging.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSImageCell.h"
#import "AppKit/NSImageView.h"
#import "AppKit/NSMenuItem.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSWindow.h"

/*
 * Class variables
 */
static Class usedCellClass;
static Class imageCellClass;

@implementation NSImageView

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSImageView class])
    {
      [self setVersion: 2];
      imageCellClass = [NSImageCell class];
      usedCellClass = imageCellClass;
    }
}

/*
 * Setting the Cell class
 */
+ (Class) cellClass
{
  return usedCellClass;
}

+ (void) setCellClass: (Class)factoryId
{
  usedCellClass = factoryId ? factoryId : imageCellClass;
}


- (id) initWithFrame: (NSRect)aFrame
{
  self = [super initWithFrame: aFrame];
  if (!self)
    return self;

  // set the default values
  [self setImageAlignment: NSImageAlignCenter];
  [self setImageFrameStyle: NSImageFrameNone];
  [self setImageScaling: NSScaleProportionally];
  [self setEditable: NO];
  [self setAllowsCutCopyPaste: YES];

  return self;
}

- (void) setImage: (NSImage *)image
{
  [_cell setImage: image];
  [self updateCell: _cell];
}

- (void) setImageAlignment: (NSImageAlignment)align
{
  [_cell setImageAlignment: align];
  [self updateCell: _cell];
}

- (void) setImageScaling: (NSImageScaling)scaling
{
  [_cell setImageScaling: scaling];
  [self updateCell: _cell];
}

- (void) setImageFrameStyle: (NSImageFrameStyle)style
{
  [_cell setImageFrameStyle: style];
  [self updateCell: _cell];
}

- (void) setEditable: (BOOL)flag
{
  [_cell setEditable: flag];
  if (flag)
    {
      [self registerForDraggedTypes: [NSImage imagePasteboardTypes]];
    }
  else
    {
      [self unregisterDraggedTypes];
    }
}

- (NSImage *) image
{
  return [_cell image];
}

- (NSImageAlignment) imageAlignment
{
  return [_cell imageAlignment];
}

- (NSImageScaling) imageScaling
{
  return [_cell imageScaling];
}

- (NSImageFrameStyle) imageFrameStyle
{
  return [_cell imageFrameStyle];
}

- (BOOL) isEditable
{
  return [_cell isEditable];
}

- (BOOL) animates
{
  // FIXME: Should be passed on to cell.
  return NO;
}

- (void) setAnimates: (BOOL) flag
{
  // FIXME: Should be passed on to cell.
}

- (BOOL) allowsCutCopyPaste
{
  return _ivflags.allowsCutCopyPaste;
}

- (void) setAllowsCutCopyPaste: (BOOL)flag
{
  _ivflags.allowsCutCopyPaste = flag;
}

- (void) delete: (id)sender
{
  if ([self allowsCutCopyPaste])
    [self setImage: nil];
}

- (void) deleteBackward: (id)sender
{
  if ([self allowsCutCopyPaste])
    [self setImage: nil];
}

- (void) copy: (id)sender
{
  if ([self allowsCutCopyPaste])
    {
      NSImage *anImage = [self image];

      if (anImage != nil)
        {
          // copy to pasteboard
          NSPasteboard *pboard = [NSPasteboard generalPasteboard];
          
          [pboard declareTypes: [NSArray arrayWithObject: NSTIFFPboardType] 
                         owner: self];
          [pboard setData: [anImage TIFFRepresentation]
                  forType: NSTIFFPboardType];
        }
    }
}

- (void) cut: (id)sender
{
  if ([self allowsCutCopyPaste])
    {
      [self copy: sender];
      [self setImage: nil];
    }
}

- (void) paste: (id)sender
{
  if ([self allowsCutCopyPaste])
    {
      // paste from pasteboard
      NSPasteboard *pboard = [NSPasteboard generalPasteboard];
      NSImage *image = [[NSImage alloc] initWithPasteboard: pboard];

      if (image != nil)
        {
          [self setImage: image];
          RELEASE(image);
        }
    }
}

- (BOOL) validateUserInterfaceItem: (id <NSValidatedUserInterfaceItem>)anItem
{
  SEL action = [anItem action];

  if ([self allowsCutCopyPaste])
    {
      if (sel_isEqual(action, @selector(cut:)) ||
          sel_isEqual(action, @selector(copy:)) ||
          sel_isEqual(action, @selector(deleteBackward:)) ||
          sel_isEqual(action, @selector(delete:)))
        return [self image] != nil;
      if (sel_isEqual(action, @selector(paste:)))
        {
          return [NSImage canInitWithPasteboard: 
                            [NSPasteboard generalPasteboard]];
        }
    }
  return NO;
}

- (BOOL) validateMenuItem: (NSMenuItem *)anItem
{
  return [self validateUserInterfaceItem: anItem];
}

- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>)sender
{
  if (([sender draggingSource] != self) && ([self isEditable])
    && ([NSImage canInitWithPasteboard: [sender draggingPasteboard]]))
    {
      [_cell setHighlighted: YES];
      return NSDragOperationCopy;
    }
  else
    {
      return NSDragOperationNone;
    }
}

// NSDraggingDestination protocol

- (void) draggingExited: (id <NSDraggingInfo>)sender
{
  [_cell setHighlighted: NO];
}

- (BOOL) prepareForDragOperation: (id <NSDraggingInfo>)sender
{
  if (([sender draggingSource] != self) && ([self isEditable]))
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

- (BOOL) performDragOperation: (id <NSDraggingInfo>)sender
{
  NSImage *image;

  image = [[NSImage alloc] initWithPasteboard: [sender draggingPasteboard]];
  if (image == nil)
    {
      return NO;
    }
  else 
    {
      [self setImage: image];
      [self sendAction: _action to: _target];
      RELEASE(image);
      return YES;
    }
}

- (void) concludeDragOperation: (id <NSDraggingInfo>)sender
{
  [_cell setHighlighted: NO];
}

- (void) mouseDown: (NSEvent*)theEvent
{
  if ([self initiatesDrag])
    {
      NSPasteboard *pboard;
      NSImage *anImage = [self image];

      if (anImage != nil)
        {
	  pboard = [NSPasteboard pasteboardWithName: NSDragPboard];
	  [pboard declareTypes: [NSArray arrayWithObject: NSTIFFPboardType] 
		  owner: self];
	  if ([pboard setData: [anImage TIFFRepresentation]
		      forType: NSTIFFPboardType])
	    {
	      NSSize	s;
	      NSPoint	p;

	      // Center the image on the mouse position ... is this right?
	      s = [anImage size];
	      p = [theEvent locationInWindow];
	      p.x -= s.width/2;
	      p.y -= s.height/2;
	      [_window dragImage: anImage
		       at: p
		       offset: NSMakeSize(0, 0)
		       event: theEvent
		       pasteboard: pboard
		       source: self
		       slideBack: YES];
	      return;
	    }
	}
    }
  [super mouseDown: theEvent];
}

- (NSDragOperation) draggingSourceOperationMaskForLocal: (BOOL)isLocal
{
  return NSDragOperationCopy;
}

//
//  Target and Action
//
//  Target and action are handled by NSImageView itself, not its own cell.
//
- (id) target
{
  return _target;
}

- (void) setTarget: (id)anObject
{
  _target = anObject;
}

- (SEL) action
{
  return _action;
}

- (void) setAction: (SEL)aSelector
{
  _action = aSelector;
}

//
//  NSCoding Protocol
//
- (void) encodeWithCoder: (NSCoder *)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: [NSImage imagePasteboardTypes] 
                    forKey: @"NSDragTypes"];
      [aCoder encodeBool: [self isEditable] forKey: @"NSEditable"];
    }
  else
    {
      [aCoder encodeConditionalObject: _target];
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_action];
    }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return self;

  [self setAllowsCutCopyPaste: YES];
  if ([aDecoder allowsKeyedCoding])
    {
      //NSArray *dragType = [aDecoder decodeObjectForKey: @"NSDragTypes"];
      if ([aDecoder containsValueForKey: @"NSEditable"])
        {
	  [self setEditable: [aDecoder decodeBoolForKey: @"NSEditable"]];
	}
    }
  else
    {
      if ([aDecoder versionForClassName: @"NSImageView"] >= 2)
	{
	  _target = [aDecoder decodeObject];
	  [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_action];
	}
    }
  return self;
}

@end

@implementation NSImageView (GNUstep)

- (BOOL)initiatesDrag
{
  return _ivflags.initiatesDrag;
}

- (void)setInitiatesDrag: (BOOL)flag
{
  _ivflags.initiatesDrag = flag;
}

@end
