/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/Foundation.h>
#import "NSString+KVCAdditions.h"
#import <Foundation/NSRaiseException.h>

@implementation NSArray (NSKeyValueCoding)
-(id)valueForKey:(NSString*)key
{
	if([key hasPrefix:@"@"]) // operator
	{
		/*
		"If key indicates an operation that doesn't require an argument,
		valueForKey performs the operation and returns the result. key indicates
		an operation if its first character is "@". For example, if key is
		"@count", valueForKey invokes compute on the "count" operator. This has
		the effect of computing and returning the number of elements in the
		receiver.
		Don't use valueForKey for operations that take arguments; instead use
		valueForKeyPath." - from the standard
		*/
		if([key rangeOfString:@"."].location!=NSNotFound)
			[NSException raise:@"NSKeyValueCodingException"
						format:@"valueForKey called for operator with parameters %@. Use valueForKeyPath: instead", key];

		id operator=[key substringFromIndex:1];
		// find operator selector (e.g. _kvo_operator_count for @count)
		SEL operatorSelector=NSSelectorFromString([NSString stringWithFormat:@"_kvo_operator_%@", operator]);
		if(![self respondsToSelector:operatorSelector])
			[NSException raise:@"NSKeyValueCodingException"
						format:@"operator %@: NSArray selector %@ not implemented", operator, NSStringFromSelector(operatorSelector)];

		return [self performSelector:operatorSelector];
	}
	/*
	 For keys which do not begin with "@", valueForKey creates a new array with
	 the same number of elements as this array. For each element, the
	 corresponding element in the new array is the result of invoking
	 valueForKeyPath with key as the key path on the element. For example, if
	 key is "firstName", this method returns an array containing the firstName
	 values for each of the array's elements. The key argument can be a key path
	 of the form relationship.property. For example, "department.name".
	 valueForKey replaces null values with an instance of NSKeyValueCoding.Null
	 */
	NSMutableArray *array=[NSMutableArray array];
	id en=[self objectEnumerator];
	id obj;
	while((obj=[en nextObject]))
	{
		id val=[obj valueForKey:key];
		if(!val){
			val=[NSNull null];
        }
		[array addObject:val];
	}
	return array;
}

-(id)valueForKeyPath:(NSString*)keyPath
{
	if([keyPath hasPrefix:@"@"]) // operator
	{
		/*
		 If keyPath indicates an operation takes an argument (such as computing
		 an average), valueForKeyPath performs the operation and returns the
		 result. key indicates an aggregate operation if its first character
		 is "@". For example, if key is "@avg.salary", valueForKey invokes
		 compute on the "avg" operator specifying the array and "salary" as
		 arguments. This has the effect of computing and returning the average
		 salary of the array's elements.
		*/
		NSString *operator, *parameter;
		[[keyPath substringFromIndex:1] _KVC_partBeforeDot:&operator afterDot:&parameter];

		// find operator selector (e.g. _kvo_operator_avg: for @avg)
		SEL operatorSelector=NSSelectorFromString([NSString stringWithFormat:@"_kvo_operator_%@:", operator]);
		if(![self respondsToSelector:operatorSelector])
			[NSException raise:@"NSKeyValueCodingException"
						format:@"operator %@: NSArray selector %@ not implemented (parameter was %@)", operator, NSStringFromSelector(operatorSelector), parameter];

		return [self performSelector:operatorSelector withObject:parameter];
	}

	/*
	 Otherwise, valueForKeyPath behaves similarly to valueForKey and produces a
	 new NSArray whose elements correspond to the results of invoking
	 valueForKeyPath on each element of this array.
	 */
    NSMutableArray *array=[NSMutableArray array];

	id en=[self objectEnumerator];
	id obj;
	while((obj=[en nextObject]))
	{
		id val=[obj valueForKeyPath:keyPath];
		if(val==nil)
			val=[NSNull null];

		[array addObject:val];
	}

	return array;
}

-(id)_kvo_operator_avg:(NSString*)parameter
{
	NSArray* objects=[self valueForKeyPath:parameter];
	NSInteger count=[objects count];
	NSInteger i;
	double average=0;
	for(i=0; i<count; i++)
	{
		average+=[[objects objectAtIndex:i] doubleValue] / (double)count;
	}
	return [NSNumber numberWithDouble:average];
}

-(id)_kvo_operator_max:(NSString*)parameter
{
	NSArray* objects=[self valueForKeyPath:parameter];
	NSInteger count=[objects count];
	NSInteger i;

	id currentMaximum=[objects lastObject];
	for(i=0; i<count; i++)
	{
		id obj=[objects objectAtIndex:i];
		if([(NSString */* to avoid warnings */)currentMaximum compare:obj]<0)
			currentMaximum=obj;
	}
	return currentMaximum;
}

-(id)_kvo_operator_min:(NSString*)parameter
{
	NSArray* objects=[self valueForKeyPath:parameter];
	NSInteger count=[objects count];
	NSInteger i;

	id currentMinimum=[objects lastObject];
	for(i=0; i<count; i++)
	{
		id obj=[objects objectAtIndex:i];
		if([(NSString */* to avoid warnings */)currentMinimum compare:obj]>0)
			currentMinimum=obj;
	}
	return currentMinimum;
}

-(id)_kvo_operator_count
{
	return [NSNumber numberWithUnsignedInteger:[self count]];
}

-(id)_kvo_operator_count:(NSString*)parameter
{
	if([parameter length]>0)
		[NSException raise:NSInvalidArgumentException
					format:@"array operator @count called with argument (%@)", parameter];
	return [self _kvo_operator_count];
}

-(id)_kvo_operator_sum:(NSString*)parameter
{
	NSArray* objects=[self valueForKeyPath:parameter];
	NSInteger count=[objects count];
	NSInteger i;

	double sum=0.0;
	for(i=0; i<count; i++)
	{
		id obj=[objects objectAtIndex:i];
		sum+=[obj doubleValue];
	}
	return [NSNumber numberWithDouble:sum];
}


-(void)setValue:(id)value forKey:(NSString*)key
{
	id en=[self objectEnumerator];
	id obj;
	while((obj=[en nextObject]))
	{
		[obj setValue:value forKey:key];
	}
}

-(void)setValue:(id)value forKeyPath:(NSString*)keyPath
{
	id en=[self objectEnumerator];
	id obj;
	while((obj=[en nextObject]))
	{
		[obj setValue:value forKeyPath:keyPath];
	}
}
@end


@implementation NSArray (KVO)

- (void)addObserver:(NSObject *)observer toObjectsAtIndexes:(NSIndexSet *)indexes forKeyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(void *)context
{
	NSUInteger idx=[indexes firstIndex];
	while(idx!=NSNotFound)
	{
		[[self objectAtIndex:idx] addObserver:observer
								   forKeyPath:keyPath
									  options:options
									  context:context];
		idx=[indexes indexGreaterThanIndex:idx];
	}
}

- (void)removeObserver:(NSObject *)observer fromObjectsAtIndexes:(NSIndexSet *)indexes forKeyPath:(NSString *)keyPath
{
	NSUInteger idx=[indexes firstIndex];
	while(idx!=NSNotFound)
	{
		[[self objectAtIndex:idx] removeObserver:observer
									  forKeyPath:keyPath];
		idx=[indexes indexGreaterThanIndex:idx];
	}
}

-(void)addObserver:(id)observer forKeyPath:(NSString*)keyPath options:(NSKeyValueObservingOptions)options context:(void*)context;
{
	if([isa instanceMethodForSelector:_cmd]==[NSArray instanceMethodForSelector:_cmd])
		NSRaiseException(NSInvalidArgumentException,self,_cmd,@"not supported for key path %@ (observer was %@)", keyPath, observer);
	else
		[super addObserver:observer
				forKeyPath:keyPath
				   options:options
				   context:context];
}

-(void)removeObserver:(id)observer forKeyPath:(NSString*)keyPath;
{
	if([isa instanceMethodForSelector:_cmd]==[NSArray instanceMethodForSelector:_cmd])
		NSRaiseException(NSInvalidArgumentException,self,_cmd,@"not supported for key path %@ (observer was %@)", keyPath, observer);
	else
		[super removeObserver:observer
				   forKeyPath:keyPath];
}
@end
