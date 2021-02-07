/** <title>GSThemeInspector</title>

   <abstract>Utility fgor inspecting themes</abstract>

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2007,2008
   
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

#import "AppKit/NSImageView.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSWindow.h"
#import "GSThemePrivate.h"
#import "GSGuiPrivate.h"

static NSTextField *
new_label (NSString *value)
{
  NSTextField *t;

  t = AUTORELEASE([NSTextField new]);
  [t setStringValue: value];
  [t setDrawsBackground: NO];
  [t setEditable: NO];
  [t setSelectable: NO];
  [t setBezeled: NO];
  [t setBordered: NO];
  [t setAlignment: NSLeftTextAlignment];
  return t;
}

/* Implemented in GSInfoPanel.m
 * An object that displays a list of left-aligned strings (used for the authors)
 */
@interface _GSLabelListView: NSView
{
}
/* After initialization, its size is the size it needs, just move it
   where we want it to show */
- (id) initWithStringArray: (NSArray *)array
		      font: (NSFont *)font;
@end

@implementation	GSThemeInspector

static GSThemeInspector	*sharedInspector = nil;

+ (GSThemeInspector*) sharedThemeInspector
{
  if (sharedInspector == nil)
    {
      sharedInspector = [self new];
    }
  return sharedInspector;
}

- (id) init
{
  NSRect	frame;

  frame.size = NSMakeSize(272,388);
  frame.origin = NSZeroPoint;
  self = [super initWithContentRect: frame
    styleMask: (NSTitledWindowMask | NSClosableWindowMask
      | NSMiniaturizableWindowMask | NSResizableWindowMask)
    backing: NSBackingStoreBuffered
    defer: NO];
  
  [self setReleasedWhenClosed: NO];

  return self;
}

- (void) update: (id)sender
{
  GSTheme	*theme = [GSTheme theme];
  NSString	*version;
  NSString	*details;
  NSArray	*authors;
  NSView	*content = [self contentView];
  NSRect	cFrame = [content frame];
  NSView	*view;
  NSImageView	*iv;
  NSTextField	*tf;
  NSRect	nameFrame;
  NSRect	frame;
  int		width;
  int		fsize = 32;

  while ((view = [[content subviews] lastObject]) != nil)
    {
      [view removeFromSuperview];
    }
  frame = NSMakeRect(cFrame.size.width - 58, cFrame.size.height - 58, 48, 48);
  iv = [[NSImageView alloc] initWithFrame: frame];
  [iv setImage: [[GSTheme theme] icon]];
  [content addSubview: iv];
  RELEASE(iv);

  width = cFrame.size.width - 58;
  tf = new_label([theme name]);
  do
    {
      [tf setFont: [NSFont boldSystemFontOfSize: fsize]];
      fsize -= 2;
      [tf sizeToFit];
      nameFrame = [tf frame];
    }
  while (nameFrame.size.width > width && fsize > 8);
  nameFrame.origin.x = (width - nameFrame.size.width) / 2;
  nameFrame.origin.y = cFrame.size.height - nameFrame.size.height - 25;
  [tf setFrame: nameFrame];
  [content addSubview: tf];

  version = [[theme infoDictionary] objectForKey: @"GSThemeVersion"];
  if ([version length] > 0)
    {
      version = [NSString stringWithFormat: _(@"Version: %@"), version];
      tf = new_label(version);
      [tf setFont: [NSFont systemFontOfSize: 12]];
      [tf sizeToFit];
      frame = [tf frame];
      frame.origin.x = (cFrame.size.width - frame.size.width) / 2;
      frame.origin.y = nameFrame.origin.y - frame.size.height - 25;
      [tf setFrame: frame];
      [content addSubview: tf];
      nameFrame = [tf frame];
    }

  authors = [[theme infoDictionary] objectForKey: @"GSThemeAuthors"];
  if ([authors count] > 0)
    {
      view = [[_GSLabelListView alloc] initWithStringArray: authors
        font: [NSFont systemFontOfSize: 14]];
      frame = [view frame];
      frame.origin.x = (cFrame.size.width - frame.size.width) / 2;
      frame.origin.y = nameFrame.origin.y - frame.size.height - 25;
      [view setFrame: frame];
      [content addSubview: view];
      RELEASE(view);
    }

  details = [[theme infoDictionary] objectForKey: @"GSThemeDetails"];
  if ([details length] > 0)
    {
      NSScrollView	*s;
      NSTextView	*v;
      NSRect		r;

      r = NSMakeRect(10, 10, cFrame.size.width - 20, frame.origin.y - 20);
      s = [[NSScrollView alloc] initWithFrame: r];
      [s setHasHorizontalScroller: NO];
      [s setHasVerticalScroller: YES];
      [s setBorderType: NSBezelBorder];
      [s setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
      [content addSubview: s];
      RELEASE(s);

      r = [[s contentView] bounds];
      v = [[NSTextView alloc] initWithFrame: r];
      [v setBackgroundColor: [self backgroundColor]];
      [v setHorizontallyResizable: YES];
      [v setVerticallyResizable: YES];
      [v setEditable: NO];
      [v setRichText: YES];
      [v setMinSize: NSMakeSize (0, 0)];
      [v setMaxSize: NSMakeSize (1E7, 1E7)];
      [v setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];
      [[v textContainer] setContainerSize:
	NSMakeSize (r.size.width, 1e7)];
      [[v textContainer] setWidthTracksTextView: YES];
      [v setString: details];
      [s setDocumentView: v];
      RELEASE(v);
    }

  [content setNeedsDisplay: YES];
}

@end
