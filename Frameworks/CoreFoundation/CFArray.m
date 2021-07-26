#import <CoreFoundation/CFArray.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSCFTypeID.h>
#import <Foundation/NSArray_concrete.h>
#import <Foundation/NSRaiseException.h>

static const void *defaultRetain(CFAllocatorRef allocator,const void *value) {
   return value;
}

static void defaultRelease(CFAllocatorRef allocator,const void *value) {
}

static CFStringRef defaultCopyDescription(const void *value) {
   return (CFStringRef)@"**UNIMPLEMENTED CFArray defaultCopyDescription";
}

static Boolean defaultEqual(const void *value,const void *other) {
   return (value==other)?TRUE:FALSE;
}

@interface __CFArray : NSMutableArray {
   CFArrayCallBacks _callBacks;
   CFIndex _count;
   CFIndex _capacity;
   void  **_values;
}

-initWithCallBacks:(const CFArrayCallBacks *)callBacks;

@end

@implementation __CFArray : NSMutableArray

-initWithCallBacks:(const CFArrayCallBacks *)callBacks {
   _callBacks.version=(callBacks!=NULL)?callBacks->version:0;
   _callBacks.retain=(callBacks!=NULL && callBacks->retain!=NULL)?callBacks->retain:defaultRetain;
   _callBacks.release=(callBacks!=NULL && callBacks->release!=NULL)?callBacks->release:defaultRelease;
   _callBacks.copyDescription=(callBacks!=NULL && callBacks->copyDescription!=NULL)?callBacks->copyDescription:defaultCopyDescription;
   _callBacks.equal=(callBacks!=NULL && callBacks->equal!=NULL)?callBacks->equal:defaultEqual;
   _count=0;
   _capacity=4;
   _values=NSZoneMalloc(NULL,sizeof(void *)*_capacity);
   return self;
}

-(void)dealloc {
   CFIndex count=_count;

   while(--count>=0)
    _callBacks.release(NULL,_values[count]);

   NSZoneFree(NULL,_values);

   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(NSUInteger)count {
   return _count;
}

-objectAtIndex:(NSUInteger)index {
   if(index>=_count){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond count %d",index,_count);
    return nil;
   }

   return _values[index];
}

-(void)addObject:object {
   object=(id)_callBacks.retain(NULL,object);

   _count++;
   if(_count>_capacity){
    _capacity=_count*2;
    _values=NSZoneRealloc(NULL,_values,sizeof(void *)*_capacity);
   }
   _values[_count-1]=object;
}

-(void)insertObject:object atIndex:(NSUInteger)index {
   NSInteger c;

   if(index>_count){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond count %d",index,_count);
    return;
   }

   _count++;
   if(_count>_capacity){
    _capacity=_count*2;
    _values=NSZoneRealloc(NULL,_values,sizeof(void *)*_capacity);
   }

   if(_count>1)
    for(c=_count-1;c>index && c>0;c--)
     _values[c]=_values[c-1];

   _values[index]=(id)_callBacks.retain(NULL,object);
}

-(void)removeObjectAtIndex:(NSUInteger)index {
   NSUInteger i;
   id object;

   if(index>=_count){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond count %d",index,_count);
    return;
   }

   object=_values[index];
   _count--;
   for(i=index;i<_count;i++)
    _values[i]=_values[i+1];

   _callBacks.release(NULL,object);

   if(_capacity>_count*2){
    _capacity=_count;
    _values=NSZoneRealloc(NULL,_values,sizeof(void *)*_capacity);
   }
}

-(BOOL)isEqualToArray:(NSArray *)array {
	NSInteger i,count;

	if(self==array)
		return YES;

	count=[self count];
	if(count!=[array count])
		return NO;

	for(i=0;i<count;i++)
		if (!_callBacks.equal([self objectAtIndex:i], [array objectAtIndex:i]))
			return NO;

	return YES;
}

-(NSUInteger)indexOfObject:object inRange:(NSRange)range {
	NSInteger i,count=[self count];

	if(NSMaxRange(range)>count)
		NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond count %d",
						 NSStringFromRange(range),[self count]);

	for(i=range.location;i<range.length;i++)
		if (_callBacks.equal([self objectAtIndex:i], object))
			return i;

	return NSNotFound;
}

@end

static const void *cfArrayRetain(CFAllocatorRef allocator,const void *value) {
   return CFRetain(value);
}

static  void cfArrayRelease(CFAllocatorRef allocator,const void *value) {
  CFRelease(value);
}

static  CFStringRef cfArrayCopyDescription(const void *value) {
   return CFCopyDescription(value);
}

static  Boolean cfArrayEqual(const void *value,const void *other) {
   return CFEqual(value,other);
}


const CFArrayCallBacks kCFTypeArrayCallBacks={
 0,cfArrayRetain,cfArrayRelease,cfArrayCopyDescription,cfArrayEqual
};

#define nsrange(r) NSMakeRange(r.location,r.length)

CFTypeID CFArrayGetTypeID(void) {
   return kNSCFTypeArray;
}

CFArrayRef CFArrayCreate(CFAllocatorRef allocator,const void **values,CFIndex count,const CFArrayCallBacks *callbacks) {
	return (CFArrayRef)NSArray_concreteNew(NULL, (id*)values, count);
}

CFArrayRef CFArrayCreateCopy(CFAllocatorRef allocator,CFArrayRef self) {
	return (CFArrayRef)[(id) self copy];
}


CFIndex CFArrayGetCount(CFArrayRef self) {
	return [(NSArray*)self count];
}

const void *CFArrayGetValueAtIndex(CFArrayRef self,CFIndex index) {
	return [(NSArray*)self objectAtIndex:index];
}

void CFArrayGetValues(CFArrayRef self,CFRange range,const void **values) {
	[(NSArray*)self getObjects:(id*)values range:nsrange(range)];
}

Boolean CFArrayContainsValue(CFArrayRef self,CFRange range,const void *value) {
	NSRange inrange = NSMakeRange(range.location, range.length);
	return [(NSArray*)self indexOfObject:(id)value inRange:inrange] != NSNotFound;
}


CFIndex CFArrayGetFirstIndexOfValue(CFArrayRef self, CFRange range, const void *value)
{
	int i;
	for (i = range.location; i < range.location + range.length; i++) {
		if ([[(NSArray*)self objectAtIndex:i] isEqual:(id)value]) {
			return i;
        }
	}
	return NSNotFound;
}


CFIndex CFArrayGetLastIndexOfValue(CFArrayRef self,CFRange range,const void *value) {
    // backwards search
	NSInteger i=range.location+range.length;
    NSInteger location=range.location;

	while(--i>=location) {
		if([[(NSArray*)self objectAtIndex:i]isEqual:(id)value])
            return i;
	}
    // doc.s say -1
	return -1;
}

CFIndex CFArrayGetCountOfValue(CFArrayRef self,CFRange range,const void *value) {
	int i;
	int count=0;
	for(i=range.location+range.length;i>range.location;i--)
	{
		if([[(NSArray*)self objectAtIndex:i]isEqual:(id)value])
         count++;
	}
	return count;

}

void CFArrayApplyFunction(CFArrayRef self,CFRange range,CFArrayApplierFunction function,void *context) {
	int i;
	for(i=range.location+range.length;i>range.location;i--)
	{
		if([(NSArray*)self objectAtIndex:i]) function(self,context);
	}

}

CFIndex CFArrayBSearchValues(CFArrayRef self,CFRange range,const void *value,CFComparatorFunction function,void *context) {
   NSUnimplementedFunction();
   return 0;
}


// mutable
CFMutableArrayRef CFArrayCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFArrayCallBacks *callBacks)
{
    return (CFMutableArrayRef)[[__CFArray allocWithZone:NULL] initWithCallBacks:callBacks];
}


CFMutableArrayRef CFArrayCreateMutableCopy(CFAllocatorRef allocator,CFIndex capacity,CFArrayRef self) {
			return (CFMutableArrayRef)[(id)self mutableCopy];
}


void CFArrayAppendValue(CFMutableArrayRef self,const void *value) {
	return [(NSMutableArray*)self addObject:(id) value];
}

void CFArrayAppendArray(CFMutableArrayRef self,CFArrayRef other,CFRange range) {
	[(NSMutableArray*)self addObjectsFromArray:[(NSMutableArray*)other subarrayWithRange:nsrange(range)]];
}

void CFArrayRemoveValueAtIndex(CFMutableArrayRef self,CFIndex index) {
	[(NSMutableArray*)self removeObjectAtIndex:index];
}

void CFArrayRemoveAllValues(CFMutableArrayRef self) {
	[(NSMutableArray*)self removeAllObjects];
}

void CFArrayInsertValueAtIndex(CFMutableArrayRef self,CFIndex index,const void *value) {
	[(NSMutableArray*)self insertObject:(id)value atIndex:index];
}

void CFArraySetValueAtIndex(CFMutableArrayRef self,CFIndex index,const void *value) {
	[(NSMutableArray*)self replaceObjectAtIndex:index withObject:(id) value];
}

void CFArrayReplaceValues(CFMutableArrayRef self,CFRange range,const void **values,CFIndex count) {
   NSUnimplementedFunction();
}

void CFArrayExchangeValuesAtIndices(CFMutableArrayRef self,CFIndex index,CFIndex other) {
	[(NSMutableArray*)self exchangeObjectAtIndex:index withObjectAtIndex:other];
}

void CFArraySortValues(CFMutableArrayRef self,CFRange range,CFComparatorFunction function,void *context) {
   NSUnimplementedFunction();
}



