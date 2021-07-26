/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSTimeZone.h>
#import <Foundation/NSTimeZone_absolute.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSPlatform.h>
#include <stdio.h>

NSString * const NSSystemTimeZoneDidChangeNotification=@"NSSystemTimeZoneDidChangeNotification";

static NSTimeZone *_systemTimeZone=nil;
static NSTimeZone *_defaultTimeZone=nil;
static NSTimeZone *_localTimeZone=nil;

@implementation NSTimeZone

+allocWithZone:(NSZone *)zone {
    if(self==[NSTimeZone class])
        return NSAllocateObject([[NSPlatform currentPlatform] timeZoneClass],0,zone);
    else
        return NSAllocateObject(self,0,zone);
}

+(NSTimeZone *)localTimeZone {
    if (_localTimeZone == nil)
        _localTimeZone = [self defaultTimeZone];

    return _localTimeZone;
}

+(NSTimeZone *)systemTimeZone {
    if (_systemTimeZone == nil) { 
        _systemTimeZone = [[[[NSPlatform currentPlatform] timeZoneClass] systemTimeZone] retain];        
    }

    return _systemTimeZone;
}

+(NSTimeZone *)defaultTimeZone {
    if (_defaultTimeZone == nil)
        _defaultTimeZone = [[self systemTimeZone] retain];
    
    return _defaultTimeZone;
}

+(void)resetSystemTimeZone {
    [_systemTimeZone release];
    _systemTimeZone=nil;
}

+(void)setDefaultTimeZone:(NSTimeZone *)timeZone {
    [_defaultTimeZone autorelease];
    _defaultTimeZone = [timeZone retain];
}

+(NSArray *)knownTimeZoneNames {
    
    return nil;

  /* static NSDictionary *_regionsDictionary = nil;
    if (_regionsDictionary == nil) {
        NSString *pathToPlist = [[NSBundle bundleForClass:self] pathForResource:@"NSTimeZoneRegions"
                                                                         ofType:@"plist"];
        _regionsDictionary = [[NSDictionary allocWithZone:NULL] initWithContentsOfFile:pathToPlist];
    }

    return [_regionsDictionary allKeys];*/
}

+(NSDictionary *)abbreviationDictionary {
   static NSDictionary *_abbreviationDictionary = nil;
    if (_abbreviationDictionary == nil) {
        NSString *pathToPlist = [[NSBundle bundleForClass:self] pathForResource:@"NSTimeZoneAbbreviations"
                                                                         ofType:@"plist"];
        _abbreviationDictionary = [[NSDictionary allocWithZone:NULL] initWithContentsOfFile:pathToPlist];
    }

    return _abbreviationDictionary;
}

-initWithName:(NSString *)name data:(NSData *)data {
    NSInvalidAbstractInvocation();
    return nil;
}

-initWithName:(NSString *)name {
    return [self initWithName:name data:nil];
}

+(NSTimeZone *)timeZoneWithName:(NSString *)name data:(NSData *)data {
    return [[[self allocWithZone:NULL] initWithName:name data:data] autorelease];
}

+(NSTimeZone *)timeZoneWithName:(NSString *)name {
    return [[[self allocWithZone:NULL] initWithName:name] autorelease];
}

+(NSTimeZone *)timeZoneForSecondsFromGMT:(NSInteger)seconds {
    return [[[NSTimeZone_absolute allocWithZone:NULL] initWithSecondsFromGMT:seconds] autorelease];
}

+(NSTimeZone *)timeZoneWithAbbreviation:(NSString *)abbreviation {
    NSString *fullName = [[self abbreviationDictionary] objectForKey:abbreviation];

    if (fullName != nil)
        return [self timeZoneWithName:fullName];

    return nil;
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    _name=[[coder decodeObjectForKey:@"NS.name"] copy];
    _data=[[coder decodeObjectForKey:@"NS.data"] copy];
   }
   else {
    NSInvalidAbstractInvocation();
    return nil;
}
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    [coder encodeObject:[self name] forKey:@"NS.name"];
    [coder encodeObject:[self data] forKey:@"NS.data"];
   }
   else {
    NSInvalidAbstractInvocation();
}
}

-copyWithZone:(NSZone *)zone {
    NSInvalidAbstractInvocation();
    return nil;
}

-(NSString *)name {
    return _name;
}

-(NSData *)data {
    return _data;
}

-(BOOL)isEqual:other {
   if(self==other)
    return YES;

    if ([other isKindOfClass:[NSTimeZone class]])
        return [self isEqualToTimeZone:other];

    return NO;
}

-(BOOL)isEqualToTimeZone:(NSTimeZone *)timeZone {
   if(self==timeZone)
    return YES;

    if ([[timeZone name] isEqualToString:[self name]])
        return YES;
    
    return NO;
}

-(NSInteger)secondsFromGMT {
    return [self secondsFromGMTForDate:[NSDate date]];
}

-(NSString *)abbreviation {
    return [self abbreviationForDate:[NSDate date]];
}

-(BOOL)isDaylightSavingTime {
    return [self isDaylightSavingTimeForDate:[NSDate date]];
}

-(NSTimeInterval)daylightSavingTimeOffset {
    return [self daylightSavingTimeOffsetForDate:[NSDate date]];
}

-(NSDate *)nextDaylightSavingTimeTransition {
    return [self nextDaylightSavingTimeTransitionAfterDate:[NSDate date]];
}

-(NSInteger)secondsFromGMTForDate:(NSDate *)date {
    NSInvalidAbstractInvocation();
    return -1;
}

-(NSString *)abbreviationForDate:(NSDate *)date {
    NSInvalidAbstractInvocation();
    return nil;
}

-(BOOL)isDaylightSavingTimeForDate:(NSDate *)date {
    NSInvalidAbstractInvocation();
    return NO;
}

-(NSTimeInterval)daylightSavingTimeOffsetForDate:(NSDate *)date {
   NSUnimplementedMethod();
   return 0;
}

-(NSDate *)nextDaylightSavingTimeTransitionAfterDate:(NSDate *)date {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)localizedName:(NSTimeZoneNameStyle)style locale:(NSLocale *)locale {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] name: %@>", [self class], self, [self name]];
}

@end
