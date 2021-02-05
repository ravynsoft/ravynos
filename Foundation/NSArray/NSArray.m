/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSData.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSArray_placeholder.h>
#import <Foundation/NSArray_concrete.h>
#import <Foundation/NSEnumerator_array.h>
#import <Foundation/NSEnumerator_arrayReverse.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSPropertyListReader.h>
#import <Foundation/NSPropertyListWriter_vintage.h>
#import <Foundation/NSKeyedUnarchiver.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSPredicate.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSRaiseException.h>

@interface NSKeyedArchiver (PrivateToContainers)
- (void)encodeArray:(NSArray *)array forKey:(NSString *)key;
@end


@implementation NSArray 

+allocWithZone:(NSZone *)zone {
   if(self==objc_lookUpClass("NSArray"))
    return NSAllocateObject([NSArray_placeholder class],0,NULL);

   return NSAllocateObject(self,0,zone);
}

-initWithArray:(NSArray *)array {
   NSUInteger count=[array count];
   id      *objects=__builtin_alloca(sizeof(id)*count);

   [array getObjects:objects];

   return [self initWithObjects:objects count:count];
}

-initWithArray:(NSArray *)array copyItems:(BOOL)copyItems {
   if (copyItems == NO) {
      return [self initWithArray:array];
   }

   const NSUInteger count=[array count];
   id *oldObjects = __builtin_alloca(sizeof(id)*count);
   id *newObjects = __builtin_alloca(sizeof(id)*count);

   [array getObjects:oldObjects];

   NSUInteger i;
   for (i = 0; i < count; i++) {
      newObjects[i] = [oldObjects[i] copyWithZone:NULL];
   }
   
   self = [self initWithObjects:newObjects count:count];
   
   for (i = 0; i < count; i++) {
      [newObjects[i] release];
   }
   return self;
}

-initWithContentsOfFile:(NSString *)path {
    id contents = [NSPropertyListReader arrayWithContentsOfFile: path]; 

    if(contents==nil) { 
        [self dealloc]; 
        return nil; 
    } 

    return [self initWithArray: contents]; 
}

-initWithContentsOfURL:(NSURL *)url {
   if(![url isFileURL]){
    [self dealloc];
    return nil;
   }
   return [self initWithContentsOfFile:[url path]];
}

-initWithObjects:(id *)objects count:(NSUInteger)count {
   NSInvalidAbstractInvocation();
   return nil;
}


-initWithObjects:object,... {
   va_list  arguments;
   NSUInteger i,count;
   id      *objects;

   va_start(arguments,object);
   count=1;
   while(va_arg(arguments,id)!=nil)
    count++;
   va_end(arguments);

   objects=__builtin_alloca(sizeof(id)*count);

   va_start(arguments,object);
   objects[0]=object;
   for(i=1;i<count;i++)
    objects[i]=va_arg(arguments,id);
   va_end(arguments);

   return [self initWithObjects:objects count:count];
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}


-mutableCopyWithZone:(NSZone *)zone {
   return [[NSMutableArray allocWithZone:zone] initWithArray:self];
}


-(Class)classForCoder {
   return objc_lookUpClass("NSArray");
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    NSArray           *array=[keyed decodeObjectForKey:@"NS.objects"];
    
    return [self initWithArray:array];
   }
   else {
    unsigned i,count;
    id      *objects;

    [coder decodeValueOfObjCType:@encode(int) at:&count];

    objects=__builtin_alloca(count*sizeof(id));

    for(i=0;i<count;i++)
     objects[i]=[coder decodeObject];

    return [self initWithObjects:objects count:count];
   }
}


-(void)encodeWithCoder:(NSCoder *)coder {
  if([coder isKindOfClass:[NSKeyedArchiver class]]){
    NSKeyedArchiver *keyed=(NSKeyedArchiver *)coder;
    
    [keyed encodeArray:self forKey:@"NS.objects"];
  }
  else {
    int i,count=[self count];

    [coder encodeValueOfObjCType:@encode(int) at:&count];
    for(i=0;i<count;i++)
     [coder encodeObject:[self objectAtIndex:i]];
  }
}

+array {
   return [[[self allocWithZone:NULL] init] autorelease];
}


+arrayWithContentsOfFile:(NSString *)path {
   return [[[self allocWithZone:NULL] initWithContentsOfFile:path] autorelease];
}

+arrayWithContentsOfURL:(NSURL *)url {
   return [[[self allocWithZone:NULL] initWithContentsOfURL:url] autorelease];
}

+arrayWithObject:object {
   return [[[self allocWithZone:NULL]
      initWithObjects:&object count:1] autorelease];
}


+arrayWithObjects:object,... {
   NSUInteger i,count=0; 
   id        *objects=NULL;
   
   if(object!=nil){
    va_list  arguments;

    va_start(arguments,object);
    count=1; // include object
    while(va_arg(arguments,id)!=nil)
     count++;
    va_end(arguments);

    objects=__builtin_alloca(sizeof(id)*count);

    va_start(arguments,object);
    objects[0]=object;
    for(i=1;i<count;i++)
     objects[i]=va_arg(arguments,id);
    va_end(arguments);
   }
   
   return [[[self allocWithZone:NULL] initWithObjects:objects count:count] autorelease];
}


+arrayWithArray:(NSArray *)array {
   return [[[self allocWithZone:NULL] initWithArray:array] autorelease];
}


+arrayWithObjects:(id *)objects count:(NSUInteger)count {
   return [[[self allocWithZone:NULL] initWithObjects:objects count:count] autorelease];
}


-(NSUInteger)count {
   NSInvalidAbstractInvocation();
   return 0;
}


-objectAtIndex:(NSUInteger)index {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)getObjects:(id *)objects {
   NSUInteger i,count=[self count];

   for(i=0;i<count;i++)
    objects[i]=[self objectAtIndex:i];
}


-(void)getObjects:(id *)objects range:(NSRange)range {
   NSUInteger i,count=[self count],loc=range.location;

   if(NSMaxRange(range)>count)
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond count %d",
     NSStringFromRange(range),[self count]);

   for(i=0;i<range.length;i++)
    objects[i]=[self objectAtIndex:loc+i];
}

-(NSArray *)subarrayWithRange:(NSRange)range {
   if(NSMaxRange(range)>[self count])
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond count %d",
     NSStringFromRange(range),[self count]);

   return NSAutorelease(NSArray_concreteWithArrayRange(self,range));
}

-(NSUInteger)hash {
   return [self count];
}

-(BOOL)isEqual:array {
   if(self==array)
    return YES;

   if(![array isKindOfClass:objc_lookUpClass("NSArray")])
    return NO;

   return [self isEqualToArray:array];
}


-(BOOL)isEqualToArray:(NSArray *)array {
   NSInteger i,count;

   if(self==array)
    return YES;

   count=[self count];
   if(count!=[array count])
    return NO;

   for(i=0;i<count;i++)
    if(![[self objectAtIndex:i] isEqual:[array objectAtIndex:i]])
     return NO;

   return YES;
}

-(NSUInteger)indexOfObject:object {
   NSInteger i,count=[self count];

   for(i=0;i<count;i++)
    if([[self objectAtIndex:i] isEqual:object])
     return i;

   return NSNotFound;
}


-(NSUInteger)indexOfObject:object inRange:(NSRange)range {
	NSInteger i,count=[self count],loc=range.location;
    
    if(NSMaxRange(range)>count)
        NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond count %d",NSStringFromRange(range),[self count]);
    
    for(i=0;i<range.length;i++)
        if([[self objectAtIndex:loc+i] isEqual:object])
            return loc+i;
    
    return NSNotFound;
}


-(NSUInteger)indexOfObjectIdenticalTo:object {
   NSInteger i,count=[self count];

   for(i=0;i<count;i++)
    if([self objectAtIndex:i]==object)
     return i;

   return NSNotFound;
}


-(NSUInteger)indexOfObjectIdenticalTo:object inRange:(NSRange)range {
    NSInteger i,count=[self count],loc=range.location;
    
    if(NSMaxRange(range)>count)
        NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond count %d",NSStringFromRange(range),[self count]);
    
    for(i=0;i<range.length;i++)
        if([self objectAtIndex:loc+i]==object)
            return loc+i;
    
    return NSNotFound;
}

-(NSEnumerator *)objectEnumerator {
   return NSAutorelease(NSEnumerator_arrayNew(self));
}


-(NSEnumerator *)reverseObjectEnumerator {
   return NSAutorelease(NSEnumerator_arrayReverseNew(self));
}


-(NSArray *)arrayByAddingObject:object {
   return NSAutorelease(NSArray_concreteWithArrayAndObject(self,object));
}


-(NSArray *)arrayByAddingObjectsFromArray:(NSArray *)array {
   return NSAutorelease(NSArray_concreteWithArrayAndArray(self,array));
}

-(NSString *)componentsJoinedByString:(NSString *)separator {
   NSMutableString *string=[NSMutableString stringWithCapacity:256];
   NSInteger i,count=[self count];

   for(i=0;i<count;i++){
    [string appendString:[[self objectAtIndex:i] description]];
    if(i+1<count)
     [string appendString:separator];
   }
   return string;
}


-(BOOL)containsObject:object {
   return ([self indexOfObject:object]!=NSNotFound)?YES:NO;
}


-firstObjectCommonWithArray:(NSArray *)array {
   if ([array count]) { 
    NSInteger i,count=[self count];

    for(i=0;i<count;i++){
     id object=[self objectAtIndex:i];

     if([array indexOfObject:object]!=NSNotFound)
      return object;
    }
   }
   return nil;
}

-lastObject {
   NSInteger count=[self count];

   if(count==0)
    return nil;

   return [self objectAtIndex:count-1];
}

-firstObject {
    NSInteger count=[self count];
    
    if(count==0)
        return nil;
    
    return [self objectAtIndex:0];
}

-(NSArray *)sortedArrayUsingSelector:(SEL)selector {
   NSMutableArray *array=[NSMutableArray arrayWithArray:self];

   [array sortUsingSelector:selector];

   return array;
}

-(NSArray *)sortedArrayUsingFunction:(NSInteger (*)(id, id, void *))function
   context:(void *)context {
   NSMutableArray *array=[NSMutableArray arrayWithArray:self];

   [array sortUsingFunction:function context:context];

   return array;
}

-(BOOL)writeToFile:(NSString *)path atomically:(BOOL)atomically {
   return [NSPropertyListWriter_vintage writePropertyList:self toFile:path atomically:atomically];
}

-(BOOL)writeToURL:(NSURL *)aURL atomically:(BOOL)atomically {
	if ([aURL isFileURL]) {
		return [self writeToFile:[aURL path] atomically:atomically];
	}
	else {
		return NO;
	}
}


-(void)makeObjectsPerformSelector:(SEL)selector {
	NSInteger i, count = [self count];
	
	for (i = 0; i < count; i++)
		[[self objectAtIndex:i] performSelector:selector];
}


-(void)makeObjectsPerformSelector:(SEL)selector withObject:object {
	NSInteger i, count = [self count];
	
	for (i = 0; i < count; i++)
		[[self objectAtIndex:i] performSelector:selector withObject:object];
}

-(NSString *)description {
   return [NSPropertyListWriter_vintage stringWithPropertyList:self];
}

-(NSString *)descriptionWithLocale:(NSDictionary *)locale {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)descriptionWithLocale:(NSDictionary *)locale indent:(NSUInteger)indent {
   NSUnimplementedMethod();
   return nil;
}

-(NSArray *)filteredArrayUsingPredicate:(NSPredicate *)predicate {
   NSInteger             i,count=[self count];
   NSMutableArray *result=[NSMutableArray arrayWithCapacity:count];
   
   for(i=0;i<count;i++){
    id check=[self objectAtIndex:i];
    
    if([predicate evaluateWithObject:check])
     [result addObject:check];
   }
    
   return result;
}

-(NSArray *)sortedArrayUsingDescriptors:(NSArray *)descriptors {
   NSMutableArray *result=[NSMutableArray arrayWithArray:self];
   
   [result sortUsingDescriptors:descriptors];
   
   return result;
}

#if 0
// untested
-(NSArray *)objectsAtIndexes:(NSIndexSet *)indexes {
   NSUInteger i,count=[indexes count];
   NSUInteger buffer[count];
   id       objects[count];

   count=[indexes getIndexes:buffer maxCount:count inIndexRange:NULL];
//  getObjects:range: would make more sense
   for(i=0;i<count;i++)
    objects[i]=[self objectAtIndex:buffer[i]];
   
   return [NSArray arrayWithObjects:objects count:count];
}
#endif

-(NSArray *)objectsAtIndexes:(NSIndexSet*)indexes
{
	NSUInteger idx=[indexes firstIndex];
	id ret=[NSMutableArray array];
	while(idx!=NSNotFound)
	{
		[ret addObject:[self objectAtIndex:idx]];
		idx=[indexes indexGreaterThanIndex:idx];
	}
	return ret;
}

-(NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id *)stackbuf count:(NSUInteger)length;
{
	NSInteger numObjects=MIN([self count] - state->extra[0], length);
	NSInteger i=state->extra[0];
	NSInteger j=0;
	state->itemsPtr=stackbuf;

	for(j=0; j<numObjects; j++, i++)
	{
		state->itemsPtr[j]=[self objectAtIndex:i];
	}

	state->extra[0]+=numObjects;
	
	state->mutationsPtr=(unsigned long *)self;

	return numObjects;
}

@end

void objc_enumerationMutation(id collection)
{
	[NSException raise:NSInternalInconsistencyException format:@"Collection %p of class %@ was mutated during enumeration. Break on objc_enumerationMutation to debug.", collection, [collection className]];
}

#import <Foundation/NSCFTypeID.h>

@implementation NSArray (CFTypeID)

- (unsigned) _cfTypeID
{
   return kNSCFTypeArray;
}

@end
