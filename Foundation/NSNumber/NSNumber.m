/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSValue.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSNumber_char.h>
#import <Foundation/NSNumber_double.h>
#import <Foundation/NSNumber_float.h>
#import <Foundation/NSNumber_int.h>
#import <Foundation/NSNumber_longLong.h>
#import <Foundation/NSNumber_long.h>
#import <Foundation/NSNumber_short.h>
#import <Foundation/NSNumber_unsignedChar.h>
#import <Foundation/NSNumber_unsignedInt.h>
#import <Foundation/NSNumber_unsignedLongLong.h>
#import <Foundation/NSNumber_unsignedLong.h>
#import <Foundation/NSNumber_unsignedShort.h>
#import <Foundation/NSNumber_BOOL.h>
#import <Foundation/NSNumber_placeholder.h>
#import <Foundation/NSAutoreleasePool-private.h>

#include <string.h>

@implementation NSNumber

+allocWithZone:(NSZone *)zone {
   if(self==objc_lookUpClass("NSNumber"))
      return [NSNumber_placeholder _sharedInstance];

   return NSAllocateObject(self,0,zone);
}

-initWithBool:(BOOL)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithChar:(char)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithShort:(short)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithInt:(int)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithLong:(long)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithLongLong:(long long)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithFloat:(float)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithDouble:(double)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithInteger:(NSInteger)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithUnsignedChar:(unsigned char)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithUnsignedShort:(unsigned short)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithUnsignedInt:(unsigned int)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithUnsignedLong:(unsigned long)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithUnsignedLongLong:(unsigned long long)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithUnsignedInteger:(NSUInteger)value {
   NSInvalidAbstractInvocation();
   return nil;
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

+(void)invalidType:(const char *)type {
   [NSException raise:@"NSNumberCannotInitWithCoderException"
               format:@"NSNumber cannot initWithCoder: type=%s",type];
}

-(Class)classForCoder {
   return objc_lookUpClass("NSNumber");
}

-initWithCoder:(NSCoder *)coder {
   char *type;

   [self dealloc];

   [coder decodeValueOfObjCType:@encode(char *) at:&type];
   if(strlen(type)!=1)
    [NSNumber invalidType:type];

   switch(*type){

    case 'c':{
      char value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_charNew(NULL,value);
     }
     break;

    case 'C':{
      unsigned char value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_unsignedCharNew(NULL,value);
     }
     break;

    case 's':{
      short value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_shortNew (NULL,value);
     }
     break;

    case 'S':{
      unsigned short value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_unsignedShortNew(NULL,value);
     }
     break;

    case 'i':{
      int value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_intNew(NULL,value);
     }
     break;

    case 'I':{
      unsigned int value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_unsignedIntNew(NULL,value);
     }
     break;

    case 'l':{
      long value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_longNew(NULL,value);
     }
     break;

    case 'L':{
      unsigned long value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_unsignedLongNew(NULL,value);
     }
     break;

    case 'q':{
      long long value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_longLongNew(NULL,value);
     }
     break;

    case 'Q':{
      unsigned long long value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_unsignedLongLongNew(NULL,value);
     }
     break;

    case 'f':{
      float value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_floatNew(NULL,value);
     }
     break;

    case 'd':{
      double value;

      [coder decodeValueOfObjCType:type at:&value];
      return NSNumber_doubleNew(NULL,value);
     }
     break;

    default:
     [NSNumber invalidType:type];
     break;
   }

   return nil;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   const char *type=[self objCType];

   [coder encodeValueOfObjCType:@encode(char *) at:&type];

   switch(*type){

    case 'c':{
      char value=[self charValue];;

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'C':{
      unsigned char value=[self unsignedCharValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 's':{
      short value=[self shortValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'S':{
      unsigned short value=[self unsignedShortValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'i':{
      int value=[self intValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'I':{
      unsigned int value=[self unsignedIntValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'l':{
      long value=[self longValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'L':{
      unsigned long value=[self unsignedLongValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'q':{
      long long value=[self longLongValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'Q':{
      unsigned long long value=[self unsignedLongLongValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'f':{
      float value=[self floatValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;

    case 'd':{
      double value=[self doubleValue];

      [coder encodeValueOfObjCType:type at:&value];
     }
     break;
   }
}


+(NSNumber *)numberWithBool:(BOOL)value {
   return NSNumber_BOOLNew(NULL,value);
}

+(NSNumber *)numberWithChar:(char)value {
   return [[[self allocWithZone:NULL] initWithChar:value] autorelease];
}

+(NSNumber *)numberWithShort:(short)value {
   return [[[self allocWithZone:NULL] initWithShort:value] autorelease];
}

+(NSNumber *)numberWithInt:(int)value {
   return [[[self allocWithZone:NULL] initWithInt:value] autorelease];
}

+(NSNumber *)numberWithLong:(long)value {
   return [[[self allocWithZone:NULL] initWithLong:value] autorelease];
}

+(NSNumber *)numberWithLongLong:(long long)value {
   return [[[self allocWithZone:NULL] initWithLongLong:value] autorelease];
}

+(NSNumber *)numberWithFloat:(float)value {
   return [[[self allocWithZone:NULL] initWithFloat:value] autorelease];
}

+(NSNumber *)numberWithDouble:(double)value {
   return [[[self allocWithZone:NULL] initWithDouble:value] autorelease];
}

+(NSNumber *)numberWithInteger:(NSInteger)value {
#if defined(__LP64__)
   return [self numberWithLong:value];
#else
   return [self numberWithInt:value];
#endif
}

+(NSNumber *)numberWithUnsignedChar:(unsigned char)value {
   return [[[self allocWithZone:NULL] initWithUnsignedChar:value] autorelease];
}

+(NSNumber *)numberWithUnsignedShort:(unsigned short)value {
   return [[[self allocWithZone:NULL] initWithUnsignedShort:value] autorelease];
}

+(NSNumber *)numberWithUnsignedInt:(unsigned int)value {
   return [[[self allocWithZone:NULL] initWithUnsignedInt:value] autorelease];
}

+(NSNumber *)numberWithUnsignedLong:(unsigned long)value {
   return [[[self allocWithZone:NULL] initWithUnsignedLong:value] autorelease];
}

+(NSNumber *)numberWithUnsignedLongLong:(unsigned long long)value {
   return [[[self allocWithZone:NULL] initWithUnsignedLongLong:value] autorelease];
}

+(NSNumber *)numberWithUnsignedInteger:(NSUInteger)value {
#if defined(__LP64__)
   return [self numberWithUnsignedLong:value];
#else
   return [self numberWithUnsignedInt:value];
#endif
}


-(NSComparisonResult)compare:(NSNumber *)other {
    double d1 = [self doubleValue];
    double d2 = [other doubleValue];

    if (d1 == d2)
        return NSOrderedSame;
    else if (d1 > d2)
        return NSOrderedDescending;
    else
        return NSOrderedAscending;
}

-(BOOL)isEqual:other {
   if(self==other)
    return YES;

   if(![other isKindOfClass:objc_lookUpClass("NSNumber")])
    return NO;

   return [self isEqualToNumber:other];
}

-(BOOL)isEqualToNumber:(NSNumber *)other {
   if(self==other)
    return YES;

   return ([self compare:other]==NSOrderedSame)?YES:NO;
}


-(NSUInteger)hash {
   return [self doubleValue];
}

-(BOOL)boolValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(char)charValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(short)shortValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(int)intValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(long)longValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(long long)longLongValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(float)floatValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(double)doubleValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSDecimal)decimalValue {
   NSDecimal result = {0};
   NSInvalidAbstractInvocation();
   return result;
}

-(NSInteger)integerValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(unsigned char)unsignedCharValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(unsigned short)unsignedShortValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(unsigned int)unsignedIntValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(unsigned long)unsignedLongValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(unsigned long long)unsignedLongLongValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSUInteger)unsignedIntegerValue {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSString *)stringValue {
   return [self description];
}

-(NSString *)descriptionWithLocale:(NSDictionary *)locale {
   NSInvalidAbstractInvocation();
   return [NSString stringWithFormat:@"<%@ %p>",isa,self];
}

-(NSString *)description {
   return [self descriptionWithLocale:nil];
}

@end

#import <Foundation/NSCFTypeID.h>

@implementation NSNumber (CFTypeID)

- (unsigned) _cfTypeID
{
   return kNSCFTypeNumber;
}

@end
