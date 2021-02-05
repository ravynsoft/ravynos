/* Copyright (c) 2007 Dirk Theisen

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSIndexPath.h>
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSRaise.h>
#include <string.h>
#include <stdlib.h>

@implementation NSIndexPath

+ (NSIndexPath*) indexPathWithIndex: (NSUInteger) index {
	return [[[self alloc] initWithIndexes: &index length: 1] autorelease];
}

+ (NSIndexPath*) indexPathWithIndexes: (NSUInteger*) indexes length: (NSUInteger) length {
	return [[[self alloc] initWithIndexes: indexes length: length] autorelease];
}

-  initWithIndex: (NSUInteger) index {
	return [self initWithIndexes: &index length: 1];
}

-  initWithIndexes: (NSUInteger*) indexes length: (NSUInteger) length {
   int i;

   _length  = length;
   _indexes = NSZoneMalloc(NULL,length*sizeof(NSUInteger));

   for(i=0;i<length;i++)
    _indexes[i]=indexes[i];

   return self;
}

- (BOOL) isEqual:  other {
	if ([other isKindOfClass: [NSIndexPath class]]) {
		NSIndexPath* otherPath = other;
		return _length == otherPath->_length && memcmp(_indexes, otherPath->_indexes, _length);
	}
	return NO;
}

- (void) dealloc {
   if (_indexes)
    free(_indexes);
   [super dealloc];
}

- (NSIndexPath*) indexPathByAddingIndex: (NSUInteger) index {
   NSUInteger length=[self length];
   NSUInteger buffer[length+1];

   [self getIndexes:buffer];
   buffer[length]=index;
   length++;

   return [[[NSIndexPath alloc] initWithIndexes:buffer length:length] autorelease];
}

- (NSIndexPath*) indexPathByRemovingLastIndex {
   if(_length==0){
	[NSException raise:NSInvalidArgumentException format:@"Unable to remove index from zero length path."];
    return nil;
   }

   return [[[NSIndexPath alloc] initWithIndexes: _indexes length: _length-1] autorelease];
}

- (NSUInteger) indexAtPosition: (NSUInteger) position {
   if(position>=_length){
	[NSException raise:NSInvalidArgumentException format:@"Unable to remove index from zero length path."];
    return 0;
   }
   return _indexes[position];
}

- (NSUInteger) length {
	return _length;
}

- (void) getIndexes: (NSUInteger*) indexes {
	memcpy(indexes, _indexes, _length * sizeof(NSUInteger));
}

/** Note: Sorting an array of indexPaths using this comparison method results in an array representing nodes in depth-first traversal order. */
- (NSComparisonResult) compare: (NSIndexPath*) otherObject {
	int i;
	for (i=0; i<_length; i++) {
		if (_indexes[i] != otherObject->_indexes[i]) {
			return _indexes[i] < otherObject->_indexes[i] ? NSOrderedAscending : NSOrderedDescending;
		}
	}
	return NSOrderedSame;
}

-copyWithZone:(NSZone*) zone {
   return [self retain];
}


-copy  {
   return [self retain];
}

-(NSUInteger) hash {
   NSUInteger result=0;
   int i;

   for(i=0;i<_length;i++){
    result = result*2 + _indexes[i];
   }
   result = 2*result + _length;

   return result;
}

-(void)encodeWithCoder: (NSCoder*) coder {
   NSUnimplementedMethod();
}

- initWithCoder: (NSCoder*) coder {
   NSUnimplementedMethod();
   return nil;
}

@end
