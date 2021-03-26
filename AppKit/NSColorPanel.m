/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/AppKit.h>
#import <AppKit/NSColorPickerColorList.h>
#import <AppKit/NSColorPickerSliders.h>
#import <AppKit/NSColorPickerWheel.h>
#import <AppKit/NSRaise.h>

NSString * const NSColorPanelColorDidChangeNotification=@"NSColorPanelColorDidChangeNotification";

@interface NSColorPicker(PrivateInterface)

- (void)setColor:(NSColor*)color;

@end

@implementation NSColorPanel

static NSColorPanel *_colorPanel=nil;
static NSUInteger    _pickerMask=0;

+(void)setColorPanel:(NSColorPanel *)colorPanel {
   [_colorPanel release];
   _colorPanel=[colorPanel retain];
}

+(BOOL)sharedColorPanelExists {
   return (_colorPanel!=nil)?YES:NO;
}

+(NSColorPanel *)sharedColorPanel {
   if(_colorPanel==nil){
    if(![NSBundle loadNibNamed:@"NSColorPanel" owner:self])
     NSLog(@"Cannot load NSColorPanel.nib");
    
    if(![_colorPanel setFrameUsingName:@"NSColorPanel"])
     [_colorPanel center];

    [_colorPanel setFrameAutosaveName:@"NSColorPanel"];
   }

   return _colorPanel;
}

+(void)setPickerMask:(NSUInteger)mask {
    if (_colorPanel == nil)	// only works if color panel is not yet created
        _pickerMask=mask;
}

+ (void)setPickerMode:(NSColorPanelMode)mode {
   NSUnimplementedMethod();
}

+(BOOL)dragColor:(NSColor *)color withEvent:(NSEvent *)event fromView:(NSView *)view {
   NSPasteboard *pboard=[NSPasteboard pasteboardWithName:NSDragPboard];
   NSSize        size=NSMakeSize(12,12);
   NSImage      *image=[[[NSImage alloc] initWithSize:size] autorelease];

   [image setCachedSeparately:YES];
   [image lockFocus];
   [color drawSwatchInRect:NSMakeRect(0,0,12,12)];
   [image unlockFocus];

   [pboard declareTypes:[NSArray arrayWithObject:NSColorPboardType] owner:nil];
   [color writeToPasteboard:pboard];

   [view dragImage:image at:NSMakePoint(0,0) offset:NSMakeSize(0,0) event:event pasteboard:pboard source:view slideBack:YES];
   return YES;
}

-(void)swapInNewView:sender {
   NSColorPicker<NSColorPickingCustom> *chosenColorPicker=[_colorPickers objectAtIndex:[sender selectedTag]];
   NSView *newView;

   newView=[chosenColorPicker provideNewView:YES];

   if (currentColorPickerView != newView) {
      [[sender selectedCell] setImage:[chosenColorPicker provideNewButtonImage]];

        if (currentColorPickerView != nil)
            [newView setFrame:[currentColorPickerView frame]];
        else
         [newView setFrame:[colorPickerView bounds]];

        [currentColorPickerView retain];
        [currentColorPickerView removeFromSuperview];

        [colorPickerView addSubview:newView];
       // [splitView adjustSubviews];
        currentColorPickerView=[newView retain];
    }
}

-(void)awakeFromNib {
    // time to load the color pickers. theoretically we should be searching all the /Library/ColorPickers out there, but...
    NSArray *colorPickersClassArray=[NSArray arrayWithObjects:
									[NSColorPickerWheel class],
									[NSColorPickerSliders class],
#if 0
                                     // Disabled to see if it fixes a nib instantiation issue
                                     [NSColorPickerColorList class],
#endif
									nil];
    unsigned i,count=[colorPickersClassArray count];

    [colorWell setBordered:NO];
    
   _colorPickers=[[NSMutableArray alloc] init];
   [colorPickersMatrix renewRows:1 columns:count];

   for(i=0;i<count;i++){
    Class          colorPickerClass=[colorPickersClassArray objectAtIndex:i];
    NSColorPicker *newPicker=[[[colorPickerClass alloc] initWithPickerMask:_pickerMask colorPanel:self] autorelease];
    NSCell        *cell=[colorPickersMatrix cellAtRow:0 column:i];

    [_colorPickers addObject:newPicker];

    [cell setImage:[newPicker provideNewButtonImage]];
    [cell setTag:i];
    [cell setTarget:self];
    [cell setAction:@selector(swapInNewView:)];
   }

   [colorPickersMatrix selectCellAtRow:0 column:0];
   [self swapInNewView:colorPickersMatrix];
   
   [opacityTitle setHidden:YES];
   [opacitySlider setHidden:YES];
   [opacityTextField setHidden:YES];
   [opacityPercentLabel setHidden: YES];
   [opacitySlider setTarget:self];
   [opacitySlider setAction:@selector(_alphaChanged:)];
   [opacityTextField setTarget:self];
   [opacityTextField setAction:@selector(_alphaChanged:)];
}

-(NSColor *)color {
   return [colorWell color];
}

-(CGFloat)alpha {
   return _showsAlpha?MIN(MAX(0.0,[opacitySlider floatValue]/100.0),1.0):1.0f;
}

-(NSColorPanelMode)mode {
   return _mode;
}

-(BOOL)showsAlpha {
   return _showsAlpha;
}

- (BOOL)isContinuous {
   return _continuous;
}

-(NSView *)accessoryView {
   NSUnimplementedMethod();
   return nil;
}

-(void)setColorButtonClicked:sender {
    [NSApp sendAction:_action to:_target from:self];
}

- (NSColorPicker*)_selectedColorPicker
{
	int index = [colorPickersMatrix selectedTag];
	NSColorPicker *picker = [_colorPickers objectAtIndex: index];
	return picker;
}

-(void)setColor:(NSColor *)color {
   [colorWell setColor:color];
   [self setColorButtonClicked:nil];
	
	NSColorPicker *picker = [self _selectedColorPicker];
	if ([picker respondsToSelector: @selector(setColor:)]) {
		[picker setColor: color];
	}
    float alpha = [color alphaComponent];
    [opacitySlider setFloatValue: alpha * 100.f];
    [opacityTextField setFloatValue: alpha * 100.f];
    
   [[NSNotificationQueue defaultQueue] enqueueNotification:[NSNotification notificationWithName:NSColorPanelColorDidChangeNotification object:self] postingStyle:NSPostNow coalesceMask:NSNotificationCoalescingOnName forModes:nil];  
}

-(void)setMode:(NSColorPanelMode)mode {
   _mode=mode;
}

-(void)setShowsAlpha:(BOOL)flag {
   _showsAlpha=flag;
   [opacityTitle setHidden:_showsAlpha?NO:YES];
   [opacitySlider setHidden:_showsAlpha?NO:YES];
   [opacityTextField setHidden:_showsAlpha?NO:YES];
   [opacityPercentLabel setHidden:_showsAlpha?NO:YES];
	
	if (_showsAlpha) {
		// Update the controls!
		NSColor* color = [self color];
		float alpha = [color alphaComponent];
		[opacitySlider setFloatValue: alpha * 100.f];
		[opacityTextField setFloatValue: alpha * 100.f];
	}
}

-(void)setContinuous:(BOOL)flag {
   _continuous=flag;
}

-(void)setAccessoryView:(NSView *)view {
   NSUnimplementedMethod();
}

-(void)setAction:(SEL)action {
   _action=action;
}

-(void)setTarget:target {
   _target=target;
}

-(void)attachColorList:(NSColorList *)colorList {
   NSUnimplementedMethod();
}

-(void)detachColorList:(NSColorList *)colorList {
   NSUnimplementedMethod();
}

-(void)_alphaChanged:sender {
   CGFloat  alpha=MIN(MAX(0.0,[sender floatValue]/100.0),1.0);
   NSColor *color=[[self color] colorWithAlphaComponent:alpha];

	if (sender == opacitySlider) {
		[opacityTextField setFloatValue: [sender floatValue]];
	} else {
		[opacitySlider setFloatValue: [sender floatValue]];
	}
   [self setColor:color];
}

@end

