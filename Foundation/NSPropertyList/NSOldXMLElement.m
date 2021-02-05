/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import "NSOldXMLElement.h"
#import "NSOldXMLAttribute.h"
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSScanner.h>

@implementation NSOldXMLElement

+(NSOldXMLElement *)elementWithName:(NSString *)name {
   return [[[self alloc] initWithName:name] autorelease];
}

-initWithName:(NSString *)name {
   _name=[name copy];
   _attributes=[NSMutableArray new];
   _contents=[NSMutableArray new];
   return self;
}

-(void)dealloc {
   [_name release];
   [_attributes release];
   [_contents release];
   [super dealloc];
}

-(NSString *)name {
   return _name;
}

-(NSArray *)attributes {
   return _attributes;
}

-(NSArray *)contents {
   return _contents;
}

-(NSString *)xid {
   return [[self attributeWithName:@"id"] value];
}

-(NSOldXMLAttribute *)attributeWithName:(NSString *)name {
   NSInteger i,count=[_attributes count];

   for(i=0;i<count;i++){
    NSOldXMLAttribute *check=[_attributes objectAtIndex:i];

    if([[check name] isEqualToString:name])
     return check;
   }
   return nil;
}

-(void)addAttribute:(NSOldXMLAttribute *)attribute {
   [_attributes addObject:attribute];
}

-(void)addContent:(id)content {
   [_contents addObject:content];
}

-(NSString *)stringValue {
   NSInteger i,count=[_contents count];

   if(count==0)
    return @"";
   if(count==1)
    return [_contents lastObject];
   else {
    NSMutableString *result=[NSMutableString string];

    for(i=0;i<count;i++)
     [result appendString:[_contents objectAtIndex:i]];

    return result;
   }
}

-(int)intValue {
   return [[self stringValue] intValue];
}

-(unsigned)unsignedIntValue {
   return [[self stringValue] intValue];
}

-(float)floatValue {
   return [[self stringValue] floatValue];
}

-(NSRect)rectValue {
   NSRect     result=NSZeroRect;
   NSScanner *scanner=[NSScanner scannerWithString:[self stringValue]];
   double     value;
   
   [scanner scanDouble:&value];
   result.origin.x=value;
   [scanner scanDouble:&value];
   result.origin.y=value;
   [scanner scanDouble:&value];
   result.size.width=value;
   [scanner scanDouble:&value];
   result.size.height=value;

   return result;
}

-(NSSize)sizeValue {
   NSSize     result=NSZeroSize;
   NSScanner *scanner=[NSScanner scannerWithString:[self stringValue]];
   double     value;
   
   [scanner scanDouble:&value];
   result.width=value;
   [scanner scanDouble:&value];
   result.height=value;

   return result;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %@ %@ %@>",[self class],
     _name,_attributes,_contents];
}

-(NSOldXMLElement *)nextElement {
   NSOldXMLElement *next;

   if([_contents count]==0)
    next=nil;
   else {
    next=[[[_contents objectAtIndex:0] retain] autorelease];
    [_contents removeObjectAtIndex:0];
   }

   return next;
}

@end
