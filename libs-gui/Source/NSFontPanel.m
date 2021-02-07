/** <title>NSFontPanel</title>

   <abstract>System generic panel for selecting and previewing fonts</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: Febuary 2000
   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: January 2001 - sizings and resizings
 
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
#import <Foundation/NSDebug.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSDragging.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSFontPanel.h"
#import "AppKit/NSFontManager.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSSplitView.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSBox.h"
#import "GNUstepGUI/GSCharacterPanel.h"

#import "GSGuiPrivate.h"

#define _SAVE_PANEL_X_PAD	5
#define _SAVE_PANEL_Y_PAD	4

static inline void _setFloatValue (NSTextField *field, float size)
{
  /* If casting size to int and then back to float we get no change, 
     it means it's an integer */
  if ((float)((int)size) == size)
    {
      /* We prefer using this if it's an int, so that it's
	 displayed like in `8' rather than like in
	 `8.0000000000'.  Yes - when NSCell's formatters are
	 finished we won't need this. */
      [field setIntValue: (int)size];
    }
  else
    {
      [field setFloatValue: size];
    }
}


static NSText *sizeFieldText = nil;

static float sizes[] = {4.0, 6.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0,
			14.0, 16.0, 18.0, 24.0, 36.0, 48.0, 64.0};

/* Implemented in NSBrowser */
@interface GSBrowserTitleCell : NSTextFieldCell
{
}
@end

@interface NSFontPanel (Private)
- (NSFont*) _fontForSelection: (NSFont*) fontObject;

-(void) _trySelectSize: (float)size
       updateSizeField: (BOOL)updateSizeField;

// Some action methods
- (void) cancel: (id) sender;
- (void) _togglePreview: (id) sender;
- (void) _doPreview;
- (void) ok: (id) sender;

- (void) _getOriginalSize;
- (id)_initWithoutGModel;

- (BOOL) _includeFont: (NSString *)fontName  delegate: (id)delegate;
@end

@implementation NSFontPanel

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSFontPanel class])
    {
      [self setVersion: 1];
    }
}

/** <p>Creates ( if needed ) and returns the shared NSFontPanel.</p> 
 */
+ (NSFontPanel*) sharedFontPanel
{
  NSFontManager	*fm = [NSFontManager sharedFontManager];

  return [fm fontPanel: YES];
}

+ (BOOL) sharedFontPanelExists
{
  NSFontManager	*fm = [NSFontManager sharedFontManager];

  return ([fm fontPanel: NO] != nil);
}

/*
 * Instance methods
 */
- (id) init
{
  //  if (![NSBundle loadNibNamed: @"FontPanel" owner: self]);
  [self _initWithoutGModel];

  ASSIGN(_faceList, [NSArray array]);
  _face = -1;
  _family = -1;
  [self reloadDefaultFontFamilies];
  [self _getOriginalSize];

  return self;
}

- (void) dealloc
{
  RELEASE(_panelFont);
  RELEASE(_familyList);
  TEST_RELEASE(_faceList);

  TEST_RELEASE(_accessoryView);

  [super dealloc];
}

/** <p>Returns whether the "set" button is enabled.</p>
    <p>See Also: -setEnabled:</p>
 */
- (BOOL) isEnabled
{
  NSButton *setButton = [[self contentView] viewWithTag: NSFPSetButton];

  return [setButton isEnabled];
}

/**<p>Sets whether the "set" button is enabled.</p>
   <p>See Also: -isEnabled</p>
 */
- (void) setEnabled: (BOOL)flag
{
  NSButton *setButton = [[self contentView] viewWithTag: NSFPSetButton];

  [setButton setEnabled: flag];
}

- (void) reloadDefaultFontFamilies
{
  NSFontManager *fm = [NSFontManager sharedFontManager];
  id fmDelegate = [fm delegate];
  NSBrowser *familyBrowser = [[self contentView] viewWithTag: NSFPFamilyBrowser];
  NSArray *fontFamilies = [fm availableFontFamilies];
  unsigned int i,j;
  NSMutableArray *familyList;

  /*
  Build an array of all families that have a font that will be included.
  */
  familyList = [[NSMutableArray alloc]
		  initWithCapacity: [fontFamilies count]];

  for (i = 0; i < [fontFamilies count]; i++)
    {
      NSArray *familyMembers;
      familyMembers = [fm availableMembersOfFontFamily:
			[fontFamilies objectAtIndex: i]];

      for (j = 0; j < [familyMembers count]; j++)
	{
	  if ([self _includeFont: [[familyMembers objectAtIndex: j] objectAtIndex: 0]
			delegate: fmDelegate])
	    {
	      [familyList addObject: [fontFamilies objectAtIndex: i]];
	      break;
	    }
	}
    }

  DESTROY(_familyList);
  _familyList = familyList;
  // Reload the display. 
  [familyBrowser loadColumnZero];
  // Reselect the current font. (Hopefully still there)
  [self setPanelFont: [fm selectedFont]
	isMultiple: [fm isMultiple]];
}

- (void) setPanelFont: (NSFont *)fontObject
	   isMultiple: (BOOL)flag
{
  NSTextField *previewArea;

  previewArea = [[self contentView] viewWithTag: NSFPPreviewField];

  ASSIGN(_panelFont, fontObject);
  _multiple = flag;
  
  if (fontObject == nil)
    {
      return;
    }

  if (flag)
    {
      // TODO: Unselect all items and show a message
      [previewArea setStringValue: _(@"Multiple fonts selected")];
      _family = -1;
      _face = -1;
    }
  else
    {
      NSFontManager *fm = [NSFontManager sharedFontManager];
      NSString *family = [fontObject familyName];
      NSString *fontName = [fontObject fontName];
      float size = [fontObject pointSize];
      NSBrowser *familyBrowser = [[self contentView] viewWithTag: NSFPFamilyBrowser];
      NSBrowser *faceBrowser = [[self contentView] viewWithTag: NSFPFaceBrowser];
      NSString *face = @"";
      unsigned int i;
 
      // Store style information for font
      _traits = [fm traitsOfFont: fontObject];
      _weight = [fm weightOfFont: fontObject];
      
      // Select the row for the font family
      for (i = 0; i < [_familyList count]; i++)
	{
	  if ([[_familyList objectAtIndex: i] isEqualToString: family])
	    break;
	}
      if (i < [_familyList count])
	{
	  [familyBrowser selectRow: i inColumn: 0];
	  _family = i;
	  ASSIGN(_faceList, [fm availableMembersOfFontFamily: family]);
	  [faceBrowser loadColumnZero];
	  _face = -1;
	}

      // Select the row for the font face
      for (i = 0; i < [_faceList count]; i++)
	{
	  if ([[[_faceList objectAtIndex: i] objectAtIndex: 0] 
		isEqualToString: fontName])
	    break;
	}
      if (i < [_faceList count])
	{
	  [faceBrowser selectRow: i inColumn: 0];
	  _face = i;
	  face = [[_faceList objectAtIndex: i] objectAtIndex: 1];
	}

      // show point size and select the row if there is one
      [self _trySelectSize: size  updateSizeField: YES];

      // Use in preview
      [previewArea setFont: fontObject];
      if (_previewString == nil)
        { 
	  [previewArea setStringValue: [NSString stringWithFormat: @"%@ %@ %d PT",
						 family, face, (int)size]];
	}
    }
}

/**<p>Converts the NSFont <var>fontObject</var></p>
 */
- (NSFont *) panelConvertFont: (NSFont *)fontObject
{
  NSFont	*newFont;

  if (_multiple)
    {
      //TODO: We go over every item in the panel and check if a 
      // value is selected. If so we send it on to the manager
      //  newFont = [fm convertFont: fontObject toHaveTrait: NSItalicFontMask];
      NSLog(@"Multiple font conversion not implemented in NSFontPanel");
      newFont = [self _fontForSelection: fontObject];
    }
  else 
    {
      newFont = [self _fontForSelection: fontObject];
    }

  if (newFont == nil)
    {
      newFont = fontObject;
    }
  
  return newFont;
}

/**<p>Overides the NSPanel/NSWindow method to always returns YES</p>
 */
- (BOOL) worksWhenModal
{
  return YES;
}

/** <p>Returns the NSFontPanel's accessory view.</p>
    <p>See Also: -setAccessoryView:</p>
 */
- (NSView*) accessoryView
{
  return _accessoryView;
}

/** <p>Sets the NSFontPanel's accessory view to <var>aView</var></p>
    <p>See Also: -accessoryView</p>
 */
- (void) setAccessoryView: (NSView*)aView
{
  NSRect accessoryViewFrame, bottomFrame;
  NSRect tmpRect;
  NSSize contentSize, contentMinSize;
  float addedHeight, accessoryWidth;

  if (aView == _accessoryView)
    return;
  
  /* The following code is very tricky.  Please think and test a lot
     before changing it. */

  /* Remove old accessory view if any */
  if (_accessoryView != nil)
    {
      /* Remove accessory view */
      accessoryViewFrame = [_accessoryView frame];
      [_accessoryView removeFromSuperview];

      /* Change the min size before doing the resizing otherwise it
	 could be a problem. */
      [self setMinSize: _originalMinSize];

      /* Resize the panel to the height without the accessory view. 
	 This must be done with the special care of not resizing 
	 the heights of the other views. */
      addedHeight = accessoryViewFrame.size.height + (_SAVE_PANEL_Y_PAD * 2);
      contentSize = [[self contentView] frame].size;
      contentSize.height -= addedHeight;
      // Resize without modifying topView and bottomView height.
      [_topView setAutoresizingMask: NSViewWidthSizable | NSViewMinYMargin];
      [self setContentSize: contentSize];
      [_topView setAutoresizingMask: NSViewWidthSizable|NSViewHeightSizable];
    }
  
  /* Resize the panel to its original size.  This resizes freely the
     heights of the views.  NB: minSize *must* come first */
  [self setMinSize: _originalMinSize];
  [self setContentSize: _originalSize];
  
  /* Set the new accessory view */
  _accessoryView = aView;
  
  /* If there is a new accessory view, plug it in */
  if (_accessoryView != nil)
    {
      /* Make sure the new accessory view behaves  - its height must be fixed
       * and its position relative to the bottom of the superview must not
       * change	- so its position rlative to the top must be changable. */
      [_accessoryView setAutoresizingMask: NSViewMaxYMargin
	| ([_accessoryView autoresizingMask] 
	& ~(NSViewHeightSizable | NSViewMinYMargin))];  
      
      /* Compute size taken by the new accessory view */
      accessoryViewFrame = [_accessoryView frame];
      addedHeight = accessoryViewFrame.size.height + (_SAVE_PANEL_Y_PAD * 2);
      accessoryWidth = accessoryViewFrame.size.width + (_SAVE_PANEL_X_PAD * 2);

      /* Resize content size accordingly */
      contentSize = _originalSize;
      contentSize.height += addedHeight;
      if (accessoryWidth > contentSize.width)
	{
	  contentSize.width = accessoryWidth;
	}
      
      /* Set new content size without resizing heights of topView, bottomView */
      // Our views should resize horizontally if needed, but not vertically
      [_topView setAutoresizingMask: NSViewWidthSizable | NSViewMinYMargin];
      [self setContentSize: contentSize];
      // Restore the original autoresizing masks
      [_topView setAutoresizingMask: NSViewWidthSizable|NSViewHeightSizable];

      /* Compute new min size */
      contentMinSize = _originalMinSize;
      contentMinSize.height += addedHeight;
      // width is more delicate
      tmpRect = NSMakeRect (0, 0, contentMinSize.width, contentMinSize.height);
      tmpRect = [NSWindow contentRectForFrameRect: tmpRect 
			  styleMask: [self styleMask]];
      if (accessoryWidth > tmpRect.size.width)
	{
	  contentMinSize.width += accessoryWidth - tmpRect.size.width;
	}
      // Set new min size
      [self setMinSize: contentMinSize];

      /*
       * Pack the Views
       */

      /* BottomView is ready */
      bottomFrame = [_bottomView frame];

      /* AccessoryView */
      accessoryViewFrame.origin.x 
	= (contentSize.width - accessoryViewFrame.size.width) / 2;
      accessoryViewFrame.origin.y =  NSMaxY (bottomFrame) + _SAVE_PANEL_Y_PAD;
      [_accessoryView setFrameOrigin: accessoryViewFrame.origin];

      /* Add the accessory view */
      [[self contentView] addSubview: _accessoryView];
    }
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  [aCoder encodeObject: _panelFont];
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_multiple];
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_preview];
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  [super initWithCoder: aDecoder];

  _panelFont = RETAIN([aDecoder decodeObject]);
  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_multiple];
  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_preview];

  return self;
}

/*
 * Overriding fieldEditor:forObject: because we don't want the field
 * editor (which is used when you type in the size browser) to use the
 * font panel, otherwise typing in the size browser can modify the
 * currently selected font in unexpected ways !  */
- (NSText *) fieldEditor: (BOOL)createFlag
	       forObject: (id)anObject
{
  if (([anObject respondsToSelector: @selector(tag)])
      && ([anObject tag] == NSFPSizeField))
    {
      if ((sizeFieldText == nil) && createFlag)
        {
          sizeFieldText = [NSText new];
          [sizeFieldText setUsesFontPanel: NO];
          [sizeFieldText setFieldEditor: YES];
        }
      return sizeFieldText;
    }

  return [super fieldEditor: createFlag  forObject: anObject];
}

@end

@implementation NSFontPanel (Private)

- (id) _initWithoutGModel
{
  NSRect contentRect = {{100, 100}, {320, 300}};
  NSRect topAreaRect = {{0, 42}, {320, 258}};
  NSRect splitViewRect = {{8, 8}, {304, 243}};
  NSRect topSplitRect = {{0, 0}, {304, 45}};
  NSRect previewAreaRect = {{0, 1}, {304, 44}};
  NSRect bottomSplitRect = {{0, 0}, {304, 190}};
  NSRect familyBrowserRect = {{0, 0}, {111, 189}};
  NSRect typefaceBrowserRect = {{113, 0}, {111, 189}};
  NSRect sizeBrowserRect = {{226, 0}, {78, 143}};
  NSRect sizeLabelRect = {{226, 145}, {78, 21}};
  NSRect sizeTitleRect = {{226, 168}, {78, 21}};
  NSRect bottomAreaRect   = {{0, 0}, {320, 42}};
  NSRect slashRect        = {{0, 40}, {320, 2}};
  NSRect revertButtonRect = {{83, 8}, {71, 24}};
  NSRect previewButtonRect = {{162, 8}, {71, 24}};
  NSRect setButtonRect = {{241, 8}, {71, 24}};
  NSRect characterPanelButtonRect = {{8, 8}, {24, 24}};
  NSView *v;
  NSView *topArea;
  NSView *bottomArea;
  NSView *topSplit;
  NSView *bottomSplit;
  NSSplitView *splitView;
  NSTextField *previewArea;
  NSBrowser *sizeBrowser;
  NSBrowser *familyBrowser;
  NSBrowser *faceBrowser;
  NSTextField *label;
  NSTextField *sizeField;
  NSButton *revertButton;
  NSButton *previewButton;
  NSButton *setButton;
  NSButton *characterPanelButton;
  NSBox *slash;

  unsigned int style = NSTitledWindowMask | NSClosableWindowMask
                     | NSResizableWindowMask | NSUtilityWindowMask;

  self = [super initWithContentRect: contentRect 
			  styleMask: style
			    backing: NSBackingStoreRetained
			      defer: YES
			     screen: nil];
  if (!self)
    {
      return nil;
    }

  [self setTitle: _(@"Font Panel")];
  [self setBecomesKeyOnlyIfNeeded: YES];

  v = [self contentView];

  // preview and selection
  topArea = [[NSView alloc] initWithFrame: topAreaRect];
  [topArea setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  _topView = topArea;

  splitView = [[NSSplitView alloc] initWithFrame: splitViewRect];
  [splitView setVertical: NO]; 
  [splitView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];

  topSplit = [[NSView alloc] initWithFrame: topSplitRect];
  [topSplit setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];

  // Display for the font example
  previewArea = [[NSTextField alloc] initWithFrame: previewAreaRect];
  [previewArea setBackgroundColor: [NSColor textBackgroundColor]];
  [previewArea setDrawsBackground: YES];
  [previewArea setEditable: NO];
  [previewArea setSelectable: NO];
  //[previewArea setUsesFontPanel: NO];
  [previewArea setAlignment: NSCenterTextAlignment];
  [previewArea setStringValue: _(@"Font preview")];
  [previewArea setAutoresizingMask: (NSViewWidthSizable|NSViewHeightSizable)];

  [previewArea setTag: NSFPPreviewField];
  [topSplit addSubview: previewArea];
  RELEASE(previewArea);

  bottomSplit = [[NSView alloc] initWithFrame: bottomSplitRect];
  [bottomSplit setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];

  // Selection of the font family
  // We use a browser with one column to get a selection list
  familyBrowser = [[NSBrowser alloc] initWithFrame: familyBrowserRect];
  [familyBrowser setDelegate: self];
  [familyBrowser setMaxVisibleColumns: 1];
  [familyBrowser setMinColumnWidth: 0];
  [familyBrowser setAllowsMultipleSelection: NO];
  [familyBrowser setAllowsEmptySelection: YES];
  [familyBrowser setHasHorizontalScroller: NO];
  [familyBrowser setTitled: YES];
  [familyBrowser setTakesTitleFromPreviousColumn: NO];
  [familyBrowser setTarget: self];
  [familyBrowser setDoubleAction: @selector(familySelected:)];
  [familyBrowser setAction: @selector(_familySelectionChanged:)];
  [familyBrowser setAutoresizingMask: (NSViewWidthSizable
				       | NSViewMaxXMargin
				       | NSViewHeightSizable)];
  [familyBrowser setTag: NSFPFamilyBrowser];
  [bottomSplit addSubview: familyBrowser];
  RELEASE(familyBrowser);

  // selection of type face
  // We use a browser with one column to get a selection list
  faceBrowser = [[NSBrowser alloc] initWithFrame: typefaceBrowserRect];
  [faceBrowser setDelegate: self];
  [faceBrowser setMaxVisibleColumns: 1];
  [faceBrowser setMinColumnWidth: 0];
  [faceBrowser setAllowsMultipleSelection: NO];
  [faceBrowser setAllowsEmptySelection: YES];
  [faceBrowser setHasHorizontalScroller: NO];
  [faceBrowser setTitled: YES];
  [faceBrowser setTakesTitleFromPreviousColumn: NO];
  [faceBrowser setTarget: self];
  [faceBrowser setDoubleAction: @selector(faceSelected:)];
  [faceBrowser setAction: @selector(_faceSelectionChanged:)];
  [faceBrowser setAutoresizingMask: (NSViewWidthSizable
				     | NSViewMinXMargin
				     | NSViewHeightSizable)];
  [faceBrowser setTag: NSFPFaceBrowser];
  [bottomSplit addSubview: faceBrowser];
  RELEASE(faceBrowser);

  // label for selection of size
  label = [[NSTextField alloc] initWithFrame: sizeTitleRect];
  [label setCell: AUTORELEASE([GSBrowserTitleCell new])];
  [label setFont: [NSFont boldSystemFontOfSize: 0]];
  [label setAlignment: NSCenterTextAlignment];
  [label setDrawsBackground: YES];
  [label setEditable: NO];
  [label setTextColor: [NSColor windowFrameTextColor]];
  [label setBackgroundColor: [NSColor controlShadowColor]];
  [label setStringValue: _(@"Size")];
  [label setAutoresizingMask: NSViewMinXMargin | NSViewMinYMargin];
  [label setTag: NSFPSizeTitle];
  [bottomSplit addSubview: label];
  RELEASE(label);

  // this is the size input field
  sizeField = [[NSTextField alloc] initWithFrame: sizeLabelRect];
  [sizeField setDrawsBackground: YES];
  [sizeField setEditable: YES];
  [sizeField setAllowsEditingTextAttributes: NO];
  [sizeField setAlignment: NSCenterTextAlignment];
  [sizeField setAutoresizingMask: NSViewMinXMargin | NSViewMinYMargin];
  [sizeField setDelegate: self];
  [sizeField setTag: NSFPSizeField];
  [bottomSplit addSubview: sizeField];
  RELEASE(sizeField);

  sizeBrowser = [[NSBrowser alloc] initWithFrame: sizeBrowserRect];
  [sizeBrowser setDelegate: self];
  [sizeBrowser setMaxVisibleColumns: 1];
  [sizeBrowser setAllowsMultipleSelection: NO];
  [sizeBrowser setAllowsEmptySelection: YES];
  [sizeBrowser setHasHorizontalScroller: NO];
  [sizeBrowser setTitled: NO];
  [sizeBrowser setTakesTitleFromPreviousColumn: NO];
  [sizeBrowser setTarget: self];
  [sizeBrowser setDoubleAction: @selector(sizeSelected:)];
  [sizeBrowser setAction: @selector(_sizeSelectionChanged:)];
  [sizeBrowser setAutoresizingMask: NSViewMinXMargin | NSViewHeightSizable];
  [sizeBrowser setTag: NSFPSizeBrowser];
  [bottomSplit addSubview: sizeBrowser];
  RELEASE(sizeBrowser);

  [splitView addSubview: topSplit];
  RELEASE(topSplit);
  [splitView addSubview: bottomSplit];
  RELEASE(bottomSplit);

  [splitView setDelegate: self];

  [topArea addSubview: splitView];
  RELEASE(splitView);

  // action buttons
  bottomArea = [[NSView alloc] initWithFrame: bottomAreaRect];
  _bottomView = bottomArea;

  slash = [[NSBox alloc] initWithFrame: slashRect];
  [slash setBorderType: NSGrooveBorder];
  [slash setTitlePosition: NSNoTitle];
  [slash setAutoresizingMask: NSViewWidthSizable];
  [bottomArea addSubview: slash];
  RELEASE(slash);

  // cancel button
  revertButton = [[NSButton alloc] initWithFrame: revertButtonRect];
  [revertButton setTitle: _(@"Revert")];
  [revertButton setAction: @selector(cancel:)];
  [revertButton setTarget: self];
  [revertButton setTag: NSFPRevertButton];
  [revertButton setAutoresizingMask: NSViewMinXMargin];
  [bottomArea addSubview: revertButton];
  RELEASE(revertButton);

  // toggle button for preview
  previewButton = [[NSButton alloc] initWithFrame: previewButtonRect];
  [previewButton setTitle: _(@"Preview")];
  [previewButton setButtonType: NSOnOffButton];
  [previewButton setAction: @selector(_togglePreview:)];
  [previewButton setTarget: self];
  [previewButton setTag: NSFPPreviewButton];
  [previewButton setAutoresizingMask: NSViewMinXMargin];
  [previewButton setState: NSOnState];
  _preview = YES;
  [bottomArea addSubview: previewButton];
  RELEASE(previewButton);

  // button to set the font
  setButton = [[NSButton alloc] initWithFrame: setButtonRect];
  [setButton setTitle: _(@"Set")];
  [setButton setAction: @selector(ok:)];
  [setButton setTarget: self];
  [setButton setTag: NSFPSetButton];
  [setButton setAutoresizingMask: NSViewMinXMargin];
  [bottomArea addSubview: setButton];
  // make it the default button
  [self setDefaultButtonCell: [setButton cell]];
  RELEASE(setButton);

  // Character Panel button
  {
    NSString *label;
    unichar labelchars[2] = {0x03b1, 0x03b2}; // alpha, beta
    label = [[[NSString alloc] initWithCharacters: labelchars
					   length: 2] autorelease];

    characterPanelButton = [[NSButton alloc] initWithFrame: characterPanelButtonRect];
    [characterPanelButton setTitle: label];
    [characterPanelButton setToolTip: _(@"Character Panel")];
    [characterPanelButton setAction: @selector(characterPanel:)];
    [characterPanelButton setTarget: self];
    [bottomArea addSubview: characterPanelButton];
    RELEASE(characterPanelButton);
  }

  // set up the next key view chain
  [familyBrowser setNextKeyView: faceBrowser];
  [faceBrowser setNextKeyView: sizeField];
  [sizeField setNextKeyView: sizeBrowser];
  [sizeBrowser setNextKeyView: revertButton];
  [revertButton setNextKeyView: previewButton];
  [previewButton setNextKeyView: setButton];
  [setButton setNextKeyView: familyBrowser];

  [v addSubview: topArea];
  RELEASE(topArea);

  // Add the accessory view, if there is one
  if (_accessoryView != nil)
    {
      [v addSubview: _accessoryView];
    }

  [bottomArea setAutoresizingMask: NSViewWidthSizable];
  [v addSubview: bottomArea];
  RELEASE(bottomArea);

  [self setMinSize: [self frame].size];

  [self setInitialFirstResponder: setButton];
  [self setBecomesKeyOnlyIfNeeded: YES];
  
  return self;
}


- (void) _togglePreview: (id)sender
{
  _preview = (sender == nil) ? YES : [sender state];
  [self _doPreview];
}

- (void) _doPreview
{
  NSFont *font = nil;
  NSTextField *previewArea = [[self contentView] viewWithTag: NSFPPreviewField];

  if (_preview)
    {
      font = [self _fontForSelection: _panelFont];
      // build up a font and use it in the preview area
      if (font != nil)
	{
	  [previewArea setFont: font];
	}
    }

  if (_previewString == nil)
    { 
      NSTextField *sizeField = [[self contentView] viewWithTag: NSFPSizeField];
      float	size = [sizeField floatValue];
      NSString	*faceName;
      NSString	*familyName;
      
      if (size == 0 && font != nil)
	{
	  size = [font pointSize];
	}
      if (_family == -1)
	{
	  familyName = _(@"NoFamily");
	}
      else
	{
	  familyName = [_familyList objectAtIndex: _family];
	}
      if (_face == -1 || ![_faceList count])
	{
	  faceName = _(@"NoFace");
	}
      else
	{
	  faceName = [[_faceList objectAtIndex: _face] objectAtIndex: 1];
	}
      [previewArea setStringValue: [NSString stringWithFormat: @"%@ %@ %d PT",
					     familyName, faceName, (int)size]];
    }    
}

- (void) ok: (id)sender
{
  // The set button has been pushed
  NSFontManager *fm = [NSFontManager sharedFontManager];

  [fm modifyFontViaPanel: self];
}

- (void) cancel: (id)sender
{
  // FIXME/TODO

  /*
   * The cancel button has been pushed
   * we should reset the items in the panel 
   */
  [self setPanelFont: _panelFont
	  isMultiple: _multiple];
}

- (void) characterPanel: (id)sender
{
  [[NSApplication sharedApplication] orderFrontCharacterPalette: sender];
}

- (NSFont *) _fontForSelection: (NSFont *)fontObject
{
  float		size;
  NSString	*fontName;
  NSTextField	*sizeField = [[self contentView] viewWithTag: NSFPSizeField];
  unsigned	i = [_faceList count];

  size = [sizeField floatValue];
  if (size == 0.0)
    {
      if (fontObject == nil)
	{
	  size = 12.0;
	}
      else
	{
	  size = [fontObject pointSize];
	}
    }
  if (_face < 0)
    {

      if (i == 0)
	{
	  return nil;	/* Nothing available	*/
	}
      // FIXME - just uses first face
      fontName = [[_faceList objectAtIndex: 0] objectAtIndex: 0];
    }
  else
    {
      /*
      i really should be > 0 here, except for the very obscure case where
      the delegate has refused all fonts (so that our family and face lists
      are completely empty).
      */
      if (i)
	fontName = [[_faceList objectAtIndex: _face] objectAtIndex: 0];
      else
	return nil;
    }
  
  // FIXME: We should check if the font is correct
  return [NSFont fontWithName: fontName size: size];
}


-(void) _trySelectSize: (float)size
       updateSizeField: (BOOL)updateSizeField
{
  unsigned int i;
  NSBrowser *sizeBrowser = [[self contentView] viewWithTag: NSFPSizeBrowser];
  NSTextField *sizeField;

  if (updateSizeField)
    {
      /* Make sure our sizeField is updated. */
      sizeField = [[self contentView] viewWithTag: NSFPSizeField];
      _setFloatValue (sizeField, size);
    }
        
  /* Make sure our column is loaded. */ 
  [sizeBrowser loadColumnZero];

  for (i = 0; i < sizeof(sizes) / sizeof(float); i++)
    {
      if (size == sizes[i])
	{
          /* select the cell */
	  [sizeBrowser selectRow: i inColumn: 0];
	  break;
	}
    }
  if (i == sizeof(sizes) / sizeof(float))
    {
      /* TODO: No matching size found in the list. We should deselect
      everything. */
    }
}

- (void) controlTextDidChange: (NSNotification *)n
{
  NSTextField *sizeField = [[self contentView] viewWithTag: NSFPSizeField];
  float size = [sizeField floatValue];
  [self _trySelectSize: size  updateSizeField: NO];
  [self _doPreview];
}

/*
Ask the NSFontManager:s delegate if a font should be included. For speed,
the delegate is an argument; a repeat-caller can then cache it.
*/
- (BOOL) _includeFont: (NSString*)fontName  delegate: (id)fmDelegate
{
  if (fmDelegate != nil
    && [fmDelegate respondsToSelector: @selector(fontManager:willIncludeFont:)])
    {
      return [fmDelegate fontManager: [NSFontManager sharedFontManager]
		     willIncludeFont: fontName];
    }
  else
    return YES;
}


- (void) _getOriginalSize
{
  /* Used in setMinSize: */
  _originalMinSize = [self minSize];
  /* Used in setContentSize: */
  _originalSize = [[self contentView] frame].size;
}

@end


@implementation NSFontPanel (NSBrowserDelegate)


static int score_difference(int weight1, int traits1,
			    int weight2, int traits2)
{
  int score, t;

  score = (weight1 - weight2);
  score = 10 * score * score;

  t = traits1 ^ traits2;

  if (t & NSFixedPitchFontMask) score += 1000;
  if (t & NSCompressedFontMask) score += 150;
  if (t & NSPosterFontMask) score += 200;
  if (t & NSSmallCapsFontMask) score += 200;
  if (t & NSCondensedFontMask) score += 150;
  if (t & NSExpandedFontMask) score += 150;
  if (t & NSNarrowFontMask) score += 150;
  if (t & NSBoldFontMask) score += 20;
  if (t & NSItalicFontMask) score += 45;

  return score;
}


- (void) _familySelectionChanged: (id)sender
{
  NSFontManager *fm = [NSFontManager sharedFontManager];
  id fmDelegate = [fm delegate];

  NSBrowser *faceBrowser = [[self contentView] viewWithTag: NSFPFaceBrowser];
  NSBrowser *familyBrowser = [[self contentView] viewWithTag: NSFPFamilyBrowser];
  int row = [familyBrowser selectedRowInColumn: 0];

  unsigned int i;
  NSArray *entireFaceList;
  NSMutableArray *faceList;

  entireFaceList = [fm availableMembersOfFontFamily:
  			[_familyList objectAtIndex: row]];

  faceList = [[NSMutableArray alloc] initWithCapacity: [entireFaceList count]];

  for (i = 0; i < [entireFaceList count]; i++)
    {
      id aFace = [entireFaceList objectAtIndex:i];
      if ([self _includeFont: [aFace objectAtIndex:0]  delegate: fmDelegate])
	{
	  [faceList addObject: aFace];
	}
    }

  DESTROY(_faceList);
  _faceList = faceList;
  _family = row;

  // Select a face with the same properties
  for (i = 0; i < [_faceList count]; i++)
    {
      NSArray *font_info = [_faceList objectAtIndex: i];

      if (([[font_info objectAtIndex: 2] intValue] == _weight)
	  && ([[font_info objectAtIndex: 3] unsignedIntValue] == _traits))
	break;
    }

  // Find the face that differs the least from what we want
  if (i == [_faceList count])
    {
      int best, best_score, score;

      best_score = 1e6;
      best = -1;

      for (i = 0; i < [_faceList count]; i++)
	{
	  NSArray *font_info = [_faceList objectAtIndex: i];
	  score = score_difference(_weight, _traits,
	    [[font_info objectAtIndex: 2] intValue],
	    [[font_info objectAtIndex: 3] unsignedIntValue]);
	  if (score < best_score)
	    {
	      best = i;
	      best_score = score;
	    }
	}
      if (best != -1)
	i = best;
    }

  if (i == [_faceList count])
    i = 0;

  _face = i;
  [faceBrowser loadColumnZero];
  [faceBrowser selectRow: i inColumn: 0];

  /* Also make sure the size column has some value */
  {
    NSTextField *sizeField = [[self contentView] viewWithTag: NSFPSizeField];
    float size = [sizeField floatValue];

    if (size == 0.0)
      {
	[self _trySelectSize: 12.0  updateSizeField: YES];
      }
  }

  [self _doPreview];
}

- (void) _faceSelectionChanged: (id)sender
{
  NSBrowser *faceBrowser = [[self contentView] viewWithTag: NSFPFaceBrowser];
  int row = [faceBrowser selectedRowInColumn: 0];
  NSArray *font_info = [_faceList objectAtIndex: row];

  _face = row;
  _weight = [[font_info objectAtIndex: 2] intValue];
  _traits = [[font_info objectAtIndex: 3] unsignedIntValue];

  [self _doPreview];
}

- (void) _sizeSelectionChanged: (id)sender
{
  NSBrowser *sizeBrowser = [[self contentView] viewWithTag: NSFPSizeBrowser];
  int row = [sizeBrowser selectedRowInColumn: 0];
  NSTextField *sizeField;

  sizeField = [[self contentView] viewWithTag: NSFPSizeField];
  _setFloatValue (sizeField, sizes[row]);
  
  [self _doPreview];
}


- (NSInteger) browser: (NSBrowser*)sender  numberOfRowsInColumn: (NSInteger)column
{
  switch ([sender tag])
    {
    case NSFPFamilyBrowser: 
      {
	return [_familyList count];
      }
    case NSFPFaceBrowser:
      {
	return [_faceList count];
      }
    case NSFPSizeBrowser:
      {
	return sizeof (sizes) / sizeof (float);
      }
    default:
      {
	return 0;
      }
    }
}

- (NSString*) browser: (NSBrowser*)sender  titleOfColumn: (NSInteger)column
{
  switch ([sender tag])
    {
    case NSFPFamilyBrowser:
      {
	return _(@"Family");
      }
    case NSFPFaceBrowser:
      {
	return _(@"Typeface");
      }
    default:
      {
	return @"";
      }
    }
}

- (void) browser: (NSBrowser *)sender 
 willDisplayCell: (id)cell 
	   atRow: (NSInteger)row 
	  column: (NSInteger)column
{
  NSString *value = nil;

  if (row < 0)
    return;

  switch ([sender tag])
    {
    case NSFPFamilyBrowser:
      {
	if ([_familyList count] > (NSUInteger)row)
	  {
	    value = [_familyList objectAtIndex: row];
	  }
	break;
      }
    case NSFPFaceBrowser:
      {
	if ([_faceList count] > (NSUInteger)row)
	  {
	    value = [[_faceList objectAtIndex: row] objectAtIndex: 1];
	  } 
	break;
      }
    case NSFPSizeBrowser:
    default:
      {
	value = [NSString stringWithFormat: @"%d", (int) sizes[row]];
      }
    }
  
  [cell setStringValue: value];
  [cell setLeaf: YES];
}

- (BOOL) browser: (NSBrowser *)sender 
   isColumnValid: (NSInteger)column;
{
  return NO;
}

@end

@implementation NSFontPanel (NSSplitViewDelegate)

- (void) splitView: (NSSplitView *)splitView  
constrainMinCoordinate: (CGFloat *)min 
     maxCoordinate: (CGFloat *)max
       ofSubviewAt: (NSInteger)offset
{
  *max = *max - 100;
}

@end
