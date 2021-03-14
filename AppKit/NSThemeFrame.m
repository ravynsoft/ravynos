/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSThemeFrame.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSMenuView.h>
#import <AppKit/NSToolbarView.h>
#import <AppKit/NSMainMenuView.h>

@interface NSWindow(private)
-(BOOL)hasMainMenu;
+(BOOL)hasMainMenuForStyleMask:(NSUInteger)styleMask;
@end

@implementation NSThemeFrame

-(BOOL)isOpaque {
   return YES;
}

-(NSWindowBorderType)windowBorderType {
   return _borderType;
}

-(void)setWindowBorderType:(NSWindowBorderType)borderType {
   _borderType = borderType;
   [self setNeedsDisplay:YES];
}

-(void)drawRect:(NSRect)rect {
   NSRect bounds=[self bounds];
   float cheatSheet = 0;

   [[[self window] backgroundColor] setFill];
   NSRectFill(bounds);
   
    switch(_borderType){
        case NSNoBorder:
            break;
            
        case NSWindowToolTipBorderType:
            [[NSColor blackColor] setStroke];
            NSFrameRect(bounds);
            bounds = NSInsetRect(bounds, 1, 1);
            cheatSheet = 1;
            break;
                
        case NSWindowSheetBorderType:
            NSDrawButton(bounds,bounds);
            bounds = NSInsetRect(bounds, 2, 2);
            cheatSheet = 2;
            break;
   }
    
   if([[self window] isSheet])
    bounds.size.height += cheatSheet;

   [[[self window] backgroundColor] setFill];
   NSRectFill([[[self window] contentView] frame]);
}

-(void)resizeSubviewsWithOldSize:(NSSize)oldSize {
   NSView *menuView=nil;
   NSToolbarView *toolbarView=nil;
   NSView *contentView=nil;
   
// tile the subviews, when/if we add titlebars and such do it here
   for(NSView *view in _subviews){
    if([view isKindOfClass:[NSMenuView class]])
     menuView=view;
    else if([view isKindOfClass:[NSToolbarView class]])
     toolbarView=(NSToolbarView *)view;
    else
     contentView=view;
   }
   
   // subtracts menu height but not toolbar height
   NSRect contentFrame=[[[self window] class] contentRectForFrameRect:[self bounds] styleMask:[[self window] styleMask]];

   // If the class thinks there is a menu but the instance does not want an instance
   // we need to add the menu height back to the content view as contentRectForFrameRect subtracts it
   
   if([[[self window] class] hasMainMenuForStyleMask:[[self window] styleMask]]) {
        if(![[self window] hasMainMenu])
            contentFrame.size.height+=[NSMainMenuView menuHeight];
   }

   NSRect menuFrame=(menuView!=nil)?[menuView frame]:NSZeroRect;
   NSRect toolbarFrame=(toolbarView!=nil)?[toolbarView frame]:NSZeroRect;

   menuFrame.origin.y=NSMaxY(contentFrame);
   menuFrame.origin.x=contentFrame.origin.x;
   menuFrame.size.width=contentFrame.size.width;
   [menuView setFrame:menuFrame];
   
   toolbarFrame.origin.y=NSMaxY(contentFrame)-toolbarFrame.size.height;
   toolbarFrame.origin.x=contentFrame.origin.x;
   toolbarFrame.size.width=contentFrame.size.width;
   
   [toolbarView setFrame:toolbarFrame];
   [toolbarView layoutViews];
   
   contentFrame.size.height-=toolbarFrame.size.height;
   [contentView setFrame:contentFrame];
}

-(void)mouseDown:(NSEvent *)event {
   if(![[self window] isMovableByWindowBackground])
    return;
   
   NSPoint origin=[[self window] frame].origin;
   NSPoint firstLocation=[[self window] convertBaseToScreen:[event locationInWindow]];
   do {
    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];
    
    NSPoint delta=[[self window] convertBaseToScreen:[event locationInWindow]];
    
    delta.x-=firstLocation.x;
    delta.y-=firstLocation.y;
    
    [[self window] setFrameOrigin:NSMakePoint(origin.x+delta.x,origin.y+delta.y)];
    
   }while([event type]!=NSLeftMouseUp);   
}

@end
