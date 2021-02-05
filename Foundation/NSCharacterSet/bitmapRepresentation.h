/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSCharacterSet.h>

enum {
    NSBitmapCharacterSetSize = 8192
};

static inline BOOL bitmapIsSet(uint8_t bitmap[8192], unichar character) {
    return bitmap[character >> 3] & (1 << (character & 0x07));
}

static inline void bitmapSet(uint8_t bitmap[8192], unichar character) {
    bitmap[character >> 3] |= 1 << (character & 0x07);
}

static inline void bitmapClear(uint8_t bitmap[8192], unichar character) {
    bitmap[character >> 3] &= ~(1 << (character & 0x07));
}

static inline void bitmapEnable(uint8_t bitmap[8192], unichar character, BOOL yorn) {

    if(yorn)
        bitmap[character >> 3] |= 1 << (character & 0x07);
    else
        bitmap[character >> 3] &= ~(1 << (character & 0x07));
}

static inline uint8_t *bitmapBytes(NSCharacterSet *self) {
    BOOL (*method)() = (void *)[self methodForSelector:@selector(characterIsMember:)];
    uint8_t *bitmap = NSZoneMalloc(NULL, sizeof(uint8_t) * NSBitmapCharacterSetSize);
    uint32_t code;

    for(code = 0; code <= 0xFFFF; code++)
        bitmapEnable(bitmap, code, method(self,
                                       @selector(characterIsMember:), (unichar)code));

    return bitmap;
}
