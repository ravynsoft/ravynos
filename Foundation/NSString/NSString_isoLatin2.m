/* Copyright (c) 2012 Glenn Ganz
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString_isoLatin2.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

typedef struct
{
	const unsigned char		latin2;
	const unichar	unicode;
} CharMapping;

static CharMapping mapping_array[]=
{
    {(const unsigned char)0xA1,	(const unichar)0x0104}, //LATIN CAPITAL LETTER A WITH OGONEK
    {(const unsigned char)0xA2,	(const unichar)0x02D8}, //BREVE
    {(const unsigned char)0xA3,	(const unichar)0x0141}, //LATIN CAPITAL LETTER L WITH STROKE
    {(const unsigned char)0xA4,	(const unichar)0x00A4}, //CURRENCY SIGN
    {(const unsigned char)0xA5,	(const unichar)0x013D}, //LATIN CAPITAL LETTER L WITH CARON
    {(const unsigned char)0xA6,	(const unichar)0x015A}, //LATIN CAPITAL LETTER S WITH ACUTE
    {(const unsigned char)0xA7,	(const unichar)0x00A7}, //SECTION SIGN
    {(const unsigned char)0xA8,	(const unichar)0x00A8}, //DIAERESIS
    {(const unsigned char)0xA9,	(const unichar)0x0160}, //LATIN CAPITAL LETTER S WITH CARON
    {(const unsigned char)0xAA,	(const unichar)0x015E}, //LATIN CAPITAL LETTER S WITH CEDILLA
    {(const unsigned char)0xAB,	(const unichar)0x0164}, //LATIN CAPITAL LETTER T WITH CARON
    {(const unsigned char)0xAC,	(const unichar)0x0179}, //LATIN CAPITAL LETTER Z WITH ACUTE
    {(const unsigned char)0xAD,	(const unichar)0x00AD}, //SOFT HYPHEN
    {(const unsigned char)0xAE,	(const unichar)0x017D}, //LATIN CAPITAL LETTER Z WITH CARON
    {(const unsigned char)0xAF,	(const unichar)0x017B}, //LATIN CAPITAL LETTER Z WITH DOT ABOVE
    {(const unsigned char)0xB0,	(const unichar)0x00B0}, //DEGREE SIGN
    {(const unsigned char)0xB1,	(const unichar)0x0105}, //LATIN SMALL LETTER A WITH OGONEK
    {(const unsigned char)0xB2,	(const unichar)0x02DB}, //OGONEK
    {(const unsigned char)0xB3,	(const unichar)0x0142}, //LATIN SMALL LETTER L WITH STROKE
    {(const unsigned char)0xB4,	(const unichar)0x00B4}, //ACUTE ACCENT
    {(const unsigned char)0xB5,	(const unichar)0x013E}, //LATIN SMALL LETTER L WITH CARON
    {(const unsigned char)0xB6,	(const unichar)0x015B}, //LATIN SMALL LETTER S WITH ACUTE
    {(const unsigned char)0xB7,	(const unichar)0x02C7}, //CARON
    {(const unsigned char)0xB8,	(const unichar)0x00B8}, //CEDILLA
    {(const unsigned char)0xB9,	(const unichar)0x0161}, //LATIN SMALL LETTER S WITH CARON
    {(const unsigned char)0xBA,	(const unichar)0x015F}, //LATIN SMALL LETTER S WITH CEDILLA
    {(const unsigned char)0xBB,	(const unichar)0x0165}, //LATIN SMALL LETTER T WITH CARON
    {(const unsigned char)0xBC,	(const unichar)0x017A}, //LATIN SMALL LETTER Z WITH ACUTE
    {(const unsigned char)0xBD,	(const unichar)0x02DD}, //DOUBLE ACUTE ACCENT
    {(const unsigned char)0xBE,	(const unichar)0x017E}, //LATIN SMALL LETTER Z WITH CARON
    {(const unsigned char)0xBF,	(const unichar)0x017C}, //LATIN SMALL LETTER Z WITH DOT ABOVE
    {(const unsigned char)0xC0,	(const unichar)0x0154}, //LATIN CAPITAL LETTER R WITH ACUTE
    {(const unsigned char)0xC1,	(const unichar)0x00C1}, //LATIN CAPITAL LETTER A WITH ACUTE
    {(const unsigned char)0xC2,	(const unichar)0x00C2}, //LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    {(const unsigned char)0xC3,	(const unichar)0x0102}, //LATIN CAPITAL LETTER A WITH BREVE
    {(const unsigned char)0xC4,	(const unichar)0x00C4}, //LATIN CAPITAL LETTER A WITH DIAERESIS
    {(const unsigned char)0xC5,	(const unichar)0x0139}, //LATIN CAPITAL LETTER L WITH ACUTE
    {(const unsigned char)0xC6,	(const unichar)0x0106}, //LATIN CAPITAL LETTER C WITH ACUTE
    {(const unsigned char)0xC7,	(const unichar)0x00C7}, //LATIN CAPITAL LETTER C WITH CEDILLA
    {(const unsigned char)0xC8,	(const unichar)0x010C}, //LATIN CAPITAL LETTER C WITH CARON
    {(const unsigned char)0xC9,	(const unichar)0x00C9}, //LATIN CAPITAL LETTER E WITH ACUTE
    {(const unsigned char)0xCA,	(const unichar)0x0118}, //LATIN CAPITAL LETTER E WITH OGONEK
    {(const unsigned char)0xCB,	(const unichar)0x00CB}, //LATIN CAPITAL LETTER E WITH DIAERESIS
    {(const unsigned char)0xCC,	(const unichar)0x011A}, //LATIN CAPITAL LETTER E WITH CARON
    {(const unsigned char)0xCD,	(const unichar)0x00CD}, //LATIN CAPITAL LETTER I WITH ACUTE
    {(const unsigned char)0xCE,	(const unichar)0x00CE}, //LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    {(const unsigned char)0xCF,	(const unichar)0x010E}, //LATIN CAPITAL LETTER D WITH CARON
    {(const unsigned char)0xD0,	(const unichar)0x0110}, //LATIN CAPITAL LETTER D WITH STROKE
    {(const unsigned char)0xD1,	(const unichar)0x0143}, //LATIN CAPITAL LETTER N WITH ACUTE
    {(const unsigned char)0xD2,	(const unichar)0x0147}, //LATIN CAPITAL LETTER N WITH CARON
    {(const unsigned char)0xD3,	(const unichar)0x00D3}, //LATIN CAPITAL LETTER O WITH ACUTE
    {(const unsigned char)0xD4,	(const unichar)0x00D4}, //LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    {(const unsigned char)0xD5,	(const unichar)0x0150}, //LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
    {(const unsigned char)0xD6,	(const unichar)0x00D6}, //LATIN CAPITAL LETTER O WITH DIAERESIS
    {(const unsigned char)0xD7,	(const unichar)0x00D7}, //MULTIPLICATION SIGN
    {(const unsigned char)0xD8,	(const unichar)0x0158}, //LATIN CAPITAL LETTER R WITH CARON
    {(const unsigned char)0xD9,	(const unichar)0x016E}, //LATIN CAPITAL LETTER U WITH RING ABOVE
    {(const unsigned char)0xDA,	(const unichar)0x00DA}, //LATIN CAPITAL LETTER U WITH ACUTE
    {(const unsigned char)0xDB,	(const unichar)0x0170}, //LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
    {(const unsigned char)0xDC,	(const unichar)0x00DC}, //LATIN CAPITAL LETTER U WITH DIAERESIS
    {(const unsigned char)0xDD,	(const unichar)0x00DD}, //LATIN CAPITAL LETTER Y WITH ACUTE
    {(const unsigned char)0xDE,	(const unichar)0x0162}, //LATIN CAPITAL LETTER T WITH CEDILLA
    {(const unsigned char)0xDF,	(const unichar)0x00DF}, //LATIN SMALL LETTER SHARP S
    {(const unsigned char)0xE0,	(const unichar)0x0155}, //LATIN SMALL LETTER R WITH ACUTE
    {(const unsigned char)0xE1,	(const unichar)0x00E1}, //LATIN SMALL LETTER A WITH ACUTE
    {(const unsigned char)0xE2,	(const unichar)0x00E2}, //LATIN SMALL LETTER A WITH CIRCUMFLEX
    {(const unsigned char)0xE3,	(const unichar)0x0103}, //LATIN SMALL LETTER A WITH BREVE
    {(const unsigned char)0xE4,	(const unichar)0x00E4}, //LATIN SMALL LETTER A WITH DIAERESIS
    {(const unsigned char)0xE5,	(const unichar)0x013A}, //LATIN SMALL LETTER L WITH ACUTE
    {(const unsigned char)0xE6,	(const unichar)0x0107}, //LATIN SMALL LETTER C WITH ACUTE
    {(const unsigned char)0xE7,	(const unichar)0x00E7}, //LATIN SMALL LETTER C WITH CEDILLA
    {(const unsigned char)0xE8,	(const unichar)0x010D}, //LATIN SMALL LETTER C WITH CARON
    {(const unsigned char)0xE9,	(const unichar)0x00E9}, //LATIN SMALL LETTER E WITH ACUTE
    {(const unsigned char)0xEA,	(const unichar)0x0119}, //LATIN SMALL LETTER E WITH OGONEK
    {(const unsigned char)0xEB,	(const unichar)0x00EB}, //LATIN SMALL LETTER E WITH DIAERESIS
    {(const unsigned char)0xEC,	(const unichar)0x011B}, //LATIN SMALL LETTER E WITH CARON
    {(const unsigned char)0xED,	(const unichar)0x00ED}, //LATIN SMALL LETTER I WITH ACUTE
    {(const unsigned char)0xEE,	(const unichar)0x00EE}, //LATIN SMALL LETTER I WITH CIRCUMFLEX
    {(const unsigned char)0xEF,	(const unichar)0x010F}, //LATIN SMALL LETTER D WITH CARON
    {(const unsigned char)0xF0,	(const unichar)0x0111}, //LATIN SMALL LETTER D WITH STROKE
    {(const unsigned char)0xF1,	(const unichar)0x0144}, //LATIN SMALL LETTER N WITH ACUTE
    {(const unsigned char)0xF2,	(const unichar)0x0148}, //LATIN SMALL LETTER N WITH CARON
    {(const unsigned char)0xF3,	(const unichar)0x00F3}, //LATIN SMALL LETTER O WITH ACUTE
    {(const unsigned char)0xF4,	(const unichar)0x00F4}, //LATIN SMALL LETTER O WITH CIRCUMFLEX
    {(const unsigned char)0xF5,	(const unichar)0x0151}, //LATIN SMALL LETTER O WITH DOUBLE ACUTE
    {(const unsigned char)0xF6,	(const unichar)0x00F6}, //LATIN SMALL LETTER O WITH DIAERESIS
    {(const unsigned char)0xF7,	(const unichar)0x00F7}, //DIVISION SIGN
    {(const unsigned char)0xF8,	(const unichar)0x0159}, //LATIN SMALL LETTER R WITH CARON
    {(const unsigned char)0xF9,	(const unichar)0x016F}, //LATIN SMALL LETTER U WITH RING ABOVE
    {(const unsigned char)0xFA,	(const unichar)0x00FA}, //LATIN SMALL LETTER U WITH ACUTE
    {(const unsigned char)0xFB,	(const unichar)0x0171}, //LATIN SMALL LETTER U WITH DOUBLE ACUTE
    {(const unsigned char)0xFC,	(const unichar)0x00FC}, //LATIN SMALL LETTER U WITH DIAERESIS
    {(const unsigned char)0xFD,	(const unichar)0x00FD}, //LATIN SMALL LETTER Y WITH ACUTE
    {(const unsigned char)0xFE,	(const unichar)0x0163}, //LATIN SMALL LETTER T WITH CEDILLA
    {(const unsigned char)0xFF,	(const unichar)0x02D9} //DOT ABOVE
};



const unichar _mapISOLatin2ToUnichar(const unsigned char c)
{
	if (c>= 0xA1) {
        return mapping_array[c - 0xA1].unicode;
	}
    
    return c;
}
unichar *NSISOLatin2ToUnicode(const char *cString,NSUInteger length,
                            NSUInteger *resultLength,NSZone *zone) {
	unichar *characters=NSZoneMalloc(zone,sizeof(unichar)*length);
	int      i;
    
	for(i=0;i<length;i++)
	{
		characters[i]=_mapISOLatin2ToUnichar(cString[i]);
    }
    
	*resultLength=i;
	return characters;
}

char *NSUnicodeToISOLatin2(const unichar *characters,NSUInteger length,
                         BOOL lossy,NSUInteger *resultLength,NSZone *zone,BOOL zeroTerminate) {
	char *isolatin2=NSZoneMalloc(zone,sizeof(char)*(length + (zeroTerminate == YES ? 1 : 0)));
	int   i;
    
	for(i=0;i<length;i++){
        
		if(characters[i] < 0xA1)
			isolatin2[i]=characters[i];
		else
		{
            
			static int size = sizeof(mapping_array) / sizeof(mapping_array[0]);
			int j = 0;
			BOOL found = NO;
            
			for(;j < size;j++)
			{
				if(mapping_array[j].unicode == characters[i])
				{
					isolatin2[i]=mapping_array[j].latin2;
					found = YES;
					break;
				}
			}
			if(!found)
			{
				if(lossy)
					isolatin2[i]='\0';
				else
				{
					NSZoneFree(zone,isolatin2);
					return NULL;
				}
			}
		}
	}
	if(zeroTerminate == YES) {
        isolatin2[i++]='\0';
    }
	*resultLength=i;
    
	return isolatin2;
}

NSString *NSISOLatin2CStringNewWithCharacters(NSZone *zone,
                                            const unichar *characters,NSUInteger length,BOOL lossy) {
    NSString *string;
    NSUInteger  bytesLength;
    char     *bytes;
    
    bytes=NSUnicodeToISOLatin2(characters,length,lossy,&bytesLength,zone,NO);
    
    if(bytes==NULL)
        string=nil;
    else{
        string=NSString_isoLatin2NewWithBytes(zone,bytes,bytesLength);
        NSZoneFree(zone,bytes);
    }
    
    return string;
}

NSUInteger NSGetISOLatin2CStringWithMaxLength(const unichar *characters,NSUInteger length,NSUInteger *location,char *cString,NSUInteger maxLength,BOOL lossy) {
    NSUInteger i,result=0;
    
    
    if(length+1 > maxLength) {
        cString[0]='\0';
        return NSNotFound;
    }
    for(i=0;i<length && result<=maxLength;i++){
        const unichar code=characters[i];
        
        if(code < 0xA1)
            cString[result++]=code;
        else {
            unsigned char j;
            BOOL found = NO;
            for(j=0xA0;j<0xFF;j++) {
                if(code==_mapISOLatin2ToUnichar(j+1)) {
                    found = YES;
                    break;
                }
            }            

            if(found == YES)
                cString[result++]=j+1;
            else if(lossy)
                cString[result++]='\0';
            else {
                return NSNotFound;
            }
        }
    }
    
    cString[result]='\0';
    
    *location=i;
    
    return result;
}

@implementation NSString_isoLatin2

NSString *NSString_isoLatin2NewWithBytes(NSZone *zone,
                                       const char *bytes,NSUInteger length) {
	NSString_isoLatin2 *string;
	int                i;
    
	string=NSAllocateObject([NSString_isoLatin2 class],length*sizeof(char),zone);
    
	string->_length=length;
	for(i=0;i<length;i++) {
        unsigned char c = ((uint8_t *)bytes)[i];
		string->_bytes[i]=c;
    }
	string->_bytes[i]='\0';
    
	return string;
}

-(NSUInteger)length {
	return _length;
}

-(unichar)characterAtIndex:(NSUInteger)location {
	if(location>=_length){
		NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
						 location,[self length]);
	}
    
	return _mapISOLatin2ToUnichar(_bytes[location]);
}

-(void)getCharacters:(unichar *)buffer {
	int i;
    
	for(i=0;i<_length;i++)
		buffer[i]=_mapISOLatin2ToUnichar(_bytes[i]);
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
	NSInteger i,loc=range.location,len=range.length;
    
	if(NSMaxRange(range)>_length){
		NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
						 NSStringFromRange(range),[self length]);
	}
    
	for(i=0;i<len;i++)
		buffer[i]=_mapISOLatin2ToUnichar(_bytes[loc+i]);
}

@end
