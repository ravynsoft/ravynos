/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSNumber_char.h>
#import <Foundation/NSStringFormatter.h>

@implementation NSNumber_char

NSNumber *NSNumber_charNew(NSZone *zone,char value) {
   NSNumber_char *self=NSAllocateObject([NSNumber_char class],0,zone);
    if (self) {
       self->_type=kCFNumberCharType;
       self->_value=value;
    }
   return self;
}

-(void)getValue:(void *)value {
   *((char *)value)=_value;
}

-(const char *)objCType {
   return @encode(char);
}

-(char)charValue {
   return _value;
}

-(unsigned char)unsignedCharValue {
   return _value;
}

-(short)shortValue {
   return _value;
}

-(unsigned short)unsignedShortValue {
   return _value;
}

-(int)intValue {
   return _value;
}

-(unsigned int)unsignedIntValue {
   return _value;
}

-(long)longValue {
   return _value;
}

-(unsigned long)unsignedLongValue {
   return _value;
}

-(long long)longLongValue {
   return _value;
}

-(unsigned long long)unsignedLongLongValue {
   return _value;
}

-(float)floatValue {
   return _value;
}

-(double)doubleValue {
   return _value;
}

-(BOOL)boolValue {
   return _value?YES:NO;
}

-(NSInteger)integerValue {
   return (NSInteger)_value;
}

-(NSUInteger)unsignedIntegerValue {
   return (NSUInteger)_value;
}

-(NSString *)descriptionWithLocale:(NSDictionary *)locale {
   return NSStringWithFormatAndLocale(@"%i",locale,_value);
}

@end
