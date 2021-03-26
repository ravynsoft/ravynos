#import <Foundation/Foundation.h>

/*
Gif-Lib by Gershon Elber,Eric S. Raymond,Toshio Kuratomi
The GIFLIB distribution is Copyright (c) 1997  Eric S. Raymond

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define LZ_MAX_CODE 4095 /* Biggest code possible in 12 bits. */

typedef int LZWWord;
typedef unsigned int LZWPrefixType;
typedef unsigned char LZWPixelType;

typedef struct LZWFileType {

    int LZWError;

    LZWWord
        BitsPerPixel, /* Bits per pixel (Codes uses at least this + 1). */
        ClearCode, /* The CLEAR LZ code. */
        EOFCode, /* The EOF LZ code. */
        RunningCode, /* The next code algorithm can generate. */
        RunningBits, /* The number of bits required to represent RunningCode. */
        MaxCode1, /* 1 bigger than max. possible code, in RunningBits bits. */
        LastCode, /* The code before the current code. */
        CrntCode, /* Current algorithm code. */
        StackPtr, /* For character stack (see below). */
        CrntShiftState; /* Number of bits in CrntShiftDWord. */
    unsigned long CrntShiftDWord; /* For bytes decomposition into codes. */
    unsigned long PixelCount; /* Number of pixels in image. */
    NSInputStream *inputStream;
    uint8_t Stack[LZ_MAX_CODE]; /* Decoded pixels are stacked here. */
    uint8_t Suffix[LZ_MAX_CODE + 1]; /* So we can trace the codes. */
    LZWPrefixType Prefix[LZ_MAX_CODE + 1];
} LZWFileType;

NSData *LZWDecodeWithExpectedResultLength(NSData *data, unsigned stripLength);
