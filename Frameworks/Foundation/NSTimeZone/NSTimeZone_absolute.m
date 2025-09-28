/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSTimeZone_absolute.h>
#import <Foundation/NSTimeZone.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSRaise.h>

@implementation NSTimeZone_absolute

-initWithSecondsFromGMT:(NSInteger)seconds {
    [super init];
    _secondsFromGMT = seconds;
    _name = [[NSString allocWithZone:NULL] initWithFormat:@"GMT %@%02d%02d",
           (seconds >= 0 ? @"+" : @""),
           seconds/3600,
           (seconds % 3600)/60];
    _abbreviation = [[NSString allocWithZone:NULL] initWithFormat:@"%@%02d%02d",
           (seconds >= 0 ? @"+" : @""),
           seconds/3600,
           (seconds % 3600)/60];

    return self;
}

-(void)dealloc {
    [_name release];
    [_abbreviation release];

    [super dealloc];
}

-(void)encodeWithCoder:(NSCoder *)coder {
    int value=(int)_secondsFromGMT;
    [coder encodeValueOfObjCType:@encode(int) at:&value];
    [coder encodeObject:_name];
    [coder encodeObject:_abbreviation];
}

-initWithCoder:(NSCoder *)coder {
    int value=0;
    [coder decodeValueOfObjCType:@encode(int) at:&value];
    _secondsFromGMT=value;
    _name = [[coder decodeObject] retain];
    _abbreviation = [[coder decodeObject] retain];

    return self;
}

-copyWithZone:(NSZone *)zone {
    return [self retain];
}

-(NSString *)name {
    return _name;
}

-(NSData *)data {
    return nil;
}

-(NSInteger)secondsFromGMTForDate:(NSDate *)date {
    return _secondsFromGMT;
}

-(NSString *)abbreviationForDate:(NSDate *)date {
    return _abbreviation;
}

-(BOOL)isDaylightSavingTimeForDate:(NSDate *)date {
    return NO;
}

@end
