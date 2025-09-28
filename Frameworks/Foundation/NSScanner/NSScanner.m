/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSScanner.h>
#import <Foundation/NSScanner_concrete.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSLocale.h>

@implementation NSScanner

+allocWithZone:(NSZone *)zone {
   if(self==[NSScanner class])
    return NSAllocateObject([NSScanner_concrete class],0,NULL);

   return NSAllocateObject(self,0,zone);
}

+scannerWithString:(NSString *)string {
    return [[[self allocWithZone:NULL] initWithString:string] autorelease];
}

+localizedScannerWithString:(NSString *)string {
    NSScanner *scanner = [self scannerWithString:string];

    [scanner setLocale:[NSLocale currentLocale]];

    return scanner;
}

-initWithString:(NSString *)string {
    NSInvalidAbstractInvocation();
    return nil;
}

-copyWithZone:(NSZone *)zone {
   NSScanner *copy=[[NSScanner alloc] initWithString:[self string]];
   
   [copy setCharactersToBeSkipped:[self charactersToBeSkipped]];
   [copy setCaseSensitive:[self caseSensitive]];
   [copy setLocale:[self locale]];
   [copy setScanLocation:[self scanLocation]];
   
   return copy;
}

-(NSString *)string {
    NSInvalidAbstractInvocation();
    return nil;
}

-(NSCharacterSet *)charactersToBeSkipped {
    NSInvalidAbstractInvocation();
    return nil;
}

-(BOOL)caseSensitive {
    NSInvalidAbstractInvocation();
    return NO;
}

-(NSDictionary *)locale {
    NSInvalidAbstractInvocation();
    return nil;
}

-(void)setCharactersToBeSkipped:(NSCharacterSet *)set {
    NSInvalidAbstractInvocation();
}

-(void)setCaseSensitive:(BOOL)flag {
    NSInvalidAbstractInvocation();
}

-(void)setLocale:(NSDictionary *)locale {
    NSInvalidAbstractInvocation();
}

-(BOOL)isAtEnd {
    NSInvalidAbstractInvocation();
    return NO;
}

-(NSUInteger)scanLocation {
    NSInvalidAbstractInvocation();
    return -1;
}

-(void)setScanLocation:(NSUInteger)location {
    NSInvalidAbstractInvocation();
}

-(BOOL)scanInt:(int *)value {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanLongLong:(long long *)value {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanFloat:(float *)value {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanDouble:(double *)value {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanDecimal:(NSDecimal *)valuep {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanInteger:(NSInteger *)valuep {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanHexInt:(unsigned *)value {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanHexLongLong:(unsigned long long *)value {
	NSInvalidAbstractInvocation();
	return NO;
}

-(BOOL)scanString:(NSString *)string intoString:(NSString **)stringp {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanUpToString:(NSString *)string intoString:(NSString **)stringp {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanCharactersFromSet:(NSCharacterSet *)charset intoString:(NSString **)stringp {
    NSInvalidAbstractInvocation();
    return NO;
}

-(BOOL)scanUpToCharactersFromSet:(NSCharacterSet *)charset intoString:(NSString **)stringp {
    NSInvalidAbstractInvocation();
    return NO;
}

@end
