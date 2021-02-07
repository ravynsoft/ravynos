/** <title>NSPrintPanel</title>

   <abstract>Standard panel for querying user about printing.</abstract>

   Copyright (C) 2001,2004 Free Software Foundation, Inc.

   Written By: Adam Fedor <fedor@gnu.org>
   Date: Oct 2001
   Modified for Printing Backend Support
   Author: Chad Hardin <cehardin@mac.com>
   Date: June 2004
   
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

#import "config.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSBox.h"
#import "AppKit/NSForm.h"
#import "AppKit/NSNib.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSPrinter.h"
#import "AppKit/NSPrintPanel.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPrintOperation.h"
#import "AppKit/NSPopUpButton.h"
#import "AppKit/NSSavePanel.h"
#import "AppKit/NSView.h"
#import "GSGuiPrivate.h"
#import "GNUstepGUI/GSPrinting.h"
#import "GNUstepGUI/GSTheme.h"

static NSPrintPanel *shared_instance = nil;

#define GSPANELNAME @"GSPrintPanel"

#define CONTROL(panel, name) [[panel contentView] viewWithTag: name]

@interface NSPrintPanel (GSPrivate)
- (void)_updateFromPrintInfo: (NSPrintInfo*)info;
- (void)_finalWritePrintInfo: (NSPrintInfo*)info;
@end

/**
  <unit>
  <heading>Class Description</heading>
  <p>
  NSPrintPanel provides a standard print panel allowing the user to 
  specify information about how a document is to be printed, including
  the page range, number of copies and scale of the document. It also
  allows the user to save the document to a file or preview it.

  When a print: message is sent to an NSView or NSWindow, an
  NSPrintOpertation is created which manages printing of a view. The
  NSPrintOperation object would by default show a print panel in a modal
  loop for the user. You can avoid showing the print panel by sending
  the setShowsPanels: message in the print operation with a NO argument.
  </p>
  </unit>
*/ 
@implementation NSPrintPanel

//
// Class Methods
//
/** Load the appropriate bundle for the PrintPanel
    and alloc the class from that in our place
*/

+ (id) allocWithZone: (NSZone*) zone
{
  Class principalClass;
  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return nil;

  return [[principalClass printPanelClass] allocWithZone: zone];
}

/** Creates ( if needed) and returns a shared instance of the
    NSPrintPanel panel.
 */
+ (NSPrintPanel *)printPanel
{
  if (shared_instance == nil)
    {
      shared_instance = [[NSPrintPanel alloc] init];
    }
  return shared_instance;
}

//
// Instance methods
//
- (id) init
{
  return [self initWithContentRect: NSMakeRect(300, 300, 420, 350)
			 styleMask: NSTitledWindowMask
			   backing: NSBackingStoreBuffered
			     defer: YES];
}

/* Designated initializer */
- (id) initWithContentRect: (NSRect)contentRect
		 styleMask: (NSUInteger)aStyle
		   backing: (NSBackingStoreType)bufferingType
		     defer: (BOOL)flag
{
  unsigned int i;
  id control;
  NSArray *subviews, *list;
  NSString *panel;
  NSDictionary *table;

  self = [super initWithContentRect: contentRect
		 styleMask: aStyle
		   backing: bufferingType
		     defer: flag];
  if (self == nil)
    return nil;

  /* Set the title */
  [self setTitle: _(@"Print Panel")];

  _accessoryControllers = [[NSMutableArray alloc] init];

  // self will come from a bundle, to get the panel from the GUI library 
  // we have to select that bundle explicitly
  panel = [GSGuiBundle() pathForNibResource: GSPANELNAME];
  if (panel == nil)
    {
      NSRunAlertPanel(@"Error", @"Could not find print panel resource", 
		      @"OK", NULL, NULL);
      return nil;
    }
  table = [NSDictionary dictionaryWithObject: self forKey: NSNibOwner];
  if ([NSBundle loadNibFile: panel 
	  externalNameTable: table
		withZone: [self zone]] == NO)
    {
      NSRunAlertPanel(@"Error", @"Could not load print panel resource", 
		      @"OK", NULL, NULL);
      return nil;
    }

  /* Transfer the objects to us. FIXME: There must be a way to 
     instantiate the panel directly */
  subviews = [[_panel contentView] subviews];
  for (i = 0; i < [subviews count]; i++)
    {
      [_contentView addSubview: [subviews objectAtIndex: i]];
    }
  DESTROY(_panel);

  /* Setup the layout popup */
  control = CONTROL(self, NSPPLayoutButton);
  list = [NSArray arrayWithObjects: _(@"1 up"), _(@"2 up"), _(@"4 up"), _(@"6 up"), 
		  _(@"8 up"), nil];
  [control removeAllItems];
  for (i = 0; i < [list count]; i++)
    {
      [control addItemWithTitle: [list objectAtIndex: i]];
    }
  [control selectItemAtIndex: 0];

  return self;
}

- (void) dealloc
{
  RELEASE(_accessoryView);
  RELEASE(_savePath);
  RELEASE(_optionPanel);
  RELEASE(_printInfo);
  RELEASE(_accessoryControllers);
  RELEASE(_jobStyleHint);
  RELEASE(_helpAnchor);

  [super dealloc];
}

//
// Customizing the Panel 
//
/** <p>Sets the accessory view for the print panel to aView</p>
    <p>See Also: -accessoryView</p>
 */
- (void) setAccessoryView: (NSView *)aView
{
  ASSIGN(_accessoryView, aView);
}

/** <p>Returns the accessory view for the print panel </p>
    <p>See Also: -setAccessoryView:</p>
 */
- (NSView *) accessoryView
{
  return _accessoryView;
}

- (NSArray *) accessoryControllers
{
  return _accessoryControllers;
}

- (void) addAccessoryController: (NSViewController < NSPrintPanelAccessorizing >*)accessoryController
{
  [_accessoryControllers addObject: accessoryController];
}

- (void) removeAccessoryController: (NSViewController < NSPrintPanelAccessorizing >*)accessoryController
{
  [_accessoryControllers removeObjectIdenticalTo: accessoryController];
}

- (NSString *) defaultButtonTitle
{
  NSButton *defaultButton = CONTROL(self, NSOKButton);

  return [defaultButton title];
}

- (void) setDefaultButtonTitle: (NSString *)defaultButtonTitle
{
  NSButton *defaultButton = CONTROL(self, NSOKButton);

  [defaultButton setTitle: defaultButtonTitle];
}

- (NSPrintPanelOptions) options
{
  return _options;
}

- (void) setOptions: (NSPrintPanelOptions)options
{
  _options = options;
}

- (NSString *) jobStyleHint
{
  return _jobStyleHint;
}

- (void) setJobStyleHint: (NSString *)hint
{
  ASSIGN(_jobStyleHint, hint);
}

- (NSString *) helpAnchor
{
  return _helpAnchor;
}

- (void) setHelpAnchor: (NSString *)helpAnchor
{
  ASSIGN(_helpAnchor, helpAnchor);
}

- (NSPrintInfo *) printInfo
{
  return _printInfo;
}

//
// Running the Panel 
//
/** Display the Print panel in a modal loop. Saves any aquired 
   information in the NSPrintInfo object for the current NSPrintOperation. 
   Returns NSCancelButton if the user clicks the Cancel button or 
   NSOKButton otherwise. Unlike other panels, this one does not order
   itself out after the modal session is finished. You must do that
   yourself.
*/
- (NSInteger)runModal
{
  NSPrintInfo* info = [[NSPrintOperation currentOperation] printInfo];
  return [self runModalWithPrintInfo: info];
}

- (NSInteger) runModalWithPrintInfo: (NSPrintInfo *)printInfo
{
  NSInteger ret;

  _picked = NSOKButton;
  ASSIGN(_printInfo, printInfo);
  // Set the values from printInfo
  [self _updateFromPrintInfo: _printInfo];
  ret = [NSApp runModalForWindow: self];
  [_optionPanel orderOut: self];
  DESTROY(_printInfo);
  /* Don't order ourselves out, let the NSPrintOperation do that */
  return ret;
}

- (void) beginSheetWithPrintInfo: (NSPrintInfo *)printInfo 
		  modalForWindow: (NSWindow *)docWindow 
			delegate: (id)delegate 
		  didEndSelector: (SEL)didEndSelector 
		     contextInfo: (void *)contextInfo
{
  _picked = NSOKButton;
  ASSIGN(_printInfo, printInfo);
  // Set the values from printInfo
  [self _updateFromPrintInfo: _printInfo];
  [NSApp beginSheet: self
         modalForWindow: docWindow
         modalDelegate: delegate
         didEndSelector: didEndSelector
         contextInfo: contextInfo];
  [_optionPanel orderOut: self];
  DESTROY(_printInfo);
  [self orderOut: self];
}

- (void) _changeSaveType: (id)sender
{ 
  NSString *ext = [[sender selectedItem] representedObject];
  [(NSSavePanel *)[sender window] setAllowedFileTypes: [NSArray arrayWithObject: ext]];
}

- (NSBox *) _savePanelAccessory
{
  NSRect accessoryFrame = NSMakeRect(0,0,380,70);
  NSRect spaFrame = NSMakeRect(115,14,150,22);
  
  NSBox *save_panel_accessory = [[[NSBox alloc] initWithFrame: accessoryFrame] autorelease];
  [save_panel_accessory setTitle: _(@"File Type")];
  [save_panel_accessory setAutoresizingMask: 
			   NSViewWidthSizable | NSViewHeightSizable];
  NSPopUpButton *spa_button = [[NSPopUpButton alloc] initWithFrame: spaFrame];
  [spa_button setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable | NSViewMinYMargin |
	       NSViewMaxYMargin | NSViewMinXMargin | NSViewMaxXMargin];
  [spa_button setTarget: self];
  [spa_button setAction: @selector(_changeSaveType:)];
  [spa_button addItemWithTitle: @"PDF"];
  [[spa_button itemAtIndex: 0] setRepresentedObject: @"pdf"];
  [spa_button addItemWithTitle: @"PostScript"];
  [[spa_button itemAtIndex: 1] setRepresentedObject: @"ps"];
  [spa_button selectItemAtIndex: 0];
  [save_panel_accessory addSubview: spa_button];
  [spa_button release];

  return save_panel_accessory;
}

- (BOOL) _getSavePath
{
  int result;
  NSSavePanel *sp;

  sp = [NSSavePanel savePanel];
  [sp setAllowedFileTypes: [NSArray arrayWithObjects: @"pdf", nil]];
  [sp setAccessoryView: [self _savePanelAccessory]];
  result = [sp runModal];
  if (result == NSOKButton)
    {
      _savePath = RETAIN([sp filename]);
    }
  return (result == NSOKButton);
}

/* Private communication with our panel objects */
- (void) _pickedButton: (id)sender
{
  int tag = [sender tag];

  if (tag == NSPPSaveButton)
    {
      _picked = NSPPSaveButton;
      if ([self _getSavePath] == NO)
	{
	  /* User pressed save, then changed his mind, so go back to
	     the print panel (don't stop the modal session) */
	  return;
	}
    }
  else if (tag == NSPPPreviewButton)
    {
      _picked = NSPPPreviewButton;
    }
  else if (tag == NSFaxButton)
    {
      _picked = NSFaxButton;
      NSRunAlertPanel(_(@"Sorry"), _(@"Faxing of print file not implemented"), 
		      _(@"OK"), NULL, NULL);
      /* Don't stop the modal session */
      return;
    }
  else if (tag == NSCancelButton)
    {
      _picked = NSCancelButton;
    }
  else if (tag == NSOKButton)
    {
      _picked = NSOKButton;
    }
  else if (tag == NSPPOptionsButton)
    {
      /* Open the options panel */
      [NSApp runModalForWindow: _optionPanel];
      [_optionPanel orderOut: self];
      return;
    }
  else if (tag == NSPPOptionOKButton)
    {
      /* Do nothing. Just stops model on the options panel */
    }
  else
    {
      NSLog(@"Print panel buttonAction: from unknown sender - x%p\n", sender);
    }

  // FIXME
  [self _finalWritePrintInfo: _printInfo];

  [NSApp stopModalWithCode: (_picked == NSCancelButton) ? NSCancelButton :  NSOKButton];
}

- (void) _pickedPrinter: (id)sender
{
  NSString *name = [sender titleOfSelectedItem];
  NSPrinter *printer = [NSPrinter printerWithName: name];
 
  [_printInfo setPrinter: printer];
  [self _updateFromPrintInfo: _printInfo];
}

- (void) _pickedPage: (id)sender
{
  id pageMatrix = CONTROL(self, NSPPPageChoiceMatrix);
  id fromRangeForm = CONTROL(self, NSPPPageRangeFrom);
  id toRangeForm = CONTROL(self, NSPPPageRangeTo);

  if ([pageMatrix selectedColumn] == 0)
    {
      [[fromRangeForm cellAtIndex: 0] setStringValue: @"" ];
      [[toRangeForm cellAtIndex: 0] setStringValue: @"" ];
    }
  else
    {
      NSString *str;
      str = [NSString stringWithFormat: @"%lu", (unsigned long) _pages.location];
      [[fromRangeForm cellAtIndex: 0] setStringValue: str];
      str = [NSString stringWithFormat: @"%lu", (unsigned long) NSMaxRange(_pages)-1];
      [[toRangeForm cellAtIndex: 0] setStringValue: str];
    }
}

/* Depreciated communication methods */
/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedButton:(id)sender
{
  NSLog(@"[NSPrintPanel -pickedButton:] method depreciated");
  [self _pickedButton: sender];
}

/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedAllPages:(id)sender
{
  NSLog(@"[NSPrintPanel -pickedAllPages:] method depreciated");
  [self _pickedPage: sender];
}

/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedLayoutList:(id)sender
{
  NSLog(@"[NSPrintPanel -pickedLayoutList:] method depreciated");
}

//
// Communicating with the NSPrintInfo Object 
//
/** Setup the display in the receiver's panel based on the values stored
   in the NSPrintInfo object from the current NSPrintOperation.
*/
- (void)updateFromPrintInfo
{
  NSPrintInfo* info = [[NSPrintOperation currentOperation] printInfo];

  [self _updateFromPrintInfo: info];
}

/** Saves information set by the user in the receiver's panel 
   in the NSPrintInfo object from the current NSPrintOperation.
*/
- (void)finalWritePrintInfo
{
  NSPrintInfo* info = [[NSPrintOperation currentOperation] printInfo];

  [self _finalWritePrintInfo: info];
}

/* Private method for NSPrintOperation */
- (void) _setStatusStringValue: (NSString *)string
{
  [CONTROL(self, NSPPStatusField) setStringValue: string ];
}
@end

@implementation NSPrintPanel (GSPrivate)
- (void)_updateFromPrintInfo: (NSPrintInfo*)info
{
  id control;
  int layout;
  double scale;
  NSString *str;
  NSPrinter *printer;
  NSDictionary *dict;

  printer = [info printer];
  dict = [info dictionary];

  /* Setup printer information */
  {
    NSArray *printerNames = [NSPrinter printerNames];
    NSInteger i;
    NSPopUpButton *button = CONTROL(self, NSPPNameField);
    [button removeAllItems];
    
    for (i=0; i<[printerNames count]; i++)
      {
      NSString *printerName = [printerNames objectAtIndex: i];
      [button addItemWithTitle: printerName];
      if ([[printer name] isEqual: printerName])
	{
	  [button selectItemAtIndex: i];
	}
    }
  }

  [CONTROL(self, NSPPNoteField) setStringValue: [printer note] ];
  [CONTROL(self, NSPPStatusField) setStringValue: @"Idle" ];

  /* Setup copies, page range, scale */
  [CONTROL(self, NSPPCopiesField)  setIntValue: 1];
  [[CONTROL(self, NSPPPageRangeFrom) cellAtIndex: 0] setStringValue: @"" ];
  [[CONTROL(self, NSPPPageRangeTo) cellAtIndex: 0] setStringValue: @"" ];
  [CONTROL(self, NSPPPageChoiceMatrix) selectCellAtRow: 0 column: 0];
  if ([dict objectForKey: NSPrintScalingFactor])
    scale = [[dict objectForKey: NSPrintScalingFactor] doubleValue];
  else 
    scale = 0;
  if (scale == 0)
    scale = 1;
  [CONTROL(self, NSPPScaleField) setIntValue: (int)(scale*100)];


  dict = [info dictionary];
  NSDebugLLog(@"NSPrinting", 
	      @"Update PrintInfo dictionary\n %@ \n --------------", dict);
  _pages = NSMakeRange([[dict objectForKey: NSPrintFirstPage] intValue],
		       [[dict objectForKey: NSPrintLastPage] intValue]);
  if (NSMaxRange(_pages) == 0)
    _pages = NSMakeRange(1, 0);

  /* Setup the layout popup */
  layout = [[dict objectForKey: NSPrintPagesPerSheet] intValue];
  if (layout == 0)
    layout = 1;
  if (layout > 4)
    layout = 4;
  control = CONTROL(self, NSPPLayoutButton);
  [control selectItemAtIndex: (int)(layout/2)];

  /* Setup the resolution popup */
  control = CONTROL(_optionPanel, NSPPResolutionButton);
  [control removeAllItems];
  /* FIXME: Get default from printInfo? */
  str = [printer stringForKey:@"DefaultResolution" inTable: @"PPD"];
  if (str)
    {
      NSArray *list;
      list = [printer stringListForKey:@"Resolution" inTable: @"PPD"];
      if ([list count])
	{
	  unsigned int i;
	  NSString *display, *option;

	  for (i = 0; i < [list count]; i++)
	    {
	      NSString *key = [list objectAtIndex: i];
	      option = [@"Resolution/" stringByAppendingString: key];
	      display = [printer stringForKey: option
				        inTable: @"PPDOptionTranslation"];

	      if (display == nil)
		display = key;
	      [control addItemWithTitle: display];
	    }
	  option = [@"Resolution/" stringByAppendingString: str];
	  display = [printer stringForKey: option
				inTable: @"PPDOptionTranslation"];
	  
	  if (display == nil)
	    display = str;
	  [control selectItemWithTitle: display];
	}
      else
	{
	  [control addItemWithTitle: str];
	}
    }
  else
    [control addItemWithTitle: _(@"Unknown")];

  /* Setup the paper feed popup */
  control = CONTROL(_optionPanel, NSPPPaperFeedButton);
  [control removeAllItems];
  str = [printer stringForKey:@"DefaultInputSlot" inTable: @"PPD"];
  if (str)
    {
      NSString *manual;
      NSArray *list;

      manual = [printer stringForKey:@"DefaultManualFeed" inTable: @"PPD"];
      if (manual)
	[control addItemWithTitle: _(@"Manual")];
      list = [printer stringListForKey:@"InputSlot" inTable: @"PPD"];
      if ([list count])
	{
	  unsigned int i;
	  NSString *display, *option;

	  for (i = 0; i < [list count]; i++)
	    {
	      NSString *paper = [list objectAtIndex: i];
	      option = [@"InputSlot/" stringByAppendingString: paper];
	      display = [printer stringForKey: option
				        inTable: @"PPDOptionTranslation"];

	      if (display == nil)
		display = paper;
	      [control addItemWithTitle: display];
	    }
	  /* FIXME: What if manual is default ? */
	  option = [@"InputSlot/" stringByAppendingString: str];
	  display = [printer stringForKey: option
				  inTable: @"PPDOptionTranslation"];
	  
	  if (display == nil)
	    display = str;
	  [control selectItemWithTitle: display];
	}
      else
	{
	  [control addItemWithTitle: str];
	}
    }
  else
    [control addItemWithTitle: _(@"Unknown")];
}

#define NSNUMBER(a) [NSNumber numberWithInt: (a)]

- (void)_finalWritePrintInfo: (NSPrintInfo*)info
{
  id control;
  double scale;
  int layout;
  NSString *sel;
  NSArray  *list;
  NSPrinter *printer;
  NSMutableDictionary *dict;
  NSMutableDictionary *features;

  dict = [info dictionary];
  printer = [info printer];
  features = [dict objectForKey: NSPrintJobFeatures];

  /* Copies */
  control = CONTROL(self, NSPPCopiesField);
  if ([control intValue] > 1)
    {
      [dict setObject: NSNUMBER([control intValue]) 
	       forKey: NSPrintCopies];
    }

  /* Pages */
  control = CONTROL(self, NSPPPageChoiceMatrix);
  if ([control selectedColumn] != 0)
    {
      id fromRangeForm = CONTROL(self, NSPPPageRangeFrom);
      id toRangeForm = CONTROL(self, NSPPPageRangeTo);
      [dict setObject: NSNUMBER([[fromRangeForm cellAtIndex: 0] intValue])
	    forKey: NSPrintFirstPage];
      [dict setObject: NSNUMBER([[toRangeForm cellAtIndex: 0] intValue])
	    forKey: NSPrintLastPage];
      [dict setObject: NSNUMBER(NO) forKey: NSPrintAllPages];
    }
  else
      [dict setObject: NSNUMBER(YES) forKey: NSPrintAllPages];

  /* Scale */
  control = CONTROL(self, NSPPScaleField);
  scale = [control doubleValue]/100.0;
  if (scale <= 0)
    scale = .1;
  if (scale >= 10)
    scale = 10;
  [control setIntValue: (int)(scale*100)];
  [dict setObject: [NSNumber numberWithDouble: scale]
	   forKey: NSPrintScalingFactor];

  /* Layout */
  layout = [CONTROL(self, NSPPLayoutButton) indexOfSelectedItem] * 2;
  if (layout == 0)
    layout = 1;
  [dict setObject: NSNUMBER(layout) forKey: NSPrintPagesPerSheet];

  /* Resolution */
  /* Here we take advantage of the fact the names in the popup list
     are in the same order as the PPD file, so we don't actually compare
     the values */
  control = CONTROL(_optionPanel, NSPPResolutionButton);
  list = [printer stringListForKey: @"Resolution" inTable: @"PPD"];
  if (list)
    {
      NSString *def;
      sel = [list objectAtIndex: [control indexOfSelectedItem]];
      def = [printer stringForKey:@"DefaultResolution" inTable: @"PPD"];
      if ([sel isEqual: def] == NO
	  || [features objectForKey: @"Resolution"])
	{
	  if (features == nil)
	    {
	      features = [NSMutableDictionary dictionary];
	      [dict setObject: features forKey: NSPrintJobFeatures];
	    }
	  sel = [@"Resolution/" stringByAppendingString: sel];
	  [features setObject: sel forKey: @"Resolution"];
	}
    }
  
  /* Input Slot */
  control = CONTROL(_optionPanel, NSPPPaperFeedButton);
  list = [printer stringListForKey:@"InputSlot" inTable: @"PPD"];
  if (list)
    {
      int selected;
      NSString *def, *manual;
      sel = nil;
      selected = [control indexOfSelectedItem];
      manual = [printer stringForKey:@"DefaultManualFeed" inTable: @"PPD"];
      
      if (manual)
	{
	  if (selected == 0)
	    sel = _(@"Manual");
	  else
	    selected--;
	}
      if (sel == nil)
	sel = [list objectAtIndex: selected];
      def = [printer stringForKey:@"DefaultInputSlot" inTable: @"PPD"];
      if ([sel isEqual: _(@"Manual")] == YES)
	{
	  [dict setObject: NSPrintManualFeed forKey: NSPrintPaperFeed];
	  /* FIXME: This needs to be more robust. I'm just assuming
	     that all Manual Feed keys can be True or False (which is
	     the case for all the ppd files that I know of). */
	  [dict setObject: @"ManualFeed/True" forKey: NSPrintManualFeed];
	  [features setObject: @"ManualFeed/True" forKey: NSPrintPaperFeed];
	}
      else if ([sel isEqual: def] == NO
	       || [dict objectForKey: NSPrintPaperFeed])
	{
	  if (features == nil)
	    {
	      features = [NSMutableDictionary dictionary];
	      [dict setObject: features forKey: NSPrintJobFeatures];
	    }
	  sel = [@"InputSlot/" stringByAppendingString: sel];
	  [features setObject: sel forKey: @"InputSlot"];
	  [dict setObject: sel forKey: NSPrintPaperFeed];
	}
    }

  /* Job Resolution */
  switch (_picked)
    {
    case NSPPSaveButton:
      sel = NSPrintSaveJob;
      [dict setObject: _savePath forKey: NSPrintSavePath];
      break;
    case NSPPPreviewButton:
      sel = NSPrintPreviewJob;
      break;
    case NSFaxButton:
      sel = NSPrintFaxJob;
      break;
    case NSOKButton:
      sel = NSPrintSpoolJob;
      break;
    case NSCancelButton:
    default:
      sel = NSPrintCancelJob;
    }
  [info setJobDisposition: sel];

  NSDebugLLog(@"NSPrinting", 
	      @"Final info dictionary ----\n %@ \n --------------", dict);
}

@end
