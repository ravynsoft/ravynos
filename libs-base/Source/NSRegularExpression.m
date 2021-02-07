/** Implementation of NSRegularExpression for GNUStep

   Copyright (C) 2010 Free Software Foundation, Inc.

   This file is part of the GNUstep Base Library.

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

   $Date: 2010-09-18 16:09:58 +0100 (Sat, 18 Sep 2010) $ $Revision: 31371 $
   */


#define	EXPOSE_NSRegularExpression_IVARS	1
#import "common.h"

#if GS_USE_ICU == 1
#include "unicode/uregex.h"

/* FIXME It would be nice to use autoconf for checking whether uregex_openUText
 * is defined.  However the naive check using AC_CHECK_FUNCS(uregex_openUText)
 * won't work because libicu internally renames all entry points with some cpp
 * magic.
 */
#if (U_ICU_VERSION_MAJOR_NUM > 4 || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 4))
#define HAVE_UREGEX_OPENUTEXT 1
#endif

#define NSRegularExpressionWorks

#define GSREGEXTYPE URegularExpression
#import "GSICUString.h"
#import "Foundation/NSRegularExpression.h"
#import "Foundation/NSTextCheckingResult.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSNotification.h"


/**
 * To be helpful, Apple decided to define a set of flags that mean exactly the
 * same thing as the URegexpFlags enum in libicu, but have different values.
 * This was completely stupid, but we probably have to live with it.  We could
 * in theory use the libicu values directly (that would be sensible), but that
 * would break any code that didn't correctly use the symbolic constants.
 */
uint32_t
NSRegularExpressionOptionsToURegexpFlags(NSRegularExpressionOptions opts)
{
  uint32_t flags = 0;

  if (opts & NSRegularExpressionCaseInsensitive)
    {
      flags |= UREGEX_CASE_INSENSITIVE;
    }
  if (opts & NSRegularExpressionAllowCommentsAndWhitespace)
    {
      flags |= UREGEX_COMMENTS;
    }
  if (opts & NSRegularExpressionIgnoreMetacharacters)
    {
      flags |= UREGEX_LITERAL;
    }
  if (opts & NSRegularExpressionDotMatchesLineSeparators)
    {
      flags |= UREGEX_DOTALL;
    }
  if (opts & NSRegularExpressionAnchorsMatchLines)
    {
      flags |= UREGEX_MULTILINE;
    }
  if (opts & NSRegularExpressionUseUnixLineSeparators)
    {
      flags |= UREGEX_UNIX_LINES;
    }
  if (opts & NSRegularExpressionUseUnicodeWordBoundaries)
    {
      flags |= UREGEX_UWORD;
    }
  return flags;
}

@implementation NSRegularExpression

+ (NSRegularExpression*) regularExpressionWithPattern: (NSString*)aPattern
  options: (NSRegularExpressionOptions)opts
  error: (NSError**)e
{
  return [[[self alloc] initWithPattern: aPattern
				options: opts
				  error: e] autorelease];
}


#if HAVE_UREGEX_OPENUTEXT
- (id) initWithPattern: (NSString*)aPattern
	       options: (NSRegularExpressionOptions)opts
		 error: (NSError**)e
{
  uint32_t	flags = NSRegularExpressionOptionsToURegexpFlags(opts);
  UText		p = UTEXT_INITIALIZER;
  UParseError	pe = {0};
  UErrorCode	s = 0;

#if !__has_feature(blocks)
  if ([self class] != [NSRegularExpression class])
    {
      GSOnceMLog(@"Warning: NSRegularExpression was built by a compiler without blocks support.  NSRegularExpression will deviate from the documented behaviour when subclassing and any code that subclasses NSRegularExpression may break in unexpected ways.  If you must subclass NSRegularExpression, you are strongly recommended to use a compiler with blocks support.");
    }
#endif

  UTextInitWithNSString(&p, aPattern);
  regex = uregex_openUText(&p, flags, &pe, &s);
  utext_close(&p);
  if (U_FAILURE(s))
    {
      // FIXME: Do something sensible with the error parameter.
      [self release];
      return nil;
    }
  options = opts;
  return self;
}

- (BOOL) isEqual: (id)obj
{
  if ([obj isKindOfClass: [NSRegularExpression class]])
    {
      if (self == obj)
        {
          return YES;
        }
      else if (options != ((NSRegularExpression*)obj)->options)
        {
          return NO;
        }
      else
        {
          UErrorCode  myErr      = 0;
          UErrorCode  theirErr   = 0;
          const UText *myText    = uregex_patternUText(regex, &myErr);
          const UText *theirText =
           uregex_patternUText(((NSRegularExpression*)obj)->regex, &theirErr);
          if (U_FAILURE(myErr) != U_FAILURE(theirErr))
            {
              return NO;
            }
          else if (U_FAILURE(myErr) && U_FAILURE(theirErr))
            {
              return YES;
            }
          return utext_equals(myText, theirText);
        }
    }
  else
    {
      return [super isEqual: obj];
    }
}

- (NSString*) pattern
{
  UErrorCode	s = 0;
  UText		*t = uregex_patternUText(regex, &s);
  GSUTextString	*str = NULL;

  if (U_FAILURE(s))
    {
      return nil;
    }
  str = [GSUTextString new];
  utext_clone(&str->txt, t, FALSE, TRUE, &s);
  return [str autorelease];
}
#else
- (id) initWithPattern: (NSString*)aPattern
	       options: (NSRegularExpressionOptions)opts
		 error: (NSError**)e
{
  int32_t	length = [aPattern length];
  uint32_t	flags = NSRegularExpressionOptionsToURegexpFlags(opts);
  UParseError	pe = {0};
  UErrorCode	s = 0;
  TEMP_BUFFER(buffer, length);

#if !__has_feature(blocks)
  if ([self class] != [NSRegularExpression class])
    {
      GSOnceMLog(@"Warning: NSRegularExpression was built by a compiler without blocks support.  NSRegularExpression will deviate from the documented behaviour when subclassing and any code that subclasses NSRegularExpression may break in unexpected ways.  If you must subclass NSRegularExpression, you are strongly recommended to use a compiler with blocks support.");
    }
#endif

  [aPattern getCharacters: buffer range: NSMakeRange(0, length)];
  regex = uregex_open(buffer, length, flags, &pe, &s);
  if (U_FAILURE(s))
    {
      // FIXME: Do something sensible with the error parameter.
      [self release];
      return nil;
    }
  options = opts;
  return self;
}

- (BOOL) isEqual: (id)obj
{
  if ([obj isKindOfClass: [NSRegularExpression class]])
    {
      if (self == obj)
        {
          return YES;
        }
      else if (options != ((NSRegularExpression*)obj)->options)
        {
          return NO;
        }
      else
        {
          UErrorCode  myErr      = 0;
          UErrorCode  theirErr   = 0;
          int32_t     myLen      = 0;
          int32_t     theirLen   = 0;
          const UChar *myText    = uregex_pattern(regex, &myLen, &myErr);
          const UChar *theirText = uregex_pattern(
                                     ((NSRegularExpression*)obj)->regex,
                                     &theirLen, &theirErr);
          if (U_FAILURE(myErr) != U_FAILURE(theirErr))
            {
              return NO;
            }
          else if (U_FAILURE(myErr) && U_FAILURE(theirErr))
            {
              return YES;
            }
          if (myLen != theirLen)
            {
              return NO;
            }
          return
           (0 == memcmp((const void*)myText, (const void*)theirText, myLen));
        }
    }
  else
    {
      return [super isEqual: obj];
    }
}



- (NSString*) pattern
{
  UErrorCode	s = 0;
  int32_t	length;
  const unichar *pattern = uregex_pattern(regex, &length, &s);

  if (U_FAILURE(s))
    {
      return nil;
    }
  return [NSString stringWithCharacters: pattern length: length];
}
#endif

- (NSUInteger) hash
{
  return [[self pattern] hash] ^ options;
}

static UBool
callback(const void *context, int32_t steps)
{
  BOOL		stop = NO;
  GSRegexBlock	block = (GSRegexBlock)context;

  if (NULL == context)
    {
      return FALSE;
    }
  CALL_BLOCK(block, nil, NSMatchingProgress, &stop);
  return stop;
}


#define DEFAULT_WORK_LIMIT 1500
/**
 * The work limit specifies the number of iterations the matcher will do before
 * aborting an operation. This ensures that degenerate pattern/input
 * combinations don't send the application into what for all intents and
 * purposes seems like an infinite loop.
 */
static int32_t _workLimit = DEFAULT_WORK_LIMIT;

+ (void) _defaultsChanged: (NSNotification*)n
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  id value = [defs objectForKey: @"GSRegularExpressionWorkLimit"];
  int32_t newLimit = DEFAULT_WORK_LIMIT;
  if ([value respondsToSelector: @selector(intValue)])
    {
      int32_t v = [value intValue];
      if (v >= 0)
        {
          newLimit = v;
        }
    }
  _workLimit = newLimit;
}

+ (void) initialize
{
  if (self == [NSRegularExpression class])
    {
      [[NSNotificationCenter defaultCenter]
        addObserver: self
           selector: @selector(_defaultsChanged:)
              name: NSUserDefaultsDidChangeNotification
            object: nil];
      [self _defaultsChanged: nil];
    }
}




/**
 * Sets up a libicu regex object for use.  Note: the documentation states that
 * NSRegularExpression must be thread safe.  To accomplish this, we store a
 * prototype URegularExpression in the object, and then clone it in each
 * method.  This is required because URegularExpression, unlike
 * NSRegularExpression, is stateful, and sharing this state between threads
 * would break concurrent calls.
 */
#if HAVE_UREGEX_OPENUTEXT
static URegularExpression *
setupRegex(URegularExpression *regex,
  NSString *string,
  UText *txt,
  NSMatchingOptions options,
  NSRange range,
  GSRegexBlock block)
{
  UErrorCode		s = 0;
  URegularExpression	*r = uregex_clone(regex, &s);

  if (options & NSMatchingReportProgress)
    {
      uregex_setMatchCallback(r, callback, block, &s);
    }
  UTextInitWithNSString(txt, string);
  uregex_setUText(r, txt, &s);
  uregex_setRegion(r, range.location, range.location+range.length, &s);
  if (options & NSMatchingWithoutAnchoringBounds)
    {
      uregex_useAnchoringBounds(r, FALSE, &s);
    }
  if (options & NSMatchingWithTransparentBounds)
    {
      uregex_useTransparentBounds(r, TRUE, &s);
    }
  uregex_setTimeLimit(r, _workLimit, &s);
  if (U_FAILURE(s))
    {
      uregex_close(r);
      return NULL;
    }
  return r;
}
#else
static URegularExpression *
setupRegex(URegularExpression *regex,
  NSString *string,
  unichar *buffer,
  int32_t length,
  NSMatchingOptions options,
  NSRange range,
  GSRegexBlock block)
{
  UErrorCode		s = 0;
  URegularExpression	*r = uregex_clone(regex, &s);

  [string getCharacters: buffer range: NSMakeRange(0, length)];
  if (options & NSMatchingReportProgress)
    {
      uregex_setMatchCallback(r, callback, block, &s);
    }
  uregex_setText(r, buffer, length, &s);
  uregex_setRegion(r, range.location, range.location+range.length, &s);
  if (options & NSMatchingWithoutAnchoringBounds)
    {
      uregex_useAnchoringBounds(r, FALSE, &s);
    }
  if (options & NSMatchingWithTransparentBounds)
    {
      uregex_useTransparentBounds(r, TRUE, &s);
    }
  uregex_setTimeLimit(r, _workLimit, &s);
  if (U_FAILURE(s))
    {
      uregex_close(r);
      return NULL;
    }
  return r;
}
#endif

static uint32_t
prepareResult(NSRegularExpression *regex,
  URegularExpression *r,
  NSRangePointer ranges,
  NSUInteger groups,
  UErrorCode *s)
{
  uint32_t	flags = 0;
  NSUInteger	i = 0;

  for (i = 0; i < groups; i++)
    {
      NSInteger start = uregex_start(r, i, s);
      NSInteger end = uregex_end(r, i, s);
      // The ICU API defines -1 as not found. Convert to
      // NSNotFound if applicable.
      if (start == -1)
        {
          start = NSNotFound;
        }
      if (end == -1)
        {
          end = NSNotFound;
        }

      if (end < start)
        {
          flags |= NSMatchingInternalError;
          end = start = NSNotFound;
        }
      ranges[i] = NSMakeRange(start, end-start);
    }
  if (uregex_hitEnd(r, s))
    {
      flags |= NSMatchingHitEnd;
    }
  if (uregex_requireEnd(r, s))
    {
      flags |= NSMatchingRequiredEnd;
    }
  if (0 != *s)
    {
      flags |= NSMatchingInternalError;
    }
  return flags;
}

#if HAVE_UREGEX_OPENUTEXT
- (void) enumerateMatchesInString: (NSString*)string
                          options: (NSMatchingOptions)opts
                            range: (NSRange)range
                       usingBlock: (GSRegexBlock)block
{
  UErrorCode	s = 0;
  UText		txt = UTEXT_INITIALIZER;
  BOOL		stop = NO;
  URegularExpression *r = setupRegex(regex, string, &txt, opts, range, block);
  NSUInteger	groups = [self numberOfCaptureGroups] + 1;
  NSRange	ranges[groups];

  // Should this throw some kind of exception?
  if (NULL == r)
    {
      return;
    }
  if (opts & NSMatchingAnchored)
    {
      if (uregex_lookingAt(r, -1, &s) && (0 == s))
	{
	  // FIXME: Factor all of this out into prepareResult()
	  uint32_t		flags;
	  NSTextCheckingResult *result;

	  flags = prepareResult(self, r, ranges, groups, &s);
	  result = (flags & NSMatchingInternalError) ? nil
            : [NSTextCheckingResult
	    regularExpressionCheckingResultWithRanges: ranges
						count: groups
				    regularExpression: self];
	  CALL_BLOCK(block, result, flags, &stop);
	}
    }
  else
    {
      while (!stop && uregex_findNext(r, &s) && (0 == s))
	{
	  uint32_t		flags;
	  NSTextCheckingResult	*result;

	  flags = prepareResult(self, r, ranges, groups, &s);
	  result = (flags & NSMatchingInternalError) ? nil
            : [NSTextCheckingResult
	    regularExpressionCheckingResultWithRanges: ranges
						count: groups
				    regularExpression: self];
	  CALL_BLOCK(block, result, flags, &stop);
	}
    }
  if (opts & NSMatchingCompleted)
    {
      CALL_BLOCK(block, nil, NSMatchingCompleted, &stop);
    }
  utext_close(&txt);
  uregex_close(r);
}
#else
- (void) enumerateMatchesInString: (NSString*)string
                          options: (NSMatchingOptions)opts
                            range: (NSRange)range
                       usingBlock: (GSRegexBlock)block
{
  UErrorCode	s = 0;
  BOOL		stop = NO;
  int32_t	length = [string length];
  URegularExpression *r;
  NSUInteger	groups = [self numberOfCaptureGroups] + 1;
  NSRange	ranges[groups];
  TEMP_BUFFER(buffer, length);

  r = setupRegex(regex, string, buffer, length, opts, range, block);

  // Should this throw some kind of exception?
  if (NULL == r)
    {
      return;
    }
  if (opts & NSMatchingAnchored)
    {
      if (uregex_lookingAt(r, -1, &s) && (0 == s))
	{
	  // FIXME: Factor all of this out into prepareResult()
	  uint32_t		flags;
	  NSTextCheckingResult *result;

	  flags = prepareResult(self, r, ranges, groups, &s);
	  result = (flags & NSMatchingInternalError) ? nil
            : [NSTextCheckingResult
	    regularExpressionCheckingResultWithRanges: ranges
						count: groups
				    regularExpression: self];
	  CALL_BLOCK(block, result, flags, &stop);
	}
    }
  else
    {
      while (!stop && uregex_findNext(r, &s) && (0 == s))
	{
	  uint32_t		flags;
	  NSTextCheckingResult	*result;

	  flags = prepareResult(self, r, ranges, groups, &s);
	  result = (flags & NSMatchingInternalError) ? nil
            : [NSTextCheckingResult
	    regularExpressionCheckingResultWithRanges: ranges
						count: groups
				    regularExpression: self];
	  CALL_BLOCK(block, result, flags, &stop);
	}
    }
  if (opts & NSMatchingCompleted)
    {
      CALL_BLOCK(block, nil, NSMatchingCompleted, &stop);
    }
  uregex_close(r);
}
#endif

/* The remaining methods are all meant to be wrappers around the primitive
 * method that takes a block argument.  Unfortunately, this is not really
 * possible when compiling with a compiler that doesn't support blocks.
 */
#if __has_feature(blocks)
- (NSUInteger) numberOfMatchesInString: (NSString*)string
                               options: (NSMatchingOptions)opts
                                 range: (NSRange)range

{
  __block NSUInteger	count = 0;

  opts &= ~NSMatchingReportProgress;
  opts &= ~NSMatchingReportCompletion;

  GSRegexBlock block =
    ^(NSTextCheckingResult *result, NSMatchingFlags flags, BOOL *stop)
    {
      count++;
    };
  [self enumerateMatchesInString: string
			 options: opts
			   range: range
		      usingBlock: block];
  return count;
}

- (NSTextCheckingResult*) firstMatchInString: (NSString*)string
                                     options: (NSMatchingOptions)opts
                                       range: (NSRange)range
{
  __block NSTextCheckingResult *r = nil;

  opts &= ~NSMatchingReportProgress;
  opts &= ~NSMatchingReportCompletion;

  GSRegexBlock block =
    ^(NSTextCheckingResult *result, NSMatchingFlags flags, BOOL *stop)
    {
      r = result;
      *stop = YES;
    };
  [self enumerateMatchesInString: string
			 options: opts
			   range: range
		      usingBlock: block];
  return r;
}

- (NSArray*) matchesInString: (NSString*)string
                     options:(NSMatchingOptions)opts
                       range:(NSRange)range
{
  NSMutableArray	*array = [NSMutableArray array];

  opts &= ~NSMatchingReportProgress;
  opts &= ~NSMatchingReportCompletion;

  GSRegexBlock block =
    ^(NSTextCheckingResult *result, NSMatchingFlags flags, BOOL *stop)
    {
      [array addObject: result];
    };
  [self enumerateMatchesInString: string
			 options: opts
			   range: range
		      usingBlock: block];
  return array;
}

- (NSRange) rangeOfFirstMatchInString: (NSString*)string
                              options: (NSMatchingOptions)opts
                                range: (NSRange)range
{
  __block NSRange r = {NSNotFound, 0};

  opts &= ~NSMatchingReportProgress;
  opts &= ~NSMatchingReportCompletion;

  GSRegexBlock block =
    ^(NSTextCheckingResult *result, NSMatchingFlags flags, BOOL *stop)
    {
      r = [result range];
      *stop = YES;
    };
  [self enumerateMatchesInString: string
			 options: opts
			   range: range
		      usingBlock: block];
  return r;
}

#else
#  ifdef __clang__ /* FIXME ... this is blocks specific, not clang specific */
#    warning Your compiler does not support blocks.  NSRegularExpression will deviate from the documented behaviour when subclassing and any code that subclasses NSRegularExpression may break in unexpected ways.  If you must subclass NSRegularExpression, you may want to use a compiler with blocks support.
#    warning Your compiler would support blocks if you added -fblocks to your OBJCFLAGS
#  endif
#if HAVE_UREGEX_OPENUTEXT
#define FAKE_BLOCK_HACK(failRet, code) \
  UErrorCode s = 0;\
  UText txt = UTEXT_INITIALIZER;\
  BOOL stop = NO;\
  URegularExpression *r = setupRegex(regex, string, &txt, opts, range, 0);\
  if (NULL == r) { return failRet; }\
  if (opts & NSMatchingAnchored)\
    {\
      if (uregex_lookingAt(r, -1, &s) && (0==s))\
	{\
	  code\
	}\
    }\
  else\
    {\
      while (!stop && uregex_findNext(r, &s) && (s == 0))\
	{\
	  code\
	}\
    }\
  utext_close(&txt);\
  uregex_close(r);
#else
#define FAKE_BLOCK_HACK(failRet, code) \
  UErrorCode s = 0;\
  BOOL stop = NO;\
  uint32_t length = [string length];\
  URegularExpression *r;\
  TEMP_BUFFER(buffer, length);\
  r = setupRegex(regex, string, buffer, length, opts, range, 0);\
  if (NULL == r) { return failRet; }\
  if (opts & NSMatchingAnchored)\
    {\
      if (uregex_lookingAt(r, -1, &s) && (0==s))\
	{\
	  code\
	}\
    }\
  else\
    {\
      while (!stop && uregex_findNext(r, &s) && (s == 0))\
	{\
	  code\
	}\
    }\
  uregex_close(r);
#endif

- (NSUInteger) numberOfMatchesInString: (NSString*)string
                               options: (NSMatchingOptions)opts
                                 range: (NSRange)range

{
  NSUInteger	count = 0;

  FAKE_BLOCK_HACK(count,
    {
      count++;
    });
  return count;
}

- (NSTextCheckingResult*) firstMatchInString: (NSString*)string
                                     options: (NSMatchingOptions)opts
                                       range: (NSRange)range
{
  NSTextCheckingResult	*result = nil;
  NSUInteger		groups = [self numberOfCaptureGroups] + 1;
  NSRange		ranges[groups];

  FAKE_BLOCK_HACK(result,
    {
      uint32_t  flags;

      flags = prepareResult(self, r, ranges, groups, &s);
      result = (flags & NSMatchingInternalError) ? nil
        : [NSTextCheckingResult
	regularExpressionCheckingResultWithRanges: ranges
					    count: groups
				regularExpression: self];
      stop = YES;
    });
  return result;
}

- (NSArray*) matchesInString: (NSString*)string
                     options: (NSMatchingOptions)opts
                       range: (NSRange)range
{
  NSMutableArray	*array = [NSMutableArray array];
  NSUInteger		groups = [self numberOfCaptureGroups] + 1;
  NSRange		ranges[groups];

  FAKE_BLOCK_HACK(array,
    {
      NSTextCheckingResult	*result = NULL;
      uint32_t                  flags;

      flags = prepareResult(self, r, ranges, groups, &s);
      result = (flags & NSMatchingInternalError) ? nil
        : [NSTextCheckingResult
	regularExpressionCheckingResultWithRanges: ranges
					    count: groups
				regularExpression: self];
      if (nil != result)
        {
          [array addObject: result];
        }
    });
  return array;
}

- (NSRange) rangeOfFirstMatchInString: (NSString*)string
                              options: (NSMatchingOptions)opts
                                range: (NSRange)range
{
  NSRange result = {NSNotFound, 0};

  FAKE_BLOCK_HACK(result,
    {
      prepareResult(self, r, &result, 1, &s);
      stop = YES;
    });
  return result;
}

#endif

#if HAVE_UREGEX_OPENUTEXT
- (NSUInteger) replaceMatchesInString: (NSMutableString*)string
                              options: (NSMatchingOptions)opts
                                range: (NSRange)range
                         withTemplate: (NSString*)template
{
  // FIXME: We're computing a value that is most likely ignored in an
  // expensive way.
  NSInteger	results = [self numberOfMatchesInString: string
						options: opts
						  range: range];
  UErrorCode	s = 0;
  UText		txt = UTEXT_INITIALIZER;
  UText		replacement = UTEXT_INITIALIZER;
  GSUTextString	*ret = [GSUTextString new];
  URegularExpression *r = setupRegex(regex, string, &txt, opts, range, 0);
  UText		*output = NULL;

  UTextInitWithNSString(&replacement, template);

  output = uregex_replaceAllUText(r, &replacement, NULL, &s);
  if (0 != s)
    {
      uregex_close(r);
      utext_close(&replacement);
      utext_close(&txt);
      DESTROY(ret);
      return 0;
    }
  utext_clone(&ret->txt, output, TRUE, TRUE, &s);
  [string setString: ret];
  [ret release];
  uregex_close(r);

  utext_close(&txt);
  utext_close(output);
  utext_close(&replacement);
  return results;
}

- (NSString*) stringByReplacingMatchesInString: (NSString*)string
                                       options: (NSMatchingOptions)opts
                                         range: (NSRange)range
                                  withTemplate: (NSString*)template
{
  UErrorCode	s = 0;
  UText		txt = UTEXT_INITIALIZER;
  UText		replacement = UTEXT_INITIALIZER;
  UText		*output = NULL;
  GSUTextString	*ret = [GSUTextString new];
  URegularExpression *r = setupRegex(regex, string, &txt, opts, range, 0);

  UTextInitWithNSString(&replacement, template);

  output = uregex_replaceAllUText(r, &replacement, NULL, &s);
  if (0 != s)
    {
      uregex_close(r);
      utext_close(&replacement);
      utext_close(&txt);
      DESTROY(ret);
      return nil;
    }
  utext_clone(&ret->txt, output, TRUE, TRUE, &s);
  uregex_close(r);

  utext_close(&txt);
  utext_close(output);
  utext_close(&replacement);
  return AUTORELEASE(ret);
}

- (NSString*) replacementStringForResult: (NSTextCheckingResult*)result
                                inString: (NSString*)string
                                  offset: (NSInteger)offset
                                template: (NSString*)template
{
  UErrorCode	s = 0;
  UText		txt = UTEXT_INITIALIZER;
  UText		replacement = UTEXT_INITIALIZER;
  UText		*output = NULL;
  GSUTextString	*ret = [GSUTextString new];
  NSRange	range = [result range];
  URegularExpression *r = setupRegex(regex,
				     [string substringWithRange: range],
				     &txt,
				     0,
				     NSMakeRange(0, range.length),
				     0);

  UTextInitWithNSString(&replacement, template);

  output = uregex_replaceFirstUText(r, &replacement, NULL, &s);
  if (0 != s)
    {
      uregex_close(r);
      utext_close(&replacement);
      utext_close(&txt);
      DESTROY(ret);
      return nil;
    }
  utext_clone(&ret->txt, output, TRUE, TRUE, &s);
  uregex_close(r);

  utext_close(&txt);
  utext_close(output);
  utext_close(&replacement);
  return AUTORELEASE(ret);
}
#else
- (NSUInteger) replaceMatchesInString: (NSMutableString*)string
                              options: (NSMatchingOptions)opts
                                range: (NSRange)range
                         withTemplate: (NSString*)template
{
  // FIXME: We're computing a value that is most likely ignored in an
  // expensive way.
  NSInteger	results = [self numberOfMatchesInString: string
						options: opts
						  range: range];
  UErrorCode	s = 0;
  uint32_t	length = [string length];
  uint32_t	replLength = [template length];
  unichar	replacement[replLength];
  int32_t	outLength;
  unichar	*output;
  NSString	*out;
  URegularExpression *r;
  TEMP_BUFFER(buffer, length);

  r = setupRegex(regex, string, buffer, length, opts, range, 0);
  [template getCharacters: replacement range: NSMakeRange(0, replLength)];

  outLength = uregex_replaceAll(r, replacement, replLength, NULL, 0, &s);

  s = 0;
  output = NSZoneMalloc(0, outLength * sizeof(unichar));
  uregex_replaceAll(r, replacement, replLength, output, outLength, &s);
  out =
    [[NSString alloc] initWithCharactersNoCopy: output
					length: outLength
				  freeWhenDone: YES];
  [string setString: out];
  RELEASE(out);

  return results;
}

- (NSString*) stringByReplacingMatchesInString: (NSString*)string
                                       options: (NSMatchingOptions)opts
                                         range: (NSRange)range
                                  withTemplate: (NSString*)template
{
  UErrorCode	s = 0;
  uint32_t	length = [string length];
  URegularExpression *r;
  uint32_t	replLength = [template length];
  unichar	replacement[replLength];
  int32_t	outLength;
  unichar	*output;
  TEMP_BUFFER(buffer, length);

  r = setupRegex(regex, string, buffer, length, opts, range, 0);
  [template getCharacters: replacement range: NSMakeRange(0, replLength)];

  outLength = uregex_replaceAll(r, replacement, replLength, NULL, 0, &s);

  s = 0;
  output = NSZoneMalloc(0, outLength * sizeof(unichar));
  uregex_replaceAll(r, replacement, replLength, output, outLength, &s);
  return AUTORELEASE([[NSString alloc] initWithCharactersNoCopy: output
							 length: outLength
						   freeWhenDone: YES]);
}

- (NSString*) replacementStringForResult: (NSTextCheckingResult*)result
                                inString: (NSString*)string
                                  offset: (NSInteger)offset
                                template: (NSString*)template
{
  UErrorCode	s = 0;
  NSRange	range = [result range];
  URegularExpression *r;
  uint32_t	replLength = [template length];
  unichar	replacement[replLength];
  int32_t	outLength;
  unichar	*output;
  TEMP_BUFFER(buffer, range.length);

  r = setupRegex(regex,
		 [string substringWithRange: range],
		 buffer,
		 range.length,
		 0,
		 NSMakeRange(0, range.length),
		 0);
  [template getCharacters: replacement range: NSMakeRange(0, replLength)];

  outLength = uregex_replaceFirst(r, replacement, replLength, NULL, 0, &s);
  s = 0;
  output = NSZoneMalloc(0, outLength * sizeof(unichar));
  uregex_replaceFirst(r, replacement, replLength, output, outLength, &s);
  return AUTORELEASE([[NSString alloc] initWithCharactersNoCopy: output
							 length: outLength
						   freeWhenDone: YES]);
}
#endif

- (NSRegularExpressionOptions) options
{
  return options;
}

- (NSUInteger) numberOfCaptureGroups
{
  UErrorCode s = 0;
  return uregex_groupCount(regex, &s);
}

- (void) dealloc
{
  uregex_close(regex);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeInteger: options forKey: @"options"];
      [aCoder encodeObject: [self pattern] forKey: @"pattern"];
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(NSRegularExpressionOptions)
				 at: &options];
      [aCoder encodeObject: [self pattern]];
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  NSString	*pattern;

  if ([aCoder allowsKeyedCoding])
    {
      options = [aCoder decodeIntegerForKey: @"options"];
      pattern = [aCoder decodeObjectForKey: @"pattern"];
    }
  else
    {
      [aCoder decodeValueOfObjCType: @encode(NSRegularExpressionOptions)
				 at: &options];
      pattern = [aCoder decodeObject];
    }
  return [self initWithPattern: pattern options: options error: NULL];
}

- (id) copyWithZone: (NSZone*)aZone
{
  NSRegularExpressionOptions	opts = options;
  UErrorCode			s = 0;
  URegularExpression		*r = uregex_clone(regex, &s);

  if (0 != s)
    {
      return nil;
    }

  self = [[self class] allocWithZone: aZone];
  if (nil == self)
    {
      return nil;
    }
  options = opts;
  regex = r;
  return self;
}
@end
#endif //GS_ICU == 1

#ifndef NSRegularExpressionWorks
#import "Foundation/NSRegularExpression.h"
#import "Foundation/NSZone.h"
#import "Foundation/NSException.h"
@implementation NSRegularExpression
+ (id)allocWithZone: (NSZone*)aZone
{
  [NSException raise: NSInvalidArgumentException
              format: @"NSRegularExpression requires ICU 4.4 or later"];
  return nil;
}
- (id) copyWithZone: (NSZone*)zone
{
  return nil;
}
- (void) encodeWithCoder: (NSCoder*)aCoder
{
}
- (id) initWithCoder: (NSCoder*)aCoder
{
  return nil;
}
@end
#endif
