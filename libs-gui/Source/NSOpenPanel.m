/** <title>NSOpenPanel</title> -*-objc-*-

   <abstract>Standard panel for opening files</abstract>

   Copyright (C) 1996, 1998, 1999, 2000 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996

   Author: Daniel Boehringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998

   Source by Daniel Boehringer integrated into Scott Christley's preliminary
   implementation by Felipe A. Rodriguez <far@ix.netcom.com>

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: October 1999 Completely Rewritten.

   Author: Mirko Viviani <mirko.viviani@rccr.cremona.it>
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

#import <Foundation/NSArray.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSForm.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSOpenPanel.h"

#import "GSGuiPrivate.h"
#import "GNUstepGUI/GSTheme.h"

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

static NSOpenPanel *_gs_gui_open_panel = nil;

// Pacify the compiler
@interface NSSavePanel (GSPrivateMethods)
- (void) _resetDefaults;
- (void) _updateDefaultDirectory;
- (void) _reloadBrowser;
- (void) _selectCellName: (NSString *)title;
- (void) _selectTextInColumn: (int)column;
- (void) _setupForDirectory: (NSString *)path file: (NSString *)filename;
- (BOOL) _shouldShowExtension: (NSString *)extension;
- (NSComparisonResult) _compareFilename: (NSString *)n1 with: (NSString *)n2;
@end

@implementation NSOpenPanel (GSPrivateMethods)
- (void) _resetDefaults
{
  [super _resetDefaults];
  [self setTitle: _(@"Open")];
  [self setCanChooseFiles: YES];
  [self setCanChooseDirectories: YES];
  [self setAllowsMultipleSelection: NO];
  [_okButton setEnabled: NO];
}

- (BOOL) _shouldShowExtension: (NSString *)extension
{
  if (_canChooseFiles == NO ||
      (_allowedFileTypes != nil && [_allowedFileTypes containsObject: extension] == NO))
    return NO;

  return YES;
}

- (void) _selectTextInColumn: (int)column
{
  NSMatrix *matrix;

  if (column == -1)
    return;

  if (_delegateHasSelectionDidChange) 
    {
      [_delegate panelSelectionDidChange: self];
    }

  matrix = [_browser matrixInColumn: column];

  // Validate selection

  BOOL selectionValid = YES;

  NSArray *selectedCells = [matrix selectedCells];
  NSEnumerator *e = [selectedCells objectEnumerator];
  id o;
  while ((o = [e nextObject]) != nil)
    {
      if ((![o isLeaf] && !_canChooseDirectories) ||
	  ([o isLeaf] && !_canChooseFiles))
	{
	  selectionValid = NO;
	}
    }
  [_okButton setEnabled: selectionValid];
  
  // Set form label

  if ([selectedCells count] > 1)
    {
      [[_form cellAtIndex: 0] setStringValue: @""];
    }
  else
    {
      [[_form cellAtIndex: 0] setStringValue: [[matrix selectedCell] stringValue]];
    }  
}

- (void) _setupForDirectory: (NSString *)path file: (NSString *)filename
{
  // FIXME: Not sure if this is needed
  if ((filename == nil) || ([filename isEqual: @""] == NO))
    [_okButton setEnabled: YES];

  if (_canChooseDirectories == NO)
    {
      if ([_browser allowsMultipleSelection] == YES)
	[_browser setAllowsBranchSelection: NO];
    }
  
  [super _setupForDirectory: path file: filename];
}

@end

/** 
    <p>Implements a panel that allows the user to select a file or files.
    NSOpenPanel is based on the NSSavePanel implementation and shares
    a lot of similarities with it.
    </p>
    <p>
    There is only one open panel per application and this panel is obtained
    by calling the +openPanel class method. From here, you should set the
    characteristics of the file selection mechanism using the
    -setCanChooseFiles:, -setCanChooseDirectories: and 
    -setAllowsMultipleSelection: methods. The default is YES except for
    allowing multiple selection. When ready to show the panel, use the
    -runModalForTypes:, or a similar method to show the panel in a modal
    session. Other methods allow you to set the initial directory and
    initially selected file. The method will return one of NSOKButton
    or NSCancelButton depending on which button the user pressed.
    </p>
    <p>
    Use the [NSSavePanel-filename] or -filenames method to retrieve
    the name of the file the user selected.
    </p>
 */
@implementation NSOpenPanel

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSOpenPanel class])
  {
    [self setVersion: 1];
  }
}

/**<p>Creates ( if needed ) and returns the shared NSOpenPanel instance.</p> 
*/
+ (NSOpenPanel *) openPanel
{
  if (!_gs_gui_open_panel)
    {
      Class openPanelClass = [[GSTheme theme] openPanelClass];
      _gs_gui_open_panel = [[openPanelClass alloc] init];
    }
  [_gs_gui_open_panel _resetDefaults];

  return _gs_gui_open_panel;
}

- (id) init
{
  self = [super init];
  if (self != nil)
    {
      _canChooseDirectories = YES;
      _canChooseFiles = YES;
    }
  return self;
}


/*
 * Filtering Files
 */
/**<p> Allows the user to select multiple files if flag is YES.
   The default behavior is not to allow multiple selections</p><p>See Also:
   -allowsMultipleSelection [NSBrowser-setAllowsMultipleSelection:]</p>
*/
- (void) setAllowsMultipleSelection: (BOOL)flag
{
  [_browser setAllowsMultipleSelection: flag];
}

/**<p>Returns YES if the user is allowed to select multiple files. The
   default behavior is not to allow mutiple selections.</p><p>See Also:
   -setAllowsMultipleSelection: [NSBrowser-allowsMultipleSelection]</p>
*/
- (BOOL) allowsMultipleSelection
{
  return [_browser allowsMultipleSelection];
}

/**<p>Allows the user to choose directories if flag is YES. The default 
   behavior is to allow choosing directories.</p>
   <p>See Also: -canChooseDirectories [NSBrowser-setAllowsBranchSelection:]</p>
*/
- (void) setCanChooseDirectories: (BOOL)flag
{
  if (flag != _canChooseDirectories)
    {
      _canChooseDirectories = flag;
      [_browser setAllowsBranchSelection: flag];
      if (!flag)
	{
	  /* FIXME If the user disables directory selection we should deselect
	     any directories that are currently selected. This is achieved by
	     calling _reloadBrowser, but this may be considered overkill, since
	     the displayed files are the same whether canChooseDirectories is
	     enabled or not. */
	  [self _reloadBrowser];
	}
    }
}

/** <p>Returns YES if the user is allowed to choose directories  The
    default behavior is to allow choosing directories.</p>
    <p>See Also: -setCanChooseDirectories:</p>
 */
- (BOOL) canChooseDirectories
{
  return _canChooseDirectories;
}

/** <p>Allows the user to choose files if flag is YES.The default behavior it
    to allow choosing files. </p><p>See Also: -canChooseFiles</p>
*/
- (void) setCanChooseFiles: (BOOL)flag
{
  if (flag != _canChooseFiles)
    {
      _canChooseFiles = flag;
      [self _reloadBrowser];
    }
}

/**<p>Returns YES if the user is allowed to choose files.  The
    default behavior it to allow choosing files.</p>
    <p>See Also: -setCanChooseFiles:</p>
*/
- (BOOL) canChooseFiles
{
  return _canChooseFiles;
}

/** <p>Returns the absolute path of the file selected by the user.</p>
*/
- (NSString*) filename
{
  NSArray *ret;

  ret = [self filenames];

  if ([ret count] == 1)
    return [ret objectAtIndex: 0];
  else 
    return nil;
}

/**<p>Returns an array containing the absolute paths (as NSString
   objects) of the selected files and directories.  If multiple
   selections aren't allowed, the array contains a single name.</p>
*/
- (NSArray *) filenames
{
  if ([_browser allowsMultipleSelection])
    {
      NSArray         *cells = [_browser selectedCells];
      NSEnumerator    *cellEnum = [cells objectEnumerator];
      NSBrowserCell   *currCell;
      NSMutableArray  *ret = [NSMutableArray array];
      NSString        *dir = [self directory];
      
      if ([_browser selectedColumn] != [_browser lastColumn])
	{
	  /*
	   * The last column doesn't have anything selected - so we must
	   * have selected a directory.
	   */
	  if (_canChooseDirectories == YES)
	    {
	      [ret addObject: dir];
	    }
	}
      else
	{
	  while ((currCell = [cellEnum nextObject]))
	    {
	      [ret addObject:
                [dir stringByAppendingPathComponent: [currCell stringValue]]];
	    }
	}
      return ret;
    }
  else
    {
      if (_canChooseDirectories == YES)
	{
	  if ([_browser selectedColumn] != [_browser lastColumn])
	    return [NSArray arrayWithObject: [self directory]];
	}

      return [NSArray arrayWithObject: [super filename]];
    }
}

/** Returns an array of the selected files as URLs */
- (NSArray *) URLs
{
  NSMutableArray *ret = [NSMutableArray new];
  NSEnumerator *enumerator = [[self filenames] objectEnumerator];
  NSString *filename;

  while ((filename = [enumerator nextObject]) != nil)
    {
      [ret addObject: [NSURL fileURLWithPath: filename]];
    } 

  return AUTORELEASE(ret);
}

/*
 * Running the NSOpenPanel
 */

/**<p>Displays the open panel in a modal session, showing the current directory
   (or last selected), and filtering for files that matches the allowed file 
   types.</p>
   <p>See Also: -runModalForDirectory:file:types: and -allowedFileTypes</p>
*/
- (NSInteger) runModal
{
  return [self runModalForTypes: [self allowedFileTypes]];
}

/**<p>Displays the open panel in a modal session, showing the current directory 
   (or last selected), and filtering for files that have the specified types.</p>
   <p>See Also: -runModalForDirectory:file:types:</p>
*/
- (NSInteger) runModalForTypes: (NSArray *)fileTypes
{
  return [self runModalForDirectory: [self directory]
			       file: @""
			      types: fileTypes];
}

/** <p>Displays the open panel in a modal session, with the directory
    path shown and file name (if any) selected. Files are filtered for the
    specified types. If the directory is nil, then the directory shown in 
    the open panel is the last directory selected.</p>
    <p>See Also: -runModalForTypes:</p>
*/
- (NSInteger) runModalForDirectory: (NSString *)path
                              file: (NSString *)name
                             types: (NSArray *)fileTypes
{
  [self setAllowedFileTypes: fileTypes];
  return [self runModalForDirectory: path 
			       file: name];  
}

- (NSInteger) runModalForDirectory: (NSString *)path
                              file: (NSString *)name
                             types: (NSArray *)fileTypes
                  relativeToWindow: (NSWindow*)window
{
  [self setAllowedFileTypes: fileTypes];
  return [self runModalForDirectory: path 
			       file: name
		   relativeToWindow: window];
}

- (void) beginSheetForDirectory: (NSString *)path
			   file: (NSString *)name
			  types: (NSArray *)fileTypes
		 modalForWindow: (NSWindow *)docWindow
		  modalDelegate: (id)delegate
		 didEndSelector: (SEL)didEndSelector
		    contextInfo: (void *)contextInfo
{
  [self setAllowedFileTypes: fileTypes];
  [self beginSheetForDirectory: path
			  file: name
		modalForWindow: docWindow
		 modalDelegate: delegate
		didEndSelector: didEndSelector
		   contextInfo: contextInfo];
}

- (void) beginForDirectory: (NSString *)path
                      file: (NSString *)filename
                     types: (NSArray *)fileTypes
          modelessDelegate: (id)modelessDelegate
            didEndSelector: (SEL)didEndSelector
               contextInfo: (void *)contextInfo
{
  // FIXME: This should be modeless
  [self setAllowedFileTypes: fileTypes];
  [self _setupForDirectory: path file: filename];
  if ([filename length] > 0)
    [_okButton setEnabled: YES];
  [NSApp beginSheet: self
	 modalForWindow: nil
	 modalDelegate: modelessDelegate
	 didEndSelector: didEndSelector
	 contextInfo: contextInfo];
}

- (void) ok: (id)sender
{
  NSMatrix      *matrix = nil;
  NSBrowserCell *selectedCell = nil;
  NSArray       *selectedCells = nil;
  int            selectedColumn, lastColumn;
  NSString	*tmp;

  selectedColumn = [_browser selectedColumn];
  lastColumn = [_browser lastColumn];
  if (selectedColumn >= 0)
    {
      matrix = [_browser matrixInColumn: selectedColumn];

      if ([_browser allowsMultipleSelection] == YES)
	{
	  selectedCells = [matrix selectedCells];

	  if (selectedColumn == lastColumn && [selectedCells count] == 1)
	    selectedCell = [selectedCells objectAtIndex: 0];
	}
      else
	{
	  if (selectedColumn == lastColumn)
	    selectedCell = [matrix selectedCell];
	}
    }

  if (selectedCell)
    {
      if ([selectedCell isLeaf] == NO)
	{
	  [[_form cellAtIndex: 0] setStringValue: @""];
	  [_browser doClick: matrix];
	  [_form selectTextAtIndex: 0];
	  [_form setNeedsDisplay: YES];

	  return;
	}
    }
  else if (_canChooseDirectories == NO
    && (selectedColumn != lastColumn || ![selectedCells count]))
    {
      [_form selectTextAtIndex: 0];
      [_form setNeedsDisplay: YES];
      return;
    }

  ASSIGN (_directory, pathToColumn(_browser, [_browser lastColumn]));

  if (selectedCell)
    tmp = [selectedCell stringValue];
  else
    tmp = [[_form cellAtIndex: 0] stringValue];

  if ([tmp isAbsolutePath] == YES)
    {
      ASSIGN (_fullFileName, tmp);
    }
  else
    {
      ASSIGN (_fullFileName, [_directory stringByAppendingPathComponent: tmp]);
    }

  if (_delegateHasValidNameFilter)
    {
      NSEnumerator *enumerator;
      NSArray      *filenames = [self filenames];
      NSString     *filename;

      enumerator = [filenames objectEnumerator];
      while ((filename = [enumerator nextObject]))
	{
	  if ([_delegate panel: self isValidFilename: filename] == NO)
	    return;
	}
    }

  [self _updateDefaultDirectory];
  [NSApp stopModalWithCode: NSOKButton];
  [_okButton setEnabled: NO];
  [self close];
}

- (BOOL) resolvesAliases
{
  // FIXME
  return YES;
}

- (void) setResolvesAliases: (BOOL) flag
{
  // FIXME
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_canChooseDirectories];
  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_canChooseFiles];
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_canChooseDirectories];
  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_canChooseFiles];

  return self;
}
@end

//
// NSForm delegate methods
//
@interface NSOpenPanel (FormDelegate)
- (void) controlTextDidChange: (NSNotification *)aNotification;
@end
@implementation NSOpenPanel (FormDelegate)

- (void) controlTextDidChange: (NSNotification *)aNotification;
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

  matrix = [_browser matrixInColumn: [_browser lastColumn]];

  if (sLength == 0)
    {
      [matrix deselectAllCells];
      [_okButton setEnabled: _canChooseDirectories];
      return;
    }

  selectedCell = [matrix selectedCell];
  selectedString = [selectedCell stringValue];
  selectedRow = [matrix selectedRow];
  cells = [matrix cells];

  if (selectedString)
    {
      cellLength = [selectedString length];

      if (cellLength < sLength)
	range.length = cellLength;

      result = [selectedString compare: s options: 0 range: range];

      if (result == NSOrderedSame)
	return;
      else if (result == NSOrderedAscending)
	result = NSOrderedDescending;
      else if (result == NSOrderedDescending)
	result = NSOrderedAscending;

      range.length = sLength;
    }
  else
    result = NSOrderedDescending;

  if (result == NSOrderedDescending)
    {
      int numberOfCells = [cells count];

      for (i = selectedRow+1; i < numberOfCells; i++)
	{
	  selectedString = [[matrix cellAtRow: i column: 0] stringValue];

	  cellLength = [selectedString length];
	  if (cellLength < sLength)
	    continue;

	  result = [selectedString compare: s options: 0 range: range];

	  if (result == NSOrderedSame)
	    {
	      [matrix deselectAllCells];
	      [matrix selectCellAtRow: i column: 0];
	      [matrix scrollCellToVisibleAtRow: i column: 0];
	      [_okButton setEnabled: YES];
	      return;
	    }
	}
    }
  else
    {
      for (i = selectedRow; i >= 0; --i)
	{
	  selectedString = [[matrix cellAtRow: i column: 0] stringValue];

	  cellLength = [selectedString length];
	  if (cellLength < sLength)
	    continue;

	  result = [selectedString compare: s options: 0 range: range];

	  if (result == NSOrderedSame)
	    {
	      [matrix deselectAllCells];
	      [matrix selectCellAtRow: i column: 0];
	      [matrix scrollCellToVisibleAtRow: i column: 0];
	      [_okButton setEnabled: YES];
	      return;
	    }
	}
    }

  [matrix deselectAllCells];
  [_okButton setEnabled: NO];
}

@end /* NSOpenPanel */
