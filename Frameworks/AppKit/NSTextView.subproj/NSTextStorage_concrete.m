/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSTextStorage_concrete.h"
#import <Foundation/NSKeyedArchiver.h>

@implementation NSTextStorage_concrete

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];
   
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _delegate=[keyed decodeObjectForKey:@"NSDelegate"];
    _string=[[keyed decodeObjectForKey:@"NSString"] retain];
    _rangeToAttributes=NSCreateRangeToCopiedObjectEntries(0);
    NSRangeEntryInsert(_rangeToAttributes,NSMakeRange(0,[_string length]),[NSDictionary dictionary]);
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}


-initWithString:(NSString *)string {
   [super initWithString:string];
   _string=[string mutableCopy];
   _rangeToAttributes=NSCreateRangeToCopiedObjectEntries(0);
   NSRangeEntryInsert(_rangeToAttributes,NSMakeRange(0,[_string length]),[NSDictionary dictionary]);
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

-(NSDictionary *)attributesAtIndex:(NSUInteger)location effectiveRange:(NSRangePointer)effectiveRangep {
   NSDictionary *result;

   NSAssert2(location<=[self length],@"index %d beyond length %d",location,[self length]);

   if((result=NSRangeEntryAtIndex(_rangeToAttributes,location,effectiveRangep))==nil)
    result=[NSDictionary dictionary];

   if(effectiveRangep!=NULL && effectiveRangep->length==NSNotFound)
    effectiveRangep->length=[self length]-effectiveRangep->location;

	return [[result retain] autorelease];
}

static inline int replaceCharactersInRangeWithString(NSTextStorage_concrete *self,NSRange range,NSString *string){
   int delta=[string length]-range.length;

   [self->_string replaceCharactersInRange:range withString:string];

//NSRangeEntriesDump(self->_rangeToAttributes);

   NSRangeEntriesExpandAndWipe(self->_rangeToAttributes,range,delta);
   if(NSCountRangeEntries(self->_rangeToAttributes)==0)
    NSRangeEntryInsert(self->_rangeToAttributes,NSMakeRange(0,[self->_string length]),[NSDictionary dictionary]);

NSRangeEntriesVerify(self->_rangeToAttributes,[self length]);

   return delta;
}

static inline void setAttributes(NSTextStorage_concrete *self,NSDictionary *attributes,NSRange range){
   if(attributes==nil)
    attributes=[NSDictionary dictionary];

   if([self->_string length]==0){
    NSResetRangeEntries(self->_rangeToAttributes);
    NSRangeEntryInsert(self->_rangeToAttributes,range,attributes);
   }
   else if(range.length>0){
    NSRangeEntriesDivideAndConquer(self->_rangeToAttributes,range);
    NSRangeEntryInsert(self->_rangeToAttributes,range,attributes);
   }

NSRangeEntriesVerify(self->_rangeToAttributes,[self length]);

}

static inline void replaceCharactersInRangeWithAttributedString(NSTextStorage_concrete *self,NSRange replaced,NSAttributedString *other) {
   NSString *string=[other string];
   unsigned location=0;
   unsigned limit=[string length];
   int      delta=replaceCharactersInRangeWithString(self,replaced,string);

   while(location<limit){
    NSRange       effectiveRange;
    NSDictionary *attributes=[other attributesAtIndex:location effectiveRange:&effectiveRange];
    NSRange       range=NSMakeRange(replaced.location+location,effectiveRange.length);

    setAttributes(self,attributes,range);

    location=NSMaxRange(effectiveRange);
   }

	if (limit == 0) {
          // That will just try to merge attributes at the location when possible
          NSRangeEntryInsert(self->_rangeToAttributes,NSMakeRange(replaced.location,0),nil);
	}
	
   [self edited:NSTextStorageEditedAttributes|NSTextStorageEditedCharacters range:replaced changeInLength:delta];
}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
   int delta=replaceCharactersInRangeWithString(self,range,string);
   [self edited:NSTextStorageEditedAttributes|NSTextStorageEditedCharacters range:range changeInLength:delta];
}

-(void)setAttributes:(NSDictionary *)attributes range:(NSRange)range {
   setAttributes(self,attributes,range);
   [self edited:NSTextStorageEditedAttributes range:range changeInLength:0];
}

-(void)replaceCharactersInRange:(NSRange)replaced withAttributedString:(NSAttributedString *)other {
   replaceCharactersInRangeWithAttributedString(self,replaced,other);
}

-(void)setAttributedString:(NSAttributedString *)attributedString {
   [self beginEditing];
   replaceCharactersInRangeWithAttributedString(self,NSMakeRange(0,[self length]),attributedString);
   [self endEditing];
}

-(NSMutableString *)mutableString {
   return [[[NSClassFromString(@"NSMutableString_proxyToMutableAttributedString") allocWithZone:NULL] performSelector:@selector(initWithMutableAttributedString:) withObject:self] autorelease];
}

-(void)fixAttributesAfterEditingRange:(NSRange)range {
}


@end
