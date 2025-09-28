/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct CFStringTokenizer *CFStringTokenizerRef;

typedef CFOptionFlags CFStringTokenizerTokenType;

COREFOUNDATION_EXPORT CFTypeID CFStringTokenizerGetTypeID(void);

COREFOUNDATION_EXPORT CFOptionFlags CFStringTokenizerGetSupportedOptionsForLanguage(CFStringRef language);

COREFOUNDATION_EXPORT CFStringTokenizerRef CFStringTokenizerCreate(CFAllocatorRef allocator, CFStringRef string, CFRange range, CFOptionFlags options, CFLocaleRef locale);

COREFOUNDATION_EXPORT CFStringTokenizerTokenType CFStringTokenizerAdvanceToNextToken(CFStringTokenizerRef self);
COREFOUNDATION_EXPORT CFStringRef CFStringTokenizerCopyBestStringLanguage(CFStringRef string, CFRange range);
COREFOUNDATION_EXPORT CFTypeRef CFStringTokenizerCopyCurrentTokenAttribute(CFStringTokenizerRef self, CFOptionFlags attribute);
COREFOUNDATION_EXPORT CFIndex CFStringTokenizerGetCurrentSubTokens(CFStringTokenizerRef self, CFRange *ranges, CFIndex maxRangeLength, CFMutableArrayRef subTokens);
COREFOUNDATION_EXPORT CFRange CFStringTokenizerGetCurrentTokenRange(CFStringTokenizerRef self);
COREFOUNDATION_EXPORT CFStringTokenizerTokenType CFStringTokenizerGoToTokenAtIndex(CFStringTokenizerRef self, CFIndex index);
COREFOUNDATION_EXPORT void CFStringTokenizerSetString(CFStringTokenizerRef self, CFStringRef string, CFRange range);
