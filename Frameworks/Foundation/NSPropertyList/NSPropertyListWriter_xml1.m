/* Copyright (c) 2008 Pauli Olavi Ojala

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSPropertyListWriter_xml1.h"
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNumber.h>
#include <string.h>

@implementation NSPropertyListWriter_xml1

- (id)init
{
    _data = [NSMutableData new];
    return self;
}

- (void)dealloc
{
   [_data release];
   [super dealloc];
}

-(void)_encodeIndent:(NSInteger)indent
{
    int i;
    for(i = 0; i < indent; i++)
        [_data appendBytes:"    " length:4];
}

-(void)_encodeString:(NSString *)string
{
    if ( !string) return;
    const char *utf8 = [string UTF8String];
    size_t len = strlen(utf8);
    
    if (utf8 && len > 0) {
        // FIXME: to match Cocoa, the string should be escaped HTML-style
        // (e.g. '>' --> "&gt;") instead of using CDATA
        BOOL addCDATA = ([string rangeOfString:@"<"].location != NSNotFound ||
                         [string rangeOfString:@">"].location != NSNotFound ||
                         [string rangeOfString:@"&"].location != NSNotFound ||
                         [string rangeOfString:@"\'"].location != NSNotFound ||
                         [string rangeOfString:@"\""].location != NSNotFound
                        );

        if (addCDATA) [_data appendBytes:"<![CDATA[" length:9];
        [_data appendBytes:utf8 length:len];
        if (addCDATA) [_data appendBytes:"]]>" length:3];
    }
}

-(void)encodeString:(NSString *)string indent:(NSInteger)indent
{
    [self _encodeIndent:indent];
    
    [_data appendBytes:"<string>" length:8];
    [self _encodeString:string];
    [_data appendBytes:"</string>\n" length:10];
}

-(void)encodeKey:(NSString *)key indent:(NSInteger)indent
{
    [self _encodeIndent:indent];
    
    [_data appendBytes:"<key>" length:5];
    [self _encodeString:key];
    [_data appendBytes:"</key>\n" length:7];
}

- (void)encodeInteger:(NSInteger)integer
{
    [_data appendBytes:"<integer>" length:9];
    [self _encodeString:[NSString stringWithFormat:@"%li", integer]];
    [_data appendBytes:"</integer>\n" length:11];
}

- (void)encodeUnsignedInteger:(NSUInteger)uinteger
{
    [_data appendBytes:"<integer>" length:9];
    [self _encodeString:[NSString stringWithFormat:@"%lu", uinteger]];
    [_data appendBytes:"</integer>\n" length:11];
}

- (void)encodeReal:(NSNumber *)number
{
    [_data appendBytes:"<real>" length:6];
    [self _encodeString:[NSString stringWithFormat:@"%.16f", [number doubleValue]]];
    [_data appendBytes:"</real>\n" length:8];
}

- (void)encodeChar:(char)ch
{
    // all chars of value 0 or 1 are interpreted as BOOL.
    // not sure if this is a good idea, but BOOL doesn't have a type encoding of its own, so I don't know what else to do
    if (ch == NO) {
        [_data appendBytes:"<false/>\n" length:9];
    } else if (ch == YES) {
        [_data appendBytes:"<true/>\n" length:8];
    } else {
        [self encodeInteger:ch];
    }
}

- (void)encodeNumber:(NSNumber *)number indent:(NSInteger)indent
{
    [self _encodeIndent:indent];
    
    const char *type = [number objCType];
    
    // TODO: should possibly handle other integer type encodings such as long long?
    switch (*type) {
        case 'i':
        case 'l':
        case 'q':
            [self encodeInteger:[number intValue]];  break;
            
        case 'I':
        case 'L':
        case 'Q':
            [self encodeUnsignedInteger:[number unsignedIntValue]];  break;
            
        case 'c':  // this is the encoding for BOOL
            [self encodeChar:[number charValue]];  break;
            
        case 'f':
        case 'd':
        default:
            [self encodeReal:number];  break;
    }    
}

- (void)encodeArray:(NSArray *)array indent:(NSInteger)indent
{
    NSUInteger i, count = [array count];
    
    [self _encodeIndent:indent];
    [_data appendBytes:"<array>\n" length:8];    
    indent++;
    
    for(i=0;i<count;i++) {
        id obj = [array objectAtIndex:i];
        [self encodePropertyList:obj indent:indent];
    }
    
    indent--;
    [self _encodeIndent:indent];
    [_data appendBytes:"</array>\n" length:9];
}

- (void)encodeDictionary:(NSDictionary *)dictionary indent:(NSInteger)indent
{
    NSArray *allKeys = [[dictionary allKeys] sortedArrayUsingSelector:@selector(compare:)];
    
    NSUInteger i, count = [allKeys count];

    [self _encodeIndent:indent];
    [_data appendBytes:"<dict>\n" length:7];    
    indent++;
    
    for(i=0;i<count;i++) {
        id key = [allKeys objectAtIndex:i];

        [self encodeKey:key indent:indent];
        [self encodePropertyList:[dictionary objectForKey:key] indent:indent];
    }
    
    indent--;
    [self _encodeIndent:indent];
    [_data appendBytes:"</dict>\n" length:8];
}

- (void)encodeData:(NSData *)data indent:(NSInteger)indent
{
    static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    NSUInteger y, n;
    const NSUInteger len = [data length];
    const uint8_t *bytes = [data bytes];

    [self _encodeIndent:indent];
    [_data appendBytes:"<data>\n" length:7];    
    indent++;
    
    const NSUInteger numRows = 1 + len / 45;   // 60 chars per output row -> 45 input bytes per row
    uint8_t srcBuf[48];
    uint8_t dstBuf[64];
    
    n = 0;
    for (y = 0; y < numRows; y++) {
        const NSUInteger rowSrcLen = (y < numRows - 1) ? 45 : (len - n);

        [self _encodeIndent:indent];

        memcpy(srcBuf, bytes + n, rowSrcLen);
        n += rowSrcLen;
        
        if (rowSrcLen < 45)
            memset(srcBuf + rowSrcLen, 0, 45 - rowSrcLen);

        unsigned char *inBuf = srcBuf;
        unsigned char *outBuf = dstBuf;
        int x;
        for (x = 0; x < rowSrcLen; x += 3) {
            const NSInteger blockLen = (x + 3 < rowSrcLen) ? 3 : (rowSrcLen - x);
            
            // base64 encode
            outBuf[0] = b64[ inBuf[0] >> 2 ];
            outBuf[1] = b64[ ((inBuf[0] & 0x03) << 4) | ((inBuf[1] & 0xf0) >> 4) ];
            outBuf[2] = (unsigned char) (blockLen > 1) ? b64[ ((inBuf[1] & 0x0f) << 2) | ((inBuf[2] & 0xc0) >> 6) ] : '=';
            outBuf[3] = (unsigned char) (blockLen > 2) ? b64[ inBuf[2] & 0x3f ] : '=';
            inBuf += 3;
            outBuf += 4;
        }
        outBuf[0] = (unsigned char)'\n';
        
        [_data appendBytes:dstBuf length:(outBuf - dstBuf + 1)];
    }
    
    indent--;
    [self _encodeIndent:indent];
    [_data appendBytes:"</data>\n" length:8];
}

- (void)encodePropertyList:(id)plist indent:(NSInteger)indent
{
    if ([plist isKindOfClass:objc_lookUpClass("NSString")])
        [self encodeString:plist indent:indent];
    else if ([plist isKindOfClass:objc_lookUpClass("NSArray")])
        [self encodeArray:plist indent:indent];
    else if ([plist isKindOfClass:objc_lookUpClass("NSDictionary")])
        [self encodeDictionary:plist indent:indent];
    else if ([plist isKindOfClass:objc_lookUpClass("NSNumber")])
        [self encodeNumber:plist indent:indent];
    else if ([plist isKindOfClass:objc_lookUpClass("NSData")])
        [self encodeData:plist indent:indent];
    else
        [self encodeString:[plist description] indent:indent];
        
    // TODO: NSDate needs special encoding as well
}


- (NSData *)dataForRootObject:(id)object
{
    const char header[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n";
    [_data appendBytes:header length:strlen(header)];
   
    [_data appendBytes:"<plist version=\"1.0\">\n" length:22];
   
    [self encodePropertyList:object indent:0];

    [_data appendBytes:"</plist>\n" length:9];
    
    return _data;
}


+ (NSData *)dataWithPropertyList:(id)plist
{
   NSPropertyListWriter_xml1 *writer = [[self alloc] init];
   NSData *result = [[[writer dataForRootObject:plist] retain] autorelease];

   [writer release];

   return result;
}

@end
