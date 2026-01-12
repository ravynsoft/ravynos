/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef struct __NSMutableCharacterSet *CFCharacterSetRef;
typedef struct __NSMutableCharacterSet *CFMutableCharacterSetRef;

#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFData.h>

enum {
    kCFCharacterSetControl = 1,
    kCFCharacterSetWhitespace = 2,
    kCFCharacterSetWhitespaceAndNewline = 3,
    kCFCharacterSetDecimalDigit = 4,
    kCFCharacterSetLetter = 5,
    kCFCharacterSetLowercaseLetter = 6,
    kCFCharacterSetUppercaseLetter = 7,
    kCFCharacterSetNonBase = 8,
    kCFCharacterSetDecomposable = 9,
    kCFCharacterSetAlphaNumeric = 10,
    kCFCharacterSetPunctuation = 11,
    kCFCharacterSetIllegal = 12,
    kCFCharacterSetCapitalizedLetter = 13,
    kCFCharacterSetSymbol = 14,
    kCFCharacterSetNewline = 15,
};
typedef CFIndex CFCharacterSetPredefinedSet;

COREFOUNDATION_EXPORT CFTypeID CFCharacterSetGetTypeID(void);

COREFOUNDATION_EXPORT CFCharacterSetRef CFCharacterSetGetPredefined(CFCharacterSetPredefinedSet predefined);

COREFOUNDATION_EXPORT CFCharacterSetRef CFCharacterSetCreateWithBitmapRepresentation(CFAllocatorRef allocator, CFDataRef data);
COREFOUNDATION_EXPORT CFCharacterSetRef CFCharacterSetCreateWithCharactersInRange(CFAllocatorRef allocator, CFRange range);
COREFOUNDATION_EXPORT CFCharacterSetRef CFCharacterSetCreateWithCharactersInString(CFAllocatorRef allocator, CFStringRef string);

COREFOUNDATION_EXPORT CFCharacterSetRef CFCharacterSetCreateCopy(CFAllocatorRef allocator, CFCharacterSetRef self);

COREFOUNDATION_EXPORT Boolean CFCharacterSetHasMemberInPlane(CFCharacterSetRef self, CFIndex plane);
COREFOUNDATION_EXPORT Boolean CFCharacterSetIsCharacterMember(CFCharacterSetRef self, UniChar character);
COREFOUNDATION_EXPORT Boolean CFCharacterSetIsLongCharacterMember(CFCharacterSetRef self, UTF32Char character);
COREFOUNDATION_EXPORT Boolean CFCharacterSetIsSupersetOfSet(CFCharacterSetRef self, CFCharacterSetRef other);

COREFOUNDATION_EXPORT CFDataRef CFCharacterSetCreateBitmapRepresentation(CFAllocatorRef allocator, CFCharacterSetRef self);
COREFOUNDATION_EXPORT CFCharacterSetRef CFCharacterSetCreateInvertedSet(CFAllocatorRef allocator, CFCharacterSetRef self);

// mutable

COREFOUNDATION_EXPORT CFMutableCharacterSetRef CFCharacterSetCreateMutable(CFAllocatorRef alloc);
COREFOUNDATION_EXPORT CFMutableCharacterSetRef CFCharacterSetCreateMutableCopy(CFAllocatorRef allocator, CFCharacterSetRef self);

COREFOUNDATION_EXPORT void CFCharacterSetAddCharactersInRange(CFMutableCharacterSetRef self, CFRange range);
COREFOUNDATION_EXPORT void CFCharacterSetAddCharactersInString(CFMutableCharacterSetRef self, CFStringRef string);
COREFOUNDATION_EXPORT void CFCharacterSetRemoveCharactersInRange(CFMutableCharacterSetRef self, CFRange range);
COREFOUNDATION_EXPORT void CFCharacterSetRemoveCharactersInString(CFMutableCharacterSetRef self, CFStringRef string);
COREFOUNDATION_EXPORT void CFCharacterSetIntersect(CFMutableCharacterSetRef self, CFCharacterSetRef other);
COREFOUNDATION_EXPORT void CFCharacterSetUnion(CFMutableCharacterSetRef self, CFCharacterSetRef other);
COREFOUNDATION_EXPORT void CFCharacterSetInvert(CFMutableCharacterSetRef self);
