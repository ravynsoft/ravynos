/** <title>GSHelpManagerPanel.m</title>

   <abstract>GSHelpManagerPanel displays a help message for an item.</abstract>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Pedro Ivo Andrade Tavares <ptavares@iname.com>
   Date: September 1999
   
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

#import "AppKit/NSApplication.h"
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSImage.h"
#import "GNUstepGUI/GSHelpManagerPanel.h"
#import "GSGuiPrivate.h"

@implementation GSHelpManagerPanel

static GSHelpManagerPanel* _GSsharedGSHelpPanel;

+ (id) sharedHelpManagerPanel
{
  if (!_GSsharedGSHelpPanel)
    _GSsharedGSHelpPanel = [[GSHelpManagerPanel alloc] init];

  return _GSsharedGSHelpPanel;
}

- (id)init
{
  self = [super initWithContentRect: NSMakeRect(100, 100, 470, 200)
		                      styleMask: NSTitledWindowMask | NSResizableWindowMask
		                        backing: NSBackingStoreRetained
		                          defer: NO];
  
  if (self) {
    NSRect scrollViewRect = {{8, 40}, {454, 152}};
    NSRect buttonRect = {{390, 6}, {72, 27}};
    NSRect r;
    NSScrollView *scrollView;
    NSButton *button;
    
    [self setReleasedWhenClosed: NO]; 
    [self setFloatingPanel: YES];
    [self setTitle: NSLocalizedString(@"Help", @"")];

    scrollView = [[NSScrollView alloc] initWithFrame: scrollViewRect];
    [scrollView setBorderType: NSBezelBorder];
    [scrollView setHasHorizontalScroller: NO];
    [scrollView setHasVerticalScroller: YES]; 
    [scrollView setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];

    r = [[scrollView contentView] frame];
    textView = [[NSTextView alloc] initWithFrame: r];
    [textView setRichText: YES];
    [textView setEditable: NO];
    [textView setSelectable: NO];
    [textView setHorizontallyResizable: NO];
    [textView setVerticallyResizable: YES];
    [textView setMinSize: NSMakeSize (0, 0)];
    [textView setMaxSize: NSMakeSize (1E7, 1E7)];
    [textView setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];
    [[textView textContainer] setContainerSize: NSMakeSize(r.size.width, 1e7)];
    [[textView textContainer] setWidthTracksTextView: YES];
    [textView setUsesRuler: NO];
    
    [scrollView setDocumentView: textView];
    RELEASE (textView);
    
    [[self contentView] addSubview: scrollView];
    RELEASE (scrollView);
    
    button = [[NSButton alloc] initWithFrame: buttonRect];
    [button setAutoresizingMask: NSViewMinXMargin | NSViewMaxYMargin];
    [button setButtonType: NSMomentaryLight];
    [button setTitle: NSLocalizedString(@"OK", @"")];
    [button setKeyEquivalent: @"\r"];
    [button setImagePosition: NSImageRight];
    [button setImage: [NSImage imageNamed: @"common_ret"]];
    [button setAlternateImage: [NSImage imageNamed: @"common_retH"]];
	  [button setTarget: self];
	  [button setAction: @selector(buttonAction:)];		

    [[self contentView] addSubview: button];
    RELEASE (button);

    [self makeFirstResponder: button];
  }

  return self;
}

- (void)setHelpText:(NSAttributedString *)helpText
{
  [[textView textStorage] setAttributedString: helpText];
}

- (void)buttonAction:(id)sender
{
  [self close];
}

- (void) close
{
  if ([self isVisible])
    {
      [NSApp stopModal];
    }
  [super close];
}

@end
