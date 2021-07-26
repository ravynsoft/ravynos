/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSPropertyListWriter_vintage.h>
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>

@implementation NSPropertyListWriter_vintage

static BOOL _NSPropertyListNameSet[128]={
 NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO,// 0
 NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO, NO,// 16
 NO, NO, NO, NO,YES, NO, NO, NO, NO, NO, NO, NO, NO, NO,YES,YES,// 32
YES,YES,YES,YES,YES,YES,YES,YES,YES,YES, NO, NO, NO, NO, NO, NO,// 48
 NO,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,// 64
YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES, NO, NO, NO, NO,YES,// 80
 NO,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,// 96
YES,YES,YES,YES,YES,YES,YES,YES,YES,YES,YES, NO, NO, NO, NO, NO,// 112
};

static NSInteger keySort(id key1, id key2, void *context)
{
    
    if ([key1 isKindOfClass:objc_lookUpClass("NSString")] 
        && [key2 isKindOfClass:objc_lookUpClass("NSString")])
        return [key1 compare:key2];
    else
        //undefined
        return NSOrderedDescending;
}

-init {
   _data=[NSMutableData new];
   return self;
}

-(void)dealloc {
   [_data release];
   [super dealloc];
}

-(void)encodeIndent:(NSInteger)indent {
   int i;

   [_data appendBytes:" " length:1];
   for(i=0;i<indent;i++)
    [_data appendBytes:"  " length:2];
}

-(void)encodeString:(NSString *)string escape:(BOOL)escape {
   NSUInteger length=[string length];
   unichar  buffer[length];
   int      i;

   [string getCharacters:buffer];

   if(length==0){
    [_data appendBytes:"\"\"" length:2];
    return;
   }

   for(i=0;i<length;i++)
    if(buffer[i]>=128 || !_NSPropertyListNameSet[buffer[i]])
     break;

   if(i>=length){
    char *charBuf;

    charBuf=NSZoneMalloc(NULL,length);

    for(i=0;i<length;i++)
     charBuf[i]=buffer[i];

    [_data appendBytes:charBuf length:length];
    NSZoneFree(NULL,charBuf);
   }
   else {
    char *charBuf;
    int  bufLen=0;

    charBuf=NSZoneMalloc(NULL,length*6+2);

    charBuf[bufLen++]='\"';

    for(i=0;i<length;i++){
     unichar unicode=buffer[i];

     if(unicode<' ' || unicode==127){
      if(!escape && unicode=='\n'){
       charBuf[bufLen++]=unicode;
      }
      else {
       charBuf[bufLen++]='\\';
       charBuf[bufLen++]=(unicode>>6)+'0';
       charBuf[bufLen++]=((unicode>>3)&0x07)+'0';
       charBuf[bufLen++]=(unicode&0x07)+'0';
      }
     }
     else if(unicode<128){
      if(escape && (unicode=='\"' || unicode=='\\'))
       charBuf[bufLen++]='\\';
      charBuf[bufLen++]=unicode;
     }
     else {
      const char *hex="0123456789ABCDEF";

      charBuf[bufLen++]='\\';
      charBuf[bufLen++]='U';
      charBuf[bufLen++]=hex[(unicode>>12)&0x0F];
      charBuf[bufLen++]=hex[(unicode>>8)&0x0F];
      charBuf[bufLen++]=hex[(unicode>>4)&0x0F];
      charBuf[bufLen++]=hex[unicode&0x0F];
     }
    }

    charBuf[bufLen++]='\"';

    [_data appendBytes:charBuf length:bufLen];
    NSZoneFree(NULL,charBuf);
   }
}

-(void)encodeArray:(NSArray *)array indent:(NSInteger)indent {
   NSInteger i,count=[array count];

   [_data appendBytes:"(\n" length:2];
   for(i=0;i<count;i++){
    [self encodeIndent:indent];
    [self encodePropertyList:[array objectAtIndex:i] indent:indent+1];
    if(i+1<count)
     [_data appendBytes:",\n" length:2];
    else
     [_data appendBytes:"\n" length:1];
   }
   [self encodeIndent:indent-1];
   [_data appendBytes:")" length:1];
}

-(void)encodeDictionary:(NSDictionary *)dictionary indent:(NSInteger)indent {
   NSArray *allKeys=[[dictionary allKeys]
    sortedArrayUsingFunction:keySort context:NULL];
   NSInteger      i,count=[allKeys count];

   [_data appendBytes:"{\n" length:2];
   for(i=0;i<count;i++){
    id key=[allKeys objectAtIndex:i];

    [self encodeIndent:indent];
    [self encodeString:[key description] escape:YES];
    [_data appendBytes:" = " length:3];
    [self encodePropertyList:[dictionary objectForKey:key] indent:indent+1];
    [_data appendBytes:";\n" length:2];
   }
   if(indent>0)
    [self encodeIndent:indent-1];
   [_data appendBytes:"}" length:1];
}

-(void)encodePropertyList:plist escape:(BOOL)escape indent:(NSInteger)indent {
   if([plist isKindOfClass:objc_lookUpClass("NSString")])
    [self encodeString:plist escape:escape];
   else if([plist isKindOfClass:objc_lookUpClass("NSArray")])
    [self encodeArray:plist indent:indent];
   else if([plist isKindOfClass:objc_lookUpClass("NSDictionary")])
    [self encodeDictionary:plist indent:indent];
   else
    [self encodeString:[plist description] escape:escape];
}

-(void)encodePropertyList:plist indent:(NSInteger)indent {
   [self encodePropertyList:plist escape:YES indent:indent];
}

-(NSData *)dataForRootObject:object {
   [self encodePropertyList:object escape:YES indent:0];
   return _data;
}

-(NSData *)nullTerminatedASCIIDataWithString:(NSString *)string {
   [self encodeString:string escape:NO];
   [_data appendBytes:"\0" length:1];
   return _data;
}

+(NSData *)nullTerminatedASCIIDataWithString:(NSString *)string {
   NSPropertyListWriter_vintage *writer=[[self alloc] init];
   NSData               *result=[[[writer nullTerminatedASCIIDataWithString:string] retain] autorelease];

   [writer release];

   return result;
}

-(NSData *)nullTerminatedASCIIDataWithPropertyList:plist {
   [self encodePropertyList:plist escape:YES indent:0];
   [_data appendBytes:"\0" length:1];
   return _data;
}

+(NSData *)nullTerminatedASCIIDataWithPropertyList:plist {
   NSPropertyListWriter_vintage *writer=[[self alloc] init];
   NSData               *result=[[[writer nullTerminatedASCIIDataWithPropertyList:plist] retain] autorelease];

   [writer release];

   return result;
}

+(NSData *)dataWithPropertyList:plist {
   NSPropertyListWriter_vintage *writer=[[self alloc] init];
   NSData               *result=[[[writer dataForRootObject:plist] retain] autorelease];

   [writer release];

   return result;
}

+(NSString *)stringWithPropertyList:plist {
   NSPropertyListWriter_vintage *writer=[[self alloc] init];
   NSData               *data=[writer dataForRootObject:plist];
   NSString             *result=[[[NSString allocWithZone:NULL] initWithData:data encoding:NSASCIIStringEncoding] autorelease];

   [writer release];

   return result;
}

+(BOOL)writePropertyList:object toFile:(NSString *)path atomically:(BOOL)atomically {
   NSPropertyListWriter_vintage *writer=[[self alloc] init];
   NSData               *data=[writer dataForRootObject:object];
   BOOL                  result=[data writeToFile:path atomically:atomically];

   [writer release];

   return result;
}

@end
