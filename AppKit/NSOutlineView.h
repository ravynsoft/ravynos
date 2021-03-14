/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSTableView.h>
#import <AppKit/NSDragging.h>
#import <AppKit/AppKitExport.h>

@class NSTableColumn, NSButtonCell;

APPKIT_EXPORT NSString *const NSOutlineViewItemWillExpandNotification;
APPKIT_EXPORT NSString *const NSOutlineViewItemDidExpandNotification;
APPKIT_EXPORT NSString *const NSOutlineViewItemWillCollapseNotification;
APPKIT_EXPORT NSString *const NSOutlineViewItemDidCollapseNotification;

APPKIT_EXPORT NSString *const NSOutlineViewColumnDidMoveNotification;
APPKIT_EXPORT NSString *const NSOutlineViewColumnDidResizeNotification;

APPKIT_EXPORT NSString *const NSOutlineViewSelectionDidChangeNotification;
APPKIT_EXPORT NSString *const NSOutlineViewSelectionIsChangingNotification;

@interface NSOutlineView : NSTableView {
    NSTableColumn *_outlineTableColumn;
    NSMapTable *_rowToItem;
    NSMapTable *_itemToRow;
    NSMapTable *_itemToParent;
    NSMapTable *_itemToLevel;
    NSMapTable *_itemToExpansionState;
    NSMapTable *_itemToNumberOfChildren;

    unsigned _numberOfCachedRows;
    NSButtonCell *_markerCell;
    NSSize _markerSize;
    id _clickedItem;
    float _widestColumnWidth;

    float _indentationPerLevel;
    BOOL _indentationMarkerFollowsCell;
    BOOL _autoresizesOutlineColumn;
    BOOL _autosaveExpandedItems;
    float _editingCellPadding;
}

- (NSTableColumn *)outlineTableColumn;

- itemAtRow:(int)row;
- (int)rowForItem:item;
- parentForItem:item;

- (BOOL)isExpandable:item;
- (int)levelForItem:item;
- (int)levelForRow:(int)row;
- (BOOL)isItemExpanded:item;
- (float)indentationPerLevel;

- (BOOL)autoresizesOutlineColumn;
- (BOOL)indentationMarkerFollowsCell;
- (BOOL)autosaveExpandedItems;

- (void)setOutlineTableColumn:(NSTableColumn *)tableColumn;

- (void)setIndentationPerLevel:(float)value;
- (void)setAutoresizesOutlineColumn:(BOOL)flag;
- (void)setIndentationMarkerFollowsCell:(BOOL)flag;
- (void)setAutosaveExpandedItems:(BOOL)flag;

- (void)expandItem:item expandChildren:(BOOL)expandChildren;
- (void)expandItem:item;
- (void)collapseItem:item collapseChildren:(BOOL)collapseChildren;
- (void)collapseItem:item;
- (void)reloadItem:item reloadChildren:(BOOL)reloadChildren;
- (void)reloadItem:item;

- (void)setDropItem:item dropChildIndex:(int)index;
- (BOOL)shouldCollapseAutoExpandedItemsForDeposited:(BOOL)collapse;

- (NSRect)frameOfOutlineCellAtRow:(NSInteger)row;

@end

@interface NSObject (NSOutlineView_dataSource)
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:item;
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:item;
- outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:item;
- outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:item;
- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:object forTableColumn:(NSTableColumn *)tableColumn byItem:item;
@end

@interface NSObject (NSOutlineView_delegate)
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldExpandItem:item;
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:item;
- (BOOL)selectionShouldChangeInOutlineView:(NSOutlineView *)outlineView;
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:item;
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:item;
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectTableColumn:(NSTableColumn *)tableColumn;
- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:cell forTableColumn:(NSTableColumn *)tableColumn item:item;
- (void)outlineView:(NSOutlineView *)outlineView willDisplayOutlineCell:cell forTableColumn:(NSTableColumn *)tableColumn item:item;
@end

@interface NSObject (NSOutlineView_notifications)
- (void)outlineViewItemWillExpand:(NSNotification *)note;
- (void)outlineViewItemDidExpand:(NSNotification *)note;
- (void)outlineViewItemWillCollapse:(NSNotification *)note;
- (void)outlineViewItemDidCollapse:(NSNotification *)note;
- (void)outlineViewColumnDidMove:(NSNotification *)note;
- (void)outlineViewColumnDidResize:(NSNotification *)note;
- (void)outlineViewSelectionIsChanging:(NSNotification *)note;
- (void)outlineViewSelectionDidChange:(NSNotification *)note;
@end

@interface NSObject (NSOutlineView_cellSizing)
- (float)outlineView:(NSOutlineView *)outlineView widthOfCell:cell forTableColumn:(NSTableColumn *)tableColumn byItem:item;
@end
