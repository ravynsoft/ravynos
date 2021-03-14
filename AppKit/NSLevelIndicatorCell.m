/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSLevelIndicatorCell.h>
#import <AppKit/NSRaise.h>

@implementation NSLevelIndicatorCell

-initWithLevelIndicatorStyle:(NSLevelIndicatorStyle)style {
   [super init];
   _style=style;
   _minValue=0;
   _maxValue=(_style==NSContinuousCapacityLevelIndicatorStyle)?100:5;
   _warningValue=_maxValue;
   _criticalValue=_maxValue;
   _numberOfTickMarks=0;
   _numberOfMajorTickMarks=0;
   _tickMarkPosition=NSTickMarkBelow;
   return self;
}

-(NSLevelIndicatorStyle)levelIndicatorStyle {
   return _style;
}

-(double)minValue {
   return _minValue;
}

-(double)maxValue {
   return _maxValue;
}

-(double)warningValue {
   return _warningValue;
}

-(double)criticalValue {
   return _criticalValue;
}

-(int)numberOfTickMarks {
   return _numberOfTickMarks;
}

-(int)numberOfMajorTickMarks {
   return _numberOfMajorTickMarks;
}

-(NSTickMarkPosition)tickMarkPosition {
   return _tickMarkPosition;
}

-(void)setLevelIndicatorStyle:(NSLevelIndicatorStyle)style {
   _style=style;
}

-(void)setMinValue:(double)value {
   _minValue=value;
}

-(void)setMaxValue:(double)value {
   _maxValue=value;
}

-(void)setWarningValue:(double)value {
   _warningValue=value;
}

-(void)setCriticalValue:(double)value {
   _criticalValue=value;
}

-(void)setNumberOfTickMarks:(int)count {
   _numberOfTickMarks=count;
}

-(void)setNumberOfMajorTickMarks:(int)count {
   _numberOfMajorTickMarks=count;
}

-(void)setTickMarkPosition:(NSTickMarkPosition)position {
   _tickMarkPosition=position;
}

-(void)setImage:(NSImage *)image {
}

-(double)tickMarkValueAtIndex:(int)index {
   NSUnimplementedMethod();
   return 0;
}

-(NSRect)rectOfTickMarkAtIndex:(int)index {
   NSUnimplementedMethod();
   return NSZeroRect;
}

@end
