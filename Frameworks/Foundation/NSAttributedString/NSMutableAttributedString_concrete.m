/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSMutableAttributedString_concrete.h"
#import <Foundation/NSMutableString_proxyToMutableAttributedString.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSRaiseException.h>

@implementation NSMutableAttributedString_concrete

-initWithString:(NSString *)string {
   _string=[string mutableCopy];
   _rangeToAttributes=NSCreateRangeToCopiedObjectEntries(0);
   NSRangeEntryInsert(_rangeToAttributes,NSMakeRange(0,[_string length]),[NSDictionary dictionary]);
   return self;
}

/*
 * NOTE: Cocoa encodes the attribute ranges in an unconventional way. They compress
 * the data down to a byte encoding with the following format:
 *  <runlength><attribute set>
 * where runlength is the length that the attribute set applies and attribute set is an
 * index or id pointing to an attribute dictionary.
 * Things get trickier when the runlength exceeds 127 - in that case a multi-byte encoding is used
 * where the first byte has the top bit set to indicate the following byte is a multiplier - this
 * is extended for runs greater than 127*127 - but this implementation doesn't go that far.
 *
 * Decoding simply reverses the process.
 */
static void encodeIntoData(NSMutableData* data, NSUInteger value)
{
	char c = value/127;
	char v = value % 127;
	if (c > 0) {
		v |= 0x80;
	}
	[data appendBytes:&v length: 1];
	if (c > 0) {
		[data appendBytes:&c length: 1];
	}
}

static NSUInteger decodeFromData(NSData* data, NSUInteger offset, NSUInteger *value)
{
	*value = 0;
	const char* bytes = [data bytes];
	
	char v = bytes[offset++];
	if (v & 0x80) {
		char c = bytes[offset++];
		v ^= 0x80;
		*value = v * c;
	} else {
		*value = v;
	}
	return offset;
}

- (id)initWithCoder:(NSCoder *)coder
{
	if([coder isKindOfClass:[NSKeyedUnarchiver class]]){
		NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
		NSString* string = [keyed decodeObjectForKey: @"NSString"];
		id attributes = [keyed decodeObjectForKey: @"NSAttributes"];
		if (attributes == nil) {
			return [self initWithString: string];
		} else if ([attributes isKindOfClass: [NSDictionary class]]) {
			return [self initWithString: string attributes: attributes];
		} else {
			// we've got an array to work through
			NSMutableAttributedString* attrStr = [[[NSMutableAttributedString alloc] initWithString: string] autorelease];
			NSData* data = [keyed decodeObjectForKey: @"NSAttributeInfo"];
			NSArray* attributesArray = attributes;
			NSUInteger offset =0;
            NSUInteger location = 0;
			for (NSUInteger i = 0; i < [attributesArray count]; i++) {
				NSUInteger length = 0;
				NSUInteger index = 0;
				// See note above on encoding and decoding
				offset = decodeFromData(data, offset, &length);
				offset = decodeFromData(data, offset, &index);

                NSDictionary* dict = [attributesArray objectAtIndex: index];
				NSRange range = NSMakeRange(location, length);
				[attrStr addAttributes: dict range: range];
                location += length;
			}
			return [self initWithAttributedString: attrStr];
		}
	} else {
		NSUnimplementedMethod();
	}
	return self;
}

-(void)encodeWithCoder:(NSCoder *)coder
{
	if([coder isKindOfClass:[NSKeyedArchiver class]]){
		NSKeyedArchiver *keyed=(NSKeyedArchiver *)coder;
		
		[keyed encodeObject: [self string] forKey:@"NSString"];
		NSRange range;
		NSDictionary* dict = [self attributesAtIndex: 0 effectiveRange: &range];
		if (dict == nil || range.length == [self length]) {
			[keyed encodeObject: dict forKey: @"NSAttributes"];
		} else {
			NSMutableArray* attributesArray = [NSMutableArray arrayWithCapacity: 10];
			// we've got more than one set of attributes so we have to encode more data
			NSUInteger count = NSCountRangeEntries(_rangeToAttributes);
			NSMutableData* data = [NSMutableData dataWithCapacity: count * 4];
            NSRangeEnumerator enumerator = NSRangeEntryEnumerator(_rangeToAttributes);
            NSDictionary *attributes = nil;
            int i = 0;
            while (NSNextRangeEnumeratorEntry(&enumerator,&range,(void**)&attributes)) {
 				[attributesArray addObject: attributes];
				// See note above on encoding and decoding
				encodeIntoData(data, range.length);
				encodeIntoData(data, i++);
            }
			[keyed encodeObject: attributesArray forKey: @"NSAttributes"];
			[keyed encodeObject: data forKey: @"NSAttributeInfo"];
		}
	} else {
		NSUnimplementedMethod();
	}	
}

-(void)dealloc {
   [_string release];
   NSFreeRangeEntries(_rangeToAttributes);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(NSString *)string {
	// This string is mutable - so be kind to unwary callers
   return [[_string retain] autorelease];
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

	// The string could be being mutated so these attributes could disappear on an unwary caller
   return [[result retain] autorelease];
}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
   NSInteger delta=[string length]-range.length;

   [_string replaceCharactersInRange:range withString:string];
   NSRangeEntriesExpandAndWipe(_rangeToAttributes,range,delta);
   if(NSCountRangeEntries(_rangeToAttributes)==0)
    NSRangeEntryInsert(_rangeToAttributes,NSMakeRange(0,[_string length]),[NSDictionary dictionary]);

NSRangeEntriesVerify(_rangeToAttributes,[self length]);
}

-(void)setAttributes:(NSDictionary *)attributes range:(NSRange)range {
   if(attributes==nil)
    attributes=[NSDictionary dictionary];
   else
    attributes=[[attributes copy] autorelease];

   if([_string length]==0){
    NSResetRangeEntries(_rangeToAttributes);
    NSRangeEntryInsert(_rangeToAttributes,range,attributes);
   }
   else if(range.length>0){
    NSRangeEntriesDivideAndConquer(_rangeToAttributes,range);
    NSRangeEntryInsert(_rangeToAttributes,range,attributes);
   }

NSRangeEntriesVerify(_rangeToAttributes,[self length]);
}

-(NSMutableString *)mutableString {
   return [[[NSMutableString_proxyToMutableAttributedString allocWithZone:NULL] initWithMutableAttributedString:self] autorelease];
}

-(void)fixAttributesAfterEditingRange:(NSRange)range {
}


@end
