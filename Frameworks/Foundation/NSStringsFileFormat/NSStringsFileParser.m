/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSStringsFileParser.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSString_cString.h>

static inline unsigned short SwapWord(unsigned short w){
   unsigned short hi=w>>8;
   unsigned short lo=w&0xFF;
   
   return (lo<<8)|hi;
}

static inline unsigned short PickWord(unsigned short w){
 return w;
}

static NSArray *error(NSArray *array,unichar *buffer,NSString *fmt,...) {
   va_list list;
   va_start(list,fmt);

   [array release];
   if(buffer!=NULL)
    NSZoneFree(NSZoneFromPointer(buffer),buffer);

   NSLogv(fmt,list);
   va_end(list);
   
   return nil;
}

static NSArray *stringListFromBytes(const unichar unicode[],NSInteger length){
   NSMutableArray *array=[[NSMutableArray allocWithZone:NULL] initWithCapacity:1024];
   NSInteger  index;
   NSUInteger bufferCount=0,bufferCapacity=2048;
   unichar   *buffer=NSZoneMalloc(NSDefaultMallocZone(),bufferCapacity*sizeof(unichar));
   
   enum {
    STATE_WHITESPACE,
    STATE_COMMENT_SLASH,
    STATE_COMMENT,
    STATE_COMMENT_STAR,
    STATE_STRING,
    STATE_STRING_KEY,
    STATE_STRING_SLASH,
    STATE_STRING_SLASH_X00,
    STATE_STRING_SLASH_XX0
   } state=STATE_WHITESPACE;
   enum {
    EXPECT_KEY,
    EXPECT_EQUAL_SEMI,
    EXPECT_VAL,
    EXPECT_SEMI
   } expect=EXPECT_KEY;

   unichar (*mapUC)(unichar);
   if (unicode[0]==0xFFFE){
    // reverse endianness
    mapUC=SwapWord;
    index=1;
   }
   else if (unicode[0]==0xFEFF){
    // native endianness
    mapUC=PickWord;
    index=1;
   }
   else{
    // no BOM, assume native endianness
    mapUC=PickWord;
    index=0;
   }
	
	// Remove the linefeed at the end of the file (if there is one)
	// Not sure what the length >>= 1 is doing for us - length is the right number
   if(mapUC(unicode[(length>>=1)-1])==0x0A)
		length--;
	
	// Now iterate over the unichar words skipping the endianness marker word if necessary)
   for(;index<length;index++){
	   
	   // Get the unichar in native format
    unichar code=mapUC(unicode[index]);
         
    switch(state){

     case STATE_WHITESPACE:
			// We're looking for anything non-whitespace
      if(code=='/')
		  // Found what looks like the start of a comment; a '*" should come next
       state=STATE_COMMENT_SLASH;
			// An '=' in the middle of whitespace is only valid if we're looking for the value after finding a key
      else if(code=='='){
       if(expect==EXPECT_EQUAL_SEMI)
        expect=EXPECT_VAL;
       else
        return error(array,buffer,@"unexpected character %02X '%C' at %d",code,code,index);
      }
      else if(code==';'){
		  // A semi-colon means we're at the end of a key value pair so start expecting a new key
       if(expect==EXPECT_SEMI)
        expect=EXPECT_KEY;
       else if(expect==EXPECT_EQUAL_SEMI){
		   // Special case where the value is the same as the key (and thus not present in the file)
		   // not sure if this is a bodge or what but we just add the key again as the value
		   // And start expecting a new key
        expect=EXPECT_KEY;
        [array addObject:[array lastObject]];
       }
       else
        return error(array,buffer,@"unexpected character %02X '%C' at %d",code,code,index);
      }
      else if(code=='\"'){
		  // A quote by itself that's not starting a key or value is a big no no
       if(expect!=EXPECT_KEY && expect!=EXPECT_VAL)
        return error(array,buffer,@"unexpected character %02X '%C' at %d",code,code,index);

		  // Start looking for string within the quotes
       bufferCount=0;
       state=STATE_STRING;
      }
      else if(code>' '){
		  // Not sure what non-white space really should mean - but we're interpreting as a STATE_STRING_KEY
       if(expect!=EXPECT_KEY)
        return error(array,buffer,@"unexpected character %02X '%C' at %d",code,code,index);

       buffer[0]=code;
       bufferCount=1;
       state=STATE_STRING_KEY;
      }
      break;

     case STATE_COMMENT_SLASH:
      if(code=='*')
		  // Looks like we've found a comment
       state=STATE_COMMENT;
      else
       return error(array,buffer,@"unexpected character %02X '%C',after /",code,code);
      break;

     case STATE_COMMENT:
      if(code=='*')
		  // Perhaps we're hitting the end of the comment?
       state=STATE_COMMENT_STAR;
      break;

     case STATE_COMMENT_STAR:
      if(code=='/')
		  // Yep we're at the end - switch back to looking at whitespace
       state=STATE_WHITESPACE;
      else if(code!='*')
		  // I guess we're not there yet
       state=STATE_COMMENT;
      break;

     case STATE_STRING_KEY:
      switch(code){
			  // I guess a '"' is not valid in this special state?
       case '\"':
        return error(array,buffer,@"unexpected character %02X '%C' at %d",code,code,index);
       case '=':
			  // Uh-oh we're going backwards now...
         index-=2;
       case ' ':
			  // And now we're diddling the code - STATE_STRING_KEY is very magic!
         code='\"';
      }
     case STATE_STRING:
      if(code=='\"'){
		  // We've found a key or value
       NSString *string=[[NSString allocWithZone:NULL] initWithCharacters:buffer length:bufferCount];

		  // So save it off
       [array addObject:string];
       [string release];
		  // Switch back to looking at whitespace
       state=STATE_WHITESPACE;
		  
       if(expect==EXPECT_KEY)
		   // If we found a key then we're looking for "=" or ";"
        expect=EXPECT_EQUAL_SEMI;
       else
		   // Else we found a value so look for a ";"
        expect=EXPECT_SEMI;
      }
      else{
		  // accumulate the unichars of the string in a buffer
       if(bufferCount>=bufferCapacity){
        bufferCapacity*=2;
        buffer=NSZoneRealloc(NSZoneFromPointer(buffer),buffer,bufferCapacity*sizeof(unichar));
       }
       if(code=='\\')
		   // Apparently escaped chars can be embedded in the string so look for that
        state=STATE_STRING_SLASH;
       else 
        buffer[bufferCount++]=code;
      }
      break;

     case STATE_STRING_SLASH:
      switch(code){
			  // Handle the escaped char in the string
       case 'a': buffer[bufferCount++]='\a'; state=STATE_STRING; break;
       case 'b': buffer[bufferCount++]='\b'; state=STATE_STRING; break;
       case 'f': buffer[bufferCount++]='\f'; state=STATE_STRING; break;
       case 'n': buffer[bufferCount++]='\n'; state=STATE_STRING; break;
       case 'r': buffer[bufferCount++]='\r'; state=STATE_STRING; break;
       case 't': buffer[bufferCount++]='\t'; state=STATE_STRING; break;
       case 'v': buffer[bufferCount++]='\v'; state=STATE_STRING; break;
       case '0': case '1': case '2': case '3':
       case '4': case '5': case '6': case '7':
        buffer[bufferCount++]=code-'0';
        state=STATE_STRING_SLASH_X00;
        break;

       default:
        buffer[bufferCount++]=code;
        state=STATE_STRING; 
        break;
      }
      break;

     case STATE_STRING_SLASH_X00:
      if(code<'0' || code>'7'){
       state=STATE_STRING;
       index--;
      }
      else{
       state=STATE_STRING_SLASH_XX0;
       buffer[bufferCount-1]*=8;
       buffer[bufferCount-1]+=code-'0';
      }
      break;

     case STATE_STRING_SLASH_XX0:
      state=STATE_STRING;
      if(code<'0' || code>'7')
       index--;
      else{
       buffer[bufferCount-1]*=8;
       buffer[bufferCount-1]+=code-'0';
      }
      break;

    }
   }

   NSZoneFree(NSZoneFromPointer(buffer),buffer);

	// We better not be in the middle of parsing something important when we ran out of chars!
   if(state!=STATE_WHITESPACE)
    return error(array,NULL,@"unexpected EOF\n");

	// or expecting something important either
   switch(expect){
    case EXPECT_EQUAL_SEMI:
     return error(array,NULL,@"unexpected EOF, expecting = or ;");

    case EXPECT_VAL:
     return error(array,NULL,@"unexpected EOF, expecting value");

    case EXPECT_SEMI:
     return error(array,NULL,@"unexpected EOF, expecting ;");

    default:
     break;
   }

   return array;
}

NSDictionary *NSDictionaryFromStringsFormatData(NSData *data) {
   NSArray      *array=stringListFromBytes((unichar *)[data bytes],[data length]);
   NSDictionary *dictionary;
   id           *keys,*values;
   NSInteger           i,count;

   if(array==nil)
    return nil;

   count=[array count]/2;

   keys=__builtin_alloca(sizeof(id)*count);
   values=__builtin_alloca(sizeof(id)*count);

   for(i=0;i<count;i++){
    keys[i]=[array objectAtIndex:i*2];
    values[i]=[array objectAtIndex:i*2+1];
   }

   dictionary=[[[NSDictionary allocWithZone:NULL] initWithObjects:values
      forKeys:keys count:count] autorelease];

   [array release];

   return dictionary;
}

NSDictionary *NSDictionaryFromStringsFormatString(NSString *string) {
   NSData *data=[string dataUsingEncoding:NSUnicodeStringEncoding];
   return NSDictionaryFromStringsFormatData(data);
}

NSDictionary *NSDictionaryFromStringsFormatFile(NSString *path) {
   NSData       *data;
   NSDictionary *dictionary;

   if((data=[[NSData allocWithZone:NULL] initWithContentsOfMappedFile:path])==nil)
    return nil;

   dictionary=NSDictionaryFromStringsFormatData(data);

   [data release];

   return dictionary;
}
