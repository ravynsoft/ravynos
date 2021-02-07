/** Implementation of GSICUString for GNUStep

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

#import "common.h"
#if GS_USE_ICU == 1
#import "GSICUString.h"

/**
 * The number of characters that we use per chunk when fetching a block of
 * characters at once for iteration.  Making this value larger will make UText
 * iteration faster, at the cost of more memory.  Making it larger than the
 * size of a typical string will make it no faster but will still cost memory.
 */
static const NSUInteger chunkSize = 32;

/**
 * Returns the number of UTF16 characters in a UText backed by an NSString.
 */
static int64_t
UTextNSStringNativeLength(UText *ut)
{
  /* For constant strings the length is stored in ut->c, but for mutable
   * strings this is set to -1 and we must check the length every time.
   */
  return (-1 == ut->c) ? [(NSString*)ut->p length] : ut->c;
}


/**
 * Loads a group of characters into the buffer that can be directly accessed by
 * users of the UText.  This is used for iteration but UText users.
 */
UBool
UTextNSStringAccess(UText *ut, int64_t nativeIndex, UBool forward)
{
  NSString	*str = (NSString*)ut->p;
  NSInteger	length = (-1 == ut->c) ? (NSInteger)[str length] : ut->c;
  NSInteger     nativeStart = ut->chunkNativeStart;
  NSInteger     nativeLimit = ut->chunkNativeLimit;
  NSRange	r;

  if (forward)
    {
      if (nativeIndex < nativeLimit && nativeIndex >= nativeStart)
        {
          /* The chunk already contains the index, set the offset
           * to match it.
           */
          ut->chunkOffset = nativeIndex - nativeStart;
          return TRUE;
        }

      if (nativeIndex >= length && nativeLimit >= length)
        {
          /* Asking for a position beyond the end of the string;
           * Limit it to point just after the last character.
           */
          ut->chunkOffset = ut->chunkLength;
          return FALSE;
        }

      /* Set up to fill the chunk with characters from the string
       * and to start at the beginning of that buffer.
       */
      nativeStart = nativeIndex;
      nativeLimit = nativeIndex + chunkSize;
      if (nativeLimit > length)
        {
          nativeLimit = length;
        }
      r.location = nativeIndex;
      r.length = nativeLimit - nativeIndex;
      ut->chunkOffset = 0;
    }
  else
    {
      if (nativeIndex <= nativeLimit && nativeIndex > nativeStart)
        {
          /* The chunk already contains the index, set the offset
           * to match it.
           */
          ut->chunkOffset = nativeIndex - nativeStart;
          return TRUE;
        }

      if (nativeIndex <= 0 && nativeStart <= 0)
        {
          /* Asking for a position beyond the start of the string;
           * Limit it to position of the first character.
           */
          ut->chunkOffset = 0;
          return FALSE;
        }

      nativeLimit = nativeIndex;
      if (nativeLimit > length)
        {
          nativeLimit = length;
        }
      nativeStart = nativeLimit - chunkSize;
      if (nativeStart < 0)
        {
          nativeStart = 0;
        }
      r.location = nativeStart;
      r.length = nativeLimit - nativeStart;
      ut->chunkOffset = r.length;
    }
  [str getCharacters: ut->pExtra range: r];
  ut->chunkNativeLimit = nativeLimit;
  ut->chunkNativeStart = nativeStart;
  ut->nativeIndexingLimit = r.length;
  ut->chunkLength = r.length;
  return TRUE;
}

/**
 * Replaces characters in an NSString-backed UText.
 */
static int32_t
UTextNSMutableStringReplace(UText *ut,
  int64_t nativeStart,
  int64_t nativeLimit,
  const UChar *replacementText,
  int32_t replacmentLength,
  UErrorCode *status)
{
  NSMutableString	*str = (NSMutableString*)ut->p;
  NSRange		r = NSMakeRange(nativeStart, nativeLimit-nativeStart);
  NSString		*replacement = [NSString alloc];

  if (replacmentLength < 0)
    {
      replacement = [replacement initWithCString: (const char*)replacementText
					encoding: NSUTF16StringEncoding];
    }
  else
    {
      replacement = [replacement
        initWithCharactersNoCopy: (unichar*)replacementText
                          length: replacmentLength 
                    freeWhenDone: NO];
    }
  [str replaceCharactersInRange: r withString: replacement];

  // Setting the chunk length to 0 here forces UTextNSStringAccess to fetch
  // the data from the string object.
  ut->chunkLength = 0;
  UTextNSStringAccess(ut, r.location + [replacement length] + 1, TRUE);
  ut->chunkOffset++;
  
  [replacement release];
  if (NULL != status)
    {
      *status = 0;
    }
  return 0;
}

/**
 * Reads some characters.  This is roughly analogous to NSString's
 * -getCharacters:range:.
 */
static int32_t
UTextNSStringExtract(UText *ut,
  int64_t nativeStart,
  int64_t nativeLimit,
  UChar *dest,
  int32_t destCapacity,
  UErrorCode *status)
{
  /* If we're loading no characters, we are expected to return the number of
   * characters that we could load if requested.
   */
  if (destCapacity <= 0)
    {
      return nativeLimit - nativeStart;
    }
  else
    {
      NSString	        *str = (NSString*)ut->p;
      NSUInteger	length = (-1 == ut->c) ? [str length] : ut->c;
      NSRange	        r;

      if (nativeLimit > length)
        {
          nativeLimit = length;
        }
      r = NSMakeRange(nativeStart, nativeLimit - nativeStart );
      if (destCapacity < r.length)
        {
          r.length = destCapacity;
        }
      [str getCharacters: dest range: r];
      if (destCapacity > r.length)
        {
          dest[r.length] = 0;
        }
      return r.length;
    }
}

/**
 * Copy or move some characters within a UText.
 */
void UTextNSStringCopy(UText *ut,
  int64_t nativeStart,
  int64_t nativeLimit,
  int64_t nativeDest,
  UBool move,
  UErrorCode *status)
{
  NSMutableString	*str = (NSMutableString*)ut->p;
  NSUInteger	        length = (-1 == ut->c) ? [str length] : ut->c;
  NSRange		r;
  NSString		*substr;

  if (nativeLimit > length)
    {
      nativeLimit = length;
    }
  r = NSMakeRange(nativeStart, nativeLimit - nativeStart);
  substr = [str substringWithRange: r];
  [str insertString: substr atIndex: nativeDest];
  if (move)
    {
      if (nativeDest < r.location)
	{
	  r.location += r.length;
	}
      [str deleteCharactersInRange: r];
    }
  if (NULL != status)
    {
      *status = 0;
    }
}


/**
 * Destructor for the NSString-specific parts of the UText.  Because UTexts can
 * be allocated on the stack, or reused by different storage implementations,
 * this does not destroy the UText itself.
 */
static void
UTextNStringClose(UText *ut)
{
  ut->chunkContents = NULL;
  [(NSString*)ut->p release];
  ut->p = NULL;
}

/**
 * Copies the UText object, optionally copying the NSString.  This version is
 * for NSString-backed UTexts, so uses -copy to copy the string if required.
 * Typically, this should not actually copy the underlying storage, because it
 * is immutable.
 */
UText*
UTextNSStringClone(UText *dest,
  const UText *src,
  UBool deep,
  UErrorCode *status)
{
  NSString	*str = (NSString*)src->p;

  if (deep)
    {
      str = [[str copy] autorelease];
    }
  return UTextInitWithNSString(dest, str);
}

/**
 * Copies the UText object, optionally copying the NSMutableString.
 */
UText*
UTextNSMutableStringClone(UText *dest,
  const UText *src,
  UBool deep,
  UErrorCode *status)
{
  NSMutableString	*str = (NSMutableString*)src->p;
  UText			*txt;

  if (deep)
    {
      str = [str mutableCopy];
      txt = UTextInitWithNSMutableString(dest, str);
      [str release];
    }
  else
    {
      txt = UTextInitWithNSMutableString(dest, str);
    }
  return txt;
}

/**
 * Returns the index of the current character in the temporary buffer.
 */
int64_t
UTextNSStringMapOffsetToNative(const UText *ut)
{
  return ut->chunkNativeStart + ut->chunkOffset;
}

/**
 * Vtable for NSString-backed UTexts.
 */
static const UTextFuncs NSStringFuncs = 
{
  sizeof(UTextFuncs), // Table size
  0, 0, 0,            // Reserved
  UTextNSStringClone,
  UTextNSStringNativeLength,
  UTextNSStringAccess,
  UTextNSStringExtract,
  0,                  // Replace
  UTextNSStringCopy,
  UTextNSStringMapOffsetToNative,
  0,                // Map to UTF16
  UTextNStringClose,
  0, 0, 0             // Spare
};

/**
 * Vtable for NSMutableString-backed UTexts.
 */
static const UTextFuncs NSMutableStringFuncs = 
{
  sizeof(UTextFuncs), // Table size
  0, 0, 0,            // Reserved
  UTextNSMutableStringClone,
  UTextNSStringNativeLength,
  UTextNSStringAccess,
  UTextNSStringExtract,
  UTextNSMutableStringReplace,
  UTextNSStringCopy,
  UTextNSStringMapOffsetToNative,
  0,                // Map to UTF16
  UTextNStringClose,
  0, 0, 0             // Spare
};

UText*
UTextInitWithNSMutableString(UText *txt, NSMutableString *str)
{
  UErrorCode status = 0;

  txt = utext_setup(txt, chunkSize * sizeof(unichar), &status);

  if (U_FAILURE(status))
    {
      return NULL;
    }

  txt->p = [str retain];
  txt->pFuncs = &NSMutableStringFuncs;
  txt->chunkContents = txt->pExtra;
  txt->c = -1;  // Need to fetch length every time
  txt->providerProperties = 1<<UTEXT_PROVIDER_WRITABLE;

  return txt;
}

UText*
UTextInitWithNSString(UText *txt, NSString *str)
{
  UErrorCode status = 0;
  txt = utext_setup(txt, 64, &status);

  if (U_FAILURE(status))
    {
      return NULL;
    }

  txt->p = [str retain];
  txt->pFuncs = &NSStringFuncs;
  txt->chunkContents = txt->pExtra;
  txt->c = [str length];

  return txt;
}

@implementation GSUTextString
- (id) init
{
  if (nil != (self = [super init]))
    {
      UText t = UTEXT_INITIALIZER;

      memcpy(&txt, &t, sizeof(t));
    }
  return self;
}

- (NSUInteger) length
{
  return utext_nativeLength(&txt);
}

- (unichar) characterAtIndex: (NSUInteger)idx
{
  unichar c;

  [self getCharacters: &c range: NSMakeRange(idx, 1)];
  return c;
}

- (void) getCharacters: (unichar*)buffer range: (NSRange)r
{
  UErrorCode status = 0;

  utext_extract(&txt, r.location, r.location+r.length, buffer, r.length,
    &status);
  if (U_FAILURE(status))
    {
      _NSRangeExceptionRaise();
    }
}

- (void) dealloc
{
  utext_close(&txt);
  [super dealloc];
}
@end

@implementation GSUTextMutableString
- (id) init
{
  if (nil != (self = [super init]))
    {
      UText t = UTEXT_INITIALIZER;

      memcpy(&txt, &t, sizeof(t));
    }
  return self;
}

- (NSUInteger) length
{
  return utext_nativeLength(&txt);
}

- (unichar) characterAtIndex: (NSUInteger)idx
{
  unichar c;
  [self getCharacters: &c range: NSMakeRange(idx, 1)];
  return c;
}

- (void) getCharacters: (unichar*)buffer range: (NSRange)r
{
  UErrorCode status = 0;

  utext_extract(&txt, r.location, r.location+r.length, buffer, r.length,
    &status);
  if (U_FAILURE(status))
    {
      _NSRangeExceptionRaise();
    }
}

- (void) replaceCharactersInRange: (NSRange)r
                       withString: (NSString*)aString
{
  NSUInteger	size = [aString length];
  UErrorCode	status = 0;

  TEMP_BUFFER(buffer, size);
  [aString getCharacters: buffer range: NSMakeRange(0, size)];

  utext_replace(&txt, r.location, r.location + r.length, buffer, size, &status);
}

- (void) dealloc
{
  utext_close(&txt);
  [super dealloc];
}
@end
#endif // HAV_ICU
