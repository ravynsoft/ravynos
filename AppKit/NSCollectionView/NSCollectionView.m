/* Copyright (c) 2009 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSCollectionView.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSRaise.h>

@implementation NSCollectionView

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
   }
   else
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   
   return self;
}

-(NSArray *)content {
   return _content;
}

-(NSCollectionViewItem *)itemPrototype {
   return _itemPrototype;
}

-(BOOL)isSelectable {
   return _isSelectable;
}

-(NSSize)minItemSize {
   return _minItemSize;
}

-(NSSize)maxItemSize {
   return _maxItemSize;
}

-(NSUInteger)maxNumberOfRows {
   return _maxNumberOfRows;
}

-(NSUInteger)maxNumberOfColumns {
   return _maxNumberOfColumns;
}

-(NSArray *)backgroundColors {
   return _backgroundColors;
}

-(BOOL)allowsMultipleSelection {
   return _allowsMultipleSelection;
}

-(NSIndexSet *)selectionIndexes {
   return _selectionIndexes;
}

-(void)setContent:(NSArray *)value {
   value=[value retain];
   [_content release];
   _content=value;
}

-(void)setItemPrototype:(NSCollectionViewItem *)value {
   value=[value retain];
   [_itemPrototype release];
   _itemPrototype=value;
}

-(void)setSelectable:(BOOL)value {
   _isSelectable=value;
}

-(void)setMinItemSize:(NSSize)value {
   _minItemSize=value;
}

-(void)setMaxItemSize:(NSSize)value {
   _maxItemSize=value;
}

-(void)setMaxNumberOfRows:(NSUInteger)value {
   _maxNumberOfRows=value;
}

-(void)setMaxNumberOfColumns:(NSUInteger)value {
   _maxNumberOfColumns=value;
}

-(void)setBackgroundColors:(NSArray *)value {
   value=[value copy];
   [_backgroundColors release];
   _backgroundColors=value;
}

-(void)setAllowsMultipleSelection:(BOOL)value {
   _allowsMultipleSelection=value;
}

-(void)setSelectionIndexes:(NSIndexSet *)value {
   value=[value copy];
   [_selectionIndexes release];
   _selectionIndexes=value;
}

-(BOOL)isFirstResponder {
   return ([[self window] firstResponder]==self)?YES:NO;
}

-(NSCollectionViewItem *)newItemForRepresentedObject:object {
   NSUnimplementedMethod();
   return nil;
}

@end
