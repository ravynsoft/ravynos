/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSEvent_other.h"


@implementation NSEvent_other
-initWithType:(NSEventType)type location:(NSPoint)location modifierFlags:(NSUInteger)modifierFlags timestamp:(NSTimeInterval)time windowNumber:(NSInteger)windowNum context:(NSGraphicsContext *)context subtype:(short)subtype data1:(NSInteger)data1 data2:(NSInteger)data2 {
  _type = type;
  _locationInWindow=location;
  _modifierFlags=modifierFlags;
  _timestamp = time;
  _windowNumber = windowNum;
  _subtype = subtype;
  _data1 = data1;
  _data2 = data2;
  return self;
}

-(short)subtype {
  return _subtype;
}

-(NSInteger)data1 {
  return _data1;
}

-(NSInteger)data2 {
  return _data2;
}



@end
