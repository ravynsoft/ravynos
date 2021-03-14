/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSToolbarCustomizationPalette.h>
#import <AppKit/NSToolbarCustomizationView.h>
#import <AppKit/NSToolbar.h>
#import <AppKit/NSToolbarView.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSPopUpButton.h>
#import <AppKit/NSButton.h>

@implementation NSToolbarCustomizationPalette

static NSToolbarCustomizationPalette *_nextPanel;

+(void)setPanel:(NSToolbarCustomizationPalette *)panel {
   _nextPanel=panel;
}

+(NSToolbarCustomizationPalette *)toolbarCustomizationPalette {
   NSToolbarCustomizationPalette *result;
   
   [NSBundle loadNibNamed:@"NSToolbarCustomizationPalette" owner:self];
   result=_nextPanel;
   _nextPanel=nil;
   
   return result;
}

-initWithContentRect:(NSRect)contentRect styleMask:(unsigned)styleMask backing:(unsigned)backing defer:(BOOL)defer {
   return [super initWithContentRect:contentRect styleMask:NSDocModalWindowMask backing:backing defer:defer];
}

-(void)dealloc {
   [_toolbar release];
   [super dealloc];
}

-(void)setToolbar:(NSToolbar *)toolbar {
   _toolbar=[toolbar retain];

   int index=[_displayModePopUp indexOfItemWithTag:[toolbar displayMode]];
    
   [_displayModePopUp selectItemAtIndex:(index == -1)?0:index];
   [_smallSizeModeButton setState:([_toolbar sizeMode]==NSToolbarSizeModeSmall)?NSOnState:NSOffState];
   [self setDefaultButtonCell:[_button cell]];

   NSSize oldSize,newSize;
   CGFloat deltaWidth=0,deltaHeight=0;
   
   oldSize=[_allowedItemsView frame].size;
   [_allowedItemsView setToolbar:_toolbar];
   [_allowedItemsView setDefaultSetView:NO];
   newSize=[_allowedItemsView desiredSize];
   
   deltaWidth=newSize.width-oldSize.width;
   deltaHeight=newSize.height-oldSize.height;
   
   oldSize=[_allowedItemsView frame].size;
   [_defaultItemsView setToolbar:_toolbar];
   [_defaultItemsView setDefaultSetView:YES];
   newSize=[_defaultItemsView desiredSize];

   deltaWidth=MAX(deltaWidth,newSize.width-oldSize.width);
// FIXME: We're depending on autosizing to distribute the height change properly which isnt the case

   deltaHeight+=newSize.height-oldSize.height; 
   NSRect frame=[self frame];
   frame.size.width+=deltaWidth;
   frame.size.height+=deltaHeight;
   
   [self setFrame:frame display:NO];
}    

-(NSToolbar *)toolbar {
   return _toolbar;
}

-(void)displayModeChanged:sender {
   NSToolbarDisplayMode displayMode=[[_displayModePopUp selectedCell] tag];
   
   [_toolbar setDisplayMode:displayMode];
}

-(void)sizeModeChanged:sender {
    NSToolbarSizeMode sizeMode=[_smallSizeModeButton state] ? NSToolbarSizeModeSmall : NSToolbarSizeModeRegular;
    
    [_toolbar setSizeMode:sizeMode];
}

-(void)customizationPaletteDidFinish:sender {
   [NSApp endSheet:self returnCode:NSAlertDefaultReturn];
}

@end
