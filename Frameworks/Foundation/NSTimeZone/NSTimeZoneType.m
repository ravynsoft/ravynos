/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSTimeZoneType.h>
#import <Foundation/NSString.h>
#import <Foundation/NSCoder.h>

@implementation NSTimeZoneType

+(NSTimeZoneType *)timeZoneTypeWithSecondsFromGMT:(NSInteger)seconds isDaylightSavingTime:(BOOL)isDST abbreviation:(NSString *)abbreviation {
    return [[[self allocWithZone:NULL] initWithSecondsFromGMT:seconds isDaylightSavingTime:isDST abbreviation:abbreviation] autorelease];
}

-initWithSecondsFromGMT:(NSInteger)seconds isDaylightSavingTime:(BOOL)isDST abbreviation:(NSString *)abbreviation {
    _secondsFromGMT = seconds;
    _isDaylightSavingTime = isDST;
    _abbreviation = [abbreviation retain];

    return self;
}

-(void)dealloc {
    [_abbreviation release];

    [super dealloc];
}

-(NSInteger)secondsFromGMT {
    return _secondsFromGMT;
}

-(BOOL)isDaylightSavingTime {
    return _isDaylightSavingTime;
}

-(NSString *)abbreviation {
    return _abbreviation;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] secondsFromGMT: %d isDaylightSavingTime: %@ abbreviation: %@>",
        [self class], self,
        _secondsFromGMT, _isDaylightSavingTime ? @"YES" : @"NO", _abbreviation];
}

-copyWithZone:(NSZone *)zone {
    return [self retain];
}

-(void)encodeWithCoder:(NSCoder *)coder {
    int value=(int)_secondsFromGMT;
    [coder encodeValueOfObjCType:@encode(int) at:&value];
    [coder encodeValueOfObjCType:@encode(BOOL) at:&_isDaylightSavingTime];
    [coder encodeObject:_abbreviation];
}

-initWithCoder:(NSCoder *)coder {
    int value=0;
    [coder decodeValueOfObjCType:@encode(int) at:&value];
    _secondsFromGMT=value;
    [coder decodeValueOfObjCType:@encode(BOOL) at:&_isDaylightSavingTime];
    _abbreviation = [[coder decodeObject] retain];

    return self;
}

@end

