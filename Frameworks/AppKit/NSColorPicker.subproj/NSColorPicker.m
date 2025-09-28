/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSColorPicker.h>
#import <AppKit/NSColorPanel.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSButtonCell.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSRaise.h>

@implementation NSColorPicker

- initWithPickerMask:(NSUInteger)mask colorPanel:(NSColorPanel *)colorPanel
{
    if (![NSBundle loadNibNamed:[[self class] description] owner:self])
         NSLog(@"Couldn't load %@.nib", [[self class] description]);

    _mask = mask;
    _colorPanel = [colorPanel retain];
    return self;
}

- (void)dealloc
{
    [_colorPanel release];
    [super dealloc];
}

// for IB
- (void)setSubview:(NSView *)subview
{
    _subview = [subview retain];
}

-(void)setMode:(int)mode
{
    NSInvalidAbstractInvocation();
}

- (void)attachColorList:(NSColorList *)colorList
{
}

-(void)detachColorList:(NSColorList *)colorList {
}

-(NSImage *)provideNewButtonImage {
    return [NSImage imageNamed:@"NSAlertPanelExclamation"];
}

-(void)insertNewButtonImage:(NSImage *)image in:(NSButtonCell *)buttonCell {
    [buttonCell setImage:image];
}

-(void)alphaControlAddedOrRemoved:sender {
}

- (void)viewSizeChanged:sender {
    NSInvalidAbstractInvocation();
}

- (int)currentMode {
    return [_colorPanel mode];
}

- (BOOL)supportsMode:(int)mode {
    return NO;
}

- (void)setColor:(NSColor *)color {
    NSInvalidAbstractInvocation();
}

- (NSView *)provideNewView:(BOOL)firstTime {
    return _subview;
}

-(NSSize)minContentSize {
    NSInvalidAbstractInvocation();
    return NSMakeSize(0,0);
}

-(NSString *)buttonToolTip {
    NSInvalidAbstractInvocation();
    return nil;
}

- (NSColorPanel *)colorPanel { 
   return _colorPanel;
}

@end
