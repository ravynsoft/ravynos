/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreData/NSAttributeDescription.h>
#import <CoreData/NSEntityDescription.h>
#import <Foundation/NSKeyedUnarchiver.h>

@implementation NSAttributeDescription

-initWithCoder:(NSCoder *)coder {
   if(![coder allowsKeyedCoding])
    [NSException raise: NSInvalidArgumentException format: @"%@ can not initWithCoder:%@", isa, [coder class]];

   [super initWithCoder:coder];

   _attributeType = [coder decodeIntForKey: @"NSAttributeType"];
   _valueClassName= [[coder decodeObjectForKey: @"NSAttributeValueClassName"] retain];
   _defaultValue = [[coder decodeObjectForKey: @"NSDefaultValue"] retain];
   _valueTransformerName= [[coder decodeObjectForKey: @"NSValueTransformerName"] retain];
   if(_valueTransformerName!=nil)
    NSLog(@"UNIMPLEMENTED _valueTransformerName=%@",_valueTransformerName);
    
   return self;
}


-copyWithZone: (NSZone *) zone {
   return [self retain];
}


-(void)dealloc {
   [_valueClassName release];
   [_defaultValue release];
   [_propertyName release];
   [_valueTransformerName release];
   [super dealloc];
}


- (NSString *) description {
    return [NSString stringWithFormat: @"<NSAttributeDescription: %@ %@>",_valueClassName, _propertyName];
}


- (NSString *) attributeValueClassName {
    return _valueClassName;
}


- (NSAttributeType) attributeType {
    return _attributeType;
}


- (id) defaultValue {
    return _defaultValue;
}


- (void) setAttributeType: (NSAttributeType) value {
    if([_entity _hasBeenInstantiated]) {
	NSLog(@"Attempt to modify entity after instantiating it.");
	return;
    }
    
    _attributeType = value;
}


- (void) setDefaultValue: (id) value {
    if([_entity _hasBeenInstantiated]) {
	NSLog(@"Attempt to modify entity after instantiating it.");
	return;
    }
    
    value=[value retain];
    [_defaultValue release];
    _defaultValue=value;
}

@end

