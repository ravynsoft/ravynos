/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSOldXMLReader.h"
#import "NSOldXMLElement.h"
#import "NSOldXMLAttribute.h"
#import "NSOldXMLDocument.h"
#import <Foundation/NSData.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSStringUTF8.h>
#include <string.h>

enum {
   STATE_content,
   STATE_Reference,
   STATE_CharRef,
   STATE_CharRef_hex,
   STATE_CharRef_decimal,
   STATE_EntityRef_Name,
   STATE_Tag,
   STATE_ignore_unhandled,
   STATE_STag,
   STATE_ETag,
   STATE_ETag_whitespace,
   STATE_Attributes,
   STATE_EmptyElementTag,
   STATE_Attribute_Name,
   STATE_Attribute_Name_whitespace,
   STATE_Attribute_Equal,
   STATE_Attribute_Value,
   STATE_Attribute_Value_DoubleQuote,
   STATE_Attribute_Value_SingleQuote,
   STATE_CDATA
};

@implementation NSOldXMLReader

-initWithData:(NSData *)data {
   _data=[data copy];
   _bytes=[_data bytes];
   _length=[_data length];
   _range=NSMakeRange(0,0);

   _entityRefContents=[NSMutableDictionary new];
   [_entityRefContents setObject:@"&" forKey:@"amp"];
   [_entityRefContents setObject:@"<" forKey:@"lt"];
   [_entityRefContents setObject:@">" forKey:@"gt"];
   [_entityRefContents setObject:@"\'" forKey:@"apos"];
   [_entityRefContents setObject:@"\"" forKey:@"quot"];

   _state=STATE_content;
   _stack=[NSMutableArray new];
   _strings=NSCreateHashTable(NSObjectHashCallBacks,0);
   _rootElement=nil;
   return self;
}

-initWithContentsOfFile:(NSString *)path {
   NSData *data=[NSData dataWithContentsOfMappedFile:path];

   if(data==nil){
    [self dealloc];
    return nil;
   }

   return [self initWithData:data];
}

-(void)dealloc {
   [_data release];
   [_entityRefContents release];
   [_stack release];
   NSFreeHashTable(_strings);
   [_rootElement release];
   [super dealloc];
}

-(NSOldXMLElement *)rootElement {
   return _rootElement;
}

-(NSUInteger)length {
   return _range.length;
}

-(unichar)characterAtIndex:(NSUInteger)location {
   return _bytes[_range.location+location];
}

-(void)getCharacters:(unichar *)buffer {
   NSUInteger i;

   for(i=0;i<_range.length;i++)
    buffer[i]=_bytes[_range.location+i];
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSUInteger location=range.location,max=NSMaxRange(range);
   NSUInteger i;

   for(i=0;location<max;i++,location++)
    buffer[i]=_bytes[_range.location+location];
}

-(void)content:(NSString *)content {
   [[_stack lastObject] addContent:content];
}

-(void)charRef:(NSString *)charRef {
   [[_stack lastObject] addContent:charRef];
}

-(void)entityRef:(NSString *)entityRef {
   NSOldXMLElement *element=[_stack lastObject];
   NSString   *contents=[_entityRefContents objectForKey:self];

   if(contents!=nil)
    [element addContent:contents];
}

-(void)sTag:(NSString *)sTag {
   [_stack addObject:[NSOldXMLElement elementWithName:sTag]];
   if([_stack count]==1){
    [_rootElement release];
    _rootElement=[[_stack lastObject] retain];
   }
}

-(void)popElement {
   NSOldXMLElement *last=[[_stack lastObject] retain];

   [_stack removeLastObject];
   [[_stack lastObject] addContent:last];
   [last release];
}

-(void)eTag:(NSString *)eTag {
// FIX, maybe double check name here
   [self popElement];
}

-(void)emptyElementTag {
   [self popElement];
}

-(void)attributeName:(NSString *)name {
   [_stack addObject:name];
}

-(void)attributeValue:(NSString *)value {
   NSString     *name=[_stack lastObject];
   NSOldXMLAttribute *attribute=[NSOldXMLAttribute attributeWithName:name value:value];

   [_stack removeLastObject];
   [[_stack lastObject] addAttribute:attribute];
}

-(NSString *)stringForSelf {
   if(NSUTF8IsASCII((const char *)(_bytes+_range.location),_range.length))
    return self;
   else {
    NSUInteger  length;
    unichar  *buffer=NSUTF8ToUnicode((const char *)(_bytes+_range.location),_range.length,&length,NULL);
    NSString *result=[[NSString alloc] initWithCharacters:buffer length:length];
    
    NSZoneFree(NULL,buffer);
    
    return result;
   }
}

-(NSString *)uniqueSelf {
   NSString *string=[self stringForSelf];
   NSString *result;

   result=NSHashGet(_strings,string);

   if(result==nil){
    result=[[NSString alloc] initWithString:string];
    NSHashInsert(_strings,result);
    [result release];
   }

   return result;
}

static inline BOOL codeIsWhitespace(uint8_t code){
   if(code==0x20 || code==0x0A || code==0x0D || code==0x09)
    return YES;
   return NO;
}

static inline BOOL codeIsNameStart(uint8_t code){
   if((code>='A' && code<='Z') ||
      (code>='a' && code<='z') || 
       code==':' || code=='_')
    return YES;

   return NO;
}

static inline BOOL codeIsNameContinue(uint8_t code){
   if((code>='A' && code<='Z') ||
      (code>='a' && code<='z') ||
       code==':' || code=='_' ||
      (code>='0' && code<='9') ||
       code=='.' || code=='-')
    return YES;

   return NO;
}

-(void)unexpectedIn:(NSString *)state {
   NSUInteger      position=NSMaxRange(_range)-1;
   uint8_t code=_bytes[position];

   [NSException raise:@"" format:@"Unexpected character %c in %@, position=%d",code,state,position];
}

-(BOOL)tokenize {

   while(NSMaxRange(_range)<_length){
    uint8_t code=_bytes[NSMaxRange(_range)];
    enum  {
     extendLength,
     advanceLocationToNext,
     advanceLocationToCurrent,
    } rangeAction=extendLength;

    switch(_state){

     case STATE_content:
      if(code=='&'){
       if(_range.length>0)
        [self content:[self uniqueSelf]];
       _state=STATE_Reference;
       rangeAction=advanceLocationToNext;
      }
      else if(code=='<'){
       if(_range.length>0)
        [self content:[self uniqueSelf]];
       _state=STATE_Tag;
       rangeAction=advanceLocationToNext;
      }
      else {
       _state=STATE_content;
      }
      break;

     case STATE_Reference:
      if(code=='#'){
       _charRef=0;
       _state=STATE_CharRef;
       rangeAction=advanceLocationToNext;
      }
      else if(codeIsNameStart(code)){
       _state=STATE_EntityRef_Name;
       rangeAction=advanceLocationToCurrent;
      }
      else {
       [self unexpectedIn:@"Reference"];
       return NO;
      }
      break;

     case STATE_CharRef:
      if(code=='x'){
       _state=STATE_CharRef_hex;
       rangeAction=advanceLocationToCurrent;
      }
      else if(code>='0' && code<='9'){
       _charRef=code-'0';
       _state=STATE_CharRef_decimal;
       rangeAction=advanceLocationToCurrent;
      }
      else {
       [self unexpectedIn:@"CharRef"];
       return NO;
      }
      break;

     case STATE_CharRef_hex:
      if(code>='0' && code<='9'){
       _charRef=_charRef*16+code-'0';
       _state=STATE_CharRef_hex;
      }
      else if(code>='a' && code<='z'){
       _charRef=_charRef*16+code-'a'+10;
       _state=STATE_CharRef_hex;
      }
      else if(code>='A' && code<='Z'){
       _charRef=_charRef*16+code-'A'+10;
       _state=STATE_CharRef_hex;
      }
      else if(code==';'){
       [self charRef:[NSString stringWithCharacters:&_charRef length:1]];
       _state=STATE_content;
       rangeAction=advanceLocationToNext;
      }
      else{
       [self unexpectedIn:@"hexadecimal CharRef"];
       return NO;
      }
      break;

     case STATE_CharRef_decimal:
      if(code>='0' && code<='9'){
       _charRef=_charRef*10+code-'0';
       _state=STATE_CharRef_decimal;
     }
      else if(code==';'){
       [self charRef:[NSString stringWithCharacters:&_charRef length:1]];
       _state=STATE_content;
       rangeAction=advanceLocationToNext;
      }
      else {
       [self unexpectedIn:@"decimal CharRef"];
       return NO;
      }
      break;

     case STATE_EntityRef_Name:
      if(codeIsNameContinue(code))
       _state=STATE_EntityRef_Name;
      else if(code==';'){
       [self entityRef:[self uniqueSelf]];
       _state=STATE_content;
       rangeAction=advanceLocationToNext;
      }
      else {
       [self unexpectedIn:@"EntityRef Name"];
       return NO;
      }
      break;

     case STATE_Tag:
      if(code=='/'){
       _state=STATE_ETag;
       rangeAction=advanceLocationToNext;
      }
      else if(codeIsNameStart(code)){
       _state=STATE_STag;
       rangeAction=advanceLocationToCurrent;
      }
      else if(code=='?'){ // FIX, to just get through ?xml
       _state=STATE_ignore_unhandled;
       rangeAction=advanceLocationToNext;
      }
      else if(code=='!'){
       if (NSMaxRange(_range)+8 < _length) {
         if (0 == memcmp(_bytes + NSMaxRange(_range), "![CDATA[", 8)) {
           _state = STATE_CDATA;
           _range.length += 8;
           rangeAction = advanceLocationToCurrent;
         }
       }
       if (_state != STATE_CDATA) {  // get through !DOCTYPE
         _state=STATE_ignore_unhandled;
         rangeAction=advanceLocationToNext;
       }
      }
      else {
       [self unexpectedIn:@"Tag"];
       return NO;
      }
      break;
      
     case STATE_CDATA:
      if (code==']' && NSMaxRange(_range)+3 < _length) {
        if (0 == memcmp(_bytes + NSMaxRange(_range), "]]>", 3)) {
          if(_range.length>0)
            [self content:[self uniqueSelf]];
        
          _state = STATE_content;
          _range.length += 3;
          rangeAction = advanceLocationToCurrent;
        }
      }
      break;

     case STATE_ignore_unhandled:
      rangeAction=advanceLocationToNext;
      if(code=='>')
       _state=STATE_content;
      break;
      
     case STATE_STag:
      if(codeIsNameContinue(code))
       _state=STATE_STag;
      else {
       [self sTag:[self uniqueSelf]];
       _state=STATE_Attributes;
       rangeAction=advanceLocationToCurrent;
      }
      break;
      
     case STATE_ETag:
      if(codeIsNameContinue(code))
       _state=STATE_ETag;
      else {
       [self eTag:[self uniqueSelf]];
       _state=STATE_ETag_whitespace;
       rangeAction=advanceLocationToCurrent;
      }
      break;

     case STATE_ETag_whitespace:
      if(codeIsWhitespace(code))
       _state=STATE_ETag_whitespace;
      else if(code=='>'){
       _state=STATE_content;
       rangeAction=advanceLocationToNext;
      }
      else {
       [self unexpectedIn:@"ETag"];
       return NO;
      }
      break;

     case STATE_Attributes:
      if(codeIsWhitespace(code))
       _state=STATE_Attributes;
      else if(code=='/')
       _state=STATE_EmptyElementTag;
      else if(code=='>'){
       _state=STATE_content;
       rangeAction=advanceLocationToNext;
      }
      else if(codeIsNameStart(code)){
       _state=STATE_Attribute_Name;
       rangeAction=advanceLocationToCurrent;
      }
      break;

     case STATE_EmptyElementTag:
      if(code=='>'){
       [self emptyElementTag];
       _state=STATE_content;
       rangeAction=advanceLocationToNext;
      }
      else {
       [self unexpectedIn:@"EmptyElementTag"];
       return NO;
      }
      break;

     case STATE_Attribute_Name:
      if(codeIsNameContinue(code))
       _state=STATE_Attribute_Name;
      else {
       [self attributeName:[self uniqueSelf]];
       _state=STATE_Attribute_Name_whitespace;
       rangeAction=advanceLocationToCurrent;
      }
      break;

     case STATE_Attribute_Name_whitespace:
      if(codeIsWhitespace(code))
       _state=STATE_Attribute_Name_whitespace;
      else if(code=='=')
       _state=STATE_Attribute_Equal;
      break;

     case STATE_Attribute_Equal:
      if(codeIsWhitespace(code))
       _state=STATE_Attribute_Equal;
      else {
       rangeAction=advanceLocationToCurrent;
       _state=STATE_Attribute_Value;
      }
      break;

     case STATE_Attribute_Value:
      if(code=='\"'){
       _state=STATE_Attribute_Value_DoubleQuote;
       rangeAction=advanceLocationToNext;
      }
      else if(code=='\''){
       _state=STATE_Attribute_Value_SingleQuote;
       rangeAction=advanceLocationToNext;
      }
      else {
       [self unexpectedIn:@"Attribute Value"];
       return NO;
      }
      break;

     case STATE_Attribute_Value_DoubleQuote:
      if(code=='\"'){
       [self attributeValue:[self uniqueSelf]];
       _state=STATE_Attributes;
       rangeAction=advanceLocationToNext;
      }
      break;

     case STATE_Attribute_Value_SingleQuote:
      if(code=='\''){
        [self attributeValue:[self uniqueSelf]];
       _state=STATE_Attributes;
       rangeAction=advanceLocationToNext;
      }
      break;
    }

    switch(rangeAction){
     case extendLength:
      _range.length++;
      break;

     case advanceLocationToNext:
      _range.location=NSMaxRange(_range)+1;
      _range.length=0;
      break;

     case advanceLocationToCurrent:
      _range.location=NSMaxRange(_range);
      _range.length=0;
      break;
    }
   }
   return YES;
}

+(NSOldXMLDocument *)documentWithContentsOfFile:(NSString *)path {
   NSOldXMLReader   *reader=[[self alloc] initWithContentsOfFile:path];
   NSOldXMLDocument *document;

    if(![reader tokenize]) {
        [reader release];
    return nil;
    }

   document=[[[NSOldXMLDocument alloc] init] autorelease];
   [document setRootElement:[reader rootElement]];

   [reader release];

   return document;
}

+(NSOldXMLDocument *)documentWithData:(NSData *)data {
   NSOldXMLReader   *reader=[[self alloc] initWithData:data];
   NSOldXMLDocument *document;

    if(![reader tokenize]) {
        [reader release];
    return nil;
    }

   document=[[[NSOldXMLDocument alloc] init] autorelease];
   [document setRootElement:[reader rootElement]];

   [reader release];

   return document;
}

@end
