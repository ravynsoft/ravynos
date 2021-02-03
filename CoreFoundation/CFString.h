/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef const struct __CFString *CFStringRef;
typedef struct __CFString *CFMutableStringRef;

#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFArray.h>
#import <CoreFoundation/CFDictionary.h>
#import <CoreFoundation/CFLocale.h>
#import <CoreFoundation/CFData.h>
#import <CoreFoundation/CFCharacterSet.h>
#include <stdarg.h>

typedef CFOptionFlags CFStringCompareFlags;

typedef CFUInteger CFStringEncoding;

enum {
    kCFCompareCaseInsensitive = (1 << 0),
    // no 2
    kCFCompareBackwards = (1 << 2),
    kCFCompareAnchored = (1 << 3),
    kCFCompareNonliteral = (1 << 4),
    kCFCompareLocalized = (1 << 5),
    kCFCompareNumerically = (1 << 6),
    kCFCompareDiacriticInsensitive = (1 << 7),
    kCFCompareWidthInsensitive = (1 << 8),
    kCFCompareForcedOrdering = (1 << 9),
};

#define kCFStringEncodingInvalidId (0xffffffffU)

typedef enum {
    kCFStringEncodingUTF8 = 0x08000100,
    kCFStringEncodingUTF16 = 0x00000100,
    kCFStringEncodingUTF16BE = 0x10000100,
    kCFStringEncodingUTF16LE = 0x14000100,
    kCFStringEncodingUTF32 = 0x0c000100,
    kCFStringEncodingUTF32BE = 0x18000100,
    kCFStringEncodingUTF32LE = 0x1c000100,

    kCFStringEncodingMacRoman = 0,
    kCFStringEncodingWindowsLatin1 = 0x0500,
    kCFStringEncodingISOLatin1 = 0x0201,
    kCFStringEncodingNextStepLatin = 0x0B01,
    kCFStringEncodingASCII = 0x0600,
    kCFStringEncodingUnicode = kCFStringEncodingUTF16,
    kCFStringEncodingNonLossyASCII = 0x0BFF,

    // A few more supported encoding - at least -[NSString initWithBytes:length:encoding:] can work with them
    kCFStringEncodingDOSThai = 0x041D, /* CP874, also for Windows */
    kCFStringEncodingDOSJapanese = 0x0420, /* CP932, also for Windows */
    kCFStringEncodingDOSChineseSimplif = 0x0421, /* CP936, also for Windows */
    kCFStringEncodingDOSKorean = 0x0422, /* CP949, also for Windows; Unified Hangul Code */
    kCFStringEncodingDOSChineseTrad = 0x0423, /* CP950, also for Windows */
    kCFStringEncodingWindowsLatin2 = 0x0501, /* CP1250, Central Europe */
    kCFStringEncodingWindowsCyrillic = 0x0502, /* CP1251, Slavic Cyrillic */
    kCFStringEncodingWindowsGreek = 0x0503, /* CP1253 */
    kCFStringEncodingWindowsLatin5 = 0x0504, /* CP1254, Turkish */
    kCFStringEncodingWindowsHebrew = 0x0505, /* CP1255 */
    kCFStringEncodingWindowsArabic = 0x0506, /* CP1256 */
    kCFStringEncodingWindowsBalticRim = 0x0507, /* CP1257 */
    kCFStringEncodingWindowsVietnamese = 0x0508, /* CP1258 */

} CFStringBuiltInEncodings;

typedef struct CFStringInlineBuffer {
    int nothing;
} CFStringInlineBuffer;

COREFOUNDATION_EXPORT CFTypeID CFStringGetTypeID(void);

COREFOUNDATION_EXPORT CFStringEncoding CFStringGetSystemEncoding(void);
COREFOUNDATION_EXPORT const CFStringEncoding *CFStringGetListOfAvailableEncodings(void);
COREFOUNDATION_EXPORT Boolean CFStringIsEncodingAvailable(CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFStringRef CFStringGetNameOfEncoding(CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFStringEncoding CFStringGetMostCompatibleMacStringEncoding(CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFIndex CFStringGetMaximumSizeForEncoding(CFIndex length, CFStringEncoding encoding);

#ifdef __OBJC__
#define CFSTR(s) (CFStringRef)(@s)
#else
COREFOUNDATION_EXPORT CFStringRef CFStringMakeConstant(const char *cString);
#define CFSTR(s) CFStringMakeConstant(s)
#endif

COREFOUNDATION_EXPORT void CFStringAppendCharacters(CFMutableStringRef mutableString, const UniChar *chars, CFIndex numChars);

COREFOUNDATION_EXPORT CFStringRef CFStringCreateByCombiningStrings(CFAllocatorRef allocator, CFArrayRef array, CFStringRef separator);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateCopy(CFAllocatorRef allocator, CFStringRef self);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateMutableCopy(CFAllocatorRef allocator, CFIndex maxLength, CFStringRef self);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithBytes(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length, CFStringEncoding encoding, Boolean isExternalRepresentation);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithBytesNoCopy(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length, CFStringEncoding encoding, Boolean isExternalRepresentation, CFAllocatorRef contentsDeallocator);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithCharacters(CFAllocatorRef allocator, const UniChar *chars, CFIndex length);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithCharactersNoCopy(CFAllocatorRef allocator, const UniChar *chars, CFIndex length, CFAllocatorRef contentsDeallocator);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithCString(CFAllocatorRef allocator, const char *cString, CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithCStringNoCopy(CFAllocatorRef allocator, const char *cString, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithFileSystemRepresentation(CFAllocatorRef allocator, const char *buffer);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithFormat(CFAllocatorRef allocator, CFDictionaryRef formatOptions, CFStringRef format, ...);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithFormatAndArguments(CFAllocatorRef allocator, CFDictionaryRef formatOptions, CFStringRef format, va_list arguments);
COREFOUNDATION_EXPORT CFStringRef CFStringCreateFromExternalRepresentation(CFAllocatorRef allocator, CFDataRef data, CFStringEncoding encoding);

COREFOUNDATION_EXPORT CFStringRef CFStringCreateWithSubstring(CFAllocatorRef allocator, CFStringRef self, CFRange range);

COREFOUNDATION_EXPORT void CFShow(CFTypeRef self);
COREFOUNDATION_EXPORT void CFShowStr(CFStringRef self);

COREFOUNDATION_EXPORT CFComparisonResult CFStringCompare(CFStringRef self, CFStringRef other, CFOptionFlags options);
COREFOUNDATION_EXPORT CFComparisonResult CFStringCompareWithOptions(CFStringRef self, CFStringRef other, CFRange rangeToCompare, CFOptionFlags options);
COREFOUNDATION_EXPORT CFComparisonResult CFStringCompareWithOptionsAndLocale(CFStringRef self, CFStringRef other, CFRange rangeToCompare, CFOptionFlags options, CFLocaleRef locale);

COREFOUNDATION_EXPORT CFStringRef CFStringConvertEncodingToIANACharSetName(CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFUInteger CFStringConvertEncodingToNSStringEncoding(CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFUInteger CFStringConvertEncodingToWindowsCodepage(CFStringEncoding encoding);

COREFOUNDATION_EXPORT CFStringEncoding CFStringConvertIANACharSetNameToEncoding(CFStringRef self);
COREFOUNDATION_EXPORT CFStringEncoding CFStringConvertNSStringEncodingToEncoding(CFUInteger encoding);
COREFOUNDATION_EXPORT CFStringEncoding CFStringConvertWindowsCodepageToEncoding(CFUInteger codepage);
COREFOUNDATION_EXPORT CFArrayRef CFStringCreateArrayBySeparatingStrings(CFAllocatorRef allocator, CFStringRef self, CFStringRef separatorString);
COREFOUNDATION_EXPORT CFArrayRef CFStringCreateArrayWithFindResults(CFAllocatorRef allocator, CFStringRef self, CFStringRef stringToFind, CFRange range, CFOptionFlags options);
COREFOUNDATION_EXPORT CFDataRef CFStringCreateExternalRepresentation(CFAllocatorRef allocator, CFStringRef self, CFStringEncoding encoding, uint8_t lossByte);
COREFOUNDATION_EXPORT void CFStringDelete(CFMutableStringRef theString, CFRange range);

COREFOUNDATION_EXPORT Boolean CFStringHasPrefix(CFStringRef self, CFStringRef prefix);
COREFOUNDATION_EXPORT Boolean CFStringHasSuffix(CFStringRef self, CFStringRef suffix);
COREFOUNDATION_EXPORT CFRange CFStringFind(CFStringRef self, CFStringRef stringToFind, CFOptionFlags options);
COREFOUNDATION_EXPORT Boolean CFStringFindCharacterFromSet(CFStringRef self, CFCharacterSetRef set, CFRange range, CFOptionFlags options, CFRange *result);
COREFOUNDATION_EXPORT Boolean CFStringFindWithOptions(CFStringRef self, CFStringRef stringToFind, CFRange range, CFOptionFlags options, CFRange *result);
COREFOUNDATION_EXPORT Boolean CFStringFindWithOptionsAndLocale(CFStringRef self, CFStringRef stringToFind, CFRange range, CFOptionFlags options, CFLocaleRef locale, CFRange *result);
COREFOUNDATION_EXPORT CFIndex CFStringGetBytes(CFStringRef self, CFRange range, CFStringEncoding encoding, uint8_t lossByte, Boolean isExternalRepresentation, uint8_t *bytes, CFIndex length, CFIndex *resultLength);

COREFOUNDATION_EXPORT CFIndex CFStringGetLength(CFStringRef self);
COREFOUNDATION_EXPORT UniChar CFStringGetCharacterAtIndex(CFStringRef self, CFIndex index);

COREFOUNDATION_EXPORT void CFStringGetCharacters(CFStringRef self, CFRange range, UniChar *buffer);
COREFOUNDATION_EXPORT const UniChar *CFStringGetCharactersPtr(CFStringRef self);

COREFOUNDATION_EXPORT Boolean CFStringGetCString(CFStringRef self, char *buffer, CFIndex bufferSize, CFStringEncoding encoding);
COREFOUNDATION_EXPORT const char *CFStringGetCStringPtr(CFStringRef self, CFStringEncoding encoding);

COREFOUNDATION_EXPORT void CFStringInitInlineBuffer(CFStringRef self, CFStringInlineBuffer *buffer, CFRange range);
COREFOUNDATION_EXPORT void CFStringInsert(CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr);
COREFOUNDATION_EXPORT UniChar CFStringGetCharacterFromInlineBuffer(CFStringInlineBuffer *buffer, CFIndex index);

COREFOUNDATION_EXPORT CFInteger CFStringGetIntValue(CFStringRef self);
COREFOUNDATION_EXPORT double CFStringGetDoubleValue(CFStringRef self);
COREFOUNDATION_EXPORT CFStringEncoding CFStringGetFastestEncoding(CFStringRef self);
COREFOUNDATION_EXPORT CFStringEncoding CFStringGetSmallestEncoding(CFStringRef self);

COREFOUNDATION_EXPORT CFIndex CFStringGetMaximumSizeOfFileSystemRepresentation(CFStringRef self);
COREFOUNDATION_EXPORT Boolean CFStringGetFileSystemRepresentation(CFStringRef self, char *buffer, CFIndex bufferCapacity);

COREFOUNDATION_EXPORT void CFStringGetLineBounds(CFStringRef self, CFRange range, CFIndex *beginIndex, CFIndex *endIndex, CFIndex *contentsEndIndex);
COREFOUNDATION_EXPORT void CFStringGetParagraphBounds(CFStringRef self, CFRange range, CFIndex *beginIndex, CFIndex *endIndex, CFIndex *contentsEndIndex);
COREFOUNDATION_EXPORT CFRange CFStringGetRangeOfComposedCharactersAtIndex(CFStringRef self, CFIndex index);
