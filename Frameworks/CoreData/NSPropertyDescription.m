/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreData/NSPropertyDescription.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCoder.h>

@implementation NSPropertyDescription

-init {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithCoder:(NSCoder *)coder {
   if(![coder allowsKeyedCoding])
    [NSException raise: NSInvalidArgumentException format: @"%@ can not initWithCoder:%@", isa, [coder class]];

   _entity=[coder decodeObjectForKey: @"NSEntity"];
   _propertyName=[[coder decodeObjectForKey: @"NSPropertyName"] copy];
   
   return self;
}


- (void) encodeWithCoder: (NSCoder *) coder {
    NSInvalidAbstractInvocation();
}


- (BOOL) isEqual: (id) object {
    if(![object isKindOfClass: [NSPropertyDescription class]])
	return NO;
    NSPropertyDescription *property = (NSPropertyDescription *) object;
    if((_entity == [property entity]) &&
       [_propertyName isEqualToString: [property name]])
	return YES;
    else
	return NO;
}


- (id) copyWithZone: (NSZone *) zone {
    return [self retain];
}


- (NSEntityDescription *) entity {
    return _entity;
}


- (NSString *) name {
    return _propertyName;
}


- (BOOL) isOptional {
    NSUnimplementedMethod();
    return NO;
}


- (BOOL) isTransient {
    NSUnimplementedMethod();
    return NO;
}


- (NSDictionary *) userInfo {
    NSUnimplementedMethod();
    return nil;
}


- (NSArray *) validationPredicates {
    NSUnimplementedMethod();
    return nil;
}


- (NSArray *) validationWarnings {
    NSUnimplementedMethod();
    return nil;
}


- (void) setName: (NSString *) value {
    NSUnimplementedMethod();
}


- (void) setOptional: (BOOL) value {
    NSUnimplementedMethod();
}


- (void) setTransient: (BOOL) value {
    NSUnimplementedMethod();
}


- (void) setUserInfo: (NSDictionary *) value {
    NSUnimplementedMethod();
}


- (void) setValidationPredicates: (NSArray *) predicates
	  withValidationWarnings: (NSArray *) warnings
{
    NSUnimplementedMethod();
}


@end
