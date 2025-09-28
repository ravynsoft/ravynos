/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSToolbarCustomizationView.h>
#import <AppKit/NSToolbar.h>
#import <AppKit/NSToolbarItem.h>
#import <AppKit/NSToolbarView.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSDragging.h>

@interface NSToolbar (Private)
-(NSArray *)_allowedToolbarItems;
-(NSArray *)_defaultToolbarItems;
@end

@interface NSToolbarItem (NSToolbarItem_private)
-(NSSize)sizeForSizeMode:(NSToolbarSizeMode)sizeMode displayMode:(NSToolbarDisplayMode)displayMode;
-(void)drawInRect:(NSRect)rect highlighted:(BOOL)highlighted;
@end

@implementation NSToolbarCustomizationView

-(void)dealloc {
   [_toolbar release];
   [super dealloc];
}

-(NSToolbar *)toolbar {
    return _toolbar;
}

-(void)setToolbar:(NSToolbar *)toolbar {
   toolbar=[toolbar retain];
   [_toolbar release];
   _toolbar=toolbar;
}

-(void)setDefaultSetView:(BOOL)flag {
   _isDefaultSetView = flag;
}

-(BOOL)isDefaultSetView {
   return _isDefaultSetView;
}

-(NSArray *)toolbarItems {
   if(_isDefaultSetView)
    return [_toolbar _defaultToolbarItems];
   else
    return [_toolbar _allowedToolbarItems];
}

-(BOOL)isFlipped {
   return YES;
}

-(void)layoutFrames:(NSRect *)frames count:(NSUInteger)count {
   NSArray  *items=[self toolbarItems];
   NSInteger i;
   NSRect    bounds=[self bounds];
   CGFloat   maxx=NSMaxX(bounds);
   CGFloat   padding=4;
   CGFloat   nextx=bounds.origin.x;
   CGFloat   nexty=bounds.origin.y;
   CGFloat   currentHeight=0;
   
   for(i=0;i<count;i++){
    NSToolbarItem *item=[items objectAtIndex:i];
    NSSize         size=[item sizeForSizeMode:NSToolbarSizeModeDefault displayMode:NSToolbarDisplayModeDefault];
    
    if(nextx+size.width>maxx && currentHeight>0){
     nextx=bounds.origin.x;
     nexty+=currentHeight+padding;
     currentHeight=0;
    }
    
    frames[i].origin.x=nextx;
    frames[i].origin.y=nexty;
    frames[i].size=size;
    nextx+=size.width+padding;

    currentHeight=MAX(size.height,currentHeight);
   }
   
}

-(NSSize)desiredSize {
   NSRect     unionRect=NSZeroRect;
   NSArray   *items=[self toolbarItems];
   NSUInteger i,count=[items count];
   NSRect     frames[count];
   
   [self layoutFrames:frames count:count];
   for(i=0;i<count;i++){
    if(i==0)
     unionRect=frames[i];
    else
     unionRect=NSUnionRect(unionRect,frames[i]);
   }
   
   return unionRect.size;
}

-(void)drawRect:(NSRect)ignore {
   NSArray   *items=[self toolbarItems];
   NSUInteger i,count=[items count];
   NSRect     frames[count];
   
   [self layoutFrames:frames count:count];
   for(i=0;i<count;i++){
    NSToolbarItem *item=[items objectAtIndex:i];
    [item drawInRect:frames[i] highlighted:NO];
   }
 }


-(void)mouseDown:(NSEvent *)event {
   NSPasteboard *pasteboard=[NSPasteboard pasteboardWithName:NSDragPboard];
   NSImage      *image=nil;
   NSData       *data=nil;
   
   if (_isDefaultSetView) {
    image=[[[NSImage alloc] initWithSize:[self bounds].size] autorelease];
    data=[NSArchiver archivedDataWithRootObject:[[_toolbar _defaultToolbarItems] valueForKey:@"itemIdentifier"]];
        
    [image setCachedSeparately:YES];
    [image lockFocus];
    [self drawRect:[self bounds]];        
    [image unlockFocus];
        
   }
   else {
    NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
    NSArray   *items=[self toolbarItems];
    NSUInteger i,count=[items count];
    NSRect     frames[count];
   
    [self layoutFrames:frames count:count];
    for(i=0;i<count;i++)
     if(NSPointInRect(point,frames[i])){
      NSToolbarItem *item=[items objectAtIndex:i];
      
      image=[[[NSImage alloc] initWithSize:frames[i].size] autorelease];
      data=[NSArchiver archivedDataWithRootObject:[item itemIdentifier]];
      
      [image setCachedSeparately:YES];
      [image lockFocus];
      [item drawInRect:frames[i] highlighted:NO];
      [image unlockFocus];
      break;
    }
   }
   
   if(data!=nil){
    [pasteboard declareTypes:[NSArray arrayWithObject:NSToolbarItemIdentifierPboardType] owner:nil];
    [pasteboard setData:data forType:NSToolbarItemIdentifierPboardType];
                
    [self dragImage:image at:NSMakePoint(0,0) offset:NSMakeSize(0,0) event:event pasteboard:pasteboard  source:self slideBack:YES];
   }
}
 
- (unsigned)draggingSourceOperationMaskForLocal:(BOOL)isLocal 
{
    return NSDragOperationCopy;
}

@end

NSString * const NSToolbarItemIdentifierPboardType = @"NSToolbarItemIdentifierPboardType";
