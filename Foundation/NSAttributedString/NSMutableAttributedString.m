/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSMutableAttributedString.h>
#import "NSMutableAttributedString_concrete.h"
#import <Foundation/NSRaise.h>
#import <Foundation/NSDictionary.h>

@implementation NSMutableAttributedString

+allocWithZone:(NSZone *)zone {
   if(self==[NSMutableAttributedString class])
    return NSAllocateObject([NSMutableAttributedString_concrete class],0,NULL);

   return NSAllocateObject(self,0,zone);
}

-initWithString:(NSString *)string attributes:(NSDictionary *)attributes {
   self=[self initWithString:string];
   [self setAttributes:attributes range:NSMakeRange(0,[string length])];
   return self;
}

-initWithAttributedString:(NSAttributedString *)other {
   self=[self init];
   [self setAttributedString:other];
   return self;
}

-(Class)classForCoder {
	return objc_lookUpClass("NSMutableAttributedString");
}

-(id)copyWithZone:(NSZone *)zone {
   return [[NSAttributedString allocWithZone:NULL] initWithAttributedString:self];
}

-(NSMutableString *)mutableString {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)addAttribute:(NSString *)name value:(id)value range:(NSRange)range {
   [self addAttributes:[NSDictionary dictionaryWithObject:value forKey:name] range:range];
}

-(void)addAttributes:(NSDictionary *)attributes range:(NSRange)range {
   NSUInteger location=range.location;
   NSUInteger limit=NSMaxRange(range);

   while(location<limit){
    NSRange       effectiveRange;
    NSMutableDictionary *modify=[[[self attributesAtIndex:location effectiveRange:&effectiveRange] mutableCopy] autorelease];
    NSRange       replace;

    [modify addEntriesFromDictionary:attributes];

    replace.location=MAX(location,effectiveRange.location);
    replace.length=MIN(NSMaxRange(range),NSMaxRange(effectiveRange))-replace.location;

    [self setAttributes:modify range:replace];

    location=NSMaxRange(replace);
   }
}

-(void)appendAttributedString:(NSAttributedString *)attributedString {
   [self replaceCharactersInRange:NSMakeRange([self length],0) withAttributedString:attributedString];
}

-(void)deleteCharactersInRange:(NSRange)range {
   [self replaceCharactersInRange:range withString:@""];
}

-(void)removeAttribute:(NSString *)name range:(NSRange)range {
   NSUInteger location=range.location;
   // TODO, raise exception if beyond length
   NSUInteger limit=MIN(NSMaxRange(range),[self length]);

   while(location<limit){
    NSRange       effectiveRange;
    NSDictionary *check=[self attributesAtIndex: location effectiveRange:&effectiveRange];
    NSRange       replace;
    
    replace.location=location;
    replace.length=MIN(NSMaxRange(range),NSMaxRange(effectiveRange))-location;

    if([check objectForKey:name]!=nil){
     NSMutableDictionary *modify=[[check mutableCopy] autorelease];

     [modify removeObjectForKey:name];

     [self setAttributes:modify range:replace];
    }
    
    location=NSMaxRange(replace);
   }
}

-(void)insertAttributedString:(NSAttributedString *)attributedString atIndex:(NSUInteger)index {
   [self replaceCharactersInRange:NSMakeRange(index,0) withAttributedString:attributedString];
}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
   NSInvalidAbstractInvocation();
}

-(void)replaceCharactersInRange:(NSRange)replaced withAttributedString:(NSAttributedString *)other {
   NSString *string=[other string];
   NSUInteger location=0;
   NSUInteger limit=[string length];

   [self replaceCharactersInRange:replaced withString:string];

   while(location<limit){
    NSRange       effectiveRange;
    NSDictionary *attributes=[other attributesAtIndex:location effectiveRange:&effectiveRange];

    [self setAttributes:attributes
         range:NSMakeRange(replaced.location+location,effectiveRange.length)];

    location=NSMaxRange(effectiveRange);
   }
}

-(void)setAttributes:(NSDictionary *)attributes range:(NSRange)range {
   NSInvalidAbstractInvocation();
}

-(void)setAttributedString:(NSAttributedString *)attributedString {
   [self replaceCharactersInRange:NSMakeRange(0,[self length]) withAttributedString:attributedString];
}

-(void)beginEditing {
  // do nothing
}

-(void)endEditing {
  // do nothing
}

@end
