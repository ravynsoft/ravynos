/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSString_cString.h>
#import <Foundation/NSString_defaultEncoding.h>
#import <Foundation/NSString_nextstepCString.h>
#import <Foundation/NSString_unicode.h>
#import <Foundation/NSString_macOSRoman.h>
#import <Foundation/NSString_win1252.h>
#import <Foundation/NSStringSymbol.h>
#import <Foundation/NSString_isoLatin1.h>
#import <Foundation/NSString_isoLatin2.h>
#import <Foundation/NSStringUTF8.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

#import "NSStringEncoder.h"

#include <string.h>

// Note: we are falling back to NeXTSTEP encoding for unsupported ones, and log a message only once
//       And before supporting non ASCII based ones, we should probably first clean any use of NSString<->C string conversion
//       with no explicit encoding because some place probably really expect some kind of ASCII

// Only the first messages about unsupported encoding are logged so we don't spend our time and fill the disk logging these errors
// (just logging the first one seem not enough - the first messages seem lost in space)
#define logEncodingError(encoding)\
{\
    if(encoding != defaultEncoding()) {\
        static int unsupportedEncodingLogged = 0;\
        if (unsupportedEncodingLogged < 10) {\
            unsupportedEncodingLogged++;\
            NSCLog("Use of unknown encoding %d",encoding);\
        }\
    } else {\
        static int unsupportedDefaultEncodingLogged = 0;\
        if (unsupportedDefaultEncodingLogged < 10) {\
            unsupportedDefaultEncodingLogged++;\
            NSCLog("Unsupported default encoding %d",encoding);\
            NSCLog("%s() unimplemented in %s at %d",__PRETTY_FUNCTION__,__FILE__,__LINE__);\
        }\
    }\
}

unichar *NSCharactersFromCString(const char *cString,NSUInteger length,
  NSUInteger *resultLength,NSZone *zone) {
   return NSString_anyCStringToUnicode(defaultEncoding(),cString,length,resultLength,zone);
}

char    *NSString_cStringFromCharacters(const unichar *characters,NSUInteger length,
  BOOL lossy,NSUInteger *resultLength,NSZone *zone,BOOL zeroTerminate) {
   return NSString_unicodeToAnyCString(defaultEncoding(),characters,length,lossy,resultLength,zone, zeroTerminate);
}

NSUInteger NSGetCStringWithMaxLength(const unichar *characters,NSUInteger length,NSUInteger *location,char *cString,NSUInteger maxLength,BOOL lossy){
   return NSGetAnyCStringWithMaxLength(defaultEncoding(), characters,length,location,cString,maxLength,lossy);
}

NSString *NSString_cStringNewWithBytesAndZero(NSZone *zone,const char *bytes) {
   int length=0;

   while(bytes[length]!='\0')
    length++;

   return NSString_cStringNewWithBytes(zone,bytes,length);
}

NSString *NSString_cStringNewWithBytes(NSZone *zone,
 const char *bytes,NSUInteger length) {
   return NSString_anyCStringNewWithBytes(defaultEncoding(),zone,bytes,length);
}

NSString *NSString_cStringNewWithCharacters(NSZone *zone,
 const unichar *characters,NSUInteger length,BOOL lossy) {
   return NSString_anyCStringNewWithCharacters(defaultEncoding(),zone,characters,length,lossy);
}

NSString *NSString_cStringNewWithCapacity(NSZone *zone,NSUInteger capacity,char **ptr) {
   return NSNEXTSTEPCStringNewWithCapacity(zone,capacity,ptr);
}

NSString *NSString_cStringWithBytesAndZero(NSZone *zone,const char *bytes) {
    return NSAutorelease(NSString_cStringNewWithBytesAndZero(zone,bytes));
}
unichar *NSString_anyCStringToUnicode(NSStringEncoding encoding, const char *cString,NSUInteger length, NSUInteger *resultLength,NSZone *zone)
{
    switch(encoding) {
        case NSNEXTSTEPStringEncoding:
            return NSNEXTSTEPToUnicode(cString,length,resultLength,zone);
        case NSASCIIStringEncoding:
        case NSISOLatin1StringEncoding:
            return NSISOLatin1ToUnicode(cString,length,resultLength,zone);
        case NSISOLatin2StringEncoding:
            return NSISOLatin2ToUnicode(cString,length,resultLength,zone);
        case NSWindowsCP1252StringEncoding:
            return NSWin1252ToUnicode(cString,length,resultLength,zone);
        case NSMacOSRomanStringEncoding:
            return NSMacOSRomanToUnicode(cString,length,resultLength,zone);
        case NSUTF8StringEncoding:
            return NSUTF8ToUnicode(cString,length,resultLength,zone);
        default: {
            unichar *chars = NSBytesToUnicode(cString, length, encoding, resultLength, zone);
            if (chars) {
                return chars;
            }
            logEncodingError(encoding);
            // we're using an unsupported default encoding - assuming NextSTEP
            return NSNEXTSTEPToUnicode(cString,length,resultLength,zone);
        }
    }
}

char *NSString_unicodeToAnyCString(NSStringEncoding encoding, const unichar *characters,NSUInteger length, BOOL lossy,NSUInteger *resultLength,NSZone *zone,BOOL zeroTerminate)
{
    switch(encoding) {
        case NSNEXTSTEPStringEncoding:
            return NSUnicodeToNEXTSTEP(characters,length,lossy,resultLength,zone, zeroTerminate);
        case NSUTF8StringEncoding:
            return NSUnicodeToUTF8(characters,length,lossy,resultLength,zone, zeroTerminate);
        case NSMacOSRomanStringEncoding:
            return NSUnicodeToMacOSRoman(characters,length,lossy,resultLength,zone, zeroTerminate);
        case NSASCIIStringEncoding:
        case NSISOLatin1StringEncoding:
            return NSUnicodeToISOLatin1(characters,length,lossy,resultLength,zone, zeroTerminate);
        case NSISOLatin2StringEncoding:
            return NSUnicodeToISOLatin2(characters,length,lossy,resultLength,zone, zeroTerminate);
        case NSWindowsCP1252StringEncoding:
            return NSUnicodeToWin1252(characters,length,lossy,resultLength,zone, zeroTerminate);
        case NSSymbolStringEncoding:
            return NSUnicodeToSymbol(characters,length,lossy,resultLength,zone, zeroTerminate);
        case NSUnicodeStringEncoding:
            return NSUnicodeToUnicode(characters,length,resultLength,zone, zeroTerminate);
        default: {
            unsigned char *bytes = NSBytesFromUnicode(characters, length, encoding, lossy, resultLength, zone);
            if (bytes) {
                return bytes;
            }
            logEncodingError(encoding);
            // we're using an unsupported default encoding - assuming NextSTEP
            return NSUnicodeToNEXTSTEP(characters,length,lossy,resultLength,zone, zeroTerminate);
        }
    }
}

NSString *NSString_anyCStringNewWithBytes(NSStringEncoding encoding, NSZone *zone, const char *bytes,NSUInteger length)
{
    switch(encoding) {
        case NSNEXTSTEPStringEncoding:
            return NSNEXTSTEPCStringNewWithBytes(zone,bytes,length);
        case NSMacOSRomanStringEncoding:
            return NSString_macOSRomanNewWithBytes(zone,bytes,length);
        case NSASCIIStringEncoding:
        case NSISOLatin1StringEncoding:
            return NSString_isoLatin1NewWithBytes(zone,bytes,length);
        case NSISOLatin2StringEncoding:
            return NSString_isoLatin2NewWithBytes(zone,bytes,length);
        case NSWindowsCP1252StringEncoding:
            return NSString_win1252NewWithBytes(zone,bytes,length);
        default: {
            NSUInteger decodedLength = 0;
            unichar *chars = NSBytesToUnicode(bytes, length, encoding, &decodedLength, zone);
            if (chars) {
                NSString *result = [[NSString allocWithZone:zone] initWithCharacters:chars length:decodedLength];
                NSZoneFree(zone, chars);
                return result;
            }

            logEncodingError(encoding);
            // we're using an unsupported default encoding - assuming NextSTEP
            return NSNEXTSTEPCStringNewWithBytes(zone,bytes,length);
        }
    }
}

// Not sure what's the goal of the method and why we wouldn't just return a NSString made from the unichar
// It seems unused anyway
NSString *NSString_anyCStringNewWithCharacters(NSStringEncoding encoding, NSZone *zone, const unichar *characters,NSUInteger length,BOOL lossy)
{
    switch(encoding) {
        case NSNEXTSTEPStringEncoding:
            return NSNEXTSTEPCStringNewWithCharacters(zone,characters,length, lossy);
        case NSWindowsCP1252StringEncoding:
            return NSWin1252CStringNewWithCharacters(zone,characters,length, lossy);
        case NSMacOSRomanStringEncoding:
            return NSMacOSRomanCStringNewWithCharacters(zone,characters,length, lossy);
        case NSASCIIStringEncoding:
        case NSISOLatin1StringEncoding:
            return NSISOLatin1CStringNewWithCharacters(zone,characters,length, lossy);
        case NSISOLatin2StringEncoding:
            return NSISOLatin2CStringNewWithCharacters(zone,characters,length, lossy);
        default:
            return [[NSString allocWithZone:zone] initWithCharacters:characters length:length];
    }
    return nil;
}

NSUInteger NSGetAnyCStringWithMaxLength(NSStringEncoding encoding, const unichar *characters,NSUInteger length,NSUInteger *location,char *cString,NSUInteger maxLength,BOOL lossy)
{
    if (cString == NULL || maxLength == 0) {
        return NSNotFound;
    }
    
    switch(encoding) {
        case NSNEXTSTEPStringEncoding:
            return NSGetNEXTSTEPCStringWithMaxLength(characters,length, location, cString, maxLength, lossy);
        case NSUnicodeStringEncoding:
            return NSGetUnicodeCStringWithMaxLength(characters,length, location, cString, maxLength);
        case NSISOLatin1StringEncoding:
        case NSASCIIStringEncoding:
            return NSGetISOLatin1CStringWithMaxLength(characters,length, location, cString, maxLength, lossy);
        case NSISOLatin2StringEncoding:
            return NSGetISOLatin2CStringWithMaxLength(characters,length, location, cString, maxLength, lossy);
        case NSMacOSRomanStringEncoding:
            return NSGetMacOSRomanCStringWithMaxLength(characters,length, location, cString, maxLength, lossy);
        case NSWindowsCP1252StringEncoding:
            return NSGetWin1252CStringWithMaxLength(characters,length, location, cString, maxLength, lossy);
        case NSUTF8StringEncoding:
            return NSGetUTF8CStringWithMaxLength(characters,length, location, cString, maxLength);
        default: {
            NSUInteger decodedLength = 0;
            unsigned char *bytes = NSBytesFromUnicode(characters, length, encoding, lossy, &decodedLength, nil);
            if (bytes == NULL) {
                logEncodingError(encoding);
                // we're using an unsupported default encoding - assuming NextSTEP
                return NSGetNEXTSTEPCStringWithMaxLength(characters,length, location, cString, maxLength, lossy);
            } else {
                NSUInteger len = MIN(decodedLength,maxLength);
                memcpy(cString, bytes, len);
                *location = len;
                NSZoneFree(nil, bytes);
                return len;
            }
        }
    }
}

