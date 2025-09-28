/* Copyright (c) 2013 Aiy André - plasq

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSStringEncoder.h"

#import <CoreFoundation/CFString.h>
#import <CoreFoundation/CFStringEncodingExt.h>
#import <Foundation/NSMutableDictionary.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSData.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSRaiseException.h>

@interface _NSEncodingRegistration
+ (void)registerTable:(uint16_t *)table forEncoding:(CFStringEncoding)encoding;
+ (uint16_t *)tableForEncoding:(CFStringEncoding)encoding;
@end

@implementation _NSEncodingRegistration
static NSMapTable *sTables = nil;

+ (void)registerTable:(uint16_t *)table forEncoding:(CFStringEncoding)encoding
{
    if (sTables == nil) {
        sTables = NSCreateMapTable(NSIntegerMapKeyCallBacks, NSNonOwnedPointerMapValueCallBacks, 20);
    }
    NSMapInsert(sTables, (void *)encoding, table);
}

+ (void)registerEncoding
{
    
}

+ (void)registerTables
{
    int num = objc_getClassList(NULL, 0);
    Class classList[num];

    num = objc_getClassList(classList, num);
    for (int i = 0; i < num; i++) {
        Class class = classList[i];
        if (class_getSuperclass(class) == self) {
            [class registerEncoding];
        }
    }
}

+ (uint16_t *)tableForEncoding:(CFStringEncoding)encoding
{
    if (sTables == nil) {
        [self registerTables];
    }
    return NSMapGet(sTables, (void *)encoding);
}
@end

// See the end of the following .h file to see how to use this macro to automatically register an encoding table
#define NSEncodingRegistration(x, y) @interface _NSEncodingRegistration##y : _NSEncodingRegistration; @end @implementation _NSEncodingRegistration##y +(void)registerEncoding { [self registerTable:x forEncoding:y]; } @end

#import "Encoding/CP874.h"
#import "Encoding/CP932.h"
#import "Encoding/CP936.h"
#import "Encoding/CP949.h"
#import "Encoding/CP950.h"
#import "Encoding/CP1250.h"
#import "Encoding/CP1251.h"
#import "Encoding/CP1252.h"
#import "Encoding/CP1253.h"
#import "Encoding/CP1254.h"
#import "Encoding/CP1255.h"
#import "Encoding/CP1256.h"
#import "Encoding/CP1257.h"
#import "Encoding/CP1258.h"
#import "Encoding/8859-1.h"
#import "Encoding/8859-2.h"
#import "Encoding/8859-3.h"
#import "Encoding/8859-4.h"
#import "Encoding/8859-5.h"
#import "Encoding/8859-6.h"
#import "Encoding/8859-7.h"
#import "Encoding/8859-8.h"
#import "Encoding/8859-9.h"
#import "Encoding/8859-10.h"
#import "Encoding/8859-11.h"
#import "Encoding/8859-13.h"
#import "Encoding/8859-14.h"
#import "Encoding/8859-15.h"
#import "Encoding/8859-16.h"

// Returns the
static const uint16_t *tableForCFEncoding(CFStringEncoding encoding)
{
    return [_NSEncodingRegistration tableForEncoding:encoding];
}

static const uint16_t *tableForNSEncoding(NSStringEncoding encoding)
{
    CFStringEncoding cfencoding = CFStringConvertNSStringEncodingToEncoding(encoding);
    return tableForCFEncoding(cfencoding);
}

static NSDictionary *dictionaryForEncoding(NSStringEncoding encoding)
{
    NSMutableDictionary *dict = nil;
    static NSMutableDictionary *sAllDicts = nil;
    @synchronized(sAllDicts) {
        if (sAllDicts == nil) {
            sAllDicts = [[NSMutableDictionary alloc] init];
        }
        dict = [sAllDicts objectForKey:[NSNumber numberWithInteger:encoding]];
        if (dict == nil) {
            const uint16_t *table = tableForNSEncoding(encoding);
            if (table) {
                dict = [NSMutableDictionary dictionary];
                for (int i = 0; table[i] != (uint16_t)-1; i+=2) {
                    [dict setObject:[NSNumber numberWithUnsignedShort:table[i+1]] forKey:[NSNumber numberWithUnsignedShort:table[i]]];
                }
                [sAllDicts setObject:dict forKey:[NSNumber numberWithInteger:encoding]];
            }
        }
    }
    return dict;
}

static NSDictionary *dictionaryForDecoding(NSStringEncoding encoding)
{
    NSMutableDictionary *dict = nil;
    static NSMutableDictionary *sAllDicts = nil;
    @synchronized(sAllDicts) {
        if (sAllDicts == nil) {
            sAllDicts = [[NSMutableDictionary alloc] init];
        }
        dict = [sAllDicts objectForKey:[NSNumber numberWithInteger:encoding]];
        if (dict == nil) {
            const uint16_t *table = tableForNSEncoding(encoding);
            if (table) {
                dict = [NSMutableDictionary dictionary];
                for (int i = 0; table[i] != (uint16_t)-1; i+=2) {
                    if (table[i] != 0xffff) {
                        [dict setObject:[NSNumber numberWithUnsignedShort:table[i]] forKey:[NSNumber numberWithUnsignedShort:table[i+1]]];
                    }
                }
                [sAllDicts setObject:dict forKey:[NSNumber numberWithInteger:encoding]];
            }
        }
    }
    return dict;
}

unichar *NSBytesToUnicode(const unsigned char *bytes,NSUInteger length,NSStringEncoding encoding, NSUInteger *resultLength,NSZone *zone)
{
    unichar *data = NULL;
    NSDictionary *dict = dictionaryForEncoding(encoding);
    if (dict) {
        NSUInteger unicodeLength = 0;
        unichar *unibuffer = malloc(sizeof(unichar)*length);
        unichar current = 0;
        for (NSUInteger i = 0; i < length; ++i) {
            // mask the byte with any pending one (for 2-bytes char translation)
            current |= bytes[i];
            NSNumber *n = [dict objectForKey:[NSNumber numberWithUnsignedShort:current]];
            if (n) {
                uint16_t u = [n unsignedShortValue];
                if (u == (uint16_t)-1) {
                    // 2-bytes char
                    current <<= 8;
                } else {
                    unibuffer[unicodeLength++] = [n unsignedShortValue];
                    current = 0;
                }
            } else {
                // Unknown code
                NSCLog("NSBytesToUnicode : unknown code 0x%X for encoding 0x%X", current, encoding);
                current = 0;
            }
        }
        data = NSZoneMalloc(zone, sizeof(unichar) * unicodeLength);
        if (resultLength) {
            *resultLength = unicodeLength;
        }
        bcopy(unibuffer, data, sizeof(unichar) * unicodeLength);
        free(unibuffer);
    } else {
        static int unknownEncodingLogged = 0;
        if (unknownEncodingLogged < 5) {
            unknownEncodingLogged++;
            NSCLog("NSBytesToUnicode : encoding %d (%x) to unicode not (yet) implemented", encoding, encoding);
        }
    }
    return data;
}

unsigned char *NSBytesFromUnicode(const unichar *characters,NSUInteger length,NSStringEncoding encoding, BOOL lossy, NSUInteger *resultLength,NSZone *zone)
{
    unsigned char *data = NULL;
    NSDictionary *dict = dictionaryForDecoding(encoding);
    if (dict) {
        NSUInteger charLength = 0;
        unsigned char *buffer = NSZoneMalloc(NULL, sizeof(unsigned char)*(length*2));
        if (buffer == NULL) {
            NSCLog("NSBytesFromUnicode: can't allocate buffer of size %d", length*2);
            return NULL;
        }
        for (NSUInteger i = 0; i < length; ++i) {
            // mask the byte with any pending one (for 2-bytes char translation)
            NSNumber *n = [dict objectForKey:[NSNumber numberWithUnsignedShort:characters[i]]];
            if (n) {
                uint16_t u = [n unsignedShortValue];
                if (u >= 0x100) {
                    // 2-bytes char
                    buffer[charLength++] = u>>8;
                    buffer[charLength++] = u&0xff;
                } else {
                    buffer[charLength++] = u;
                }
            } else {
                if (lossy) {
                    // Just ignore the unknown unicode char
                } else {
                    free(buffer);
                    return NULL;
                }
            }
        }
        buffer[charLength++] = 0;
        data = NSZoneMalloc(zone, sizeof(unsigned char) * charLength);
        if (resultLength) {
            *resultLength = charLength;
        }
        bcopy(buffer, data, sizeof(unsigned char) * charLength);
        NSZoneFree(NULL, buffer);
    } else {
        static int unknownEncodingLogged = 0;
        if (unknownEncodingLogged < 5) {
            unknownEncodingLogged++;
            NSCLog("NSBytesFromUnicode : encoding %d (%x) to unicode not (yet) implemented", encoding, encoding);
        }
    }
    return data;
}
