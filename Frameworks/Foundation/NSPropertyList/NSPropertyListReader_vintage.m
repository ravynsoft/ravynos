/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSPropertyListReader_vintage.h>
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSException.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSNumber.h>

@implementation NSPropertyListReader_vintage

-initWithData:(NSData *)data {
   _data=[data retain];
   _length=[_data length];
   _bytes=[_data bytes];

   _stackCapacity=4096;
   _stackSize=0;
   _stack=NSZoneMalloc(NULL,sizeof(id)*_stackCapacity);

   _bufferCapacity=256;
   _bufferSize=0;
   _buffer=NSZoneMalloc(NULL,sizeof(unichar *)*_bufferCapacity);

   _dataBufferCapacity=256;
   _dataBufferSize=0;
   _dataBuffer=NSZoneMalloc(NULL,sizeof(uint8_t *)*_dataBufferCapacity);

   _index=0;
   _lineNumber=1;

   _stringClass=[NSString class];
   _dictionaryClass=[NSMutableDictionary class];
   _arrayClass=[NSMutableArray class];
   
   return self;
}

-(void)dealloc {
   [_data release];
   if(_stack!=NULL)
    NSZoneFree(NULL,_stack);
   if(_buffer!=NULL)
    NSZoneFree(NULL,_buffer);
   if(_dataBuffer!=NULL)
    NSZoneFree(NULL,_dataBuffer);
   [super dealloc];
}

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

-(id)internalError:(id)class {
   int i;

   for(i=0;i<_stackSize;i++)
    [_stack[i] release];

  [NSException raise:NSParseErrorException format:@"*** Parse error at position %d,line %d, expecting %@ on stack",
     _index,_lineNumber,[class description]];

   return nil;
}

-(id)parseError:(int)expect token:(int)token info:(NSString *)info {
   const char *expectStr[]={ "String","=","Object",", or ;","EOF"};
   char tokenStr[2]={token,'\0'};
   int i;

   for(i=0;i<_stackSize;i++)
    [_stack[i] release];

   [NSException raise:NSParseErrorException format:@"*** Parse error at position %d, line %d. src=%@. %s expected. Next token is %s.",_index,_lineNumber,info,expectStr[expect],(token==-1)?"EOF":tokenStr];

   return nil;
}

static inline void pushObject(NSPropertyListReader_vintage *self,id object){
   if(self->_stackSize>=self->_stackCapacity){
    self->_stackCapacity*=2;
    self->_stack=NSZoneRealloc(NULL,self->_stack,sizeof(id)*self->_stackCapacity);
   }

   self->_stack[self->_stackSize++]=object;
}

static inline id popObject(NSPropertyListReader_vintage *self){
   if(self->_stackSize==0)
    return nil;

   self->_stackSize--;
   return self->_stack[self->_stackSize];
}

static inline id topObject(NSPropertyListReader_vintage *self){
   if(self->_stackSize==0)
    return nil;

   return self->_stack[self->_stackSize-1];
}

static inline void appendCharacter(NSPropertyListReader_vintage *self,uint8_t c){
   if(self->_bufferSize>=self->_bufferCapacity){
    self->_bufferCapacity*=2;
    self->_buffer=NSZoneRealloc(NULL,self->_buffer,self->_bufferCapacity*sizeof(unichar));
   }
   self->_buffer[self->_bufferSize++]=c;
}

static inline void appendByte(NSPropertyListReader_vintage *self,uint8_t c){
   if(self->_dataBufferSize>=self->_dataBufferCapacity){
    self->_dataBufferCapacity*=2;
    self->_dataBuffer=NSZoneRealloc(NULL,self->_dataBuffer,self->_dataBufferCapacity*sizeof(unichar));
   }
   self->_dataBuffer[self->_dataBufferSize++]=c;
}

-(NSObject *)propertyListWithInfo:(NSString *)info {
   enum {
    STATE_WHITESPACE,
    STATE_COMMENT_SLASH,
    STATE_COMMENT,
    STATE_COMMENT_STAR,
    STATE_COMMENT_DBL_SLASH,
    STATE_NAME,
    STATE_STRING,
    STATE_STRING_SLASH,
    STATE_STRING_SLASH_X00,
    STATE_STRING_SLASH_XX0,
    STATE_STRING_SLASH_U0000,
    STATE_STRING_SLASH_UX000,
    STATE_STRING_SLASH_UXX00,
    STATE_STRING_SLASH_UXXX0,
    STATE_DATA_HINIBBLE,
    STATE_DATA_LONIBBLE,
   } state=STATE_WHITESPACE;
   enum {
    EXPECT_KEY,
    EXPECT_EQUAL,
    EXPECT_VAL,
    EXPECT_SEPARATOR,
    EXPECT_EOF
   } expect=EXPECT_VAL;

   for(_index=0;_index<_length;){
    uint8_t code=_bytes[_index++];

    switch(state){

     case STATE_WHITESPACE:
      if(code<=' '){
       if(code=='\n')
        _lineNumber++;

       state=STATE_WHITESPACE;
      }
      else if(code=='/')
       state=STATE_COMMENT_SLASH;
      else if(code<128 && _NSPropertyListNameSet[code]){
       _bufferSize=0;
       appendCharacter(self,code);
       state=STATE_NAME;
      }
      else if(code=='\"'){
       if(expect!=EXPECT_KEY && expect!=EXPECT_VAL)
        return [self parseError:expect token:code info:info];

       _bufferSize=0;
       state=STATE_STRING;
      }
      else if(code=='<'){
       if(expect!=EXPECT_KEY && expect!=EXPECT_VAL)
        return [self parseError:expect token:code info:info];

       _dataBufferSize=0;
       state=STATE_DATA_HINIBBLE;
      }
      else if(code=='{'){
       if(expect!=EXPECT_VAL)
        return [self parseError:expect token:code info:info];

       pushObject(self,[[NSMutableDictionary allocWithZone:NULL] init]);
       expect=EXPECT_KEY;
      }
      else if(code=='='){
       if(expect!=EXPECT_EQUAL)
        return [self parseError:expect token:code info:info];
       expect=EXPECT_VAL;
      }
      else if(code==';'){
       NSMutableDictionary *dictionary;
       NSObject *key,*object;

       if(expect!=EXPECT_SEPARATOR)
        return [self parseError:expect token:code info:info];

       object=popObject(self);
       key=popObject(self);
       if(![key isKindOfClass:_stringClass]){
        [key release];
        [object release];
        return [self internalError:_stringClass];
       }

       dictionary=topObject(self);
       if(![dictionary isKindOfClass:_dictionaryClass]){
        [key release];
        [object release];
        return [self internalError:_dictionaryClass];
       }
       [dictionary setObject:object forKey:key];
       [key release];
       [object release];
       expect=EXPECT_KEY;
      }
      else if(code=='}'){
       if(expect!=EXPECT_KEY)
        return [self parseError:expect token:code info:info];

       if(![topObject(self) isKindOfClass:_dictionaryClass])
        return [self internalError:_dictionaryClass];

       expect=(_stackSize==1)?EXPECT_EOF:EXPECT_SEPARATOR;
      }
      else if(code=='('){
       if(expect!=EXPECT_VAL)
        return [self parseError:expect token:code info:info];

       pushObject(self,[[NSMutableArray allocWithZone:NULL] init]);
       expect=EXPECT_VAL;
      }
      else if(code==','){
       NSMutableArray *array;
       NSObject       *object;

       if(expect!=EXPECT_SEPARATOR)
        return [self parseError:expect token:code info:info];

       object=popObject(self);

       array=topObject(self);
       if(![array isKindOfClass:_arrayClass]){
        [object release];
        return [self internalError:_arrayClass];
       }

       [array addObject:object];
       [object release];
       expect=EXPECT_VAL;
      }
      else if(code==')'){
       NSMutableArray *array;
       NSObject       *object;

       if(expect!=EXPECT_VAL && expect!=EXPECT_SEPARATOR)
        return [self parseError:expect token:code info:info];

       if(expect==EXPECT_VAL)
        object=nil;
       else
        object=popObject(self);

       array=topObject(self);
       if(![array isKindOfClass:_arrayClass]){
        [object release];
        return [self internalError:_arrayClass];
       }

       if(object!=nil){
        [array addObject:object];
        [object release];
       }

       expect=(_stackSize==1)?EXPECT_EOF:EXPECT_SEPARATOR;
      }
      else
       return [self parseError:expect token:code info:info];
      break;

     case STATE_COMMENT_SLASH:
      if(code=='*')
       state=STATE_COMMENT;
      else if(code=='/')
       state=STATE_COMMENT_DBL_SLASH;
      else {
       _bufferSize=0;
       appendCharacter(self,'/');
       _index--;
       state=STATE_NAME;
      }
      break;

     case STATE_COMMENT:
      if(code=='*')
       state=STATE_COMMENT_STAR;
      break;

     case STATE_COMMENT_STAR:
      if(code=='/')
       state=STATE_WHITESPACE;
      else if(code!='*')
       state=STATE_COMMENT;
      break;

     case STATE_COMMENT_DBL_SLASH:
      if(code=='\n')
       state=STATE_WHITESPACE;
      break;

     case STATE_NAME:
      if(code<128 && _NSPropertyListNameSet[code])
       appendCharacter(self,code);
      else{
         NSString *string=[[NSString allocWithZone:NULL]
                           initWithCharacters:_buffer length:_bufferSize];
         pushObject(self,string);
         _index--;
       state=STATE_WHITESPACE;
       if(expect==EXPECT_KEY)
        expect=EXPECT_EQUAL;
       else
        expect=EXPECT_SEPARATOR;
      }
      break;

     case STATE_STRING:
      if(code=='\"'){
         
         NSString *string=[NSString stringWithCharacters:_buffer length:_bufferSize];
         pushObject(self,[string retain]);

       state=STATE_WHITESPACE;
       if(_stackSize==1)
        expect=EXPECT_EOF;
       else if(expect==EXPECT_KEY)
        expect=EXPECT_EQUAL;
       else
        expect=EXPECT_SEPARATOR;
      }
      else{
       if(code=='\\')
        state=STATE_STRING_SLASH;
       else
        appendCharacter(self,code);
      }
      break;

     case STATE_STRING_SLASH:
      switch(code){

       case 'a': appendCharacter(self,'\a'); state=STATE_STRING; break;
       case 'b': appendCharacter(self,'\b'); state=STATE_STRING; break;
       case 'f': appendCharacter(self,'\f'); state=STATE_STRING; break;
       case 'n': appendCharacter(self,'\n'); state=STATE_STRING; break;
       case 'r': appendCharacter(self,'\r'); state=STATE_STRING; break;
       case 't': appendCharacter(self,'\t'); state=STATE_STRING; break;
       case 'v': appendCharacter(self,'\v'); state=STATE_STRING; break;

       case '0': case '1': case '2': case '3':
       case '4': case '5': case '6': case '7':
        appendCharacter(self,code-'0');
        state=STATE_STRING_SLASH_X00;
        break;

       case 'U':
        appendCharacter(self,'\0');
        state=STATE_STRING_SLASH_U0000;
        break;

       default:
        appendCharacter(self,code);
        state=STATE_STRING; 
        break;
      }
      break;

     case STATE_STRING_SLASH_X00:
      if(code<'0' || code>'7'){
       state=STATE_STRING;
       _index--;
      }
      else{
       state=STATE_STRING_SLASH_XX0;
       _buffer[_bufferSize-1]*=8;
       _buffer[_bufferSize-1]+=code-'0';
      }
      break;

     case STATE_STRING_SLASH_XX0:
      state=STATE_STRING;
      if(code<'0' || code>'7')
       _index--;
      else{
       _buffer[_bufferSize-1]*=8;
       _buffer[_bufferSize-1]+=code-'0';
      }
      break;

     case STATE_STRING_SLASH_U0000:
     case STATE_STRING_SLASH_UX000:
     case STATE_STRING_SLASH_UXX00:
     case STATE_STRING_SLASH_UXXX0:
      if(code>='0' && code<='9'){
       _buffer[_bufferSize-1]*=16;
       _buffer[_bufferSize-1]+=code-'0';
       state=(state==STATE_STRING_SLASH_UXXX0)?STATE_STRING:state+1;
      }
      else if(code>='a' && code<='f'){
       _buffer[_bufferSize-1]*=16;
       _buffer[_bufferSize-1]+=(code-'a')+10;
       state=(state==STATE_STRING_SLASH_UXXX0)?STATE_STRING:state+1;
      }
      else if(code>='A' && code<='F'){
       _buffer[_bufferSize-1]*=16;
       _buffer[_bufferSize-1]+=(code-'A')+10;
       state=(state==STATE_STRING_SLASH_UXXX0)?STATE_STRING:state+1;
      }
      else{
       _index--;
       state=STATE_STRING;
      }
      break;

     case STATE_DATA_HINIBBLE:
     case STATE_DATA_LONIBBLE:
      if(code=='>'){

       NSData *data=[NSData dataWithBytes:_dataBuffer length:_dataBufferSize];
       pushObject(self,[data retain]);

       state=STATE_WHITESPACE;
       if(_stackSize==1)
        expect=EXPECT_EOF;
       else if(expect==EXPECT_KEY)
        expect=EXPECT_EQUAL;
       else
        expect=EXPECT_SEPARATOR;
    }
      else if(code<=' ')
       break;
      else if(code>='0' && code<='9'){
       uint8_t nibble=code-'0';
       
       if(state==STATE_DATA_HINIBBLE){
        appendByte(self,nibble<<4);
        state=STATE_DATA_LONIBBLE;
   }
       else {
        _dataBuffer[_dataBufferSize-1]|=nibble;
        state=STATE_DATA_HINIBBLE;
       }
      }
      else if(code>='A' && code<='F'){
       uint8_t nibble=(code-'A')+10;
   
       if(state==STATE_DATA_HINIBBLE){
        appendByte(self,nibble<<4);
        state=STATE_DATA_LONIBBLE;
       }
       else {
        _dataBuffer[_dataBufferSize-1]|=nibble;
        state=STATE_DATA_HINIBBLE;
       }
      }
      else if(code>='a' && code<='f'){
       uint8_t nibble=(code-'a')+10;
       
       if(state==STATE_DATA_HINIBBLE){
        appendByte(self,nibble<<4);
        state=STATE_DATA_LONIBBLE;
       }
       else {
        _dataBuffer[_dataBufferSize-1]|=nibble;
        state=STATE_DATA_HINIBBLE;
       }
      }
      
      break;

    }
   }
   
   if(state==STATE_NAME && _stackSize==0){
    NSString *result=[NSString stringWithCharacters:_buffer length:_bufferSize];

      return result;
   }

   if(state!=STATE_WHITESPACE)
    return [self parseError:expect token:-1 info:info];

   switch(expect){
    case EXPECT_EQUAL:
     return [self parseError:expect token:-1 info:info];

    case EXPECT_VAL:
     return [self parseError:expect token:-1 info:info];

    case EXPECT_SEPARATOR:
     return [self parseError:expect token:-1 info:info];

    default:
     break;
   }

   // FIX, make sure _stackSize is 1?
   return [popObject(self) autorelease];
}

+(NSObject *)propertyListFromData:(NSData *)data {
   NSPropertyListReader_vintage *reader=[[self alloc] initWithData:data];
   NSObject             *result=[[[reader propertyListWithInfo:nil] retain] autorelease];

   [reader release];

   return result;
}


@end
