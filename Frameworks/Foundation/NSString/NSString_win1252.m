/* Copyright (c) 2009 Glenn Ganz

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString_win1252.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>


#define UNDEFINED_UNICODE 0xFFFD

typedef struct
{
	const unsigned char		win1252;
	const unichar	unicode;
} CharMapping;

static CharMapping mapping_array[]=
{
{(const unsigned char)0x80,	(const unichar)0x20AC},
{(const unsigned char)0x81,	(const unichar)UNDEFINED_UNICODE},
{(const unsigned char)0x82,	(const unichar)0x201A},
{(const unsigned char)0x83,	(const unichar)0x0192},
{(const unsigned char)0x84,	(const unichar)0x201E},
{(const unsigned char)0x85,	(const unichar)0x2026},
{(const unsigned char)0x86,	(const unichar)0x2020},
{(const unsigned char)0x87,	(const unichar)0x2021},
{(const unsigned char)0x88,	(const unichar)0x02C6},
{(const unsigned char)0x89,	(const unichar)0x2030},
{(const unsigned char)0x8A,	(const unichar)0x0160},
{(const unsigned char)0x8B,	(const unichar)0x2039},
{(const unsigned char)0x8C,	(const unichar)0x0152},
{(const unsigned char)0x8D,	(const unichar)UNDEFINED_UNICODE},
{(const unsigned char)0x8E,	(const unichar)0x017D},
{(const unsigned char)0x8F,	(const unichar)UNDEFINED_UNICODE},
{(const unsigned char)0x90,	(const unichar)UNDEFINED_UNICODE},
{(const unsigned char)0x91,	(const unichar)0x2018},
{(const unsigned char)0x92,	(const unichar)0x2019},
{(const unsigned char)0x93,	(const unichar)0x201C},
{(const unsigned char)0x94,	(const unichar)0x201D},
{(const unsigned char)0x95,	(const unichar)0x2022},
{(const unsigned char)0x96,	(const unichar)0x2013},
{(const unsigned char)0x97,	(const unichar)0x2014},
{(const unsigned char)0x98,	(const unichar)0x02DC},
{(const unsigned char)0x99,	(const unichar)0x2122},
{(const unsigned char)0x9A,	(const unichar)0x0161},
{(const unsigned char)0x9B,	(const unichar)0x203A},
{(const unsigned char)0x9C,	(const unichar)0x0153},
{(const unsigned char)0x9D,	(const unichar)UNDEFINED_UNICODE},
{(const unsigned char)0x9E,	(const unichar)0x017E},
{(const unsigned char)0x9F,	(const unichar)0x0178}
};


const unichar _mapWin1252ToUnichar(const unsigned char c)
{
	if (c>= 0x80 && c<=0x9F) {
        return mapping_array[c - 0x80].unicode;
	}

   return c;
}
unichar *NSWin1252ToUnicode(const char *cString,NSUInteger length,
							  NSUInteger *resultLength,NSZone *zone) {
	unichar *characters=NSZoneMalloc(zone,sizeof(unichar)*length);
	int      i;

	for(i=0;i<length;i++)
	{
		characters[i]=_mapWin1252ToUnichar(cString[i]);
    }

	*resultLength=i;
	return characters;
}

char *NSUnicodeToWin1252(const unichar *characters,NSUInteger length,
						   BOOL lossy,NSUInteger *resultLength,NSZone *zone,BOOL zeroTerminate) {
	char *win1252=NSZoneMalloc(zone,sizeof(char)*(length + (zeroTerminate == YES ? 1 : 0)));
	int   i;

	for(i=0;i<length;i++){

		if(characters[i] <= 256 && !(characters[i] >= 0x80 && characters[i] <= 0x9F))
			win1252[i]=characters[i];
		else
		{

			static int size = sizeof(mapping_array) / sizeof(mapping_array[0]);
			int j = 0;
			BOOL found = NO;

			for(;j < size;j++)
			{
				if(mapping_array[j].unicode == characters[i] && characters[i] != UNDEFINED_UNICODE)
				{
					win1252[i]=mapping_array[j].win1252;
					found = YES;
					break;
				}
			}
			if(!found)
			{
				if(lossy)
					win1252[i]='\0';
				else
				{
					NSZoneFree(zone,win1252);
					return NULL;
				}
			}
		}
	}
	if(zeroTerminate == YES) {
        win1252[i++]='\0';
    }
	*resultLength=i;

	return win1252;
}

NSString *NSWin1252CStringNewWithCharacters(NSZone *zone,
                                             const unichar *characters,NSUInteger length,BOOL lossy) {
    NSString *string;
    NSUInteger  bytesLength;
    char     *bytes;

    bytes=NSUnicodeToWin1252(characters,length,lossy,&bytesLength,zone,NO);

    if(bytes==NULL)
        string=nil;
    else{
        string=NSString_win1252NewWithBytes(zone,bytes,bytesLength);
        NSZoneFree(zone,bytes);
    }

    return string;
}

NSUInteger NSGetWin1252CStringWithMaxLength(const unichar *characters,NSUInteger length,NSUInteger *location,char *cString,NSUInteger maxLength,BOOL lossy) {
    NSUInteger i,result=0;


    if(length+1 > maxLength) {
        cString[0]='\0';
        return NSNotFound;
    }
    for(i=0;i<length && result<=maxLength;i++){
        const unichar code=characters[i];

        if(code <= 256 && !(code >= 0x80 && code <= 0x9F))
            cString[result++]=code;
        else {
            unsigned char j;

            for(j=0x80;j<=0x9F;j++)
                if(code==_mapWin1252ToUnichar(j))
                    break;

            if(j<=0x9F)
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

@implementation NSString_win1252

NSString *NSString_win1252NewWithBytes(NSZone *zone,
										 const char *bytes,NSUInteger length) {
	NSString_win1252 *self=NSAllocateObject( [NSString_win1252 class],length*sizeof(char),zone);

    if (self) {
        self->_length=length;
        int i;
        for(i=0;i<length;i++) {
            unsigned char c = ((uint8_t *)bytes)[i];
            self->_bytes[i]=c;
            if(_mapWin1252ToUnichar(c) == UNDEFINED_UNICODE) {
                [self release];
                return nil;
            }
        }
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

	return _mapWin1252ToUnichar(_bytes[location]);
}

-(void)getCharacters:(unichar *)buffer {
	int i;

	for(i=0;i<_length;i++)
		buffer[i]=_mapWin1252ToUnichar(_bytes[i]);
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
	NSInteger i,loc=range.location,len=range.length;

	if(NSMaxRange(range)>_length){
		NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
						 NSStringFromRange(range),[self length]);
	}

	for(i=0;i<len;i++)
		buffer[i]=_mapWin1252ToUnichar(_bytes[loc+i]);
}

@end
