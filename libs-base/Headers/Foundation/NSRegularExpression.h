
/* Definition of class NSRegularExpression
   Copyright (C) 2011 Free Software Foundation, Inc.

   This file is part of the GNUstep Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSRegularExpression_h_GNUSTEP_BASE_INCLUDE
#define _NSRegularExpression_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import	<Foundation/NSObject.h>
#import "GNUstepBase/GSBlocks.h"
#import "GNUstepBase/GSConfig.h"

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString, NSTextCheckingResult;

typedef NSUInteger NSRegularExpressionOptions;
static const NSRegularExpressionOptions
  NSRegularExpressionCaseInsensitive             = 1<<0;
static const NSRegularExpressionOptions
  NSRegularExpressionAllowCommentsAndWhitespace  = 1<<1;
static const NSRegularExpressionOptions
  NSRegularExpressionIgnoreMetacharacters        = 1<<2;
static const NSRegularExpressionOptions
  NSRegularExpressionDotMatchesLineSeparators    = 1<<3;
static const NSRegularExpressionOptions
  NSRegularExpressionAnchorsMatchLines           = 1<<4;
static const NSRegularExpressionOptions
  NSRegularExpressionUseUnixLineSeparators       = 1<<5;
static const NSRegularExpressionOptions
  NSRegularExpressionUseUnicodeWordBoundaries    = 1<<6;

typedef NSUInteger NSMatchingFlags;
static const NSMatchingFlags NSMatchingProgress      = 1<<0;
static const NSMatchingFlags NSMatchingCompleted     = 1<<1;
static const NSMatchingFlags NSMatchingHitEnd        = 1<<2;
static const NSMatchingFlags NSMatchingRequiredEnd   = 1<<3;
static const NSMatchingFlags NSMatchingInternalError = 1<<4;

typedef NSUInteger NSMatchingOptions;
static const NSMatchingOptions NSMatchingReportProgress         = 1<<0;
static const NSMatchingOptions NSMatchingReportCompletion       = 1<<1;
static const NSMatchingOptions NSMatchingAnchored               = 1<<2;
static const NSMatchingOptions NSMatchingWithTransparentBounds  = 1<<3;
static const NSMatchingOptions NSMatchingWithoutAnchoringBounds = 1<<4;


DEFINE_BLOCK_TYPE(GSRegexBlock, void, NSTextCheckingResult*,
  NSMatchingFlags, BOOL*);

#ifndef GSREGEXTYPE
#  define GSREGEXTYPE void
#endif
/**
 * NSRegularExpression is used to inspect and manipulate strings using regular
 * expressions. The interface is thread safe: The same NSRegularExpression
 * object may be used to concurrently perform matching on multiple threads.
 *
 * To guard against regular expressions with extremely poor performance, the
 * underlying matcher will abort after a certain number of steps. This is
 * controlled using the GSRegularExpressionWorkLimit user default. The value of
 * this default key represents the number of steps executed by the match engine,
 * so it is only indirectly correlated with the time taken to execute the
 * pattern, but it usually in the order of milliseconds. The preset 1500,
 * setting value to 0 disables the work limit.
 */
GS_EXPORT_CLASS
@interface NSRegularExpression : NSObject <NSCoding, NSCopying>
{
#if	GS_EXPOSE(NSRegularExpression)
  @private
  GSREGEXTYPE *regex;
  NSRegularExpressionOptions options;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}
// GNUstep, like OS X, uses libicu to provide the NSRegularExpression
// implementation.  If you have configured GNUstep without this support then it
// will not work, so these methods are hidden.
#if GS_USE_ICU || GS_UNSAFE_REGEX
+ (NSRegularExpression*) regularExpressionWithPattern: (NSString*)aPattern
  options: (NSRegularExpressionOptions)opts
  error: (NSError**)e;
- (id) initWithPattern: (NSString*)aPattern
	       options: (NSRegularExpressionOptions)opts
		 error: (NSError**)e;
+ (NSRegularExpression*) regularExpressionWithPattern: (NSString*)aPattern
  options: (NSRegularExpressionOptions)opts
  error: (NSError**)e;
- (id) initWithPattern: (NSString*)aPattern
	       options: (NSRegularExpressionOptions)opts
		 error: (NSError**)e;
- (NSString*) pattern;
- (void) enumerateMatchesInString: (NSString*)string
                          options: (NSMatchingOptions)options
                            range: (NSRange)range
                       usingBlock: (GSRegexBlock)block;
- (NSUInteger) numberOfMatchesInString: (NSString*)string
                               options: (NSMatchingOptions)options
                                 range: (NSRange)range;

- (NSTextCheckingResult*) firstMatchInString: (NSString*)string
                                     options: (NSMatchingOptions)options
                                       range: (NSRange)range;
- (NSArray*) matchesInString: (NSString*)string
                     options: (NSMatchingOptions)options
                       range: (NSRange)range;
- (NSRange) rangeOfFirstMatchInString: (NSString*)string
                              options: (NSMatchingOptions)options
                                range: (NSRange)range;
- (NSUInteger) replaceMatchesInString: (NSMutableString*)string
                              options: (NSMatchingOptions)options
                                range: (NSRange)range
                         withTemplate: (NSString*)templat;
- (NSString*) stringByReplacingMatchesInString: (NSString*)string
                                       options: (NSMatchingOptions)options
                                         range: (NSRange)range
                                  withTemplate: (NSString*)templat;
- (NSString*) replacementStringForResult: (NSTextCheckingResult*)result
                                inString: (NSString*)string
                                  offset: (NSInteger)offset
                                template: (NSString*)templat;
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly) NSRegularExpressionOptions options;
@property (readonly) NSUInteger numberOfCaptureGroups;
#else
- (NSRegularExpressionOptions) options;
- (NSUInteger) numberOfCaptureGroups;
#endif
#endif // GS_USE_ICU
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSRegualrExpression_h_GNUSTEP_BASE_INCLUDE */
