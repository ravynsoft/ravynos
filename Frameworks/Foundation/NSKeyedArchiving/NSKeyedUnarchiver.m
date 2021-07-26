/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#include <math.h>

#import <Foundation/NSKeyedUnarchiver.h>
#import <Foundation/NSPropertyListReader.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSData.h>
#import <Foundation/NSException.h>
#import <Foundation/CFUID.h>
#import <Foundation/NSCFTypeID.h>

#import <Foundation/NSRaise.h>

NSString* NSInvalidUnarchiveOperationException=@"NSInvalidUnarchiveOperationException";

@interface NSObject(NSKeyedUnarchiverPrivate)
+(id)allocWithKeyedUnarchiver:(NSKeyedUnarchiver *)keyed;
@end

@implementation NSObject(NSKeyedUnarchiverPrivate)

+(id)allocWithKeyedUnarchiver:(NSKeyedUnarchiver *)keyed {
   return [self allocWithZone:NULL];
}

@end

@implementation NSKeyedUnarchiver

-initForReadingWithData:(NSData *)data {
   _nameToReplacementClass=[NSMutableDictionary new];
   _propertyList=[[NSPropertyListReader propertyListFromData:data] retain];
   _objects=[[_propertyList objectForKey:@"$objects"] retain];
   _plistStack=[NSMutableArray new];
   [_plistStack addObject:[_propertyList objectForKey:@"$top"]];
   _uidToObject=NSCreateMapTable(NSIntMapKeyCallBacks,NSNonOwnedPointerMapValueCallBacks,0);
   _objectToUid=NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,NSIntMapValueCallBacks,0);
   _classVersions=NSCreateMapTable(NSObjectMapKeyCallBacks,NSIntMapValueCallBacks,0);
   return self;
}

-(void)dealloc {
   [_nameToReplacementClass release];
   [_propertyList release];
   [_objects release];
   [_plistStack release];
   if(_uidToObject!=NULL)
    NSFreeMapTable(_uidToObject);
   if(_objectToUid!=NULL)
    NSFreeMapTable(_objectToUid);
   if(_classVersions!=NULL)
    NSFreeMapTable(_classVersions);
   [super dealloc];
}

-(BOOL)allowsKeyedCoding {
   return YES;
}

static inline int integerFromCFUID(id object){
// We can deal with CFUID's and dictionaries
   unsigned typeID=[object _cfTypeID];

   if(typeID==kNSCFTypeDictionary){
    NSNumber *uid=[object objectForKey:@"CF$UID"];
    return [uid integerValue];
   }

   return [object integerValue];
}

-(Class)decodeClassFromDictionary:(NSDictionary *)classReference {
   Class         result;
   NSDictionary *plist=[classReference objectForKey:@"$class"];
   NSDictionary *profile=[_objects objectAtIndex:integerFromCFUID(plist)];
   //unused
   NSDictionary *classes=[profile objectForKey:@"$classes"];
   NSString     *className=[profile objectForKey:@"$classname"];

   // TODO: decode class version

   if((result=[_nameToReplacementClass objectForKey:className])==Nil)
    if((result=NSClassFromString(className))==Nil)
     [NSException raise:NSInvalidArgumentException format:@"Unable to find class named %@",className];

   return result;
}

-decodeObjectForUID:(NSInteger )uidIntValue {
   id result=NSMapGet(_uidToObject,(void *)uidIntValue);

   if(result==nil){
    id plist=[_objects objectAtIndex:uidIntValue];
    unsigned typeID=[plist _cfTypeID];

    if(typeID==kNSCFTypeString){
     if([plist isEqualToString:@"$null"])
      result=nil;
     else {
      result=plist;
      NSMapInsert(_uidToObject,(void *)uidIntValue,result);
      NSMapInsert(_objectToUid,result,(void *)uidIntValue);
     }
    }
    else if(typeID==kNSCFTypeDictionary){
     Class class=[self decodeClassFromDictionary:plist];

     [_plistStack addObject:plist];
     result=[class allocWithKeyedUnarchiver:self];
     NSMapInsert(_uidToObject,(void *)uidIntValue,result);
     NSMapInsert(_objectToUid,result,(void *)uidIntValue);
     result=[result initWithCoder:self];
     NSMapInsert(_uidToObject,(void *)uidIntValue,result);
     NSMapInsert(_objectToUid,result,(void *)uidIntValue);
     result=[result awakeAfterUsingCoder:self];
     [result autorelease];
     if([_delegate respondsToSelector:@selector(unarchiver:didDecodeObject:)])
      result=[_delegate unarchiver:self didDecodeObject:result];
     NSMapInsert(_uidToObject,(void *)uidIntValue,result);
     NSMapInsert(_objectToUid,result,(void *)uidIntValue);
     [_plistStack removeLastObject];
    }
    else if(typeID==kNSCFTypeNumber || typeID==kNSCFTypeBoolean){
     result=plist;
     NSMapInsert(_uidToObject,(void *)uidIntValue,result);
     NSMapInsert(_objectToUid,result,(void *)uidIntValue);
    }
    else if (typeID==kNSCFTypeData) {
     result=plist;
     NSMapInsert(_uidToObject,(void *)uidIntValue,result);
     NSMapInsert(_objectToUid,result,(void *)uidIntValue);
    }
    else
     NSLog(@"plist of class %@",[plist class]);
   }

   return result;
}

-decodeRootObject {
   NSDictionary *top=[_propertyList objectForKey:@"$top"];
   NSArray      *values=[top allValues];

   if([values count]!=1){
    NSLog(@"multiple values=%@",values);
    return nil;
   }
   else {
    NSDictionary *object=[values objectAtIndex:0];

    return [self decodeObjectForUID:integerFromCFUID(object)];
   }
}

+unarchiveObjectWithData:(NSData *)data {
   NSKeyedUnarchiver *unarchiver=[[[self alloc] initForReadingWithData:data] autorelease];

   return [unarchiver decodeRootObject];
}

+unarchiveObjectWithFile:(NSString *)path {
   NSData *data=[NSData dataWithContentsOfFile:path];

   return [self unarchiveObjectWithData:data];
}

-(BOOL)containsValueForKey:(NSString *)key {
   return ([[_plistStack lastObject] objectForKey:key]!=nil)?YES:NO;
}

-(const uint8_t *)decodeBytesForKey:(NSString *)key returnedLength:(NSUInteger *)lengthp {
   NSData *data=[[_plistStack lastObject] objectForKey:key];

   *lengthp=[data length];

   return [data bytes];
}

static inline NSNumber *_numberForKey(NSKeyedUnarchiver *self,NSString *key){
   NSNumber *result=[[self->_plistStack lastObject] objectForKey:key];

   if(result==nil)
    return result;

   unsigned typeID=[result _cfTypeID];

   if(typeID==kNSCFTypeNumber || typeID==kNSCFTypeBoolean)
    return result;

   [NSException raise:@"NSKeyedUnarchiverException" format:@"Expecting number, got %@, for key=%@",result,key];
   return [NSNumber numberWithInt:0];
}

-(BOOL)decodeBoolForKey:(NSString *)key {
   NSNumber *number=_numberForKey(self,key);

   if(number==nil)
    return NO;

   return [number boolValue];
}

-(char)decodeCharForKey:(NSString *)key {
    NSNumber *number=_numberForKey(self,key);

    if(number==nil)
        return NO;

    return [number charValue];
}
-(unsigned char)decodeUnsignedCharForKey:(NSString *)key {
    NSNumber *number=_numberForKey(self,key);

    if(number==nil)
        return NO;

    return [number unsignedCharValue];
}

-(double)decodeDoubleForKey:(NSString *)key {
   NSNumber *number=_numberForKey(self,key);

   if(number==nil)
    return 0;

   return [number doubleValue];
}

-(float)decodeFloatForKey:(NSString *)key {
   NSNumber *number=_numberForKey(self,key);

   if(number==nil)
    return 0;

   return [number floatValue];
}

-(int)decodeIntForKey:(NSString *)key {
   NSNumber *number=_numberForKey(self,key);

   if(number==nil)
    return 0;
    
    return [number intValue];
}

-(NSInteger)decodeIntegerForKey:(NSString *)key {
	NSNumber *number=_numberForKey(self,key);
    
	if(number==nil)
		return 0;
    
    return [number integerValue];
}

-(short)decodeShortForKey:(NSString *)key {
    NSNumber *number=_numberForKey(self,key);

    if(number==nil)
        return 0;

    return [number shortValue];
}

-(int32_t)decodeInt32ForKey:(NSString *)key {
   NSNumber *number=_numberForKey(self,key);

   if(number==nil)
    return 0;

   return [number intValue];
}

-(int64_t)decodeInt64ForKey:(NSString *)key {
   NSNumber *number=_numberForKey(self,key);

   if(number==nil)
    return 0;

   return [number intValue];
}

// not a lot of validation
-(NSUInteger)decodeArrayOfFloats:(float *)result forKey:(NSString *)key {
   NSString *string=[self decodeObjectForKey:key];
   NSUInteger i,length=[string length],resultLength=0;
   unichar  buffer[length];
   float   multiplier=0.10f,sign=1,exponent=0,expsign=1;
   enum {
    expectingBraceOrSpace,
    expectingBraceSpaceOrInteger,
    expectingSpaceOrInteger,
    expectingInteger,
    expectingFraction,
    expectingExponent,
    expectingCommaBraceOrSpace,
    expectingSpace
   } state=expectingBraceOrSpace;

   if(string==nil)
    return NSNotFound;

   [string getCharacters:buffer];

   for(i=0;i<length;i++){
    unichar code=buffer[i];

    switch(state){

     case expectingBraceOrSpace:
      if(code=='{')
       state=expectingBraceSpaceOrInteger;
      else if(code>' ')
       [NSException raise:NSInvalidArgumentException format:@"Unable to parse geometry %@, state=%d, pos=%d, code=%C",string,state,i,code];
      break;

     case expectingBraceSpaceOrInteger:
      if(code=='{'){
       state=expectingSpaceOrInteger;
       break;
      }
      // fallthru
     case expectingSpaceOrInteger:
      if(code<=' ')
       break;
      // fallthru
     case expectingInteger:
      if(code=='-')
       sign=-1;
      else if(code>='0' && code<='9')
       result[resultLength]=result[resultLength]*10+(code-'0');
      else if(code=='.'){
       multiplier=0.10f;
       state=expectingFraction;
      }
      else if(code=='e' || code=='E'){
       state=expectingExponent;
       exponent=0;
      }
      else if(code==','){
       result[resultLength++]*=sign;
       sign=1;
       state=expectingSpaceOrInteger;
      }
      else if(code=='}'){
       result[resultLength++]*=sign;
       sign=1;
       state=expectingCommaBraceOrSpace;
      }
      else if(code<=' '){
       result[resultLength++]*=sign;
       sign=1;
       state=expectingCommaBraceOrSpace;
      }
      else
       [NSException raise:NSInvalidArgumentException format:@"Unable to parse geometry %@, state=%d, pos=%d, code=%C",string,state,i,code];
      break;

     case expectingFraction:
      if(code>='0' && code<='9'){
       result[resultLength]=result[resultLength]+multiplier*(code-'0');
       multiplier/=10;
      }
      else if(code=='e' || code=='E'){
       state=expectingExponent;
       exponent=0;
      }
      else if(code==','){
       result[resultLength++]*=sign;
       sign=1;
       state=expectingSpaceOrInteger;
      }
      else if(code=='}'){
       result[resultLength++]*=sign;
       sign=1;
       state=expectingCommaBraceOrSpace;
      }
      else
       [NSException raise:NSInvalidArgumentException format:@"Unable to parse geometry %@, state=%d, pos=%d, code=%C",string,state,i,code];
      break;

     case expectingExponent:
      if(code=='+')
       break;
      if(code=='-')
       expsign=-1;
      else if(code>='0' && code<='9')
       exponent=exponent*10+(code-'0');
      else if(code==','){
       result[resultLength++]*=sign*powf(10.0f,expsign*exponent);
       sign=expsign=1;
       exponent=0;
       state=expectingSpaceOrInteger;
      }
      else if(code=='}'){
       result[resultLength++]*=sign*powf(10.0f,expsign*exponent);
       sign=expsign=1;
       exponent=0;
       state=expectingCommaBraceOrSpace;
      }
      else
       [NSException raise:NSInvalidArgumentException format:@"Unable to parse geometry %@, state=%d, pos=%d, code=%C",string,state,i,code];
      break;

     case expectingCommaBraceOrSpace:
      if(code==',')
       state=expectingBraceSpaceOrInteger;
      else if(code=='}')
       state=expectingSpace;
      else if(code>=' ')
       [NSException raise:NSInvalidArgumentException format:@"Unable to parse geometry %@, state=%d, pos=%d, code=%C",string,state,i,code];
      break;

     case expectingSpace:
      if(code>=' ')
       [NSException raise:NSInvalidArgumentException format:@"Unable to parse geometry %@, state=%d, pos=%d, code=%C",string,state,i,code];
      break;
    }
   }

   return resultLength;
}

-(NSPoint)decodePointForKey:(NSString *)key {
   float    array[4]={ 0,0,0,0 };
   [self decodeArrayOfFloats:array forKey:key];

   return NSMakePoint(array[0],array[1]);
}

-(NSSize)decodeSizeForKey:(NSString *)key {
   float     array[4]={ 0,0,0,0 };
   [self decodeArrayOfFloats:array forKey:key];

   return NSMakeSize(array[0],array[1]);
}

-(NSRect)decodeRectForKey:(NSString *)key {
   float    array[4]={ 0,0,0,0 };
   [self decodeArrayOfFloats:array forKey:key];

   return NSMakeRect(array[0],array[1],array[2],array[3]);
}

// NSCoder: Subclasses must override this method and provide an implementation to decode the value.
// keys of "unnamed" values seem to be incremented indexes prefixed with "$" ($0,$1,$2...)
-(void)decodeValueOfObjCType:(const char *)type at:(void *)data {
    NSString *key = [NSString stringWithFormat:@"$%u", _unnamedKeyIndex++];

    switch (*type) {
        case _C_ID: //       '@'
            *(id*)data = [[self decodeObjectForKey:key] retain];
            break;
        case _C_CLASS: //    '#'
            *(id*)data = [[self decodeObjectForKey:key] retain];
            break;
        case _C_SEL: //      ':'
            *(SEL*)data = NSSelectorFromString([self decodeObjectForKey:key]);
            break;
        case _C_CHR: //      'c'
            *(char*)data = [self decodeCharForKey:key];
            break;
        case _C_UCHR: //     'C'
            *(unsigned char*)data = [self decodeUnsignedCharForKey:key];
            break;
        case _C_SHT: //      's'
            *(short*)data = [self decodeShortForKey:key];
            break;
        case _C_USHT: //     'S'
            *(unsigned short*)data = [self decodeShortForKey:key];
            break;
        case _C_INT: //      'i'
            *(int*)data = [self decodeIntForKey:key];
            break;
        case _C_UINT: //     'I'
            *(unsigned int*)data = [self decodeIntForKey:key];
            break;
        case _C_LNG: //      'l'
            *(long*)data = [self decodeInt32ForKey:key];
            break;
        case _C_ULNG: //     'L'
            *(unsigned long*)data = [self decodeInt32ForKey:key];
            break;
        case _C_LNG_LNG: //  'q'
            *(long long*)data = [self decodeInt64ForKey:key];
            break;
        case _C_ULNG_LNG: // 'Q'
            *(unsigned long long*)data = [self decodeInt64ForKey:key];
            break;
        case _C_FLT: //      'f'
            *(float*)data = [self decodeFloatForKey:key];
            break;
        case _C_DBL: //      'd'
            *(double*)data = [self decodeDoubleForKey:key];
            break;
/*        case _C_BFLD: //     'b'
            break;*/
        case 'B':   // _C_BOOL: //     'B'  (undefined?)
            *(BOOL*)data = [self decodeBoolForKey:key];
            break;
        case _C_VOID: //     'v'
            break;
        case _C_UNDEF: //    '?'
            break;
        case _C_PTR: //      '^'
            break;
        case _C_CHARPTR: //  '*'
            *(const char**)data = [[self decodeObjectForKey:key] cString];
            break;
/*        case _C_ATOM: //     '%'
            break;
        case _C_ARY_B: //    '['
            break;
        case _C_ARY_E: //    ']'
            break;
        case _C_UNION_B: //  '('
            break;
        case _C_UNION_E: //  ')'
            break;
        case _C_STRUCT_B: // '{'
            break;
        case _C_STRUCT_E: // '}'
            break;
        case _C_VECTOR: //   '!'
            break;
        case _C_CONST: //    'r'
            break; */
        default:
            [NSException raise:@"NSKeyedUnarchiverException" format:@"Unable to decode unnamed ObjC value with unsupported type '%s'",type];
            break;
    }

    //NSUnimplementedMethod();
}

-_decodeObjectWithPropertyList:plist {
   unsigned typeID=[plist _cfTypeID];

    int backupUnnamedKeyIndex = _unnamedKeyIndex;
    _unnamedKeyIndex = 0;

    id result = nil;

    if(typeID==kNSCFTypeString || typeID==kNSCFTypeData || typeID==kNSCFTypeNumber) {
        result = plist;
    } else if(typeID==kNSCFTypeDictionary) {
        result = [self decodeObjectForUID:integerFromCFUID(plist)];
    } else if(typeID==kNSCFTypeArray){
        result=[NSMutableArray array];
        NSInteger       i,count=[plist count];

        for(i=0;i<count;i++){
            id sibling=[plist objectAtIndex:i];

            id object = [self _decodeObjectWithPropertyList:sibling];
            if (object) {
                [result addObject: object];
            }
        }
    } else if([plist isKindOfClass:[CFUID class]]) {
        result = [self decodeObjectForUID:[plist integerValue]];
    } else {
        [NSException raise:@"NSKeyedUnarchiverException" format:@"Unable to decode property list with class %@",[plist class]];
    }

    _unnamedKeyIndex = backupUnnamedKeyIndex;

    return result;
}

-decodeObjectForKey:(NSString *)key {
   id result;

   id plist=[[_plistStack lastObject] objectForKey:key];

   if(plist==nil)
    result=nil;
   else
    result=[self _decodeObjectWithPropertyList:plist];

   return result;
}


- (void)replaceObject:object withObject:replacement
{
    NSInteger uid = (NSInteger)NSMapGet(_objectToUid, object);
    id check = NSMapGet(_uidToObject, (void *)uid);

    if (check != object) {
        NSLog(@"fail " NSIntegerFormat " %p %p", uid, check, object);
    } else {
        if ([_delegate respondsToSelector:@selector(unarchiver:willReplaceObject:withObject:)]) {
            [_delegate unarchiver:self willReplaceObject:object withObject:replacement];
        }

        NSMapInsert(_uidToObject, (void *)uid, replacement);
        NSMapInsert(_uidToObject, replacement, (void *)uid);
    }
}


-(void)finishDecoding {
}

-delegate {
   return _delegate;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

+(void)setClass:(Class)class forClassName:(NSString *)className {
}

+(Class)classForClassName:(NSString *)className {
   return Nil;
}

-(void)setClass:(Class)class forClassName:(NSString *)className {
   [_nameToReplacementClass setObject:class forKey:className];
}

-(Class)classForClassName:(NSString *)className {
   return [_nameToReplacementClass objectForKey:className];
}

-(NSInteger)versionForClassName:(NSString *)className {
   return (NSInteger)NSMapGet(_classVersions,className);
}

-(NSString*)description
{
	return [NSString stringWithFormat:@"%@ %@", [super description], [_plistStack lastObject]];
}
@end
