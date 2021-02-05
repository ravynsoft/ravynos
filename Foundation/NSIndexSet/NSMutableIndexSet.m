/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSMutableIndexSet.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSCoder.h> 
#import <Foundation/NSKeyedUnarchiver.h> 
#import <Foundation/NSNumber.h>
#include <limits.h>

// FIX: assert range values on init/insert/remove

@implementation NSMutableIndexSet

-initWithIndexSet:(NSIndexSet *)other {
   [super initWithIndexSet:other];
   _capacity=(_length==0)?1:_length;
   return self;
}

-initWithIndexesInRange:(NSRange)range {
   [super initWithIndexesInRange:range];
   _capacity=(_length==0)?1:_length;
   return self;
}

-copyWithZone:(NSZone *)zone {
   return [[NSIndexSet allocWithZone:zone] initWithIndexSet:self];
}

static NSUInteger positionOfRangeLessThanOrEqualToLocation(NSRange *ranges,NSUInteger length,NSUInteger location){
   NSInteger i=length;
   
   while(--i>=0)
    if(ranges[i].location<=location)
     return i;
         
   return NSNotFound;
}

static void removeRangeAtPosition(NSRange *ranges,NSUInteger length,NSUInteger position){
   NSUInteger i;
   
    for(i=position;i+1<length;i++)
     ranges[i]=ranges[i+1];
}

-(void)_insertRange:(NSRange)range position:(NSUInteger)position {
   NSInteger i;
    
   _length++;
   if(_capacity<_length){
    _capacity*=2;
    _ranges=NSZoneRealloc([self zone],_ranges,sizeof(NSRange)*_capacity);
   }
   for(i=_length;--i>=position+1;)
    _ranges[i]=_ranges[i-1];
     
   _ranges[position]=range;
}

-(void)addIndexesInRange:(NSRange)range {
   NSUInteger pos=positionOfRangeLessThanOrEqualToLocation(_ranges,_length,range.location);
   BOOL     insert=NO;
       
   if(pos==NSNotFound){
    pos=0;
    insert=YES;
   }
   else {
    if(NSMaxRange(range)<=NSMaxRange(_ranges[pos]))
     return; // present
   
    if(range.location<=NSMaxRange(_ranges[pos])) // intersects or adjacent
     _ranges[pos].length=NSMaxRange(range)-_ranges[pos].location;
    else {
     pos++;
     insert=YES;
    }
   }
   
   if(insert)
    [self _insertRange:range position:pos];

   while(pos+1<_length){
    NSUInteger max=NSMaxRange(_ranges[pos]);
    NSUInteger nextMax;
    
    if(max<_ranges[pos+1].location)
     break;
     
    nextMax=NSMaxRange(_ranges[pos+1]);
    if(nextMax>max)
     _ranges[pos].length=nextMax-_ranges[pos].location;
    
    removeRangeAtPosition(_ranges,_length,pos+1);
    _length--;
   }
}

-(void)addIndexes:(NSIndexSet *)other {
   NSInteger i;
   
   for(i=0;i<((NSMutableIndexSet *)other)->_length;i++)
    [self addIndexesInRange:((NSMutableIndexSet *)other)->_ranges[i]];
}

-(void)addIndex:(NSUInteger)index {
   [self addIndexesInRange:NSMakeRange(index,1)];
}

-(void)removeAllIndexes {
   _length=0;
}

-(void)removeIndexesInRange:(NSRange)range {
   NSUInteger pos=positionOfRangeLessThanOrEqualToLocation(_ranges,_length,range.location);

   if(pos==NSNotFound)
    pos=0;

   while(range.length>0 && pos<_length){
    if(_ranges[pos].location>=NSMaxRange(range))
     break;
     
    if(NSMaxRange(_ranges[pos])==NSMaxRange(range)){
   
     if(_ranges[pos].location==range.location){
      removeRangeAtPosition(_ranges,_length,pos);
      _length--;
     }
     else
      _ranges[pos].length=range.location-_ranges[pos].location;
    
     break;
    }
   
    if(NSMaxRange(_ranges[pos])>NSMaxRange(range)){
   
     if(_ranges[pos].location==range.location){
      NSUInteger max=NSMaxRange(_ranges[pos]);
     
      _ranges[pos].location=NSMaxRange(range);
      _ranges[pos].length=max-_ranges[pos].location;
     }
     else {
      NSRange iceberg;
     
      iceberg.location=NSMaxRange(range);
      iceberg.length=NSMaxRange(_ranges[pos])-iceberg.location;
     
      _ranges[pos].length=range.location-_ranges[pos].location;
     
      [self _insertRange:iceberg position:pos+1];
     }
     break;
    }

    if(range.location>=NSMaxRange(_ranges[pos]))
     pos++;
    else {
     NSUInteger max=NSMaxRange(range);
     NSRange  temp=_ranges[pos];
    
     if(_ranges[pos].location>=range.location){
      removeRangeAtPosition(_ranges,_length,pos);
      _length--;
     }
     else {
      _ranges[pos].length=range.location-_ranges[pos].location;
      pos++;
     }    
     range.location=NSMaxRange(temp);
     range.length=max-range.location;
    }
   }
}

-(void)removeIndexes:(NSIndexSet *)other {
   NSInteger i;
   
   for(i=0;i<((NSMutableIndexSet *)other)->_length;i++)
    [self removeIndexesInRange:((NSMutableIndexSet *)other)->_ranges[i]];
}

-(void)removeIndex:(NSUInteger)index {
   [self removeIndexesInRange:NSMakeRange(index,1)];
}

-(void)shiftIndexesStartingAtIndex:(NSUInteger)index by:(NSInteger)delta {

   if(delta<0){
    delta=-delta;
    NSInteger pos=positionOfRangeLessThanOrEqualToLocation(_ranges,_length,index-delta);
    
    if(pos==NSNotFound)
     return; // raise?
     
    NSInteger count=_length;
    
    while(--count>=pos){
     if(_ranges[count].location>=index) // if above index just move it down
      _ranges[count].location-=delta;
     else if(NSMaxRange(_ranges[count])<=index-delta) // below area, ignore
      ;
     else if(_ranges[count].length>delta){ // if below, shorten
      if(NSMaxRange(_ranges[count])-index>=delta) // if deletion entirely inside
       _ranges[count].length-=delta;
      else
       _ranges[count].length=NSMaxRange(_ranges[count])-(index-delta);
     }
     else { // if below and shorter than the delta, remove
       NSInteger i;
        
       _length--;
       for(i=count;i<_length;i++)
        _ranges[i]=_ranges[i+1];
     }
    }
    
   }
   else {
    NSInteger pos=positionOfRangeLessThanOrEqualToLocation(_ranges,_length,index);
        
    if(pos==NSNotFound)
     return; // raise?
    
    // if index is inside a range, split it
    if(_ranges[pos].location<index && index<NSMaxRange(_ranges[pos])){
     NSRange below=_ranges[pos];
     
     below.length=index-below.location;
     _ranges[pos].length=NSMaxRange(_ranges[pos])-index;
     _ranges[pos].location=index;
     
     [self _insertRange:below position:pos];
    }
    
    // move all ranges at or above index by delta
    NSInteger count=_length;

    while(--count>=pos){
     if(_ranges[count].location>=index)
      _ranges[count].location+=delta;
    }
   }

}

-(void)encodeWithCoder:(NSCoder *)coder {
	//Structure of this method is based on NSSortDescriptor r662
	[super encodeWithCoder:coder];
	if ([coder allowsKeyedCoding]) {
		[coder encodeObject:[NSNumber numberWithInt:_capacity] forKey:@"capacity"];
	}
	else {
		[coder encodeValueOfObjCType:@encode(NSUInteger) at:&_capacity];
	}
}
	
-(id)initWithCoder:(NSCoder *)coder {
	//Structure of this method is based on NSSortDescriptor r662
	[super initWithCoder:coder];
	if ([coder allowsKeyedCoding]) {
		NSKeyedUnarchiver *keyed = (NSKeyedUnarchiver *)coder;
		_capacity = [[keyed decodeObjectForKey:@"capacity"] intValue];
	}
	else {
		[coder decodeValueOfObjCType:@encode(NSUInteger) at:&_capacity];
	}
	return self;
}

@end
