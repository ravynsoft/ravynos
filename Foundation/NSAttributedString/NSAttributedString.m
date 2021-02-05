/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSMutableAttributedString.h>
#import <Foundation/NSAttributedString_placeholder.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSRaise.h>

@implementation NSAttributedString

+allocWithZone:(NSZone *)zone {
   if(self==[NSAttributedString class])
    return NSAllocateObject([NSAttributedString_placeholder class],0,NULL);

   return NSAllocateObject(self,0,zone);
}

-init {
   return [self initWithString:@""];
}

-initWithString:(NSString *)string {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithString:(NSString *)string attributes:(NSDictionary *)attributes {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithAttributedString:(NSAttributedString *)other {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)encodeWithCoder:(NSCoder *)coder {
	NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
	NSUnimplementedMethod();
   return self;
}

-copy {
   return [self retain];
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-mutableCopy {
   return [[NSMutableAttributedString allocWithZone:NULL] initWithAttributedString:self];
}

-mutableCopyWithZone:(NSZone *)zone {
   return [[NSMutableAttributedString allocWithZone:zone] initWithAttributedString:self];
}

-(BOOL)isEqualToAttributedString:(NSAttributedString *)other {
	if ([self length] != [other length]) {
		return NO;
	}
	unsigned i = 0;
	NSString* string = [self string];
	NSString* otherString = [other string];
	if (![string isEqualToString:otherString]) {
		return NO;
	}
	unsigned length = [self length];
	NSRange range;
	while(i<length) {
		NSDictionary* attributes = [self attributesAtIndex: i effectiveRange: &range];
		NSRange otherRange;
		NSDictionary* otherAttributes = [other attributesAtIndex: i effectiveRange: &otherRange];
		if (![attributes isEqualToDictionary:otherAttributes]) {
			return NO;
		}
		i = MIN(NSMaxRange(range), NSMaxRange(otherRange));
	}
	return YES;
}

- (NSUInteger)hash
{
	return [[self string] hash];
}

- (BOOL)isEqual:(id)other
{
	if (self == other) {
		return YES;
	}
	if ([other isKindOfClass: [NSAttributedString class]]) {
		return [self isEqualToAttributedString: other];
	}
	return NO;
}

-(NSUInteger)length {
   return [[self string] length];
}

-(NSString *)string {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSDictionary *)attributesAtIndex:(NSUInteger)location effectiveRange:(NSRange *)range {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSDictionary *)attributesAtIndex:(NSUInteger)location
   longestEffectiveRange:(NSRange *)range inRange:(NSRange)inRange {
   NSUnimplementedMethod();
   return nil;
}

-attribute:(NSString *)name atIndex:(NSUInteger)location effectiveRange:(NSRange *)range {
   return [[self attributesAtIndex:location effectiveRange:range] objectForKey:name];
}

-attribute:(NSString *)name atIndex:(NSUInteger)location longestEffectiveRange:(NSRange *)range inRange:(NSRange)inRange {
   NSUnimplementedMethod();
   return nil;
}

-(NSAttributedString *)attributedSubstringFromRange:(NSRange)range {
   NSMutableAttributedString *result=[[[NSMutableAttributedString allocWithZone:NULL] init] autorelease];
   NSUInteger  location=range.location;
   NSUInteger  limit=MIN(NSMaxRange(range), [self length]);

   while(location<limit){
    NSRange         effectiveRange,appendedRange;
    NSDictionary   *attributes=[self attributesAtIndex:location effectiveRange:&effectiveRange];

    if(effectiveRange.location<location){
     effectiveRange.length=NSMaxRange(effectiveRange)-location;
     effectiveRange.location=location;
    }
    if(NSMaxRange(effectiveRange)>limit)
     effectiveRange.length=limit-effectiveRange.location;

    appendedRange.location=[result length];
    appendedRange.length=effectiveRange.length;
    [[result mutableString] appendString:[[self string] substringWithRange:effectiveRange]];
    [result setAttributes:attributes range:appendedRange];

    location=NSMaxRange(effectiveRange);
   }
   return result;
}

- (NSString*)description
{
	if ([self length] > 0) {
		NSMutableString *result = [NSMutableString string];
		unsigned length = [self length];
		int i = 0;
		while(i<length) {
			NSRange range;
			NSDictionary* attributes = [self attributesAtIndex: i effectiveRange: &range];
			NSString *string = [[self string] substringWithRange:range];
			[result appendFormat:@"%@ {\n%@\n}\n", string, attributes];
			i = NSMaxRange(range);
		}
		return result;
	} else {
		return [NSString stringWithFormat: @"%@ {}", [self string]];
	}
}

@end
