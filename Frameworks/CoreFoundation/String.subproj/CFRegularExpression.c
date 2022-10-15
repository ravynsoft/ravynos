// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//

/*	CFRegularExpression.c
	Copyright (c) 2015 Apple Inc. and the Swift project authors
 */

#include <CoreFoundation/CFRegularExpression.h>
#include "CFInternal.h"
#define U_SHOW_DRAFT_API 1
#define U_SHOW_INTERNAL_API 1
#include <unicode/uregex.h>

#define STACK_BUFFER_SIZE 256

struct ___CFRegularExpression {
    CFRuntimeBase _base;
    CFStringRef pattern;
    _CFRegularExpressionOptions options;
    URegularExpression *regex;
    int32_t _checkout;
};

static void ___CFRegularExpressionDeallocate(CFTypeRef cf) {
    struct ___CFRegularExpression *item = (struct ___CFRegularExpression *)cf;
    if (item->regex) uregex_close(item->regex);
    if (item->pattern) CFRelease(item->pattern);
}

static CFTypeID __k_CFRegularExpressionTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass ___CFRegularExpressionClass = {
    _kCFRuntimeScannedObject,
    "_CFRegularExpression",
    NULL,   // init
    NULL,   // copy
    ___CFRegularExpressionDeallocate,
    NULL,
    NULL,
    NULL,
    NULL
};

static void ___CFRegularExpressionInitialize(void) {
    __k_CFRegularExpressionTypeID = _CFRuntimeRegisterClass(&___CFRegularExpressionClass);
}

CFTypeID _CFRegularExpressionGetTypeID(void) {
    if (__k_CFRegularExpressionTypeID == _kCFRuntimeNotATypeID) {
        ___CFRegularExpressionInitialize();
    }
    return __k_CFRegularExpressionTypeID;
}

static struct ___CFRegularExpression *__CFRegularExpressionCreate(CFAllocatorRef allocator) {
    CFIndex size = sizeof(struct ___CFRegularExpression) - sizeof(CFRuntimeBase);
    return (struct ___CFRegularExpression *)_CFRuntimeCreateInstance(allocator, _CFRegularExpressionGetTypeID(), size, NULL);
}

CFStringRef _CFRegularExpressionCreateEscapedPattern(CFStringRef pattern) {
    static CFCharacterSetRef characterSet = NULL;
    static dispatch_once_t once = 0L;
    dispatch_once(&once, ^{
        characterSet = CFCharacterSetCreateWithCharactersInString(kCFAllocatorSystemDefault, CFSTR("*?+[(){}^$|\\./"));
    });
    
    CFRange range = CFRangeMake(0, CFStringGetLength(pattern));
    CFIndex length;

    if (CFStringFindCharacterFromSet(pattern, characterSet, range, 0, &range)) {
        CFMutableStringRef mutableString = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, pattern);
        while (range.length > 0) {
            CFStringInsert(mutableString, range.location, CFSTR("\\"));
            length = CFStringGetLength(mutableString);
            if (range.location + range.length + 1 >= length) {
                break;
            }
            if (!CFStringFindCharacterFromSet(mutableString, characterSet, CFRangeMake(range.location + range.length + 1, length - (range.location + range.length) - 1), 0, &range)) {
                break;
            }
        }
        return mutableString;
    }
    return CFRetain(pattern);
}

_CFRegularExpressionRef _CFRegularExpressionCreate(CFAllocatorRef allocator, CFStringRef pattern, _CFRegularExpressionOptions options, CFErrorRef *errorPtr) {
    UniChar stackBuffer[STACK_BUFFER_SIZE], *patternBuffer = NULL;
    Boolean freePatternBuffer = false;
    uint32_t flags = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    UParseError parseError;
    CFStringRef originalPattern = pattern;
    CFIndex patternLength;
    
    if ((options & _kCFRegularExpressionIgnoreMetacharacters) != 0) {
        pattern = _CFRegularExpressionCreateEscapedPattern(pattern);
    }

    patternLength = CFStringGetLength(pattern);
    patternBuffer = (UniChar *)CFStringGetCharactersPtr(pattern);
    if (!patternBuffer) {
        if (patternLength <= STACK_BUFFER_SIZE) {
            patternBuffer = stackBuffer;
            CFStringGetCharacters(pattern, CFRangeMake(0, patternLength), patternBuffer);
        } else {
            patternBuffer = (UniChar *)malloc(sizeof(UniChar) * patternLength);
            if (patternBuffer) {
                CFStringGetCharacters(pattern, CFRangeMake(0, patternLength), patternBuffer);
                freePatternBuffer = true;
            } else {
                HALT;
            }
        }
    }
    
    if ((options & _kCFRegularExpressionCaseInsensitive) != 0) flags |= UREGEX_CASE_INSENSITIVE;
    if ((options & _kCFRegularExpressionAllowCommentsAndWhitespace) != 0) flags |= UREGEX_COMMENTS;
    if ((options & _kCFRegularExpressionDotMatchesLineSeparators) != 0) flags |= UREGEX_DOTALL;
    if ((options & _kCFRegularExpressionAnchorsMatchLines) != 0) flags |= UREGEX_MULTILINE;
    if ((options & _kCFRegularExpressionUseUnixLineSeparators) != 0) flags |= UREGEX_UNIX_LINES;
    if ((options & _kCFRegularExpressionUseUnicodeWordBoundaries) != 0) flags |= UREGEX_UWORD;
    
    URegularExpression *regex = NULL;
    if (patternLength < INT_MAX) regex = uregex_open((const UChar *)patternBuffer, (int32_t)patternLength, flags, &parseError, &errorCode);
    
    if (regex == NULL || U_FAILURE(errorCode)) {
        // ??? do we need more detailed errors here?
        if (errorPtr) {
            CFStringRef key = CFSTR("NSInvalidValue");
            CFTypeRef keys[] = {
                key
            };
            CFTypeRef values[] = {
                pattern
            };
            
            CFDictionaryRef userInfo = CFDictionaryCreate(kCFAllocatorSystemDefault, (const void **)keys, (const void **)values, sizeof(keys) / sizeof(keys[0]), &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            *errorPtr = CFErrorCreate(kCFAllocatorSystemDefault, CFSTR("NSCocoaErrorDomain"), 2048 /* NSFormattingError*/, userInfo);
            CFRelease(userInfo);
        }
        if (freePatternBuffer) free(patternBuffer);
        if (pattern != originalPattern && pattern != NULL) {
            CFRelease(pattern);
        }
        return NULL;
    }
    
    struct ___CFRegularExpression *regexObj = __CFRegularExpressionCreate(allocator);
    regexObj->regex = regex;
    regexObj->options = options;
    if (pattern != originalPattern) {
        regexObj->pattern = pattern;
    } else if (pattern != NULL) {
        regexObj->pattern = CFStringCreateCopy(kCFAllocatorSystemDefault, pattern);
    } else {
        regexObj->pattern = NULL; // should this be a fatal error?
    }
    
    return regexObj;
}

CFIndex _CFRegularExpressionGetNumberOfCaptureGroups(_CFRegularExpressionRef regex) {
    UErrorCode errorCode = U_ZERO_ERROR;
    return (CFIndex)uregex_groupCount(regex->regex, &errorCode);
}

CFIndex _CFRegularExpressionGetCaptureGroupNumberWithName(_CFRegularExpressionRef regex, CFStringRef groupName) {
    UniChar stackBuffer[STACK_BUFFER_SIZE], *nameBuffer = NULL;
    Boolean freeNameBuffer = false;

    CFIndex nameLength = CFStringGetLength(groupName);
    UErrorCode errorCode = U_ZERO_ERROR;

    nameBuffer = (UniChar *)CFStringGetCharactersPtr(groupName);
    if (!nameBuffer) {
        if (nameLength <= STACK_BUFFER_SIZE) {
            nameBuffer = stackBuffer;
            CFStringGetCharacters(groupName, CFRangeMake(0, nameLength), nameBuffer);
        } else {
            nameBuffer = (UniChar *)malloc(sizeof(UniChar) * nameLength);
            if (nameBuffer) {
                CFStringGetCharacters(groupName, CFRangeMake(0, nameLength), nameBuffer);
                freeNameBuffer = true;
            } else {
                HALT;
            }
        }
    }

    CFIndex idx = uregex_groupNumberFromName(regex->regex, nameBuffer, nameLength, &errorCode);
    if (U_FAILURE(errorCode) || idx < 0) {
        idx = kCFNotFound;
    }

    if (freeNameBuffer) {
        free(nameBuffer);
    }

    return idx;
}

struct regexCallBackContext {
    void *context;
    void (*match)(void *context, CFRange *ranges, CFIndex count, _CFRegularExpressionMatchingFlags flags, Boolean *stop);
    CFIndex anchorIndex;
    Boolean stoppedByClient;
    Boolean hitAnchorLimit;
};

static UBool regexFindProgressCallback(const void *context, int64_t matchIndex) {
    struct regexCallBackContext *ctxt = (struct regexCallBackContext *)context;
    Boolean stop = NO;
    if (ctxt) {
        if (ctxt->anchorIndex != kCFNotFound && matchIndex > ctxt->anchorIndex) {
            stop = true;
            ctxt->hitAnchorLimit = true;
        } else if (ctxt->match) {
            ctxt->match(ctxt->context, NULL, 0, _kCFRegularExpressionMatchingProgress, &stop);
            ctxt->stoppedByClient = stop;
        }
    }
    return stop ? 0 : 1;
}

CF_INLINE URegularExpression *checkOutRegularExpression(void *internal, int32_t *checkout, Boolean *checkedOutRegex) {
    URegularExpression *regex = NULL;
    UErrorCode errorCode = U_ZERO_ERROR;
    Boolean checkedOut = false;
    checkedOut = OSAtomicCompareAndSwap32Barrier(0, 1, (volatile int32_t *)checkout);
    if (checkedOut) {
        regex = (URegularExpression *)internal;
    } else {
        regex = uregex_clone((const URegularExpression *)internal, &errorCode);
    }
    *checkedOutRegex = checkedOut;
    return regex;
}

static UBool regexMatchCallback(const void *context, int32_t steps) {
    struct regexCallBackContext *ctxt = (struct regexCallBackContext *)context;
    Boolean stop = false;
    if (ctxt) {
        ctxt->match(ctxt->context, NULL, 0, _kCFRegularExpressionMatchingProgress, &stop);
        ctxt->stoppedByClient = stop;
    }
    return stop ? 0 : 1;
}


CF_INLINE URegularExpression *prepareRegularExpression(void *internal, int32_t *checkout, CFStringRef string, CFRange range, UniChar *stackBuffer, const void *context, Boolean reportProgress, Boolean anchored, Boolean transparentBounds, Boolean nonAnchoringBounds, CFIndex *offset, void **bufferToFree, void **utextToFree, Boolean *checkedOutRegex) {
    // ??? consider reusing utext
    URegularExpression *regex = NULL;
    CFIndex length = CFStringGetLength(string);
    int64_t regionStart = 0, regionLimit = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    CFRange enclosingRange;
    UniChar *stringBuffer = NULL;
    int32_t textLength = length;
 
    if (range.location + range.length > length || range.location >= INT_MAX) return NULL;
    if (range.location + range.length > INT_MAX) range.length = INT_MAX - range.location;
    
    if (range.location + range.length <= INT_MAX) stringBuffer = (UniChar *)CFStringGetCharactersPtr(string);
    if (stringBuffer) {
        regionStart = (int64_t)range.location;
        regionLimit = (int64_t)(range.location + range.length);
        *offset = 0;
    } else {
        enclosingRange = range;
        if (transparentBounds) {
            enclosingRange = CFRangeMake(0, length);
        } else if (nonAnchoringBounds) {
            if (enclosingRange.location > 0) {
                enclosingRange.location--;
                enclosingRange.length++;
            }
            if (enclosingRange.location + enclosingRange.length < length) enclosingRange.length++;
        }
        if ((transparentBounds || nonAnchoringBounds) && enclosingRange.length > INT_MAX) {
            CFIndex dist = (INT_MAX - range.length) / 2;
            if (dist > range.location) dist = range.location;
            enclosingRange.location = range.location - dist;
            enclosingRange.length = INT_MAX;
        }
        regionStart = (int64_t)(range.location - enclosingRange.location);
        regionLimit = (int64_t)((range.location + range.length) - enclosingRange.location);
        *offset = enclosingRange.location;
        if (enclosingRange.length <= STACK_BUFFER_SIZE) {
            stringBuffer = stackBuffer;
            if (enclosingRange.length > 0) {
                CFStringGetCharacters(string, enclosingRange, stringBuffer);
            }
        } else {
            stringBuffer = (UniChar *)malloc(sizeof(UniChar) * enclosingRange.length);
            if (stringBuffer) {
                CFStringGetCharacters(string, enclosingRange, stringBuffer);
                *bufferToFree = stringBuffer;
            }
        }
        textLength = enclosingRange.length;
    }
    
    if (stringBuffer) {
        regex = checkOutRegularExpression(internal, checkout, checkedOutRegex);
        uregex_setText(regex, (const UChar *)stringBuffer, textLength, &errorCode);
    }
    
    if (regex) {
        uregex_setRegion64(regex, regionStart, regionLimit, &errorCode);
        if (reportProgress) uregex_setMatchCallback(regex, regexMatchCallback, context, &errorCode);
        if (reportProgress || anchored) uregex_setFindProgressCallback(regex, (void *)regexFindProgressCallback, context, &errorCode);
        if (transparentBounds) uregex_useTransparentBounds(regex, 1, &errorCode);
        if (nonAnchoringBounds) uregex_useAnchoringBounds(regex, 0, &errorCode);
        
        if (U_FAILURE(errorCode)) {
            uregex_setText(regex, (const UChar *)stackBuffer, 0, &errorCode);
            if (reportProgress) uregex_setMatchCallback(regex, NULL, NULL, &errorCode);
            if (reportProgress || anchored) uregex_setFindProgressCallback(regex, NULL, NULL, &errorCode);
            if (transparentBounds) uregex_useTransparentBounds(regex, 0, &errorCode);
            if (nonAnchoringBounds) uregex_useAnchoringBounds(regex, 1, &errorCode);
            regex = NULL;
        }
    }
    
    return regex;
}

CF_INLINE _CFRegularExpressionMatchingFlags flagsForRegularExpression(URegularExpression *regex) {
    _CFRegularExpressionMatchingFlags flags = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    BOOL hitEnd = uregex_hitEnd(regex, &errorCode), requireEnd = uregex_requireEnd(regex, &errorCode);
    if (U_SUCCESS(errorCode)) {
        if (hitEnd) flags |= _kCFRegularExpressionMatchingHitEnd;
        if (requireEnd) flags |= _kCFRegularExpressionMatchingRequiredEnd;
    }
    return flags;
}


CF_INLINE void returnRegularExpression(URegularExpression *regex, int32_t *checkout, Boolean checkedOutRegex, Boolean reportProgress, Boolean anchored, Boolean transparentBounds, Boolean nonAnchoringBounds, UniChar *stackBuffer, void *bufferToFree, void *utextToFree) {
    UErrorCode errorCode = U_ZERO_ERROR;
    if (regex) {
        if (checkedOutRegex) {
            uregex_setText(regex, (const UChar *)stackBuffer, 0, &errorCode);
            if (reportProgress) uregex_setMatchCallback(regex, NULL, NULL, &errorCode);
            if (reportProgress || anchored) uregex_setFindProgressCallback(regex, NULL, NULL, &errorCode);
            if (transparentBounds) uregex_useTransparentBounds(regex, 0, &errorCode);
            if (nonAnchoringBounds) uregex_useAnchoringBounds(regex, 1, &errorCode);
            OSMemoryBarrier();
            *checkout = 0;
        } else {
            uregex_close(regex);
        }
    }
    if (bufferToFree) free(bufferToFree);
}

void _CFRegularExpressionEnumerateMatchesInString(_CFRegularExpressionRef regexObj, CFStringRef string, _CFRegularExpressionMatchingOptions options, CFRange range, void *matchContext, _CFRegularExpressionMatch match) {
    URegularExpression *regex = NULL;
    UniChar stackBuffer[STACK_BUFFER_SIZE];
    void *bufferToFree = NULL, *utextToFree = NULL;
    struct regexCallBackContext context;
    CFIndex offset, length = CFStringGetLength(string);
    _CFRegularExpressionMatchingFlags flags;
    Boolean checkedOutRegex = true;
    Boolean stop = false;
    Boolean reportProgress = ((options & _kCFRegularExpressionMatchingReportProgress) != 0);
    Boolean reportCompletion = ((options & _kCFRegularExpressionMatchingReportCompletion) != 0);
    Boolean anchored = ((options & _kCFRegularExpressionMatchingAnchored) != 0);
    Boolean transparentBounds = ((options & _kCFRegularExpressionMatchingWithTransparentBounds) != 0);
    Boolean nonAnchoringBounds = ((options & _kCFRegularExpressionMatchingWithoutAnchoringBounds) != 0);
    Boolean omitResult = ((options & _kCFRegularExpressionMatchingOmitResult) != 0);
    
    UErrorCode errorCode = U_ZERO_ERROR;

    context.context = matchContext;
    context.match = match;
    context.anchorIndex = anchored ? range.location : kCFNotFound;
    context.stoppedByClient = NO;
    context.hitAnchorLimit = NO;
    
    regex = prepareRegularExpression(regexObj->regex, (int32_t *)&regexObj->_checkout, string, range, stackBuffer, (const void *)&context, reportProgress, anchored, transparentBounds, nonAnchoringBounds, &offset, &bufferToFree, &utextToFree, &checkedOutRegex);
    CFIndex numberOfCaptureGroups = _CFRegularExpressionGetNumberOfCaptureGroups(regexObj);
    if (regex) {
        while (uregex_findNext(regex, &errorCode) && U_SUCCESS(errorCode) && !stop && !context.stoppedByClient && !context.hitAnchorLimit) {
            if (anchored) {
                if (uregex_start64(regex, 0, &errorCode) > (int64_t)context.anchorIndex) break;
                context.anchorIndex = (CFIndex)uregex_end64(regex, 0, &errorCode);
            }
            flags = flagsForRegularExpression(regex);
            
            if (!omitResult) {
                CFRange stack_ranges[7];
                CFRange *ranges = &stack_ranges[0];
                if (numberOfCaptureGroups + 1 > sizeof(stack_ranges) / sizeof(stack_ranges[0])) {
                    ranges = (CFRange *)malloc(sizeof(CFRange) * (numberOfCaptureGroups + 1));
                }
                CFIndex rangeCount = 0;
                for (int i = 0; i <= numberOfCaptureGroups; i++) {
                    UErrorCode errorCode = U_ZERO_ERROR;
                    int64_t start = uregex_start64(regex, (int32_t)i, &errorCode);
                    int64_t end = uregex_end64(regex, (int32_t)i, &errorCode);
                    CFRange matchedRange;
                    if (U_SUCCESS(errorCode) && start >= 0 && end >= start) {
                        matchedRange = CFRangeMake(offset + start, end - start);
                    } else {
                        matchedRange = CFRangeMake(kCFNotFound, 0);
                    }
                    ranges[i] = matchedRange;
                    rangeCount++;
                }
                if (rangeCount > 0) {
                    match(matchContext, ranges, rangeCount, flags, &stop);
                } else {
                    match(matchContext, NULL, 0, flags, &stop);
                }
                if (ranges != &stack_ranges[0]) {
                    free(ranges);
                }
            } else {
                match(matchContext, NULL, 0, flags, &stop);
            }
            if (stop) break;
        }
    }

    if (reportCompletion && !stop && !context.stoppedByClient) {
        if (regex && (U_SUCCESS(errorCode) || context.hitAnchorLimit)) {
            flags = flagsForRegularExpression(regex);
        } else {
            flags = _kCFRegularExpressionMatchingInternalError;
        }
        flags |= _kCFRegularExpressionMatchingCompleted;
        match(matchContext, NULL, 0, flags, &stop);
    }

    returnRegularExpression(regex, (int32_t *)&regexObj->_checkout, checkedOutRegex, reportProgress, anchored, transparentBounds, nonAnchoringBounds, stackBuffer, bufferToFree, utextToFree);
}

CFStringRef _CFRegularExpressionGetPattern(_CFRegularExpressionRef regex) {
    return regex->pattern;
}

_CFRegularExpressionOptions _CFRegularExpressionGetOptions(_CFRegularExpressionRef regex) {
    return regex->options;
}
