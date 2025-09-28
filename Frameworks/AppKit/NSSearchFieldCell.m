/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSSearchFieldCell.h>
#import <AppKit/NSButtonCell.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSRaise.h>

@implementation NSSearchFieldCell

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];
   _searchButtonCell=[[NSButtonCell alloc] initImageCell:[NSImage imageNamed:@"NSSearchGlass"]];
   [_searchButtonCell setImageScaling:NSImageScaleProportionallyUpOrDown];
   _cancelButtonCell=[[NSButtonCell alloc] initImageCell:[NSImage imageNamed:@"NSStopProgressFreestandingTemplate"]];
   [_cancelButtonCell setImageScaling:NSImageScaleProportionallyUpOrDown];
   return self;  
}

-initTextCell:(NSString *)string {
   [super initTextCell:string];
   _searchButtonCell=[[NSButtonCell alloc] initImageCell:[NSImage imageNamed:@"NSSearchGlass"]];
   [_searchButtonCell setImageScaling:NSImageScaleProportionallyUpOrDown];
   _cancelButtonCell=[[NSButtonCell alloc] initImageCell:[NSImage imageNamed:@"NSStopProgressFreestandingTemplate"]];
   [_cancelButtonCell setImageScaling:NSImageScaleProportionallyUpOrDown];
   return self;
}

-(void)dealloc {
   [_searchButtonCell release];
   [_cancelButtonCell release];
   [super dealloc];
}

-(NSArray *)recentSearches {
   return _recentSearches;
}

-(NSString *)recentsAutosaveName {
   return _autosaveName;
}

-(int)maximumRecents {
   return _maximumRecents;
}

-(BOOL)sendsWholeSearchString {
   return _sendsWholeSearchString;
}

-(BOOL)sendsSearchStringImmediately {
   return _sendsSearchStringImmediately;
}

-(NSButtonCell *)searchButtonCell {
   return _searchButtonCell;
}

-(NSButtonCell *)cancelButtonCell {
   return _cancelButtonCell;
}

-(NSMenu *)searchMenuTemplate {
   return _searchMenuTemplate;
}

-(void)setRecentSearches:(NSArray *)searches {
   searches=[searches copy];
   [_recentSearches release];
   _recentSearches=searches;
}

-(void)setRecentsAutosaveName:(NSString *)name {
   name=[name copy];
   [_autosaveName release];
   _autosaveName=name;
}

-(void)setMaximumRecents:(int)value {
   _maximumRecents=value;
}

-(void)setSendsWholeSearchString:(BOOL)flag {
   _sendsWholeSearchString=flag;
}

-(void)setSendsSearchStringImmediately:(BOOL)flag {
   _sendsSearchStringImmediately=flag;
}

-(void)setSearchButtonCell:(NSButtonCell *)cell {
   cell=[cell retain];
   [_searchButtonCell release];
   _searchButtonCell=cell;
}

-(void)setCancelButtonCell:(NSButtonCell *)cell {
   cell=[cell retain];
   [_cancelButtonCell release];
   _cancelButtonCell=cell;
}

-(void)setSearchMenuTemplate:(NSMenu *)menu {
   menu=[menu retain];
   [_searchMenuTemplate release];
   _searchMenuTemplate=menu;
}

-(NSRect)searchTextRectForBounds:(NSRect)rect {
   return [self titleRectForBounds:rect];
}

-(NSRect)searchButtonRectForBounds:(NSRect)rect {
   NSRect result=[self titleRectForBounds:rect];
   
   result.size.width=result.size.height;
   result.origin.x-=result.size.width;
   
   return result;
}

-(NSRect)cancelButtonRectForBounds:(NSRect)rect {
   NSRect result=[self titleRectForBounds:rect];
   
   result.origin.x+=result.size.width;
   result.size.width=result.size.height;
   
   return result;
}

-(void)resetCancelButtonCell {
   NSUnimplementedMethod();
}

-(void)resetSearchButtonCell {
   NSUnimplementedMethod();
}

-(NSRect)titleRectForBounds:(NSRect)rect {
   CGFloat radius=rect.size.height/2;
   NSRect  result=[super titleRectForBounds:rect];
   
   result=NSInsetRect(result,radius,0);
  
   return result;
}

-(BOOL)trackMouse:(NSEvent *)event inRect:(NSRect)frame ofView:(NSView *)view untilMouseUp:(BOOL)untilMouseUp {
   if([_cancelButtonCell trackMouse:event inRect:[self cancelButtonRectForBounds:frame] ofView:view untilMouseUp:YES]){
    [(NSControl *)view setStringValue:@""];

    if ([view respondsToSelector:@selector(selectText:)])
      [(NSTextField *)view selectText:nil];
    return YES;
   }

   return [super trackMouse:event inRect:frame ofView:view untilMouseUp:untilMouseUp];
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)control {
   [super drawWithFrame:frame inView:control];
   
   [_searchButtonCell drawWithFrame:[self searchButtonRectForBounds:frame] inView:control];
   if([[self attributedStringValue] length]>0)
    [_cancelButtonCell drawWithFrame:[self cancelButtonRectForBounds:frame] inView:control];
}

@end
