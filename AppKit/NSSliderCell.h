/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSActionCell.h>

typedef enum {
    NSTickMarkBelow = 0,
    NSTickMarkAbove = 1,
    NSTickMarkLeft = NSTickMarkAbove,
    NSTickMarkRight = NSTickMarkBelow,
} NSTickMarkPosition;

typedef enum {
    NSLinearSlider = 0,
    NSCircularSlider = 1,
} NSSliderType;

@interface NSSliderCell : NSActionCell {
    NSSliderType _type;
    double _minValue;
    double _maxValue;
    double _altIncrementValue;
    NSInteger _isVertical;
    NSRect _lastRect;

    int _numberOfTickMarks;
    NSTickMarkPosition _tickMarkPosition;
    BOOL _allowsTickMarkValuesOnly;
}

- (NSSliderType)sliderType;

- (double)minValue;
- (double)maxValue;
- (double)altIncrementValue;

- (NSInteger)numberOfTickMarks;
- (NSTickMarkPosition)tickMarkPosition;
- (BOOL)allowsTickMarkValuesOnly;
- (CGFloat)knobThickness;

- (void)setSliderType:(NSSliderType)value;

- (void)setMinValue:(double)minValue;
- (void)setMaxValue:(double)maxValue;
- (void)setAltIncrementValue:(double)value;

- (void)setNumberOfTickMarks:(NSInteger)number;
- (void)setTickMarkPosition:(NSTickMarkPosition)position;
- (void)setAllowsTickMarkValuesOnly:(BOOL)valuesOnly;
- (void)setKnobThickness:(CGFloat)thickness;

- (NSInteger)isVertical;

- (NSInteger)indexOfTickMarkAtPoint:(NSPoint)point;

- (double)tickMarkValueAtIndex:(NSInteger)index;
- (double)closestTickMarkValueToValue:(double)value;

- (NSRect)trackRect;

- (NSRect)rectOfTickMarkAtIndex:(NSInteger)index;

- (void)drawBarInside:(NSRect)frame flipped:(BOOL)isFlipped;
- (NSRect)knobRectFlipped:(BOOL)flipped;
- (void)drawKnob;
- (void)drawKnob:(NSRect)rect;

@end
