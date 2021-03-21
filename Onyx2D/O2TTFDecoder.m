#import <Onyx2D/O2TTFDecoder.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2Font.h>
#import <Onyx2D/O2MutablePath.h>
#import <Foundation/NSMapTable.h>

@implementation O2TTFDecoder

typedef uint32_t Fixed32;
typedef uint16_t FWord16;
typedef int64_t  longDateTime64;

O2TTFDecoderRef O2TTFDecoderCreate(O2DataProviderRef dataProvider) {
   O2TTFDecoderRef self=NSAllocateObject([O2TTFDecoder class],0,NULL);

    if (self) {
       self->_dataProvider=O2DataProviderRetain(dataProvider);
       self->_data=O2DataProviderCopyData(dataProvider);
       self->_bytes=CFDataGetBytePtr(self->_data);
       self->_length=CFDataGetLength(self->_data);
       self->_position=0;
    }
    
   return self;
}

O2TTFDecoderRef O2TTFDecoderRetain(O2TTFDecoderRef self) {
   return [self retain];
}

void O2TTFDecoderRelease(O2TTFDecoderRef self) {
   [self release];
}

static void dump(O2TTFDecoderRef self,NSString *format,...){
   if(self->_dump){
    va_list arguments;
    
    va_start(arguments,format);
    NSLogv(format,arguments);
    va_end(arguments);
   }
}

#if 0
static CFIndex currentPosition(O2TTFDecoderRef self){
   return self->_position;
}

static void seekToPosition(O2TTFDecoderRef self,CFIndex value){
   self->_position=value;
}
#endif

static uint8_t decode_uint8(O2TTFDecoderRef self){
   if(self->_position>=self->_length){
    dump(self,@"overflow");
    exit(0);
   }
   
   return self->_bytes[self->_position++];
}

static uint16_t decode_uint16(O2TTFDecoderRef self){
  uint16_t result;
  
  result=decode_uint8(self);
  result<<=8;
  result|=decode_uint8(self);
  
  return result;
}

static int16_t decode_int16(O2TTFDecoderRef self){
  uint16_t result;
  
  result=decode_uint8(self);
  result<<=8;
  result|=decode_uint8(self);
  
  return result;
}


static uint32_t decode_uint32(O2TTFDecoderRef self){
  uint32_t result;
  
  result=decode_uint8(self);
  result<<=8;
  result|=decode_uint8(self);
  result<<=8;
  result|=decode_uint8(self);
  result<<=8;
  result|=decode_uint8(self);
  
  return result;
}

static longDateTime64 decode_longDateTime64(O2TTFDecoderRef self){
   uint64_t result;
   
   result=decode_uint32(self);
   result<<=32;
   result|=decode_uint32(self);
   
   return result;
}

static Fixed32 decode_Fixed32(O2TTFDecoderRef self){
   return decode_uint32(self);
}

static FWord16 decode_FWord16(O2TTFDecoderRef self){
   return decode_uint16(self);
}

void decode_format_0(O2TTFDecoderRef self){
}
void decode_format_2(O2TTFDecoderRef self){
}
void decode_format_4(O2TTFDecoderRef self,O2Glyph **twoLevel){
   struct segment {
    uint16_t endCode;
    uint16_t startCode;
    uint16_t idDelta;
    uint16_t idRangePosition;
   } *segments;
   
   uint16_t length=decode_uint16(self);
   dump(self,@"length=%d",length);
   uint16_t language=decode_uint16(self);
   dump(self,@"language=%d",language);
   uint16_t segCountX2=decode_uint16(self);
   uint16_t segCount=segCountX2/2;
   segments=__builtin_alloca(sizeof(struct segment)*segCount);
   dump(self,@"segCountX2=%d",segCountX2);
   uint16_t searchRange=decode_uint16(self);
   dump(self,@"searchRange=%d",searchRange);
   uint16_t entrySelector=decode_uint16(self);
   dump(self,@"entrySelector=%d",entrySelector);
   uint16_t rangeShift=decode_uint16(self);
   dump(self,@"rangeShift=%d",rangeShift);
   uint16_t i;
   
   for(i=0;i<segCount;i++){
    uint16_t endCode=decode_uint16(self);
    dump(self,@"endCode[%d]=%x",i,endCode);
    segments[i].endCode=endCode;
   }
   
   uint16_t reservedPad=decode_uint16(self);
   dump(self,@"reservedPad=%d",reservedPad);
   
   for(i=0;i<segCount;i++){
    uint16_t startCode=decode_uint16(self);
    dump(self,@"startCode[%d]=%x",i,startCode);
    segments[i].startCode=startCode;
   }
   
   for(i=0;i<segCount;i++){
    uint16_t idDelta=decode_uint16(self);
    dump(self,@"idDelta[%d]=%x",i,idDelta);
    
    segments[i].idDelta=idDelta;
   }

   for(i=0;i<segCount;i++){
    uint16_t position=self->_position;
    uint16_t idRangeOffset=decode_uint16(self);
    
    if(idRangeOffset==0)
     segments[i].idRangePosition=0;
    else
     segments[i].idRangePosition=position+idRangeOffset;
    
    dump(self,@"idRangeOffset[%d]=%x",i,idRangeOffset);
   }
   
   for(i=0;i<segCount && segments[i].endCode!=0xFFFF;i++){
    uint16_t code=segments[i].startCode;

    if(segments[i].idRangePosition==0){
     for(;code<=segments[i].endCode;code++){
      uint16_t glyph=segments[i].idDelta+code;
      uint8_t  group=code>>8;
      uint8_t  index=code&0xFF;
      
      if(twoLevel[group]==NULL)
       twoLevel[group]=NSZoneCalloc(NULL,256,sizeof(O2Glyph));
       
      twoLevel[group][index]=glyph;      
     }
    }
    else {
     self->_position=segments[i].idRangePosition;
     
     for(;code<=segments[i].endCode;code++){
      uint16_t glyph=decode_uint16(self);
      uint8_t  group=code>>8;
      uint8_t  index=code&0xFF;
      
      if(twoLevel[group]==NULL)
       twoLevel[group]=NSZoneCalloc(NULL,256,sizeof(O2Glyph));
       
      twoLevel[group][index]=glyph;      
     }
    }
   }
}
void decode_format_6(O2TTFDecoderRef self){
}

void decode_subtable(O2TTFDecoderRef self,O2Glyph **twoLevel){
   uint16_t platformID=decode_uint16(self);
   dump(self,@"platformID=%d",platformID);
   uint16_t platformSpecificID=decode_uint16(self);
   dump(self,@"platformSpecificID=%d",platformSpecificID);
   uint32_t offset=decode_uint32(self);
   dump(self,@"offset=%d",offset);
   
   CFIndex save=self->_position;
   self->_position=offset;
   uint16_t format=decode_uint16(self);
   dump(self,@"format=%d",format);
   
   switch(format){
   
    case 0:
     decode_format_0(self);
     break;
     
    case 2:
     decode_format_2(self);
     break;
     
    case 4:
     decode_format_4(self,twoLevel);
     break;
     
    case 6:
     decode_format_6(self);
     break;
     
    default:
     dump(self,@"unknown format %d",format);
     break;
     
   }
   
   self->_position=save;
}

O2Glyph **O2TTFecoderTwoLevelUnicode_cmap(O2TTFDecoderRef self){
   O2Glyph **twoLevel=NSZoneCalloc(NULL,256,sizeof(O2Glyph *));
   
   uint16_t version=decode_uint16(self);
   dump(self,@"version=%d",version);
   uint16_t i,subTableCount=decode_uint16(self);
   dump(self,@"subTableCount=%d",subTableCount);
   
   for(i=0;i<subTableCount;i++){
    decode_subtable(self,twoLevel);
   }
   
   return twoLevel;
}

BOOL seekToTable(O2TTFDecoderRef self,uint32_t seekToTag){
   self->_position=0;
   uint32_t scaler=decode_uint32(self);
   
   if(scaler!=0x00010000 && scaler!=0x74727565){
    dump(self,@"invalid scaler=%08X",scaler);
    return NO;
   }

   uint16_t numTables=decode_uint16(self);
   uint16_t searchRange=decode_uint16(self);
   uint16_t entrySelector=decode_uint16(self);
   uint16_t rangeShift=decode_uint16(self);
   int i;
   
   for(i=0;i<numTables;i++){
    uint32_t tag=decode_uint32(self);
    uint32_t checkSum=decode_uint32(self);
    uint32_t offset=decode_uint32(self);
    uint32_t length=decode_uint32(self);

    if(tag==seekToTag){
     self->_position=offset;
     return YES;
    }
   }
   
   dump(self,@"unable to find tag %c%c%c%c",seekToTag>>24,(seekToTag>>16)&0xFF,(seekToTag>>8)&0xFF,seekToTag&0xFF);
   return NO;
}
static struct {
// FIXME: remove the glyph entry, pointless
    O2Glyph   glyph;
    NSString *name;
   } MacintoshNameMapping[258]={
    { 0,@".notdef" },
    { 1,@".null" },
    { 2,@"nonmarkingreturn" },
    { 3,@"space" },
    { 4,@"exclam" },
    { 5,@"quotedbl" },
    { 6,@"numbersign" },
    { 7,@"dollar" },
    { 8,@"percent" },
    { 9,@"ampersand" },
    { 10,@"quotesingle" },
    { 11,@"parenleft" },
    { 12,@"parenright" },
    { 13,@"asterisk" },
    { 14,@"plus" },
    { 15,@"comma" },
    { 16,@"hyphen" },
    { 17,@"period" },
    { 18,@"slash" },
    { 19,@"zero" },
    { 20,@"one" },
    { 21,@"two" },
    { 22,@"three" },
    { 23,@"four" },
    { 24,@"five" },
    { 25,@"six" },
    { 26,@"seven" },
    { 27,@"eight" },
    { 28,@"nine" },
    { 29,@"colon" },
    { 30,@"semicolon" },
    { 31,@"less" },
    { 32,@"equal" },
    { 33,@"greater" },
    { 34,@"question" },
    { 35,@"at" },
    { 36,@"A" },
    { 37,@"B" },
    { 38,@"C" },
    { 39,@"D" },
    { 40,@"E" },
    { 41,@"F" },
    { 42,@"G" },
    { 43,@"H" },
    { 44,@"I" },
    { 45,@"J" },
    { 46,@"K" },
    { 47,@"L" },
    { 48,@"M" },
    { 49,@"N" },
    { 50,@"O" },
    { 51,@"P" },
    { 52,@"Q" },
    { 53,@"R" },
    { 54,@"S" },
    { 55,@"T" },
    { 56,@"U" },
    { 57,@"V" },
    { 58,@"W" },
    { 59,@"X" },
    { 60,@"Y" },
    { 61,@"Z" },
    { 62,@"bracketleft" },
    { 63,@"backslash" },
    { 64,@"bracketright" },
    { 65,@"asciicircum" },
    { 66,@"underscore" },
    { 67,@"grave" },
    { 68,@"a" },
    { 69,@"b" },
    { 70,@"c" },
    { 71,@"d" },
    { 72,@"e" },
    { 73,@"f" },
    { 74,@"g" },
    { 75,@"h" },
    { 76,@"i" },
    { 77,@"j" },
    { 78,@"k" },
    { 79,@"l" },
    { 80,@"m" },
    { 81,@"n" },
    { 82,@"o" },
    { 83,@"p" },
    { 84,@"q" },
    { 85,@"r" },
    { 86,@"s" },
    { 87,@"t" },
    { 88,@"u" },
    { 89,@"v" },
    { 90,@"w" },
    { 91,@"x" },
    { 92,@"y" },
    { 93,@"z" },
    { 94,@"braceleft" },
    { 95,@"bar" },
    { 96,@"braceright" },
    { 97,@"asciitilde" },
    { 98,@"Adieresis" },
    { 99,@"Aring" },
    { 100,@"Ccedilla" },
    { 101,@"Eacute" },
    { 102,@"Ntilde" },
    { 103,@"Odieresis" },
    { 104,@"Udieresis" },
    { 105,@"aacute" },
    { 106,@"agrave" },
    { 107,@"acircumflex" },
    { 108,@"adieresis" },
    { 109,@"atilde" },
    { 110,@"aring" },
    { 111,@"ccedilla" },
    { 112,@"eacute" },
    { 113,@"egrave" },
    { 114,@"ecircumflex" },
    { 115,@"edieresis" },
    { 116,@"iacute" },
    { 117,@"igrave" },
    { 118,@"icircumflex" },
    { 119,@"idieresis" },
    { 120,@"ntilde" },
    { 121,@"oacute" },
    { 122,@"ograve" },
    { 123,@"ocircumflex" },
    { 124,@"odieresis" },
    { 125,@"otilde" },
    { 126,@"uacute" },
    { 127,@"ugrave" },
    { 128,@"ucircumflex" },
    { 129,@"udieresis" },
    { 130,@"dagger" },
    { 131,@"degree" },
    { 132,@"cent" },
    { 133,@"sterling" },
    { 134,@"section" },
    { 135,@"bullet" },
    { 136,@"paragraph" },
    { 137,@"germandbls" },
    { 138,@"registered" },
    { 139,@"copyright" },
    { 140,@"trademark" },
    { 141,@"acute" },
    { 142,@"dieresis" },
    { 143,@"notequal" },
    { 144,@"AE" },
    { 145,@"Oslash" },
    { 146,@"infinity" },
    { 147,@"plusminus" },
    { 148,@"lessequal" },
    { 149,@"greaterequal" },
    { 150,@"yen" },
    { 151,@"mu" },
    { 152,@"partialdiff" },
    { 153,@"summation" },
    { 154,@"product" },
    { 155,@"pi" },
    { 156,@"integral" },
    { 157,@"ordfeminine" },
    { 158,@"ordmasculine" },
    { 159,@"Omega" },
    { 160,@"ae" },
    { 161,@"oslash" },
    { 162,@"questiondown" },
    { 163,@"exclamdown" },
    { 164,@"logicalnot" },
    { 165,@"radical" },
    { 166,@"florin" },
    { 167,@"approxequal" },
    { 168,@"Delta" },
    { 169,@"guillemotleft" },
    { 170,@"guillemotright" },
    { 171,@"ellipsis" },
    { 172,@"nonbreakingspace" },
    { 173,@"Agrave" },
    { 174,@"Atilde" },
    { 175,@"Otilde" },
    { 176,@"OE" },
    { 177,@"oe" },
    { 178,@"endash" },
    { 179,@"emdash" },
    { 180,@"quotedblleft" },
    { 181,@"quotedblright" },
    { 182,@"quoteleft" },
    { 183,@"quoteright" },
    { 184,@"divide" },
    { 185,@"lozenge" },
    { 186,@"ydieresis" },
    { 187,@"Ydieresis" },
    { 188,@"fraction" },
    { 189,@"currency" },
    { 190,@"guilsinglleft" },
    { 191,@"guilsinglright" },
    { 192,@"fi" },
    { 193,@"fl" },
    { 194,@"daggerdbl" },
    { 195,@"periodcentered" },
    { 196,@"quotesinglbase" },
    { 197,@"quotedblbase" },
    { 198,@"perthousand" },
    { 199,@"Acircumflex" },
    { 200,@"Ecircumflex" },
    { 201,@"Aacute" },
    { 202,@"Edieresis" },
    { 203,@"Egrave" },
    { 204,@"Iacute" },
    { 205,@"Icircumflex" },
    { 206,@"Idieresis" },
    { 207,@"Igrave" },
    { 208,@"Oacute" },
    { 209,@"Ocircumflex" },
    { 210,@"apple" },
    { 211,@"Ograve" },
    { 212,@"Uacute" },
    { 213,@"Ucircumflex" },
    { 214,@"Ugrave" },
    { 215,@"dotlessi" },
    { 216,@"circumflex" },
    { 217,@"tilde" },
    { 218,@"macron" },
    { 219,@"breve" },
    { 220,@"dotaccent" },
    { 221,@"ring" },
    { 222,@"cedilla" },
    { 223,@"hungarumlaut" },
    { 224,@"ogonek" },
    { 225,@"caron" },
    { 226,@"Lslash" },
    { 227,@"lslash" },
    { 228,@"Scaron" },
    { 229,@"scaron" },
    { 230,@"Zcaron" },
    { 231,@"zcaron" },
    { 232,@"brokenbar" },
    { 233,@"Eth" },
    { 234,@"eth" },
    { 235,@"Yacute" },
    { 236,@"yacute" },
    { 237,@"Thorn" },
    { 238,@"thorn" },
    { 239,@"minus" },
    { 240,@"multiply" },
    { 241,@"onesuperior" },
    { 242,@"twosuperior" },
    { 243,@"threesuperior" },
    { 244,@"onehalf" },
    { 245,@"onequarter" },
    { 246,@"threequarters" },
    { 247,@"franc" },
    { 248,@"Gbreve" },
    { 249,@"gbreve" },
    { 250,@"Idotaccent" },
    { 251,@"Scedilla" },
    { 252,@"scedilla" },
    { 253,@"Cacute" },
    { 254,@"cacute" },
    { 255,@"Ccaron" },
    { 256,@"ccaron" },
    { 257,@"dcroat" },
   };

static void loadMacintoshNameMapping(NSMapTable *table){
   NSInteger i;
   
   for(i=0;i<258;i++)
    NSMapInsert(table,MacintoshNameMapping[i].name,(void *)i);
}

NSMapTable *O2TTFDecoderGetPostScriptNameMapTable(O2TTFDecoderRef self,int *numberOfGlyphs){
   NSMapTable *result=NSCreateMapTable(NSObjectMapKeyCallBacks,NSIntegerMapValueCallBacks,258);

   if(!seekToTable(self,'post'))
    return NULL;

   Fixed32 format=decode_Fixed32(self);
   Fixed32 italicAngle=decode_Fixed32(self);
   FWord16 underlinePosition=decode_FWord16(self);
   FWord16 underlineThickness=decode_FWord16(self);
   uint32_t isFixedPitch=decode_uint32(self);
   uint32_t minMemType42=decode_uint32(self);
   uint32_t maxMemType42=decode_uint32(self);
   uint32_t minMemType1=decode_uint32(self);
   uint32_t maxMemType1=decode_uint32(self);
      
   switch(format){
   
    default:
     NSLog(@"unimplemented 'post' format %08X",format);
     return NULL;

    case 0x00010000:;
     loadMacintoshNameMapping(result);
     break;
     
    case 0x00020000:;
     uint16_t numberOfGlyphs=decode_uint16(self);
     int      i;
     uint16_t maxIndex=0;
     uint16_t glyphNameIndex[numberOfGlyphs];
     
     for(i=0;i<numberOfGlyphs;i++){
      glyphNameIndex[i]=decode_uint16(self);
      
      maxIndex=MAX(maxIndex,glyphNameIndex[i]);
     }
     maxIndex-=258;
     maxIndex++;
     NSString *names[maxIndex];
     
     for(i=0;i<maxIndex;i++){
      uint8_t length=decode_uint8(self);
      uint8_t buffer[length];
      int     count=0;
      
      for(count=0;count<length;count++)
       buffer[count]=decode_uint8(self);
       
      names[i]=[[NSString alloc] initWithBytes:buffer length:count encoding:NSASCIIStringEncoding];
     }

     for(i=0;i<numberOfGlyphs;i++){
      uint16_t nameIndex=glyphNameIndex[i];
      
      if(nameIndex<=257)
       NSMapInsert(result,MacintoshNameMapping[nameIndex].name,(void *)i);
      else {
       nameIndex-=258;
       NSMapInsert(result,names[nameIndex],(void *)i);
      } 
     }
     
     for(i=0;i<maxIndex;i++)
      [names[i] release];
     break;
     
   }
   
   return result;
}

int O2TTFDecoderGetOffsetsAreLong(O2TTFDecoderRef self) {
   if(!seekToTable(self,'head'))
    return 0;

   Fixed32  version=decode_Fixed32(self);
   Fixed32  fontRevision=decode_Fixed32(self);
   uint32_t checkSumAdjustment=decode_uint32(self);
   uint32_t magicNumber=decode_uint32(self);
   uint16_t flags=decode_uint16(self);
   uint16_t unitsPerEm=decode_uint16(self);
   longDateTime64 created=decode_longDateTime64(self);
   longDateTime64 modified=decode_longDateTime64(self);
   FWord16 xMin=decode_FWord16(self);
   FWord16 yMin=decode_FWord16(self);
   FWord16 xMax=decode_FWord16(self);
   FWord16 yMax=decode_FWord16(self);
   uint16_t macStyle=decode_uint16(self);
   uint16_t lowestRecPPEM=decode_uint16(self);
   int16_t fontDirectionHint=decode_int16(self);
   int16_t indexToLocFormat=decode_int16(self);
   int16_t glyphDataFormat=decode_int16(self);
 
   return indexToLocFormat;
}

int *O2TTFDecoderGetGlyphLocations(O2TTFDecoderRef self,int numberOfGlyphs) {
   int *result=NSZoneMalloc(NULL,sizeof(int)*numberOfGlyphs);
   
   if(O2TTFDecoderGetOffsetsAreLong(self)){
    if(!seekToTable(self,'loca'))
     return NULL;
     
    int i;
    for(i=0;i<numberOfGlyphs;i++)
     result[i]=decode_uint32(self);
     
   }
   else {
    if(!seekToTable(self,'loca'))
     return NULL;
     
    int i;
    for(i=0;i<numberOfGlyphs;i++)
     result[i]=decode_uint16(self);
   }
   return result;
}

O2PathRef O2TTFDecoderGetGlyphOutline(O2TTFDecoderRef self,int glyphLocation) {
   O2PathRef result=O2PathCreateMutable();
   
    if(!seekToTable(self,'glyf'))
     return NULL;

   self->_position+=glyphLocation;
   
   int16_t numberOfContours=decode_int16(self);
   
   if(numberOfContours>=0){
    uint16_t endPtsOfContours[numberOfContours];
    int i;
    
    for(i=0;i<numberOfContours;i++)
     endPtsOfContours[i]=decode_uint16(self);

    uint16_t instructionLength=decode_uint16(self);
    uint8_t  instructions[instructionLength];
    
    for(i=0;i<instructionLength;i++)
     instructions[i]=decode_uint8(self);
    
    
   }
   else if(numberOfContours==-1){
   }
   else {
    NSLog(@"invalid numberOfContours=%d",numberOfContours);
   }
   
   return result;
}

void O2TTFDecoderGetNameTable(O2TTFDecoderRef self) {
   if(!seekToTable(self,'name')){
    NSLog(@"TrueType error: No name table");
    return;
   }

   int      stringTable=self->_position;
   
   uint16_t format=decode_uint16(self);
   uint16_t i,count=decode_uint16(self);
   uint16_t stringOffset=decode_uint16(self);

   stringTable+=stringOffset;
   
   for(i=0;i<count;i++){
    uint16_t platformID=decode_uint16(self);
    /*uint16_t platformSpecificID=*/decode_uint16(self);
    uint16_t languageID=decode_uint16(self);
    uint16_t nameID=decode_uint16(self);
    uint16_t length=decode_uint16(self);
    uint16_t offset=decode_uint16(self);
    
    CFIndex location=stringTable+offset;
    
    NSLog(@"platformID=%d,languageId=%d,nameID=%d",platformID,languageID,nameID);
    
    NSLog(@"position=%ld,stringOffset=%d,offset=%d",self->_position,stringOffset,offset);
    
    NSString *string=[NSString stringWithCString:(const char *)self->_bytes+location length:length];
    
    NSLog(@"platformID=%d,languageID=%d,string=%@",platformID,languageID,string);
   }


}

@end
