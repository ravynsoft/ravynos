/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSValue.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSValue_concrete.h>
#import <Foundation/NSValue_placeholder.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSValue_nonRetainedObject.h>
#import <Foundation/NSValue_pointer.h>
#import <Foundation/NSData_concrete.h>
#include <string.h>

@implementation NSValue

+allocWithZone:(NSZone *)zone {
   if(self==[NSValue class])
    return NSAllocateObject([NSValue_placeholder class],0,NULL);

   return NSAllocateObject(self,0,zone);
}

-initWithBytes:(const void *)value objCType:(const char *)type {
   NSInvalidAbstractInvocation();
   return nil;
}

+(NSValue *)valueWithBytes:(const void *)value objCType:(const char *)type {
   if(self==[NSValue class])
    return NSAutorelease(NSValue_concreteNew(NULL,value,type));

   return [[[self allocWithZone:NULL] initWithBytes:value objCType:type] autorelease];
}

+(NSValue *)value:(const void *)value withObjCType:(const char *)type {
   if(self==[NSValue class])
    return NSAutorelease(NSValue_concreteNew(NULL,value,type));

   return [[[self allocWithZone:NULL] initWithBytes:value objCType:type] autorelease];
}

+(NSValue *)valueWithNonretainedObject:object {
   return [[[NSValue_nonRetainedObject allocWithZone:NULL] initWithObject:object] autorelease];
}

+(NSValue *)valueWithPointer:(const void *)pointer {
   return [[[NSValue_pointer allocWithZone:NULL] initWithPointer:pointer] autorelease];
}

+(NSValue *)valueWithPoint:(NSPoint)point {
   return [NSValue value:&point withObjCType:@encode(NSPoint)];
}


+(NSValue *)valueWithSize:(NSSize)size {
   return [NSValue value:&size withObjCType:@encode(NSSize)];
}


+(NSValue *)valueWithRect:(NSRect)rect {
   return [NSValue value:&rect withObjCType:@encode(NSRect)];
}

+(NSValue *)valueWithRange:(NSRange)range {
   return [NSValue value:&range withObjCType:@encode(NSRange)];
}

-(BOOL)isEqual:other {
   if(![other isKindOfClass:[NSValue class]])
    return NO;

   return [self isEqualToValue:other];
}

-(BOOL)isEqualToValue:(NSValue *)other {
   const char *type=[self objCType];
   NSUInteger    size,alignment;
   void       *selfData,*otherData;

   if(self==other)
    return YES;

   if(strcmp(type,[other objCType])!=0)
    return NO;

   NSGetSizeAndAlignment(type,&size,&alignment);
   selfData=__builtin_alloca(size);
   otherData=__builtin_alloca(size);

   [self getValue:selfData];
   [other getValue:otherData];

   return NSBytesEqual(selfData,otherData,size);
}

-(void)getValue:(void *)value {
   NSInvalidAbstractInvocation();
}

-(const char *)objCType {
   NSInvalidAbstractInvocation();
   return NULL;
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-(Class)classForCoder {
   return objc_lookUpClass("NSValue");
}

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-nonretainedObjectValue {
   const char *type=[self objCType];
   NSUInteger    size,alignment;
   id          value;

   NSGetSizeAndAlignment(type,&size,&alignment);
   if(size!=sizeof(id))
    return nil;

   [self getValue:&value];
   return value;
}

-(void *)pointerValue {
   const char *type=[self objCType];
   NSUInteger    size,alignment;
   void       *value;

   NSGetSizeAndAlignment(type,&size,&alignment);
   if(size!=sizeof(void *))
    return NULL;

   [self getValue:&value];
   return value;
}

-(NSPoint)pointValue {
   NSPoint point;
   [self getValue:&point];
   return point;
}

-(NSSize)sizeValue {
   NSSize size;
   [self getValue:&size];
   return size;
}

-(NSRange)rangeValue {
	NSRange range;
	[self getValue:&range];
	return range;
}

-(NSRect)rectValue {
   NSRect rect;
   [self getValue:&rect];
   return rect;
}

-(id)descriptionWithLocale:(id)locale
{
   return [self description];
}


-(id)description
{
   return [NSString stringWithFormat:@"<%@, %s>", [super description], [self objCType]];
}
@end
