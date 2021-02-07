/** <title>NSMenuItemCell</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Michael Hanni <mhanni@sprintmail.com>
   Date: 1999
   
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

#include "config.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSDictionary.h>
//#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
//#import <Foundation/NSNotification.h>
//#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSMenuItemCell.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSStringDrawing.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSGuiPrivate.h"

static NSString *controlKeyString = @"^";
static NSString *alternateKeyString = @"+";
static NSString *shiftKeyString = @"/";
static NSString *commandKeyString = @"#";

@interface NSMenuItemCell (Private)
- (GSThemeControlState) themeControlState;
@end

@implementation NSMenuItemCell

+ (void) initialize
{
  if (self == [NSMenuItemCell class])
    {
      NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
      NSString *keyString;

      [self setVersion: 2];
      keyString = [userDefaults objectForKey: @"GSControlKeyString"];
      if (nil != keyString)
	{
	  controlKeyString = [keyString retain];
	}
      keyString = [userDefaults objectForKey: @"GSAlternateKeyString"];
      if (nil != keyString)
	{
	  alternateKeyString = [keyString retain];
	}
      keyString = [userDefaults objectForKey: @"GSShiftKeyString"];
      if (nil != keyString)
	{
	  shiftKeyString = [keyString retain];
	}
      keyString = [userDefaults objectForKey: @"GSCommandKeyString"];
      if (nil != keyString)
	{
	  commandKeyString = [keyString retain];
	}
    }
}

+ (NSFocusRingType) defaultFocusRingType
{
  return NSFocusRingTypeNone;
}

- (id) init
{
  self = [super init];
  if (nil == self)
    return nil;

  [self setButtonType: NSMomentaryLightButton];
  [self setAlignment: NSLeftTextAlignment];
  [self setFont: [NSFont menuFontOfSize: 0]];
  [self setNeedsSizing: YES];

  return self;
}

- (void) dealloc
{
  RELEASE(_menuItem);
  [super dealloc];
}

- (void) setHighlighted:(BOOL)flag
{
  _cell.is_highlighted = flag;
  [self setNeedsSizing: YES];
}

- (BOOL) isHighlighted
{
  // Same as in super class
  return _cell.is_highlighted;
}

- (NSColor *)textColor
{
  if (_cell.is_highlighted && [self isEnabled])
    {
      return [NSColor selectedMenuItemTextColor];
    }

  return [super textColor];
}

- (NSColor *) backgroundColor
{
  unsigned	mask;
  NSColor	*color;
  GSThemeControlState state = GSThemeNormalState;

  if (_cell.is_highlighted)
    {
      mask = _highlightsByMask;

      if (_cell.state)
        mask &= ~_showAltStateMask;
    }
  else if (_cell.state)
    mask = _showAltStateMask;
  else
    mask = NSNoCellMask;

  // Determine the background color
  if (mask & (NSChangeGrayCellMask | NSChangeBackgroundCellMask))
    {
      state = GSThemeHighlightedState;
    }

  if (mask & NSPushInCellMask)
    {
      state = GSThemeSelectedState;
    }

  // TODO: Make the color lookup simpler.
  color = [[GSTheme theme] colorNamed: @"NSMenuItem" state: state];
  if (color == nil)
    {
      if ((state == GSThemeHighlightedState) || (state == GSThemeSelectedState))
	{
	  color = [NSColor selectedMenuItemColor];
	}
      else
	{
	  color = [[GSTheme theme] menuItemBackgroundColor];
	}
    }

  return color;
}

- (void) setMenuItem: (NSMenuItem *)item
{
  ASSIGN(_menuItem, item);
  [self setEnabled: [_menuItem isEnabled]];
  [self setNeedsSizing: YES];
}

- (NSMenuItem *) menuItem
{
  return _menuItem;
}

- (NSInteger) tag
{
  return [[self menuItem] tag];
}

- (void) setMenuView: (NSMenuView *)menuView
{
  /* The menu view is retaining us, we should not retain it.  */
  _menuView = menuView;
  /*
   * Determine whether we have horizontal or vertical layout and adjust.
   */
  if ([_menuView isHorizontal] == YES)
    {
      [self setAlignment: NSCenterTextAlignment];
      [self setBordered: NO];
      [self setImagePosition: NSImageLeft];
    }
  else
    {
      [self setAlignment: NSLeftTextAlignment];
      [self setBordered: YES];
    }
  [self setNeedsSizing: YES];
}

- (NSMenuView *) menuView
{
  return _menuView;
}

- (NSString*) _keyEquivalentString
{
  NSString *key = [_menuItem keyEquivalent];
  unsigned int m = [_menuItem keyEquivalentModifierMask];
  NSString *ucKey = [key uppercaseString];
  unichar uchar;

  if ((key == nil) || [key isEqualToString: @""])
    return key;
  
  uchar = [key characterAtIndex: 0];
  if (uchar >= 0xF700)
    {
      // FIXME: At the moment we are not able to handle function keys
      // as key equivalent
      return nil;
    }

  if ([key isEqualToString: @"\\r"])
    key = @"RET";
  else if ([key isEqualToString: @"\\e"])
    key = @"ESC";
  else if ([key isEqualToString: @"\\d"])
    key = @"DEL";

  if (m != 0)
    {
      BOOL shift;
      NSFontTraitMask traits = 0;
      
      if ((m & NSAlternateKeyMask) &&
          (!alternateKeyString || [alternateKeyString isEqualToString: @""]))
        {
          traits |= NSItalicFontMask;
        }
      if ((m & NSControlKeyMask) &&
          (!controlKeyString || [controlKeyString isEqualToString: @""]))
        {
          traits |= NSBoldFontMask;
        }
        
      if (traits)
        {
          NSFont *font;
          font = [[NSFontManager sharedFontManager]
                   fontWithFamily:[[NSFont controlContentFontOfSize:0.0] familyName]
                           traits:traits
                           weight:(traits & NSBoldFontMask) ? 9 : 5
                             size:[NSFont systemFontSize]];
          if (font)
            [self setKeyEquivalentFont:font];
        }
      else
        {
          // shift mask and not an upper case string?
          shift = (m & NSShiftKeyMask) & ![key isEqualToString: ucKey];
          key = [NSString stringWithFormat:@"%@%@%@%@%@",
                          (m & NSControlKeyMask) ? controlKeyString : @"",
                          (m & NSAlternateKeyMask) ? alternateKeyString : @"",
                          (shift != NO) ? shiftKeyString : @"",
                          (m & NSCommandKeyMask) ? commandKeyString : @"",
                          key];
        }
    }

  return key;
}

- (NSSize) _sizeKeyEquivalentText: (NSString*)title
{
  NSSize size;
  NSDictionary *attrs;
  
  if (title == nil) {
    return NSMakeSize (0,0);
  }
  
  if (_keyEquivalentFont) {
    attrs = [NSDictionary dictionaryWithObject: _keyEquivalentFont
                                        forKey: NSFontAttributeName];
    size = [title sizeWithAttributes: attrs];
  }
  else {
    size = [self _sizeText: title];
  }
  return size;
}

- (void) calcSize
{
  NSSize   componentSize;
  NSImage *anImage = nil;
  CGFloat  neededMenuItemHeight = 20;
 
  // Check if _mcell_belongs_to_popupbutton = NO while cell owned by 
  // popup button. FIXME
  if (!_mcell_belongs_to_popupbutton && [[_menuView menu] _ownedByPopUp])
    {
      _mcell_belongs_to_popupbutton = YES;
      [self setImagePosition: NSImageRight];
    }

  // State Image
  if ([_menuItem changesState])
    {
      // NSOnState
      if ([_menuItem onStateImage])
        componentSize = [[_menuItem onStateImage] size];
      else
        componentSize = NSMakeSize(0,0);
      _stateImageWidth = componentSize.width;
      if (componentSize.height > neededMenuItemHeight)
        neededMenuItemHeight = componentSize.height;

      // NSOffState
      if ([_menuItem offStateImage])
        componentSize = [[_menuItem offStateImage] size];
      else
        componentSize = NSMakeSize(0,0);
      if (componentSize.width > _stateImageWidth)
        _stateImageWidth = componentSize.width;
      if (componentSize.height > neededMenuItemHeight)
        neededMenuItemHeight = componentSize.height;

      // NSMixedState
      if ([_menuItem mixedStateImage])
        componentSize = [[_menuItem mixedStateImage] size];
      else
        componentSize = NSMakeSize(0,0);
      if (componentSize.width > _stateImageWidth)
        _stateImageWidth = componentSize.width;
      if (componentSize.height > neededMenuItemHeight)
        neededMenuItemHeight = componentSize.height;
    }
  else
    {
      _stateImageWidth = 0.0;
    }

  // Image
  if ((anImage = [_menuItem image]) && _cell.image_position == NSNoImage)
    [self setImagePosition: NSImageLeft];
  if (anImage)
    {
      componentSize = [anImage size];
      _imageWidth = componentSize.width;
      if (componentSize.height > neededMenuItemHeight)
        neededMenuItemHeight = componentSize.height;
    }
  else
    {
      _imageWidth = 0.0;
    }

  // Title and Key Equivalent
  componentSize = [self _sizeText: [_menuItem title]];
  _titleWidth = componentSize.width;
  if (componentSize.height > neededMenuItemHeight)
    neededMenuItemHeight = componentSize.height;
  componentSize = [self _sizeKeyEquivalentText: [self _keyEquivalentString]];
  _keyEquivalentWidth = componentSize.width;
  if (componentSize.height > neededMenuItemHeight)
    neededMenuItemHeight = componentSize.height;

  // Submenu Arrow
  if ([_menuItem hasSubmenu])
    {
      NSImage	*arrow = [NSImage imageNamed: @"NSMenuArrow"];

      if (arrow != nil)
	{
          componentSize = [arrow size];
	}
      else
	{
	  componentSize = NSMakeSize(0, 0);
	}
      _keyEquivalentWidth = componentSize.width;
      if (componentSize.height > neededMenuItemHeight)
        neededMenuItemHeight = componentSize.height;
    }

  // Cache definitive height
  _menuItemHeight = neededMenuItemHeight;

  // At the end we set sizing to NO.
  _needs_sizing = NO;
}

- (void) setNeedsSizing:(BOOL)flag
{
  _needs_sizing = flag;
}

- (BOOL) needsSizing
{
  return _needs_sizing;
}

- (void) setNeedsDisplay:(BOOL)flag
{
  _needs_display = flag;
}

- (BOOL) needsDisplay
{
  return _needs_display;
}

- (CGFloat) imageWidth
{
  if (_needs_sizing)
    [self calcSize];

  return _imageWidth;
}

- (CGFloat) titleWidth
{
  if (_needs_sizing)
    [self calcSize];

  return _titleWidth;
}

- (CGFloat) keyEquivalentWidth
{
  if (_needs_sizing)
    [self calcSize];

  return _keyEquivalentWidth;
}

- (CGFloat) stateImageWidth
{
  if (_needs_sizing)
    [self calcSize];

  return _stateImageWidth;
}

//
// Sizes for drawing taking into account NSMenuView adjustments.
//
- (NSRect) imageRectForBounds: (NSRect)cellFrame
{
  if (_needs_sizing)
    [self calcSize];

  if (_mcell_belongs_to_popupbutton && _cell.image_position)
    {
      // Special case: draw image on the extreme right 
        cellFrame.origin.x  += cellFrame.size.width - _imageWidth - 4;
        cellFrame.size.width = _imageWidth;
        return cellFrame;
    }

  if ([_menuView isHorizontal] == YES)
    {
      switch (_cell.image_position)
        {
          case NSNoImage:
            cellFrame = NSZeroRect;
            break;
            
          case NSImageOnly:
          case NSImageOverlaps:
            break;
            
          case NSImageLeft:
            cellFrame.origin.x  += 4.; // _horizontalEdgePad
            cellFrame.size.width = _imageWidth;
            break;
        
          case NSImageRight:
            cellFrame.origin.x  += _titleWidth;
            cellFrame.size.width = _imageWidth;
            break;
           
          case NSImageBelow:
            cellFrame.size.height /= 2;
            break;
            
          case NSImageAbove:
            cellFrame.size.height /= 2;
            cellFrame.origin.y += cellFrame.size.height;
            break;
        }
    }
  else
    {
      // Calculate the image part of cell frame from NSMenuView
      cellFrame.origin.x  += [_menuView imageAndTitleOffset];
      cellFrame.size.width = [_menuView imageAndTitleWidth];

      switch (_cell.image_position)
        {
          case NSNoImage: 
            cellFrame = NSZeroRect;
            break;

          case NSImageOnly:
          case NSImageOverlaps:
            break;

          case NSImageLeft:
            cellFrame.size.width = _imageWidth;
            break;

          case NSImageRight:
            cellFrame.origin.x  += _titleWidth + GSCellTextImageXDist;
            cellFrame.size.width = _imageWidth;
            break;

          case NSImageBelow: 
            cellFrame.size.height /= 2;
            break;

          case NSImageAbove: 
            cellFrame.size.height /= 2;
            cellFrame.origin.y += cellFrame.size.height;
            break;
        }
    }
  return cellFrame;
}

- (NSRect) keyEquivalentRectForBounds:(NSRect)cellFrame
{
  // Calculate the image part of cell frame from NSMenuView
  cellFrame.origin.x  += [_menuView keyEquivalentOffset];
  cellFrame.size.width = [_menuView keyEquivalentWidth];

  return cellFrame;
}

- (NSRect) stateImageRectForBounds:(NSRect)cellFrame
{
  // Calculate the image part of cell frame from NSMenuView
  cellFrame.origin.x  += [_menuView stateImageOffset];
  cellFrame.size.width = [_menuView stateImageWidth];

  return cellFrame;
}

- (NSRect) titleRectForBounds:(NSRect)cellFrame
{
  if (_needs_sizing)
    [self calcSize];

  if (_mcell_belongs_to_popupbutton && _cell.image_position)
    {
      // Special case: draw image on the extreme right 
      cellFrame.origin.x  += [_menuView imageAndTitleOffset];
      cellFrame.size.width = _titleWidth;
      return cellFrame;
    }

  if ([_menuView isHorizontal] == YES)
    {
      /* This adjust will center us within the menubar. */

      cellFrame.size.height -= 2;

      switch (_cell.image_position)
        {
          case NSNoImage:
          case NSImageOverlaps:
            break;
      
          case NSImageOnly:
            cellFrame = NSZeroRect;
            break;
        
          case NSImageLeft:
            cellFrame.origin.x  += _imageWidth + GSCellTextImageXDist + 4;
            cellFrame.size.width = _titleWidth;
            break;
            
          case NSImageRight:
            cellFrame.size.width = _titleWidth;
            break;
                     
          case NSImageBelow:
            cellFrame.size.height /= 2;
            cellFrame.origin.y += cellFrame.size.height;
            break;

          case NSImageAbove:
            cellFrame.size.height /= 2;
            break;
        }
    }
  else
    {
      // Calculate the image part of cell frame from NSMenuView
      cellFrame.origin.x  += [_menuView imageAndTitleOffset];
      cellFrame.size.width = cellFrame.size.width - [_menuView imageAndTitleOffset];

      switch (_cell.image_position)
        {
          case NSNoImage:
          case NSImageOverlaps:
            break;

          case NSImageOnly:
            cellFrame = NSZeroRect;
            break;

          case NSImageLeft:
            cellFrame.origin.x  += _imageWidth + GSCellTextImageXDist;
            cellFrame.size.width = cellFrame.size.width - (_imageWidth + GSCellTextImageXDist);
            break;

          case NSImageRight:
            cellFrame.size.width = cellFrame.size.width - (_imageWidth + GSCellTextImageXDist);
            break;

          case NSImageBelow:
            cellFrame.size.height /= 2;
            cellFrame.origin.y += cellFrame.size.height;
            break;

          case NSImageAbove:
            cellFrame.size.height /= 2;
            break;
        }
    }
  return cellFrame;
}

- (NSRect) drawingRectForBounds: (NSRect)theRect
{
  if (_needs_sizing)
    [self calcSize];

  if ([_menuView isHorizontal] == YES)
    {
      /* A horizontal menu does not have borders drawn by the cell,
       * but it does have a border round the menu as a whole, so we
       * must inset from that.
       */
      return NSMakeRect (theRect.origin.x, theRect.origin.y + 2,
        theRect.size.width, theRect.size.height - 2);
    }
  else
    {
      if (_cell.is_bordered)
        {
          CGFloat yDelta = [_control_view isFlipped] ? 1. : 2.;
          unsigned mask;
          NSRect interiorFrame;
          
          if (_cell.is_highlighted)
            {
              mask = _highlightsByMask;
                
              if (_cell.state)
                mask &= ~_showAltStateMask;
            }
          else if (_cell.state)
            mask = _showAltStateMask;
          else
            mask = NSNoCellMask;
 
          /*
           * Special case:  Buttons have only three different paths for border.
           * One white path at the top left corner, one black path at the
           * bottom right and another in dark gray at the inner bottom right.
           */
          interiorFrame = NSMakeRect(theRect.origin.x + 1.,
                                     theRect.origin.y + yDelta,
                                     theRect.size.width - 3.,
                                     theRect.size.height - 3.);

          // pushed in buttons contents are displaced to the bottom right 1px
          if ((mask & NSPushInCellMask))
            {
              interiorFrame
                  = NSOffsetRect(interiorFrame, 1., [_control_view isFlipped] ? 1. : -1.);
            }
          return interiorFrame;
        }
      else
        {
          return theRect;
        }
    }
}

//
// Drawing.
//
- (void) drawBorderAndBackgroundWithFrame: (NSRect)cellFrame
                                   inView: (NSView *)controlView
{
  [[GSTheme theme] drawBorderAndBackgroundForMenuItemCell: self
                   withFrame: cellFrame
                   inView: controlView
                   state: [self themeControlState]
                   isHorizontal: [_menuView isHorizontal]];
}

- (void) drawImageWithFrame: (NSRect)cellFrame
                     inView: (NSView *)controlView
{
  cellFrame = [self imageRectForBounds: cellFrame];
  [self drawImage: _imageToDisplay withFrame: cellFrame inView: controlView];
}

- (void) drawKeyEquivalentWithFrame: (NSRect)cellFrame
			     inView: (NSView *)controlView
{
  NSImage	*arrow = nil;
  if (_cell.is_highlighted)
    {
      arrow = [NSImage imageNamed: @"NSHighlightedMenuArrow"];
    }

  if (arrow == nil)
    {
      arrow = [NSImage imageNamed: @"NSMenuArrow"];
    }


  cellFrame = [self keyEquivalentRectForBounds: cellFrame];

  if ([_menuItem hasSubmenu] && arrow != nil)
    {
      NSSize size;
      NSPoint position;

      size = [arrow size];
      position.x = cellFrame.origin.x + cellFrame.size.width - size.width;
      position.y = MAX(NSMidY(cellFrame) - (size.height/2.), 0.);
      /*
       * Images are always drawn with their bottom-left corner at the origin
       * so we must adjust the position to take account of a flipped view.
       */
      if ([controlView isFlipped])
        position.y += size.height;

      [arrow compositeToPoint: position operation: NSCompositeSourceOver];
    }
  /* FIXME/TODO here - decide a consistent policy for images.
   *
   * The reason of the following code is that we draw the key
   * equivalent, but not if we are a popup button and are displaying
   * an image (the image is displayed in the title or selected entry
   * in the popup, it's the small square on the right). In that case,
   * the image will be drawn in the same position where the key
   * equivalent would be, so we do not display the key equivalent,
   * else they would be displayed one over the other one.
   */
  else if (![[_menuView menu] _ownedByPopUp] || (_imageToDisplay == nil))
    {    
      if (_keyEquivalentFont != nil)
        {
          NSDictionary       *attrs;
          NSArray            *attrObjects, *attrKeys;
          NSAttributedString *aString;

          attrObjects = [NSArray arrayWithObjects: _keyEquivalentFont,
                                 [self textColor], nil];
          attrKeys = [NSArray arrayWithObjects: NSFontAttributeName,
                              NSForegroundColorAttributeName, nil];
          attrs = [NSDictionary dictionaryWithObjects: attrObjects
                                              forKeys: attrKeys];
          aString = [[NSAttributedString alloc]
                      initWithString: [self _keyEquivalentString]
                          attributes: attrs];
          [self _drawAttributedText: aString inFrame: cellFrame];
          [aString release];
        }
      else
        {
          [self _drawText: [self _keyEquivalentString] inFrame: cellFrame];
        }
    }
}


- (void) drawSeparatorItemWithFrame:(NSRect)cellFrame
                            inView:(NSView *)controlView
{
  [[GSTheme theme] drawSeparatorItemForMenuItemCell: self
                                          withFrame: cellFrame
                                             inView: controlView
                                       isHorizontal: [_menuView isHorizontal]]; 
}

- (void) drawStateImageWithFrame: (NSRect)cellFrame
                          inView: (NSView*)controlView
{
  NSImage *imageToDisplay;

  switch ([_menuItem state])
    {
      case NSOnState:
        imageToDisplay = [_menuItem onStateImage];
        break;

      case NSMixedState:
        imageToDisplay = [_menuItem mixedStateImage];
        break;

      case NSOffState:
      default:
        imageToDisplay = [_menuItem offStateImage];
        break;
    }

  if (imageToDisplay == nil)
    {
      return;
    }
  
  cellFrame = [self stateImageRectForBounds: cellFrame];
  [self drawImage: imageToDisplay withFrame: cellFrame inView: controlView];
}

- (void) drawTitleWithFrame: (NSRect)cellFrame
                     inView: (NSView *)controlView
{
  [[GSTheme theme] drawTitleForMenuItemCell: self 
                                  withFrame: cellFrame
                                     inView: controlView
                                      state: [self themeControlState]
                               isHorizontal: [_menuView isHorizontal]];
}

- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  [self drawBorderAndBackgroundWithFrame: cellFrame inView: controlView];
}

- (void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  unsigned  mask;

  // Transparent buttons never draw
  if (_buttoncell_is_transparent)
    return;

  if ([_menuItem isSeparatorItem])
    {
      [self drawSeparatorItemWithFrame: cellFrame inView: controlView];
      return;
    }

  cellFrame = [self drawingRectForBounds: cellFrame];
  
  // FIXME: I think all this code belongs into calcSize.
  if (_cell.is_highlighted)
    {
      mask = _highlightsByMask;

      if (_cell.state)
        mask &= ~_showAltStateMask;
    }
  else if (_cell.state)
    mask = _showAltStateMask;
  else
    mask = NSNoCellMask;

  /*
   * Determine the image and the title that will be
   * displayed. If the NSContentsCellMask is set the
   * image and title are swapped only if state is 1 or
   * if highlighting is set (when a button is pushed it's
   * content is changed to the face of reversed state).
   * The results are saved in two ivars for use in other
   * drawing methods.
   */
  if (mask & NSContentsCellMask)
    {
      _imageToDisplay = _altImage;
      if (!_imageToDisplay)
        _imageToDisplay = [_menuItem image];
      _titleToDisplay = _altContents;
      if (_titleToDisplay == nil || [_titleToDisplay isEqual: @""])
        _titleToDisplay = [_menuItem title];
    }
  else
    {
      _imageToDisplay = [_menuItem image];
      _titleToDisplay = [_menuItem title];
    }
       
  if (_imageToDisplay)
    {
      _imageWidth = [_imageToDisplay size].width;
    }

  if ([_menuView isHorizontal] == YES)
    {
      // Draw the image
      if (_imageWidth > 0)
        [self drawImageWithFrame: cellFrame inView: controlView];
         
      // Draw the title
      if (_titleWidth > 0)
        [self drawTitleWithFrame: cellFrame inView: controlView];
    }
  else
    {
      // Draw the state image
      if (_stateImageWidth > 0)
        [self drawStateImageWithFrame: cellFrame inView: controlView];

      // Draw the image
      if (_imageWidth > 0)
        [self drawImageWithFrame: cellFrame inView: controlView];

      // Draw the title
      if (_titleWidth > 0)
        [self drawTitleWithFrame: cellFrame inView: controlView];

      // Draw the key equivalent
      if (_keyEquivalentWidth > 0)
        [self drawKeyEquivalentWithFrame: cellFrame inView: controlView];
    }
}

//
// NSCopying protocol
//
- (id) copyWithZone: (NSZone*)zone
{
  NSMenuItemCell *c = [super copyWithZone: zone];

  if (_menuItem)
    c->_menuItem = [_menuItem copyWithZone: zone];

  /* We do not copy _menuView, because _menuView owns the old cell,
     but not the new one!  _menuView knows nothing about c.  If we copy
     the pointer to _menuView into c, then that pointer might become
     invalid at any point in time (it never becomes invalid for the original
     cell because _menuView will call [originalCell setMenuView: nil]
     when it's being deallocated.  But it will not do the same for c, because
     it doesn't even know that c exists!)  */
  c->_menuView = nil;

  return c;
}

/*
 * NSCoding protocol
 *
 * Normally unused since the NSMenu encodes/decodes the NSMenuItems, but
 * not the NSMenuItemCells.
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: _menuItem 
	      forKey: @"NSMenuItem"];
    }
  else
    {
      [aCoder encodeConditionalObject: _menuItem];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      [self setMenuItem: [aDecoder decodeObjectForKey: @"NSMenuItem"]];
    }
  else
    {
      ASSIGN (_menuItem, [aDecoder decodeObject]);

      if ([aDecoder versionForClassName: @"NSMenuItemCell"] < 2)
        {
          /* In version 1, we used to encode the _menuView here.  */
          [aDecoder decodeObject];
        }
    }
  _needs_sizing = YES;

  return self;
}

@end
