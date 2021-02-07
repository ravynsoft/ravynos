/* 
   NSToolbarItem.m

   The Toolbar item class.
   
   Copyright (C) 2002 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>,
            Fabien Vallon <fabien.vallon@fr.alcove.com>,
            Quentin Mathe <qmathe@club-internet.fr>
   Date: May 2002
   
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
#import <Foundation/NSArchiver.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSDragging.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuItem.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSToolbar.h"
#import "AppKit/NSView.h"
#import "GNUstepGUI/GSToolbarView.h"
#import "AppKit/NSToolbarItem.h"

#import "NSToolbarFrameworkPrivate.h"
#import "GSGuiPrivate.h"

/*
 * Each NSToolbarItem object are coupled with a backView which is their 
 * representation on the screen.
 * backView for the standard toolbar item (without custom view) are NSButton 
 * subclass called GSToolbarButton.
 * backView for the toolbar item with a custom view are NSView subclass called
 * GSToolbarBackView.
 * GSToolbarButton and GSToolbarBackView are adjusted according to their content
 * and their title when the method layout is called.
 * The predefined GNUstep toolbar items are implemented with a class cluster 
 * pattern: initWithToolbarItemIdentifier: returns differents concrete subclass 
 * in accordance with the item identifier.
 */
 
typedef enum {
  ItemBackViewDefaultHeight = 60,
  ItemBackViewRegularHeight = 60,
  ItemBackViewSmallHeight = 50
} ItemBackViewHeight;

typedef enum {
  ItemBackViewDefaultWidth = 60,
  ItemBackViewRegularWidth = 60,
  ItemBackViewSmallWidth = 50
} ItemBackViewWidth;

static const int ItemBackViewX = 0;
static const int ItemBackViewY = 0;
static const int InsetItemViewX = 10;
// static const int InsetItemViewY = 26;
static const int InsetItemTextX = 3;
static const int InsetItemTextY = 4;
 
// See NSToolbarItem -initialize method
static NSFont *NormalFont = nil;
static NSFont *SmallFont = nil;

NSString *GSMovableToolbarItemPboardType = @"GSMovableToolbarItemPboardType";

/*
 * NSButton subclass is the toolbar buttons _backView
 */
@interface GSToolbarButton : NSButton
{
  NSToolbarItem *_toolbarItem;
}

- (id) initWithToolbarItem: (NSToolbarItem *)toolbarItem;
- (void) layout;

// Accessors
- (NSToolbarItem *) toolbarItem;
@end

@interface GSToolbarButtonCell : NSButtonCell
{
  NSRect titleRect;
  NSRect imageRect;
}

@end

// ---

@implementation GSToolbarButton

+ (Class) cellClass
{
  return [GSToolbarButtonCell class];
}  

- (id) initWithToolbarItem: (NSToolbarItem *)toolbarItem
{ 
  // Frame will be reset by the layout method
  self = [super initWithFrame: NSMakeRect(ItemBackViewX, ItemBackViewY,
    ItemBackViewDefaultWidth, ItemBackViewDefaultHeight)]; 
  
  if (self != nil)
    {
      // Don't do an ASSIGN here, the toolbar item itself retains us.
      _toolbarItem = toolbarItem;

      [self setTitle: @""];
      [self setEnabled: NO];
      [_cell setBezeled: YES];
      [self setImagePosition: NSImageAbove];
      [self setHighlightsBy: 
                NSChangeGrayCellMask | NSChangeBackgroundCellMask];
      [self setFont: NormalFont]; 
    }

  return self;   
}

/*
 * The code below should be kept in sync with GSToolbarBackView methods which
 * have identical names.
 */
 
- (void) layout
{
  float textWidth, layoutedWidth = -1, layoutedHeight = -1;
  NSFont *font;
  unsigned int borderMask = [[[_toolbarItem toolbar] _toolbarView] borderMask];
  NSSize labelSize = NSZeroSize;

  font = NormalFont;
  
  // Adjust the layout in accordance with NSToolbarSizeMode
  switch ([[_toolbarItem toolbar] sizeMode])
    {
      case NSToolbarSizeModeDefault:
        layoutedWidth = ItemBackViewDefaultWidth;
        layoutedHeight = ItemBackViewDefaultHeight;
        [[_toolbarItem image] setSize: NSMakeSize(32, 32)];
        break;
      case NSToolbarSizeModeRegular:
        layoutedWidth = ItemBackViewRegularWidth;
        layoutedHeight = ItemBackViewRegularHeight;
        [[_toolbarItem image] setSize: NSMakeSize(32, 32)];
        break;
      case NSToolbarSizeModeSmall:
        layoutedWidth = ItemBackViewSmallWidth;
        layoutedHeight = ItemBackViewSmallHeight;
        /* Not use [self image] here because it can return nil, when image 
           position is set to NSNoImage. Even if NSToolbarDisplayModeTextOnly 
           is not true anymore -setImagePosition: is only called below, then 
           [self image] can still returns nil. */
        [[_toolbarItem image] setSize: NSMakeSize(24, 24)];
        font = SmallFont;
        break;
      default:
        NSLog(@"Invalid NSToolbarSizeMode"); // Invalid
    }
    
  [self setFont: font];
  
  // Adjust the layout in accordance with the border
  if (!(borderMask & GSToolbarViewBottomBorder))
    {
      layoutedHeight++;
      layoutedWidth++;
    }

  if (!(borderMask & GSToolbarViewTopBorder))
    {
      layoutedHeight++;
      layoutedWidth++; 
    }
             
  // Adjust the layout in accordance with the label
  {
    NSAttributedString *attrStr;
    NSDictionary *attr;
    NSString *label = [_toolbarItem label];

    attr = [NSDictionary dictionaryWithObject: font forKey: NSFontAttributeName];
    if (label == nil || [label isEqualToString: @""])
      label = @"Dummy";
    attrStr = [[NSAttributedString alloc] initWithString: label attributes: attr];
    labelSize = [attrStr size];
    DESTROY(attrStr);
  }
      
  textWidth = labelSize.width + 2 * InsetItemTextX;
  if ([[_toolbarItem toolbar] displayMode] != NSToolbarDisplayModeIconOnly 
    && layoutedWidth != -1 && textWidth > layoutedWidth) 
     layoutedWidth = textWidth;
     
  // Adjust the layout in accordance with NSToolbarDisplayMode
  switch ([[_toolbarItem toolbar] displayMode])
    {
      case NSToolbarDisplayModeDefault:
        [self setImagePosition: NSImageAbove];
        break;
      case NSToolbarDisplayModeIconAndLabel:
        [self setImagePosition: NSImageAbove];
        break;
      case NSToolbarDisplayModeIconOnly:
        [self setImagePosition: NSImageOnly];
        layoutedHeight -= labelSize.height + InsetItemTextY;
        break;
      case NSToolbarDisplayModeLabelOnly:
        [self setImagePosition: NSNoImage];
        layoutedHeight = labelSize.height + InsetItemTextY * 2;
        break;
      default:
        ; // Invalid
    }
      
  // Set the frame size to use the new layout
  [self setFrameSize: NSMakeSize(layoutedWidth, layoutedHeight)];
}

- (void) mouseDown: (NSEvent *)event
{
  NSToolbar *toolbar = [_toolbarItem toolbar];
  
  if (([event modifierFlags] == NSCommandKeyMask 
       && [toolbar allowsUserCustomization])
      || [toolbar customizationPaletteIsRunning] || toolbar == nil)
    {          
      NSSize viewSize = [self frame].size;
      NSImage *image = [[NSImage alloc] initWithSize: viewSize];
      NSCell *cell = [self cell];
      NSPasteboard *pboard;
      NSInteger index = NSNotFound;
          
      // Prepare the drag
      
      /* We need to keep this view (aka self) to be able to draw the drag
         image. */
      RETAIN(self); 
      
      // Draw the drag content in an image
      /* The code below is only partially supported by GNUstep, then NSImage
         needs to be improved. */
      [image lockFocus];
      [cell setShowsFirstResponder: NO]; // To remove the dotted rect
      [cell drawWithFrame: 
                NSMakeRect(0, 0, viewSize.width, viewSize.height) inView: nil];
      [cell setShowsFirstResponder: YES];
      [image unlockFocus];
      
      pboard = [NSPasteboard pasteboardWithName: NSDragPboard];
      [pboard declareTypes: [NSArray arrayWithObject: GSMovableToolbarItemPboardType] 
              owner: nil];
      if (toolbar != nil)
	{
          index = [toolbar _indexOfItem: _toolbarItem];
        }
	  [GSToolbarView setDraggedItemIndex:index];
      [pboard setString: [NSString stringWithFormat:@"%ld", (long) index] 
              forType: GSMovableToolbarItemPboardType];
          
      [self dragImage: image 
            at: NSMakePoint(0.0, viewSize.height)
            offset: NSMakeSize(0.0, 0.0)
            event: event 
            pasteboard: pboard 
            source: self
            slideBack: NO];          
      RELEASE(image);
    }
  else if ([event modifierFlags] != NSCommandKeyMask)
    {
      [super mouseDown: event];
    }
}

- (void) draggedImage: (NSImage *)dragImage beganAt: (NSPoint)location
{
  //nothing to do
}

- (void) draggedImage: (NSImage *)dragImage 
              endedAt: (NSPoint)location 
            operation: (NSDragOperation)operation
{
  //nothing to do
}

- (NSDragOperation) draggingSourceOperationMaskForLocal: (BOOL)isLocal
{
  return isLocal ? NSDragOperationGeneric : NSDragOperationNone;
}

- (NSToolbarItem *) toolbarItem
{
  return _toolbarItem;
}

/*
 * End of the code to keep in sync
 */

- (BOOL) sendAction: (SEL)action to: (id)target
{ 
  if ([_toolbarItem _selectable])
    {
      [[_toolbarItem toolbar] 
        setSelectedItemIdentifier: [_toolbarItem itemIdentifier]];
    }
  
  if (action)
    {
      // Send from toolbar item not self
      return [NSApp sendAction: action 
                            to: target 
                          from: _toolbarItem];
    }
  else
    {
      return NO;
    }
}

@end

@implementation GSToolbarButtonCell

/* Overriden NSButtonCell method to handle cell type in a basic way which avoids
   to lose image or empty title on new image position (when this involves a cell
   type switch) and the need to reset it. That would happen in GSToolbarButton
   -layout method (on toolbar display mode switch).
   Note that empty title are used with space or separator toolbar items. */
- (void) setImagePosition: (NSCellImagePosition)aPosition
{
  _cell.image_position = aPosition;

  if (_cell.image_position == NSNoImage)
    {
      _cell.type = NSTextCellType;
    }
  else
    {
      _cell.type = NSImageCellType;
    }
}

/* Allways return the image, even when no image gets displayed. */
- (NSImage*) image
{
  return _cell_image;
}

// Overriden NSButtonCell method to make sure all text is at the same height.
- (void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  BOOL flippedView = [controlView isFlipped];
  NSCellImagePosition ipos = _cell.image_position;
  // We ignore alternateAttributedTitle, it is not needed
  NSSize titleSize = [[self attributedTitle] size];
  
  if (flippedView == YES)
    {
      if (ipos == NSImageAbove)
        {
          ipos = NSImageBelow;
        }
      else if (ipos == NSImageBelow)
        {
          ipos = NSImageAbove;
        }
    }

  /* We store the values we need to customize the drawing into titleRect and 
     imageRect. */
  switch (ipos)
    {
      case NSNoImage: 
        titleRect = cellFrame;
        break;

      case NSImageOnly: 
        imageRect = cellFrame;
        break;

      default:
      case NSImageBelow: 
        titleRect.origin.x = cellFrame.origin.x;
        titleRect.origin.y = NSMaxY(cellFrame) - titleSize.height - InsetItemTextY;
        titleRect.size.width = cellFrame.size.width;
        titleRect.size.height = titleSize.height;
        
        imageRect.origin.x = cellFrame.origin.x;
        imageRect.origin.y = cellFrame.origin.y;
        imageRect.size.width = cellFrame.size.width;
        imageRect.size.height = cellFrame.size.height - titleRect.size.height;
        break;

      case NSImageAbove: 
        titleRect.origin.x = cellFrame.origin.x;
        titleRect.origin.y = cellFrame.origin.y + InsetItemTextY;
        titleRect.size.width = cellFrame.size.width;
        titleRect.size.height = titleSize.height;
        
        imageRect.origin.x = cellFrame.origin.x;
        imageRect.origin.y = cellFrame.origin.y + titleRect.size.height;
        imageRect.size.width = cellFrame.size.width;
        imageRect.size.height = cellFrame.size.height - titleRect.size.height;
        break;
    }

  [super drawInteriorWithFrame: cellFrame inView: controlView];
}

// Overriden NSCell method
- (void) _drawAttributedText: (NSAttributedString*)aString 
                     inFrame: (NSRect)aRect
{
  if (aString == nil)
    return;

  /* Important: text should always be vertically centered without considering 
     descender (as if descender did not exist). This is particularly important 
     for single line texts.Please make sure the output remains always 
     correct. */

  [aString drawInRect: titleRect]; // We ignore aRect value
}

// Overriden NSButtonCell method
- (void) drawImage: (NSImage *)anImage withFrame: (NSRect)aRect inView: (NSView*)controlView
{
  // We ignore aRect value
  [super drawImage: anImage withFrame: imageRect inView: controlView];
}

@end

/*
 * Back view used to enclose toolbar item's custom view
 */
@interface GSToolbarBackView : NSView
{
  NSToolbarItem *_toolbarItem;
  NSFont *_font;
  BOOL _enabled;
  BOOL _showLabel;
  // record the fact that the view responds to these
  // to save time.
  struct __flags
  {
    // gets
    unsigned int _isEnabled:1;
    unsigned int _action:1;
    unsigned int _target:1;
    unsigned int _image:1;
    // sets
    unsigned int _setEnabled:1;
    unsigned int _setAction:1;
    unsigned int _setTarget:1;
    unsigned int _setImage:1;

    // to even out the int.
    unsigned int RESERVED:24;
  } _flags;
}

- (id) initWithToolbarItem: (NSToolbarItem *)toolbarItem;
- (void) layout;

- (NSToolbarItem *) toolbarItem;
- (BOOL) isEnabled;
- (void) setEnabled: (BOOL)enabled;
@end

@implementation GSToolbarBackView

- (id) initWithToolbarItem: (NSToolbarItem *)toolbarItem
{  
  self = [super initWithFrame: NSMakeRect(ItemBackViewX, ItemBackViewY, 
    ItemBackViewDefaultWidth, ItemBackViewDefaultHeight)];
  // Frame will be reset by the layout method
  
  if (self != nil)
    {  
      NSView *view;

      // Don't do an ASSIGN here, the toolbar item itself retains us.
      _toolbarItem = toolbarItem;
      view = [toolbarItem view];

      // gets
      _flags._isEnabled  = [view respondsToSelector: @selector(isEnabled)];
      _flags._action     = [view respondsToSelector: @selector(action)];
      _flags._target     = [view respondsToSelector: @selector(target)];
      _flags._image      = [view respondsToSelector: @selector(image)];
      // sets
      _flags._setEnabled = [view respondsToSelector: @selector(setEnabled:)];
      _flags._setAction  = [view respondsToSelector: @selector(setAction:)];
      _flags._setTarget  = [view respondsToSelector: @selector(setTarget:)];
      _flags._setImage   = [view respondsToSelector: @selector(setImage:)];
    }
  
  return self;
}

- (void) drawRect: (NSRect)rect
{  
  if (_showLabel && NSIntersectsRect(rect, [self bounds]))
    {
      NSAttributedString *attrString;
      NSDictionary *attr;
      NSColor *color;
      NSMutableParagraphStyle *pStyle;
      NSRect titleRect;
      NSRect viewBounds = [self bounds];

      if (_enabled)
        {
          color = [NSColor blackColor];
        }
      else
        {
          color = [NSColor disabledControlTextColor];
        }
        
      pStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
      [pStyle setAlignment: NSCenterTextAlignment];
      
      // We draw the label
      attr = [NSDictionary dictionaryWithObjectsAndKeys: _font, 
        NSFontAttributeName, color, NSForegroundColorAttributeName, pStyle,
        NSParagraphStyleAttributeName, nil];
      RELEASE(pStyle);

      attrString = [[NSAttributedString alloc] 
        initWithString: [_toolbarItem label] attributes: attr];
      
      titleRect.origin.x = viewBounds.origin.x;
      titleRect.origin.y = viewBounds.origin.y + InsetItemTextY;
      titleRect.size.width = viewBounds.size.width;
      titleRect.size.height = [attrString size].height;
      [attrString drawInRect: titleRect];
      
      DESTROY(attrString);
   }
}

- (NSView *) hitTest: (NSPoint)point
{
  if (_super_view && [_super_view mouse: point inRect: _frame])
    {
      NSEvent *event = [NSApp currentEvent];
      NSToolbar *toolbar = [_toolbarItem toolbar];

      if (([event type] == NSLeftMouseDown)
	  && (([event modifierFlags] == NSCommandKeyMask
	      && [toolbar allowsUserCustomization])
	      || [toolbar customizationPaletteIsRunning] || toolbar == nil))
	{
	  return self;
	}
    }

  return [super hitTest: point];
}

/*
 * The code below should be kept in sync with GSToolbarButton methods which 
 * have identical names.
 */
  
- (void) layout
{
  NSView *view = [_toolbarItem view];
  float textWidth, layoutedWidth = -1, layoutedHeight = -1;
  NSFont *font;
  unsigned int borderMask = [[[_toolbarItem toolbar] _toolbarView] borderMask];
  NSSize labelSize = NSZeroSize;
  
  font = NormalFont;
  
  if ([view superview] == nil) // Show the view to eventually hide it later
    [self addSubview: view];
	
  if ([view respondsToSelector: @selector(sizeToFit)])
    {
      NSSize newSize, minSize = [_toolbarItem minSize];
      
      [(id)view sizeToFit];
      newSize = [view frame].size;
      if (minSize.width > 0 || newSize.height < minSize.height)
        {
          if (minSize.width > 0)
			newSize.width = minSize.width;
          newSize.height = MAX(newSize.height, minSize.height);
          [view setFrameSize: newSize];
        }
    }

  // Adjust the layout in accordance with NSToolbarSizeMode
  switch ([[_toolbarItem toolbar] sizeMode])
    {
      case NSToolbarSizeModeDefault:
        layoutedWidth = ItemBackViewDefaultWidth;
        layoutedHeight = ItemBackViewDefaultHeight;
        if ([view frame].size.height > 32)
          [view removeFromSuperview];
        break;
      case NSToolbarSizeModeRegular:
        layoutedWidth = ItemBackViewRegularWidth;
        layoutedHeight = ItemBackViewRegularHeight;
        if ([view frame].size.height > 32)
          [view removeFromSuperview];
        break;
      case NSToolbarSizeModeSmall:
        layoutedWidth = ItemBackViewSmallWidth;
        layoutedHeight = ItemBackViewSmallHeight;
        font = SmallFont;
        if ([view frame].size.height > 24)
          [view removeFromSuperview];
        break;
      default:
        NSLog(@"Invalid NSToolbarSizeMode"); // Invalid
    } 
    
  _font = font;

  // Adjust the layout in accordance with the border
  if (!(borderMask & GSToolbarViewBottomBorder))
    {
      layoutedHeight++;
      layoutedWidth++;
    }

  if (!(borderMask & GSToolbarViewTopBorder))
    {
      layoutedHeight++;
      layoutedWidth++; 
    }
  
  // Adjust the layout in accordance with the label
  {
    NSAttributedString *attrStr;
    NSDictionary *attr;
    NSString *label = [_toolbarItem label];

    attr = [NSDictionary dictionaryWithObject: font forKey: NSFontAttributeName];
    if (label == nil || [label isEqualToString: @""])
      label = @"Dummy";
    attrStr = [[NSAttributedString alloc] initWithString: label attributes: attr];
    labelSize = [attrStr size];
    DESTROY(attrStr);
  }
   
  textWidth = labelSize.width + 2 * InsetItemTextX;
  if (textWidth > layoutedWidth)
    layoutedWidth = textWidth;
  
  _enabled = YES;
  /* This boolean variable is used to known when it's needed to draw the label
     in the -drawRect: method. */
  _showLabel = YES; 
   
  // Adjust the layout in accordance with NSToolbarDisplayMode
  switch ([[_toolbarItem toolbar] displayMode])
    {
      case NSToolbarDisplayModeDefault:
        break; // Nothing to do
      case NSToolbarDisplayModeIconAndLabel:
        break; // Nothing to do
      case NSToolbarDisplayModeIconOnly:
        _showLabel = NO;
        layoutedHeight -= labelSize.height + InsetItemTextY;
        break;
      case NSToolbarDisplayModeLabelOnly:
        _enabled = NO;
        layoutedHeight = labelSize.height + InsetItemTextY * 2;
        if ([view superview] != nil)
          [view removeFromSuperview];
        break;
      default:
        ; // Invalid
    }
   
  /* If the view is visible... 
     Adjust the layout in accordance with the view width in the case it is 
     needed. */
  if ([view superview] != nil)
    { 
      if (layoutedWidth < [view frame].size.width + 2 * InsetItemViewX)
        layoutedWidth = [view frame].size.width + 2 * InsetItemViewX; 
    }
  
  // Set the frame size to use the new layout
  [self setFrameSize: NSMakeSize(layoutedWidth, layoutedHeight)];
  
  /* If the view is visible...
     Adjust the view position in accordance with the new layout. */
  if ([view superview] != nil)
    {
      float insetItemViewY = ([self frame].size.height 
			      - [view frame].size.height) / 2;

      if (_showLabel)
        {
          insetItemViewY += (labelSize.height + InsetItemTextY) / 2;
        }
        
      [view setFrameOrigin: NSMakePoint((layoutedWidth 
        - [view frame].size.width) / 2, insetItemViewY)];
    }    
}

- (void) mouseDown: (NSEvent *)event
{
  NSToolbar *toolbar = [_toolbarItem toolbar];
  
  if (([event modifierFlags] == NSCommandKeyMask 
       && [toolbar allowsUserCustomization])
      || [toolbar customizationPaletteIsRunning] || toolbar == nil)
    {          
      NSSize viewSize = [self frame].size;
      NSImage *image = [[NSImage alloc] initWithSize: viewSize];
      NSPasteboard *pboard;
      NSInteger index = NSNotFound;
      
      // Prepare the drag
      
      /* We need to keep this view (aka self) to be able to draw the drag
         image. */
      RETAIN(self); 
      
      // Draw the drag content in an image
      /* The code below is only partially supported by GNUstep, then NSImage
         needs to be improved. */
      [image lockFocus];
      [self drawRect: 
                NSMakeRect(0, 0, viewSize.width, viewSize.height)];
      [image unlockFocus];
      
      pboard = [NSPasteboard pasteboardWithName: NSDragPboard];
      [pboard declareTypes: [NSArray arrayWithObject: GSMovableToolbarItemPboardType] 
              owner: nil];
      if (toolbar != nil)
	{
          index = [toolbar _indexOfItem: _toolbarItem];
        }
	  [GSToolbarView setDraggedItemIndex:index];
      [pboard setString: [NSString stringWithFormat:@"%ld", (long) index] 
              forType: GSMovableToolbarItemPboardType];
      
      [self dragImage: image 
            //at: NSMakePoint(0.0, viewSize.height)
            at: NSMakePoint(0.0, 0.0)
            offset: NSMakeSize(0.0, 0.0)
            event: event 
            pasteboard: pboard 
            source: self
            slideBack: NO];
      RELEASE(image);
    }
  else if ([event modifierFlags] != NSCommandKeyMask)
    {
      [super mouseDown: event];
    }
}

- (void) draggedImage: (NSImage *)dragImage beganAt: (NSPoint)location
{
  //nothing to do
}

- (void) draggedImage: (NSImage *)dragImage 
              endedAt: (NSPoint)location 
            operation: (NSDragOperation)operation
{
  //nothing to do
}

- (NSDragOperation) draggingSourceOperationMaskForLocal: (BOOL)isLocal
{
  return isLocal ? NSDragOperationGeneric : NSDragOperationNone;
}

- (NSToolbarItem *) toolbarItem
{
  return _toolbarItem;
}

/*
 * End of the code to keep in sync
 */

- (BOOL) isEnabled
{
  if (_flags._isEnabled)
    {
      id view = [_toolbarItem view];
  
      return [view isEnabled];
    }
  else
    {
      return _enabled;
    }
}

- (void) setEnabled: (BOOL)enabled
{
  if (_flags._setEnabled)
    {
      id view = [_toolbarItem view];

      [view setEnabled: enabled];
    }
  else 
    {
      _enabled = enabled;
    }
}

- (NSImage *) image
{
  if (_flags._image)
    {
      id view = [_toolbarItem view];

      return [view image];
    }
  else
    {
      return nil;
    }
}

- (void) setImage: (NSImage *)image
{
  if (_flags._setImage)
    {
      id view = [_toolbarItem view];

      [view setImage: image];
    }
}

- (void) setAction: (SEL)action
{
  if (_flags._setAction)
    {
      id view = [_toolbarItem view];

      [view setAction: action];
    }
}

- (SEL) action
{
  if (_flags._action)
    {
      id view = [_toolbarItem view];

      return [view action];
    }
  else
    {
      return 0;
    }
}

- (void) setTarget: (id)target
{
  if (_flags._setTarget)
    {
      id view = [_toolbarItem view];

      [view setTarget: target];
    }
}

- (id) target
{
  if (_flags._target)
    {
      id view = [_toolbarItem view];

      return [view target];
    }
  else
    {
      return nil;
    }
}

@end

/*
 * Standard toolbar items.
 */

// ---- NSToolbarSeparatorItemIdentifier
@interface NSToolbarSeparatorItem : NSToolbarItem
{
}
@end

@implementation NSToolbarSeparatorItem
- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{
  self = [super initWithItemIdentifier: itemIdentifier];
  if (!self)
    return nil;

  [(NSButton *)[self _backView] setImagePosition: NSImageOnly];
  [(NSButton *)[self _backView] setImage: 
                   [NSImage imageNamed: @"common_ToolbarSeparatorItem"]];
  /* We bypass the toolbar item accessor to set the image in order to have it
     (48 * 48) not resized. */
   [self setPaletteLabel: _(@"Separator")];
  
  [[self _backView] setFrameSize: NSMakeSize(30, ItemBackViewDefaultHeight)];
  
  return self;
}

- (NSMenuItem *) _defaultMenuFormRepresentation 
{
  return nil; // Override the default implementation in order to do nothing
}

- (void) _layout 
{
  NSView *backView = [self _backView];
  
  // Override the default implementation
  
  [(id)backView layout];
  if ([self toolbar] != nil)
    [backView setFrameSize: NSMakeSize(30, [backView frame].size.height)];
}

- (BOOL) allowsDuplicatesInToolbar
{
  return YES;
}

@end

// ---- NSToolbarSpaceItemIdentifier
@interface NSToolbarSpaceItem : NSToolbarItem
{
}
@end

@implementation NSToolbarSpaceItem
- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{ 
  self = [super initWithItemIdentifier: itemIdentifier];
  if (!self)
    return nil;
  [self setPaletteLabel: _(@"Space")];
   
  return self;
}

// Override the default implementation in order to do nothing
- (NSMenuItem *) _defaultMenuFormRepresentation 
{
  return nil;
}

- (BOOL) allowsDuplicatesInToolbar
{
  return YES;
}

@end

// ---- NSToolbarFlexibleSpaceItemIdentifier
@interface NSToolbarFlexibleSpaceItem : NSToolbarItem
{
}
@end

@implementation NSToolbarFlexibleSpaceItem
- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{
  self = [super initWithItemIdentifier: itemIdentifier];
  if (!self)
    return nil;
  [self setPaletteLabel: _(@"Flexible Space")];
  [self _layout];
  
  return self;
}

// Override the default implementation in order to do nothing
- (NSMenuItem *) _defaultMenuFormRepresentation 
{
  return nil;
}

// Override the default implementation in order to reset the _backView to a zero width
- (void) _layout 
{
  NSView *backView = [self _backView];
  NSSize size;

  [(id)backView layout];
  size = [backView frame].size;
  
  /* If the item is not part of a toolbar, this usually means it is used by
     customization palette, we shouldn't resize it in this case. */
  if ([self toolbar] != nil)
    [backView setFrameSize: NSMakeSize(0, size.height)];
  
  [self setMinSize: NSMakeSize(0, size.height)];
  [self setMaxSize: NSMakeSize(10000, size.height)];
}

- (BOOL) _isFlexibleSpace
{
  return YES;
}

- (BOOL) allowsDuplicatesInToolbar
{
  return YES;
}

@end

// ---- NSToolbarShowColorsItemIdentifier
@interface GSToolbarShowColorsItem : NSToolbarItem
{
}
@end

@implementation GSToolbarShowColorsItem
- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{
  self = [super initWithItemIdentifier: itemIdentifier];
  if (!self)
    return nil;
  [self setImage: [NSImage imageNamed: @"common_ToolbarShowColorsItem"]];
  [self setLabel: _(@"Colors")];

  // Set action...
  [self setTarget: nil]; // Goes to first responder..
  [self setAction: @selector(orderFrontColorPanel:)];

  return self;
}
@end

// ---- NSToolbarShowFontsItemIdentifier
@interface GSToolbarShowFontsItem : NSToolbarItem
{
}
@end

@implementation GSToolbarShowFontsItem
- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{
  self = [super initWithItemIdentifier: itemIdentifier];
  if (!self)
    return nil;
  [self setImage: [NSImage imageNamed: @"common_ToolbarShowFontsItem"]];
  [self setLabel: _(@"Fonts")];

  // Set action...
  [self setTarget: nil]; // Goes to first responder..
  [self setAction: @selector(orderFrontFontPanel:)];

  return self;
}
@end

// ---- NSToolbarCustomizeToolbarItemIdentifier
@interface GSToolbarCustomizeToolbarItem : NSToolbarItem
{
}
@end

@implementation GSToolbarCustomizeToolbarItem
- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{
  self = [super initWithItemIdentifier: itemIdentifier];
  if (!self)
    return nil;
  [self setImage: [NSImage imageNamed: @"common_ToolbarCustomizeToolbarItem"]];
  [self setLabel: _(@"Customize")];

  // Set action...
  [self setTarget: nil]; // Goes to first responder..
  [self setAction: @selector(runToolbarCustomizationPalette:)];

  return self;
}
@end

// ---- NSToolbarPrintItemIdentifier
@interface GSToolbarPrintItem : NSToolbarItem
{
}
@end

@implementation GSToolbarPrintItem
- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{
  self = [super initWithItemIdentifier: itemIdentifier];
  if (!self)
    return nil;
  [self setImage: [NSImage imageNamed: @"common_Printer"]];
  [self setLabel: _(@"Print...")];

  // Set action...
  [self setTarget: nil]; // goes to first responder..
  [self setAction: @selector(printDocument:)];

  return self;
}
@end


@implementation NSToolbarItem

+ (void) initialize
{
  // This used to be size 11.
  NormalFont = RETAIN([NSFont systemFontOfSize: [NSFont systemFontSize]]);
  // [NSFont smallSystemFontSize] or better should be NSControlContentFontSize
  SmallFont = RETAIN([NSFont systemFontOfSize: [NSFont smallSystemFontSize]]);
}

- (id) initWithItemIdentifier: (NSString *)itemIdentifier
{
  // GNUstep predefined toolbar items
  if ([itemIdentifier isEqualToString: NSToolbarSeparatorItemIdentifier] 
      && [self isKindOfClass: [NSToolbarSeparatorItem class]] == NO)
    {
      RELEASE(self);
      return [[NSToolbarSeparatorItem alloc] 
                 initWithItemIdentifier: itemIdentifier];
    }
  else if ([itemIdentifier isEqualToString: NSToolbarSpaceItemIdentifier]
           && [self isKindOfClass: [NSToolbarSpaceItem class]] == NO)
    {
      RELEASE(self);
      return [[NSToolbarSpaceItem alloc] 
                 initWithItemIdentifier: itemIdentifier];
    }
  else if ([itemIdentifier 
               isEqualToString: NSToolbarFlexibleSpaceItemIdentifier] 
           && [self isKindOfClass: [NSToolbarFlexibleSpaceItem class]] == NO)
    {
      RELEASE(self);
      return [[NSToolbarFlexibleSpaceItem alloc] 
                 initWithItemIdentifier: itemIdentifier];
    }
  else if ([itemIdentifier 
               isEqualToString: NSToolbarShowColorsItemIdentifier]
           && [self isKindOfClass: [GSToolbarShowColorsItem class]] == NO)
    {
      RELEASE(self);
      return [[GSToolbarShowColorsItem alloc] 
                 initWithItemIdentifier: itemIdentifier];
    }
  else if ([itemIdentifier 
               isEqualToString: NSToolbarShowFontsItemIdentifier]
           && [self isKindOfClass: [GSToolbarShowFontsItem class]] == NO)
    {
      RELEASE(self);
      return [[GSToolbarShowFontsItem alloc] 
                 initWithItemIdentifier: itemIdentifier];
    }
  else if ([itemIdentifier 
               isEqualToString: NSToolbarCustomizeToolbarItemIdentifier]
           && [self isKindOfClass: [GSToolbarCustomizeToolbarItem class]] == NO)
    {
      RELEASE(self);
      return [[GSToolbarCustomizeToolbarItem alloc] 
                 initWithItemIdentifier: itemIdentifier];
    }
  else if ([itemIdentifier isEqualToString: NSToolbarPrintItemIdentifier]
           && [self isKindOfClass: [GSToolbarPrintItem class]] == NO)
    {
      RELEASE(self);
      return [[GSToolbarPrintItem alloc] 
                 initWithItemIdentifier: itemIdentifier];
    }
  else
    {
      if ((self = [super init]) != nil)
        {
          // Normal toolbar items
          ASSIGN(_itemIdentifier, itemIdentifier);
          [self setAutovalidates: YES];

          // Set the backview to an GSToolbarButton, will get reset to a 
          // GSToolbarBackView when setView: gets called.
	  [self setView: nil];
        }        
    }
  
  return self;
}

- (void) dealloc
{
  RELEASE(_itemIdentifier);
  RELEASE(_label);
  RELEASE(_image);
  RELEASE(_menuFormRepresentation);
  RELEASE(_paletteLabel);
  RELEASE(_toolTip);
  TEST_RELEASE(_view);
  RELEASE(_backView);
  
  [super dealloc];
}

- (BOOL) allowsDuplicatesInToolbar
{
  return NO;
}

- (BOOL) isEnabled
{
  return [(GSToolbarButton*)_backView isEnabled];
}

- (NSImage *) image
{
  // return [(GSToolbarButton*)_backView image];
  return _image;
}

- (NSString *) itemIdentifier
{
  return _itemIdentifier;
}

- (NSString *) label
{
  // FIXME: I think this is not needed
  if ([[self toolbar] displayMode] == NSToolbarDisplayModeLabelOnly)
    {
      NSMenuItem *menuItem = [self menuFormRepresentation];
      
      if (menuItem != nil)
        return [menuItem title];
    }

  if (nil != _label)
    {
      return _label;
    }
  else
    {
      return @"";
    }
}

- (NSSize) maxSize
{
  return _maxSize;
}

- (NSMenuItem *) menuFormRepresentation
{
  return _menuFormRepresentation;
}

- (NSSize) minSize
{
  return _minSize;
}

- (NSString *) paletteLabel
{
  return _paletteLabel;
}

- (void) setAction: (SEL)action
{
  [(GSToolbarButton *)_backView setAction: action];
        
  [self setEnabled: (action != NULL)];
}

- (void) setEnabled: (BOOL)enabled
{
  [(GSToolbarButton*)_backView setEnabled: enabled];
}

- (void) setImage: (NSImage *)image
{
  ASSIGN(_image, image);  
      
  [_image setScalesWhenResized: YES];
  //[_image setSize: NSMakeSize(32, 32)];

  // Do not set the image on the button if we are in "LabelOnly"
  // mode. If the toolbar's displayMode changes later, we'll
  // put the image on the button in the layout method.
  if ([[self toolbar] displayMode] != NSToolbarDisplayModeLabelOnly)
    {
      [(GSToolbarButton*)_backView setImage: image];
    }
}

- (void) setLabel: (NSString *)label
{
  ASSIGN(_label, label);
  
  if (!_view)
    [(GSToolbarButton *)_backView setTitle: _label];

  _modified = YES;
  if (_toolbar != nil)
    [[_toolbar _toolbarView] _reload];
}

- (void) setMaxSize: (NSSize)maxSize
{
  _maxSize = maxSize;
}

- (void) setMenuFormRepresentation: (NSMenuItem *)menuItem
{
  ASSIGN(_menuFormRepresentation, menuItem);
}

- (void) setMinSize: (NSSize)minSize
{
  _minSize = minSize;
}

- (void) setPaletteLabel: (NSString *)paletteLabel
{
  ASSIGN(_paletteLabel, paletteLabel);
}

- (void) setTag: (NSInteger)tag
{
  _tag = tag;
}

- (void) setTarget: (id)target
{
  [(NSButton *)_backView setTarget: target];
}

- (void) setToolTip: (NSString *)toolTip
{
  ASSIGN(_toolTip, toolTip);
  if (_view)
    {
      [_view setToolTip: _toolTip];
    }
  else
    {
      [_backView setToolTip: _toolTip];
    }
}

- (void) setView: (NSView *)view
{
  if ((_view == view) && (_backView != nil))
    return;
    
  ASSIGN(_view, view);

  if (view)
    {
      NSSize size;

      size = [view frame].size;
      if (NSEqualSizes(NSZeroSize, _minSize))
          [self setMinSize: size];
      if (NSEqualSizes(NSZeroSize, _maxSize))
          [self setMaxSize: size];

      [_view setToolTip: _toolTip];

      RELEASE(_backView);
      _backView = [[GSToolbarBackView alloc] initWithToolbarItem: self];
    }
  else
    {
      RELEASE(_backView);
      _backView = [[GSToolbarButton alloc] initWithToolbarItem: self];
      [_backView setToolTip: _toolTip];
    }
}

- (NSInteger) tag
{
  return _tag;
}

- (NSString *) toolTip
{
  return _toolTip;
}

- (NSToolbar *) toolbar
{
  return _toolbar;
}

- (void) validate
{
  BOOL enabled = YES;
  id target;

  /* No validation for custom views */
  if (_view)
    return;

  target = [NSApp targetForAction: [self action] to: [self target] from: self];
  if (target == nil || ![target respondsToSelector: [self action]])
    {
      enabled = NO;
    }
  else if ([target respondsToSelector: @selector(validateToolbarItem:)])
    {
      enabled = [target validateToolbarItem: self];
    }
  else if ([target respondsToSelector: @selector(validateUserInterfaceItem:)])
    {
      enabled = [target validateUserInterfaceItem: self];
    }
  [self setEnabled: enabled];
}

- (NSView *) view
{
  return _view;
}

// Private or package like visibility methods

- (NSView *) _backView
{
  return _backView;
}

//
// This method invokes using the toolbar item as the sender.
// When invoking from the menu, it shouldn't send the menuitem as the
// sender since some applications check this and try to get additional
// information about the toolbar item which this is coming from. Since
// we implement the menu's action, we must also validate it.
//
- (void) _sendAction: (id)sender
{
  [NSApp sendAction: [self action] 
	 to: [self target]
	 from: self];
}

- (BOOL) validateMenuItem: (NSMenuItem *)menuItem
{
  return [self isEnabled];
}

- (NSMenuItem *) _defaultMenuFormRepresentation
{
  NSMenuItem *menuItem;
  
  menuItem = [[NSMenuItem alloc] initWithTitle: [self label]  
				 action: @selector(_sendAction:) 
                                 keyEquivalent: @""];
  [menuItem setTarget: self];
  AUTORELEASE(menuItem);
  
  return menuItem;
}

- (void) _layout
{
  // Reset to image on the backview: We may have toggled
  // from one NSToolbarDisplayMode to another, and it is
  // possible setImage: would have been called on the
  // NSToolbarItem while we were in NSToolbarDisplayModeLabelOnly
  if ([[self toolbar] displayMode] != NSToolbarDisplayModeLabelOnly)
    {
      [(GSToolbarButton*)_backView setImage: _image];
    }

  [(id)_backView layout];
}

- (BOOL) _isModified
{
  return _modified;
}

- (BOOL) _isFlexibleSpace
{
  return NO;
}

- (BOOL) _selectable
{
  return _selectable;
}

- (BOOL) _selected
{
  return [(GSToolbarButton *)_backView state];
}

- (void) _setSelected: (BOOL)selected
{
  if (_selectable)
    {
      if ([self _selected] != selected)
        [(GSToolbarButton *)_backView setState: selected];
    }
  else
    {
      NSLog(@"The toolbar item %@ is not selectable", self);
    }
}

- (void) _setSelectable: (BOOL)selectable
{
  if ([_backView isKindOfClass: [GSToolbarButton class]])
    {
      _selectable = selectable;
      [(GSToolbarButton *)_backView setButtonType: NSOnOffButton];
    }
  else
    {
      NSLog(@"The toolbar item %@ is not selectable", self);
    }   
}

- (void) _setToolbar: (NSToolbar *)toolbar
{
  // Don't do an ASSIGN here, the toolbar itself retains us.
  _toolbar = toolbar;
}

- (BOOL) autovalidates
{
  return _autovalidates;
}

- (void) setAutovalidates: (BOOL)autovalidates
{
  _autovalidates = autovalidates;
}

- (NSInteger) visibilityPriority
{
  return _visibilityPriority;
}

- (void) setVisibilityPriority: (NSInteger)visibilityPriority
{
  _visibilityPriority = visibilityPriority;
}

// NSValidatedUserInterfaceItem protocol
- (SEL) action
{
  return [(GSToolbarButton *)_backView action];
}

- (id) target
{
  return [(GSToolbarButton *)_backView target];
}

// NSCopying protocol
- (id) copyWithZone: (NSZone *)zone 
{
  NSToolbarItem *new = [[NSToolbarItem allocWithZone: zone] 
    initWithItemIdentifier: _itemIdentifier];
  NSString *toolTip;
  NSImage *image;
  NSString *label;
  NSMenuItem *item;

  // Copy all items individually...
  [new setTarget: [self target]];
  [new setAction: [self action]];

  toolTip = [[self toolTip] copyWithZone: zone];
  [new setToolTip: toolTip];
  RELEASE(toolTip);
  [new setTag: [self tag]];
  image = [[self image] copyWithZone: zone];
  [new setImage: image];
  RELEASE(image);
  [new setEnabled: [self isEnabled]];
  label = [[self paletteLabel] copyWithZone: zone];
  [new setPaletteLabel: label];
  RELEASE(label);
  label = [[self label] copyWithZone: zone];
  [new setLabel: label];
  RELEASE(label);
  [new setMinSize: [self minSize]];
  [new setMaxSize: [self maxSize]];
  [new setAutovalidates: [self autovalidates]];
  [new setVisibilityPriority: [self visibilityPriority]];
  item = [[self menuFormRepresentation] copyWithZone: zone];
  [new setMenuFormRepresentation: item];
  RELEASE(item);

  if ([self view] != nil)
    {
      NSData *encodedView = nil;
      NSView *superview = nil;

      /* NSView doesn't implement -copyWithZone:, that's why we encode
         then decode the view to create a copy of it. */
      superview = [[self view] superview];
      /* We must avoid to encode view hierarchy */
      [[self view] removeFromSuperviewWithoutNeedingDisplay];
      NSLog(@"Encode toolbar item with label %@, view %@ and superview %@", 
            [self label], [self view], superview);
      // NOTE: Keyed archiver would fail on NSSlider here.
      encodedView = [NSArchiver archivedDataWithRootObject: [self view]];
      [new setView: [NSUnarchiver unarchiveObjectWithData: encodedView]];
      // Re-add the view to its hierarchy
      [superview addSubview: [self view]];
    }

  return new;
}

- (NSString *) description
{
  return [NSString stringWithFormat: @"<%@ - <%@>>",[super description],[self itemIdentifier]];
}

- (id) initWithCoder: (NSCoder *)aCoder
{
  self = [self initWithItemIdentifier: [aCoder decodeObjectForKey:@"NSToolbarItemIdentifier"]];

  if ([aCoder containsValueForKey: @"NSToolbarItemTarget"])
    [self setTarget: [aCoder decodeObjectForKey:@"NSToolbarItemTarget"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemAction"])
    [self setAction: NSSelectorFromString([aCoder decodeObjectForKey:@"NSToolbarItemAction"])];
  if ([aCoder containsValueForKey: @"NSToolbarItemToolTip"])
    [self setToolTip: [aCoder decodeObjectForKey:@"NSToolbarItemToolTip"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemTag"])
    [self setTag: [aCoder decodeIntForKey:@"NSToolbarItemTag"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemImage"])
    [self setImage: [aCoder decodeObjectForKey:@"NSToolbarItemImage"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemEnabled"])
    [self setEnabled: [aCoder decodeBoolForKey:@"NSToolbarItemEnabled"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemPaletteLabel"])
    [self setPaletteLabel: [aCoder decodeObjectForKey:@"NSToolbarItemPaletteLabel"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemLabel"])
    [self setLabel: [aCoder decodeObjectForKey:@"NSToolbarItemLabel"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemMinSize"])
    [self setMinSize: [aCoder decodeSizeForKey:@"NSToolbarItemMinSize"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemMaxSize"])
    [self setMaxSize: [aCoder decodeSizeForKey:@"NSToolbarItemMaxSize"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemAutovalidates"])
    [self setAutovalidates: [aCoder decodeBoolForKey:@"NSToolbarItemAutovalidates"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemVisibilityPriority"])
    [self setVisibilityPriority: [aCoder decodeIntForKey:@"NSToolbarItemVisibilityPriority"]];
  if ([aCoder containsValueForKey: @"NSToolbarItemView"])
    [self setView: [aCoder decodeObjectForKey: @"NSToolbarItemView"]];

  return self;
}

@end
