/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Decoder_TIFF.h>
#import "O2TIFFImageDirectory.h"

@implementation O2Decoder_TIFF

-(BOOL)tracingEnabled {
   return [[NSUserDefaults standardUserDefaults] boolForKey:@"NSTIFFTracingEnabled"];
}

-(unsigned)currentOffset {
   return _position;
}

static void seekToOffset(O2Decoder_TIFF *self,unsigned offset) {
   if(offset>=self->_length)
    [NSException raise:NSInvalidArgumentException format:@"Attempt to seek past end of TIFF, length=%d,offset=%d",self->_length,offset];

   self->_position=offset;
}

-(void)seekToOffset:(unsigned)offset {
   seekToOffset(self,offset);
}

static unsigned currentThenSeekToOffset(O2Decoder_TIFF *self,unsigned offset) {
   unsigned result=self->_position;

   seekToOffset(self,offset);

   return result;
}

static uint8_t nextUnsigned8(O2Decoder_TIFF *self) {
   if(self->_position<self->_length)
    return self->_bytes[self->_position++];

   [NSException raise:NSInvalidArgumentException format:@"Attempt to read past end of TIFF, length=%d",self->_length];
   return 0;
}

-(unsigned char)nextUnsigned8 {
   return nextUnsigned8(self);
}

static unsigned nextUnsigned16(O2Decoder_TIFF *self) {
   unsigned result;
   unsigned byte0=nextUnsigned8(self);
   unsigned byte1=nextUnsigned8(self);

   if(self->_bigEndian){
    result=byte0;
    result<<=8;
    result|=byte1;
   }
   else {
    result=byte1;
    result<<=8;
    result|=byte0;
   }

   return result;
}

unsigned O2DecoderNextUnsigned16_TIFF(O2Decoder_TIFF *self) {
   return nextUnsigned16(self);
}

static unsigned nextUnsigned32(O2Decoder_TIFF *self) {
   unsigned result;
   unsigned byte0=nextUnsigned8(self);
   unsigned byte1=nextUnsigned8(self);
   unsigned byte2=nextUnsigned8(self);
   unsigned byte3=nextUnsigned8(self);

   if(self->_bigEndian){
    result=byte0;
    result<<=8;
    result|=byte1;
    result<<=8;
    result|=byte2;
    result<<=8;
    result|=byte3;
   }
   else {
    result=byte3;
    result<<=8;
    result|=byte2;
    result<<=8;
    result|=byte1;
    result<<=8;
    result|=byte0;
   }

   return result;
}

-(unsigned)nextUnsigned32 {
   return nextUnsigned32(self);
}

static unsigned nextUnsigned8AtOffset(O2Decoder_TIFF *self,unsigned *offset) {
   unsigned result;
   unsigned save=currentThenSeekToOffset(self,*offset);

   result=nextUnsigned8(self);

   *offset=currentThenSeekToOffset(self,save);

   return result;
}

static unsigned nextUnsigned16AtOffset(O2Decoder_TIFF *self,unsigned *offset) {
   unsigned result;
   unsigned save=currentThenSeekToOffset(self,*offset);

   result=nextUnsigned16(self);

   *offset=currentThenSeekToOffset(self,save);

   return result;
}

static unsigned nextUnsigned32AtOffset(O2Decoder_TIFF *self,unsigned *offset) {
   unsigned result;
   unsigned save=currentThenSeekToOffset(self,*offset);

   result=nextUnsigned32(self);

   *offset=currentThenSeekToOffset(self,save);

   return result;
}

-(unsigned)expectUnsigned16 {
   unsigned result=0;
   unsigned type=nextUnsigned16(self);
   unsigned numberOfValues=nextUnsigned32(self);

   if(numberOfValues!=1){
    NSLog(@"TIFF parse error, expecting 1 value, got %d,type=%d",numberOfValues,type);
    return 0;
   }

   if(type==NSTIFFTypeSHORT)
    result=nextUnsigned16(self);
   else
    NSLog(@"TIFF parse error, expecting unsigned16 got %d",type);

   return result;
}

-(unsigned)expectUnsigned32 {
   unsigned result=0;
   unsigned type=nextUnsigned16(self);
   unsigned numberOfValues=nextUnsigned32(self);

   if(numberOfValues!=1){
    NSLog(@"TIFF parse error, expecting 1 value, got %d,type=%d",numberOfValues,type);
    return 0;
   }

   if(type==NSTIFFTypeLONG)
    result=nextUnsigned32(self);
   else
    NSLog(@"TIFF parse error, expecting unsigned16 or unsinged32, got %d",type);

   return result;
}

-(double)expectRational {
   unsigned type=nextUnsigned16(self);
   unsigned numberOfValues=nextUnsigned32(self);
   unsigned offset=nextUnsigned32(self);
   double numerator,denominator;

   if(type!=NSTIFFTypeRATIONAL){
    NSLog(@"TIFF parse error, expecting rational, got %d",type);
    return 0;
   }

   if(numberOfValues!=1){
    NSLog(@"TIFF parse error, expecting 1 value, got %d,type=%d",numberOfValues,type);
    return 0;
   }

   numerator=nextUnsigned32AtOffset(self,&offset);
   denominator=nextUnsigned32AtOffset(self,&offset);

   return numerator/denominator;
}

-(void)_decodeArrayOfUnsigned8:(unsigned char **)valuesp count:(unsigned *)countp {
   unsigned       numberOfValues=nextUnsigned32(self);
   unsigned char *values;

   values=NSZoneMalloc([self zone],numberOfValues*sizeof(unsigned));

   if(numberOfValues==1)
    values[0]=nextUnsigned8(self);
   else if(numberOfValues==2){
    values[0]=nextUnsigned8(self);
    values[1]=nextUnsigned8(self);
   }
   else {
    unsigned i,offset=nextUnsigned32(self);

    for(i=0;i<numberOfValues;i++)
     values[i]=nextUnsigned8AtOffset(self,&offset);
   }

   *countp=numberOfValues;
   *valuesp=values; 
}

static void _decodeArrayOfUnsigned16(O2Decoder_TIFF *self,unsigned **valuesp,unsigned *countp) {
   unsigned  numberOfValues=nextUnsigned32(self);
   unsigned *values;

   values=NSZoneMalloc([self zone],numberOfValues*sizeof(unsigned));

   if(numberOfValues==1)
    values[0]=nextUnsigned16(self);
   else if(numberOfValues==2){
    values[0]=nextUnsigned16(self);
    values[1]=nextUnsigned16(self);
   }
   else {
    unsigned i,offset=nextUnsigned32(self);

    for(i=0;i<numberOfValues;i++)
     values[i]=nextUnsigned16AtOffset(self,&offset);
   }

   *countp=numberOfValues;
   *valuesp=values; 
}

static void _decodeArrayOfUnsigned32(O2Decoder_TIFF *self,unsigned **valuesp,unsigned *countp) {
   unsigned  numberOfValues=nextUnsigned32(self);
   unsigned *values;

   values=NSZoneMalloc([self zone],numberOfValues*sizeof(unsigned));

   if(numberOfValues==1)
    values[0]=nextUnsigned32(self);
   else {
    unsigned i,offset=nextUnsigned32(self);

    for(i=0;i<numberOfValues;i++)
     values[i]=nextUnsigned32AtOffset(self,&offset);
   }

   *countp=numberOfValues;
   *valuesp=values; 
}

-(NSString *)expectASCII {
   unsigned       type=nextUnsigned16(self);
   unsigned       count;
   unsigned char *ascii;

   if(type!=NSTIFFTypeASCII){
    NSLog(@"TIFF parse error, expecting ASCII, got %d",type);
    return nil;
   }

   [self _decodeArrayOfUnsigned8:&ascii count:&count];

   if(count==0){
    NSLog(@"TIFF parse error, ASCII count = 0");
    return nil;
   }

   return [[[NSString alloc] initWithBytes:ascii length:count-1 encoding:NSISOLatin1StringEncoding] autorelease];
}

-(unsigned)expectUnsigned16OrUnsigned32 {
   unsigned result=0;
   unsigned type=nextUnsigned16(self);
   unsigned numberOfValues=nextUnsigned32(self);

   if(numberOfValues!=1)
    NSLog(@"TIFF parse error, expecting 1 value, got %d,type=%d",numberOfValues,type);

   if(type==NSTIFFTypeSHORT)
    result=nextUnsigned16(self);
   else if(type==NSTIFFTypeLONG)
    result=nextUnsigned32(self);
   else
    NSLog(@"TIFF parse error, expecting unsigned16 or unsigned32, got %d",type);

   return result;
}

-(void)expectArrayOfUnsigned8:(unsigned char **)valuesp count:(unsigned *)countp {
   unsigned type=nextUnsigned16(self);

   if(type!=NSTIFFTypeBYTE){
    NSLog(@"TIFF parse error, expecting unsigned8");
    return;
   }

   [self _decodeArrayOfUnsigned8:valuesp count:countp];
}

-(void)expectArrayOfUnsigned16:(unsigned **)valuesp count:(unsigned *)countp {
   unsigned type=nextUnsigned16(self);

   if(type!=NSTIFFTypeSHORT){
    NSLog(@"TIFF parse error, expecting unsigned16");
    return;
   }

   _decodeArrayOfUnsigned16(self,valuesp,countp);
}

-(void)expectArrayOfUnsigned16OrUnsigned32:(unsigned **)valuesp count:(unsigned *)countp {
   unsigned type=nextUnsigned16(self);

   if(type==NSTIFFTypeSHORT)
    _decodeArrayOfUnsigned16(self,valuesp,countp);
   else if(type==NSTIFFTypeLONG)
    _decodeArrayOfUnsigned32(self,valuesp,countp);
   else
    NSLog(@"TIFF parse error, expecting unsigned16");
}

-(unsigned)parseImageFileDirectoryAtOffset:(unsigned)offset {
   O2TIFFImageDirectory *imageFileDirectory;

   seekToOffset(self,offset);

   imageFileDirectory=[[[O2TIFFImageDirectory alloc] initWithTIFFReader:self] autorelease];

   [_directory addObject:imageFileDirectory];

   return nextUnsigned32(self);
}

-(BOOL)parseImageFileHeader {
   BOOL result=YES;

   NS_DURING
    unsigned char byte0=nextUnsigned8(self);
    unsigned char byte1=nextUnsigned8(self);
    unsigned      fortyTwo;
    unsigned      nextEntryOffset;

    if(byte0=='I' && byte1=='I')
     _bigEndian=NO;
    else if(byte0=='M' && byte1=='M')
     _bigEndian=YES;
    else {
     NSLog(@"Unknown endian markers %02X %02X",byte0,byte1);
     return NO;
    }

    fortyTwo=nextUnsigned16(self);
    nextEntryOffset=nextUnsigned32(self);

    if(fortyTwo!=42){
     NSLog(@"FortyTwo does not equal 42, got %d",fortyTwo);
     return NO;
    }

    while((nextEntryOffset=[self parseImageFileDirectoryAtOffset:nextEntryOffset])!=0)
     ;

   NS_HANDLER
    result=NO;
   NS_ENDHANDLER

   return result;
}

-initWithData:(NSData *)data {
   _data=[data copy];
   _bytes=[_data bytes];
   _length=[_data length];
   _position=0;

   _directory=[NSMutableArray new];

   if(![self parseImageFileHeader]){
    [self dealloc];
    return nil;
   }
   return self;
}

-initWithContentsOfFile:(NSString *)path {
   NSData *data=[NSData dataWithContentsOfFile:path];
   
   if(data==nil){
    [self dealloc];
    return nil;
   }
   
   return [self initWithData:data];
}

-(void)dealloc {
   [_data release];
   [_directory release];
   [super dealloc];
}

-(NSData *)data {
   return _data;
}

-(NSArray *)imageFileDirectory {
   return _directory;
}

@end
