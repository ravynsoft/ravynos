/* GSNamedColorPicker.m

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: January 2001
   
   This file is part of GNUstep.
   
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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

@interface GSNamedColorPicker: NSColorPicker <NSColorPickingCustom, NSTextFieldDelegate>
{
  NSView *baseView;
  NSComboBox *cb;
  NSBrowser *browser;  
  NSColorList *currentList;
  NSMutableArray *lists;
}

- (void) loadViews;
- (void) listSelected: (int)index;
- (void) colorSelected: (id)sender;

@end

@implementation GSNamedColorPicker

- (id) initWithPickerMask: (int)aMask
	       colorPanel: (NSColorPanel *)colorPanel
{
  if (aMask & NSColorPanelColorListModeMask)
    {
      self = [super initWithPickerMask: aMask
		    colorPanel: colorPanel];
      if (nil == self)
        return nil;

      lists =  [[NSColorList availableColorLists] mutableCopy];
      return self;
    }
  RELEASE(self);
  return nil;
}

- (void) dealloc
{
  RELEASE(cb);
  RELEASE(browser);
  RELEASE(baseView);
  RELEASE(lists);
  [super dealloc];
}

- (void) finishInstantiate
{
}

- (void) attachColorList: (NSColorList *)colorList
{
  [lists addObject: colorList];
  [cb noteNumberOfItemsChanged];
}

- (void) detachColorList: (NSColorList *)colorList
{
  [lists removeObjectIdenticalTo: colorList];
  [cb noteNumberOfItemsChanged];
}

- (int) currentMode
{
  return NSColorListModeColorPanel;
}

- (BOOL) supportsMode: (int)mode
{
  return mode == NSColorListModeColorPanel;
}

- (NSView *) provideNewView: (BOOL)initialRequest
{
  if (initialRequest)
    {
      [self loadViews];
    }
  return baseView;
}

- (void) setColor: (NSColor *)color
{
  NSColor *c = [color colorUsingColorSpaceName: NSNamedColorSpace];
  NSString *list;
  NSString *name;
  NSUInteger index;

  if (c == nil)
    return;

  list = [c catalogNameComponent];
  name = [c colorNameComponent];
  // Select the correspondig entries in the lists
  index = [self comboBox: cb
                indexOfItemWithStringValue: list];
  if (index == NSNotFound)
    return;
  [cb selectItemAtIndex: index];

  index = [[currentList allKeys] indexOfObject: name];
  if (index == NSNotFound)
    return;
  [browser selectRow: index inColumn: 0];
}

- (void) loadViews
{
  baseView = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 200, 110)];
  [baseView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];

  cb = [[NSComboBox alloc] initWithFrame: NSMakeRect(0, 85, 196, 20)];
  [cb setAutoresizingMask: (NSViewWidthSizable | NSViewMinYMargin)];
  [cb setUsesDataSource: YES];
  [cb setDataSource: self];
  [cb setDelegate: self];
  [cb setEditable: NO];
  [baseView addSubview: cb];

  browser = [[NSBrowser alloc] initWithFrame: NSMakeRect(0, 5, 196, 75)];
  [browser setDelegate: self];
  [browser setMaxVisibleColumns: 1];
  [browser setAllowsMultipleSelection: NO];
  [browser setAllowsEmptySelection: NO];
  [browser setHasHorizontalScroller: NO];
  [browser setTitled: NO];
  [browser setTakesTitleFromPreviousColumn: NO];
  [browser setPath: @"/"];
  [browser setTarget: self];
  [browser setDoubleAction: @selector(colorSelected:)];
  [browser setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  [baseView addSubview: browser];
}

- (void) listSelected: (int)index
{
  currentList = [lists objectAtIndex: index];
  [browser loadColumnZero];
}

- (void) colorSelected: (id)sender
{
  NSCell *aCell;
  NSColor *col;

  aCell = [sender selectedCell];
  col = [aCell representedObject];
  [_colorPanel setColor: col];
}

-(NSString *) browser: (NSBrowser *)sender 
	titleOfColumn: (NSInteger)column
{
  return nil;
}

-(void) browser: (NSBrowser *)sender 
createRowsForColumn: (NSInteger)column
       inMatrix: (NSMatrix *)matrix
{
  int i;
  int count;
  NSBrowserCell *cell;
  NSColor *cl;
  NSArray *keys = [currentList allKeys];
  NSString *name;

  count = [keys count];
  //NSLog(@"In create with %@ %d", currentList, count);
  
  if (count)
    {
      [matrix addColumn];
      for (i = 0; i < count; i++)
	{
	  if (i > 0)
	    [matrix addRow];
	  name = [keys objectAtIndex: i];
	  //cl = [currentList colorWithKey: name];
	  cl = [NSColor colorWithCatalogName: [currentList name] 
			colorName: name];
	  cell = [matrix cellAtRow: i
			 column: 0];
	  [cell setStringValue: name];
	  [cell setRepresentedObject: cl];
	  [cell setLeaf: YES];
	}
    }
} 

- (BOOL) browser: (NSBrowser*)sender 
       selectRow: (NSInteger)row
	inColumn: (NSInteger)column
{
  return NO;
}

- (void) browser: (NSBrowser *)sender 
 willDisplayCell: (id)cell
	   atRow: (NSInteger)row 
	  column: (NSInteger)column
{
}

- (NSInteger) numberOfItemsInComboBox: (NSComboBox *)aComboBox
{
   return [lists count];
}

- (id) comboBox: (NSComboBox *)aComboBox 
objectValueForItemAtIndex: (NSInteger)index
{
   return [(NSColorList*)[lists objectAtIndex: index] name];
}

- (NSUInteger) comboBox: (NSComboBox *)aComboBox
indexOfItemWithStringValue: (NSString *)string
{
   return [lists indexOfObject: [NSColorList colorListNamed: string]];
}

- (void) comboBoxSelectionDidChange: (NSNotification *)notification
{
  [self listSelected: [cb indexOfSelectedItem]];
}

@end
