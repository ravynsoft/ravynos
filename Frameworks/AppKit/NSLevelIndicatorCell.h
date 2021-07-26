/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSActionCell.h>
#import <AppKit/NSSliderCell.h>

typedef enum {
    NSRatingLevelIndicatorStyle,
    NSDiscreteCapacityLevelIndicatorStyle,
    NSContinuousCapacityLevelIndicatorStyle,
    NSRelevancyLevelIndicatorStyle,
} NSLevelIndicatorStyle;

@interface NSLevelIndicatorCell : NSActionCell {
    NSLevelIndicatorStyle _style;
    double _minValue;
    double _maxValue;
    double _warningValue;
    double _criticalValue;
    int _numberOfTickMarks;
    int _numberOfMajorTickMarks;
    NSTickMarkPosition _tickMarkPosition;
}

- initWithLevelIndicatorStyle:(NSLevelIndicatorStyle)style;

- (NSLevelIndicatorStyle)levelIndicatorStyle;
- (double)minValue;
- (double)maxValue;
- (double)warningValue;
- (double)criticalValue;
- (int)numberOfTickMarks;
- (int)numberOfMajorTickMarks;
- (NSTickMarkPosition)tickMarkPosition;

- (void)setLevelIndicatorStyle:(NSLevelIndicatorStyle)style;
- (void)setMinValue:(double)value;
- (void)setMaxValue:(double)value;
- (void)setWarningValue:(double)value;
- (void)setCriticalValue:(double)value;
- (void)setNumberOfTickMarks:(int)count;
- (void)setNumberOfMajorTickMarks:(int)count;
- (void)setTickMarkPosition:(NSTickMarkPosition)position;
- (void)setImage:(NSImage *)image;

- (double)tickMarkValueAtIndex:(int)index;
- (NSRect)rectOfTickMarkAtIndex:(int)index;

@end
