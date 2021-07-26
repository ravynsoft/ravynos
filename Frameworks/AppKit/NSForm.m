/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSForm.h>
#import <AppKit/NSFormCell.h>
#import <AppKit/NSTextView.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSRaise.h>

@implementation NSForm

-(void)resizeWithOldSuperviewSize:(NSSize)oldSize {
   [super resizeWithOldSuperviewSize:oldSize];

   _cellSize.width=[self bounds].size.width;
}

-(void)calcSize {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   float    maxTitleWidth=0;

   for(i=0;i<count;i++){
    NSFormCell         *cell=[cells objectAtIndex:i];
    NSAttributedString *title=[cell attributedTitle];
    float               width=[title size].width;

    if(width>maxTitleWidth)
     maxTitleWidth=width;
   }

   maxTitleWidth=ceil(maxTitleWidth);
   maxTitleWidth+=1;

   for(i=0;i<count;i++){
    NSFormCell *cell=[cells objectAtIndex:i];
    [cell setTitleWidth:maxTitleWidth];
   }
}

-cellAtIndex:(int)index {
   return [[self cells] objectAtIndex:index];
}

-(int)indexOfCellWithTag:(int)tag {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    if([[cells objectAtIndex:i] tag]==tag)
     return i;
   
   return -1;
}

-(int)indexOfSelectedItem {
   return [self selectedRow];
}


-(void)setBordered:(BOOL)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setBordered:value];
    
   [self setNeedsDisplay:YES];
}

-(void)setBezeled:(BOOL)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setBezeled:value];

   [self setNeedsDisplay:YES];
}

-(void)setEntryWidth:(float)value {
   NSSize size=[self cellSize];
   
   size.width=value;
   [self setCellSize:size];
}

-(void)setInterlineSpacing:(float)value {
   [self setIntercellSpacing:NSMakeSize(0,value)];
}

-(void)setTitleAlignment:(NSTextAlignment)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setTitleAlignment:value];
    
   [self setNeedsDisplay:YES];
}

-(void)setTitleFont:(NSFont *)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setTitleFont:value];
    
   [self setNeedsDisplay:YES];
}

-(void)setTitleBaseWritingDirection:(NSWritingDirection)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setTitleBaseWritingDirection:value];
    
   [self setNeedsDisplay:YES];
}

-(void)setTextAlignment:(NSTextAlignment)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setAlignment:value];
    
   [self setNeedsDisplay:YES];
}

-(void)setTextFont:(NSFont *)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setFont:value];
    
   [self setNeedsDisplay:YES];
}

-(void)setTextBaseWritingDirection:(NSWritingDirection)value {
   NSArray *cells=[self cells];
   int      i,count=[cells count];
   
   for(i=0;i<count;i++)
    [[cells objectAtIndex:i] setBaseWritingDirection:value];
    
   [self setNeedsDisplay:YES];
}

-(NSFormCell *)addEntry:(NSString *)title {   
   [self addRow];
   [[[self cells] lastObject] setTitle:title];
   
   return [[self cells] lastObject];
}

-(NSFormCell *)insertEntry:(NSString *)title atIndex:(int)index {
   [self insertRow:index];
   [[self cellAtRow:index column:0] setTitle:title];
   
   return [self cellAtRow:index column:0];
}

-(void)removeEntryAtIndex:(int)index {
   [self removeRow:index];
}

-(void)selectTextAtIndex:(int)index {
   NSUnimplementedMethod();
}

-(void)drawCellAtIndex:(int)index {
   [self drawCellAtRow:index column:0];
}

@end
