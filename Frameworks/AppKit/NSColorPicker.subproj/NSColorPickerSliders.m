/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/AppKit.h>

#import <AppKit/NSColorPickerSliders.h>

@implementation NSColorPickerSliders

-initWithPickerMask:(NSUInteger)mask colorPanel:(NSColorPanel *)colorPanel {
    [super initWithPickerMask:mask colorPanel:colorPanel];

    NSSize size=[_grayscaleConstantsMatrix cellSize];
    int i;

    size.width-=10;
    size.height-=10;

    for(i=0;i<7;i++){
     NSImage *image=[[[NSImage alloc] initWithSize:size] autorelease];
     NSColor *color=[NSColor colorWithCalibratedWhite:i/6.0 alpha:1];

     [image setCachedSeparately:YES];
     [image lockFocus];
     [color drawSwatchInRect:NSMakeRect(0,0,size.width,size.height)];
     [image unlockFocus];

     [[_grayscaleConstantsMatrix cellAtRow:0 column:i] setImage:image];
    }

    [self typeChanged: typeButton];

    return self;
}

- (void)_syncSlidersToNewColor
{
    NSColor *color = [[self colorPanel] color];
	
    switch ([[typeButton selectedItem] tag]) {
        case NSGrayModeColorPanel: {
            float gray, alpha;
			
            color = [color colorUsingColorSpaceName:NSCalibratedWhiteColorSpace];
            [color getWhite:&gray alpha:&alpha];
			
            [greyscaleSlider setIntValue:gray*100];
            [greyscaleTextField setIntValue:gray*100];
            break;
        }
            
        case NSRGBModeColorPanel: {
            float red, green, blue, alpha;
			
            color = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
            [color getRed:&red green:&green blue:&blue alpha:&alpha];
			
            [_redSlider setIntValue:red*255];
            [_greenSlider setIntValue:green*255];
            [_blueSlider setIntValue:blue*255];
            [[rgbTextFieldMatrix cellAtRow:0 column:0] setIntValue:red*255];
            [[rgbTextFieldMatrix cellAtRow:1 column:0] setIntValue:green*255];
            [[rgbTextFieldMatrix cellAtRow:2 column:0] setIntValue:blue*255];
            
            break;
        }
			
        case NSCMYKModeColorPanel: {
            float cyan, magenta, yellow, black, alpha;
			
            color = [color colorUsingColorSpaceName:NSDeviceCMYKColorSpace];
            [color getCyan:&cyan magenta:&magenta yellow:&yellow black:&black alpha:&alpha];
            
            [_cyanSlider setIntValue:cyan*100];
            [_magentaSlider setIntValue:magenta*100];
            [_yellowSlider setIntValue:yellow*100];
            [_blackSlider setIntValue:black*100];
            [[cmykTextFieldMatrix cellAtRow:0 column:0] setIntValue:cyan*100];
            [[cmykTextFieldMatrix cellAtRow:1 column:0] setIntValue:magenta*100];
            [[cmykTextFieldMatrix cellAtRow:2 column:0] setIntValue:yellow*100];
            [[cmykTextFieldMatrix cellAtRow:3 column:0] setIntValue:black*100];
			
            break;
        }
			
        case NSHSBModeColorPanel: {
            float hue, saturation, brightness, alpha;
			
            color = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
            [color getHue:&hue saturation:&saturation brightness:&brightness alpha:&alpha];
			
            [_hueSlider setIntValue:hue*359];
            [_saturationSlider setIntValue:saturation*100];
            [_brightnessSlider setIntValue:brightness*100];
            [[hsbTextFieldMatrix cellAtRow:0 column:0] setIntValue:hue*359];
            [[hsbTextFieldMatrix cellAtRow:1 column:0] setIntValue:saturation*100];
            [[hsbTextFieldMatrix cellAtRow:2 column:0] setIntValue:brightness*100];
            
            break;
        }
            
        default:
            return;
    }
}

- (void)typeChanged:(id)sender
{
    NSView *newView = nil;

	[self _syncSlidersToNewColor];
	
    switch ([[sender selectedItem] tag]) {
        case NSGrayModeColorPanel: {
            newView = greyscaleSubview;
            break;
        }
            
        case NSRGBModeColorPanel: {
            newView = rgbSubview;
            break;
        }
			
        case NSCMYKModeColorPanel: {
            newView = cmykSubview;
            break;
        }
			
        case NSHSBModeColorPanel: {
            newView = hsbSubview;
            break;
        }
            
        default:
            return;
    }
	
    if (currentView != nil)
        [newView setFrame:[currentView frame]];
	
    if (currentView != newView) {
        [currentView retain];
        [currentView removeFromSuperview];
		
        [sliderSubview addSubview:newView];
        currentView = [newView retain];
    }

	NSColor *color = [[self colorPanel] color];

    [[self colorPanel] setColor:color];
}

- (void)setColor:(NSColor *)color
{
	[self _syncSlidersToNewColor];
}

// doesn't matter who sends these actions, the sliders or the textfields.
// n.b. some bounds checking should be performed, but it won't crash anything without it

- (void)greyscaleDidChange:(id)sender {
    int    intValue=[sender intValue];

    [greyscaleTextField setIntValue:intValue];
    [greyscaleSlider setIntValue:intValue];

    [[self colorPanel] setColor:[NSColor colorWithCalibratedWhite:intValue/100.0 alpha:[[self colorPanel] alpha]]];
}

-(void)grayscaleConstant:sender {
   int      i=[sender selectedColumn];
   float    value=i/6.0;
   NSColor *color=[NSColor colorWithCalibratedWhite:value alpha:1];

    [greyscaleTextField setIntValue:value*100];
    [greyscaleSlider setIntValue:value*100];
    
    [[self colorPanel] setColor:color];
}

- (NSImage *)provideNewButtonImage
{
    return [NSImage imageNamed:@"NSColorPickerSlidersIcon"];
}

-(void)_updateRed:red green:green blue:blue {
   [_redSlider setIntValue:[red intValue]];
   [_greenSlider setIntValue:[green intValue]];
   [_blueSlider setIntValue:[blue intValue]];

   [[rgbTextFieldMatrix cellAtRow:0 column:0] setIntValue:[red intValue]];
   [[rgbTextFieldMatrix cellAtRow:1 column:0] setIntValue:[green intValue]];
   [[rgbTextFieldMatrix cellAtRow:2 column:0] setIntValue:[blue intValue]];

   [[self colorPanel] setColor:[NSColor colorWithCalibratedRed:[red floatValue]/255.0
                                                                       green:[green floatValue]/255.0
                                                                        blue:[blue floatValue]/255.0
                                                                       alpha:[[self colorPanel] alpha]]];
}

-(void)rgbSliderChanged:sender {
   [self _updateRed:_redSlider green:_greenSlider blue:_blueSlider];
}

-(void)rgbTextChanged:(id)sender {
   [self _updateRed:[rgbTextFieldMatrix cellAtRow:0 column:0] green:[rgbTextFieldMatrix cellAtRow:1 column:0] blue:[rgbTextFieldMatrix cellAtRow:2 column:0]];
}

-(void)_updateCyan:cyan magenta:magenta yellow:yellow black:black {
    [[cmykTextFieldMatrix cellAtRow:0 column:0] setIntValue:[cyan intValue]];
    [[cmykTextFieldMatrix cellAtRow:1 column:0] setIntValue:[magenta intValue]];
    [[cmykTextFieldMatrix cellAtRow:2 column:0] setIntValue:[yellow intValue]];
    [[cmykTextFieldMatrix cellAtRow:3 column:0] setIntValue:[black intValue]];

    [_cyanSlider setIntValue:[cyan intValue]];
    [_magentaSlider setIntValue:[magenta intValue]];
    [_yellowSlider setIntValue:[yellow intValue]];
    [_blackSlider setIntValue:[black intValue]];

    [[self colorPanel] setColor:[NSColor colorWithDeviceCyan:[cyan floatValue]/100.0
                                                                   magenta:[magenta floatValue]/100.0
                                                                    yellow:[yellow floatValue]/100.0
                                                                     black:[black floatValue]/100.0
                                                                     alpha:[[self colorPanel] alpha]]];
}

-(void)cmykSliderChanged:sender {
   [self _updateCyan:_cyanSlider magenta:_magentaSlider yellow:_yellowSlider black:_blackSlider];
}

-(void)cmykTextChanged:sender {
   [self _updateCyan:[cmykTextFieldMatrix cellAtRow:0 column:0] magenta:[cmykTextFieldMatrix cellAtRow:1 column:0] yellow:[cmykTextFieldMatrix cellAtRow:2 column:0] black:[cmykTextFieldMatrix cellAtRow:3 column:0]];
}

-(void)_updateHue:hue saturation:saturation brightness:brightness {
    [[hsbTextFieldMatrix cellAtRow:0 column:0] setIntValue:[hue intValue]];
    [[hsbTextFieldMatrix cellAtRow:1 column:0] setIntValue:[saturation intValue]];
    [[hsbTextFieldMatrix cellAtRow:2 column:0] setIntValue:[brightness intValue]];

    [_hueSlider setIntValue:[hue intValue]];
    [_saturationSlider setIntValue:[saturation intValue]];
    [_brightnessSlider setIntValue:[brightness intValue]];

    [[self colorPanel] setColor:[NSColor colorWithCalibratedHue:[hue floatValue]/359.0
                                                                   saturation:[saturation floatValue]/100.0
                                                                   brightness:[brightness floatValue]/100.0
                                                                        alpha:[[self colorPanel] alpha]]];
}

-(void)hsbSliderChanged:sender {
   [self _updateHue:_hueSlider saturation:_saturationSlider brightness:_brightnessSlider];
}

-(void)hsbTextChanged:sender {
   [self _updateHue:[hsbTextFieldMatrix cellAtRow:0 column:0] saturation:[hsbTextFieldMatrix cellAtRow:1 column:0] brightness:[hsbTextFieldMatrix cellAtRow:2 column:0]];
}


@end
