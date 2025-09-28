/* Copyright (c) 2006-2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSKVCMutableArray.h"
#import <Foundation/NSString.h>
#import <Foundation/NSKeyValueCoding.h>

@implementation NSKVCMutableArray
-(id)initWithKey:(id)theKey forProxyObject:(id)object
{
	[super init];

    proxyObject = [object retain];
	key = [theKey retain];
	id ukey=[key capitalizedString];

	insertSel = NSSelectorFromString([NSString stringWithFormat:@"insertObject:in%@AtIndex:", ukey]);
	removeSel = NSSelectorFromString([NSString stringWithFormat:@"removeObjectFrom%@AtIndex:", ukey]);
	replaceSel = NSSelectorFromString([NSString stringWithFormat:@"replaceObjectIn%@AtIndex:withObject:", ukey]);
	accessorSel = NSSelectorFromString(key);
	objectAtIndexSel = NSSelectorFromString([NSString stringWithFormat:@"objectIn%@AtIndex:", ukey]);
	setterSel = NSSelectorFromString([NSString stringWithFormat:@"set%@:", ukey]);
	countSel = NSSelectorFromString([NSString stringWithFormat:@"countOf%@", ukey]);

	if([proxyObject respondsToSelector:insertSel])
		insert=[proxyObject methodForSelector:insertSel];
	if([proxyObject respondsToSelector:replaceSel])
		replace=[proxyObject methodForSelector:replaceSel];
	if([proxyObject respondsToSelector:removeSel])
		remove=[proxyObject methodForSelector:removeSel];
	if([proxyObject respondsToSelector:accessorSel])
		accessor=[proxyObject methodForSelector:accessorSel];
	if([proxyObject respondsToSelector:setterSel])
		setter=[proxyObject methodForSelector:setterSel];
	if([proxyObject respondsToSelector:objectAtIndexSel])
		objectAtIndex=[proxyObject methodForSelector:objectAtIndexSel];
	if([proxyObject respondsToSelector:countSel])
		count=[proxyObject methodForSelector:countSel];

	return self;
}

-(id)_representedObject
{
	if(accessor)
	{
		return accessor(proxyObject, accessorSel);
	}
	return [proxyObject valueForKey:key];
}

-(void)_setRepresentedObject:(id)object
{
	if(setter)
		setter(proxyObject, setterSel, object);
	else
		[proxyObject setValue:object forKey:key];
}

-(void)dealloc
{
	[key release];
	[proxyObject release];
	[super dealloc];
}

- (NSUInteger)count;
{
	if(count)
		return (NSUInteger)count(proxyObject, countSel);
	return [[self _representedObject] count];
}

- (id)objectAtIndex:(NSUInteger)index;
{
	if(objectAtIndex)
		return objectAtIndex(proxyObject, objectAtIndexSel, index);
	return [[self _representedObject] objectAtIndex:index];
}

- (void)addObject:(id)anObject;
{
	if(insert)
		insert(proxyObject, insertSel, anObject, [self count]); 
	else
	{
		id target=[[self _representedObject] mutableCopy];
		[target addObject:anObject];
		[self _setRepresentedObject:target];
		[target release];
	}
}

- (void)insertObject:(id)anObject atIndex:(NSUInteger)index;
{
	if(insert)
	{
		insert(proxyObject, insertSel, anObject, index); 
	}
	else
	{
		id target=[[self _representedObject] mutableCopy];
		[target insertObject:anObject atIndex:index];
		[self _setRepresentedObject:target];
		[target release];
	}
}

- (void)removeLastObject;
{
	if(remove)
		remove(proxyObject, removeSel, [self count]-1); 
	else
	{
		id target=[[self _representedObject] mutableCopy];
		[target removeLastObject];
		[self _setRepresentedObject:target];
		[target release];
	}
}

- (void)removeObjectAtIndex:(NSUInteger)index;
{
	if(remove)
		remove(proxyObject, removeSel, index); 
	else
	{
		id target=[[self _representedObject] mutableCopy];
		[target removeObjectAtIndex:index];
		[self _setRepresentedObject:target];
		[target release];
	}
}

- (void)replaceObjectAtIndex:(NSUInteger)index withObject:(id)anObject;
{
	if(replace)
		replace(proxyObject, replaceSel, index, anObject);
	else
	{
		id target=[[self _representedObject] mutableCopy];
		[target replaceObjectAtIndex:index withObject:anObject];
		[self _setRepresentedObject:target];
		[target release];
	}
}

@end
