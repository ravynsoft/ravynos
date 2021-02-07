/** <title>NSPageLayout</title>

   <abstract>Standard panel for querying user about page layout.</abstract>

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

#import <Foundation/NSDebug.h>
#import <Foundation/NSDecimalNumber.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSNumberFormatter.h>
#import <Foundation/NSUserDefaults.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSImageView.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSBox.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSComboBox.h"
#import "AppKit/NSPopUpButton.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSForm.h"
#import "AppKit/NSFormCell.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPageLayout.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSTabView.h"
#import "AppKit/NSTabViewItem.h"
#import "AppKit/NSPrinter.h"
#import "GSGuiPrivate.h"
#import "GNUstepGUI/GSPrinting.h"
#import "GNUstepGUI/GSTheme.h"

static NSPageLayout *shared_instance;


//
// The buttons on the panel for controlling
// the addition, deletion, etc, of custom
// papers.
// The panel is also filled with NSTextFields
// that appear as "un" when opened in GORM.
// These have tags that start with 100 and
// are numbered up from there.  They are programatically
// set to text for the user's prefered measuremnt
// unit, by looking at NSMeasurementUnit.
enum {
  GSPLNewCustomPaperButton = 200,
  GSPLDuplicateCustomPaperButton = 201,
  GSPLDeleteCustomPaperButton = 202,
  GSPLSaveCustomPaperButton = 203
};



@implementation NSApplication (NSPageLayout)

- (void) runPageLayout: sender
{
  [[NSPageLayout pageLayout] runModal];
}

@end

//
// This is the controller for NSPageLayout, it does most of the
// work.  Implementation is near the end of the file
//
@interface GSPageLayoutController : NSObject //<NSTableDataSource>
{
  NSMutableDictionary *customPapers;  //a dictionary of dictionaries
  BOOL customPapersNeedSaving;
  NSString *measurementString; //examples are "in", "pt", or "cm"
  double factorValue;  //how to convert from points to cm or in
  NSTabViewItem *attributesTabViewItem;
  NSTabViewItem *customTabViewItem;
  NSTabViewItem *summaryTabViewItem;
  
  //IBOutlets
  id panel;
  id tabView;
  id applicationImageButton;
  id panelTitleField;
  id printerPopUp;
  id paperRadioMatrix;
  id standardPaperSizePopUp;
  id customPaperSizePopUp;
  id dimensionsTextField;
  id paperOrientationMatrix;
  id paperAttributesPreview;
  id scaleTextField;
  id customPaperTableView;
  id customPaperNameColumn;
  id customPaperWidthTextField;
  id customPaperHeightTextField;
  id customPaperMarginTopTextField;
  id customPaperMarginBottomTextField;
  id customPaperMarginRightTextField;
  id customPaperMarginLeftTextField;
  id customPaperPreview;
  id newCustomPaperButton;
  id duplicateCustomPaperButton;
  id deleteCustomPaperButton;
  id saveCustomPaperButton;
  id summaryTableView;
  id summarySettingColumn;
  id summaryValueColumn;
  
  //What used to be NSPageLayout's ivars
  NSPrintInfo *_printInfo;
  NSView *_accessoryView;
}

-(NSPageLayout*) panel;

//IBActions
-(void) okButtonClicked: (id)sender;
-(void) cancelButtonClicked: (id)sender;
-(void) printerPopUpClicked: (id)sender;
-(void) paperRadioMatrixClicked: (id)sender;
-(void) paperPopUpClicked: (id)sender;
-(void) paperOrientationMatrixClicked: (id) sender;
-(void) customPaperButtonsClicked: (id)sender;

//internal
-(void) determineMeasurements;
-(void) processAttributes;
-(void) syncInterface;

//access to ivars
-(NSPrintInfo*) printInfo;
-(void) setPrintInfo:(NSPrintInfo*)printInfo;
-(NSView*) accessoryView;
-(void) setAccessoryView:(NSView*)accessoryView;


//Handling of NSPageLayout implementation
-(void)readPrintInfo;
-(void)writePrintInfo;

//A NSTextField delegate handler we care about
-(void) controlTextDidChange:(NSNotification*) notification;

//NSTableView datasource handlers
-(NSInteger) numberOfRowsInTableView:(NSTableView*) tableView;

-(id)                 tableView: (NSTableView*) tableView
      objectValueForTableColumn: (NSTableColumn*) tableColumn
                            row: (NSInteger) index;

//NSTabView delegate handler we care about
-(void)          tabView: (NSTabView*) tabView
   willSelectTabViewItem: (NSTabViewItem*) tabViewItem;
@end



//
// This NSView subclass shows a preview of a
// page, including the margins.  Implementation
// is at the end of the file.
//
@interface GSPageLayoutMiniPageView : NSView
{
  NSSize _paperSize;
  NSRect _marginsRect;
  BOOL   _drawsMargins;
}

-(void) setPaperSize: (NSSize) paperSize;

-(void) setMarginsRect: (NSRect) marginsRect;

-(void) setDrawsMargins: (BOOL) drawsMargins;

@end



/**
<unit>
  <heading>NSPageLayout</heading>
  <p>
  NSPageLayout provides a panel that allows the user to specify certain
  information about the printer and how data is formatted for printing.
  This includes information about the paper size and orientation.

  Typically you would create a page layout panel with the 
  +pageLayout class method. However, the best
  way to use the panel is to have the application
  call the runPageLayout: method in the NSApplication object
  which would both create a standard NSPageLayout panel and display it
  in a modal loop. This method would be sent up the responder chain if
  the user clicked on a Page Layout menu item.
  </p>
</unit>
*/
@implementation NSPageLayout

//
// Class methods
//
/** Load the appropriate bundle for the PageLayout 
*/

+ (id) allocWithZone: (NSZone*) zone
{
  Class principalClass;
  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return nil;

  return [[principalClass pageLayoutClass] allocWithZone: zone];
}

/** Creates ( if needed )  and returns a shared instance of the
    NSPageLayout panel.
 */
+ (NSPageLayout *)pageLayout
{
  if (shared_instance == nil)
    {
      GSPageLayoutController *controller;
      controller = [[GSPageLayoutController alloc] init];
      shared_instance = [controller panel];
    }
  return shared_instance;
}


//
// Running the Panel 
//
/** <p>Display the Page Layout panel in a modal loop. Saves any aquired 
   information in the shared NSPrintInfo object. Returns NSCancelButton 
   if the user clicks the Cancel button or NSOKButton otherwise.</p>
   <p>See Also: -runModalWithPrintInfo:</p>
*/
- (NSInteger)runModal
{
  return [self runModalWithPrintInfo: [NSPrintInfo sharedPrintInfo]];
}

/** <p>Displays the Page Layout panel in a modal loop. Saves any aquired 
   information in the indicated NSPrintInfo object. Returns NSCancelButton 
   if the user clicks the Cancel button or NSOKButton otherwise.</p>
   <p>See Also: -runModal</p>
*/
- (NSInteger)runModalWithPrintInfo:(NSPrintInfo *)printInfo
{
  NSInteger result;
  
  [_controller setPrintInfo: printInfo];
  [_controller readPrintInfo];

  result = [NSApp runModalForWindow: self];
  [self orderOut: self];
  return result;
}

- (void)beginSheetWithPrintInfo:(NSPrintInfo *)printInfo
                 modalForWindow:(NSWindow *)docWindow
                       delegate:(id)delegate
                 didEndSelector:(SEL)didEndSelector
                    contextInfo:(void *)contextInfo
{
  [_controller setPrintInfo: printInfo];
  [_controller readPrintInfo];

  [NSApp beginSheet: self
     modalForWindow: docWindow
      modalDelegate: delegate
     didEndSelector: didEndSelector
        contextInfo: contextInfo];

  [self orderOut: self];
}

//
// Customizing the Panel 
//
/**<p> Returns the accessory view for the page layout panel.</p>
   <p>See Also: -setAccessoryView:</p>
 */
- (NSView *)accessoryView
{
  return [_controller accessoryView];
}

/** <p>Sets the accessory view for the page layout panel</p>
    <p>See Also: -accessoryView</p>
 */
- (void)setAccessoryView:(NSView *)aView
{
  [_controller setAccessoryView: aView];
}


/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)convertOldFactor:(float *)old
               newFactor:(float *)newFactor
{
  NSLog(@"[NSPageLayout -convertOldFactor:newFactor:] method depreciated");
}


/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedButton:(id)sender
{
  NSLog(@"[NSPageLayout -pickedButton:] method depreciated");
}

/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedOrientation:(id)sender
{
  NSLog(@"[NSPageLayout -pickedOrientation:] method depreciated");
}

/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedPaperSize:(id)sender
{
  NSLog(@"[NSPageLayout -pickedPaperSize:] method depreciated");
}

/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedLayout:(id)sender
{
  NSLog(@"[NSPageLayout -pickedLayout:] method depreciated");
}

/** This method has been depreciated. It doesn't do anything useful.
*/
- (void)pickedUnits:(id)sender
{
  NSLog(@"[NSPageLayout -pickedUnits:] method depreciated");
}

//
// Communicating with the NSPrintInfo Object 
//
/** Return the NSPrintInfo object that the receiver stores layout information
    into.
*/
- (NSPrintInfo *)printInfo
{
  return [_controller printInfo];
}

/** <p>Updates the receiver panel with information from its NSPrintInfo object.
    </p><p>See Also: -writePrintInfo</p>
 */
- (void)readPrintInfo
{
  [_controller readPrintInfo];
}

/** <p>Writes any layout information set by the user to the receiver's
    NSPrintInfo object</p><p>See Also: -readPrintInfo</p>
*/
- (void)writePrintInfo
{
  [_controller writePrintInfo];
}

@end



//
// Controller for the PageLayout Panel
//
@implementation GSPageLayoutController
- (id) init
{
  NSImage *image;
  NSNumberFormatter *sizeFormatter;
  NSNumberFormatter *scaleFormatter;

  if ((self = [super init]) == nil)
    return nil;
  
  if ([NSBundle loadNibNamed: @"GSPageLayout" owner: self] == NO)
    {
      NSRunAlertPanel(@"Error", @"Could not load page layout panel resource", 
                      @"OK", NULL, NULL);
      return nil;
    }

  //
  //find out what the user's preffered measurements are
  //and what the scaleFactor will be
  //
  [self determineMeasurements];
    
  //Put the applications icon image in the panel
  image = [[NSApplication sharedApplication] applicationIconImage];
  [applicationImageButton setImage: image];
  
  //
  //Put NSNumberFormatters in ALL the NSTextFields
  //since GORMS is not to good at this yet.
  //I look forward to the day when this code can
  //be removed.
  
  { //for the Scale field
    scaleFormatter = AUTORELEASE([[NSNumberFormatter alloc] init]);
    [scaleFormatter setAllowsFloats: NO];
    [scaleFormatter setMinimum: 
                  [NSDecimalNumber decimalNumberWithString: @"1.0"]];
		     
    [scaleFormatter setMaximum: 
                  [NSDecimalNumber decimalNumberWithString: @"1000.0"]];
  
    [scaleFormatter setHasThousandSeparators: NO];
    [scaleTextField setFormatter: scaleFormatter];  
  }
  
  { //For the the width and height of custom papers ONLY.
    //The NSFormatters for the margins are made on the fly
    //because they change based upon the values of the width
    //and height.  Makes sense, right?
    sizeFormatter = AUTORELEASE([[NSNumberFormatter alloc] init]);
    [sizeFormatter setAllowsFloats: YES];
    [sizeFormatter setMinimum: 
                 [NSDecimalNumber decimalNumberWithString: @"0.00001"]];
  
    [sizeFormatter setHasThousandSeparators: NO];
		     
    [customPaperWidthTextField  setFormatter: sizeFormatter];
    [customPaperHeightTextField setFormatter: sizeFormatter];
  }
  
  //
  // Load the custom paper sizes
  //
  {
    NSUserDefaults *defaults;
    NSDictionary *globalDomain;
    
    customPapersNeedSaving = NO;
    
    defaults = [NSUserDefaults standardUserDefaults];
    globalDomain = [defaults persistentDomainForName: NSGlobalDomain];
    
    customPapers = [globalDomain objectForKey: @"GSPageLayoutCustomPaperSizes"];
    
    if (customPapers)
      {
        customPapers = [customPapers mutableCopy];
      }
    else
      {
        customPapers = RETAIN([NSMutableDictionary dictionary]);
      }
  } 

  //
  //Set the proper measurement strings on the "Custom Paper Size" tab
  //There are six NSTextFields with ids ranging from 100 to 105.  Set them.
  //the string used, measurementString, was determined by the call made
  //to [self determineMeasurements] earlier in -init.  measurementString is
  //an ivar.
  {
    int n;
    for (n = 100; n <= 105; n++)
      {
        NSTextField *textField;
        textField = [[[tabView tabViewItemAtIndex:1] view] viewWithTag: n];
        [textField setStringValue: measurementString];
      }
    //doing the above puts selects the 2nd tab, we don't want that.
    [tabView selectFirstTabViewItem: self];
  }

  //assign the tab views items
  {
    attributesTabViewItem = [tabView tabViewItemAtIndex: 0];
    customTabViewItem = [tabView tabViewItemAtIndex: 1];
    summaryTabViewItem = [tabView tabViewItemAtIndex: 2];
  }


  return self;
}

-(void) dealloc
{
  RELEASE(customPapers);
  RELEASE(measurementString);
  [super dealloc];
}


- (NSPageLayout*) panel
{
  return (NSPageLayout*)panel;
}



- (void) okButtonClicked: (id)sender
{
  [self writePrintInfo];
  [NSApp stopModalWithCode: NSOKButton];
}

- (void) cancelButtonClicked: (id)sender
{
  [NSApp stopModalWithCode: NSCancelButton];
}

-(void) printerPopUpClicked: (id)sender
{
  NSPrinter *printer;
  NSString *prevPaperName;
  NSArray *newPaperNames;
  
  printer = [NSPrinter printerWithName: [printerPopUp titleOfSelectedItem]];

  /* Setup standardPaperSizePopUp for the new printer */
  prevPaperName = [standardPaperSizePopUp titleOfSelectedItem];

  newPaperNames = [printer stringListForKey:@"PageSize" 
                                    inTable:@"PPD"];

  [standardPaperSizePopUp removeAllItems];
  [standardPaperSizePopUp addItemsWithTitles: newPaperNames];

  //be nice and try to select the previous selection
  [standardPaperSizePopUp selectItemWithTitle: prevPaperName];

  [self processAttributes];
}

-(void) paperRadioMatrixClicked: (id) sender
{
  if ([sender selectedRow] == 0) //Standard Paper Sizes
    {
      [customPaperSizePopUp   setEnabled: NO];
      [standardPaperSizePopUp setEnabled: YES];
    }
  else  //Custom Paper Sizes
    {
      [standardPaperSizePopUp setEnabled: NO];
      [customPaperSizePopUp   setEnabled: YES];
    }
  [self processAttributes];
}

-(void) paperPopUpClicked: (id) sender
{
  [self processAttributes];
}

-(void) paperOrientationMatrixClicked: (id) sender
{
  [self processAttributes];
}


-(void) customPaperButtonsClicked: (id) sender
{
}

//determine the measurement string and factor value to use
-(void) determineMeasurements
{
  NSUserDefaults *defaults;
  NSString *string;
  
  defaults = [NSUserDefaults standardUserDefaults];
  string = [defaults objectForKey: @"NSMeasurementUnit"];
  NSDebugLLog(@"NSPageLayout", @"NSMeasurementUnit is %@", string);

  if (string == nil) //default to cm, most of the world is metric...
    {
      measurementString = @"cm";
      factorValue = 2.54/72.0;
    }
  else
    {
      if ([string caseInsensitiveCompare: @"CENTIMETERS"] == NSOrderedSame)
        {
          measurementString = @"cm";
          factorValue = 2.54/72.0;
        }
      else if ([string caseInsensitiveCompare: @"INCHES"] == NSOrderedSame)
        {
          measurementString = @"in";
          factorValue = 1.0/72.0;
        }
      else if ([string caseInsensitiveCompare: @"POINTS"] == NSOrderedSame)
        {
          measurementString = @"pt";
          factorValue = 1.0;
        }
      else if ([string caseInsensitiveCompare: @"PICAS"] == NSOrderedSame)
        {
          measurementString = @"pi";
          factorValue = 1.0/12.0;
        }
      else //default to cm, most of the world is metric...
        {
          measurementString = @"cm";
          factorValue = 2.54/72.0;
        }
   }
}


//
// Reads in the values of the controls and responds accordingly
// This updates two ivars: (NSTextField*)dimensionsTextField and
// (GSPageLayoutMiniPageView*)paperAttributesPreview.
-(void) processAttributes
{
  NSString *paperName;
  NSPrinter *printer;
  NSSize paperSize;  
  
  //Get the printer
  printer = [NSPrinter printerWithName: [printerPopUp titleOfSelectedItem]];

  //Get the paper name and size
  if ([paperRadioMatrix selectedRow] == 0) //Standard Papers
    {
      paperName = [standardPaperSizePopUp titleOfSelectedItem];
      paperSize = [printer pageSizeForPaper: paperName];
    }
  else //Custom Papers
   {
     NSMutableDictionary *customPaperDict;
     paperName = [customPaperSizePopUp titleOfSelectedItem];
     customPaperDict = [customPapers objectForKey: paperName];
     paperSize = [[customPaperDict objectForKey: @"PaperSize"] sizeValue];
   }

  
  //check if the user selected landscape mode, if so, switch out the 
  //width and height
  if ([paperOrientationMatrix selectedColumn] > 0)
    {
      double temp      = paperSize.width;
      paperSize.width  = paperSize.height;
      paperSize.height = temp;
    }

  //construct the string for the dimensions NSTextField and set it
  {
    NSString *dimensionsString;
    dimensionsString = [NSString stringWithFormat: @"%.2f %@ x %.2f %@",
                        paperSize.width * factorValue, 
                        measurementString,
                        paperSize.height * factorValue,
                        measurementString];

    [dimensionsTextField setStringValue: dimensionsString];
  }

  //tell the preview view what the new paper size is
  [paperAttributesPreview setPaperSize: paperSize]; 
}


-(NSPrintInfo*) printInfo
{
  return _printInfo;
}

-(void) setPrintInfo:(NSPrintInfo*)printInfo
{
  ASSIGN(_printInfo, printInfo);
}

-(NSView*) accessoryView
{
  return _accessoryView;
}

-(void) setAccessoryView:(NSView*)accessoryView
{
  ASSIGN(_accessoryView, accessoryView);
}


//
// Syncs the interface with the current printers (if any)
// and its papers.  Also makes sure that the custom papers
// pop up is up to date.  This method is called by 
// readPrintInfo.
-(void) syncInterface
{
  NSArray *printerNames;
  id radioButton;

  [printerPopUp removeAllItems];
  [standardPaperSizePopUp removeAllItems];
  
  //Fill in the printers
  printerNames = [NSPrinter printerNames];
  if ([printerNames count] == 0)  //NO PRINTERS
    {
      [printerPopUp addItemWithTitle: _(@"(none)")];
      [printerPopUp setEnabled: NO];
      [standardPaperSizePopUp addItemWithTitle: _(@"(none)")];
      [standardPaperSizePopUp setEnabled: NO];
      radioButton = [paperRadioMatrix cellAtRow: 0
                                         column: 0];
      [radioButton setEnabled: NO];
      
    }
  else //THERE ARE PRINTERS
    {
      NSPrinter *printer;
      NSArray *paperNames;

      printer = [_printInfo printer];
      [printerPopUp addItemsWithTitles: printerNames];
      [printerPopUp setEnabled: YES];
      [printerPopUp selectItemWithTitle: [printer name]];

      //fill in the standardPaperSizePopUp based upon what the printer supports
      paperNames = [printer stringListForKey:@"PageSize" 
                                     inTable:@"PPD"];
 
      [standardPaperSizePopUp addItemsWithTitles: paperNames];
      [standardPaperSizePopUp setEnabled: YES];
      radioButton = [paperRadioMatrix cellAtRow: 0
                                         column: 0];

      [radioButton setEnabled: YES];
    }
}

- (void)readPrintInfo
{
  NSString *paperName;
  NSNumber *scaleNumber; 

  NSDebugLLog(@"NSPageLayout", @"readPrintInfo: %@", [[_printInfo dictionary] description]);

  [self syncInterface];
    
  //set the paper.  Try to set the custom paper first.
  paperName = [_printInfo paperName];

  if (([customPaperSizePopUp isEnabled] == YES) &&
     ([customPaperSizePopUp indexOfItemWithTitle: paperName] != -1))
    {
      [paperRadioMatrix selectCellAtRow: 1
                                 column: 0];

      [customPaperSizePopUp selectItemWithTitle: paperName]; 
    }
  else if ([standardPaperSizePopUp isEnabled] == YES)
    {
      [paperRadioMatrix selectCellAtRow: 0
                                 column: 0];

      [standardPaperSizePopUp selectItemWithTitle: paperName];
    }

  //set the orientation
  if ([_printInfo orientation] == NSPortraitOrientation)
    {
      [paperOrientationMatrix selectCellAtRow: 0 
                                       column: 0];
    }
  else
    {
      [paperOrientationMatrix selectCellAtRow: 0 
                                       column: 1];
    }

  //set the scaling
  scaleNumber = [[_printInfo dictionary] objectForKey: NSPrintScalingFactor];

  if (scaleNumber == nil)
    {
      NSLog(@"NSPrintScalingFactor was nil in NSPrintInfo");
      scaleNumber = [NSNumber numberWithDouble: 100.0];
    }
  else
    {
      scaleNumber = [NSNumber numberWithDouble: (100.0*[scaleNumber doubleValue])];
    }

  [scaleTextField setObjectValue: scaleNumber];
  
  [self processAttributes];
}


- (void)writePrintInfo
{
  NSPrinter *printer;
  NSString *paperName;
  NSNumber *scaleNumber;

  //Write printer object
  if ([printerPopUp isEnabled] == NO) //NO PRINTERS
    {
      printer = nil;
    }
  else //HAS PRINTERS
    {
      printer = [NSPrinter printerWithName: [printerPopUp titleOfSelectedItem]];
    }
  [_printInfo setPrinter: printer];

  //write paper name
  if (([paperRadioMatrix selectedRow] == 0) &&
     ([standardPaperSizePopUp isEnabled] == YES)) //standard paper sizes
    {
      paperName = [standardPaperSizePopUp titleOfSelectedItem];
      [_printInfo setPaperName: paperName];
    }
  else if (([paperRadioMatrix selectedRow] == 1) &&
     ([customPaperSizePopUp isEnabled] == YES)) //custom paper size
    {
     NSDictionary *customPaper;
     NSNumber *number;
     NSSize size;

     paperName = [customPaperSizePopUp titleOfSelectedItem];
     [_printInfo setPaperName: paperName];

     customPaper = [customPapers objectForKey: paperName];

     number = [customPaper objectForKey: @"TopMargin"];
     [_printInfo setTopMargin: [number doubleValue]];

     number = [customPaper objectForKey: @"BottomMargin"];
     [_printInfo setBottomMargin: [number doubleValue]];

     number = [customPaper objectForKey: @"LeftMargin"];
     [_printInfo setLeftMargin: [number doubleValue]];

     number = [customPaper objectForKey: @"RightMargin"];
     [_printInfo setRightMargin: [number doubleValue]];

     size = [[customPaper objectForKey: @"PaperSize"] sizeValue];
     [_printInfo setPaperSize: size];
    }
  else //NO PAPER CAN BE SET
    {
      [_printInfo setPaperName: nil];
    }
  
  //Write orientation
  if ([paperOrientationMatrix selectedColumn] == 0)
    {
      [_printInfo setOrientation: NSPortraitOrientation];
    }
  else
    {
      [_printInfo setOrientation: NSLandscapeOrientation];
    }

  //Write scaling and don't forget to convert from percent
  scaleNumber = [NSNumber numberWithDouble: [scaleTextField doubleValue]/100.0];
  [[_printInfo dictionary] setObject: scaleNumber
                              forKey: NSPrintScalingFactor];

  NSDebugLLog(@"NSPageLayout", @"writePrintInfo: %@", [[_printInfo dictionary] description]);
}


//NSTextField delegate handlers
-(void) textDidBeginEditing:(NSNotification*) notification
{
  NSDebugLLog(@"NSPageLayout", @"textDidBeginEditing: %@", [notification description]);
}


-(void) textDidEndEditing:(NSNotification*) notification
{
  NSDebugLLog(@"NSPageLayout", @"textDidEndEditing: %@", [notification description]);
}


-(void) textDidChange:(NSNotification*) notification
{
  NSDebugLLog(@"NSPageLayout", @"textDidChange: %@", [notification description]);
}


-(void) controlTextDidChange:(NSNotification*) notification
{
  NSDebugLLog(@"NSPageLayout", @"controlTextDidChange: %@", [notification description]);
}



//NSTableView datasource handlers
-(NSInteger) numberOfRowsInTableView:(NSTableView*) tableView
{
  if (tableView == customPaperTableView)
    {
      return [customPapers count];
    }
  else  //summaryTableView
    {
      return 8;
    }
}


-(id)                 tableView: (NSTableView*) tableView
      objectValueForTableColumn: (NSTableColumn*) tableColumn
                            row: (NSInteger) index
{
  if (tableView == customPaperTableView)
    {
      return [[customPapers allKeys] objectAtIndex: index];
    }
  else  //summaryTableView
    {
      if (tableColumn == summarySettingColumn)
        {
          switch(index)
            {
              case 0:  return _(@"Name");
              case 1:  return _(@"Dimensions");
              case 2:  return _(@"Orientation");
              case 3:  return _(@"Scale");
              case 4:  return _(@"Top Margin");
              case 5:  return _(@"Bottom Margin");
              case 6:  return _(@"Left Margin");
              case 7:  return _(@"Right Margin");
              default: return _(@"Unknown");
            }
        }
      else //The value column
        {
          //These vars are used to calculate the margins
          NSString *printerName;
          NSPrinter *printer;
          NSString *paperName;
          double topMargin, bottomMargin, leftMargin, rightMargin;

          printerName = [printerPopUp titleOfSelectedItem];
          printer = [NSPrinter printerWithName: printerName];

          if ([paperRadioMatrix selectedRow] == 0) //standard papers
            {
              NSRect imageRect;
              NSSize paperSize;

              paperName = [standardPaperSizePopUp titleOfSelectedItem];
              paperSize = [printer pageSizeForPaper: paperName];
              imageRect = [printer imageRectForPaper: paperName];
              topMargin    = (paperSize.height - NSMaxY(imageRect)) * factorValue;
              bottomMargin = imageRect.origin.x * factorValue;
              leftMargin   = imageRect.origin.y * factorValue;
              rightMargin  = (paperSize.width - NSMaxX(imageRect)) * factorValue;
            }
          else  //Custom Papers
            {
              paperName = [customPaperSizePopUp titleOfSelectedItem];

              topMargin    = [[customPapers objectForKey: @"TopMargin"] 
                               doubleValue];
              bottomMargin = [[customPapers objectForKey: @"BottomMargin"]
                               doubleValue];
              leftMargin   = [[customPapers objectForKey: @"LeftMargin"]
                               doubleValue];
              rightMargin  = [[customPapers objectForKey: @"RightMargin"]
                               doubleValue];
            }
          switch(index)
            {    
              case 0:  
                return paperName;
              case 1:
                return [dimensionsTextField stringValue];
              case 2:
                if ([paperOrientationMatrix selectedColumn] == 0)
                  {
                    return _(@"Portrait");
                  }
                else
                  {
                    return _(@"Landscape") ;
                  }
              case 3:
                return [NSString stringWithFormat: @"%@%%",
                        [scaleTextField stringValue]];
              case 4:  
                return [NSString stringWithFormat: @"%.2f %@", topMargin,
                        measurementString];
              case 5:
                return [NSString stringWithFormat: @"%.2f %@", bottomMargin,
                        measurementString];
              case 6:
                return [NSString stringWithFormat: @"%.2f %@", leftMargin,
                        measurementString];
              case 7:
                return [NSString stringWithFormat: @"%.2f %@", rightMargin,
                        measurementString];
              default: 
                return _(@"Unknown");
            }
        }
    }
}

//NSTabView delegate handler we care about
-(void)          tabView: (NSTabView*) tabView
   willSelectTabViewItem: (NSTabViewItem*) tabViewItem
{
  if (tabViewItem == summaryTabViewItem)
    {
      [summaryTableView sizeToFit];
    }
}

@end






//
// Show the preview of the page's dimensions
//
@implementation GSPageLayoutMiniPageView
-(void) setPaperSize: (NSSize) paperSize
{
  _paperSize = paperSize;
  [self setNeedsDisplay: YES];
}

-(void) setMarginsRect: (NSRect) marginsRect
{
  _marginsRect = marginsRect;
  [self setNeedsDisplay: YES];
}

-(void) setDrawsMargins: (BOOL) drawsMargins
{
  _drawsMargins = drawsMargins;
  [self setNeedsDisplay: YES];
}


- (void) drawRect: (NSRect)rect
{
  NSRect bounds;
  NSRect paper;  //the size on the screen
  NSRect shadow; 
  double ratio;
  double width, height;
  NSColor *shadowColor;
  
  bounds = [self bounds];
  
  //
  //Figure out if we we should scale according to the
  //the width or the height
  //
  if (_paperSize.width >= _paperSize.height)
    {
      ratio = _paperSize.height/ _paperSize.width;
      width  = bounds.size.width;
      height = width * ratio;
    }
  else
    {
      ratio =  _paperSize.width / _paperSize.height;
      height = bounds.size.height;
      width  = height * ratio;
    }
  
  //make the page a bit smaller
  width  *= 0.95;
  height *= 0.95;
  
  paper.origin.x = (bounds.size.width  - width)  / 2;
  paper.origin.y = (bounds.size.height - height) / 2;
  paper.size.width  = width;
  paper.size.height = height;
  
  shadow = paper;
  if ([self isFlipped] == NO)
    shadow.origin.y -= 2;
  else
    shadow.origin.y += 2;
      
  shadow.origin.x += 2;
  
  
  //first draw the shadow
  shadowColor = [[NSColor windowBackgroundColor] shadowWithLevel: 0.5];
  [shadowColor set];
  [NSBezierPath fillRect: shadow];
  
  //now draw the paper
  [[NSColor whiteColor] set];
  [NSBezierPath fillRect: paper];
  [[NSColor blackColor] set];
  [NSBezierPath strokeRect: paper];

  //Draw the margins?
  if (_drawsMargins == YES)
    {
      NSRect margins;
      double scale;

      scale = paper.size.width / _paperSize.width;

      margins.size.height = (_marginsRect.size.height * scale);

      margins.size.width  = (_marginsRect.size.width * scale);
 
      margins.origin.x = paper.origin.x + (_marginsRect.origin.x * scale);
      
      margins.origin.y = paper.origin.y + (_marginsRect.origin.y * scale);

      [[NSColor redColor] set];
      [NSBezierPath strokeRect: margins];
    }
}

@end

