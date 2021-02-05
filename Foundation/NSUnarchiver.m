/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSUnarchiver.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSData.h>
#import <Foundation/NSMutableArray.h>
#import <Foundation/NSByteOrder.h>
#include <string.h>

@implementation NSUnarchiver

-(void)cannotDecodeType:(const char *)type {
   [NSException raise:@"NSUnarchiverCannotDecodeException"
               format:@"NSUnarchiver cannot decode type=%s",type];
}

-(void)_ensureLength:(NSUInteger)length {
   if(_position+length>_length)
    [NSException raise:@"NSUnarchiverBadArchiveException"
                format:@"NSUnarchiver attempt to read beyond length"];
}

-(uint8_t)_extractWordOne {
   [self _ensureLength:1];
   return _bytes[_position++];
}

-(uint16_t)_extractWordTwo {
   uint16_t result;

   [self _ensureLength:2];

   result=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];

   return result;
}

-(unsigned int)_extractWordFour {
   unsigned int result;

   [self _ensureLength:4];

   result=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];

   return result;
}

-(unsigned long long)_extractWordEight {
   unsigned long long result;

   [self _ensureLength:8];

   result=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];
   result<<=8;
   result|=_bytes[_position++];

   return result;
}

-(float)_extractDataFloat {
   NSSwappedFloat swapped;

   swapped.floatWord=[self _extractWordFour];

   return NSConvertSwappedFloatToHost(swapped);
}

-(double)_extractDataDouble {
   NSSwappedDouble swapped;

   swapped.doubleWord=[self _extractWordEight];

   return NSConvertSwappedDoubleToHost(swapped);
}

-(NSString *)_extractCStringBytes {
   NSString *result;
   NSUInteger  length=0;

   while((_position+length)<_length && (_bytes[_position+length]!='\0'))
    length++;

   result=[NSString stringWithCString:(char *)(_bytes+_position) length:length];
   _position+=length+1;

   return result;
}

-(NSUInteger)_extractReference {
   return (NSUInteger)[self _extractWordFour];
}

-(NSString *)_extractCStringString {
   NSUInteger  ref=[self _extractReference];
   NSString *result=NSMapGet(_cStrings,(void *)ref);

   if(result==nil){
    result=[self _extractCStringBytes];
    NSMapInsert(_cStrings,(void *)ref,result);
   }

   return result;
}

-(const char *)_extractCString {
   return [[self _extractCStringString] cString];
}

-(Class)_extractClass {
   NSUInteger ref=[self _extractReference];
   Class    result;

   if(ref==0)
    return [NSObject class];
   else if((result=NSMapGet(_classes,(void *)ref))!=Nil)
    return result;
   else {
    NSString *className=[self _extractCStringString];
    NSUInteger  version=[self _extractWordFour];

    result=NSClassFromString(className);

    NSMapInsert(_classes,(void *)ref,result);
    NSMapInsert(_classVersions,className,(void *)version);

    [self _extractClass];

    return result;
   }
}

-(id)_extractObject {
   NSUInteger  ref=[self _extractReference];
   id        result;

   if(ref==0)
    return nil;
   else if((result=NSMapGet(_objects,(void *)ref))!=nil)
    [result retain];
   else{
    Class class=[self _extractClass];

    result=[class allocWithZone:NULL];
    NSMapInsert(_objects,(void *)ref,result);
    result=[result initWithCoder:self];
    result=[result awakeAfterUsingCoder:self];

    NSMapInsert(_objects,(void *)ref,result);

    [_allObjects addObject:result];
   }

   return result;
}

-(void)_extractArrayOfObjCType:(const char *)type length:(NSUInteger)length
  at:(void *)addr {

   switch(*type){
    case 'c':
    case 'C':{
      unsigned char *values=addr;
      int i;

      for(i=0;i<length;i++)
       values[i]=[self _extractWordOne];
     }
     break;

    case 's':
    case 'S':{
      unsigned short *values=addr;
      int i;

      for(i=0;i<length;i++)
       values[i]=[self _extractWordTwo];
     }
     break;

    default:
     [self cannotDecodeType:type];
     break;
   }
}

-(void)decodeValueOfObjCType:(const char *)type at:(void *)addr {
   const char *checkType=[self _extractCString];

   if(strcmp(checkType,type)!=0)
    [NSException raise:@"NSUnarchiverTypeMismatchException"
                format:@"NSUnarchiver type mismatch decoding %s, contains %s",type,checkType];

   switch(*type){
    case 'c':
    case 'C':{
      unsigned char *value=addr;
      *value=[self _extractWordOne];
     }
     break;

    case 's':
    case 'S':{
      unsigned short *value=addr;
      *value=[self _extractWordTwo];
     }
     break;

    case 'i':
    case 'I':{
      unsigned int *value=addr;
      *value=[self _extractWordFour];
     }
     break;

    case 'l':
    case 'L':{
      unsigned long *value=addr;
      *value=[self _extractWordFour];
     }
     break;

    case 'q':
    case 'Q':{
      unsigned long long *value=addr;
      *value=[self _extractWordEight];
     }
     break;

    case 'f':{
      float *value=addr;
      *value=[self _extractDataFloat];
     }
     break;

    case 'd':{
      double *value=addr;
      *value=[self _extractDataDouble];
     }
     break;

    case '*':{
      char     **cString=addr;
      NSString *string=[self _extractCStringString];

      *cString=NSZoneMalloc(NSDefaultMallocZone(),[string cStringLength]+1);
      [string getCString:*cString];
     }
     break;

    case '@':{
      id *object=addr;

      *object=[self _extractObject];
     }
     break;

    case '#':
     [self cannotDecodeType:type];
     break;

    case ':':{
      SEL      *value=addr;
      NSString *string=[self _extractCStringString];

      *value=NSSelectorFromString(string);
     }
     break;

    case '[':{
      const char *tmp=type;
      NSUInteger    length=0;

      tmp++; // skip [
      for(;*tmp>='0' && *tmp<='9';tmp++)
       length=(length*10)+(*tmp-'0');

      [self _extractArrayOfObjCType:tmp length:length at:addr];
     }
     break;

    case '{':
     if(strcmp(type,"{?=II}")==0){
      NSRange *value=addr;

      value->location=[self _extractWordFour];
      value->length=[self _extractWordFour];
      break;
     }
     if(strcmp(type,@encode(NSPoint))==0){
      NSPoint *value=addr;

      value->x=[self _extractDataFloat];
      value->y=[self _extractDataFloat];
      break;
     }
     if(strcmp(type,@encode(NSSize))==0){
      NSSize *value=addr;

      value->width=[self _extractDataFloat];
      value->height=[self _extractDataFloat];
      break;
     }
     if(strcmp(type,@encode(NSRect))==0){
      NSRect *value=addr;

      value->origin.x=[self _extractDataFloat];
      value->origin.y=[self _extractDataFloat];
      value->size.width=[self _extractDataFloat];
      value->size.height=[self _extractDataFloat];
      break;
     }
     [self cannotDecodeType:type];
     break;

    case '(':
    case 'b':
    case '^':
    case '?':
    default:
     [self cannotDecodeType:type];
     break;
   }

}

-(void *)decodeBytesWithReturnedLength:(NSUInteger *)lengthp {
   void    *result;
   unsigned length=[self _extractWordFour];

   [self _ensureLength:length];
   result=(void *)(_bytes+_position);
   _position+=length;

  *lengthp=length;

   return result;
}


-(NSData *)decodeDataObject {
   [self cannotDecodeType:"decodeDataObject"];
   return nil;
}


- (NSInteger)versionForClassName:(NSString *)className
{
    void *oKey, *oVal;

    if (!NSMapMember(_classVersions, className, &oKey, &oVal)) {
        //NSLog(@"no version for %@",className);
    }

    return (NSInteger)NSMapGet(_classVersions, className);
}


-(BOOL)invalidHeader {
   NSString *label;

   if(_length<strlen("~V1~")+1+4)
    return YES;

   label=[self _extractCStringBytes];
   if(![label isEqualToString:@"~V1~"])
    return YES;

   _version=[self _extractWordFour];
   if(_version>0)
    [NSException raise:@"NSUnarchiverInvalidVersionException"
                format:@"NSUnarchiver cannot unarchive version %d",_version];

   return NO;
}

-initForReadingWithData:(NSData *)data {
   _data=[data copy];
   _bytes=[data bytes];
   _position=0;
   _length=[data length];

   _objectZone=NSDefaultMallocZone();
   _version=0;
   _objects=NSCreateMapTable(NSIntMapKeyCallBacks,
       NSNonRetainedObjectMapValueCallBacks,0);
   _classes=NSCreateMapTable(NSIntMapKeyCallBacks,
      NSNonRetainedObjectMapValueCallBacks,0);
   _cStrings=NSCreateMapTable(NSIntMapKeyCallBacks,NSObjectMapValueCallBacks,0);
   _classVersions=NSCreateMapTable(NSObjectMapKeyCallBacks,
        NSIntMapValueCallBacks,0);

   _allObjects=[NSMutableArray new];

   if([self invalidHeader]){
    [self dealloc];
    return nil;
   }
   return self;
}

-(void)dealloc {
   [_data release];
   NSFreeMapTable(_objects);
   NSFreeMapTable(_classes);
   NSFreeMapTable(_cStrings);
   NSFreeMapTable(_classVersions);
   [_allObjects release];
   [super dealloc];
}

+(id)unarchiveObjectWithData:(NSData *)data {
   NSUnarchiver *unarchiver=[[[NSUnarchiver allocWithZone:NULL] initForReadingWithData:data] autorelease];

   return [unarchiver decodeObject];
}

+(id)unarchiveObjectWithFile:(NSString *)path {
   NSData       *data=[NSData dataWithContentsOfFile:path];
   NSUnarchiver *unarchiver;

   if(data==nil)
    return nil;

   unarchiver=[[[NSUnarchiver allocWithZone:NULL] initForReadingWithData:data] autorelease];

   return [unarchiver decodeObject];
}

-(BOOL)isAtEnd {
   return (_position<_length)?NO:YES;
}

-(NSZone *)objectZone {
   return _objectZone;
}

-(void)setObjectZone:(NSZone *)zone {
   _objectZone=zone;
}

-(void)decodeClassName:(NSString *)archiveName asClassName:(NSString *)runtimeName {
   NSUnimplementedMethod();
}

-(NSString *)classNameDecodedForArchiveClassName:(NSString *)className {
   NSUnimplementedMethod();
   return 0;
}

+(void)decodeClassName:(NSString *)archiveName asClassName:(NSString *)runtimeName {
   NSUnimplementedMethod();
}

+(NSString *)classNameDecodedForArchiveClassName:(NSString *)className {
   NSUnimplementedMethod();
   return 0;
}

-(void)replaceObject:original withObject:replacement {
   NSUnimplementedMethod();
}

@end
