/* Copyright (c) 2009 Glenn Ganz

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString_macOSRoman.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

typedef struct
{
    const unsigned char	macOSRoman;
    const unichar       unicode;
} CharMapping;

static CharMapping mapping_array[]=
{
{(const unsigned char)0x80,	(const unichar)0x00C4},
{(const unsigned char)0x81,	(const unichar)0x00C5},
{(const unsigned char)0x82,	(const unichar)0x00C7},
{(const unsigned char)0x83,	(const unichar)0x00C9},
{(const unsigned char)0x84,	(const unichar)0x00D1},
{(const unsigned char)0x85,	(const unichar)0x00D6},
{(const unsigned char)0x86,	(const unichar)0x00DC},
{(const unsigned char)0x87,	(const unichar)0x00E1},
{(const unsigned char)0x88,	(const unichar)0x00E0},
{(const unsigned char)0x89,	(const unichar)0x00E2},
{(const unsigned char)0x8A,	(const unichar)0x00E4},
{(const unsigned char)0x8B,	(const unichar)0x00E3},
{(const unsigned char)0x8C,	(const unichar)0x00E5},
{(const unsigned char)0x8D,	(const unichar)0x00E7},
{(const unsigned char)0x8E,	(const unichar)0x00E9},
{(const unsigned char)0x8F,	(const unichar)0x00E8},
{(const unsigned char)0x90,	(const unichar)0x00EA},
{(const unsigned char)0x91,	(const unichar)0x00EB},
{(const unsigned char)0x92,	(const unichar)0x00ED},
{(const unsigned char)0x93,	(const unichar)0x00EC},
{(const unsigned char)0x94,	(const unichar)0x00EE},
{(const unsigned char)0x95,	(const unichar)0x00EF},
{(const unsigned char)0x96,	(const unichar)0x00F1},
{(const unsigned char)0x97,	(const unichar)0x00F3},
{(const unsigned char)0x98,	(const unichar)0x00F2},
{(const unsigned char)0x99,	(const unichar)0x00F4},
{(const unsigned char)0x9A,	(const unichar)0x00F6},
{(const unsigned char)0x9B,	(const unichar)0x00F5},
{(const unsigned char)0x9C,	(const unichar)0x00FA},
{(const unsigned char)0x9D,	(const unichar)0x00F9},
{(const unsigned char)0x9E,	(const unichar)0x00FB},
{(const unsigned char)0x9F,	(const unichar)0x00FC},
{(const unsigned char)0xA0,	(const unichar)0x2020},
{(const unsigned char)0xA1,	(const unichar)0x00B0},
{(const unsigned char)0xA2,	(const unichar)0x00A2},
{(const unsigned char)0xA3,	(const unichar)0x00A3},
{(const unsigned char)0xA4,	(const unichar)0x00A7},
{(const unsigned char)0xA5,	(const unichar)0x2022},
{(const unsigned char)0xA6,	(const unichar)0x00B6},
{(const unsigned char)0xA7,	(const unichar)0x00DF},
{(const unsigned char)0xA8,	(const unichar)0x00AE},
{(const unsigned char)0xA9,	(const unichar)0x00A9},
{(const unsigned char)0xAA,	(const unichar)0x2122},
{(const unsigned char)0xAB,	(const unichar)0x00B4},
{(const unsigned char)0xAC,	(const unichar)0x00A8},
{(const unsigned char)0xAD,	(const unichar)0x2260},
{(const unsigned char)0xAE,	(const unichar)0x00C6},
{(const unsigned char)0xAF,	(const unichar)0x00D8},
{(const unsigned char)0xB0,	(const unichar)0x221E},
{(const unsigned char)0xB1,	(const unichar)0x00B1},
{(const unsigned char)0xB2,	(const unichar)0x2264},
{(const unsigned char)0xB3,	(const unichar)0x2265},
{(const unsigned char)0xB4,	(const unichar)0x00A5},
{(const unsigned char)0xB5,	(const unichar)0x00B5},
{(const unsigned char)0xB6,	(const unichar)0x2202},
{(const unsigned char)0xB7,	(const unichar)0x2211},
{(const unsigned char)0xB8,	(const unichar)0x220F},
{(const unsigned char)0xB9,	(const unichar)0x03C0},
{(const unsigned char)0xBA,	(const unichar)0x222B},
{(const unsigned char)0xBB,	(const unichar)0x00AA},
{(const unsigned char)0xBC,	(const unichar)0x00BA},
{(const unsigned char)0xBD,	(const unichar)0x03A9},
{(const unsigned char)0xBE,	(const unichar)0x00E6},
{(const unsigned char)0xBF,	(const unichar)0x00F8},
{(const unsigned char)0xC0,	(const unichar)0x00BF},
{(const unsigned char)0xC1,	(const unichar)0x00A1},
{(const unsigned char)0xC2,	(const unichar)0x00AC},
{(const unsigned char)0xC3,	(const unichar)0x221A},
{(const unsigned char)0xC4,	(const unichar)0x0192},
{(const unsigned char)0xC5,	(const unichar)0x2248},
{(const unsigned char)0xC6,	(const unichar)0x2206},
{(const unsigned char)0xC7,	(const unichar)0x00AB},
{(const unsigned char)0xC8,	(const unichar)0x00BB},
{(const unsigned char)0xC9,	(const unichar)0x2026},
{(const unsigned char)0xCA,	(const unichar)0x00A0},
{(const unsigned char)0xCB,	(const unichar)0x00C0},
{(const unsigned char)0xCC,	(const unichar)0x00C3},
{(const unsigned char)0xCD,	(const unichar)0x00D5},
{(const unsigned char)0xCE,	(const unichar)0x0152},
{(const unsigned char)0xCF,	(const unichar)0x0153},
{(const unsigned char)0xD0,	(const unichar)0x2013},
{(const unsigned char)0xD1,	(const unichar)0x2014},
{(const unsigned char)0xD2,	(const unichar)0x201C},
{(const unsigned char)0xD3,	(const unichar)0x201D},
{(const unsigned char)0xD4,	(const unichar)0x2018},
{(const unsigned char)0xD5,	(const unichar)0x2019},
{(const unsigned char)0xD6,	(const unichar)0x00F7},
{(const unsigned char)0xD7,	(const unichar)0x25CA},
{(const unsigned char)0xD8,	(const unichar)0x00FF},
{(const unsigned char)0xD9,	(const unichar)0x0178},
{(const unsigned char)0xDA,	(const unichar)0x2044},
{(const unsigned char)0xDB,	(const unichar)0x20AC},
{(const unsigned char)0xDC,	(const unichar)0x2039},
{(const unsigned char)0xDD,	(const unichar)0x203A},
{(const unsigned char)0xDE,	(const unichar)0xFB01},
{(const unsigned char)0xDF,	(const unichar)0xFB02},
{(const unsigned char)0xE0,	(const unichar)0x2021},
{(const unsigned char)0xE1,	(const unichar)0x00B7},
{(const unsigned char)0xE2,	(const unichar)0x201A},
{(const unsigned char)0xE3,	(const unichar)0x201E},
{(const unsigned char)0xE4,	(const unichar)0x2030},
{(const unsigned char)0xE5,	(const unichar)0x00C2},
{(const unsigned char)0xE6,	(const unichar)0x00CA},
{(const unsigned char)0xE7,	(const unichar)0x00C1},
{(const unsigned char)0xE8,	(const unichar)0x00CB},
{(const unsigned char)0xE9,	(const unichar)0x00C8},
{(const unsigned char)0xEA,	(const unichar)0x00CD},
{(const unsigned char)0xEB,	(const unichar)0x00CE},
{(const unsigned char)0xEC,	(const unichar)0x00CF},
{(const unsigned char)0xED,	(const unichar)0x00CC},
{(const unsigned char)0xEE,	(const unichar)0x00D3},
{(const unsigned char)0xEF,	(const unichar)0x00D4},
{(const unsigned char)0xF0,	(const unichar)0xF8FF},
{(const unsigned char)0xF1,	(const unichar)0x00D2},
{(const unsigned char)0xF2,	(const unichar)0x00DA},
{(const unsigned char)0xF3,	(const unichar)0x00DB},
{(const unsigned char)0xF4,	(const unichar)0x00D9},
{(const unsigned char)0xF5,	(const unichar)0x0131},
{(const unsigned char)0xF6,	(const unichar)0x02C6},
{(const unsigned char)0xF7,	(const unichar)0x02DC},
{(const unsigned char)0xF8,	(const unichar)0x00AF},
{(const unsigned char)0xF9,	(const unichar)0x02D8},
{(const unsigned char)0xFA,	(const unichar)0x02D9},
{(const unsigned char)0xFB,	(const unichar)0x02DA},
{(const unsigned char)0xFC,	(const unichar)0x00B8},
{(const unsigned char)0xFD,	(const unichar)0x02DD},
{(const unsigned char)0xFE,	(const unichar)0x02DB},
{(const unsigned char)0xFF,	(const unichar)0x02C7}};


const unichar _mapMacOSRomanToUnichar(const unsigned char c)
{
	if(c>= 0x80)
	{
        return mapping_array[c - 0x80].unicode;
	}

   return c;
}
unichar *NSMacOSRomanToUnicode(const char *cString,NSUInteger length,
                            NSUInteger *resultLength,NSZone *zone) {
	unichar *characters=NSZoneMalloc(zone,sizeof(unichar)*length);
	int      i;

	for(i=0;i<length;i++)
	{
		characters[i]=_mapMacOSRomanToUnichar(cString[i]);
	}

	*resultLength=i;
	return characters;
}

char *NSUnicodeToMacOSRoman(const unichar *characters,NSUInteger length,
                         BOOL lossy,NSUInteger *resultLength,NSZone *zone,BOOL zeroTerminate) {
	char *macOSRoman=NSZoneMalloc(zone,sizeof(char)*(length + (zeroTerminate == YES ? 1 : 0)));
	int   i;

	for(i=0;i<length;i++){

		if(characters[i]<0x80)
			macOSRoman[i]=characters[i];
		else
		{

			static int size = sizeof(mapping_array) / sizeof(mapping_array[0]);
			int j = 0;
			BOOL found = NO;

			for(;j < size;j++)
			{
				if(mapping_array[j].unicode == characters[i])
				{
					macOSRoman[i]=mapping_array[j].macOSRoman;
					found = YES;
					break;
				}
			}
			if(!found)
			{
				if(lossy)
					macOSRoman[i]='\0';
				else
				{
					NSZoneFree(zone,macOSRoman);
					return NULL;
				}
			}
		}
	}
    if(zeroTerminate == YES) {
        macOSRoman[i++]='\0';
    }
	*resultLength=i;

	return macOSRoman;
}

NSString *NSMacOSRomanCStringNewWithCharacters(NSZone *zone,
                                            const unichar *characters,NSUInteger length,BOOL lossy) {
    NSString *string;
    NSUInteger  bytesLength;
    char     *bytes;

    bytes=NSUnicodeToMacOSRoman(characters,length,lossy,&bytesLength,zone, NO);

    if(bytes==NULL)
        string=nil;
    else{
        string=NSString_macOSRomanNewWithBytes(zone,bytes,bytesLength);
        NSZoneFree(zone,bytes);
    }

    return string;
}

NSUInteger NSGetMacOSRomanCStringWithMaxLength(const unichar *characters,NSUInteger length,
                                               NSUInteger *location,char *cString,NSUInteger maxLength,BOOL lossy)
{
    NSUInteger i,result=0;


    if(length+1 > maxLength) {
        cString[0]='\0';
        return NSNotFound;
    }
    for(i=0;i<length && result<=maxLength;i++){
        const unichar code=characters[i];

        if(code<0x80)
            cString[result++]=code;
        else {
            unsigned int j;

            for(j=0x80;j<=0xFF;j++)
                if(code==_mapMacOSRomanToUnichar(j))
                    break;

            if(j<=0xFF)
                cString[result++]=j;
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

@implementation NSString_macOSRoman

NSString *NSString_macOSRomanNewWithBytes(NSZone *zone,
                                       const char *bytes,NSUInteger length) {
	NSString_macOSRoman *self=NSAllocateObject([NSString_macOSRoman class],length*sizeof(char),zone);
    
    if (self) {
        self->_length=length;
        int i;
        for(i=0;i<length;i++)
            self->_bytes[i]=((uint8_t *)bytes)[i];
        self->_bytes[i]='\0';
    }
	return self;
}

-(NSUInteger)length {
	return _length;
}

-(unichar)characterAtIndex:(NSUInteger)location {
	if(location>=_length){
		NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
						 location,[self length]);
	}

	return _mapMacOSRomanToUnichar(_bytes[location]);
}

-(void)getCharacters:(unichar *)buffer {
	int i;

	for(i=0;i<_length;i++)
		buffer[i]=_mapMacOSRomanToUnichar(_bytes[i]);
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
	NSInteger i,loc=range.location,len=range.length;

	if(NSMaxRange(range)>_length){
		NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
						 NSStringFromRange(range),[self length]);
	}

	for(i=0;i<len;i++)
		buffer[i]=_mapMacOSRomanToUnichar(_bytes[loc+i]);
}

@end
