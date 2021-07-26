/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSBrowserCell.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSMatrix.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSRaise.h>

@implementation NSBrowserCell

+(NSImage *)branchImage {
   return [NSImage imageNamed:@"NSBrowserCellArrow"];
}

+(NSImage *)highlightedBranchImage {
   return [NSImage imageNamed:@"NSHighlightedBrowserCellArrow"];
}

// overrides NSCell behavior
-init {
   return [self initTextCell:@""];
}

-(void)dealloc
{
	[_alternateImage release];
	[super dealloc];
}

-(BOOL)isLeaf {
   return _isLeaf;
}

-(BOOL)isLoaded {
   return _isLoaded;
}

/*
-(NSImage *)image {
   NSUnimplementedMethod();
   return nil;
}
*/

-(NSImage *)alternateImage {
   return _alternateImage;
}

-(void)setLeaf:(BOOL)value {
   _isLeaf=value;
}

-(void)setLoaded:(BOOL)value {
   _isLoaded=value;
}

-(void)setImage:(NSImage *)value {
   NSUnimplementedMethod();
}

-(void)setAlternateImage:(NSImage *)value {
   value=[value retain];
   [_alternateImage release];
   _alternateImage=value;
}

-(void)setState:(int)value {
   _state=value;
   _isHighlighted=(_state==NSOnState)?YES:NO;
}

-(void)set {
   [self setState:NSOnState];
   _isHighlighted=YES;
}

-(void)reset {
   [self setState:NSOffState];
   _isHighlighted=NO;
}

-(NSColor *)highlightColorInView:(NSView *)view {
   return [NSColor highlightColor];
}

-(NSImage *)branchImage {
   if([self isLeaf])
    return nil;
   else if([self isHighlighted] || [self state])
    return [NSBrowserCell highlightedBranchImage];
   else
    return [NSBrowserCell branchImage];
}

-(BOOL)wraps {
   return NO;
}

-(NSColor *)highlightColorWithFrame:(NSRect)frame inView:(NSView *)view {
   return [NSColor selectedControlColor];
}


-(void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)control {
   NSAttributedString *title=[self attributedStringValue];
   NSImage            *branchImage=[self branchImage];
   NSSize              branchImageSize=(branchImage==nil)?NSMakeSize(0,0):[branchImage size];
   NSPoint             branchImageOrigin=frame.origin;
   NSImage            *image=[self image];
   NSSize              imageSize=(image==nil)?NSMakeSize(0,0):[image size];
   NSPoint             imageOrigin=frame.origin;
   NSSize              titleSize=[title size];
   NSRect              titleRect=frame;
   BOOL                drawBranchImage=YES,drawTitle=YES,drawImage=YES;

   branchImageOrigin.x=frame.origin.x+(frame.size.width-branchImageSize.width);
   branchImageOrigin.y+=floor((frame.size.height-branchImageSize.height)/2);

   imageOrigin.x=frame.origin.x;
   imageOrigin.y+=floor((frame.size.height-imageSize.height)/2);

   titleRect.origin.x+=imageSize.width+2;
   titleRect.size.width-=imageSize.width+2;
   titleRect.origin.y+=floor((titleRect.size.height-titleSize.height)/2);
   titleRect.size.height=titleSize.height;

   titleRect.size.width-=(branchImageSize.width+2);

   if([self isHighlighted] || [self state]){
    /* check for length to avoid exception */
    if([title length]>0 && [title attribute:NSForegroundColorAttributeName atIndex:0 effectiveRange:NULL]==nil){
     NSMutableAttributedString *change=[[title mutableCopy] autorelease];

     [change addAttribute:NSForegroundColorAttributeName value:[NSColor selectedControlTextColor] range:NSMakeRange(0,[title length])];
     title=change;
    }
    NSColor *color=[self highlightColorWithFrame:frame inView:control];
    [color setFill];
   }
   else{
    [[NSColor controlBackgroundColor] setFill];
   }

   NSRectFill(frame);

   if(drawTitle)
    [title _clipAndDrawInRect:titleRect];

    if(drawBranchImage){
        [branchImage drawInRect:NSMakeRect(branchImageOrigin.x,branchImageOrigin.y,branchImageSize.width,branchImageSize.height) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];

    }
    
    if(drawImage){
       [image drawInRect:NSMakeRect(imageOrigin.x,imageOrigin.y,imageSize.width,imageSize.height) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
    }
}

@end
