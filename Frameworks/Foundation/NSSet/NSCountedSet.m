/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSCountedSet.h>
#import <Foundation/NSInlineSetTable.h>
#import <Foundation/NSEnumerator_set.h>
#import <Foundation/NSAutoreleasePool-private.h>

@implementation NSCountedSet : NSMutableSet

-initWithCapacity:(NSUInteger)capacity {
   _table=NSZoneMalloc([self zone],sizeof(NSSetTable));
   NSSetTableInit(_table,capacity,[self zone]);
   return self;
}

-(void)dealloc {
   NSSetTableFreeObjects(_table);
   NSSetTableFreeBuckets(_table);
   NSZoneFree(NSZoneFromPointer(_table),_table);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(NSUInteger)count {
   return ((NSSetTable *)_table)->count;
}

-member:object {
   return NSSetTableMember(_table,object);
}

-(NSEnumerator *)objectEnumerator {
   return NSAutorelease(NSEnumerator_setNew(NULL,self,_table));
}

-(void)addObject:object {
   NSSetTableAddObjectCount(_table,object);
}

-(void)removeObject:object {
   NSSetTableRemoveObjectCount(_table,object);
}

-(NSUInteger)countForObject:object {
   return NSSetTableObjectCount(_table,object);
}

-(BOOL)isEqualToSet:(NSSet *)other {
	NSEnumerator *state;
	id            object;
	
	if(self==other)
		return YES;
	
	if([self count]!=[other count])
		return NO;
	if([other isKindOfClass:[NSCountedSet class]])
	{
		state=[self objectEnumerator];
		while((object=[state nextObject])!=nil)
		{
			if([(NSCountedSet*)other countForObject:object]!=[self countForObject:object])
				return NO;
		}
		return YES;
	}
	
	// FIXME: what to do here? Can't compare counts, but can the sets still be equal?
	
	state=[self objectEnumerator];
	while((object=[state nextObject])!=nil)
		if([other member:object]==nil)
			return NO;
	
	return YES;
}
@end
