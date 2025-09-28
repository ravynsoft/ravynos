/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSAttributedString_manyAttributes.h"
#import <Foundation/NSRaise.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSRaiseException.h>

@implementation NSAttributedString_manyAttributes

-initWithAttributedString:(NSAttributedString *)other {
   NSUInteger location=0;
   NSUInteger limit=[other length];

   _string=[[other string] copy];
   _rangeToAttributes=NSCreateRangeToCopiedObjectEntries(0);

   while(location<limit){
    NSRange       effectiveRange;
    NSDictionary *attributes=[other attributesAtIndex:location effectiveRange:&effectiveRange];

    NSRangeEntryInsert(_rangeToAttributes,effectiveRange,attributes);

    location=NSMaxRange(effectiveRange);
   }

   return self;
}

-(void)dealloc {
   [_string release];
   NSFreeRangeEntries(_rangeToAttributes);
   [super dealloc];
}

-(NSString *)string {
   return _string;
}

-(NSDictionary *)attributesAtIndex:(NSUInteger)location
   effectiveRange:(NSRange *)effectiveRangep {
   NSDictionary *result;

   if(location>=[self length])
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",location,[self length]);

   if((result=NSRangeEntryAtIndex(_rangeToAttributes,location,effectiveRangep))==nil)
    result=[NSDictionary dictionary];

   if(effectiveRangep!=NULL && effectiveRangep->length==NSNotFound)
    effectiveRangep->length=[self length]-effectiveRangep->location;

	return [[result retain] autorelease];
}

@end
