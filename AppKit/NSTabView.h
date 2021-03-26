/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSCell.h>
#import <AppKit/NSView.h>

@class NSTabViewItem, NSFont;

typedef enum {
    NSTopTabsBezelBorder,
    NSLeftTabsBezelBorder,
    NSBottomTabsBezelBorder,
    NSRightTabsBezelBorder,
    NSNoTabsBezelBorder,
    NSNoTabsLineBorder,
    NSNoTabsNoBorder
} NSTabViewType;

@interface NSTabView : NSView {
    NSMutableArray *_items;
    NSTabViewItem *_selectedItem;
    NSFont *_font;
    NSTabViewType _type;
    BOOL _allowsTruncatedLabels;
    id _delegate;
    BOOL _drawsBackground;
    NSControlSize _controlSize;
}

- delegate;

- (NSSize)minimumSize;
- (NSRect)contentRect;

- (NSFont *)font;
- (NSTabViewType)tabViewType;
- (BOOL)drawsBackground;
- (BOOL)allowsTruncatedLabels;

- (void)setDelegate:delegate;
- (void)setFont:(NSFont *)font;
- (void)setTabViewType:(NSTabViewType)type;
- (void)setDrawsBackground:(BOOL)flag;
- (void)setAllowsTruncatedLabels:(BOOL)flag;

- (int)numberOfTabViewItems;
- (NSArray *)tabViewItems;
- (NSTabViewItem *)tabViewItemAtIndex:(int)index;
- (NSTabViewItem *)tabViewItemAtPoint:(NSPoint)point;
- (int)indexOfTabViewItem:(NSTabViewItem *)item;
- (int)indexOfTabViewItemWithIdentifier:identifier;

- (void)addTabViewItem:(NSTabViewItem *)item;
- (void)removeTabViewItem:(NSTabViewItem *)item;
- (void)insertTabViewItem:(NSTabViewItem *)item atIndex:(int)index;

- (NSTabViewItem *)selectedTabViewItem;
- (void)selectTabViewItem:(NSTabViewItem *)item;
- (void)selectTabViewItemAtIndex:(int)index;
- (void)selectTabViewItemWithIdentifier:identifier;
- (void)selectFirstTabViewItem:sender;
- (void)selectLastTabViewItem:sender;
- (void)takeSelectedTabViewItemFromSender:sender;

- (void)_itemViewDidChange:(NSTabViewItem *)item;

@end

@interface NSObject (NSTabView_delegate)
- (void)tabViewDidChangeNumberOfTabViewItems:(NSTabView *)tabView;

- (BOOL)tabView:(NSTabView *)tabView shouldSelectTabViewItem:(NSTabViewItem *)item;
- (void)tabView:(NSTabView *)tabView willSelectTabViewItem:(NSTabViewItem *)item;
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)item;
@end
