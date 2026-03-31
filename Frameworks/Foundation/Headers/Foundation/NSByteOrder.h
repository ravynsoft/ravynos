/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

typedef enum {
    NS_UnknownByteOrder,
    NS_LittleEndian,
    NS_BigEndian
} NSByteOrder;

typedef struct {
    uint32_t floatWord;
} NSSwappedFloat;

typedef struct {
    uint64_t doubleWord;
} NSSwappedDouble;

FOUNDATION_EXPORT NSByteOrder NSHostByteOrder(void);

FOUNDATION_EXPORT unsigned short NSSwapHostShortToLittle(unsigned short value);
FOUNDATION_EXPORT unsigned short NSSwapLittleShortToHost(unsigned short value);
FOUNDATION_EXPORT unsigned int NSSwapHostIntToLittle(unsigned int value);
FOUNDATION_EXPORT unsigned int NSSwapLittleIntToHost(unsigned int value);
FOUNDATION_EXPORT unsigned long NSSwapHostLongToLittle(unsigned long value);
FOUNDATION_EXPORT unsigned long NSSwapLittleLongToHost(unsigned long value);
FOUNDATION_EXPORT unsigned long long NSSwapHostLongLongToLittle(unsigned long long value);
FOUNDATION_EXPORT unsigned long long NSSwapLittleLongLongToHost(unsigned long long value);
FOUNDATION_EXPORT NSSwappedFloat NSSwapHostFloatToLittle(float value);
FOUNDATION_EXPORT float NSSwapLittleFloatToHost(NSSwappedFloat value);
FOUNDATION_EXPORT NSSwappedDouble NSSwapHostDoubleToLittle(double value);
FOUNDATION_EXPORT double NSSwapLittleDoubleToHost(NSSwappedDouble value);

FOUNDATION_EXPORT unsigned short NSSwapHostShortToBig(unsigned short value);
FOUNDATION_EXPORT unsigned short NSSwapBigShortToHost(unsigned short value);
FOUNDATION_EXPORT unsigned int NSSwapHostIntToBig(unsigned int value);
FOUNDATION_EXPORT unsigned int NSSwapBigIntToHost(unsigned int value);
FOUNDATION_EXPORT unsigned long NSSwapHostLongToBig(unsigned long value);
FOUNDATION_EXPORT unsigned long NSSwapBigLongToHost(unsigned long value);
FOUNDATION_EXPORT unsigned long long NSSwapHostLongLongToBig(unsigned long long value);
FOUNDATION_EXPORT unsigned long long NSSwapBigLongLongToHost(unsigned long long value);
FOUNDATION_EXPORT NSSwappedFloat NSSwapHostFloatToBig(float value);
FOUNDATION_EXPORT float NSSwapBigFloatToHost(NSSwappedFloat value);
FOUNDATION_EXPORT NSSwappedDouble NSSwapHostDoubleToBig(double value);
FOUNDATION_EXPORT double NSSwapBigDoubleToHost(NSSwappedDouble value);

FOUNDATION_EXPORT unsigned short NSSwapShort(unsigned short value);
FOUNDATION_EXPORT unsigned int NSSwapInt(unsigned int value);
FOUNDATION_EXPORT unsigned long NSSwapLong(unsigned long value);
FOUNDATION_EXPORT unsigned long long NSSwapLongLong(unsigned long long valueX);
FOUNDATION_EXPORT NSSwappedFloat NSSwapFloat(NSSwappedFloat value);
FOUNDATION_EXPORT NSSwappedDouble NSSwapDouble(NSSwappedDouble value);

FOUNDATION_EXPORT NSSwappedFloat NSConvertHostFloatToSwapped(float value);
FOUNDATION_EXPORT float NSConvertSwappedFloatToHost(NSSwappedFloat swapped);
FOUNDATION_EXPORT NSSwappedDouble NSConvertHostDoubleToSwapped(double value);
FOUNDATION_EXPORT double NSConvertSwappedDoubleToHost(NSSwappedDouble swapped);
