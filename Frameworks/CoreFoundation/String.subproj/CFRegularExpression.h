// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//

/*	CFRegularExpression.h
	Copyright (c) 2015 Apple Inc. and the Swift project authors
 */

#if !defined(__COREFOUNDATION_CFREGULAREXPRESSION__)
#define __COREFOUNDATION_CFREGULAREXPRESSION__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFError.h>

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

typedef CF_OPTIONS(CFOptionFlags, _CFRegularExpressionOptions) {
    _kCFRegularExpressionCaseInsensitive             = 1 << 0,
    _kCFRegularExpressionAllowCommentsAndWhitespace  = 1 << 1,
    _kCFRegularExpressionIgnoreMetacharacters        = 1 << 2,
    _kCFRegularExpressionDotMatchesLineSeparators    = 1 << 3,
    _kCFRegularExpressionAnchorsMatchLines           = 1 << 4,
    _kCFRegularExpressionUseUnixLineSeparators       = 1 << 5,
    _kCFRegularExpressionUseUnicodeWordBoundaries    = 1 << 6
};

typedef CF_OPTIONS(CFOptionFlags, _CFRegularExpressionMatchingOptions) {
    _kCFRegularExpressionMatchingReportProgress         = 1 << 0,
    _kCFRegularExpressionMatchingReportCompletion       = 1 << 1,
    _kCFRegularExpressionMatchingAnchored               = 1 << 2,
    _kCFRegularExpressionMatchingWithTransparentBounds  = 1 << 3,
    _kCFRegularExpressionMatchingWithoutAnchoringBounds = 1 << 4,
    _kCFRegularExpressionMatchingOmitResult             = 1 << 13
};

typedef CF_OPTIONS(CFOptionFlags, _CFRegularExpressionMatchingFlags) {
    _kCFRegularExpressionMatchingProgress               = 1 << 0,       /* Set when the block is called to report progress during a long-running match operation. */
    _kCFRegularExpressionMatchingCompleted              = 1 << 1,       /* Set when the block is called after completion of any matching. */
    _kCFRegularExpressionMatchingHitEnd                 = 1 << 2,       /* Set when the current match operation reached the end of the search range. */
    _kCFRegularExpressionMatchingRequiredEnd            = 1 << 3,       /* Set when the current match depended on the location of the end of the search range. */
    _kCFRegularExpressionMatchingInternalError          = 1 << 4        /* Set when matching failed due to an internal error. */
};

typedef const struct CF_BRIDGED_TYPE(_NSCFRegex) ___CFRegularExpression * _CFRegularExpressionRef;

typedef void (*_CFRegularExpressionMatch)(void *_Nullable context, CFRange * _Nullable ranges, CFIndex count, _CFRegularExpressionMatchingFlags flags, Boolean *stop);

CFStringRef _CFRegularExpressionCreateEscapedPattern(CFStringRef pattern);
_CFRegularExpressionRef _Nullable _CFRegularExpressionCreate(CFAllocatorRef allocator, CFStringRef pattern, _CFRegularExpressionOptions options, CFErrorRef *errorPtr);
void _CFRegularExpressionDestroy(_CFRegularExpressionRef regex);

CFIndex _CFRegularExpressionGetNumberOfCaptureGroups(_CFRegularExpressionRef regex);
CFIndex _CFRegularExpressionGetCaptureGroupNumberWithName(_CFRegularExpressionRef regex, CFStringRef groupName);
void _CFRegularExpressionEnumerateMatchesInString(_CFRegularExpressionRef regexObj, CFStringRef string, _CFRegularExpressionMatchingOptions options, CFRange range, void *_Nullable context, _CFRegularExpressionMatch match);

CFStringRef _CFRegularExpressionGetPattern(_CFRegularExpressionRef regex);
_CFRegularExpressionOptions _CFRegularExpressionGetOptions(_CFRegularExpressionRef regex);

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END
#endif /* __COREFOUNDATION_CFREGULAREXPRESSION__ */
