/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSColorPicker.h>

@interface NSColorPickerSliders : NSColorPicker {
    IBOutlet NSBox *sliderSubview;
    IBOutlet NSBox *greyscaleSubview;
    IBOutlet NSBox *rgbSubview;
    IBOutlet NSBox *cmykSubview;
    IBOutlet NSBox *hsbSubview;
    IBOutlet NSPopUpButton *typeButton;

    IBOutlet NSView *currentView;
    IBOutlet NSSlider *greyscaleSlider;
    IBOutlet NSTextField *greyscaleTextField;
    IBOutlet NSMatrix *_grayscaleConstantsMatrix;

    IBOutlet NSSlider *_redSlider;
    IBOutlet NSSlider *_greenSlider;
    IBOutlet NSSlider *_blueSlider;
    IBOutlet NSMatrix *rgbTextFieldMatrix;

    IBOutlet NSSlider *_cyanSlider;
    IBOutlet NSSlider *_magentaSlider;
    IBOutlet NSSlider *_yellowSlider;
    IBOutlet NSSlider *_blackSlider;
    IBOutlet NSMatrix *cmykTextFieldMatrix;

    IBOutlet NSSlider *_hueSlider;
    IBOutlet NSSlider *_saturationSlider;
    IBOutlet NSSlider *_brightnessSlider;

    IBOutlet NSMatrix *hsbTextFieldMatrix;
}

- (void)typeChanged:(id)sender;

- (void)greyscaleDidChange:(id)sender;
- (void)grayscaleConstant:sender;

- (void)rgbSliderChanged:sender;
- (void)rgbTextChanged:sender;

- (void)cmykSliderChanged:sender;
- (void)cmykTextChanged:sender;

- (void)hsbSliderChanged:sender;
- (void)hsbTextChanged:sender;

@end
