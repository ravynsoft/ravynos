/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFString.h>
#import <CoreFoundation/CFDictionary.h>

typedef struct __NSMutableAttributedString *CFAttributedStringRef;
typedef struct __NSMutableAttributedString *CFMutableAttributedStringRef;

COREFOUNDATION_EXPORT CFTypeID CFAttributedStringGetTypeID(void);

COREFOUNDATION_EXPORT CFAttributedStringRef CFAttributedStringCreate(CFAllocatorRef allocator, CFStringRef string, CFDictionaryRef attributes);
COREFOUNDATION_EXPORT CFAttributedStringRef CFAttributedStringCreateWithSubstring(CFAllocatorRef allocator, CFAttributedStringRef self, CFRange range);

COREFOUNDATION_EXPORT CFAttributedStringRef CFAttributedStringCreateCopy(CFAllocatorRef allocator, CFAttributedStringRef self);

COREFOUNDATION_EXPORT CFIndex CFAttributedStringGetLength(CFAttributedStringRef self);
COREFOUNDATION_EXPORT CFStringRef CFAttributedStringGetString(CFAttributedStringRef self);

COREFOUNDATION_EXPORT CFTypeRef CFAttributedStringGetAttribute(CFAttributedStringRef self, CFIndex location, CFStringRef name, CFRange *effectiveRange);
COREFOUNDATION_EXPORT CFTypeRef CFAttributedStringGetAttributeAndLongestEffectiveRange(CFAttributedStringRef self, CFIndex location, CFStringRef name, CFRange range, CFRange *longestEffectiveRange);
COREFOUNDATION_EXPORT CFDictionaryRef CFAttributedStringGetAttributes(CFAttributedStringRef self, CFIndex location, CFRange *effectiveRange);
COREFOUNDATION_EXPORT CFDictionaryRef CFAttributedStringGetAttributesAndLongestEffectiveRange(CFAttributedStringRef self, CFIndex location, CFRange range, CFRange *longestEffectiveRange);
// mutable

COREFOUNDATION_EXPORT CFMutableAttributedStringRef CFAttributedStringCreateMutable(CFAllocatorRef allocator, CFIndex maxLength);

COREFOUNDATION_EXPORT CFMutableAttributedStringRef CFAttributedStringCreateMutableCopy(CFAllocatorRef allocator, CFIndex maxLength, CFAttributedStringRef self);

COREFOUNDATION_EXPORT CFMutableStringRef CFAttributedStringGetMutableString(CFMutableAttributedStringRef self);

COREFOUNDATION_EXPORT void CFAttributedStringRemoveAttribute(CFMutableAttributedStringRef self, CFRange range, CFStringRef name);
COREFOUNDATION_EXPORT void CFAttributedStringReplaceAttributedString(CFMutableAttributedStringRef self, CFRange range, CFAttributedStringRef replacement);
COREFOUNDATION_EXPORT void CFAttributedStringReplaceString(CFMutableAttributedStringRef self, CFRange range, CFStringRef replacement);
COREFOUNDATION_EXPORT void CFAttributedStringSetAttribute(CFMutableAttributedStringRef self, CFRange range, CFStringRef name, CFTypeRef value);
COREFOUNDATION_EXPORT void CFAttributedStringSetAttributes(CFMutableAttributedStringRef self, CFRange range, CFDictionaryRef replacement, Boolean clearPreviousAttributes);

COREFOUNDATION_EXPORT void CFAttributedStringBeginEditing(CFMutableAttributedStringRef self);
COREFOUNDATION_EXPORT void CFAttributedStringEndEditing(CFMutableAttributedStringRef self);
