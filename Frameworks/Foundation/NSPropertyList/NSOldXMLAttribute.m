/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import "NSOldXMLAttribute.h"
#import <Foundation/NSString.h>

@implementation NSOldXMLAttribute

+(NSOldXMLAttribute *)attributeWithName:(NSString *)name value:(NSString *)value {
   return [[[self alloc] initWithName:name value:value] autorelease];
}

-initWithName:(NSString *)name value:(NSString *)value {
   _name=[name copy];
   _value=[value copy];
   return self;
}

-(void)dealloc {
   [_name release];
   [_value release];
   [super dealloc];
}

-(NSString *)name {
   return _name;
}

-(NSString *)value {
   return _value;
}

-(NSString *)stringValue {
   return _value;
}

-(float)floatValue {
   return [_value floatValue];
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %@=%@>",[self class],_name,_value];
}

@end
