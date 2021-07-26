/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSTextFieldCell.h>

@class NSButtonCell;

enum {
    NSSearchFieldRecentsTitleMenuItemTag = 1000,
    NSSearchFieldRecentsMenuItemTag = 1001,
    NSSearchFieldClearRecentsMenuItemTag = 1002,
    NSSearchFieldNoRecentsMenuItemTag = 1003,
};

@interface NSSearchFieldCell : NSTextFieldCell {
    NSArray *_recentSearches;
    NSString *_autosaveName;
    int _maximumRecents;
    BOOL _sendsWholeSearchString;
    BOOL _sendsSearchStringImmediately;
    NSButtonCell *_searchButtonCell;
    NSButtonCell *_cancelButtonCell;
    NSMenu *_searchMenuTemplate;
}

- (NSArray *)recentSearches;
- (NSString *)recentsAutosaveName;
- (int)maximumRecents;
- (BOOL)sendsWholeSearchString;
- (BOOL)sendsSearchStringImmediately;
- (NSButtonCell *)searchButtonCell;
- (NSButtonCell *)cancelButtonCell;
- (NSMenu *)searchMenuTemplate;

- (void)setRecentSearches:(NSArray *)searches;
- (void)setRecentsAutosaveName:(NSString *)name;
- (void)setMaximumRecents:(int)value;
- (void)setSendsWholeSearchString:(BOOL)flag;
- (void)setSendsSearchStringImmediately:(BOOL)flag;
- (void)setSearchButtonCell:(NSButtonCell *)cell;
- (void)setCancelButtonCell:(NSButtonCell *)cell;
- (void)setSearchMenuTemplate:(NSMenu *)menu;

- (NSRect)searchTextRectForBounds:(NSRect)rect;
- (NSRect)searchButtonRectForBounds:(NSRect)rect;
- (NSRect)cancelButtonRectForBounds:(NSRect)rect;

- (void)resetCancelButtonCell;
- (void)resetSearchButtonCell;

@end
