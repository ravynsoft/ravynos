/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSData.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSPropertyList.h>
#import <Foundation/NSString.h>


@implementation NSKeyedArchiver

static NSMapTable *_globalNameToClass=NULL;

+(void)initialize {
   if(self==[NSKeyedArchiver class]){
    _globalNameToClass=NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0);
   }
}

+(NSData *)archivedDataWithRootObject:rootObject {
    NSMutableData *data = [NSMutableData data];
    NSKeyedArchiver *archiver = [[[self class] alloc] initForWritingWithMutableData:data];
    
    [archiver encodeObject:rootObject forKey:@"root"];
    [archiver finishEncoding];
    [archiver release];
    return data;
}

+(BOOL)archiveRootObject:rootObject toFile:(NSString *)path {
   NSData *data=[self archivedDataWithRootObject:rootObject];
   
   return [data writeToFile:path atomically:YES];
}

-initForWritingWithMutableData:(NSMutableData *)data {
   _data=[data retain];
   _plistStack=[NSMutableArray new];
   [_plistStack addObject:[NSMutableDictionary dictionary]];
   
   _objects=[NSMutableArray new];
   [[_plistStack lastObject] setObject:_objects forKey:@"$objects"];
   [[_plistStack lastObject] setObject:[self className] forKey:@"$archiver"];
   [[_plistStack lastObject] setObject:[NSNumber numberWithInt:100000] forKey:@"$version"];
   
   // Cocoa puts this default object here so that CF$UID==0 acts as nil
   [_objects addObject:@"$null"];

   _top=[NSMutableDictionary dictionary];
   [[_plistStack lastObject] setObject:_top forKey:@"$top"];
   
   _nameToClass=NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0);
   _pass=0;
   
   NSMapTableKeyCallBacks objectToUidKeyCb = NSNonRetainedObjectMapKeyCallBacks;
   objectToUidKeyCb.isEqual = NULL;
   // setting the equality callback to NULL means that the maptable will use pointer comparison.
   // this is necessary to properly archive classes like NSMutableString which encodes an internal immutable
   // object that returns YES to -isEqual with the mutable parent (and thus wouldn't get encoded at all without this change)
   
   _objectToUid=NSCreateMapTable(objectToUidKeyCb,NSObjectMapValueCallBacks,0);

   // FIXME: should be binary format per Apple's docs but we don't support writing that yet
   _outputFormat=NSPropertyListXMLFormat_v1_0;
   return self;
}

-init {
   return [self initForWritingWithMutableData:[NSMutableData data]];
}

-(void)dealloc {
   [_data release];
   [_plistStack release];
   NSFreeMapTable(_nameToClass);
   NSFreeMapTable(_objectToUid);
   [super dealloc];
}

-(BOOL)allowsKeyedCoding {
   return YES;
}

+(NSString *)classNameForClass:(Class)class {
   return NSMapGet(_globalNameToClass,(void *)class);
}

+(void)setClassName:(NSString *)className forClass:(Class)class {
  NSMapInsert(_globalNameToClass,class,className);
}

-delegate {
   return _delegate;
}

-(NSString *)classNameForClass:(Class)class {
   return NSMapGet(_nameToClass,(void *)class);
}

-(NSPropertyListFormat)outputFormat {
   return _outputFormat;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

-(void)setClassName:(NSString *)className forClass:(Class)class {
  NSMapInsert(_nameToClass,class,className);
}

-(void)setOutputFormat:(NSPropertyListFormat)format {
   _outputFormat=format;
}

-(void)encodeBool:(BOOL)value forKey:(NSString *)key {
   if(_pass==0)
    return;

   [[_plistStack lastObject] setObject:[NSNumber numberWithBool:value] forKey:key];
}

-(void)encodeInt:(int)value forKey:(NSString *)key {
   if(_pass==0)
    return;

   [[_plistStack lastObject] setObject:[NSNumber numberWithInt:value] forKey:key];
}

-(void)encodeInteger:(NSInteger)value forKey:(NSString *)key {
    if(_pass==0)
        return;
    
    [[_plistStack lastObject] setObject:[NSNumber numberWithInteger:value] forKey:key];
}

-(void)encodeInt32:(int32_t)value forKey:(NSString *)key {
   if(_pass==0)
    return;

   [[_plistStack lastObject] setObject:[NSNumber numberWithInt:value] forKey:key];
}

-(void)encodeInt64:(int64_t)value forKey:(NSString *)key {
   if(_pass==0)
    return;

   [[_plistStack lastObject] setObject:[NSNumber numberWithLongLong:value] forKey:key];
}

-(void)encodeFloat:(float)value forKey:(NSString *)key {
   if(_pass==0)
    return;

   [[_plistStack lastObject] setObject:[NSNumber numberWithFloat:value] forKey:key];
}

-(void)encodeDouble:(double)value forKey:(NSString *)key {
   if(_pass==0)
    return;

   [[_plistStack lastObject] setObject:[NSNumber numberWithDouble:value] forKey:key];
}

-(void)encodeBytes:(const uint8_t *)ptr length:(NSUInteger)length forKey:(NSString *)key {
   if(_pass==0)
    return;
   
   [[_plistStack lastObject] setObject:[NSData dataWithBytes:ptr length:length] forKey:key];
}


- (void)encodeArrayOfDoubles:(double *)array count:(NSUInteger)count forKey:(NSString *)key {
   if(_pass == 0 || count < 1)
    return;
   
    NSMutableString *str = [NSMutableString stringWithString:@"{"];
    NSUInteger i;
    for (i = 0; i < count; i++) {
        [str appendFormat:@"%.12f%@", array[i], (i < count-1) ? @", " : @"}"];
    }
    
    [self encodeObject:[NSString stringWithString:str] forKey:key];
}

-(void)encodePoint:(NSPoint)value forKey:(NSString *)key {
    double array[2] = {value.x, value.y};

    [self encodeArrayOfDoubles:array count:2 forKey:key];
}

-(void)encodeRect:(NSRect)value forKey:(NSString *)key {
    double array[4] = {value.origin.x, value.origin.y, value.size.width, value.size.height};

    [self encodeArrayOfDoubles:array count:4 forKey:key];
}

-(void)encodeSize:(NSSize)value forKey:(NSString *)key {
    double array[2] = {value.width, value.height};

    [self encodeArrayOfDoubles:array count:2 forKey:key];
}



-plistForObject:object flag:(BOOL)flag {
   NSNumber *uid=NSMapGet(_objectToUid,object);
   
   if(uid==nil){
    uid=[NSNumber numberWithUnsignedInteger:[_objects count]];
    NSMapInsert(_objectToUid,object,uid);
    
    NSString *archClass = NSStringFromClass([object classForKeyedArchiver]);
    
    //NSLog(@"uid %@: encoding class %@ as '%@'", uid, [object class], archClass);
    
    if ([archClass isEqualToString:@"NSString"]) {
        [_objects addObject:[NSString stringWithString:[object description]]];
    }
    else if ([archClass isEqualToString:@"NSNumber"]) {
        [_objects addObject:object];
    }
    else if ([archClass isEqualToString:@"NSData"]) {
        [_objects addObject:object];
    }
    else if ([archClass isEqualToString:@"NSDictionary"] && flag) {
        [_objects addObject:object];
    }
    else if (object == nil || [object isKindOfClass:[NSNull class]]) {
        [_objects addObject:@"$null"];
    }
    else {
        [_objects addObject:[NSMutableDictionary dictionary]];
        [_plistStack addObject:[_objects lastObject]];
        
        [object encodeWithCoder:self];
		
	NSMutableArray *supers = [[NSMutableArray alloc] init];
	[supers addObject:archClass];
	Class sup = class_getSuperclass([object classForKeyedArchiver]);
	while( sup != nil )
	{
		[supers addObject:NSStringFromClass(sup)];
		sup = class_getSuperclass(sup);
	}
		
	NSDictionary *classMap = [NSDictionary dictionaryWithObjectsAndKeys:
				    supers, @"$classes",
                                    archClass, @"$classname",
                                    nil];
		
	[supers release];
                                    
        [[_plistStack lastObject] setObject:[self plistForObject:classMap flag:YES] forKey:@"$class"];
        [_plistStack removeLastObject];
    }
   }
   
   return [NSDictionary dictionaryWithObject:uid forKey:@"CF$UID"];
}

-(void)encodeObject:object forKey:(NSString *)key {
    if (_pass == 0) {
        [_plistStack addObject:_top];
    }

    _pass++;
   [[_plistStack lastObject] setObject:[self plistForObject:object flag:NO] forKey:key];
   _pass--;
   
    if (_pass == 0) {
        [_plistStack removeLastObject];
    }
}

-(void)encodeConditionalObject:object forKey:(NSString *)key {
   if(_pass==0)
    return;
	
    // Only encode the object if it's already somewhere
    if (NSMapGet(_objectToUid,object)) {
        [self encodeObject:object forKey:key];
    }
}


// private, only called by the -encodeWithCoder methods of NSArray and NSSet
- (void)encodeArray:(NSArray *)array forKey:(NSString *)key {
    if(_pass==0)
     return;
    
    NSInteger count = [array count];
    NSMutableArray *plistArr = [NSMutableArray arrayWithCapacity:count];
    int i;
    for (i = 0; i < count; i++) {
        id obj = [array objectAtIndex:i];
        id plist = [self plistForObject:obj flag:NO];
        [plistArr addObject:plist];
    }
    
    [[_plistStack lastObject] setObject:plistArr forKey:key];
}


-(void)finishEncoding {   
   NSData *newData = [NSPropertyListSerialization dataFromPropertyList:[_plistStack lastObject]
                                                  format:_outputFormat
                                                  errorDescription:NULL];
   
   [_data appendData:newData];
}

@end
