/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSComboBox.h>
#import <AppKit/NSComboBoxCell.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSTextView.h>

NSString * const NSComboBoxSelectionDidChangeNotification=@"NSComboBoxSelectionDidChangeNotification";
NSString * const NSComboBoxSelectionIsChangingNotification=@"NSComboBoxSelectionIsChangingNotification";
NSString * const NSComboBoxWillDismissNotification=@"NSComboBoxWillDismissNotification";
NSString * const NSComboBoxWillPopUpNotification=@"NSComboBoxWillPopUpNotification";

@implementation NSComboBox

-dataSource {
   return [[self cell] dataSource];
}

-(BOOL)usesDataSource {
   return [[self cell] usesDataSource];
}

-(BOOL)isButtonBordered {
   return [[self cell] isButtonBordered];
}

-(float)itemHeight {
   return [[self cell] itemHeight];
}

-(BOOL)hasVerticalScroller {
   return [[self cell] hasVerticalScroller];
}

-(NSSize)intercellSpacing {
   return [[self cell] intercellSpacing];
}

-(BOOL)completes {
   return [[self cell] completes];
}

-(int)numberOfVisibleItems {
   return [[self cell] numberOfVisibleItems];
}

-(void)setDataSource:value {
   [[self cell] setDataSource:value];
   [self setNeedsDisplay:YES];
}

-(void)setUsesDataSource:(BOOL)value {
   [[self cell] setUsesDataSource:value];
   [self setNeedsDisplay:YES];
}

-(void)setButtonBordered:(BOOL)value {
   [[self cell] setButtonBordered:value];
   [self setNeedsDisplay:YES];
}

-(void)setItemHeight:(float)value {
   [[self cell] setItemHeight:value];
   [self setNeedsDisplay:YES];
}

-(void)setHasVerticalScroller:(BOOL)value {
   [[self cell] setHasVerticalScroller:value];
   [self setNeedsDisplay:YES];
}

-(void)setIntercellSpacing:(NSSize)value {
   [[self cell] setIntercellSpacing:value];
   [self setNeedsDisplay:YES];
}

-(void)setCompletes:(BOOL)completes {
   [[self cell] setCompletes:completes];
}

-(void)setNumberOfVisibleItems:(int)value {
   [[self cell] setNumberOfVisibleItems:value];
   [self setNeedsDisplay:YES];
}

-(int)numberOfItems {
   return [[self cell] numberOfItems];
}

-(NSArray *)objectValues {
   return [[self cell] objectValues];
}

-itemObjectValueAtIndex:(int)index {
   return [[self cell] itemObjectValueAtIndex:index];
}

-(int)indexOfItemWithObjectValue:(id)object {
   return [[self cell] indexOfItemWithObjectValue:object];
}

-(void)addItemWithObjectValue:(id)object {
   [[self cell] addItemWithObjectValue:object];
}

-(void)addItemsWithObjectValues:(NSArray *)objects {
   [[self cell] addItemsWithObjectValues:objects];
}

-(void)removeAllItems {
   [[self cell] removeAllItems];
}

-(void)removeItemAtIndex:(int)index {
   [[self cell] removeItemAtIndex:index];
}

-(void)removeItemWithObjectValue:value {
   [[self cell] removeItemWithObjectValue:value];
}

-(void)insertItemWithObjectValue:value atIndex:(int)index {
   [[self cell] insertItemWithObjectValue:value atIndex:index];
}

-(int)indexOfSelectedItem {
   return [[self cell] indexOfSelectedItem];
}

-objectValueOfSelectedItem {
   return [[self cell] objectValueOfSelectedItem];
}

-(void)selectItemAtIndex:(int)index {
   [[self cell] selectItemAtIndex:index];
}

-(void)selectItemWithObjectValue:value {
   [[self cell] selectItemWithObjectValue:value];
}

-(void)deselectItemAtIndex:(int)index {
   [[self cell] deselectItemAtIndex:index];
}

-(void)scrollItemAtIndexToTop:(int)index {
   [[self cell] scrollItemAtIndexToTop:index];
}

-(void)scrollItemAtIndexToVisible:(int)index {
   [[self cell] scrollItemAtIndexToVisible:index];
}

-(void)noteNumberOfItemsChanged {
   [[self cell] noteNumberOfItemsChanged];
}

-(void)reloadData {
   [[self cell] reloadData];
}

-(void)mouseDown:(NSEvent *)event {
   if(![[self cell] trackMouse:event inRect:[self bounds] ofView:self untilMouseUp:YES])
    [super mouseDown:event];
}

// hrm.. since the field editor has focus, we can use this delegate method to effect keyboard
// navigation.
// ...i also thought it might be fun to preserve the half-typed text in objectValue index 0...
- (BOOL)textView:(NSTextView *)textView doCommandBySelector:(SEL)selector {
    int index = [[self cell] indexOfItemWithObjectValue:[self objectValue]];
    id objectCache = nil;

    if ([textView rangeForUserCompletion].location != NSNotFound)
        return NO;

    if (index == NSNotFound) {
        objectCache = [[self objectValue] retain];
        index = -1;
    }
    
    if (selector == @selector(moveUp:)) {
        [[self cell] selectItemAtIndex:index-1];
        [_currentEditor setString:[[self cell] objectValue]];
        if (objectCache != nil)
            [[self cell] insertItemWithObjectValue:[objectCache autorelease] atIndex:0];
        return YES;
    }
    else if (selector == @selector(moveDown:)) {
        [[self cell] selectItemAtIndex:index+1];
        [_currentEditor setString:[[self cell] objectValue]];
        if (objectCache != nil)
            [[self cell] insertItemWithObjectValue:[objectCache autorelease] atIndex:0];
        return YES;
    }

    return NO;
}

- (NSArray *)textView:(NSTextView *)textView completions:(NSArray *)words forPartialWordRange:(NSRange)range indexOfSelectedItem:(int *)index {
    NSString *string = [[self cell] completedString:[[textView string] substringWithRange:range]];

//    NSLog(@"NSComboBox delegate OK: %@", string);

    if (string != nil) {
        *index = 0;
        return [NSArray arrayWithObject:string];
    }
    else {
        *index = -1;
        return nil;
    }
}

@end
