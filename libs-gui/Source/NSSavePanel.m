/** <title>NSSavePanel</title>

   <abstract>Standard panel for saving files</abstract>

   Copyright (C) 1999, 2000 Free Software Foundation, Inc.

   Author: Jonathan Gapen <jagapen@smithlab.chem.wisc.edu>
   Date: 1999

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: 1999

   Author:  Mirko Viviani <mirko.viviani@rccr.cremona.it>
   Date: September 2000

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

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSURL.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSBox.h"
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSForm.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSImageView.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSDragging.h"
#import "AppKit/NSSavePanel.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSWindowController.h"
#import "AppKit/NSWorkspace.h"

#import "GSGuiPrivate.h"
#import "GNUstepGUI/GSTheme.h"

#define _SAVE_PANEL_X_PAD	5
#define _SAVE_PANEL_Y_PAD	4

static NSSavePanel *_gs_gui_save_panel = nil;
static NSFileManager *_fm = nil;

static BOOL _gs_display_reading_progress = NO;

static NSString	*
pathToColumn(NSBrowser *browser, int column)
{
#if	defined(__MINGW32__)
  if (column == 0)
    return @"/";
  else if (column == 1)
    return [[[browser pathToColumn: column] substringFromIndex: 1]
      stringByAppendingString: @"/"];
  else
    return [[browser pathToColumn: column] substringFromIndex: 1];
#else
  return [browser pathToColumn: column];
#endif
}

static void
setPath(NSBrowser *browser, NSString *path)
{
#if	defined(__MINGW32__)
  [browser setPath: [@"/" stringByAppendingString: path]];
#else
  [browser setPath: path];
#endif
}

//
// SavePanel filename compare
//
@interface NSString (GSSavePanel)
- (NSComparisonResult)_gsSavePanelCompare:(NSString *)other;
@end

//
// NSSavePanel private methods
//
@interface NSSavePanel (GSPrivateMethods)

// Methods to manage default settings
- (id) _initWithoutGModel;
- (void) _getOriginalSize;
- (void) _setDefaultDirectory;
- (void) _updateDefaultDirectory;
- (void) _resetDefaults;
- (void) _reloadBrowser;
// Methods invoked by buttons
- (void) _setHomeDirectory;
- (void) _mountMedia;
- (void) _unmountMedia;
- (void) _selectTextInColumn: (int)column;
- (void) _selectCellName: (NSString *)title;
- (void) _setFileName: (NSString *)name;
- (void) _setupForDirectory: (NSString *)path file: (NSString *)name;
- (BOOL) _shouldShowExtension: (NSString *)extension;
- (void) _windowResized: (NSNotification*)n;
- (NSComparisonResult) _compareFilename: (NSString *)n1 with: (NSString *)n2;
@end /* NSSavePanel (PrivateMethods) */

@implementation NSSavePanel (PrivateMethods)

- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>)sender
{
  NSPasteboard *pb;

  pb = [sender draggingPasteboard];
  if ([[pb types] indexOfObject: NSFilenamesPboardType] == NSNotFound)
    {
      return NSDragOperationNone;
    }

  return NSDragOperationEvery;
}
    
- (BOOL) performDragOperation: (id<NSDraggingInfo>)sender
{
  NSArray	*types;
  NSPasteboard	*dragPb;

  dragPb = [sender draggingPasteboard];
  types = [dragPb types];
  if ([types containsObject: NSFilenamesPboardType] == YES)
    {
      NSArray	*names = [dragPb propertyListForType: NSFilenamesPboardType];
      NSString	*file = [[names lastObject] stringByStandardizingPath];
      BOOL isDir;

      if (file && [_fm fileExistsAtPath: file isDirectory: &isDir] && isDir)
        {
	  [self setDirectory: file];
	}
      else
        {
	  NSString *path = [file stringByDeletingLastPathComponent];
	  NSString *filename = [file lastPathComponent];
	  
	  [self _setupForDirectory: path file: filename];
	}

      return YES;
    }
  return NO;
}

- (BOOL) prepareForDragOperation: (id<NSDraggingInfo>)sender
{
  return YES;
}

-(id) _initWithoutGModel
{
  NSBox *bar;
  NSButton *button;
  NSImage *image;
  NSImageView *imageView;
  NSRect r;
  id lastKeyView;

  // Track window resizing so we can change number of browser columns.
  [[NSNotificationCenter defaultCenter] addObserver: self
    selector: @selector(_windowResized:)
    name: NSWindowDidResizeNotification
    object: self];

  //
  // WARNING: We create the panel sized (308, 317), which is the 
  // minimum size we want it to have.  Then, we resize it at the 
  // comfortable size of (384, 426).
  //
  self = [super initWithContentRect: NSMakeRect (100, 100, 308, 317)
                          styleMask: (NSTitledWindowMask | NSResizableWindowMask) 
                            backing: 2 defer: YES];
  if (nil == self)
    return nil;

  [self setMinSize: [self frame].size];

  r = NSMakeRect (0, 0, 308, 317);
  [[self contentView] setBounds: r];
  
  r = NSMakeRect (0, 64, 308, 245);
  _topView = [[NSView alloc] initWithFrame: r]; 
  [_topView setBounds:  r];
  [_topView setAutoresizingMask: NSViewWidthSizable|NSViewHeightSizable];
  [_topView setAutoresizesSubviews: YES];
  [[self contentView] addSubview: _topView];
  [_topView release];
  
  r = NSMakeRect (0, 0, 308, 64);
  _bottomView = [[NSView alloc] initWithFrame: r];
  [_bottomView setBounds:  r];
  [_bottomView setAutoresizingMask: NSViewWidthSizable|NSViewMaxYMargin];
  [_bottomView setAutoresizesSubviews: YES];
  [[self contentView] addSubview: _bottomView];
  [_bottomView release];

  r = NSMakeRect (8, 68, 292, 177);
  _browser = [[NSBrowser alloc] initWithFrame: r]; 
  lastKeyView = _browser;
  [_browser setDelegate: self];
  [_browser setHasHorizontalScroller: YES];
  [_browser setAllowsMultipleSelection: NO];
  [_browser setAutoresizingMask: NSViewWidthSizable|NSViewHeightSizable];
  [_browser setTag: NSFileHandlingPanelBrowser];
  [_browser setAction: @selector(_selectText:)];
  [_browser setDoubleAction: @selector(_doubleClick:)];
  [_browser setTarget: self];
  [_browser setMinColumnWidth: 140];
  [_topView addSubview: _browser];
  [_browser release];
  
  _showsHiddenFilesMenu = [[NSMenu alloc] initWithTitle: @""];
  [_showsHiddenFilesMenu insertItemWithTitle: _(@"Show Hidden Files") action:@selector(_toggleShowsHiddenFiles:) keyEquivalent:@"" atIndex:0];
  [[_showsHiddenFilesMenu itemAtIndex: 0] setTarget: self];
  [[_showsHiddenFilesMenu itemAtIndex: 0] setState: [self showsHiddenFiles]];
  [_browser setMenu: _showsHiddenFilesMenu];
  [_showsHiddenFilesMenu release];


  r = NSMakeRect (8, 39, 291, 21);
  _form = [NSForm new];
  [_form addEntry: _(@"Name:")];
  [_form setFrame: r];
  // Force the size we want
  [_form setCellSize: NSMakeSize (291, 21)];
  [_form setEntryWidth: 291];
  [_form setInterlineSpacing: 0];
  [_form setAutosizesCells: YES];
  [_form setDrawsBackground: NO];
  [_form setTag: NSFileHandlingPanelForm];
  [_form setAutoresizingMask: NSViewWidthSizable];
  [_form setDelegate: self];
  [_bottomView addSubview: _form];
  [lastKeyView setNextKeyView: _form];
  lastKeyView = _form;
  [_form release];

  r = NSMakeRect (43, 6, 27, 27);
  button = [[NSButton alloc] initWithFrame: r];
  [button setBordered: YES];
  image = [NSImage imageNamed: @"common_Home"];
  [button setImage: image];
  [button setImagePosition: NSImageOnly]; 
  [button setTarget: self];
  [button setAction: @selector(_setHomeDirectory)];
  [button setAutoresizingMask: NSViewMinXMargin];
  [button setTag: NSFileHandlingPanelHomeButton];
  [button setToolTip:_(@"Home")];
  [_bottomView addSubview: button];
  [lastKeyView setNextKeyView: button];
  lastKeyView = button;
  [button release];
  
  r = NSMakeRect (78, 6, 27, 27);
  button = [[NSButton alloc] initWithFrame: r];
  [button setBordered: YES];
  image = [NSImage imageNamed: @"common_Mount"]; 
  [button setImage: image]; 
  [button setImagePosition: NSImageOnly]; 
  [button setTarget: self];
  [button setAction: @selector(_mountMedia)];
  [button setAutoresizingMask: NSViewMinXMargin];
  [button setTag: NSFileHandlingPanelDiskButton];
  [button setToolTip:_(@"Mount")];
  [_bottomView addSubview: button];
  [lastKeyView setNextKeyView: button];
  lastKeyView = button;
  [button release];

  r = NSMakeRect (112, 6, 27, 27);
  button = [[NSButton alloc] initWithFrame: r];
  [button setBordered: YES];
  image = [NSImage imageNamed: @"common_Unmount"]; 
  [button setImage: image];
  [button setImagePosition: NSImageOnly]; 
  [button setTarget: self];
  [button setAction: @selector(_unmountMedia)];
  [button setAutoresizingMask: NSViewMinXMargin];
  [button setTag: NSFileHandlingPanelDiskEjectButton];
  [button setToolTip:_(@"Unmount")];
  [_bottomView addSubview: button];
  [lastKeyView setNextKeyView: button];
  lastKeyView = button;
  [button release];
  
  r = NSMakeRect (148, 6, 71, 27);
  button = [[NSButton alloc] initWithFrame: r]; 
  [button setBordered: YES];
  [button setTitle:  _(@"Cancel")];
  [button setImagePosition: NSNoImage]; 
  [button setTarget: self];
  [button setAction: @selector(cancel:)];
  [button setAutoresizingMask: NSViewMinXMargin];
  [button setTag: NSFileHandlingPanelCancelButton];
  [button setKeyEquivalent: @"\e"];
  [button setKeyEquivalentModifierMask: 0];
  [_bottomView addSubview: button];
  [lastKeyView setNextKeyView: button];
  lastKeyView = button;
  [button release];
  
  r = NSMakeRect (228, 6, 71, 27);
  _okButton = [[NSButton alloc] initWithFrame: r]; 
  [_okButton setBordered: YES];
  [_okButton setTitle:  _(@"OK")];
  [_okButton setImagePosition: NSImageRight]; 
  [_okButton setImage: [NSImage imageNamed: @"common_ret"]];
  [_okButton setAlternateImage: [NSImage imageNamed: @"common_retH"]];
  [_okButton setTarget: self];
  [_okButton setAction: @selector(ok:)];
  [_okButton setEnabled: NO];
  [_okButton setAutoresizingMask: NSViewMinXMargin];
  [_okButton setTag: NSFileHandlingPanelOKButton];
  [_bottomView addSubview: _okButton];
  [lastKeyView setNextKeyView: _okButton];
  [_okButton setNextKeyView: _browser];
  [self setDefaultButtonCell: [_okButton cell]];
  [_okButton release];

  r = NSMakeRect (8, 261, 48, 48);
  image = [[NSApplication sharedApplication] applicationIconImage];
  imageView = [[NSImageView alloc] initWithFrame: r];
  [imageView setAutoresizingMask: NSViewMinYMargin];
  [imageView setImage:image];
  [imageView setTag: NSFileHandlingPanelImageButton];
  [_topView addSubview: imageView];
  [imageView release];

  r = NSMakeRect (67, 276, 200, 14);
  _titleField = [[NSTextField alloc] initWithFrame: r]; 
  [_titleField setSelectable: NO];
  [_titleField setEditable: NO];
  [_titleField setDrawsBackground: NO];
  [_titleField setBezeled: NO];
  [_titleField setBordered: NO];
  [_titleField setFont: [NSFont messageFontOfSize: 18]];
  [_titleField setAutoresizingMask: NSViewMinYMargin];
  [_titleField setTag: NSFileHandlingPanelTitleField];
  [_topView addSubview: _titleField];
  [_titleField release];

  r = NSMakeRect (0, 252, 308, 2);
  bar = [[NSBox alloc] initWithFrame: r]; 
  [bar setBorderType: NSGrooveBorder];
  [bar setTitlePosition: NSNoTitle];
  [bar setAutoresizingMask: NSViewWidthSizable|NSViewMinYMargin];
  [_topView addSubview: bar];
  [bar release];

  [self setContentSize: NSMakeSize (384, 426)];
  [self setInitialFirstResponder: _form];
  [super setTitle: @""];

  [self registerForDraggedTypes: [NSArray arrayWithObjects:
	NSFilenamesPboardType, nil]];

  return self;
}

- (void) _toggleShowsHiddenFiles: (id)sender
{
  NSMenuItem *menuItem = (NSMenuItem*)sender;
  [self setShowsHiddenFiles: ![menuItem state]];
}

- (void) _getOriginalSize
{
  /* Used in setMinSize: */
  _originalMinSize = [self minSize];
  /* Used in setContentSize: */
  _originalSize = [[self contentView] frame].size;
}

/* Set the current directory to a useful default value */
- (void) _setDefaultDirectory
{
  NSString *path;

  path = [[NSUserDefaults standardUserDefaults] 
	     objectForKey: @"NSDefaultOpenDirectory"];
  if (path == nil)
    {
      // FIXME: Should we use this or the home directory?
      ASSIGN(_directory, [_fm currentDirectoryPath]);
    }
  else
    {
      ASSIGN(_directory, path);
    }
}

- (void) _updateDefaultDirectory
{
  [[NSUserDefaults standardUserDefaults]
      setObject: _directory
      forKey: @"NSDefaultOpenDirectory"];
}

- (void) _resetDefaults
{
  [self _setDefaultDirectory];
  [self setPrompt: _(@"Name:")];
  [self setTitle: _(@"Save")];
  [self setAllowedFileTypes: nil];
  [self setAllowsOtherFileTypes: NO];
  [self setTreatsFilePackagesAsDirectories: NO];
  [self setDelegate: nil];
  [self setAccessoryView: nil];
}

- (void) _reloadBrowser
{
  NSString *path = [_browser path];
  [_browser loadColumnZero];
  setPath(_browser, path);
}

//
// Methods invoked by button press
//
- (void) _setHomeDirectory
{
  [self setDirectory: NSHomeDirectory()];
}

- (void) _mountMedia
{
  [[NSWorkspace sharedWorkspace] mountNewRemovableMedia];
}

- (void) _unmountMedia
{
  [[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtPath: [self directory]];
}

- (void) _selectTextInColumn: (int)column
{
  NSMatrix      *matrix;
  NSBrowserCell *selectedCell;
  BOOL           isLeaf;

  if (column == -1)
    return;

  matrix = [_browser matrixInColumn:column];
  selectedCell = [matrix selectedCell];
  isLeaf = [selectedCell isLeaf];

  if (_delegateHasSelectionDidChange) 
    {
      [_delegate panelSelectionDidChange: self];
    }

  if (isLeaf)
    {
      [[_form cellAtIndex: 0] setStringValue: [selectedCell stringValue]];
      //      [_form selectTextAtIndex:0];
      [_okButton setEnabled: YES];
    }
  else
    {
      if (_delegateHasDirectoryDidChange)
        {
	  [_delegate panel: self
	    directoryDidChange: pathToColumn(_browser, column)];
	}

      if ([[[_form cellAtIndex: 0] stringValue] length] > 0)
	{
	  [_okButton setEnabled: YES];
	  [self _selectCellName: [[_form cellAtIndex: 0] stringValue]];
	  //	  [_form selectTextAtIndex:0];
	}
      else
	[_okButton setEnabled: NO];
    }
}

- (void) _doubleClick: (id)sender
{
  [_okButton performClick: sender];
}

- (void) _selectText: (id)sender
{
  [self _selectTextInColumn:[_browser selectedColumn]];
}

- (void) _selectCellName: (NSString *)title
{
  NSString           *cellString;
  NSArray            *cells;
  NSMatrix           *matrix;
  NSComparisonResult  result;
  int                 i, titleLength, cellLength, numberOfCells;

  matrix = [_browser matrixInColumn:[_browser lastColumn]];
  if ([matrix selectedCell])
    return;

  titleLength = [title length];
  if (!titleLength)
    return;

  cells = [matrix cells];
  numberOfCells = [cells count];

  for (i = 0; i < numberOfCells; i++)
    {
      cellString = [[matrix cellAtRow:i column:0] stringValue];

      cellLength = [cellString length];
      if (cellLength != titleLength)
	continue;

      result = [self _compareFilename:cellString with:title];

      if (result == NSOrderedSame)
	{
	  [matrix selectCellAtRow:i column:0];
	  [matrix scrollCellToVisibleAtRow:i column:0];
	  [_okButton setEnabled:YES];
	  return;
	}
      else if (result == NSOrderedDescending)
	break;
    }
}

- (BOOL) _browser: (NSBrowser*)sender
selectCellWithString: (NSString*)title
	inColumn: (NSInteger)column
{
  NSMatrix *m;
  BOOL isLeaf;
  NSString *path;

  m = [sender matrixInColumn: column];
  isLeaf = [[m selectedCell] isLeaf];
  path = pathToColumn(sender, column);

  if (isLeaf)
    {
      ASSIGN (_directory, path);
      ASSIGN (_fullFileName, [path stringByAppendingPathComponent: title]);
    }
  else
    {
      ASSIGN (_directory, [path stringByAppendingPathComponent: title]);
      ASSIGN (_fullFileName, nil);
    }

  [self _selectTextInColumn:column];
  
  return YES;
}

- (void) _setFileName: (NSString *)filename
{
  [self _selectCellName: filename];
  [[_form cellAtIndex: 0] setStringValue: filename];
  [_form selectTextAtIndex: 0];
  [_form setNeedsDisplay: YES];
}

- (void) _setupForDirectory: (NSString *)path file: (NSString *)filename
{
  if (path == nil)
    {
      if (_directory == nil)
        {
	  [self _setDefaultDirectory];
	}
    }
  else
    {
      ASSIGN(_directory, path);
    }
  if (filename == nil)
    filename = @"";
  ASSIGN(_fullFileName, [_directory stringByAppendingPathComponent: filename]);
  setPath(_browser, _fullFileName);
  [self _setFileName: filename];

  [self _browser: _browser
	selectCellWithString: [[_browser selectedCell] stringValue] 
	inColumn: [_browser selectedColumn]];
}

- (BOOL) _shouldShowExtension: (NSString *)extension
{
  if (_allowedFileTypes != nil
      && [_allowedFileTypes indexOfObject: extension] == NSNotFound
      && [_allowedFileTypes indexOfObject: @""] == NSNotFound)
    return NO;

  return YES;
}

- (void) _windowResized: (NSNotification*)n
{
  [_browser setMaxVisibleColumns: [_browser frame].size.width / 140];
}

- (NSComparisonResult) _compareFilename: (NSString *)n1 with: (NSString *)n2
{
  if (_delegateHasCompareFilter)
    {
      return [_delegate panel: self
              compareFilename: n1 
                         with: n2 
                caseSensitive: YES];
    }
  else
    {
      return [n1 _gsSavePanelCompare: n2];
    }
}

@end /* NSSavePanel (PrivateMethods) */

//
// NSSavePanel methods
//
/** 
    <p>Implements a panel that allows the user to save a file.
    </p>
    <p>
    There is only one save panel per application and this panel is obtained
    by calling the +savePanel class method. From here, you should set the
    required file extension using -setRequiredFileType:
    When ready to show the panel, use the
    -runModal, or a similar method to show the panel in a modal
    session. Other methods allow you to set the initial directory and
    initially choosen file. The method will return one of NSOKButton
    or NSCancelButton depending on which button the user pressed.
    </p>
    <p>
    Use the -filename method to retrieve the name of the
    file the user choose.
    </p>
 */
@implementation NSSavePanel

+ (void) initialize
{
  if (self == [NSSavePanel class])
    {
      [self setVersion: 1];
      ASSIGN (_fm, [NSFileManager defaultManager]);

      // A GNUstep feature
      if ([[NSUserDefaults standardUserDefaults] 
	    boolForKey: @"GSSavePanelShowProgress"])
	{
	  _gs_display_reading_progress = YES;
	}
    }
}

/**<p>Creates ( if needed) and returns the shared NSSavePanel instance.</p>
 */
+ (NSSavePanel *) savePanel
{
  if (_gs_gui_save_panel == nil)
    {
      Class savePanelClass = [[GSTheme theme] savePanelClass];
      _gs_gui_save_panel = [[savePanelClass alloc] init]; 
    }

  [_gs_gui_save_panel _resetDefaults];

  return _gs_gui_save_panel;
}
//

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  TEST_RELEASE (_fullFileName);
  TEST_RELEASE (_directory);  
  TEST_RELEASE (_allowedFileTypes);

  [super dealloc];
}

// If you do a simple -init, we initialize the panel with
// the system size/mask/appearance/subviews/etc.  If you do more
// complicated initializations, you get a simple panel from super.
-(id) init
{
  self = [self _initWithoutGModel];
  if (nil == self)
    return nil;

/*
 * All these are set automatically  
  _directory = nil;
  _fullFileName = nil;
  _allowedFileTypes = nil;
  _delegate = nil;
  
  _treatsFilePackagesAsDirectories = NO;
  _delegateHasCompareFilter = NO;
  _delegateHasShowFilenameFilter = NO;
  _delegateHasValidNameFilter = NO;
  _delegateHasDirectoryDidChange = NO;
  _delegateHasSelectionDidChange = NO;
*/
  [self _getOriginalSize];
  return self;
}

/** <p>Sets an accessory view which is shown near the bottom of the
    panel. The panel is automatically expanded with enough room to
    show the extra view. You can use this extra view to customize
    various characteristics of the file selection mechanism. For instance
    you could add a popup button which allows the user to select the
    format that the file is saved in (e.g. rtf or txt). See
    also -validateVisibleColumns .</p><p>See Also: -accessoryView</p>
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

/**<p>Sets the title of the NSSavePanel to title. By default, 
  'Save' is the title string. If you adapt the NSSavePanel 
  for other uses, its title should reflect the user action 
  that brings it to the screen.</p><p>See Also: -title</p>
*/
- (void) setTitle: (NSString*)title
{
  // keep the window title in sync with the title field
  [super setTitle:title];
  [_titleField setStringValue: title];

  // TODO: Improve the following by managing 
  // vertical alignment better.
  [_titleField sizeToFit];
}

/**<p>Returns the title of the save panel </p>
 <p>See Also: -setTitle:</p>
*/
- (NSString*) title
{
  return [_titleField stringValue];
}

/**<p> Returns the prompt of the Save panel field that holds 
  the current pathname or file name. By default this 
  prompt is 'Name: '.</p><p>See Also: -prompt</p>
*/
- (void) setPrompt: (NSString*)prompt
{
  [[_form cellAtIndex: 0] setTitle: prompt];
  [_form setNeedsDisplay: YES];
}

/**<p>Returns the prompt used in the current path field.</p>
 <p>See Also: -setPrompt:</p>
*/
- (NSString*) prompt
{
  return [[_form cellAtIndex: 0] title];
}

/** <p>Returns the accesory view (if any).</p>
    <p>See Also: -setAccessoryView:</p> 
*/
- (NSView*) accessoryView
{
  return _accessoryView;
}

- (void) setNameFieldStringValue:(NSString *)value
{
  [[_form cellAtIndex: 0] setStringValue: value];
}

- (NSString *) nameFieldStringValue
{
  return [[_form cellAtIndex: 0] stringValue];
}

- (void) setNameFieldLabel: (NSString *)label
{
  [[_form cellAtIndex: 0] setTitle: label];
}

- (NSString *) nameFieldLabel
{
  return [[_form cellAtIndex: 0] title];
}

- (void) setMessage: (NSString *)message
{
  // FIXME
}

- (NSString *) message
{
  // FIXME
  return nil;
}


/** <p>Sets the current path name in the Save panel's browser. 
    The path argument must be an absolute path name.</p>
    <p>See Also: -directory</p>
 */
- (void) setDirectory: (NSString*)path
{
  NSString *standardizedPath = [path stringByStandardizingPath];
  BOOL	   isDir;

  if (standardizedPath 
      && [_fm fileExistsAtPath: standardizedPath 
	      isDirectory: &isDir] 
      && isDir)
    {
      ASSIGN (_directory, standardizedPath);
      setPath(_browser, _directory);
    }
}
/** <p>Sets the current path name in the Save panel's browser. 
    The path argument must be an absolute path name.</p>
    <p>See Also: -directory</p>
 */
- (void) setDirectoryURL: (NSURL*)url
{
  [self setDirectory: [url path]];
}

/**<p> Specifies the type, a file name extension to be appended to 
   any selected files that don't already have that extension;
   The argument type should not include the period that begins 
   the extension.  Invoke this method each time the Save panel 
   is used for another file type within the application.  If
   you do not invoke it, or set it to empty string or nil, no
   extension will be appended, indicated by an empty string
   returned from -requiredFileType.</p><p>This method is equivalent
   to calling -setAllowedFileTypes: with an array containing only
   fileType.</p><p>See Also: -requiredFileType</p>
 */
- (void) setRequiredFileType: (NSString*)fileType
{
  NSArray *fileTypes;

  if ([fileType length] == 0)
    fileTypes = nil;
  else
    fileTypes = [NSArray arrayWithObject: fileType];
  [self setAllowedFileTypes: fileTypes];
}

/**<p>Returns the required file type.  The default, indicated by an empty
 * string, is no required file type.</p><p>This method is equivalent to
 * calling -allowedFileTypes and returning the first element of the list
 * of allowed types, or the empty string if there are none.</p>
 * <p>See Also: -setRequiredFileType:</p>
 */
- (NSString*) requiredFileType
{
  if ([_allowedFileTypes count] > 0)
    return [_allowedFileTypes objectAtIndex: 0];
  else
    return @"";
}

/**<p> Specifies the allowed types, i.e., file name extensions to
   be appended to any selected files that don't already have one
   of those extensions.  The elements of the array should be strings
   that do not include the period that begins the extension.  Invoke
   this method each time the Save panel is used for another file type
   within the application.  If you do not invoke it, or set it to an
   empty array or nil, no extension will be appended, indicated by nil
   returned from -allowedFileTypes.</p><p>See Also: -allowedFileTypes</p>
 */
- (void) setAllowedFileTypes: (NSArray *)types
{
  if (types != _allowedFileTypes)
    {
      BOOL hasAllowedExtension = NO;
      NSString *filename, *extension;

      filename = [[_form cellAtIndex: 0] stringValue];
      extension = [filename pathExtension];
      if ([extension length] && [_allowedFileTypes count] &&
	  [_allowedFileTypes indexOfObject: extension] != NSNotFound)
	hasAllowedExtension = YES;

      if ([types count] == 0)
   	DESTROY(_allowedFileTypes);
      else
	ASSIGN(_allowedFileTypes, types);
      [self _reloadBrowser];

      if (hasAllowedExtension && [types count] &&
	  [types indexOfObject: extension] == NSNotFound &&
	  [types indexOfObject: @""] == NSNotFound)
        {
	  extension = [types objectAtIndex: 0];
	  filename = [filename stringByDeletingPathExtension];
	  filename = [filename stringByAppendingPathExtension: extension];
	  [[_form cellAtIndex: 0] setStringValue: filename];
	}
    }
}

/**<p>Returns an array of the allowed file types. The default, indicated by
 * nil, is any file type is allowed.</p><p>See Also: -setAllowedFileTypes:</p>
 */
- (NSArray *) allowedFileTypes
{
  return _allowedFileTypes;
}

- (void) setAllowsOtherFileTypes: (BOOL)flag
{
  _allowsOtherFileTypes = flag;
}

- (BOOL) allowsOtherFileTypes
{
  return _allowsOtherFileTypes;
}

/** Returns YES if file packages are shown as directories. The default
    is NO.  */
- (BOOL) treatsFilePackagesAsDirectories
{
  return _treatsFilePackagesAsDirectories;
}

/**<p> Sets the NSSavePanel's behavior for displaying file packages 
   (for example, MyApp.app) to the user.  If flag is YES, the 
   user is shown files and subdirectories within a file 
   package.  If NO, the NSSavePanel shows each file package as 
   a file, thereby giving no indication that it is a directory.</p>
   <p>See Also: -treatsFilePackagesAsDirectories</p>
 */
- (void) setTreatsFilePackagesAsDirectories: (BOOL)flag
{
  if (flag != _treatsFilePackagesAsDirectories)
    {
      _treatsFilePackagesAsDirectories = flag;
      [self _reloadBrowser];
    }
}

/**<p> Validates and possibly reloads the browser columns that are visible 
 * in the Save panel by causing the delegate method 
 * -panel:shouldShowFilename: to be invoked. One situation in 
 * which this method would find use is whey you want the 
 * browser to show only files with certain extensions based on the 
 * selection made in an accessory-view pop-up list.  When the 
 * user changes the selection, you would invoke this method to
 * revalidate the visible columns. </p>
 */
- (void) validateVisibleColumns
{
  [_browser validateVisibleColumns];
}

- (void) setCanCreateDirectories: (BOOL)flag
{
  _canCreateDirectories = flag;
}

- (BOOL) canCreateDirectories
{
  return _canCreateDirectories;
}

/**<p>Shows the save panel for the user. This method invokes
  -runModalForDirectory:file: with empty strings for the filename.
  Returns NSOKButton (if the user clicks the OK button) or
  NSCancelButton (if the user clicks the Cancel button).</p>
  <p>See Also: -runModalForDirectory:file:</p>
 */
- (NSInteger) runModal
{
  return [self runModalForDirectory: [self directory]
                               file: [[self filename] lastPathComponent]];
}

- (void) beginSheetModalForWindow:(NSWindow *)window
                completionHandler:(GSSavePanelCompletionHandler)handler
{
  NSInteger result = [NSApp runModalForWindow: self
                             relativeToWindow: window];
  CALL_BLOCK(handler, result);
}

- (void) beginWithCompletionHandler:(GSSavePanelCompletionHandler)handler
{
  self->_completionHandler = Block_copy(handler);
  [self makeKeyAndOrderFront: self];
}

/**<p> Initializes the panel to the directory specified by path and,
  optionally, the file specified by filename, then displays it and
  begins its modal event loop; path and filename can be empty
  strings.  The method invokes [NSApplication:-runModalForWindow:]
  method with self as the argument.  Returns NSOKButton (if the user
  clicks the OK button) or NSCancelButton (if the user clicks the
  Cancel button). If path is nil then the panel displays the last
  selected directory or as a last resort, the current working directory.</p>
  <p>See Also: -runModal</p>
 */
- (NSInteger) runModalForDirectory: (NSString*)path file: (NSString*)filename
{
  [self _setupForDirectory: path file: filename];
  if ([filename length] > 0)
    [_okButton setEnabled: YES];
  return [NSApp runModalForWindow: self];
}

- (NSInteger) runModalForDirectory: (NSString *)path
                              file: (NSString *)filename
                  relativeToWindow: (NSWindow*)window
{
  [self _setupForDirectory: path file: filename];
  if ([filename length] > 0)
    [_okButton setEnabled: YES];
  return [NSApp runModalForWindow: self
		relativeToWindow: window];
}

- (void) beginSheetForDirectory: (NSString *)path
			   file: (NSString *)filename
		 modalForWindow: (NSWindow *)docWindow
		  modalDelegate: (id)delegate
		 didEndSelector: (SEL)didEndSelector
		    contextInfo: (void *)contextInfo
{
  [self _setupForDirectory: path file: filename];
  if ([filename length] > 0)
    [_okButton setEnabled: YES];
  [NSApp beginSheet: self
	 modalForWindow: docWindow
	 modalDelegate: delegate
	 didEndSelector: didEndSelector
	 contextInfo: contextInfo];
}

/**<p> Returns the directory choosen by the user.  Do not invoke directory
   within a modal loop because the information that these methods
   fetch is updated only upon return.</p><p>See Also: -setDirectory:</p>
 */
- (NSString*) directory
{
  if (_directory)
    return AUTORELEASE([_directory copy]);
  else 
    return @"";
}

- (NSURL *) directoryURL
{
  return [NSURL fileURLWithPath: [self directory]];
}

/**<p> Returns the absolute filename choosen by the user.  Do not invoke
   filename within a modal loop because the information that these
   methods fetch is updated only upon return.</p>
 */
- (NSString*) filename
{
  NSString *fileType;

  if (_fullFileName == nil)
   return @"";

  if (_allowedFileTypes == nil ||
      [_allowedFileTypes indexOfObject: @""] != NSNotFound)
    return AUTORELEASE([_fullFileName copy]);

  /* add file type extension if the file name does not have an extension or
     the file name's extension is not one of the allowed extensions and the
     save panel does not allow other extensions */
  fileType = [_fullFileName pathExtension];
  if ([fileType length] == 0 ||
      ((!_allowsOtherFileTypes &&
	[_allowedFileTypes indexOfObject: fileType] == NSNotFound)))
    {
      fileType = [_allowedFileTypes objectAtIndex: 0];
      return [_fullFileName stringByAppendingPathExtension: fileType];
    }
  else
    {
      return AUTORELEASE([_fullFileName copy]);
    }
}

- (NSURL *) URL
{
  return [NSURL fileURLWithPath: [self filename]];
}

/**<p>Invoked by the 'Cancel' button. Saves the current directory browsed
 and stop the modal event loop using [NSApplication-stopModalWithCode:]</p>
 <p>See Also: -ok:</p>
 */
- (void) cancel: (id)sender
{
  ASSIGN(_directory, pathToColumn(_browser, [_browser lastColumn]));
  [self _updateDefaultDirectory];

  if (self->_completionHandler == NULL)
    [NSApp stopModalWithCode: NSCancelButton];
  else
    {
      CALL_BLOCK(self->_completionHandler, NSCancelButton);
      Block_release(self->_completionHandler);
      self->_completionHandler = NULL;
    }

  [_okButton setEnabled: NO];
  [self close];
}

/**<p>Invoked by the "OK" button.</p>
 *<p>See Also: -cancel:</p>
 */
- (void) ok: (id)sender
{
  NSMatrix      *matrix;
  NSBrowserCell *selectedCell;
  NSString      *filename;
  BOOL		isDir = NO;

  matrix = [_browser matrixInColumn: [_browser lastColumn]];
  selectedCell = [matrix selectedCell];

  if (selectedCell && [selectedCell isLeaf] == NO)
    {
      [[_form cellAtIndex: 0] setStringValue: @""];
      [_browser doClick: matrix];
      [_form selectTextAtIndex: 0];
      [_form setNeedsDisplay: YES];

      return;
    }

  ASSIGN (_directory, pathToColumn(_browser, [_browser lastColumn]));
  filename = [[_form cellAtIndex: 0] stringValue];
  if ([filename isAbsolutePath] == NO)
    {
      filename = [_directory stringByAppendingPathComponent: filename];
    }
  ASSIGN (_fullFileName, [filename stringByStandardizingPath]);

  if (_delegateHasUserEnteredFilename)
    {
      filename = [_delegate panel: self
			    userEnteredFilename: _fullFileName
			    confirmed: YES];
      if (!filename)
	return;
      else if (![_fullFileName isEqual: filename])
	{
	  ASSIGN (_directory, [filename stringByDeletingLastPathComponent]);
	  ASSIGN (_fullFileName, filename);
	  setPath(_browser, _fullFileName);
	  [self _setFileName: [_fullFileName lastPathComponent]];
	}
    }

  /* Warn user if a wrong extension was entered */
  if (_allowedFileTypes != nil &&
      [_allowedFileTypes indexOfObject: @""] == NSNotFound)
    {
      NSString *fileType = [_fullFileName pathExtension];
      if ([fileType length] != 0 &&
	  [_allowedFileTypes indexOfObject: fileType] == NSNotFound)
	{
	  int result;
	  NSString *msgFormat, *butFormat;
	  NSString *altType, *requiredType;

	  requiredType = [self requiredFileType];
	  if ([self allowsOtherFileTypes])
	    {
	      msgFormat =
		_(@"You have used the extension '.%@'.\n"
		  @"The standard extension is '.%@'.'");
	      butFormat = _(@"Use .%@");
	      altType = fileType;
	    }
	  else
	    {
	      msgFormat =
		_(@"You cannot save this document with extension '.%@'.\n"
		  @"The required extension is '.%@'.");
	      butFormat = _(@"Use .%@");
	      altType = [fileType stringByAppendingPathExtension: requiredType];
	    }

	  result = NSRunAlertPanel(_(@"Save"),
		     msgFormat,
		     [NSString stringWithFormat: butFormat, requiredType],
		     _(@"Cancel"),
		     [NSString stringWithFormat: butFormat, altType],
		     fileType, requiredType);
	  switch (result)
	    {
	    case NSAlertDefaultReturn:
	      filename = [_fullFileName stringByDeletingPathExtension];
	      filename =
		[filename stringByAppendingPathExtension: requiredType];

	      ASSIGN (_fullFileName, filename);
	      setPath(_browser, _fullFileName);
	      [self _setFileName: [_fullFileName lastPathComponent]];
	      break;
	    case NSAlertOtherReturn:
	      if (altType != fileType)
		{
		  filename =
		    [_fullFileName stringByAppendingPathExtension: requiredType];

		  ASSIGN (_fullFileName, filename);
		  setPath(_browser, _fullFileName);
		  [self _setFileName: [_fullFileName lastPathComponent]];
		}
	      break;
	    default:
	      return;
	    }
	}
    }

  filename = [_fullFileName stringByDeletingLastPathComponent];
  if ([_fm fileExistsAtPath: filename isDirectory: &isDir] == NO)
    {
      int	result;

      result = NSRunAlertPanel(_(@"Save"),
	_(@"The directory '%@' does not exist, do you want to create it?"),
	_(@"Yes"), _(@"No"), nil,
	filename
	);

      if (result == NSAlertDefaultReturn)
	{
	  if ([_fm createDirectoryAtPath: filename
	     withIntermediateDirectories: YES
			      attributes: nil
				   error: NULL] == NO)
	    {
	      NSRunAlertPanel(_(@"Save"),
		_(@"The directory '%@' could not be created."),
		_(@"Dismiss"), nil, nil,
		filename
		);
	      return;
	    }
	}
    }
  else if (isDir == NO)
    {
      NSRunAlertPanel(_(@"Save"),
	_(@"The path '%@' is not a directory."),
	_(@"Dismiss"), nil, nil,
	filename
	);
      return;
    }
  if ([_fm fileExistsAtPath: [self filename] isDirectory: NULL])
    {
      int result;

      result = NSRunAlertPanel(_(@"Save"),
			       _(@"The file '%@' in '%@' exists. Replace it?"),
			       _(@"Replace"), _(@"Cancel"), nil,
			       [[self filename] lastPathComponent],
			       _directory);

      if (result != NSAlertDefaultReturn)
	return;
    }

  if (_delegateHasValidNameFilter)
    if (![_delegate panel: self isValidFilename: [self filename]])
      return;

  [self _updateDefaultDirectory];

  if (self->_completionHandler == NULL)
    [NSApp stopModalWithCode: NSOKButton];
  else
    {
      CALL_BLOCK(self->_completionHandler, NSOKButton);
      Block_release(self->_completionHandler);
      self->_completionHandler = NULL;
    }

  [_okButton setEnabled: NO];
  [self close];
}

- (void) selectText: (id)sender
{
  NSEvent  *theEvent = [self currentEvent];
  NSString *characters = [theEvent characters];
  unichar   character = 0;

  if ([characters length] > 0)
    {
      character = [characters characterAtIndex: 0];
    }

  switch (character)
    {
    case NSUpArrowFunctionKey:
    case NSDownArrowFunctionKey:
    case NSLeftArrowFunctionKey:
    case NSRightArrowFunctionKey:
      [_form abortEditing];
      [[_form cellAtIndex:0] setStringValue: @""];
      [_browser keyDown:theEvent];
      break;
    }
}

- (id<NSOpenSavePanelDelegate>) delegate
{
  return [super delegate];
}

- (void) setDelegate: (id<NSOpenSavePanelDelegate>)aDelegate
{
  if ([aDelegate respondsToSelector:
		   @selector(panel:compareFilename:with:caseSensitive:)])
    _delegateHasCompareFilter = YES;
  else 
    _delegateHasCompareFilter = NO;

  if ([aDelegate respondsToSelector: @selector(panel:shouldShowFilename:)])
    _delegateHasShowFilenameFilter = YES;      
  else
    _delegateHasShowFilenameFilter = NO;      

  if ([aDelegate respondsToSelector: @selector(panel:isValidFilename:)])
    _delegateHasValidNameFilter = YES;
  else
    _delegateHasValidNameFilter = NO;

  if ([aDelegate respondsToSelector: @selector(panel:userEnteredFilename:confirmed:)])
    _delegateHasUserEnteredFilename = YES;
  else
    _delegateHasUserEnteredFilename = NO;

  if ([aDelegate respondsToSelector: @selector(panel:directoryDidChange:)])
    _delegateHasDirectoryDidChange = YES;
  else 
    _delegateHasDirectoryDidChange = NO;      

  if ([aDelegate respondsToSelector: @selector(panelSelectionDidChange:)])
    _delegateHasSelectionDidChange = YES;
  else 
    _delegateHasSelectionDidChange = NO;      

  [super setDelegate: aDelegate];
  [self validateVisibleColumns];
}

- (void) setCanSelectHiddenExtension: (BOOL) flag
{
  _canSelectHiddenExtension = flag;
}
- (BOOL) canSelectHiddenExtension
{
  return _canSelectHiddenExtension;
}

- (BOOL) isExtensionHidden
{
  return _isExtensionHidden;
}

- (void) setExtensionHidden: (BOOL) flag
{
  _isExtensionHidden = flag;
}

- (BOOL) showsHiddenFiles
{
  return _showsHiddenFiles;
}

- (void) setShowsHiddenFiles: (BOOL) flag
{
  if (flag != _showsHiddenFiles)
    {
      _showsHiddenFiles = flag;
      [[_showsHiddenFilesMenu itemAtIndex: 0] setState: flag];
      [self _reloadBrowser];
    }
}

- (BOOL) isExpanded
{
  // FIXME
  return NO;
}

//
// NSCoding protocol
//
- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  // TODO
  return self;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder]; 
  // TODO
}

@end

//
// SavePanel filename compare
//
@implementation NSString (GSSavePanel)
- (NSComparisonResult)_gsSavePanelCompare:(NSString *)other
{
  int                sLength, oLength;
  unichar            sChar, oChar;
  NSComparisonResult result;
  NSRange            range;

  sLength = [self length];
  oLength = [other length];
  range.location = 0;
  range.length = sLength;

  if (sLength == 0)
    {
      if (oLength == 0)
	return NSOrderedSame;
      else
	return NSOrderedAscending;
    }
  else if (oLength == 0)
    {
      return NSOrderedDescending;
    }

  sChar = [self characterAtIndex: 0];
  oChar = [other characterAtIndex: 0];

  if (sChar == '.' && oChar != '.')
    return NSOrderedDescending;
  else if (sChar != '.' && oChar == '.')
    return NSOrderedAscending;

  if (sLength == oLength)
    {
      result = [self compare: other
		     options: NSCaseInsensitiveSearch
		     range: range];

      if (result == NSOrderedSame)
	result = [self compare: other options: 0 range: range];
    }
  else
    {
      if (sLength < oLength)
	{
	  result = [other compare: self
			  options: NSCaseInsensitiveSearch
			  range: range];

	  if (result == NSOrderedAscending)
	    result = NSOrderedDescending;
	  else if (result == NSOrderedDescending)
	    result = NSOrderedAscending;
	  else
	    {
	      result = [other compare: self options: 0 range: range];

	      if (result == NSOrderedAscending)
		result = NSOrderedDescending;
	      else
		result = NSOrderedAscending;
	    }
	}
      else
	result = [self compare: other
		       options: NSCaseInsensitiveSearch
		       range: range];

      if (result == NSOrderedSame)
	result = [self compare: other options: 0 range: range];
    }

  return result;
}

@end

//
// NSSavePanel browser delegate methods
//
@interface NSSavePanel (GSBrowserDelegate)
- (void) browserDidScroll: (NSBrowser *)sender;
- (void) browser: (NSBrowser*)sender
createRowsForColumn: (NSInteger)column
        inMatrix: (NSMatrix*)matrix;

- (BOOL) browser: (NSBrowser*)sender
   isColumnValid: (NSInteger)column;

- (void) browser: (NSBrowser*)sender
 willDisplayCell: (id)cell
           atRow: (NSInteger)row
          column: (NSInteger)column;
@end 

static NSComparisonResult compareFilenames (id elem1, id elem2, void *context)
{
  /* TODO - use IMP optimization here.  */
  NSSavePanel *self = (NSSavePanel *)context;

  return [self->_delegate panel: self
             compareFilename: elem1
                        with: elem2
               caseSensitive: YES];
}


@implementation NSSavePanel (GSBrowserDelegate)
- (void) browserDidScroll: (NSBrowser *)sender
{
  [self validateVisibleColumns];
}

- (void) browser: (NSBrowser*)sender
createRowsForColumn: (NSInteger)column
	inMatrix: (NSMatrix*)matrix
{
  NSString              *path, *file, *pathAndFile, *extension; 
  NSArray               *files;
  unsigned	         i, count, addedRows; 
  BOOL		         exists, isDir;
  NSBrowserCell         *cell;
  // _gs_display_reading_progress variables
  unsigned               reached_frac = 0;
  unsigned               base_frac = 1;
  BOOL                   display_progress = NO;
  NSString              *progressString = nil;
  NSWorkspace		*ws;
  /* We create lot of objects in this method, so we use a pool */
  NSAutoreleasePool     *pool;

  pool = [NSAutoreleasePool new];
  ws = [NSWorkspace sharedWorkspace];
  path = pathToColumn(_browser, column);
#if	defined(__MINGW32__)
  if (column == 0)
    {
      NSMutableArray	*m;
      unsigned		i;

      files = [ws mountedLocalVolumePaths];
      m = [files mutableCopy];
      i = [m count];
      while (i-- > 0)
	{
	  NSString	*file = [m objectAtIndex: i];

	  /* Strip the backslash from the drive name so we  don't
	   * get it confusing the path we have.
	   */
	  file = [file substringToIndex: [file length] - 1];
	  [m replaceObjectAtIndex: i withObject: file];
	}
      files = [m autorelease];
    }
  else
    {
      files = [[NSFileManager defaultManager] directoryContentsAtPath: path];
    }
#else
  files = [[NSFileManager defaultManager] directoryContentsAtPath: path];
#endif

  /* Remove hidden files.  */
  {
    NSString *h;
    NSArray *hiddenFiles = nil;
    
    // FIXME: Use NSFileManager to tell us what files are hidden/non-hidden
    // rather than having it hardcoded here

    if ([files containsObject: @".hidden"] == YES)
      {
	/* We need to remove files listed in the xxx/.hidden file.  */
	h = [path stringByAppendingPathComponent: @".hidden"];
	h = [NSString stringWithContentsOfFile: h];
	hiddenFiles = [h componentsSeparatedByString: @"\n"];
      }

    /* Alse remove files starting with `.' (dot) */

    /* Now copy the files array into a mutable array - but only if
       strictly needed.  */
    if (!_showsHiddenFiles)
      {
	int j;
	/* We must make a mutable copy of the array because the API
	   says that NSFileManager -directoryContentsAtPath: return a
	   NSArray, not a NSMutableArray, so we shouldn't expect it to
	   be mutable.  */
	NSMutableArray *mutableFiles = AUTORELEASE ([files mutableCopy]);
	
	/* Ok - now modify the mutable array removing unwanted files.  */
	if (hiddenFiles != nil)
	  {
	    [mutableFiles removeObjectsInArray: hiddenFiles];
	  }
	
	
	/* Don't use i which is unsigned.  */
	j = [mutableFiles count] - 1;
	
	while (j >= 0)
	  {
	    NSString *file = (NSString *)[mutableFiles objectAtIndex: j];
	    
	    if ([file hasPrefix: @"."])
	      {
		/* NSLog (@"Removing dot file %@", file); */
		[mutableFiles removeObjectAtIndex: j];
	      }
	    j--;
	  }
	
	files = mutableFiles;
      }
  }

  count = [files count];

  /* If array is empty, just return (nothing to display).  */
  if (count == 0)
    {
      RELEASE (pool);
      return;
    }

  // Prepare Messages on title bar if directory is big and user wants them
  if (_gs_display_reading_progress && (count > 100))
    {
      display_progress = YES;
      base_frac = count / 4;
      progressString = [_(@"Reading Directory ") stringByAppendingString: path];
      [super setTitle: progressString];
      // Is the following really safe? 
      [self flushWindow];
    }

  //TODO: Sort after creation of matrix so we do not sort 
  // files we are not going to show.  Use NSMatrix sorting cells method
  // Sort list of files to display
  if (_delegateHasCompareFilter == YES)
    {
      files = [files sortedArrayUsingFunction: compareFilenames 
		     context: self];
    }
  else
    files = [files sortedArrayUsingSelector: @selector(_gsSavePanelCompare:)];

  addedRows = 0;
  for (i = 0; i < count; i++)
    {
      // Update displayed message if needed
      if (display_progress && (i > (base_frac * (reached_frac + 1))))
        {
          reached_frac++;
          progressString = [progressString stringByAppendingString: @"."];
          [super setTitle: progressString];
          [self flushWindow];
        }
      // Now the real code
      file = [files objectAtIndex: i];
      extension = [file pathExtension];
      
      pathAndFile = [path stringByAppendingPathComponent: file];
      exists = [_fm fileExistsAtPath: pathAndFile 
		    isDirectory: &isDir];

      /* Note: The initial directory and its parents are always shown, even if
       * it they are file packages or would be rejected by the validator. */
#define HAS_PATH_PREFIX(aPath, otherPath) \
  ([aPath isEqualToString: otherPath] || \
   [aPath hasPrefix: [otherPath stringByAppendingString: @"/"]])

      if (exists && (!isDir || !HAS_PATH_PREFIX(_directory, pathAndFile)))
	{
	  if (isDir && !_treatsFilePackagesAsDirectories
	      && ([ws isFilePackageAtPath: pathAndFile]
		  || [_allowedFileTypes containsObject: extension]))
	    {
	      isDir = NO;
	    }

	  if (_delegateHasShowFilenameFilter)
	    {
	      exists = [_delegate panel: self shouldShowFilename: pathAndFile];
	    }

	  if (exists && !isDir)
	    {
	      exists = [self _shouldShowExtension: extension];
	    }
	} 

      if (exists)
	{
	  if (addedRows == 0)
	    {
	      [matrix addColumn];
	    }
	  else // addedRows > 0
	    {
	      /* Same as [matrix addRow] */
	      [matrix insertRow: addedRows  withCells: nil];
	      /* Possible TODO: Faster would be to create all the
		 cells at once with a single call instead of resizing 
		 the matrix each time a cell is inserted. */
	    }

	  cell = [matrix cellAtRow: addedRows column: 0];
	  [cell setStringValue: file];

	  {
	    NSImage *icon = [[ws iconForFile: pathAndFile] copy];
	    CGFloat iconSize = [cell cellSize].height - 1;
	    [icon setSize: NSMakeSize(iconSize, iconSize)];
	    [cell setImage: icon];
	    [icon release];
	  }

	  if (isDir)
	    [cell setLeaf: NO];
	  else
	    [cell setLeaf: YES];

	  addedRows++;
	}
    }

  if (display_progress)
    {
      [super setTitle: @""];
      [self flushWindow];
    }

  RELEASE (pool);
}

- (BOOL) browser: (NSBrowser*)sender
   isColumnValid: (NSInteger)column
{
  /*
   * FIXME This code doesn't handle the case where the delegate now wants
   *       to show additional files, which were not displayed before.
   */
  NSArray	*cells = [[sender matrixInColumn: column] cells];
  unsigned	count = [cells count], i;
  NSString	*path = pathToColumn(sender, column);

  // iterate through the cells asking the delegate if each filename is valid
  // if it says no for any filename, the column is not valid
  if (_delegateHasShowFilenameFilter == YES)
    for (i = 0; i < count; i++)
      {
	if (![_delegate panel: self 
			shouldShowFilename:
			  [path stringByAppendingPathComponent:
			    [[cells objectAtIndex: i] stringValue]]])
	  return NO;
      }

  return YES;
}

- (void) browser: (NSBrowser*)sender
 willDisplayCell: (id)cell
	   atRow: (NSInteger)row
	  column: (NSInteger)column
{
}
@end

//
// NSForm delegate methods
//
@interface NSSavePanel (FormDelegate)
- (void) controlTextDidChange: (NSNotification *)aNotification;
@end
@implementation NSSavePanel (FormDelegate)

- (void) controlTextDidChange: (NSNotification *)aNotification
{
  NSString           *s, *selectedString;
  NSArray            *cells;
  NSMatrix           *matrix;
  NSCell             *selectedCell;
  int                 i, sLength, cellLength, selectedRow;
  NSComparisonResult  result;
  NSRange             range;

  s = [[[aNotification userInfo] objectForKey: @"NSFieldEditor"] string];

  /*
   * If the user typed in an absolute path, display it.
   */
  if ([s isAbsolutePath] == YES)
    {
      [self setDirectory: s];
    }

  sLength = [s length];
  range.location = 0;
  range.length = sLength;

  matrix = [_browser matrixInColumn:[_browser lastColumn]];
  if (sLength == 0)
    {
      [matrix deselectAllCells];
      [_okButton setEnabled:NO];
      return;
    }

  selectedCell = [matrix selectedCell];
  selectedString = [selectedCell stringValue];
  selectedRow = [matrix selectedRow];
  cells = [matrix cells];

  if (selectedString)
    {
      result = [s compare:selectedString options:0 range:range];

      if (result == NSOrderedSame)
	return;
    }
  else
    result = NSOrderedDescending;

  if (result == NSOrderedDescending)
    {
      int numberOfCells = [cells count];

      for (i = selectedRow+1; i < numberOfCells; i++)
	{
	  selectedString = [[matrix cellAtRow:i column:0] stringValue];

	  cellLength = [selectedString length];
	  if (cellLength != sLength)
	    continue;

	  result = [selectedString compare:s options:0 range:range];

	  if (result == NSOrderedSame)
	    {
	      [matrix deselectAllCells];
	      [matrix selectCellAtRow:i column:0];
	      [matrix scrollCellToVisibleAtRow:i column:0];
	      [_okButton setEnabled:YES];
	      return;
	    }
	}
    }
  else
    {
      for (i = selectedRow; i >= 0; --i)
	{
	  selectedString = [[matrix cellAtRow:i column:0] stringValue];

	  cellLength = [selectedString length];
	  if (cellLength != sLength)
	    continue;

	  result = [selectedString compare:s options:0 range:range];

	  if (result == NSOrderedSame)
	    {
	      [matrix deselectAllCells];
	      [matrix selectCellAtRow:i column:0];
	      [matrix scrollCellToVisibleAtRow:i column:0];
	      [_okButton setEnabled:YES];
	      return;
	    }
	}
    }

  [matrix deselectAllCells];
  [_okButton setEnabled:YES];
}

@end /* NSSavePanel */
