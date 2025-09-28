/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSByteOrder.h>

#ifdef __LITTLE_ENDIAN__

NSByteOrder NSHostByteOrder(void){
   return NS_LittleEndian;
}

unsigned short NSSwapHostShortToLittle(unsigned short value){
    return value;
}

unsigned short NSSwapLittleShortToHost(unsigned short value){
    return value;
}

unsigned int NSSwapHostIntToLittle(unsigned int value){
    return value;
}

unsigned int NSSwapLittleIntToHost(unsigned int value){
    return value;
}

unsigned long NSSwapHostLongToLittle(unsigned long value){
    return value;
}

unsigned long NSSwapLittleLongToHost(unsigned long value){
    return value;
}

unsigned long long NSSwapHostLongLongToLittle(unsigned long long value){
    return value;
}

unsigned long long NSSwapLittleLongLongToHost(unsigned long long value){
    return value;
}

NSSwappedFloat NSSwapHostFloatToLittle(float value){
    return NSConvertHostFloatToSwapped(value);
}

float NSSwapLittleFloatToHost(NSSwappedFloat value){
    return NSConvertSwappedFloatToHost(value);
}

NSSwappedDouble NSSwapHostDoubleToLittle(double value){
    return NSConvertHostDoubleToSwapped(value);
}

double NSSwapLittleDoubleToHost(NSSwappedDouble value){
    return NSConvertSwappedDoubleToHost(value);
}

unsigned short NSSwapHostShortToBig(unsigned short value){
    return NSSwapShort(value);
}

unsigned short NSSwapBigShortToHost(unsigned short value){
    return NSSwapShort(value);
}

unsigned int NSSwapHostIntToBig(unsigned int value){
    return NSSwapInt(value);
}

unsigned int NSSwapBigIntToHost(unsigned int value){
    return NSSwapInt(value);
}

unsigned long NSSwapHostLongToBig(unsigned long value){
    return NSSwapLong(value);
}

unsigned long NSSwapBigLongToHost(unsigned long value){
    return NSSwapLong(value);
}

unsigned long long NSSwapHostLongLongToBig(unsigned long long value){
    return NSSwapLongLong(value);
}

unsigned long long NSSwapBigLongLongToHost(unsigned long long value){
    return NSSwapLongLong(value);
}

NSSwappedFloat NSSwapHostFloatToBig(float value){
    return NSSwapFloat(NSConvertHostFloatToSwapped(value));
}

float NSSwapBigFloatToHost(NSSwappedFloat value){
    return NSConvertSwappedFloatToHost(NSSwapFloat(value));
}

NSSwappedDouble NSSwapHostDoubleToBig(double value){
    return NSSwapDouble(NSConvertHostDoubleToSwapped(value));
}

double NSSwapBigDoubleToHost(NSSwappedDouble value){
    return NSConvertSwappedDoubleToHost(NSSwapDouble(value));
}

#elif defined(__BIG_ENDIAN__)

NSByteOrder NSHostByteOrder(void){
   return NS_BigEndian;
}

unsigned short NSSwapHostShortToLittle(unsigned short value){
    return NSSwapShort(value);
}

unsigned short NSSwapLittleShortToHost(unsigned short value){
    return NSSwapShort(value);
}

unsigned int NSSwapHostIntToLittle(unsigned int value){
    return NSSwapInt(value);
}

unsigned int NSSwapLittleIntToHost(unsigned int value){
    return NSSwapInt(value);
}

unsigned long NSSwapHostLongToLittle(unsigned long value){
    return NSSwapLong(value);
}

unsigned long NSSwapLittleLongToHost(unsigned long value){
    return NSSwapLong(value);
}

unsigned long long NSSwapHostLongLongToLittle(unsigned long long value){
    return NSSwapLongLong(value);
}

unsigned long long NSSwapLittleLongLongToHost(unsigned long long value){
    return NSSwapLongLong(value);
}

NSSwappedFloat NSSwapHostFloatToLittle(float value){
    return NSSwapFloat(NSConvertHostFloatToSwapped(value));
}

float NSSwapLittleFloatToHost(NSSwappedFloat value){
    return NSConvertSwappedFloatToHost(NSSwapFloat(value));
}

NSSwappedDouble NSSwapHostDoubleToLittle(double value){
    return NSSwapDouble(NSConvertHostDoubleToSwapped(value));
}

double NSSwapLittleDoubleToHost(NSSwappedDouble value){
    return NSConvertSwappedDoubleToHost(NSSwapDouble(value));
}

unsigned short NSSwapHostShortToBig(unsigned short value){
    return value;
}

unsigned short NSSwapBigShortToHost(unsigned short value){
    return value;
}

unsigned int NSSwapHostIntToBig(unsigned int value){
    return value;
}

unsigned int NSSwapBigIntToHost(unsigned int value){
    return value;
}

unsigned long NSSwapHostLongToBig(unsigned long value){
    return value;
}

unsigned long NSSwapBigLongToHost(unsigned long value){
    return value;
}

unsigned long long NSSwapHostLongLongToBig(unsigned long long value){
    return value;
}

unsigned long long NSSwapBigLongLongToHost(unsigned long long value){
    return value;
}

NSSwappedFloat NSSwapHostFloatToBig(float value){
    return NSConvertHostFloatToSwapped(value);
}

float NSSwapBigFloatToHost(NSSwappedFloat value){
    return NSConvertSwappedFloatToHost(value);
}

NSSwappedDouble NSSwapHostDoubleToBig(double value){
    return NSConvertHostDoubleToSwapped(value);
}

double NSSwapBigDoubleToHost(NSSwappedDouble value){
    return NSConvertSwappedDoubleToHost(value);
}

#endif


unsigned short NSSwapShort(unsigned short value){
   unsigned short result;

   result=(value<<8)|(value>>8);

   return result;
}

unsigned int NSSwapInt(unsigned int value){
   unsigned int result;

   result=value<<24;
   result|=(value<<8)&0x00FF0000;
   result|=(value>>8)&0x0000FF00;
   result|=value>>24;

   return result;
}

unsigned long NSSwapLong(unsigned long value){
#ifdef __LP64__
   return NSSwapLongLong(value);
#else
   return NSSwapInt(value);
#endif
}

unsigned long long NSSwapLongLong(unsigned long long valueX){
   union {
    unsigned long long word;
    uint8_t      bytes[8];
   } value,result;

   value.word=valueX;

   result.bytes[0]=value.bytes[7];
   result.bytes[1]=value.bytes[6];
   result.bytes[2]=value.bytes[5];
   result.bytes[3]=value.bytes[4];
   result.bytes[4]=value.bytes[3];
   result.bytes[5]=value.bytes[2];
   result.bytes[6]=value.bytes[1];
   result.bytes[7]=value.bytes[0];

   return result.word;
}

NSSwappedFloat NSSwapFloat(NSSwappedFloat value){
   value.floatWord=NSSwapInt(value.floatWord);
   return value;
}

NSSwappedDouble NSSwapDouble(NSSwappedDouble value){
   value.doubleWord=NSSwapLongLong(value.doubleWord);
   return value;
}

NSSwappedFloat NSConvertHostFloatToSwapped(float value){
   union {
    float          value;
    NSSwappedFloat swapped;
   } result;

   result.value=value;

   return result.swapped;
}

float NSConvertSwappedFloatToHost(NSSwappedFloat swapped){
   union {
    float          value;
    NSSwappedFloat swapped;
   } result;

   result.swapped=swapped;

   return result.value;
}

NSSwappedDouble NSConvertHostDoubleToSwapped(double value){
   union {
    double          value;
    NSSwappedDouble swapped;
   } result;

   result.value=value;

   return result.swapped;
}

double NSConvertSwappedDoubleToHost(NSSwappedDouble swapped){
   union {
    double          value;
    NSSwappedDouble swapped;
   } result;

   result.swapped=swapped;

   return result.value;
}

