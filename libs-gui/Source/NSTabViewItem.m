/** <title>NSTabViewItem</title>

   Copyright (C) 2000-2016 Free Software Foundation, Inc.

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

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSTabView.h"
#import "AppKit/NSTabViewItem.h"
#import "AppKit/PSOperators.h"
#import "AppKit/NSViewController.h"

@implementation NSTabViewItem

- (id) init
{
  return [self initWithIdentifier: @""];
}

- (id) initWithIdentifier: (id)identifier
{
  self = [super init];

  if (self)
    {
      ASSIGN(_ident, identifier);
      _state = NSBackgroundTab;
      _view = [NSView new];
      // Use the window background colour as default, not the control background colour.
      [self setColor: [NSColor windowBackgroundColor]];
    }

  return self;
}

- (void) dealloc
{
  TEST_RELEASE(_ident);
  RELEASE(_label);
  RELEASE(_view);
  RELEASE(_color);
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@: %@ (ident: %@)", 
		   NSStringFromClass([self class]), _label, _ident];
}

- (NSViewController *) viewController
{
  return _viewController;
}

- (void) setViewController: (NSViewController *)vc
{
  _viewController = vc; // weak
  [self setView: [vc view]];
}

+ (instancetype) tabViewItemWithViewController: (NSViewController *)vc
{
  NSTabViewItem *item = AUTORELEASE([[NSTabViewItem alloc] init]);
  if ([vc title] == nil || [[vc title] isEqualToString: @""])
    {
      NSString *className = [vc className];
      [item setLabel: className];
    }
  else
    {
      [item setLabel: [vc title]];
    }
  [item setViewController: vc];
  return item;
}

// Set identifier.

- (void) setIdentifier: (id)identifier
{
  ASSIGN(_ident, identifier);
}

- (id) identifier
{
  return _ident;
}

// Set label for item.

- (void) setLabel: (NSString*)label
{
  ASSIGN(_label, label);
}

- (NSString *) label
{
  return _label;
}

- (NSSize) sizeOfLabel: (BOOL)shouldTruncateLabel
{
  NSDictionary *  attr;
  NSString *string;
  NSSize rSize;

  if (nil == _label)
    return NSZeroSize;

  attr = [[NSDictionary alloc] initWithObjectsAndKeys: 
	       [_tabview font], NSFontAttributeName,
	       nil];

  if (shouldTruncateLabel) 
    {
      string = [self _truncatedLabel];
    } 
  else 
    {
      string = _label;
    }

  rSize = [string sizeWithAttributes: attr];
  RELEASE(attr);
  return rSize;
}

// Set view to display when item is clicked.

- (void) setView: (NSView*)view
{
  ASSIGN(_view, view);
}

- (NSView*) view
{
  return _view;
}

// Set color of tab surface.

- (void) setColor: (NSColor*)color
{
  ASSIGN(_color, color);
}

- (NSColor*) color
{
  return _color;
}

// tab state

- (NSTabState) tabState
{
  return _state;
}


// Tab view, this is the "super" view.

- (NSTabView*) tabView
{
  return _tabview;
}

// First responder.

- (void) setInitialFirstResponder: (NSView*)view
{
  // We don't retain this.  
  _first_responder = view;
}

- (id) initialFirstResponder
{
  return _first_responder;
}

// Draw item.

- (void) drawLabel: (BOOL)shouldTruncateLabel
	    inRect: (NSRect)tabRect
{
  NSDictionary *attr;
  NSString *string;

  if (nil == _label)
    return;

  _rect = tabRect;

  if (shouldTruncateLabel) 
    {
      string = [self _truncatedLabel];
    } 
  else 
    {
      string = _label;
    }

  attr = [[NSDictionary alloc] initWithObjectsAndKeys: 
			       [_tabview font], NSFontAttributeName,
			       [NSColor controlTextColor], NSForegroundColorAttributeName,
			       nil];

  {
    NSSize size = [string sizeWithAttributes: attr];
    NSRect labelRect = tabRect;

    labelRect.origin.y = tabRect.origin.y + ((tabRect.size.height - size.height) / 2);
    labelRect.size.height = size.height;

    [string drawInRect: labelRect withAttributes: attr];
  }
  RELEASE(attr);
}

- (void) setToolTip: (NSString*)toolTip
{
  ASSIGN(_toolTip, toolTip);
}

- (NSString*) toolTip
{
  return _toolTip;
}

// NSCoding protocol.

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: _ident forKey: @"NSIdentifier"];
      [aCoder encodeObject: _label forKey: @"NSLabel"];
      [aCoder encodeObject: _view forKey: @"NSView"];
      [aCoder encodeObject: _color forKey: @"NSColor"];
      [aCoder encodeObject: _tabview forKey: @"NSTabView"];
    }
  else
    {
      [aCoder encodeObject:_ident];
      [aCoder encodeObject:_label];
      [aCoder encodeObject:_view];
      [aCoder encodeObject:_color];
      [aCoder encodeValueOfObjCType: @encode(int) at: &_state];
      [aCoder encodeObject:_first_responder];
      [aCoder encodeObject:_tabview];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      id identifier = [aDecoder decodeObjectForKey: @"NSIdentifier"];

      self = [self initWithIdentifier: identifier];
      [self setLabel: [aDecoder decodeObjectForKey: @"NSLabel"]];
      [self setView: [aDecoder decodeObjectForKey: @"NSView"]];
      [self setColor: [aDecoder decodeObjectForKey: @"NSColor"]];
      [self _setTabView: [aDecoder decodeObjectForKey: @"NSTabView"]];
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_ident];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_label];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_view];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_color];
      [aDecoder decodeValueOfObjCType: @encode(int) at:&_state];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_first_responder];
      AUTORELEASE(_first_responder);
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_tabview];
      AUTORELEASE(_tabview);
    }

  return self;
}
@end

@implementation NSTabViewItem (GNUstep)

// Non spec

- (NSRect) _tabRect
{
  return _rect;
}

- (void) _setTabState: (NSTabState)tabState
{
  _state = tabState;
}

- (void) _setTabView: (NSTabView*)tabView
{
  _tabview = tabView;
}

- (NSString*) _truncatedLabel
{
  // FIXME: What is the algo to truncate?
  return _label;
}

@end
