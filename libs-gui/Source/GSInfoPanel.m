/** <title>GSInfoPanel.m</title>

   <abstract>Standard GNUstep info panel</abstract>

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author:  Nicola Pero <n.pero@mi.flashnet.it>
   Date: January 2000

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

#import <Foundation/NSBundle.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSString.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSProcessInfo.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSImageView.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSTextField.h"
#import "GNUstepGUI/GSInfoPanel.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSGuiPrivate.h"

static id
value_from_info_plist_for_key (NSString *key)
{
  static NSDictionary *d = nil;
  /* We use this additional BOOL so that if loading Info-gnustep.plist
     fails once, we do not try again. */
  static BOOL load_failed = NO;
  
  if ((d == nil) && (load_failed == NO))
    {
      d = [[NSBundle mainBundle] localizedInfoDictionary];
      
      if (d == nil)
	load_failed = YES;
    }

  if (d)
    return [d objectForKey: key];      
  
  return nil;
}

static BOOL
nil_or_not_of_class (id object, Class class)
{
  return ((object == nil) || ([object isKindOfClass: class] == NO));
}

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



/*
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

@implementation _GSLabelListView

- (id) initWithStringArray: (NSArray *)array
		      font: (NSFont *)font
{
  self = [super init];
  if (self != nil)
    {
      unsigned int	count;
      NSTextField	*field;
      float		height = 2;
      float		width = 0;
      NSRect		r;

      count = [array count];
      /*
       * We go through the array in reverse order, adding items from
       * the bottom of the view working upwards.  This means that the
       * order of strings in the array will appear orderd from top to
       * bottom in the view.
       */
      while (count-- > 0)
	{
	  id	item = [array objectAtIndex: count];

	  if ([item isKindOfClass: [NSString class]] == NO)
	    continue;
	  field = new_label (item);
	  [field setFont: font];
	  [field sizeToFit];
	  [field setAutoresizingMask: NSViewNotSizable];
	  r = [field frame];
	  r.origin.x = 0;
	  r.origin.y = height;
	  if (r.size.width > width)
	    width = r.size.width;
	  height += r.size.height + 2;
	  [field setFrame: r];
	  [self setFrameSize: NSMakeSize (width, height)];
	  [self addSubview: field];
	}
      [self setFrameSize: NSMakeSize (width, height - 2)];
    }
  return self;
}
@end


@implementation GSInfoPanel: NSPanel
+ (void) initialize
{
  if (self == [GSInfoPanel class])
    {
      [self setVersion: 1];
    }
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  [super dealloc];
}

/* When the current theme changes, we need to update the info panel to match.
 */
- (void) _themeDidActivate: (NSNotification*)n
{
  NSView	*c = [self contentView];
  NSEnumerator	*e = [[c subviews] objectEnumerator];
  NSView	*v;
  NSButton	*b;

  while ((v = [e nextObject]) != nil)
    {
      if ([v isKindOfClass: [NSButton class]]
	&& [(b = (NSButton*)v) target] == [GSTheme class])
	{
	  NSString	*s;
	  NSRect	f;

	  s = [NSString stringWithFormat: @"%@: %@",
	    _(@"Current theme"), [[GSTheme theme] name]];
	  [b setTitle: s];
	  [b sizeToFit];
	  f = [b frame];
	  f.origin.x = ([c frame].size.width - f.size.width) / 2;
	  [b setFrame: f];
	  [c setNeedsDisplay: YES];
	}
    }
}

- (id) initWithDictionary: (NSDictionary *)dictionary;
{
  /* Info to show */
  NSString *name = nil;
  NSString *description = nil;
  NSImage  *icon = nil;
  NSString *release = nil;
  NSString *fullVersionID = nil;
  NSArray  *authors = nil;
  NSString *url = nil;
  NSString *copyright = nil;
  NSString *copyrightDescription = nil;
  NSString *theme = nil;

  /* GUI Objects used to show the Info */
  NSButton *iconButton;
  NSTextField *nameLabel;
  NSTextField *descriptionLabel = nil;
  NSTextField *versionLabel;
  _GSLabelListView *authorsList;
  NSTextField *authorTitleLabel;
  NSTextField *urlLabel = nil;
  NSTextField *copyrightLabel;
  NSTextField *copyrightDescriptionLabel = nil;
  NSButton    *themeLabel = nil;
  NSFont      *smallFont;

  /* Minimum size we use for the panel */
  float       width = 241;
  float       height = 107;

  /* Used for computations */
  float       tmp_A;
  float       tmp_a;
  float       tmp_b;
  float       tmp_c;
  NSRect      f;
  NSView      *cv;
  
  /*
   * Gets what we need to show
   */
  
  /* Application Name */ 
  if (dictionary)
    name = [dictionary objectForKey: @"ApplicationName"];

  if (nil_or_not_of_class (name, [NSString class]))
    {
      name = value_from_info_plist_for_key (@"ApplicationName");
      if (nil_or_not_of_class (name, [NSString class]))
	{
	  name = value_from_info_plist_for_key (@"NSHumanReadableShortName");
	  if (nil_or_not_of_class (name, [NSString class]))
	    {
	      name = value_from_info_plist_for_key (@"CFBundleName");
	      if (nil_or_not_of_class (name, [NSString class]))
		{
		  name = [[NSProcessInfo processInfo] processName]; 
		}
	    }
	}
    }
  /* Application Description */
  if (dictionary)
    description = [dictionary objectForKey: @"ApplicationDescription"];

  if (nil_or_not_of_class (description, [NSString class]))
    {
      description = value_from_info_plist_for_key
	(@"ApplicationDescription");
      
      if ([description isKindOfClass: [NSString class]] == NO)
	description = nil;
    }
  /* NB: description might be nil */
  /* Application Icon */
  if (dictionary)
    icon = [dictionary objectForKey: @"ApplicationIcon"];
  
  if (nil_or_not_of_class (icon, [NSImage class]))
    {
      icon = [NSImage imageNamed:
	value_from_info_plist_for_key (@"ApplicationIcon")];

      if (nil_or_not_of_class (icon, [NSImage class]))
	{
	  icon = [NSImage imageNamed: @"NSApplicationIcon"];
	  
	  if (nil_or_not_of_class (icon, [NSImage class]))
	    icon = [NSApp applicationIconImage];
	}
    }
  /* Release */
  if (dictionary)
    release = [dictionary objectForKey: @"ApplicationRelease"];
  
  if (nil_or_not_of_class (release, [NSString class]))
    {
      if (dictionary)
	release = [dictionary objectForKey: @"ApplicationVersion"];
      
      if (nil_or_not_of_class (release, [NSString class]))
	{
	  release = value_from_info_plist_for_key (@"ApplicationRelease");
  
	  if (nil_or_not_of_class (release, [NSString class]))
	    {
	      release = value_from_info_plist_for_key (@"NSAppVersion");
	      
	      if (nil_or_not_of_class (release, [NSString class]))
		{
		  release = value_from_info_plist_for_key (@"CFBundleVersion");
	      
		  if (nil_or_not_of_class (release, [NSString class]))
		    {
		      release = @"Unknown";
		    }
		}
	    }
	}
    }
  /* FullVersionID */
  if (dictionary)
    fullVersionID = [dictionary objectForKey: @"FullVersionID"];

  if (nil_or_not_of_class (fullVersionID, [NSString class]))
    {
      if (dictionary)
	fullVersionID = [dictionary objectForKey: @"Version"];

      if (nil_or_not_of_class (fullVersionID, [NSString class]))
	{
	  fullVersionID = value_from_info_plist_for_key (@"NSBuildVersion");
	  if ([fullVersionID isKindOfClass: [NSString class]] == NO)
	    fullVersionID = nil;
	}
    }
  /* NB: fullVersionID can be nil! */

  /* Now we prepare the complete release string */
  release = [_(@"Release: ") stringByAppendingString: release];
  if (fullVersionID)
    {
      release = [release stringByAppendingString: @" ("];
      release = [release stringByAppendingString: fullVersionID];
      release = [release stringByAppendingString: @")"];
    }
  /* Authors */
  if (dictionary)
    authors = [dictionary objectForKey: @"Authors"];

  if (nil_or_not_of_class (authors, [NSArray class]))
    {
      if ([authors isKindOfClass: [NSString class]])
	{
	  authors = [NSArray arrayWithObject: authors];
	}
      else
	{
	  authors = value_from_info_plist_for_key (@"Authors");
	  if (nil_or_not_of_class (authors, [NSArray class]))
	    {
	      if ([authors isKindOfClass: [NSString class]])
		{
		  authors = [NSArray arrayWithObject: authors];
		}
	      else
		{
		  authors = [NSArray arrayWithObject: @"Unknown"];
		}
	    }
	}
    }
  /* URL */
  if (dictionary)
      url = [dictionary objectForKey: @"URL"];

  if (nil_or_not_of_class (url, [NSString class]))
    {
      url = value_from_info_plist_for_key (@"URL");
    }
  // URL can be nil

  /* Copyright */
  if (dictionary)
    copyright = [dictionary objectForKey: @"Copyright"];

  if (nil_or_not_of_class (copyright, [NSString class]))
    {
      copyright = value_from_info_plist_for_key (@"Copyright");

      if (nil_or_not_of_class (copyright, [NSString class]))
	{
	  copyright = value_from_info_plist_for_key
	    (@"NSHumanReadableCopyright");
	  
	  if (nil_or_not_of_class (copyright, [NSString class]))
	    copyright = _(@"Copyright Information Not Available");
	}
    }
  /* Copyright Description */
  if (dictionary)
    copyrightDescription = [dictionary objectForKey: @"CopyrightDescription"];

  if (nil_or_not_of_class (copyrightDescription, [NSString class]))
    {
      copyrightDescription = value_from_info_plist_for_key
	(@"CopyrightDescription");

      if ([copyrightDescription isKindOfClass: [NSString class]] == NO)
	copyrightDescription = nil;
    }
  /* NB: copyrightDescription can be nil */
  
  /*
   * Create GUI Objects 
   */
  f = NSMakeRect(0, 0, 48, 48);

  iconButton = AUTORELEASE([[NSButton alloc] initWithFrame: f]); 
  [[iconButton cell] setImageScaling: NSImageScaleProportionallyUpOrDown];
  [iconButton setImage: icon];
  [iconButton setBordered: NO];
  [iconButton setImagePosition: NSImageOnly];
  /* Clicking on the iconButton starts the GSMemoryPanel.  */
  [iconButton setEnabled: YES];
  [iconButton setTarget: NSApp];
  [iconButton setAction: @selector(orderFrontSharedMemoryPanel:)];

  nameLabel = new_label (name);
  [nameLabel setFont: [NSFont boldSystemFontOfSize: 32]];
  [nameLabel sizeToFit];

  if (description)
    {
      descriptionLabel = new_label (description);
      [descriptionLabel setFont: [NSFont boldSystemFontOfSize: 14]];
      [descriptionLabel sizeToFit];
    }

  smallFont = [NSFont systemFontOfSize: 12];
  
  versionLabel = new_label (release);
  [versionLabel setFont: smallFont];
  [versionLabel sizeToFit];

  if ([authors count] == 0)
    {
      authorTitleLabel = new_label (@"");
    }
  else if ([authors count] == 1)
    {
      authorTitleLabel = new_label (_(@"Author: "));
    }
  else
    {
      authorTitleLabel = new_label (_(@"Authors: "));
    }
  [authorTitleLabel setFont: smallFont];
  [authorTitleLabel sizeToFit];

  authorsList = AUTORELEASE([[_GSLabelListView alloc]
    initWithStringArray: authors font: smallFont]);  
  
  if (url)
    {
      urlLabel = new_label (url);
      [urlLabel setFont: smallFont];
      [urlLabel sizeToFit];
    }

  copyrightLabel = new_label (copyright);
  [copyrightLabel setFont: smallFont];
  [copyrightLabel sizeToFit];

  if (copyrightDescription)
    {
      copyrightDescriptionLabel = new_label (copyrightDescription);
      [copyrightDescriptionLabel setFont: smallFont];
      [copyrightDescriptionLabel sizeToFit];
    }

  theme = [NSString stringWithFormat: @"%@: %@",
    _(@"Current theme"), [[GSTheme theme] name]];
  themeLabel = AUTORELEASE([NSButton new]);
  [themeLabel setTitle: theme];
  [themeLabel setBordered: NO];
  [themeLabel setAlignment: NSLeftTextAlignment];
  [themeLabel setFont: smallFont];
  [themeLabel setButtonType: NSMomentaryLightButton];
  [themeLabel setFocusRingType: NSFocusRingTypeNone];
  [themeLabel sizeToFit];

  /*
   * Compute width and height of the panel
   */

  /** width **/
  tmp_A = f.size.width;
  /* distance between icon and title */
  tmp_A += 10;
  /* compute the maximum of the following three sizes */
  tmp_a = [nameLabel frame].size.width;
  if (description)
    {
      tmp_b = [descriptionLabel frame].size.width;

      if (tmp_a < tmp_b)
	tmp_a = tmp_b;
    }
  
  tmp_b = [versionLabel frame].size.width;

  if (tmp_a < tmp_b)
    tmp_a = tmp_b;
  /* Add in to tmp_A */
  tmp_A += tmp_a;
  /* Update width */
  if (width < tmp_A)
    width = tmp_A;

  tmp_A = [authorTitleLabel frame].size.width; 
  tmp_A += [authorsList frame].size.width;
  if (width < tmp_A)
    width = tmp_A;

  /* FIXME depending on where we put url */
  if (url)
    {
      tmp_A = [urlLabel frame].size.width;
      if (tmp_A > width)
	width = tmp_A;
    }

  tmp_A = [copyrightLabel frame].size.width;
  if (tmp_A > width)
    width = tmp_A;

  if (copyrightDescription)
    {
      tmp_A = [copyrightDescriptionLabel frame].size.width;
      if (tmp_A > width)
	width = tmp_A;
    }

  tmp_A = [themeLabel frame].size.width;
  if (tmp_A > width)
    width = tmp_A;

  /* height */
  /* Warning: we implicitly assume icon height is approx of the
     standard height of 48.  The code tries to be nice so that 50 or
     47 should more or less work -- but beware that 200 or 20 will
     *not* work. */
  tmp_A = f.size.height;

  if (description)
    tmp_A += 10; 
  else
    tmp_A += 5;

  tmp_A += [versionLabel frame].size.height;
  tmp_A += 20;

  tmp_A += [authorsList frame].size.height;
  
  if (url)
    {
      tmp_A += [urlLabel frame].size.height + 2;
    }
  tmp_A += 25;
  tmp_A += [copyrightLabel frame].size.height;
  if (copyrightDescription)
    {
      tmp_A += 2;
      tmp_A += [copyrightDescriptionLabel frame].size.height;
    }

  tmp_A += 5;
  tmp_A += [themeLabel frame].size.height;

  if (tmp_A > height)
    height = tmp_A;

  /* Add border to both width and height */
  width += 32;
  height += 36;

  /*
   * TODO: Adjust/check obtained width and height ?
   * (NB: If they are adjusted, code putting views in the panel 
   *  has to be adjusted too!)
   */

  /*
   * Creates the panel with the right width and height
   */
  self = [super initWithContentRect: NSMakeRect (100, 100, width, height)
		styleMask: (NSTitledWindowMask | NSClosableWindowMask) 
		backing: NSBackingStoreRetained defer: YES];
  if (!self)
    return nil;
  
  /* 
   * Add objects to the panel in their position
   */
  cv = [self contentView];

  {
    NSImageView* backgroundImage = [[NSImageView alloc] 
                                       initWithFrame: 
                                           NSMakeRect(0, 0, width, height)];

    //[backgroundImage setImageAlignment: NSImageAlignCenter];
    //[backgroundImage setImageScaling: NSScaleProportionally];
    [backgroundImage setImage: [NSImage imageNamed: @"LogoGNUstep"]];
    [backgroundImage setEditable: NO];
    [cv addSubview: backgroundImage];
    RELEASE(backgroundImage);
  }

  f = [iconButton frame];
  f.origin.x = 16;
  f.origin.y = height - 18 - f.size.height;
  tmp_a = f.origin.x + f.size.width + 10;
  tmp_b = f.origin.y;
  tmp_c = f.size.height;
  [cv addSubview: iconButton]; 
  [iconButton setFrame: f];

  f = [nameLabel frame];
  f.origin.x = tmp_a;
  /* NB: We rely upon the fact that the text in a text field is
     vertically centered in its frame */
  if (description)
    f.origin.y = tmp_b + 10;
  else
    f.origin.y = tmp_b;
  f.size.height = tmp_c;
  [cv addSubview: nameLabel];
  [nameLabel setFrame: f];

  if (description)
    {
      f = [descriptionLabel frame];
      f.origin.x = tmp_a;
      f.origin.y = tmp_b - 5;
      [cv addSubview: descriptionLabel];
      [descriptionLabel setFrame: f];
    }

  f = [versionLabel frame];
  f.origin.x = width - 16 - f.size.width;
  if (description)
    f.origin.y = tmp_b - 10 - f.size.height;
  else
    f.origin.y = tmp_b - 5 - f.size.height;
  tmp_b = f.origin.y;
  [cv addSubview: versionLabel];
  [versionLabel setFrame: f];

  tmp_b -= 20;

  tmp_a = [authorTitleLabel frame].size.width;
  tmp_a += [authorsList frame].size.width;
  tmp_a = (width - tmp_a) / 2;
  
  f = [authorTitleLabel frame];
  f.origin.x = tmp_a;
  tmp_c = tmp_a + f.size.width;
  f.origin.y = tmp_b - f.size.height;
  [cv addSubview: authorTitleLabel];
  [authorTitleLabel setFrame: f];
  
  f = [authorsList frame];
  f.origin.x = tmp_c;
  f.origin.y = tmp_b - f.size.height;
  tmp_b = f.origin.y;
  [cv addSubview: authorsList];
  [authorsList setFrame: f];

  if (url)
    {
      /* FIXME position of this thing */
      f = [urlLabel frame];
      f.origin.x = tmp_a;
      f.origin.y = tmp_b - 2 - f.size.height;
      tmp_b = f.origin.y;
      [cv addSubview: urlLabel];
      [urlLabel setFrame: f];
    }

  f = [copyrightLabel frame];
  f.origin.x = (width - f.size.width) / 2;
  f.origin.y = tmp_b - 25 - f.size.height;
  tmp_b = f.origin.y;
  [cv addSubview: copyrightLabel];
  [copyrightLabel setFrame: f];

  if (copyrightDescription)
    {
      f = [copyrightDescriptionLabel frame];
      f.origin.x = (width - f.size.width) / 2;
      f.origin.y = tmp_b - 2 - f.size.height;
      tmp_b = f.origin.y;
      [cv addSubview: copyrightDescriptionLabel];
      [copyrightDescriptionLabel setFrame: f];
    }

  f = [themeLabel frame];
  f.origin.x = (width - f.size.width) / 2;
  f.origin.y = tmp_b - 5 - f.size.height;
  [cv addSubview: themeLabel];
  [themeLabel setFrame: f];
  [themeLabel setTarget: [GSTheme class]];
  [themeLabel setAction: @selector(orderFrontSharedThemePanel:)];

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_themeDidActivate:)
    name: GSThemeDidActivateNotification
    object: nil];

  [self center];
  return self;
}

- (void) copy: (id)sender
{  
  NSArray *types = [NSArray arrayWithObject: NSStringPboardType];
  NSPasteboard *pboard = [NSPasteboard generalPasteboard];
  NSMutableString *text = [[NSMutableString alloc] init];
  NSView *cv = [self contentView];
  NSEnumerator *enumerator = [[cv subviews] objectEnumerator];
  NSView *subview;
 
  // Loop over all the text subviews and collect the information
  while ((subview = [enumerator nextObject]) != nil)
    {
      if ([subview isKindOfClass: [NSTextField class]])
        {
          [text appendString: [(NSTextField*)subview stringValue]];
          [text appendString: @"\n"];
        }
    }
  
  [pboard declareTypes: types owner: self];
  [pboard setString: text
          forType: NSStringPboardType];
  RELEASE(text);
}

- (void) keyDown: (NSEvent*)theEvent
{
  NSString *characters = [theEvent characters];
  unichar character = 0;

  if ([characters length] > 0)
    {
      character = [characters characterAtIndex: 0];
    }

  // FIXME: Hard coded
  if (character == 'c' && ([theEvent modifierFlags] & NSCommandKeyMask))
    {
      [self copy: nil];
      return;
    }

  [super keyDown: theEvent];
}

@end
