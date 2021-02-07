/* 
   NSBrowser.h

   Control to display and select from hierarchal lists

   Copyright (C) 1996, 1997 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#ifndef _GNUstep_H_NSBrowser
#define _GNUstep_H_NSBrowser
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSControl.h>

@class NSString;
@class NSArray;
@class NSIndexPath;
@class NSIndexSet;

@class NSCell;
@class NSEvent;
@class NSMatrix;
@class NSScroller;
//@class NSBox;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
enum _NSBrowserColumnResizingType
{
  NSBrowserNoColumnResizing,
  NSBrowserAutoColumnResizing,
  NSBrowserUserColumnResizing
};
typedef NSUInteger NSBrowserColumnResizingType;
#endif

@interface NSBrowser : NSControl <NSCoding>
{
  // Attributes
  NSCell *_browserCellPrototype;
  Class _browserMatrixClass;
  NSString *_pathSeparator;
  
  //NSBox *_horizontalScrollerBox;
  NSScroller *_horizontalScroller;
  NSTimeInterval _lastKeyPressed;
  NSString *_charBuffer;

  BOOL _isLoaded;
  BOOL _allowsBranchSelection;
  BOOL _allowsEmptySelection;
  BOOL _allowsMultipleSelection;
  BOOL _reusesColumns;
  BOOL _separatesColumns;
  BOOL _takesTitleFromPreviousColumn;
  BOOL _isTitled;
  BOOL _hasHorizontalScroller;
  BOOL _skipUpdateScroller;
  BOOL _acceptsArrowKeys;
  BOOL _sendsActionOnArrowKeys;
  BOOL _acceptsAlphaNumericalKeys;
  BOOL _sendsActionOnAlphaNumericalKeys;
  BOOL _prefersAllColumnUserResizing;

  BOOL _passiveDelegate;
  id _browserDelegate;
  id _target;
  SEL _action;
  SEL _doubleAction;
  NSMutableArray *_browserColumns;
  NSSize _columnSize;
  NSRect _scrollerRect;
  int _alphaNumericalLastColumn;
  int _maxVisibleColumns;
  CGFloat _minColumnWidth;
  int _lastColumnLoaded;
  int _firstVisibleColumn;
  int _lastVisibleColumn;
  NSString *_columnsAutosaveName;
  NSBrowserColumnResizingType _columnResizing;
}

//
// Setting the Delegate 
//
- (id) delegate;
- (void) setDelegate: (id)anObject;

//
// Target and Action 
//
- (SEL) doubleAction;
- (BOOL) sendAction;
- (void) setDoubleAction: (SEL)aSelector;

//
// Setting Component Classes 
//
+ (Class) cellClass;
- (id) cellPrototype;
- (Class) matrixClass;
- (void) setCellClass: (Class)classId;
- (void) setCellPrototype: (NSCell *)aCell;
- (void) setMatrixClass: (Class)classId;

//
// Setting NSBrowser Behavior 
//
- (BOOL) reusesColumns;
- (void) setReusesColumns: (BOOL)flag;
- (void) setTakesTitleFromPreviousColumn: (BOOL)flag;
- (BOOL) takesTitleFromPreviousColumn;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (BOOL) autohidesScroller;
- (void) setAutohidesScroller: (BOOL)flag;
- (NSColor *) backgroundColor;
- (void) setBackgroundColor: (NSColor *)backgroundColor;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (BOOL) canDragRowsWithIndexes: (NSIndexSet *)rowIndexes
                       inColumn: (NSInteger)columnIndex
                      withEvent: (NSEvent *)dragEvent;
#endif

//
// Allowing Different Types of Selection 
//
- (BOOL) allowsBranchSelection;
- (BOOL) allowsEmptySelection;
- (BOOL) allowsMultipleSelection;
- (void) setAllowsBranchSelection: (BOOL)flag;
- (void) setAllowsEmptySelection: (BOOL)flag;
- (void) setAllowsMultipleSelection: (BOOL)flag;

//
// Setting Arrow Key Behavior
//
- (BOOL) acceptsArrowKeys;
- (BOOL) sendsActionOnArrowKeys;
- (void) setAcceptsArrowKeys: (BOOL)flag;
- (void) setSendsActionOnArrowKeys: (BOOL)flag;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (BOOL) allowsTypeSelect;
- (void) setAllowsTypeSelect: (BOOL)allowsTypeSelection;
#endif

//
// Showing a Horizontal Scroller 
//
- (void) setHasHorizontalScroller: (BOOL)flag;
- (BOOL) hasHorizontalScroller;

//
// Setting the NSBrowser's Appearance 
//
- (NSInteger) maxVisibleColumns;
- (CGFloat) minColumnWidth;
- (BOOL) separatesColumns;
- (void) setMaxVisibleColumns: (NSInteger)columnCount;
- (void) setMinColumnWidth: (CGFloat)columnWidth;
- (void) setSeparatesColumns: (BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (CGFloat) columnWidthForColumnContentWidth: (CGFloat)columnContentWidth;
- (CGFloat) columnContentWidthForColumnWidth: (CGFloat)columnWidth;
#endif

//
// Manipulating Columns 
//
- (void) addColumn;
- (NSInteger) columnOfMatrix: (NSMatrix *)matrix;
- (void) displayAllColumns;
- (void) displayColumn: (NSInteger)column;
- (NSInteger) firstVisibleColumn;
- (BOOL) isLoaded;
- (NSInteger) lastColumn;
- (NSInteger) lastVisibleColumn;
- (void) loadColumnZero;
- (NSInteger) numberOfVisibleColumns;
- (void) reloadColumn: (NSInteger)column;
- (void) selectAll: (id)sender;
- (void) selectRow: (NSInteger)row inColumn: (NSInteger)column;
- (NSInteger) selectedColumn;
- (NSInteger) selectedRowInColumn: (NSInteger)column;
- (void) setLastColumn: (NSInteger)column;
- (void) validateVisibleColumns;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (NSIndexPath *) selectionIndexPath;
- (NSArray *) selectionIndexPaths;
- (void) setSelectionIndexPath: (NSIndexPath *)path;
- (void) setSelectionIndexPaths: (NSArray *)paths;
#endif

//
// Manipulating Column Titles 
//
- (void) drawTitle: (NSString *)title
	    inRect: (NSRect)aRect
	  ofColumn: (NSInteger)column;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) drawTitleOfColumn: (NSInteger)column 
		    inRect: (NSRect)aRect;
#endif 
- (BOOL) isTitled;
- (void) setTitled: (BOOL)flag;
- (void) setTitle: (NSString *)aString
	 ofColumn: (NSInteger)column;
- (NSRect) titleFrameOfColumn: (NSInteger)column;
- (CGFloat) titleHeight;
- (NSString *) titleOfColumn: (NSInteger)column;

//
// Scrolling an NSBrowser 
//
- (void) scrollColumnsLeftBy: (NSInteger)shiftAmount;
- (void) scrollColumnsRightBy: (NSInteger)shiftAmount;
- (void) scrollColumnToVisible: (NSInteger)column;
- (void) scrollViaScroller: (NSScroller *)sender;
- (void) updateScroller;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (void) scrollRowToVisible: (NSInteger)row inColumn: (NSInteger)column;
#endif

//
// Event Handling 
//
- (void) doClick: (id)sender;
- (void) doDoubleClick: (id)sender;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (NSInteger) clickedColumn;
- (NSInteger) clickedRow;
#endif

//
// Getting Matrices and Cells 
//
- (id) loadedCellAtRow: (NSInteger)row
	        column: (NSInteger)column;
- (NSMatrix *) matrixInColumn: (NSInteger)column;
- (id) selectedCell;
- (id) selectedCellInColumn: (NSInteger)column;
- (NSArray *) selectedCells;

//
// Getting Column Frames 
//
- (NSRect) frameOfColumn: (NSInteger)column;
- (NSRect) frameOfInsideOfColumn: (NSInteger)column;

//
// Manipulating Paths 
//
- (NSString *) path;
- (NSString *) pathSeparator;
- (NSString *) pathToColumn: (NSInteger)column;
- (BOOL) setPath: (NSString *)path;
- (void) setPathSeparator: (NSString *)aString;

//
// Arranging an NSBrowser's Components 
//
- (void) tile;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
//
// Resizing
//
- (NSBrowserColumnResizingType) columnResizingType;
- (void) setColumnResizingType:(NSBrowserColumnResizingType) type;
- (BOOL) prefersAllColumnUserResizing;
- (void) setPrefersAllColumnUserResizing: (BOOL)flag;
- (CGFloat) widthOfColumn: (NSInteger)column;
- (void) setWidth: (CGFloat)columnWidth ofColumn: (NSInteger)columnIndex;

//
// Autosave names
//
+ (void) removeSavedColumnsWithAutosaveName: (NSString *)name;
- (NSString *) columnsAutosaveName;
- (void) setColumnsAutosaveName: (NSString *)name;
#endif
@end

//
// Controlling the alphanumerical keys behaviour
//
@interface NSBrowser (GNUstepExtensions)
- (BOOL) acceptsAlphaNumericalKeys;
- (void) setAcceptsAlphaNumericalKeys: (BOOL)flag;
- (BOOL) sendsActionOnAlphaNumericalKeys;
- (void) setSendsActionOnAlphaNumericalKeys: (BOOL)flag;
@end

//
// Methods Implemented by the Delegate 
//
@protocol NSBrowserDelegate <NSObject>
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST) && GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSBrowserDelegate)
#endif

- (void) browser: (NSBrowser *)sender createRowsForColumn: (NSInteger)column
  inMatrix: (NSMatrix *)matrix;
/** Returns YES iff */
- (BOOL) browser: (NSBrowser *)sender isColumnValid: (NSInteger)column;
- (NSInteger) browser: (NSBrowser *)sender numberOfRowsInColumn: (NSInteger)column;
- (BOOL) browser: (NSBrowser *)sender selectCellWithString: (NSString *)title
  inColumn: (NSInteger)column;
- (BOOL) browser: (NSBrowser *)sender selectRow: (NSInteger)row inColumn: (NSInteger)column;
- (NSString *) browser: (NSBrowser *)sender titleOfColumn: (NSInteger)column;
- (void) browser: (NSBrowser *)sender
 willDisplayCell: (id)cell
           atRow: (NSInteger)row
          column: (NSInteger)column;
- (void) browserDidScroll: (NSBrowser *)sender;
- (void) browserWillScroll: (NSBrowser *)sender;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (CGFloat) browser: (NSBrowser *)browser
   shouldSizeColumn: (NSInteger)column
      forUserResize: (BOOL)flag
            toWidth: (CGFloat)width;
- (CGFloat) browser: (NSBrowser *)browser
sizeToFitWidthOfColumn: (NSInteger)column;
- (void) browserColumnConfigurationDidChange: (NSNotification *)notification;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (BOOL) browser: (NSBrowser *)browser
canDragRowsWithIndexes: (NSIndexSet *)rowIndexes
        inColumn: (NSInteger)column
       withEvent: (NSEvent *)event;
#endif
@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
APPKIT_EXPORT NSString *NSBrowserColumnConfigurationDidChangeNotification;
#endif

#endif // _GNUstep_H_NSBrowser
