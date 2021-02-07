/** <title>GSThemeDrawing</title>

   <abstract>The theme methods for drawing controls</abstract>

   Copyright (C) 2004-2010 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@gnu.org>
   Date: Jan 2004
   
   This file is part of the GNU Objective C User interface library.

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

#import "GSThemePrivate.h"

#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSIndexSet.h"

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSColorList.h"
#import "AppKit/NSColorWell.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSMenuItemCell.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSPopUpButtonCell.h"
#import "AppKit/NSProgressIndicator.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTableHeaderCell.h"
#import "AppKit/NSTableHeaderView.h"
#import "AppKit/NSView.h"
#import "AppKit/NSTabView.h"
#import "AppKit/NSTabViewItem.h"
#import "AppKit/PSOperators.h"
#import "AppKit/NSSliderCell.h"
#import "AppKit/NSPathCell.h"
#import "AppKit/NSPathControl.h"
#import "AppKit/NSPathComponentCell.h"
#import "GNUstepGUI/GSToolbarView.h"
#import "GNUstepGUI/GSTitleView.h"

/* a border width of 5 gives a reasonable compromise between Cocoa metrics and looking good */
/* 7.0 gives us the NeXT Look (which is 8 pix wide including the shadow) */
#define COLOR_WELL_BORDER_WIDTH 7.0

@interface NSTableView (Private)
- (CGFloat *)_columnOrigins;
- (void) _willDisplayCell: (NSCell*)cell
	   forTableColumn: (NSTableColumn *)tb
		      row: (NSInteger)index;
- (id)_objectValueForTableColumn: (NSTableColumn *)tb
			     row: (NSInteger)index;
@end

@interface NSCell (Private)
- (void) _setInEditing: (BOOL)flag;
- (BOOL) _inEditing;
- (void) _drawEditorWithFrame: (NSRect)cellFrame
                       inView: (NSView *)controlView;
- (void) _drawAttributedText: (NSAttributedString*)aString 
                     inFrame: (NSRect)aRect;
@end

@implementation	GSTheme (Drawing)
- (void) setKeyEquivalent: (NSString *)key 
            forButtonCell: (NSButtonCell *)cell
{
  if([cell image] == nil && ([key isEqualToString:@"\r"] ||
			     [key isEqualToString:@"\n"]))
    { 
      [cell setImagePosition: NSImageRight];
      [cell setImage: [NSImage imageNamed:@"common_ret"]];
      [cell setAlternateImage: [NSImage imageNamed:@"common_retH"]];
    }
  else if([key isEqualToString:@"\r"] == NO &&
	  [key isEqualToString:@"\n"] == NO)
    {
      NSImage *cellImage = [cell image];
      if(cellImage == [NSImage imageNamed:@"common_ret"])
	{
	  [cell setImage: nil];
	  [cell setAlternateImage: nil];
	}
    }
}

- (void) drawButton: (NSRect)frame 
                 in: (NSCell*)cell 
               view: (NSView*)view 
              style: (int)style 
              state: (GSThemeControlState)state
{
  GSDrawTiles	*tiles = nil;
  NSColor	*color = nil;
  NSString	*name = [self nameForElement: cell];

  if (name == nil)
    {
      name = GSStringFromBezelStyle(style);
    }

  color = [self colorNamed: name state: state];
  if (color == nil)
    {
      if (state == GSThemeNormalState)
	{
          color = [NSColor controlBackgroundColor];
	}
      else if (state == GSThemeHighlightedState
	       || state == GSThemeHighlightedFirstResponderState)
	{
          color = [NSColor selectedControlColor];
	}
      else if (state == GSThemeSelectedState
	       || state == GSThemeSelectedFirstResponderState)
	{
          color = [NSColor selectedControlColor];
	}
      else
    	{
          color = [NSColor controlBackgroundColor];
	}
    }

  tiles = [self tilesNamed: name state: state];
  if (tiles == nil)
    {
      tiles = [self tilesNamed: @"NSButton" state: state];
    }

  if (tiles == nil)
    {
      switch (style)
        {
	  case NSRoundRectBezelStyle:
	  case NSTexturedRoundedBezelStyle:
	  case NSRoundedBezelStyle:
	    [self drawRoundBezel: frame withColor: color];
	    break;
	  case NSTexturedSquareBezelStyle:
	    frame = NSInsetRect(frame, 0, 1);
	  case NSSmallSquareBezelStyle:
	  case NSRegularSquareBezelStyle:
	  case NSShadowlessSquareBezelStyle:
	    [color set];
	    NSRectFill(frame);
	    [[NSColor controlShadowColor] set];
	    NSFrameRectWithWidth(frame, 1);
	    break;
	  case NSThickSquareBezelStyle:
	    [color set];
	    NSRectFill(frame);
	    [[NSColor controlShadowColor] set];
	    NSFrameRectWithWidth(frame, 1.5);
	    break;
	  case NSThickerSquareBezelStyle:
	    [color set];
	    NSRectFill(frame);
	    [[NSColor controlShadowColor] set];
	    NSFrameRectWithWidth(frame, 2);
	    break;
	  case NSCircularBezelStyle:
	    frame = NSInsetRect(frame, 3, 3);
	    [self drawCircularBezel: frame withColor: color]; 
	    break;
	  case NSHelpButtonBezelStyle:
	    [self drawCircularBezel: frame withColor: color];
	    {
	      NSDictionary *attributes = [NSDictionary dictionaryWithObject: [NSFont controlContentFontOfSize: 0]
								     forKey: NSFontAttributeName];
	      NSAttributedString *questionMark = [[[NSAttributedString alloc]
						    initWithString: _(@"?")
							attributes: attributes] autorelease];

	      NSRect textRect;
	      textRect.size = [questionMark size];
	      textRect.origin.x = NSMidX(frame) - (textRect.size.width / 2);
	      textRect.origin.y = NSMidY(frame) - (textRect.size.height / 2);

	      [questionMark drawInRect: textRect];
	    }
	    break;
	  case NSDisclosureBezelStyle:
	  case NSRoundedDisclosureBezelStyle:
	  case NSRecessedBezelStyle:
	    // FIXME
	    break;
	  default:
	    [color set];
	    NSRectFill(frame);

	    if (state == GSThemeNormalState || state == GSThemeHighlightedState)
	      {
		[self drawButton: frame withClip: NSZeroRect];
	      }
	    else if (state == GSThemeSelectedState || state == GSThemeSelectedFirstResponderState)
	      {
		[self drawGrayBezel: frame withClip: NSZeroRect];
	      }
	    else
	      {
		[self drawButton: frame withClip: NSZeroRect];
	      }
	}
    }
  else
    {
      /* Use tiles to draw button border with central part filled with color
       */
      [self fillRect: frame
	   withTiles: tiles
	  background: color];
    }
}

- (GSThemeMargins) buttonMarginsForCell: (NSCell*)cell
				  style: (int)style 
				  state: (GSThemeControlState)state
{
  GSDrawTiles	*tiles = nil;
  NSString	*name = [self nameForElement: cell];
  GSThemeMargins margins;

  if (name == nil)
    {
      name = GSStringFromBezelStyle(style);
    }

  tiles = [self tilesNamed: name state: state];
  if (tiles == nil)
    {
      tiles = [self tilesNamed: @"NSButton" state: state];
    } 

  if (tiles == nil)
    {
      switch (style)
        {
	  case NSRoundRectBezelStyle:
	  case NSTexturedRoundedBezelStyle:
	  case NSRoundedBezelStyle:
	    margins.left = 5; margins.top = 5; margins.right = 5; margins.bottom = 5;
	    return margins;
	  case NSTexturedSquareBezelStyle:
	    margins.left = 3; margins.top = 3; margins.right = 3; margins.bottom = 3;
	    return margins;
	  case NSSmallSquareBezelStyle:
	  case NSRegularSquareBezelStyle:
	  case NSShadowlessSquareBezelStyle:
	    margins.left = 2; margins.top = 2; margins.right = 2; margins.bottom = 2;
	    return margins;
	  case NSThickSquareBezelStyle:
	    margins.left = 3; margins.top = 3; margins.right = 3; margins.bottom = 3;
	    return margins;
	  case NSThickerSquareBezelStyle:
	    margins.left = 4; margins.top = 4; margins.right = 4; margins.bottom = 4;
	    return margins;
	  case NSCircularBezelStyle:
	    margins.left = 5; margins.top = 5; margins.right = 5; margins.bottom = 5;
	    return margins;
	  case NSHelpButtonBezelStyle:
	    margins.left = 2; margins.top = 2; margins.right = 2; margins.bottom = 2;
	    return margins;
	  case NSDisclosureBezelStyle:
	  case NSRoundedDisclosureBezelStyle:
	  case NSRecessedBezelStyle:
	    // FIXME
	    margins.left = 0; margins.top = 0; margins.right = 0; margins.bottom = 0;
	    return margins;
	  default:
	    margins.left = 2; margins.top = 2; margins.right = 3; margins.bottom = 3;
	    return margins;
	}
    }
  else
    {
      margins = [tiles themeMargins];
      return margins;
    }
}

- (void) drawFocusFrame: (NSRect) frame view: (NSView*) view
{
  GSDrawTiles *tiles = [self tilesNamed: @"NSFocusRing" state: GSThemeNormalState];

  if (tiles == nil)
    {    
      NSDottedFrameRect(frame);
    }
  else
    {
      [self fillRect: frame
           withTiles: tiles];
    }
}

- (void) drawWindowBackground: (NSRect) frame view: (NSView*) view
{
  NSColor *c;

  c = [[view window] backgroundColor];
  [c set];
  NSRectFill (frame);
}

- (void) drawBorderType: (NSBorderType)aType 
                  frame: (NSRect)frame 
                   view: (NSView*)view
{
  NSString      *name = GSStringFromBorderType(aType);
  GSDrawTiles   *tiles = [self tilesNamed: name state: GSThemeNormalState];

  if (tiles == nil)
    {
      switch (aType)
	{
	  case NSLineBorder:
	    [[NSColor controlDarkShadowColor] set];
	    NSFrameRect(frame);
	    break;
	  case NSGrooveBorder:
	    [self drawGroove: frame withClip: NSZeroRect];
	    break;
	  case NSBezelBorder:
	    [self drawWhiteBezel: frame withClip: NSZeroRect];
	    break;
	  case NSNoBorder: 
	  default:
	    break;
	}
    }
  else
    {
      [self fillRect: frame
           withTiles: tiles];
    }
}

- (NSSize) sizeForBorderType: (NSBorderType)aType
{
  NSString      *name = GSStringFromBorderType(aType);
  GSDrawTiles   *tiles = [self tilesNamed: name state: GSThemeNormalState];

  if (tiles == nil)
    {
      // Returns the size of a border
      switch (aType)
	{
	  case NSLineBorder:
	    return NSMakeSize(1, 1);
	  case NSGrooveBorder:
	  case NSBezelBorder:
	    return NSMakeSize(2, 2);
	  case NSNoBorder: 
	  default:
	    return NSZeroSize;
	}
    }
  else
    {
      // FIXME: We assume the button's top and right padding are the same as
      // its bottom and left.

      GSThemeMargins margins = [tiles themeMargins];
      return NSMakeSize(margins.left, margins.bottom);
    }
}

- (void) drawBorderForImageFrameStyle: (NSImageFrameStyle)frameStyle
                                frame: (NSRect)frame 
                                 view: (NSView*)view
{
  NSString      *name = GSStringFromImageFrameStyle(frameStyle);
  GSDrawTiles   *tiles = [self tilesNamed: name state: GSThemeNormalState];

  if (tiles == nil)
    {
      switch (frameStyle)
	{
	case NSImageFrameNone:
	  // do nothing
	  break;
	case NSImageFramePhoto:
	  [self drawFramePhoto: frame withClip: NSZeroRect];
	  break;
	case NSImageFrameGrayBezel:
	  [self drawGrayBezel: frame withClip: NSZeroRect];
	  break;
	case NSImageFrameGroove:
	  [self drawGroove: frame withClip: NSZeroRect];
	  break;
	case NSImageFrameButton:
	  [self drawButton: frame withClip: NSZeroRect];
	  break;
	}
    }
  else
    {
      [self fillRect: frame
           withTiles: tiles];
    }
}

- (NSSize) sizeForImageFrameStyle: (NSImageFrameStyle)frameStyle
{
  // Get border size
  switch (frameStyle)
    {
      case NSImageFrameNone:
      default:
        return NSZeroSize;
      case NSImageFramePhoto:
        // FIXME
        return NSMakeSize(2, 2);
      case NSImageFrameGrayBezel:
      case NSImageFrameGroove:
      case NSImageFrameButton:
        return NSMakeSize(2, 2);
    }
}


/* NSScroller themeing.
 */
- (BOOL) scrollerArrowsSameEndForScroller: (NSScroller *)aScroller
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];

  if ([defs objectForKey: @"GSScrollerArrowsSameEnd"] != nil)
    {
      return [defs boolForKey: @"GSScrollerArrowsSameEnd"];
    }
  else
    {
      NSInterfaceStyle interfaceStyle = 
	NSInterfaceStyleForKey(@"NSScrollerInterfaceStyle", aScroller);
      
      if ((interfaceStyle == NSNextStepInterfaceStyle 
	   || interfaceStyle == NSMacintoshInterfaceStyle
	   || interfaceStyle == GSWindowMakerInterfaceStyle))
	{
	  return YES;
	}
      else
	{
	  return NO;
	}
    }
}

- (BOOL) scrollerScrollsByPageForScroller: (NSScroller *)aScroller
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];

  if ([defs objectForKey: @"GSScrollerScrollsByPage"] != nil)
    {
      return [defs boolForKey: @"GSScrollerScrollsByPage"];
    }
  else
    {
      NSInterfaceStyle interfaceStyle = 
	NSInterfaceStyleForKey(@"NSScrollerInterfaceStyle", aScroller);
      
      if (interfaceStyle == NSNextStepInterfaceStyle 
	  || interfaceStyle == NSMacintoshInterfaceStyle
	  || interfaceStyle == GSWindowMakerInterfaceStyle)
	{
	  /* NeXTstep style is to scroll to point.
	   */
	  return NO;
	}
      else
	{
	  /* Windows style is to scroll by a page.
	   */
	  return YES;
	}
    }
}

- (NSButtonCell*) cellForScrollerArrow: (NSScrollerArrow)arrow
			    horizontal: (BOOL)horizontal
{
  NSButtonCell	*cell;
  NSString	*name;
  
  cell = [NSButtonCell new];
  if (horizontal)
    {
      if (arrow == NSScrollerDecrementArrow)
	{
	  [cell setHighlightsBy:
	    NSChangeBackgroundCellMask | NSContentsCellMask];
	  [cell setImage: [NSImage imageNamed: @"common_ArrowLeft"]];
	  [cell setAlternateImage: [NSImage imageNamed: @"common_ArrowLeftH"]];
	  [cell setImagePosition: NSImageOnly];
          name = GSScrollerLeftArrow;
	}
      else
	{
	  [cell setHighlightsBy:
	    NSChangeBackgroundCellMask | NSContentsCellMask];
	  [cell setImage: [NSImage imageNamed: @"common_ArrowRight"]];
	  [cell setAlternateImage: [NSImage imageNamed: @"common_ArrowRightH"]];
	  [cell setImagePosition: NSImageOnly];
          name = GSScrollerRightArrow;
	}
    }
  else
    {
      if (arrow == NSScrollerDecrementArrow)
	{
	  [cell setHighlightsBy:
	    NSChangeBackgroundCellMask | NSContentsCellMask];
	  [cell setImage: [NSImage imageNamed: @"common_ArrowUp"]];
	  [cell setAlternateImage: [NSImage imageNamed: @"common_ArrowUpH"]];
	  [cell setImagePosition: NSImageOnly];
          name = GSScrollerUpArrow;
	}
      else
	{
	  [cell setHighlightsBy:
	    NSChangeBackgroundCellMask | NSContentsCellMask];
	  [cell setImage: [NSImage imageNamed: @"common_ArrowDown"]];
	  [cell setAlternateImage: [NSImage imageNamed: @"common_ArrowDownH"]];
	  [cell setImagePosition: NSImageOnly];
          name = GSScrollerDownArrow;
	}
    }
  [self setName: name forElement: cell temporary: YES];
  RELEASE(cell);
  return cell;
}

- (NSCell*) cellForScrollerKnob: (BOOL)horizontal
{
  NSButtonCell	*cell;

  cell = [NSButtonCell new];
  [cell setButtonType: NSMomentaryChangeButton];
  [cell setImagePosition: NSImageOnly];
  if (horizontal)
    {
      [self setName: GSScrollerHorizontalKnob forElement: cell temporary: YES];
      [cell setImage: [NSImage imageNamed: @"common_DimpleHoriz"]];
    }
  else
    {
      [self setName: GSScrollerVerticalKnob forElement: cell temporary: YES];
      [cell setImage: [NSImage imageNamed: @"common_Dimple"]];
  
    }
  RELEASE(cell);
  return cell;
}

- (NSCell*) cellForScrollerKnobSlot: (BOOL)horizontal
{
  GSDrawTiles   *tiles;
  NSButtonCell	*cell;
  NSColor	*color;
  NSString      *name;

  if (horizontal)
    {
      name = GSScrollerHorizontalSlot;
    }
  else
    {
      name = GSScrollerVerticalSlot;
    }

  tiles = [self tilesNamed: name state: GSThemeNormalState];
  color = [self colorNamed: name state: GSThemeNormalState];

  cell = [NSButtonCell new];
  [cell setBordered: (tiles != nil)];
  [cell setTitle: nil];

  [self setName: name forElement: cell temporary: YES];
 
  if (color == nil)
    {
      color = [NSColor scrollBarColor];
    }
  [cell setBackgroundColor: color];
  RELEASE(cell);
  return cell;
}

- (float) defaultScrollerWidth
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  float defaultScrollerWidth;

  if ([defs objectForKey: @"GSScrollerDefaultWidth"] != nil)
    {
      defaultScrollerWidth = [defs floatForKey: @"GSScrollerDefaultWidth"];
    }
  else
    {
      defaultScrollerWidth = 18.0;
    }
  return defaultScrollerWidth;
}

- (BOOL) scrollViewUseBottomCorner
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  if ([defs objectForKey: @"GSScrollViewUseBottomCorner"] != nil)
    {
      return [defs boolForKey: @"GSScrollViewUseBottomCorner"];
    }
  return YES;
}

- (BOOL) scrollViewScrollersOverlapBorders
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  if ([defs objectForKey: @"GSScrollViewScrollersOverlapBorders"] != nil)
    {
      return [defs boolForKey: @"GSScrollViewScrollersOverlapBorders"];
    }
  return NO;
}

- (NSColor *) toolbarBackgroundColor
{
  NSColor *color;

  color = [self colorNamed: @"toolbarBackgroundColor"
                state: GSThemeNormalState];
  if (color == nil)
    {
      color = [NSColor clearColor];
    }
  return color;
}

- (NSColor *) toolbarBorderColor
{
  NSColor *color;

  color = [self colorNamed: @"toolbarBorderColor"
                state: GSThemeNormalState];
  if (color == nil)
    {
      color = [NSColor darkGrayColor];
    }
  return color;
}

- (void) drawToolbarRect: (NSRect)aRect
                   frame: (NSRect)viewFrame
              borderMask: (unsigned int)borderMask
{
  // We draw the background
  [[self toolbarBackgroundColor] set];
  [NSBezierPath fillRect: aRect];
  
  // We draw the border
  [[self toolbarBorderColor] set];
  if (borderMask & GSToolbarViewBottomBorder)
    {
      [NSBezierPath strokeLineFromPoint: NSMakePoint(0, 0.5) 
                    toPoint: NSMakePoint(viewFrame.size.width, 0.5)];
    }
  if (borderMask & GSToolbarViewTopBorder)
    {
      [NSBezierPath strokeLineFromPoint: NSMakePoint(0, 
                                                     viewFrame.size.height - 0.5) 
                    toPoint: NSMakePoint(viewFrame.size.width, 
                                         viewFrame.size.height -  0.5)];
    }
  if (borderMask & GSToolbarViewLeftBorder)
    {
      [NSBezierPath strokeLineFromPoint: NSMakePoint(0.5, 0) 
                    toPoint: NSMakePoint(0.5, viewFrame.size.height)];
    }
  if (borderMask & GSToolbarViewRightBorder)
    {
      [NSBezierPath strokeLineFromPoint: NSMakePoint(viewFrame.size.width - 0.5,0)
                    toPoint: NSMakePoint(viewFrame.size.width - 0.5, 
                                         viewFrame.size.height)];
    }
}

- (BOOL) toolbarIsOpaque
{
  if ([[self toolbarBackgroundColor] alphaComponent] < 1.0)
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (NSRect) stepperUpButtonRectWithFrame: (NSRect)frame
{
  NSSize size = [[NSImage imageNamed: @"common_StepperUp"] size];
  NSRect upRect = {{NSMinX(frame), NSMinY(frame)}, {size.width, size.height}};

  upRect.origin.x += ((int)frame.size.width / 2) - ((int)size.width / 2);
  upRect.origin.y += ((int)frame.size.height / 2);
  return upRect;
}

- (NSRect) stepperDownButtonRectWithFrame: (NSRect)frame
{
  NSSize size = [[NSImage imageNamed: @"common_StepperDown"] size];
  NSRect downRect = {{NSMinX(frame), NSMinY(frame)}, {size.width, size.height}};

  downRect.origin.x += ((int)frame.size.width / 2) - ((int)size.width / 2);
  downRect.origin.y += ((int)frame.size.height / 2) - size.height;
  return downRect;
}

- (void) drawStepperBorder: (NSRect)frame
{
}

- (NSRect) drawStepperLightButton: (NSRect)border : (NSRect)clip
{
  return NSZeroRect;
}

- (void) drawStepperUpButton: (NSRect)aRect
{
  NSImage *image = [NSImage imageNamed: @"common_StepperUp"];
  [image drawInRect: aRect
	   fromRect: NSZeroRect
	  operation: NSCompositeSourceOver
	   fraction: 1
     respectFlipped: YES
	      hints: nil];
}

- (void) drawStepperHighlightUpButton: (NSRect)aRect
{
  NSImage *image = [NSImage imageNamed: @"common_StepperUpHighlighted"];
  [image drawInRect: aRect
	   fromRect: NSZeroRect
	  operation: NSCompositeSourceOver
	   fraction: 1
     respectFlipped: YES
	      hints: nil];
}

- (void) drawStepperDownButton: (NSRect)aRect
{
  NSImage *image = [NSImage imageNamed: @"common_StepperDown"];
  [image drawInRect: aRect
	   fromRect: NSZeroRect
	  operation: NSCompositeSourceOver
	   fraction: 1
     respectFlipped: YES
	      hints: nil];
}

- (void) drawStepperHighlightDownButton: (NSRect)aRect
{
  NSImage *image = [NSImage imageNamed: @"common_StepperDownHighlighted"];
  [image drawInRect: aRect
	   fromRect: NSZeroRect
	  operation: NSCompositeSourceOver
	   fraction: 1
     respectFlipped: YES
	      hints: nil];
}

- (void) drawStepperCell: (NSCell*)cell
               withFrame: (NSRect)cellFrame
                  inView: (NSView*)controlView
             highlightUp: (BOOL)highlightUp
           highlightDown: (BOOL)highlightDown
{
  const NSRect upRect = [self stepperUpButtonRectWithFrame: cellFrame];
  const NSRect downRect = [self stepperDownButtonRectWithFrame: cellFrame];

  [self drawStepperBorder: cellFrame];
  
  if (highlightUp)
    [self drawStepperHighlightUpButton: upRect];
  else
    [self drawStepperUpButton: upRect];
  
  if (highlightDown)
    [self drawStepperHighlightDownButton: downRect];
  else
    [self drawStepperDownButton: downRect];
}

// NSSwitch drawing methods

- (void) drawSwitchBezel: (NSRect)frame
                forState: (NSControlStateValue)v
                 enabled: (BOOL)enabled
{
  NSBezierPath *p;
  NSPoint point;
  CGFloat radius;
  NSColor *backgroundColor;
  
  if (enabled == NO)
    {
      backgroundColor = [NSColor disabledControlTextColor];
    }
  else if (NSControlStateValueOn != v)
    {
      backgroundColor = [NSColor windowBackgroundColor]; // offColor
    }
  else
    {
      backgroundColor = [NSColor selectedControlColor]; // onColor
    }
  
  // make smaller than enclosing frame
  frame = NSInsetRect(frame, 4, 4);
  radius = frame.size.height / 2.0;
  point = frame.origin;
  point.x += radius;
  point.y += radius - 0.5;

  // Draw initial path to enclose the button...
  // left half-circle
  p = [NSBezierPath bezierPath];
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 90.0
			      endAngle: 270.0];

  // line to first point and right halfcircle
  point.x += frame.size.width - frame.size.height;
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 270.0
			      endAngle: 90.0];
  [p closePath];

  // fill with background color
  [backgroundColor set];
  [p fill];

  // and stroke rounded button
  [[NSColor shadowColor] set];
  [p stroke];

  // Add highlights...
  point = frame.origin;
  point.x += radius - 0.5;
  point.y += radius - 0.5;
  p = [NSBezierPath bezierPath];
  [p setLineWidth: 1.0];
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 135.0
			      endAngle: 270.0];

  // line to first point and right halfcircle
  point.x += frame.size.width - frame.size.height;
  [p appendBezierPathWithArcWithCenter: point
				radius: radius
			    startAngle: 270.0
			      endAngle: 315.0];
  [[NSColor controlLightHighlightColor] set];
  [p stroke];
}

- (void) drawSwitchKnob: (NSRect)frame
               forState: (NSControlStateValue)value
                enabled: (BOOL)enabled
{
  NSColor *backgroundColor = enabled ? [NSColor windowBackgroundColor] : [NSColor disabledControlTextColor];
  NSBezierPath *oval;
  NSRect rect = NSZeroRect;
  CGFloat w = (frame.size.width / 2) - 2;
  CGFloat h = frame.size.height - 6;
  CGFloat y = frame.origin.y + 2;
  CGFloat radius = frame.size.height / 2.0;

  [backgroundColor set];
  if (value == NSControlStateValueOff)
    {
      rect = NSMakeRect(frame.origin.x + 4,
                        y,
                        w,
                        h);
    }
  else
    {
      rect = NSMakeRect(frame.origin.x + ((frame.size.width - 2 * radius) + 2), // ((frame.size.width - w) - 2)
                        y,
                        w,
                        h);
    }
  
  
  oval = [NSBezierPath bezierPathWithOvalInRect: NSInsetRect(rect, 1, 1)];
  
  // fill oval with background color
  [backgroundColor set];
  [oval fill];
  
  // and stroke rounded button
  [[NSColor shadowColor] set];
  [oval stroke];
}


- (void) drawSwitchInRect: (NSRect)rect
                 forState: (NSControlStateValue)state
                  enabled: (BOOL)enabled
{
  // Draw the well bezel
  [self drawSwitchBezel: rect
               forState: state
                enabled: enabled];

  // Draw the knob
  [self drawSwitchKnob: rect
              forState: state
               enabled: enabled];
        
}

// NSPathComponentCell

- (void) drawPathComponentCellWithFrame: (NSRect)frame
                                 inView: (NSPathControl *)pc
                               withCell: (NSPathComponentCell *)cell
                        isLastComponent: (BOOL)last
{
  NSImage *img = [cell image];
  NSURL *url = [cell URL];
  NSString *string = [[url path] lastPathComponent];
  NSRect textFrame = frame;
  NSRect imgFrame = frame;
  NSRect arrowFrame = frame;
  NSImage *arrowImage = [NSImage imageNamed: @"NSMenuArrow"];
  NSPathStyle style= [pc pathStyle];
  NSRect newFrame = frame;

  if (style == NSPathStylePopUp)
    {
      newFrame = [pc frame];
      
      // Reset coodinates.
      newFrame.origin.x = 0.0;
      newFrame.origin.y = 0.0;
      
      // Use control frame...
      textFrame = newFrame;
      imgFrame = newFrame;
      arrowFrame = newFrame;    
    }
  
  // Modify positions...
  imgFrame.size.width = 17.0;
  imgFrame.size.height = 17.0;
  imgFrame.origin.x += 2.0;
  imgFrame.origin.y += 2.0;
  textFrame.origin.x += imgFrame.size.width + 5.0; // the width of the image plus a few pixels.
  textFrame.origin.y += 5.0; // center with the image...
  arrowFrame.origin.x += newFrame.size.width - 17.0; 
  arrowFrame.size.width = 8.0;
  arrowFrame.size.height = 8.0;
  arrowFrame.origin.y += 5.0;
      
  if (style== NSPathStyleStandard || style== NSPathStyleNavigationBar)
    {
      // Draw the image...
      [img drawInRect: imgFrame];
      
      // Draw the text...
      [[NSColor textColor] set]; 
      [string drawAtPoint: textFrame.origin
           withAttributes: nil];
      
      // Draw the arrow...
      if (last == NO)
        {
          [arrowImage drawInRect: arrowFrame];
        }
    }
  else if (style == NSPathStylePopUp)
    {
      if (last == YES)
        {
          arrowImage = [NSImage imageNamed: @"common_ArrowDown"];

          // Draw border...
          [[NSColor controlShadowColor] set];
          NSFrameRectWithWidth(newFrame, 1.0);
          
          // Draw the image...
          [img drawInRect: imgFrame];
          
          // Draw the text...
          [[NSColor textColor] set]; 
          [string drawAtPoint: textFrame.origin
               withAttributes: nil];
          
          // Draw the arrow...
          [arrowImage drawInRect: arrowFrame];
        }
    }
}

// NSSegmentedControl drawing methods

- (void) drawSegmentedControlSegment: (NSCell *)cell
                           withFrame: (NSRect)cellFrame
                              inView: (NSView *)controlView
                               style: (NSSegmentStyle)style  
                               state: (GSThemeControlState)state
                         roundedLeft: (BOOL)roundedLeft
                        roundedRight: (BOOL)roundedRight
{
  GSDrawTiles *tiles;
  NSString  *name = GSStringFromSegmentStyle(style);
  if (roundedLeft)
    {
      name = [name stringByAppendingString: @"RoundedLeft"];
    }
  if (roundedRight)
    {
      name = [name stringByAppendingString: @"RoundedRight"];
    }

  tiles = [self tilesNamed: name state: state];
 
  if (tiles == nil)
    {
      [self drawButton: cellFrame
                    in: cell
                  view: controlView
                 style: NSRegularSquareBezelStyle
                 state: state];
    }
  else
    {
      [self fillRect: cellFrame
           withTiles: tiles];
    }
}

- (NSColor *) menuBackgroundColor
{
  NSColor *color = [self colorNamed: @"menuBackgroundColor"
                              state: GSThemeNormalState];
  if (color == nil)
    {
      color = [NSColor windowBackgroundColor];
    }
  return color;
}

- (NSColor *) menuItemBackgroundColor
{
  NSColor *color = [self colorNamed: @"menuItemBackgroundColor"
                              state: GSThemeNormalState];
  if (color == nil)
    {
      color = [NSColor controlBackgroundColor];
    }
  return color;
}

- (NSColor *) menuBorderColor
{
  NSColor *color = [self colorNamed: @"menuBorderColor"
                              state: GSThemeNormalState];
  if (color == nil)
    {
      color = [NSColor darkGrayColor];
    }
  return color;
}

- (NSColor *) menuBarBackgroundColor
{
  NSColor *color = [self colorNamed: @"menuBarBackgroundColor"
                              state: GSThemeNormalState];
  if (color == nil)
    {
      color = [self menuBackgroundColor];
    }
  return color;
}

- (NSColor *) menuBarBorderColor
{
  NSColor *color = [self colorNamed: @"menuBarBorderColor"
                              state: GSThemeNormalState];
  if (color == nil)
    {
      color = [self menuBorderColor];
    }
  return color;
}

- (NSColor *) menuBorderColorForEdge: (NSRectEdge)edge isHorizontal: (BOOL)horizontal
{
  if (horizontal && edge == NSMinYEdge)
    {
      return [self menuBorderColor];
    }
  else if (edge == NSMinXEdge || edge == NSMaxYEdge)
    {
      // Draw the dark gray upper left lines.
      return [self menuBorderColor];
    }
  return nil;
}

- (void) drawBackgroundForMenuView: (NSMenuView*)menuView
                         withFrame: (NSRect)bounds
                         dirtyRect: (NSRect)dirtyRect
                        horizontal: (BOOL)horizontal 
{
  NSString  *name = horizontal ? GSMenuHorizontalBackground : 
    GSMenuVerticalBackground;
  GSDrawTiles *tiles = [self tilesNamed: name state: GSThemeNormalState];
 
  if (tiles == nil)
    {
      NSRectEdge sides[4] = { NSMinXEdge, NSMaxYEdge, NSMaxXEdge, NSMinYEdge }; 
      NSColor *colors[] = {[self menuBorderColorForEdge: NSMinXEdge isHorizontal: horizontal], 
                           [self menuBorderColorForEdge: NSMaxYEdge isHorizontal: horizontal], 
                           [self menuBorderColorForEdge: NSMaxXEdge isHorizontal: horizontal],
                           [self menuBorderColorForEdge: NSMinYEdge isHorizontal: horizontal]};

      [[self menuBackgroundColor] set];
      NSRectFill(NSIntersectionRect(bounds, dirtyRect));
      NSDrawColorTiledRects(bounds, dirtyRect, sides, colors, 4);
    }
  else
    {
      [self fillRect: bounds
           withTiles: tiles];
    }
}

- (BOOL) drawsBorderForMenuItemCell: (NSMenuItemCell *)cell 
                              state: (GSThemeControlState)state
                       isHorizontal: (BOOL)horizontal
{
  return [cell isBordered];
}

- (void) drawBorderAndBackgroundForMenuItemCell: (NSMenuItemCell *)cell
                                      withFrame: (NSRect)cellFrame
                                         inView: (NSView *)controlView
                                          state: (GSThemeControlState)state
                                   isHorizontal: (BOOL)isHorizontal
{
  NSString  *name = isHorizontal ? GSMenuHorizontalItem :
    GSMenuVerticalItem;
  GSDrawTiles *tiles = [self tilesNamed: name state: state];
 
  if (tiles == nil)
    {
      NSColor	*backgroundColor = [cell backgroundColor];

      if (isHorizontal)
	{
	  cellFrame = [cell drawingRectForBounds: cellFrame];
	  [backgroundColor set];
	  NSRectFill(cellFrame);
	  return;
	}

      // Set cell's background color
      [backgroundColor set];
      NSRectFill(cellFrame);

      if (![self drawsBorderForMenuItemCell: cell 
                                      state: state 
                               isHorizontal: isHorizontal])
        {
          return;
        }

      if (state == GSThemeSelectedState)
	{
          [self drawGrayBezel: cellFrame withClip: NSZeroRect];
        }
      else
        {
          [self drawButton: cellFrame withClip: NSZeroRect];
        }
    }
  else
    {
      [self fillRect: cellFrame
           withTiles: tiles];
    }
}

- (NSColor *) menuSeparatorColor
{
  NSColor *color = [self colorNamed: @"menuSeparatorColor"
                              state: GSThemeNormalState];
  NSInterfaceStyle style = NSInterfaceStyleForKey(@"NSMenuInterfaceStyle", nil);

  // TODO: Remove the style check... Windows theming should be in a subclass 
  // probably
  if (color == nil && style == NSWindows95InterfaceStyle)
    {
      color = [NSColor blackColor];
    }
  return color;
}

- (CGFloat) menuSeparatorInset
{
  return 3.0;
}

- (CGFloat) menuSubmenuHorizontalOverlap
{
  return [[NSUserDefaults standardUserDefaults]
    floatForKey: @"GSMenuSubmenuHorizontalOverlap"];
}

- (CGFloat) menuSubmenuVerticalOverlap
{
  return [[NSUserDefaults standardUserDefaults]
    floatForKey: @"GSMenuSubmenuVerticalOverlap"];
}

- (void) drawSeparatorItemForMenuItemCell: (NSMenuItemCell *)cell
                                withFrame: (NSRect)cellFrame
                                   inView: (NSView *)controlView
                             isHorizontal: (BOOL)isHorizontal
{
  GSDrawTiles *tiles;
 
  tiles = [self tilesNamed: GSMenuSeparatorItem state: GSThemeNormalState];
  if (tiles == nil)
    {
      NSBezierPath *path = [NSBezierPath bezierPath];
      CGFloat inset = [self menuSeparatorInset];
      NSPoint start = NSMakePoint(inset, cellFrame.size.height / 2 + 
				         cellFrame.origin.y + 0.5);
      NSPoint end = NSMakePoint(cellFrame.size.width - inset, 
        cellFrame.size.height / 2 + cellFrame.origin.y + 0.5);

      [[self menuSeparatorColor] set];

      [path setLineWidth: 0.0];
      [path moveToPoint: start];
      [path lineToPoint: end];

      [path stroke];
    }
  else
    {
      [self fillRect: cellFrame
           withTiles: tiles];
    }
}

- (void) drawTitleForMenuItemCell: (NSMenuItemCell *)cell
                        withFrame: (NSRect)cellFrame
                           inView: (NSView *)controlView
                            state: (GSThemeControlState)state
                     isHorizontal: (BOOL)isHorizontal
{
  [cell _drawText: [[cell menuItem] title]
          inFrame: [cell titleRectForBounds: cellFrame]];
}

- (Class) titleViewClassForMenuView: (NSMenuView *)aMenuView
{
  return [GSTitleView class];
}

- (NSRect) drawMenuTitleBackground: (GSTitleView *)aTitleView
			withBounds: (NSRect)bounds
			  withClip: (NSRect)clipRect
{
  GSDrawTiles *tiles = [self tilesNamed: GSMenuTitleBackground state: GSThemeNormalState];
 
  if (tiles == nil)
    {
      NSRect     workRect = bounds;
      NSRectEdge top_left[] = {NSMinXEdge, NSMaxYEdge};
      CGFloat      darkGrays[] = {NSDarkGray, NSDarkGray};
      NSColor *titleColor;

      titleColor = [self colorNamed: @"GSMenuBar" state: GSThemeNormalState];
      if (titleColor == nil)
	{
	  titleColor = [NSColor blackColor];
	}
 
      // Draw the dark gray upper left lines for menu and black for others.
      // Rectangle 1
      workRect = NSDrawTiledRects(workRect, workRect, top_left, darkGrays, 2);
      
      // Rectangle 2
      // Draw the title box's button.
      [self drawButton: workRect withClip: workRect];
      
      // Overdraw white top and left lines with light gray lines for window title
      workRect.origin.y += 1;
      workRect.size.height -= 1;
      workRect.size.width -= 1;
      
      // Rectangle 3
      // Paint background
      workRect.origin.x += 1;
      workRect.origin.y += 1;
      workRect.size.height -= 2;
      workRect.size.width -= 2;
      
      [titleColor set];
      NSRectFill(workRect);

      return workRect;
    }
  else
    {
      return [self fillRect: bounds
		  withTiles: tiles];
    }
}

- (CGFloat) menuBarHeight
{
  CGFloat height = [[NSUserDefaults standardUserDefaults]
		     floatForKey: @"GSMenuBarHeight"];
  if (height <= 0)
    {
      return 22;
    }
  return height;
}

- (CGFloat) menuItemHeight
{
  CGFloat height = [[NSUserDefaults standardUserDefaults]
		     floatForKey: @"GSMenuItemHeight"];
  if (height <= 0)
    {
      return 20;
    }
  return height;
}

- (CGFloat) menuSeparatorHeight
{
  CGFloat height = [[NSUserDefaults standardUserDefaults]
		     floatForKey: @"GSMenuSeparatorHeight"];
  if (height <= 0)
    {
      return 20;
    }
  return height;
}

// NSColorWell drawing method
- (NSRect) drawColorWellBorder: (NSColorWell*)well
                    withBounds: (NSRect)bounds
                      withClip: (NSRect)clipRect
{
  NSRect aRect = bounds;

  if ([well isBordered])
    {
      GSThemeControlState state;
      GSDrawTiles *tiles;

      if ([[well cell] isHighlighted] || [well isActive])
	{
          state = GSThemeHighlightedState;
	}
      else
	{
          state = GSThemeNormalState;
	}

      tiles = [self tilesNamed: GSColorWell state: state];
      if (tiles == nil)
        {
	  /*
	   * Draw border.
	   */
	  [self drawButton: aRect withClip: clipRect];

	  /*
	   * Fill in control color.
	   */
	  if (state == GSThemeHighlightedState)
	    {
	      [[NSColor selectedControlColor] set];
	    }
	  else
	    {
	      [[NSColor controlColor] set];
	    }
	  aRect = NSInsetRect(aRect, 2.0, 2.0);
	  NSRectFill(NSIntersectionRect(aRect, clipRect));
        }
      else
        {
          aRect = [self fillRect: aRect
                       withTiles: tiles];
        }

      /*
       * Set an inset rect for the color area
       */
      aRect = NSInsetRect(bounds, COLOR_WELL_BORDER_WIDTH, COLOR_WELL_BORDER_WIDTH);
    }

  /*
   * OpenStep 4.2 behavior is to omit the inner border for
   * non-enabled NSColorWell objects.
   */
  if ([well isEnabled])
    {
      GSDrawTiles *tiles = [self tilesNamed: GSColorWellInnerBorder state: GSThemeNormalState];
      if (tiles == nil)
        {
	  /*
	   * Draw inner frame.
	   */
	  [self drawGrayBezel: aRect withClip: clipRect];
	  aRect = NSInsetRect(aRect, 2.0, 2.0);
	}
      else
	{
	  [self fillRect: aRect
	       withTiles: tiles];

	  aRect = [tiles contentRectForRect: aRect isFlipped: [well isFlipped]];
	}
    }

  return aRect;
}

// progress indicator drawing methods
static NSColor *fillColour = nil;
#define MaxCount 10
static int indeterminateMaxCount = MaxCount;
static int spinningMaxCount = MaxCount;
static NSColor *indeterminateColors[MaxCount];
static NSImage *spinningImages[MaxCount];

- (void) initProgressIndicatorDrawing
{
  int i;
  
  // FIXME: Should come from defaults and should be reset when defaults change
  // FIXME: Should probably get the color from the color extension list (see NSToolbar)
  fillColour = RETAIN([NSColor controlShadowColor]);

  // Load images for indeterminate style
  for (i = 0; i < MaxCount; i++)
    {
      NSString *imgName = [NSString stringWithFormat: @"common_ProgressIndeterminate_%d", i + 1];
      NSImage *image = [NSImage imageNamed: imgName];
      
      if (image == nil)
        {
          indeterminateMaxCount = i;
          break;
        }
          indeterminateColors[i] = RETAIN([NSColor colorWithPatternImage: image]);
    }
  
  // Load images for spinning style
  for (i = 0; i < MaxCount; i++)
    {
      NSString *imgName = [NSString stringWithFormat: @"common_ProgressSpinning_%d", i + 1];
      NSImage *image = [NSImage imageNamed: imgName];
      
      if (image == nil)
        {
          spinningMaxCount = i;
          break;
        }
      spinningImages[i] = RETAIN(image); 
    }
}

- (void) drawProgressIndicator: (NSProgressIndicator*)progress
                    withBounds: (NSRect)bounds
                      withClip: (NSRect)rect
                       atCount: (int)count
                      forValue: (double)val
{
   NSRect r;

   if (fillColour == nil)
     {
       [self initProgressIndicatorDrawing];
     }

   // Draw the Bezel
   if ([progress isBezeled])
     {
       // Calc the inside rect to be drawn
       r = [self drawProgressIndicatorBezel: bounds withClip: rect];
     }
   else
     {
       r = bounds;
     }

   if ([progress style] == NSProgressIndicatorSpinningStyle)
     {
       NSRect imgBox = {{0,0}, {0,0}};

       if (spinningMaxCount != 0)
	 {
	   count = count % spinningMaxCount;
	   imgBox.size = [spinningImages[count] size];
	   [spinningImages[count] drawInRect: r 
				    fromRect: imgBox 
				   operation: NSCompositeSourceOver
				    fraction: 1.0];
	 }
     }
   else
     {
       if ([progress isIndeterminate])
         {
	   if (indeterminateMaxCount != 0)
	     {
	       count = count % indeterminateMaxCount;
	       [indeterminateColors[count] set];
	       NSRectFill(r);
	     }
         }
       else
         {
           // Draw determinate 
           if ([progress isVertical])
             {
               float height = NSHeight(r) * val;
               
               if ([progress isFlipped])
                 {
                   // Compensate for the flip
                   r.origin.y += NSHeight(r) - height;
                 }
               r.size.height = height;
             }
           else
             {
               r.size.width = NSWidth(r) * val;
             }
           r = NSIntersectionRect(r, rect);
           if (!NSIsEmptyRect(r))
             {
               [self drawProgressIndicatorBarDeterminate: (NSRect)r];
             }
         }
     }
}

- (NSRect) drawProgressIndicatorBezel: (NSRect)bounds withClip: (NSRect) rect
{
  GSDrawTiles *tiles = [self tilesNamed: GSProgressIndicatorBezel
                                  state: GSThemeNormalState];

  if (tiles == nil)
    {
      return [self drawGrayBezel: bounds withClip: rect];
    }
  else
    {
      [self fillRect: bounds
           withTiles: tiles];

      return [tiles contentRectForRect: bounds
			     isFlipped: [[NSView focusView] isFlipped]];
    }  
}

- (void) drawProgressIndicatorBarDeterminate: (NSRect)bounds
{
  GSDrawTiles *tiles = [self tilesNamed: GSProgressIndicatorBarDeterminate
                                  state: GSThemeNormalState];

  if (tiles == nil)
    {
      [fillColour set];
      NSRectFill(bounds);
    }
  else
    {
      [self fillRect: bounds
           withTiles: tiles
          background: fillColour];
    }
}

// Table drawing methods

- (NSColor *) tableHeaderTextColorForState: (GSThemeControlState)state
{
  NSColor *color;

  color = [self colorNamed: @"tableHeaderTextColor"
		     state: state];
  if (color == nil)
    {
      if (state == GSThemeHighlightedState)
	color = [NSColor controlTextColor];
      else
	color = [NSColor windowFrameTextColor];
    }
  return color;
}

- (void) drawTableCornerView: (NSView*)cornerView
                   withClip: (NSRect)aRect
{
  NSRect divide;
  NSRect rect;
  GSDrawTiles *tiles = [self tilesNamed: GSTableCorner state: GSThemeNormalState];

  if ([cornerView isFlipped])
    {
      NSDivideRect(aRect, &divide, &rect, 1.0, NSMaxYEdge);
    }
  else
    {
      NSDivideRect(aRect, &divide, &rect, 1.0, NSMinYEdge);
    }

  if (tiles == nil)
    { 
      rect = [self drawDarkButton: rect withClip: aRect];
      [[NSColor controlShadowColor] set];
      NSRectFill(rect);
    }
  else
    {
       [self fillRect: aRect
            withTiles: tiles];
    }
}

- (void) drawTableHeaderCell: (NSTableHeaderCell *)cell
                   withFrame: (NSRect)cellFrame
                      inView: (NSView *)controlView
                       state: (GSThemeControlState)state
{
  GSDrawTiles *tiles = [self tilesNamed: GSTableHeader state: state];

  if (tiles == nil)
    {
      NSRect rect;

      // Leave a 1pt thick horizontal line underneath the header
      if (![controlView isFlipped])
        {
          cellFrame.origin.y++;
        }
      cellFrame.size.height--;

      if (state == GSThemeHighlightedState)
        {
          rect = [self drawButton: cellFrame withClip: cellFrame];
          [[NSColor controlColor] set];
          NSRectFill(rect);        
        }
      else
        {
          rect = [self drawDarkButton: cellFrame withClip: cellFrame];
          [[NSColor controlShadowColor] set];
          NSRectFill(rect);
        }
    }
  else
    {
      [self fillRect: cellFrame
           withTiles: tiles];
    }
}


// Window decoration drawing methods
/* These include the black border. */
#define TITLE_HEIGHT 23.0
#define RESIZE_HEIGHT 9.0
#define TITLEBAR_BUTTON_SIZE 15.0
#define TITLEBAR_PADDING_TOP 4.0
#define TITLEBAR_PADDING_RIGHT 4.0
#define TITLEBAR_PADDING_LEFT 4.0

- (float) titlebarHeight
{
  return TITLE_HEIGHT;
}

- (float) resizebarHeight
{
  return RESIZE_HEIGHT;
}

- (float) titlebarButtonSize
{
  return TITLEBAR_BUTTON_SIZE;
}

- (float) titlebarPaddingRight
{
  return TITLEBAR_PADDING_RIGHT;
}

- (float) titlebarPaddingTop
{
  return TITLEBAR_PADDING_TOP;
}

- (float) titlebarPaddingLeft
{
  return TITLEBAR_PADDING_LEFT;
}

static NSDictionary *titleTextAttributes[3] = {nil, nil, nil};

- (void) drawTitleBarRect: (NSRect)titleBarRect 
             forStyleMask: (unsigned int)styleMask
                    state: (int)inputState 
                 andTitle: (NSString*)title
{
  static const NSRectEdge edges[4] = {NSMinXEdge, NSMaxYEdge,
				    NSMaxXEdge, NSMinYEdge};
  CGFloat grays[3][4] =
    {{NSLightGray, NSLightGray, NSDarkGray, NSDarkGray},
    {NSWhite, NSWhite, NSDarkGray, NSDarkGray},
    {NSLightGray, NSLightGray, NSBlack, NSBlack}};
  NSRect workRect;
  GSDrawTiles *tiles = nil;

  if (!titleTextAttributes[0])
    {
      NSMutableParagraphStyle *p;
      NSColor *keyColor, *normalColor, *mainColor;

      p = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
      [p setLineBreakMode: NSLineBreakByClipping];

      // FIXME: refine color names based on style mask
      // (HUD or textured or regular window)

      keyColor = [self colorNamed: @"keyWindowFrameTextColor"
                            state: GSThemeNormalState];
      if (nil == keyColor)
        {
          keyColor = [NSColor windowFrameTextColor];
        }

      normalColor = [self colorNamed: @"normalWindowFrameTextColor"
                               state: GSThemeNormalState];
      if (nil == normalColor)
        {
          normalColor = [NSColor blackColor];
        }
 
      mainColor = [self colorNamed: @"mainWindowFrameTextColor"
                             state: GSThemeNormalState];
      if (nil == mainColor)
        {
          mainColor = [NSColor windowFrameTextColor];
        }
 
      titleTextAttributes[0] = [[NSMutableDictionary alloc]
	initWithObjectsAndKeys:
	  [NSFont titleBarFontOfSize: 0], NSFontAttributeName,
	  keyColor, NSForegroundColorAttributeName,
	  p, NSParagraphStyleAttributeName,
	  nil];

      titleTextAttributes[1] = [[NSMutableDictionary alloc]
	initWithObjectsAndKeys:
	  [NSFont titleBarFontOfSize: 0], NSFontAttributeName,
	  normalColor, NSForegroundColorAttributeName,
	  p, NSParagraphStyleAttributeName,
	  nil];

      titleTextAttributes[2] = [[NSMutableDictionary alloc]
	initWithObjectsAndKeys:
	  [NSFont titleBarFontOfSize: 0], NSFontAttributeName,
	  mainColor, NSForegroundColorAttributeName,
	  p, NSParagraphStyleAttributeName,
	  nil];

      RELEASE(p);
    }

  tiles = [self tilesNamed: @"GSWindowTitleBar" state: GSThemeNormalState];
  if (tiles == nil)
    {
      /*
      Draw the black border towards the rest of the window. (The outer black
      border is drawn in -drawRect: since it might be drawn even if we don't have
      a title bar.
      */
      NSColor *borderColor = [self colorNamed: @"windowBorderColor"
                                        state: GSThemeNormalState];
      if (nil == borderColor)
        {
          borderColor = [NSColor blackColor];
        }
      [borderColor set];
 
      PSmoveto(0, NSMinY(titleBarRect) + 0.5);
      PSrlineto(titleBarRect.size.width, 0);
      PSstroke();

      /*
      Draw the button-like border.
      */
      workRect = titleBarRect;
      workRect.origin.x += 1;
      workRect.origin.y += 1;
      workRect.size.width -= 2;
      workRect.size.height -= 2;

      workRect = NSDrawTiledRects(workRect, workRect, edges, grays[inputState], 4);
     
      /*
      Draw the background.
      */
      switch (inputState) 
	{
	default:
	case 0:
	  [[NSColor windowFrameColor] set];
	  break;
	case 1:
	  [[NSColor lightGrayColor] set];
	  break;
	case 2:
	  [[NSColor darkGrayColor] set];
	  break;
	}
      NSRectFill(workRect);
    }
  else
    {
      [self fillRect: titleBarRect
          withTiles: tiles
         background: [NSColor windowFrameColor]];
      workRect = titleBarRect;
    }
  /* Draw the title. */
  if (styleMask & NSTitledWindowMask)
    {
      NSSize titleSize;
    
      if (styleMask & NSMiniaturizableWindowMask)
	{
	  workRect.origin.x += 17;
	  workRect.size.width -= 17;
	}
      if (styleMask & NSClosableWindowMask)
	{
	  workRect.size.width -= 17;
	}
  
      titleSize = [title sizeWithAttributes: titleTextAttributes[inputState]];
      if (titleSize.width <= workRect.size.width)
	workRect.origin.x = NSMidX(workRect) - titleSize.width / 2;
      workRect.origin.y = NSMidY(workRect) - titleSize.height / 2;
      workRect.size.height = titleSize.height;
      [title drawInRect: workRect
	 withAttributes: titleTextAttributes[inputState]];
    }
}

// FIXME: Would be good if this took the window as a param
- (void) drawResizeBarRect: (NSRect)resizeBarRect
{
  GSDrawTiles *tiles;
  tiles = [self tilesNamed: @"GSWindowResizeBar" state: GSThemeNormalState];
  if (tiles == nil)
    {
      [[NSColor lightGrayColor] set];
      PSrectfill(1.0, 1.0, resizeBarRect.size.width - 2.0, RESIZE_HEIGHT - 3.0);

      PSsetlinewidth(1.0);

      [[NSColor blackColor] set];
      PSmoveto(0.0, 0.5);
      PSlineto(resizeBarRect.size.width, 0.5);
      PSstroke();

      [[NSColor darkGrayColor] set];
      PSmoveto(1.0, RESIZE_HEIGHT - 0.5);
      PSlineto(resizeBarRect.size.width - 1.0, RESIZE_HEIGHT - 0.5);
      PSstroke();

      [[NSColor whiteColor] set];
      PSmoveto(1.0, RESIZE_HEIGHT - 1.5);
      PSlineto(resizeBarRect.size.width - 1.0, RESIZE_HEIGHT - 1.5);
      PSstroke();


      /* Only draw the notches if there's enough space. */
      if (resizeBarRect.size.width < 30 * 2)
	return;

      [[NSColor darkGrayColor] set];
      PSmoveto(27.5, 1.0);
      PSlineto(27.5, RESIZE_HEIGHT - 2.0);
      PSmoveto(resizeBarRect.size.width - 28.5, 1.0);
      PSlineto(resizeBarRect.size.width - 28.5, RESIZE_HEIGHT - 2.0);
      PSstroke();

      [[NSColor whiteColor] set];
      PSmoveto(28.5, 1.0);
      PSlineto(28.5, RESIZE_HEIGHT - 2.0);
      PSmoveto(resizeBarRect.size.width - 27.5, 1.0);
      PSlineto(resizeBarRect.size.width - 27.5, RESIZE_HEIGHT - 2.0);
      PSstroke();
    }
  else
    {
      [self fillRect: resizeBarRect
           withTiles: tiles];
    }
}

- (void) drawWindowBorder: (NSRect)rect 
                withFrame: (NSRect)frame 
             forStyleMask: (unsigned int)styleMask
                    state: (int)inputState 
                 andTitle: (NSString*)title
{
  if (styleMask & (NSTitledWindowMask | NSClosableWindowMask 
                   | NSMiniaturizableWindowMask))
    {
      NSRect titleBarRect;

      titleBarRect = NSMakeRect(0.0, frame.size.height - TITLE_HEIGHT,
                                frame.size.width, TITLE_HEIGHT);
      if (NSIntersectsRect(rect, titleBarRect))
        [self drawTitleBarRect: titleBarRect 
              forStyleMask: styleMask
              state: inputState 
              andTitle: title];
    }

  if (styleMask & NSResizableWindowMask)
    {
      NSRect resizeBarRect;

      resizeBarRect = NSMakeRect(0.0, 0.0, frame.size.width, RESIZE_HEIGHT);
      if (NSIntersectsRect(rect, resizeBarRect))
        [self drawResizeBarRect: resizeBarRect];
    }

  if (styleMask & (NSTitledWindowMask | NSClosableWindowMask 
                   | NSMiniaturizableWindowMask | NSResizableWindowMask))
    {
      NSColor *borderColor = [self colorNamed: @"windowBorderColor"
                                        state: GSThemeNormalState];
      if (nil == borderColor)
        {
          borderColor = [NSColor blackColor];
        }
      [borderColor set];
      PSsetlinewidth(1.0);
      if (NSMinX(rect) < 1.0)
	{
	  PSmoveto(0.5, 0.0);
	  PSlineto(0.5, frame.size.height);
	  PSstroke();
	}
      if (NSMaxX(rect) > frame.size.width - 1.0)
	{
	  PSmoveto(frame.size.width - 0.5, 0.0);
	  PSlineto(frame.size.width - 0.5, frame.size.height);
	  PSstroke();
	}
      if (NSMaxY(rect) > frame.size.height - 1.0)
	{
	  PSmoveto(0.0, frame.size.height - 0.5);
	  PSlineto(frame.size.width, frame.size.height - 0.5);
	  PSstroke();
	}
      if (NSMinY(rect) < 1.0)
	{
	  PSmoveto(0.0, 0.5);
	  PSlineto(frame.size.width, 0.5);
	  PSstroke();
	}
    }
}

- (NSColor *) browserHeaderTextColor
{
  NSColor *color;

  color = [self colorNamed: @"browserHeaderTextColor"
		     state: GSThemeNormalState];
  if (color == nil)
    {
      color = [NSColor windowFrameTextColor];
    }
  return color;
}

- (void) drawBrowserHeaderCell: (NSTableHeaderCell*)cell
	 	     withFrame: (NSRect)rect
			inView: (NSView*)view;
{
  GSDrawTiles *tiles;
  tiles = [self tilesNamed: GSBrowserHeader state: GSThemeNormalState];
  if (tiles == nil)
   {
     [self drawGrayBezel: rect withClip: NSZeroRect];
     [cell _drawBackgroundWithFrame: rect inView: view];
   }
  else
    {
      [self fillRect: rect
           withTiles: tiles];
    }
}

- (NSRect) browserHeaderDrawingRectForCell: (NSTableHeaderCell*)cell
				 withFrame: (NSRect)rect
{
  GSDrawTiles *tiles;
  tiles = [self tilesNamed: GSBrowserHeader state: GSThemeNormalState];
  if (tiles == nil)
    {
      return NSInsetRect(rect, 2, 2);
    }
  else
    {
      const BOOL flipped = [[cell controlView] isFlipped];
      NSRect result = [tiles contentRectForRect: rect
				      isFlipped: flipped];
      return result;
    }
}

typedef enum {
  GSTabSelectedLeft,
  GSTabSelectedRight,
  GSTabSelectedToUnSelectedJunction,
  GSTabSelectedFill,
  GSTabUnSelectedLeft,
  GSTabUnSelectedRight,
  GSTabUnSelectedToSelectedJunction,
  GSTabUnSelectedJunction,
  GSTabUnSelectedFill,
  GSTabBackgroundFill
} GSTabPart;

- (NSImage *)imageForTabPart: (GSTabPart)part type: (NSTabViewType)type
{
  NSMutableString *imageName = [NSMutableString stringWithCapacity: 32];
  NSString *typeString = nil;
  NSString *partString = nil;

  switch (type)
    {
    case NSTopTabsBezelBorder:
      typeString = @"";
      break;
    case NSBottomTabsBezelBorder:
      typeString = @"Down";
      break;
    case NSLeftTabsBezelBorder:
      typeString = @"Left";
      break;
    case NSRightTabsBezelBorder:
      typeString =  @"Right";
      break;
    default:
      return nil;
    }

  switch (part)
    {
    case GSTabSelectedLeft: 
      partString = @"SelectedLeft";
      break;
    case GSTabSelectedRight:
      partString = @"SelectedRight";
      break;
    case GSTabSelectedToUnSelectedJunction:
      partString = @"SelectedToUnSelectedJunction";
      break;
    case GSTabSelectedFill:
      return nil;
    case GSTabUnSelectedLeft:
      partString = @"UnSelectedLeft";
      break;
    case GSTabUnSelectedRight:
      partString = @"UnSelectedRight";
      break;
    case GSTabUnSelectedToSelectedJunction:
      partString = @"UnSelectedToSelectedJunction";
      break;
    case GSTabUnSelectedJunction:
      partString = @"UnSelectedJunction";
      break;
    case GSTabUnSelectedFill:
    case GSTabBackgroundFill:
      return nil;
    }

  [imageName appendString: @"common_Tab"];
  [imageName appendString: typeString];
  [imageName appendString: partString];

  return [NSImage imageNamed: imageName];
}

- (GSDrawTiles *)tilesForTabPart: (GSTabPart)part type: (NSTabViewType)type
{
  NSString *name = nil;

  if (type == NSTopTabsBezelBorder)
    {
      if (part == GSTabSelectedFill)
	name = GSTabViewSelectedTabFill;
      else if (part == GSTabUnSelectedFill)
	name = GSTabViewUnSelectedTabFill;
      else if (part == GSTabBackgroundFill)
	name = GSTabViewBackgroundTabFill;
    }
  else if (type == NSBottomTabsBezelBorder)
    {
      if (part == GSTabSelectedFill)
	name = GSTabViewBottomSelectedTabFill;
      else if (part == GSTabUnSelectedFill)
	name = GSTabViewBottomUnSelectedTabFill;
      else if (part == GSTabBackgroundFill)
	name = GSTabViewBottomBackgroundTabFill;
    }
  else if (type == NSLeftTabsBezelBorder)
    {
      if (part == GSTabSelectedFill)
	name = GSTabViewLeftSelectedTabFill;
      else if (part == GSTabUnSelectedFill)
	name = GSTabViewLeftUnSelectedTabFill;
      else if (part == GSTabBackgroundFill)
	name = GSTabViewLeftBackgroundTabFill;
    }
  else if (type == NSRightTabsBezelBorder)
    {
      if (part == GSTabSelectedFill)
	name = GSTabViewRightSelectedTabFill;
      else if (part == GSTabUnSelectedFill)
	name = GSTabViewRightUnSelectedTabFill;
      else if (part == GSTabBackgroundFill)
	name = GSTabViewRightBackgroundTabFill;
    }
  
  return [self tilesNamed: name state: GSThemeNormalState];
}

- (void) frameTabRectTopAndBottom: (NSRect)aRect 
			 topColor: (NSColor *)topColor
		      bottomColor: (NSColor *)bottomColor
{
  NSRect bottom = aRect;
  NSRect top = aRect;

  top.size.height = 1;
  bottom.origin.y = NSMaxY(aRect) - 1;
  bottom.size.height = 1;

  [topColor set];
  NSRectFill(top);

  [bottomColor set];
  NSRectFill(bottom);
}

- (void) drawTabFillInRect: (NSRect)aRect forPart: (GSTabPart)part type: (NSTabViewType)type
{
  GSDrawTiles *tiles = [self tilesForTabPart: part type: type];

  if (tiles == nil)
    {
      if (type == NSBottomTabsBezelBorder)
	{
	  switch (part)
	    {
	    case GSTabSelectedFill:
	      [self frameTabRectTopAndBottom: aRect
				    topColor: [NSColor clearColor]
				 bottomColor: [NSColor whiteColor]];
	      break;
	    case GSTabUnSelectedFill:
	      [self frameTabRectTopAndBottom: aRect 
				    topColor: [NSColor darkGrayColor] 
				 bottomColor: [NSColor whiteColor]];
	      break;
	    case GSTabBackgroundFill:
	      {
		const NSRect clip = aRect;
		aRect.origin.x -= 2;
		aRect.origin.y = NSMinY(aRect) - 2;
		aRect.size.width += 2;
		aRect.size.height = 4;
		[self drawButton: aRect withClip: clip];
		break;
	      }
	    default:
	      break;
	    }
	}
      else if (type == NSTopTabsBezelBorder)
	{
	  switch (part)
	    {
	    case GSTabSelectedFill:
	      [self frameTabRectTopAndBottom: aRect
				    topColor: [NSColor whiteColor]
				 bottomColor: [NSColor clearColor]];
	      break;
	    case GSTabUnSelectedFill:
	      [self frameTabRectTopAndBottom: aRect
				    topColor: [NSColor whiteColor]
				 bottomColor: [NSColor whiteColor]];
	      break;
	    case GSTabBackgroundFill:
	      {
		const NSRect clip = aRect;
		aRect.origin.x -= 2;
		aRect.origin.y = NSMaxY(aRect) - 1;
		aRect.size.width += 2;
		aRect.size.height = 4;
		[self drawButton: aRect withClip: clip];
		break;
	      }
	    default:
	      break;
	    }
	}
    }
  else
    {
      [self fillRect: aRect
           withTiles: tiles];
    }
}

- (CGFloat) tabHeightForType: (NSTabViewType)type
{
  NSImage *img = [self imageForTabPart: GSTabUnSelectedLeft type: type];
  if (img == nil)
    {
      return 0;
    }
  return [img size].height;
}

- (NSRect) tabViewBackgroundRectForBounds: (NSRect)aRect
			      tabViewType: (NSTabViewType)type
{
  const CGFloat tabHeight = [self tabHeightForType: type];

  switch (type)
    {
      default:
      case NSTopTabsBezelBorder: 
        aRect.size.height -= tabHeight;
        aRect.origin.y += tabHeight;
        break;

      case NSBottomTabsBezelBorder: 
        aRect.size.height -= tabHeight;
        break;

      case NSLeftTabsBezelBorder: 
        aRect.size.width -= tabHeight;
        aRect.origin.x += tabHeight;
        break;

      case NSRightTabsBezelBorder: 
        aRect.size.width -= tabHeight;
        break;

      case NSNoTabsBezelBorder: 
      case NSNoTabsLineBorder: 
      case NSNoTabsNoBorder: 
        break;
    }

  return aRect;
}


- (NSRect) tabViewContentRectForBounds: (NSRect)aRect
			   tabViewType: (NSTabViewType)type
			       tabView: (NSTabView *)view
{
  NSRect cRect = [self tabViewBackgroundRectForBounds: aRect
						   tabViewType: type];
  NSString *name = GSStringFromTabViewType(type);
  GSDrawTiles *tiles = [self tilesNamed: name state: GSThemeNormalState];

  if (tiles == nil)
    {
      switch (type)
	{
	case NSBottomTabsBezelBorder:
	  cRect.origin.x += 1;
	  cRect.origin.y += 1;
	  cRect.size.width -= 3;
	  cRect.size.height -= 2;
	  break;
	case NSNoTabsBezelBorder:
	  cRect.origin.x += 1;
	  cRect.origin.y += 1;
	  cRect.size.width -= 3;
	  cRect.size.height -= 2;
	  break;
	case NSNoTabsLineBorder:
	  cRect.origin.y += 1; 
	  cRect.origin.x += 1; 
	  cRect.size.width -= 2;
	  cRect.size.height -= 2;
	  break;
	case NSTopTabsBezelBorder:
	  cRect.origin.x += 1;
	  cRect.origin.y += 1;
	  cRect.size.width -= 3;
	  cRect.size.height -= 2;
	  break;
	case NSLeftTabsBezelBorder:
	  cRect.origin.x += 1;
	  cRect.origin.y += 1;
	  cRect.size.width -= 3;
	  cRect.size.height -= 2;
	  break;
	case NSRightTabsBezelBorder:
	  cRect.origin.x += 1;
	  cRect.origin.y += 1;
	  cRect.size.width -= 3;
	  cRect.size.height -= 2;
	  break;
	case NSNoTabsNoBorder:
	default:
	  break;
	}     
    }
  else
    {
      cRect = [tiles contentRectForRect: cRect
			      isFlipped: [view isFlipped]];
    }
  return cRect;
}


- (void) drawTabViewBezelRect: (NSRect)aRect
                  tabViewType: (NSTabViewType)type
                       inView: (NSView *)view
{
  NSString *name = GSStringFromTabViewType(type);
  GSDrawTiles *tiles = [self tilesNamed: name state: GSThemeNormalState];

  if (tiles == nil)
    {
      switch (type)
	{
	default:
	case NSTopTabsBezelBorder:
	  {
	    const NSRect clip = aRect;
	    aRect.size.height += 1;
	    aRect.origin.y -= 1;
	    [self drawButton: aRect withClip: clip];
	    break;
	  }
	case NSBottomTabsBezelBorder:
	  {
	    const NSRect clip = aRect;
	    aRect.size.height += 2;
	    [self drawButton: aRect withClip: clip];
	    break;
	  }
	case NSLeftTabsBezelBorder:
	case NSRightTabsBezelBorder:
	  [self drawButton: aRect withClip: NSZeroRect];
	  break;
	case NSNoTabsBezelBorder:
       	  break;	
	case NSNoTabsLineBorder:
	  [[NSColor controlDarkShadowColor] set];
	  NSFrameRect(aRect);
	  break;	  
	case NSNoTabsNoBorder:
	  break;
	}
    }
  else
    {
      [self fillRect: aRect
           withTiles: tiles];
    }
}

- (void) drawTabViewRect: (NSRect)rect
		  inView: (NSView *)view
	       withItems: (NSArray *)items
	    selectedItem: (NSTabViewItem *)selected
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  const NSUInteger howMany = [items count];
  int i;
  int previousState = 0;
  const NSTabViewType type = [(NSTabView *)view tabViewType];
  const NSRect bounds = [view bounds];
  NSRect aRect = [self tabViewBackgroundRectForBounds: bounds tabViewType: type];

  const BOOL truncate = [(NSTabView *)view allowsTruncatedLabels];
  const CGFloat tabHeight = [self tabHeightForType: type];
  
  DPSgsave(ctxt);
  
  [self drawTabViewBezelRect: aRect
 		 tabViewType: type
 		      inView: view];

  if (type == NSBottomTabsBezelBorder
      || type == NSTopTabsBezelBorder)
    {
      NSPoint iP;
      if (type == NSTopTabsBezelBorder)
	iP = bounds.origin;
      else
	iP = NSMakePoint(aRect.origin.x, NSMaxY(aRect));

      for (i = 0; i < howMany; i++) 
        {
          NSRect r;
          NSTabViewItem *anItem = [items objectAtIndex: i];
          const NSTabState itemState = [anItem tabState];
          const NSSize s = [anItem sizeOfLabel: truncate];

	  // Draw the left image

          if (i == 0)
            {	    
	      NSImage *part = nil;
	      if (itemState == NSSelectedTab)
                {
		  part = [self imageForTabPart: GSTabSelectedLeft type: type];
                }
              else if (itemState == NSBackgroundTab)
                {
		  part = [self imageForTabPart: GSTabUnSelectedLeft type: type];
                }
              else
                NSLog(@"Not finished yet. Luff ya.\n");

	      [part drawInRect: NSMakeRect(iP.x, iP.y, [part size].width, [part size].height)
		      fromRect: NSZeroRect
		     operation: NSCompositeSourceOver
		      fraction: 1.0
		respectFlipped: YES
			 hints: nil];

	      iP.x += [part size].width;
            }
          else
            {
	      NSImage *part = nil;
              if (itemState == NSSelectedTab)
                {
		  part = [self imageForTabPart: GSTabUnSelectedToSelectedJunction type: type];
                }
              else if (itemState == NSBackgroundTab)
                {
                  if (previousState == NSSelectedTab)
                    {
		      part = [self imageForTabPart: GSTabSelectedToUnSelectedJunction type: type];
                    }
                  else
                    {
		      part = [self imageForTabPart: GSTabUnSelectedJunction type: type];
                    }
                }
              else
                NSLog(@"Not finished yet. Luff ya.\n");

	      [part drawInRect: NSMakeRect(iP.x, iP.y, [part size].width, [part size].height)
		      fromRect: NSZeroRect
		     operation: NSCompositeSourceOver
		      fraction: 1.0
		respectFlipped: YES
			 hints: nil];

	      iP.x += [part size].width;
            }

	  // Draw the middle fill part of the tab

          r.origin = iP;
          r.size.width = s.width;
          r.size.height = tabHeight;

	  if (itemState == NSSelectedTab)
	    {
	      [self drawTabFillInRect: r forPart: GSTabSelectedFill type: type];
	    }
	  else if (itemState == NSBackgroundTab)
	    {
	      [self drawTabFillInRect: r forPart: GSTabUnSelectedFill type: type];
	    }
	  else
	    NSLog(@"Not finished yet. Luff ya.\n");

          // Label
          [anItem drawLabel: truncate inRect: r];
          
          iP.x += s.width;
          previousState = itemState;

	  // For the rightmost tab, draw the right side

          if (i == howMany - 1)
            {
	      NSImage *part = nil;
              if ([anItem tabState] == NSSelectedTab)
                {              
		  part = [self imageForTabPart: GSTabSelectedRight type: type];
                }  
              else if ([anItem tabState] == NSBackgroundTab)
                {
		  part = [self imageForTabPart: GSTabUnSelectedRight type: type];
                }
              else
                NSLog(@"Not finished yet. Luff ya.\n");

	      [part drawInRect: NSMakeRect(iP.x, iP.y, [part size].width, [part size].height)
		      fromRect: NSZeroRect
		     operation: NSCompositeSourceOver
		      fraction: 1.0
		respectFlipped: YES
			 hints: nil];

	      iP.x += [part size].width;

	      // Draw the background fill
	      if (iP.x < NSMaxX(bounds))
		{
		  r.origin = iP;
		  r.size.width = NSMaxX(bounds) - iP.x;
		  r.size.height = tabHeight;

		  [self drawTabFillInRect: r forPart: GSTabBackgroundFill type: type];
		}
            }
        }
    }
  // FIXME: Missing drawing code for other cases

  DPSgrestore(ctxt);
}

- (void) drawScrollerRect: (NSRect)rect
		   inView: (NSView *)view
		  hitPart: (NSScrollerPart)hitPart
	     isHorizontal: (BOOL)isHorizontal
{
  NSRect rectForPartIncrementLine;
  NSRect rectForPartDecrementLine;
  NSRect rectForPartKnobSlot;
  NSScroller *scroller = (NSScroller *)view;

  rectForPartIncrementLine = [scroller rectForPart: NSScrollerIncrementLine];
  rectForPartDecrementLine = [scroller rectForPart: NSScrollerDecrementLine];
  rectForPartKnobSlot = [scroller rectForPart: NSScrollerKnobSlot];

  /*
  [[[view window] backgroundColor] set];
  NSRectFill (rect);
  */

  if (NSIntersectsRect (rect, rectForPartKnobSlot) == YES)
    {
      [scroller drawKnobSlot];
      [scroller drawKnob];
    }

  if (NSIntersectsRect (rect, rectForPartDecrementLine) == YES)
    {
      [scroller drawArrow: NSScrollerDecrementArrow 
		highlight: hitPart == NSScrollerDecrementLine];
    }
  if (NSIntersectsRect (rect, rectForPartIncrementLine) == YES)
    {
      [scroller drawArrow: NSScrollerIncrementArrow 
		highlight: hitPart == NSScrollerIncrementLine];
    }
}

- (void) drawBrowserRect: (NSRect)rect
		  inView: (NSView *)view
	withScrollerRect: (NSRect)scrollerRect
	      columnSize: (NSSize)columnSize
{
  NSBrowser *browser = (NSBrowser *)view;
  NSRect bounds = [view bounds];

  // Load the first column if not already done
  if (![browser isLoaded])
    {
      [browser loadColumnZero];
    }

  // Draws titles
  if ([browser isTitled])
    {
      int i;

      for (i = [browser firstVisibleColumn]; 
	   i <= [browser lastVisibleColumn]; 
	   ++i)
        {
          NSRect titleRect = [browser titleFrameOfColumn: i];
          if (NSIntersectsRect (titleRect, rect) == YES)
            {
              [browser drawTitleOfColumn: i
                    inRect: titleRect];
            }
        }
    }

  // Draws scroller border

  if ([self browserUseBezels])
    {
      if ([browser hasHorizontalScroller] && 
	  [browser separatesColumns])
	{
	  NSRect scrollerBorderRect = scrollerRect;
	  NSSize bs = [self sizeForBorderType: NSBezelBorder];
	  
	  scrollerBorderRect.origin.x = 0;
	  scrollerBorderRect.origin.y = 0;
	  scrollerBorderRect.size.width += 2 * bs.width;
	  scrollerBorderRect.size.height += (2 * bs.height) - 1;
	  
	  if ((NSIntersectsRect (scrollerBorderRect, rect) == YES) && [view window])
	    {
	      [self drawGrayBezel: scrollerBorderRect withClip: rect];
	    }
	}
      
      if (![browser separatesColumns])
	{
	  NSPoint p1,p2;
	  int     i, visibleColumns;
	  float   hScrollerWidth = [browser hasHorizontalScroller] ? 
	    [NSScroller scrollerWidth] : 0;
	  
	  // Columns borders
	  [self drawGrayBezel: bounds withClip: rect];
	  
	  [[NSColor blackColor] set];
	  visibleColumns = [browser numberOfVisibleColumns]; 
	  for (i = 1; i < visibleColumns; i++)
	    {
	      p1 = NSMakePoint((columnSize.width * i) + 2 + (i-1), 
                           columnSize.height + hScrollerWidth + 2);
	      p2 = NSMakePoint((columnSize.width * i) + 2 + (i-1),
			       hScrollerWidth + 2);
	      [NSBezierPath strokeLineFromPoint: p1 toPoint: p2];
	    }
	  
	  // Horizontal scroller border
	  if ([browser hasHorizontalScroller])
	    {
	      p1 = NSMakePoint(2, hScrollerWidth + 2);
	      p2 = NSMakePoint(rect.size.width - 2, hScrollerWidth + 2);
	      [NSBezierPath strokeLineFromPoint: p1 toPoint: p2];
	    }
	}
    }

   if (![self browserUseBezels]
       && [self scrollViewScrollersOverlapBorders])
    {
      NSRect baseRect = NSMakeRect(0, 0, bounds.size.width, 1); 
      NSRect colFrame = [browser frameOfColumn: [browser firstVisibleColumn]];
      NSRect scrollViewRect = NSUnionRect(baseRect, colFrame);

      GSDrawTiles *tiles = [self tilesNamed: @"NSScrollView"
				      state: GSThemeNormalState];

      [self fillRect: scrollViewRect
           withTiles: tiles];
    }
}

- (CGFloat) browserColumnSeparation
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  
  if ([defs objectForKey: @"GSBrowserColumnSeparation"] != nil)
    {
      return [defs floatForKey: @"GSBrowserColumnSeparation"];
    }
  else
    {
      return 4;
    }
}

- (CGFloat) browserVerticalPadding
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  
  if ([defs objectForKey: @"GSBrowserVerticalPadding"] != nil)
    {
      return [defs floatForKey: @"GSBrowserVerticalPadding"];
    }
  else
    {
      return 2;
    }
}

- (BOOL) browserUseBezels
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  
  if ([defs objectForKey: @"GSBrowserUseBezels"] != nil)
    {
      return [defs boolForKey: @"GSBrowserUseBezels"];
    }
  else
    {
      return YES;
    }
}

- (void) drawMenuRect: (NSRect)rect
	       inView: (NSView *)view
	 isHorizontal: (BOOL)horizontal
	    itemCells: (NSArray *)itemCells
{
  int         i = 0;
  int         howMany = [itemCells count];
  NSMenuView *menuView = (NSMenuView *)view;
  NSRect      bounds = [view bounds];

  [self drawBackgroundForMenuView: menuView
	withFrame: bounds
	dirtyRect: rect
	horizontal: horizontal];
  
  // Draw the menu cells.
  for (i = 0; i < howMany; i++)
    {
      NSRect aRect;
      NSMenuItemCell *aCell;
      
      aRect = [menuView rectOfItemAtIndex: i];
      if (NSIntersectsRect(rect, aRect) == YES)
        {
          aCell = [menuView menuItemCellForItemAtIndex: i];
          [aCell drawWithFrame: aRect inView: menuView];
        }
    }
}

- (void) drawScrollViewRect: (NSRect)rect
		     inView: (NSView *)view 
{
  NSScrollView  *scrollView = (NSScrollView *)view;
  GSTheme	*theme = [GSTheme theme];
  NSColor	*color;
  NSString	*name;
  NSBorderType   borderType = [scrollView borderType];
  NSRect         bounds = [view bounds];
  BOOL hasInnerBorder = ![[NSUserDefaults standardUserDefaults]
			   boolForKey: @"GSScrollViewNoInnerBorder"];
  GSDrawTiles *tiles = nil;

  name = [theme nameForElement: self];
  if (name == nil)
    {
      name = @"NSScrollView";
    }
  color = [theme colorNamed: name state: GSThemeNormalState];
  tiles = [theme tilesNamed: name state: GSThemeNormalState];
  if (color == nil)
    {
      color = [NSColor controlDarkShadowColor];
    }

  if (tiles == nil)
    {
      switch (borderType)
	{
	case NSNoBorder:
	  break;
	  
	case NSLineBorder:
	  [color set];
	  NSFrameRect(bounds);
	  break;
	  
	case NSBezelBorder:
	  [theme drawGrayBezel: bounds withClip: rect];
	  break;
	  
	case NSGrooveBorder:
	  [theme drawGroove: bounds withClip: rect];
	  break;
	}
    }
  else
    {
      [self fillRect: bounds
	   withTiles: tiles];
    }

  if (hasInnerBorder)
    {
      NSScroller *vertScroller = [scrollView verticalScroller];
      NSScroller *horizScroller = [scrollView horizontalScroller];
      CGFloat scrollerWidth = [NSScroller scrollerWidth];
      NSRect scrollerFrame;

      [color set];

      if ([scrollView hasVerticalScroller])
	{
	  NSInterfaceStyle style;
	  CGFloat xpos;
          
          scrollerFrame = [vertScroller frame];
          
	  style = NSInterfaceStyleForKey(@"NSScrollViewInterfaceStyle", nil);
	  if (style == NSMacintoshInterfaceStyle
	      || style == NSWindows95InterfaceStyle)
	    {
              xpos = scrollerFrame.origin.x - 1.0;
	    }
	  else
	    {
              xpos = scrollerFrame.origin.x + scrollerWidth;
	    }
          NSRectFill(NSMakeRect(xpos, scrollerFrame.origin.y, 
                                1.0, scrollerFrame.size.height));
	}

      if ([scrollView hasHorizontalScroller])
	{
	  CGFloat ypos;

          scrollerFrame = [horizScroller frame];
 
	  if ([scrollView isFlipped])
	    {
	      ypos = scrollerFrame.origin.y - 1.0;
	    }
	  else
	    {
	      ypos = scrollerFrame.origin.y + scrollerWidth + 1.0;
	    }
          NSRectFill(NSMakeRect([horizScroller frame].origin.x, ypos,
                                scrollerFrame.size.width, 1.0));
	}
    }
}

- (void) drawSliderBorderAndBackground: (NSBorderType)aType
				 frame: (NSRect)cellFrame
				inCell: (NSCell *)cell
			  isHorizontal: (BOOL)horizontal
{
  NSSliderType type = [(NSSliderCell *)cell sliderType];
  if (type == NSLinearSlider)
    {
      NSString *partName = (horizontal ? GSSliderHorizontalTrack : GSSliderVerticalTrack); 
      GSDrawTiles *tiles = [self tilesNamed: partName
				      state: GSThemeNormalState];

      if (tiles == nil)
	{
	  [[GSTheme theme] drawBorderType: aType 
				    frame: cellFrame 
				     view: [cell controlView]];
	}
      else
	{
	  // FIXME: This code could be factored out
	  NSSize tilesNaturalSize = [tiles size];
	  NSRect tilesRect;

	  // Only stretch the tiles in one direction
	  if (horizontal)
	    {
	      tilesRect.size = NSMakeSize(cellFrame.size.width, tilesNaturalSize.height);
	    }
	  else
	    {
	      tilesRect.size = NSMakeSize(tilesNaturalSize.width, cellFrame.size.height);
	    }
	  tilesRect.origin = NSMakePoint((cellFrame.size.width - tilesRect.size.width) / 2.0,
					 (cellFrame.size.height - tilesRect.size.height) / 2.0);
	  
	  if ([cell controlView] != nil)
	    {
	      tilesRect = [[cell controlView] centerScanRect: tilesRect];
	    }

	  [self fillRect: tilesRect
	       withTiles: tiles];
	}
    }
}

- (void) drawBarInside: (NSRect)rect
		inCell: (NSCell *)cell
	       flipped: (BOOL)flipped
{
  NSSliderType type = [(NSSliderCell *)cell sliderType];
  if (type == NSLinearSlider)
    {
      BOOL horizontal = (rect.size.width > rect.size.height);
      NSString *partName = (horizontal ? GSSliderHorizontalTrack : GSSliderVerticalTrack); 
      GSDrawTiles *tiles = [self tilesNamed: partName
				      state: GSThemeNormalState];

      if (tiles == nil)
	{
	  [[NSColor scrollBarColor] set];
	  NSRectFill(rect);
	}
      // Don't draw anything if we have tiles, they are drawn
      // in -drawSliderBorderAndBackground:...
    }
}

- (void) drawKnobInCell: (NSCell *)cell
{
  NSView *controlView = [cell controlView];
  NSSliderCell *sliderCell = (NSSliderCell *)cell;

  [sliderCell drawKnob: 
		[sliderCell knobRectFlipped: 
			      [controlView isFlipped]]];  
}

- (NSRect) tableHeaderCellDrawingRectForBounds: (NSRect)theRect
{
  NSSize borderSize;

  // This adjustment must match the drawn border
  borderSize = NSMakeSize(1, 1);

  return NSInsetRect(theRect, borderSize.width, borderSize.height);
}

- (void)drawTableHeaderRect: (NSRect)aRect
		     inView: (NSView *)view
{
  NSTableHeaderView *tableHeaderView = (NSTableHeaderView *)view;
  NSTableView *tableView = [tableHeaderView tableView];
  NSArray *columns;
  int firstColumnToDraw;
  int lastColumnToDraw;
  NSRect drawingRect;
  NSTableColumn *column;
  NSTableColumn *highlightedTableColumn;
  float width;
  int i;
  NSCell *cell;

  if (tableView == nil)
    return;

  firstColumnToDraw = [tableHeaderView columnAtPoint: NSMakePoint (aRect.origin.x,
                                                        aRect.origin.y)];
  if (firstColumnToDraw == -1)
    firstColumnToDraw = 0;

  lastColumnToDraw = [tableHeaderView columnAtPoint: NSMakePoint (NSMaxX (aRect),
                                                       aRect.origin.y)];
  if (lastColumnToDraw == -1)
    lastColumnToDraw = [tableView numberOfColumns] - 1;

  drawingRect = [tableHeaderView headerRectOfColumn: firstColumnToDraw];
  
  columns = [tableView tableColumns];
  highlightedTableColumn = [tableView highlightedTableColumn];
  
  for (i = firstColumnToDraw; i <= lastColumnToDraw; i++)
    {
      column = [columns objectAtIndex: i];
      width = [column width];
      drawingRect.size.width = width;
      cell = [column headerCell];
      if ((column == highlightedTableColumn)
          || [tableView isColumnSelected: i])
        {
          [cell setHighlighted: YES];
        }
      else
        {
          [cell setHighlighted: NO];
        }
      [cell drawWithFrame: drawingRect
                           inView: tableHeaderView];
      drawingRect.origin.x += width;
    }
}

- (void) drawPopUpButtonCellInteriorWithFrame: (NSRect)cellFrame
				     withCell: (NSCell *)cell
				       inView: (NSView *)controlView
{
  // Default implementation of this method does nothing.
}

- (void) drawTableViewBackgroundInClipRect: (NSRect)aRect
				    inView: (NSView *)view
		       withBackgroundColor: (NSColor *)backgroundColor
{
  NSTableView *tableView = (NSTableView *)view;

  [backgroundColor set];
  NSRectFill (aRect);
  
  if ([tableView usesAlternatingRowBackgroundColors])
    {
      const CGFloat rowHeight = [tableView rowHeight];
      NSInteger startingRow = [tableView rowAtPoint: NSMakePoint(0, NSMinY(aRect))];
      NSInteger endingRow;
      NSInteger i;
      
      NSArray *rowColors = [NSColor controlAlternatingRowBackgroundColors];
      const NSUInteger rowColorCount = [rowColors count];
      
      NSRect rowRect;
      
      if (rowHeight <= 0
	  || rowColorCount == 0
	  || aRect.size.height <= 0)
	return;

      if (startingRow <= 0)
	startingRow = 0;
      
      rowRect = [tableView rectOfRow: startingRow];
      rowRect.origin.x = aRect.origin.x;
      rowRect.size.width = aRect.size.width;
      
      endingRow = startingRow + ceil(aRect.size.height / rowHeight);
      
      for (i = startingRow; i <= endingRow; i++)
	{
	  NSColor *color = [rowColors objectAtIndex: (i % rowColorCount)];
	  
	  [color set];
	  NSRectFill(rowRect);
	  
	  rowRect.origin.y += rowHeight;
	}
    }
}

- (void) drawTableViewGridInClipRect: (NSRect)aRect
			      inView: (NSView *)view
{
  NSTableView *tableView = (NSTableView *)view;

  // Cache some constants
  const CGFloat minX = NSMinX(aRect);
  const CGFloat maxX = NSMaxX(aRect);
  const CGFloat minY = NSMinY(aRect);
  const CGFloat maxY = NSMaxY(aRect);
  const NSInteger numberOfColumns = [tableView numberOfColumns];
  const NSInteger numberOfRows = [tableView numberOfRows];

  NSInteger startingRow = [tableView rowAtPoint: NSMakePoint(minX, minY)];
  NSInteger endingRow = [tableView rowAtPoint: NSMakePoint(minX, maxY)];
  NSInteger startingColumn = [tableView columnAtPoint: NSMakePoint(minX, minY)];
  NSInteger endingColumn = [tableView columnAtPoint: NSMakePoint(maxX, minY)];

  NSGraphicsContext *ctxt = GSCurrentContext();
  NSColor *gridColor = [tableView gridColor];


  if (startingRow == -1)
    startingRow = 0;
  if (endingRow == -1)
    endingRow = numberOfRows - 1;
  
  if (startingColumn == -1)
    startingColumn = 0;
  if (endingColumn == -1)
    endingColumn = numberOfColumns - 1;


  DPSgsave(ctxt);
  [gridColor set];

  // Draw horizontal lines
  if (numberOfRows > 0)
    {
      NSInteger i;
      for (i = startingRow; i <= endingRow; i++)
	{
	  NSRect rowRect = [tableView rectOfRow: i];
	  rowRect.origin.y += rowRect.size.height - 1;
	  rowRect.size.height = 1;
	  NSRectFill(rowRect);
	}
    }
  
  // Draw vertical lines
  if (numberOfColumns > 0)
    {
     NSInteger i;
      for (i = startingColumn; i <= endingColumn; i++)
	{
	  NSRect colRect = [tableView rectOfColumn: i];
	  colRect.origin.x += colRect.size.width - 1;
	  colRect.size.width = 1;
	  NSRectFill(colRect);
	}
    }

  DPSgrestore (ctxt);
}

- (void) drawTableViewRect: (NSRect)aRect
		    inView: (NSView *)view
{
  NSInteger startingRow;
  NSInteger endingRow;
  NSInteger i;
  NSTableView *tableView = (NSTableView *)view;
  NSInteger numberOfRows = [tableView numberOfRows];
  NSInteger numberOfColumns = [tableView numberOfColumns];
  BOOL drawsGrid = [tableView drawsGrid];

  /* Draw background */
  [tableView drawBackgroundInClipRect: aRect];

  if ((numberOfRows == 0) || (numberOfColumns == 0))
    {
      return;
    }

  /* Draw selection */
  [tableView highlightSelectionInClipRect: aRect];

  /* Draw grid */
  if (drawsGrid)
    {
      [tableView drawGridInClipRect: aRect];
    }
  
  /* Draw visible cells */
  /* Using rowAtPoint: here calls them only twice per drawn rect */
  startingRow = [tableView rowAtPoint: NSMakePoint (0, NSMinY (aRect))];
  endingRow   = [tableView rowAtPoint: NSMakePoint (0, NSMaxY (aRect))];

  if (startingRow == -1)
    {
      startingRow = 0;
    }
  if (endingRow == -1)
    {
      endingRow = numberOfRows - 1;
    }
  //  NSLog(@"drawRect : %d-%d", startingRow, endingRow);
  {
    SEL sel = @selector(drawRow:clipRect:);
    void (*imp)(id, SEL, NSInteger, NSRect);
    
    imp = (void (*)(id, SEL, NSInteger, NSRect))[tableView methodForSelector: sel];
    
    for (i = startingRow; i <= endingRow; i++)
      {
        imp(tableView, sel, i, aRect);
      }
  }
}

- (void) highlightTableViewSelectionInClipRect: (NSRect)clipRect
					inView: (NSView *)view
			      selectingColumns: (BOOL)selectingColumns
{
  NSTableView *tableView = (NSTableView *)view;
  NSInteger numberOfRows = [tableView numberOfRows];
  NSInteger numberOfColumns = [tableView numberOfColumns];
  NSIndexSet *selectedRows = [tableView selectedRowIndexes];
  NSIndexSet *selectedColumns = [tableView selectedColumnIndexes];
  NSColor *backgroundColor = [tableView backgroundColor];

  // Set the fill color
  {
    NSColor *selectionColor;
    
    selectionColor = [self colorNamed: @"highlightedTableRowBackgroundColor"
				state: GSThemeNormalState];

    if (selectionColor == nil)
      {
	// Switch to the alternate color of the backgroundColor is white.
	if([backgroundColor isEqual: [NSColor whiteColor]])
	  {
	    selectionColor = [NSColor colorWithCalibratedRed: 0.86
						       green: 0.92
							blue: 0.99
						       alpha: 1.0];
	  }
	else
	  {
	    selectionColor = [NSColor whiteColor];
	  }
      }
    [selectionColor set];
  }

  if (selectingColumns == NO)
    {
      NSInteger selectedRowsCount;
      NSUInteger row;
      NSInteger startingRow, endingRow;

      selectedRowsCount = [selectedRows count];      
      if (selectedRowsCount == 0)
	return;
      
      /* highlight selected rows */
      startingRow = [tableView rowAtPoint: NSMakePoint(0, NSMinY(clipRect))];
      endingRow   = [tableView rowAtPoint: NSMakePoint(0, NSMaxY(clipRect))];
      
      if (startingRow == -1)
	startingRow = 0;
      if (endingRow == -1)
	endingRow = numberOfRows - 1;
      
      row = [selectedRows indexGreaterThanOrEqualToIndex: startingRow];
      while ((row != NSNotFound) && (row <= endingRow))
	{
	  NSRectFill(NSIntersectionRect([tableView rectOfRow: row], clipRect));
	  row = [selectedRows indexGreaterThanIndex: row];
	}	  
    }
  else // Selecting columns
    {
      NSUInteger selectedColumnsCount;
      NSUInteger column;
      NSInteger startingColumn, endingColumn;
      
      selectedColumnsCount = [selectedColumns count];
      
      if (selectedColumnsCount == 0)
	return;
      
      /* highlight selected columns */
      startingColumn = [tableView columnAtPoint: NSMakePoint(NSMinX(clipRect), 0)];
      endingColumn = [tableView columnAtPoint: NSMakePoint(NSMaxX(clipRect), 0)];

      if (startingColumn == -1)
	startingColumn = 0;
      if (endingColumn == -1)
	endingColumn = numberOfColumns - 1;

      column = [selectedColumns indexGreaterThanOrEqualToIndex: startingColumn];
      while ((column != NSNotFound) && (column <= endingColumn))
	{
	  NSRectFill(NSIntersectionRect([tableView rectOfColumn: column],
					clipRect));
	  column = [selectedColumns indexGreaterThanIndex: column];
	}	  
    }
}

- (void) drawTableViewRow: (NSInteger)rowIndex 
		 clipRect: (NSRect)clipRect
		   inView: (NSView *)view
{
  NSTableView *tableView = (NSTableView *)view;
  // NSInteger numberOfRows = [tableView numberOfRows];
  NSInteger numberOfColumns = [tableView numberOfColumns];
  // NSIndexSet *selectedRows = [tableView selectedRowIndexes];
  // NSColor *backgroundColor = [tableView backgroundColor];
  CGFloat *columnOrigins = [tableView _columnOrigins];
  NSInteger editedRow = [tableView editedRow];
  NSInteger editedColumn = [tableView editedColumn];
  NSArray *tableColumns = [tableView tableColumns];
  NSInteger startingColumn; 
  NSInteger endingColumn;
  NSTableColumn *tb;
  NSRect drawingRect;
  NSCell *cell;
  NSInteger i;
  CGFloat x_pos;
  const BOOL rowSelected = [[tableView selectedRowIndexes] containsIndex: rowIndex];
  NSColor *tempColor = nil;
  NSColor *selectedTextColor = [self colorNamed: @"highlightedTableRowTextColor"
					  state: GSThemeNormalState];

  /* Using columnAtPoint: here would make it called twice per row per drawn 
     rect - so we avoid it and do it natively */

  /* Determine starting column as fast as possible */
  x_pos = NSMinX (clipRect);
  i = 0;
  while ((i < numberOfColumns) && (x_pos > columnOrigins[i]))
    {
      i++;
    }
  startingColumn = (i - 1);

  if (startingColumn == -1)
    startingColumn = 0;

  /* Determine ending column as fast as possible */
  x_pos = NSMaxX (clipRect);
  // Nota Bene: we do *not* reset i
  while ((i < numberOfColumns) && (x_pos > columnOrigins[i]))
    {
      i++;
    }
  endingColumn = (i - 1);

  if (endingColumn == -1)
    endingColumn = numberOfColumns - 1;

  /* Draw the row between startingColumn and endingColumn */
  for (i = startingColumn; i <= endingColumn; i++)
    {
      const BOOL columnSelected = [tableView isColumnSelected: i];
      const BOOL cellSelected = (rowSelected || columnSelected);
      tb = [tableColumns objectAtIndex: i];
      cell = [tb dataCellForRow: rowIndex];
      [tableView _willDisplayCell: cell
		 forTableColumn: tb
		 row: rowIndex];
      if (i == editedColumn && rowIndex == editedRow)
        {
          [cell _setInEditing: YES];
        }
      else
        {
          [cell setObjectValue: [tableView _objectValueForTableColumn: tb
                                                                  row: rowIndex]];
        }
      drawingRect = [tableView frameOfCellAtColumn: i
			       row: rowIndex];

      // Set the cell text color if the theme provides a custom highlighted
      // row color.
      //
      // FIXME: This could probably be done in a cleaner way. We should
      // probably do -setHighlighted: YES, and the implementation of
      // -textColor in NSCell could use that to return a highlighted text color.
      if (cellSelected && (selectedTextColor != nil)
	  && [cell isKindOfClass: [NSTextFieldCell class]])
	{
	  tempColor = [cell textColor];
	  [(NSTextFieldCell *)cell setTextColor: selectedTextColor];
	}

      [cell drawWithFrame: drawingRect inView: tableView];

      if (cellSelected && (selectedTextColor != nil)
	  && [cell isKindOfClass: [NSTextFieldCell class]])
	{
	  // Restore the cell's text color if we changed it
	  [(NSTextFieldCell *)cell setTextColor: tempColor];
	}

      if (i == editedColumn && rowIndex == editedRow)
	[cell _setInEditing: NO];
    }
}

- (void) drawBoxInClipRect: (NSRect)clipRect
		   boxType: (NSBoxType)boxType
		borderType: (NSBorderType)borderType
		    inView: (NSBox *)box
{
  NSColor *color;
  BOOL drawTitleBackground = YES;

  if (boxType == NSBoxCustom)
    {
      if (![box isOpaque])
        {
          color = [NSColor clearColor];
        }
      else
        {
          color = [box fillColor];
        }
    }
  else
    {
      color = [[box window] backgroundColor];
    }

  // Draw separator boxes

  if (boxType == NSBoxSeparator)
    {
      color = [box borderColor];
      if (!color || [color isEqual:[NSColor clearColor]])
        {
    	  color = [NSColor controlShadowColor];
        }
      [color set];
      NSRectFill([box borderRect]);
      return;
    }

  // Draw border

  GSDrawTiles *tiles = [[GSTheme theme] tilesNamed: GSBoxBorder state: GSThemeNormalState];
  if (tiles == nil 
      || borderType == NSNoBorder
      || boxType == NSBoxOldStyle
      || boxType == NSBoxCustom)
    {
      // Fill inside
      [color set];
      NSRectFill(clipRect);

      switch (borderType)
	{
	case NSNoBorder: 
	  break;
	case NSLineBorder: 
	  if (boxType == NSBoxCustom)
	    {
	      [[box borderColor] set];
	      NSFrameRectWithWidth([box borderRect], [box borderWidth]);
	    }
	  else
	    {
	      [[NSColor controlDarkShadowColor] set];
	      NSFrameRect([box borderRect]);
	    }
	  break;
	case NSBezelBorder:
	  [[GSTheme theme] drawDarkBezel: [box borderRect] withClip: clipRect];
	  break;
	case NSGrooveBorder: 
	  [[GSTheme theme] drawGroove: [box borderRect] withClip: clipRect];
	  break;
	}
    }
  else
    {
      drawTitleBackground = NO;
      
      // If the title is on the border, clip a hole in the later 

      [NSGraphicsContext saveGraphicsState];
      
      if ((borderType != NSNoBorder)
	  && (([box titlePosition] == NSAtTop) || ([box titlePosition] == NSAtBottom)))
	{
	  const NSRect borderRect = [box borderRect];
	  const NSRect titleRect = [box titleRect];
	  NSBezierPath *path = [NSBezierPath bezierPath];
	  
	  // Left
	  if (NSMinX(titleRect) > NSMinX(borderRect))
	    {
	      NSRect left = borderRect;
	      left.size.width = NSMinX(titleRect) - NSMinX(borderRect);
	      [path appendBezierPathWithRect: left];
	    }
	  
	  // Right
	  if (NSMaxX(borderRect) > NSMaxX(titleRect))
	    {
	      NSRect right = borderRect;
	      right.size.width = NSMaxX(borderRect) - NSMaxX(titleRect);
	      right.origin.x = NSMaxX(titleRect);
	      [path appendBezierPathWithRect: right];
	    }      
	  
	  // MinY
	  if (NSMinY(titleRect) > NSMinY(borderRect))
	    {
	      NSRect minY = borderRect;
	      minY.size.height = NSMinY(titleRect) - NSMinY(borderRect);
	      [path appendBezierPathWithRect: minY];
	    }
	  
	  // MaxY
	  if (NSMaxY(borderRect) > NSMaxY(titleRect))
	    {
	      NSRect maxY = borderRect;
	      maxY.size.height = NSMaxY(borderRect) - NSMaxY(titleRect);
	      maxY.origin.y = NSMaxY(titleRect);
	      [path appendBezierPathWithRect: maxY];
	    }      
	  
	  if (![path isEmpty])
	    {
	      [path addClip];
	    }
	}

      
      [[GSTheme theme] fillRect: [box borderRect]
		      withTiles: tiles];
      
      // Restore clipping path
      [NSGraphicsContext restoreGraphicsState];
    }


  // Draw title
  if ([box titlePosition] != NSNoTitle)
    {
      // If the title is on the border, clip a hole in the later 
      if (drawTitleBackground
	  && (borderType != NSNoBorder)
	  && (([box titlePosition] == NSAtTop) || ([box titlePosition] == NSAtBottom)))
	{	  
	  [color set];
	  NSRectFill([box titleRect]);
	}

      [[box titleCell] drawWithFrame: [box titleRect] inView: box];
    }
}

- (void) drawEditorForCell: (NSCell *)cell
		 withFrame: (NSRect)cellFrame
		    inView: (NSView *)view
{
  [cell _drawEditorWithFrame: cellFrame 
		      inView: view];
}


- (void) drawInCell: (NSCell *)cell
     attributedText: (NSAttributedString *)stringValue
	    inFrame: (NSRect)cellFrame
{
  [cell _drawAttributedText: stringValue 
		    inFrame: cellFrame];
}

// NSBrowserCell
- (void) drawBrowserInteriorWithFrame: (NSRect)cellFrame 
			     withCell: (NSBrowserCell *)cell
			       inView: (NSView *)controlView
			    withImage: (NSImage *)theImage
		       alternateImage: (NSImage *)alternateImage
			isHighlighted: (BOOL)isHighlighted
				state: (int)state
			       isLeaf: (BOOL)isLeaf
{
  NSRect	title_rect = cellFrame;
  NSImage	*branch_image = nil;
  NSImage	*cell_image = theImage;

  if (isHighlighted || state)
    {
      if (!isLeaf)
	branch_image = [self highlightedBranchImage];
      if (nil != alternateImage)
	[cell setImage: alternateImage];

      // If we are highlighted, fill the background
      [[cell highlightColorInView: controlView] setFill];
      NSRectFill(cellFrame);
    }
  else
    {
      if (!isLeaf)
	branch_image = [self branchImage];

      // (Don't fill the background)
    }
  
  // Draw the branch image if there is one
  if (branch_image) 
    {
      NSRect imgRect;

      imgRect.size = [branch_image size];
      imgRect.origin.x = MAX(NSMaxX(title_rect) - imgRect.size.width - 4.0, 0.);
      imgRect.origin.y = MAX(NSMidY(title_rect) - (imgRect.size.height/2.), 0.);

      if (controlView != nil)
	{
	  imgRect = [controlView centerScanRect: imgRect];
	}

      [branch_image drawInRect: imgRect
		      fromRect: NSZeroRect
		     operation: NSCompositeSourceOver
		      fraction: 1.0
		respectFlipped: YES
			 hints: nil];

      title_rect.size.width -= imgRect.size.width + 8;
    }

  // Skip 2 points from the left border
  title_rect.origin.x += 2;
  title_rect.size.width -= 2;
  
  // Draw the cell image if there is one
  if (cell_image) 
    {
      NSRect imgRect;
      
      imgRect.size = [cell_image size];
      imgRect.origin.x = NSMinX(title_rect);
      imgRect.origin.y = MAX(NSMidY(title_rect) - (imgRect.size.height/2.),0.);

      if (controlView != nil)
	{
	  imgRect = [controlView centerScanRect: imgRect];
	}

      [cell_image drawInRect: imgRect
		    fromRect: NSZeroRect
		   operation: NSCompositeSourceOver
		    fraction: 1.0
	      respectFlipped: YES
		       hints: nil];

      title_rect.origin.x += imgRect.size.width + 4;
      title_rect.size.width -= imgRect.size.width + 4;
   }

  // Draw the body of the cell
  if ([cell _inEditing])
    {
      [self drawEditorForCell: cell 
		    withFrame: cellFrame 
		       inView: controlView];
    }
  else
    {
      [self drawInCell: cell
	attributedText: [cell attributedStringValue]
	       inFrame: title_rect];
    }
}

- (NSImage *) branchImage
{
  return [NSImage imageNamed: @"common_3DArrowRight"];
}

- (NSImage *) highlightedBranchImage
{
  return [NSImage imageNamed: @"common_3DArrowRightH"];
}
@end
