/* Copyright (c) 2009 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSView.h>
#import <AppKit/NSCollectionViewItem.h>

@interface NSCollectionView : NSView {
    NSArray *_content;
    NSCollectionViewItem *_itemPrototype;
    BOOL _isSelectable;
    NSSize _minItemSize;
    NSSize _maxItemSize;
    NSUInteger _maxNumberOfRows;
    NSUInteger _maxNumberOfColumns;
    NSArray *_backgroundColors;
    BOOL _allowsMultipleSelection;
    NSIndexSet *_selectionIndexes;
}

- (NSArray *)content;
- (NSCollectionViewItem *)itemPrototype;

- (BOOL)isSelectable;

- (NSSize)minItemSize;
- (NSSize)maxItemSize;

- (NSUInteger)maxNumberOfRows;
- (NSUInteger)maxNumberOfColumns;

- (NSArray *)backgroundColors;
- (BOOL)allowsMultipleSelection;
- (NSIndexSet *)selectionIndexes;

- (void)setContent:(NSArray *)value;
- (void)setItemPrototype:(NSCollectionViewItem *)value;
- (void)setSelectable:(BOOL)value;

- (void)setMinItemSize:(NSSize)value;
- (void)setMaxItemSize:(NSSize)value;
- (void)setMaxNumberOfRows:(NSUInteger)value;
- (void)setMaxNumberOfColumns:(NSUInteger)value;

- (void)setBackgroundColors:(NSArray *)value;
- (void)setAllowsMultipleSelection:(BOOL)value;
- (void)setSelectionIndexes:(NSIndexSet *)value;

- (BOOL)isFirstResponder;
- (NSCollectionViewItem *)newItemForRepresentedObject:object;

@end
