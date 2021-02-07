/** Support functions for Unicode implementation
   Function to determine default c string encoding for
   GNUstep based on GNUSTEP_STRING_ENCODING environment variable.

   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by: Stevo Crvenkovski < stevo@btinternet.com >
   Date: March 1997
   Merged with GetDefEncoding.m and iconv by: Fred Kiefer <fredkiefer@gmx.de>
   Date: September 2000
   Rewrite by: Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.
*/


#import "common.h"
#if	!defined(NeXT_Foundation_LIBRARY)
#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSError.h"
#import "Foundation/NSException.h"
#import "Foundation/NSPathUtilities.h"
#endif

#import "GNUstepBase/GSLock.h"
#import "GNUstepBase/GSMime.h"
#import "GNUstepBase/Unicode.h"

#import "../GSPrivate.h"
#import "../GSPThread.h"

#include <stdio.h>

#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif
#if     defined(HAVE_UNICODE_UCNV_H)
#include <unicode/ucnv.h>
#endif


typedef struct {unichar from; unsigned char to;} _ucc_;

#include "unicode/cyrillic.h"
#include "unicode/latin2.h"
#include "unicode/latin9.h"
#include "unicode/nextstep.h"
#include "unicode/caseconv.h"
#include "unicode/cop.h"
#include "unicode/decomp.h"
#include "unicode/gsm0338.h"
#include "unicode/thai.h"

#ifdef HAVE_ICONV
#ifdef HAVE_GICONV_H
#include <giconv.h>
#else
#include <iconv.h>
#endif

/*
 * The whole of the GNUstep code stores UNICODE in internal byte order,
 * so we do the same. We have switched to using UTF16 so the defines here
 * recognise this. We use the endian specific versions of UTF16 so that
 * iconv does not introduce a BOM where we do not want it.
 * If UTF16 does not work, we revert to UCS-2-INTERNAL.
 */
#ifdef WORDS_BIGENDIAN
#define UNICODE_UTF16 "UTF-16BE"
#define UNICODE_UTF32 "UTF-32BE"
#define UNICODE_INT "UNICODEBIG"
#else
#define UNICODE_UTF16 "UTF-16LE"
#define UNICODE_UTF32 "UTF-32LE"
#define UNICODE_INT "UNICODELITTLE"
#endif

#define UNICODE_ENC ((unicode_enc) ? unicode_enc : internal_unicode_enc())

static const char *unicode_enc = NULL;

/* Check to see what type of internal unicode format the library supports */
static const char *
internal_unicode_enc(void)
{
  iconv_t conv;

  unicode_enc = UNICODE_UTF16;
  conv = iconv_open(unicode_enc, "ASCII");
  if (conv != (iconv_t)-1)
    {
      iconv_close(conv);
      return unicode_enc;
    }
  fprintf(stderr, "Could not initialise iconv() for UTF16, using UCS-2\n");
  fprintf(stderr, "Using characters outside 16 bits may give bad results.\n");

  unicode_enc = UNICODE_INT;
  conv = iconv_open(unicode_enc, "ASCII");
  if (conv != (iconv_t)-1)
    {
      iconv_close(conv);
      return unicode_enc;
    }
  unicode_enc = "UCS-2-INTERNAL";
  conv = iconv_open(unicode_enc, "ASCII");
  if (conv != (iconv_t)-1)
    {
      iconv_close(conv);
      return unicode_enc;
    }
  unicode_enc = "UCS-2";
  /* This had better work */
  return unicode_enc;
}

#else
#define UNICODE_UTF32 ""
#endif

static pthread_mutex_t local_lock = PTHREAD_MUTEX_INITIALIZER;

typedef	unsigned char	unc;
static NSStringEncoding	defEnc = GSUndefinedEncoding;
static NSStringEncoding	natEnc = GSUndefinedEncoding;
static NSStringEncoding	icuEnc = GSUndefinedEncoding;
static NSStringEncoding *_availableEncodings = 0;

struct _strenc_ {
  NSStringEncoding	enc;		// Constant representing the encoding.
  const char		*ename;		// ASCII string representation of name.
  const char		*iconv;		/* Iconv name of encoding.  If this
					 * is the empty string, we cannot use
					 * iconv perform conversions to/from
					 * this encoding.
					 * NB. do not put a null pointer in this
					 * field in the table, use "" instead.
					 */
  BOOL			eightBit;	/* Flag to say whether this encoding
					 * can be stored in a byte array ...
					 * ie whether the encoding consists
					 * entirely of single byte characters
					 * and the first 128 are identical to
					 * the ASCII character set.
					 */
  char			supported;	/* Is this supported?  Some encodings
					 * have builtin conversion to/from
					 * unicode, but for others we must
					 * check with iconv to see if it
					 * supports them on this platform.
					 * A one means supported.
					 * A negative means unsupported.
					 * A zero means not yet checked.
					 */
  const char		*lossy;		/* Iconv name for lossy encoding */
};

/*
 * The str_encoding_table is a compact representation of all the string
 * encoding information we might need.  It gets modified at runtime.
 */
static struct _strenc_ str_encoding_table[] = {
  {NSASCIIStringEncoding,
    "NSASCIIStringEncoding","ASCII",1,1,0},
  {NSNEXTSTEPStringEncoding,
    "NSNEXTSTEPStringEncoding","NEXTSTEP",1,1,0},
  {NSJapaneseEUCStringEncoding,
    "NSJapaneseEUCStringEncoding","EUC-JP",0,0,0},
  {NSUTF8StringEncoding,
    "NSUTF8StringEncoding","UTF-8",0,1,0},
  {NSISOLatin1StringEncoding,
    "NSISOLatin1StringEncoding","ISO-8859-1",1,1,0},
  {NSSymbolStringEncoding,
    "NSSymbolStringEncoding","",0,0,0},
  {NSNonLossyASCIIStringEncoding,
    "NSNonLossyASCIIStringEncoding","",0,1,0},
  {NSShiftJISStringEncoding,
    "NSShiftJISStringEncoding","SHIFT-JIS",0,0,0},
  {NSISOLatin2StringEncoding,
    "NSISOLatin2StringEncoding","ISO-8859-2",1,1,0},
  {NSUnicodeStringEncoding,
    "NSUnicodeStringEncoding","",0,1,0},
  {NSWindowsCP1251StringEncoding,
    "NSWindowsCP1251StringEncoding","CP1251",0,0,0},
  {NSWindowsCP1252StringEncoding,
    "NSWindowsCP1252StringEncoding","CP1252",0,0,0},
  {NSWindowsCP1253StringEncoding,
    "NSWindowsCP1253StringEncoding","CP1253",0,0,0},
  {NSWindowsCP1254StringEncoding,
    "NSWindowsCP1254StringEncoding","CP1254",0,0,0},
  {NSWindowsCP1250StringEncoding,
    "NSWindowsCP1250StringEncoding","CP1250",0,0,0},
  {NSISO2022JPStringEncoding,
    "NSISO2022JPStringEncoding","ISO-2022-JP",0,0,0},
  {NSMacOSRomanStringEncoding,
    "NSMacOSRomanStringEncoding","MACINTOSH",0,0,0},
#if     defined(GNUSTEP)
  {NSProprietaryStringEncoding,
    "NSProprietaryStringEncoding","",0,0,0},
#endif

// GNUstep additions
  {NSISOCyrillicStringEncoding,
    "NSISOCyrillicStringEncoding","ISO-8859-5",0,1,0},
  {NSKOI8RStringEncoding,
    "NSKOI8RStringEncoding","KOI8-R",0,0,0},
  {NSISOLatin3StringEncoding,
    "NSISOLatin3StringEncoding","ISO-8859-3",0,0,0},
  {NSISOLatin4StringEncoding,
    "NSISOLatin4StringEncoding","ISO-8859-4",0,0,0},
  {NSISOArabicStringEncoding,
    "NSISOArabicStringEncoding","ISO-8859-6",0,0,0},
  {NSISOGreekStringEncoding,
    "NSISOGreekStringEncoding","ISO-8859-7",0,0,0},
  {NSISOHebrewStringEncoding,
    "NSISOHebrewStringEncoding","ISO-8859-8",0,0,0},
  {NSISOLatin5StringEncoding,
    "NSISOLatin5StringEncoding","ISO-8859-9",0,0,0},
  {NSISOLatin6StringEncoding,
    "NSISOLatin6StringEncoding","ISO-8859-10",0,0,0},
  {NSISOThaiStringEncoding,
    "NSISOThaiStringEncoding","ISO-8859-11",1,1,0},
  {NSISOLatin7StringEncoding,
    "NSISOLatin7StringEncoding","ISO-8859-13",0,0,0},
  {NSISOLatin8StringEncoding,
    "NSISOLatin8StringEncoding","ISO-8859-14",0,0,0},
  {NSISOLatin9StringEncoding,
    "NSISOLatin9StringEncoding","ISO-8859-15",1,1,0},
  {NSUTF7StringEncoding,
    "NSUTF7StringEncoding","UTF-7",0,0,0},
  {NSGB2312StringEncoding,
    "NSGB2312StringEncoding","EUC-CN",0,0,0},
  {NSGSM0338StringEncoding,
    "NSGSM0338StringEncoding","",0,1,0},
  {NSBIG5StringEncoding,
    "NSBIG5StringEncoding","BIG5",0,0,0},
  {NSKoreanEUCStringEncoding,
    "NSKoreanEUCStringEncoding","EUC-KR",0,0,0},

/* Now Apple encodings which have high numeric values.
 */
  {NSUTF16BigEndianStringEncoding,
    "NSUTF16BigEndianStringEncoding","UTF-16BE",0,0,0},
  {NSUTF16LittleEndianStringEncoding,
    "NSUTF16LittleEndianStringEncoding","UTF-16LE",0,0,0},
  {NSUTF32StringEncoding,
    "NSUTF32StringEncoding",UNICODE_UTF32,0,0,0},
  {NSUTF32BigEndianStringEncoding,
    "NSUTF32BigEndianStringEncoding","UTF-32BE",0,0,0},
  {NSUTF32LittleEndianStringEncoding,
    "NSUTF32LittleEndianStringEncoding","UTF-32LE",0,0,0},

  {0,"Unknown encoding","",0,0,0}
};

static struct _strenc_	**encodingTable = 0;
static unsigned		encTableSize = 0;

static void GSSetupEncodingTable(void)
{
  if (encodingTable == 0)
    {
      (void)pthread_mutex_lock(&local_lock);
      if (encodingTable == 0)
	{
	  static struct _strenc_	**encTable = 0;
	  unsigned			count;
	  unsigned			i;

	  /*
	   * We want to store pointers to our string encoding info in a
	   * large table so we can do efficient lookup by encoding value.
	   */
#define	MAX_ENCODING	128
	  count = sizeof(str_encoding_table) / sizeof(struct _strenc_);

	  /*
	   * First determine the largest encoding value and create a
	   * large enough table of pointers.
	   */
	  encTableSize = 0;
	  for (i = 0; i < count; i++)
	    {
	      unsigned	tmp = str_encoding_table[i].enc;

	      if (tmp > encTableSize)
		{
		  if (tmp < MAX_ENCODING)
		    {
		      encTableSize = tmp;
		    }
		}
	    }
	  encTable = malloc(
	    (encTableSize+1)*sizeof(struct _strenc_ *));
	  memset(encTable, 0, (encTableSize+1)*sizeof(struct _strenc_ *));

	  /*
	   * Now set up the pointers at the correct location in the table.
	   */
	  for (i = 0; i < count; i++)
	    {
	      struct _strenc_ *entry = &str_encoding_table[i];
	      unsigned	tmp = entry->enc;

	      if (tmp < MAX_ENCODING)
		{
		  encTable[tmp] = entry;
		}
#ifdef HAVE_ICONV
	      if (entry->iconv != 0 && *(entry->iconv) != 0)
		{
		  iconv_t	c;
		  int		l;
		  char	*lossy;

		  /*
		   * See if we can do a lossy conversion.
		   */
		  l = strlen(entry->iconv);
		  lossy = malloc(l + 11);
		  memcpy(lossy, entry->iconv, l);
		  memcpy(lossy + l, "//TRANSLIT", 11);
		  c = iconv_open(lossy, UNICODE_ENC);
		  if (c == (iconv_t)-1)
		    {
		      free(lossy);
		    }
		  else
		    {
		      entry->lossy = lossy;
		      iconv_close(c);
		    }
		}
#endif
	    }
	  encodingTable = encTable;
	}
      (void)pthread_mutex_unlock(&local_lock);
    }
}

static struct _strenc_ *
EntryForEncoding(NSStringEncoding enc)
{
  struct _strenc_ *entry = 0;

  if (enc > 0)
    {
      GSSetupEncodingTable();
      if (enc <= encTableSize)
	{
	  entry = encodingTable[enc];
	}
      else
	{
	  unsigned	i = 0;

	  while (i < sizeof(str_encoding_table) / sizeof(struct _strenc_))
	    {
	      if (str_encoding_table[i].enc == enc)
		{
		  entry = &str_encoding_table[i];
		  break;
		}
	      i++;
	    }
	}
    }
  return entry;
}

static struct _strenc_ *
EntrySupported(NSStringEncoding enc)
{
  struct _strenc_ *entry = EntryForEncoding(enc);

  if (entry == 0)
    {
      return NULL;
    }
#ifdef HAVE_ICONV
  if (entry->iconv != 0 && entry->supported == 0)
    {
      if (enc == NSUnicodeStringEncoding)
	{
	  entry->iconv = UNICODE_ENC;
	  entry->supported = 1;
	}
      else if (entry->iconv[0] == 0)
        {
	  /* explicitly check for empty encoding name since some systems
	   * have buggy iconv_open() code which succeeds on an empty name.
	   */
	  entry->supported = -1;
	}
      else
	{
	  iconv_t	c;

	  c = iconv_open(UNICODE_ENC, entry->iconv);
	  if (c == (iconv_t)-1)
	    {
	      entry->supported = -1;
	    }
	  else
	    {
	      iconv_close(c);
	      c = iconv_open(entry->iconv, UNICODE_ENC);
	      if (c == (iconv_t)-1)
		{
		  entry->supported = -1;
		}
	      else
		{
		  iconv_close(c);
		  entry->supported = 1;
		}
	    }
	}
    }
#endif
  if (entry->supported == 1)
    {
      return entry;
    }
  return 0;
}

BOOL
GSPrivateIsEncodingSupported(NSStringEncoding enc)
{
  if (EntrySupported(enc) == 0)
    {
      return NO;
    }
  return YES;
}

/** Returns the NSStringEncoding that matches the specified
 *  character set registry and encoding information. For instance,
 *  for the iso8859-5 character set, the registry is iso8859 and
 *  the encoding is 5, and the returned NSStringEncoding is
 *  NSISOCyrillicStringEncoding. If there is no specific encoding,
 *  use @"0". Returns GSUndefinedEncoding if there is no match.
 */
NSStringEncoding
GSEncodingForRegistry (NSString *registry, NSString *encoding)
{
  NSString	*charset = registry;

  if ([encoding length] > 0 && [encoding isEqualToString: @"0"] == NO)
    {
      charset = [NSString stringWithFormat: @"%@-%@", registry, encoding];
    }
  return [GSMimeDocument encodingFromCharset: charset];
}

/** Try to deduce the string encoding from the locale string
 *  clocale. This function looks in the Locale.encodings file
 *  installed as part of GNUstep Base if the encoding cannot be
 *  deduced from the clocale string itself. If  clocale isn't set or
 *  no match can be found, returns GSUndefinedEncoding.
 */
/* It would be really nice if this could be used in +defaultCStringEncoding,
 * but there are too many dependancies on other parts of the library to
 * make this practical (even if everything possible was written in C,
 * we'd still need some way to find the Locale.encodings file).
 */
NSStringEncoding
GSEncodingFromLocale(const char *clocale)
{
  NSStringEncoding	encoding = GSUndefinedEncoding;
  NSString		*encodstr;

  if (clocale == NULL || strcmp(clocale, "C") == 0
    || strcmp(clocale, "POSIX") == 0)
    {
      /* Don't make any assumptions. Let caller handle that */
      return encoding;
    }

  if (strchr (clocale, '.') != NULL)
    {
      /* Locale contains the 'codeset' section. Parse it and see
	 if we know what encoding this cooresponds to */
      NSString	*registry;
      NSString	*charset;
      NSArray	*array;
      char	*s;

      s = strchr (clocale, '.');
      registry = [[NSString stringWithUTF8String: s+1] lowercaseString];
      array = [registry componentsSeparatedByString: @"-"];
      registry = [array objectAtIndex: 0];
      if ([array count] > 1)
	{
	  charset = [NSString stringWithFormat: @"%@-%@",
	    registry, [array lastObject]];
	}
      else
	{
	  charset = registry;
	}

      encoding = [GSMimeDocument encodingFromCharset: charset];
    }
  else
    {
      /* Look up the locale in our table of encodings */
      NSBundle *gbundle;
      NSString *table;

#ifdef GNUSTEP
      gbundle = [NSBundle bundleForLibrary: @"gnustep-base"];
#else
      gbundle = [NSBundle bundleForClass: NSClassFromString(@"GSXMLNode")];
#endif
      table = [gbundle pathForResource: @"Locale"
		                ofType: @"encodings"
		           inDirectory: @"Languages"];
      if (table != nil)
	{
	  unsigned	count;
	  NSDictionary	*dict;
	
	  dict = [NSDictionary dictionaryWithContentsOfFile: table];
	  encodstr = [dict objectForKey:
			     [NSString stringWithUTF8String: clocale]];
	  if (encodstr == nil)
	    return GSUndefinedEncoding;

	  /* Find the matching encoding */
	  count = 0;
	  while (str_encoding_table[count].enc
	    && strcmp(str_encoding_table[count].ename, [encodstr lossyCString]))
	    {
	      count++;
	    }
	  if (str_encoding_table[count].enc)
	    {
	      encoding = str_encoding_table[count].enc;
	    }
	  if (encoding == GSUndefinedEncoding)
	    {
	      NSLog(@"No known GNUstep encoding for %s = %@",
		    clocale, encodstr);
	    }
	}
    }

  return encoding;
}

/**
 * Uses direct access into a two-level table to map cases.<br />
 * The two-level table method is less space efficient (but still not bad) than
 * a single table and a linear search, but it reduces the number of
 * conditional statements to just one.
 */
unichar
uni_tolower(unichar ch)
{
  unichar result = gs_tolower_map[ch / 256][ch % 256];

  return result ? result : ch;
}

/**
 * Uses direct access into a two-level table to map cases.<br />
 * The two-level table method is less space efficient (but still not bad) than
 * a single table and a linear search, but it reduces the number of
 * conditional statements to just one.
 */
unichar
uni_toupper(unichar ch)
{
  unichar result = gs_toupper_map[ch / 256][ch % 256];

  return result ? result : ch;
}

unsigned char
GSPrivateUniCop(unichar u)
{
  if (u < uni_cop_table[0].code)
    {
      return 0;	// Special case for latin1
    }
  else
    {
      unichar	code;
      unichar	count = 0;
      unichar	first = 0;
      unichar	last = uni_cop_table_size - 1;

      while (first <= last)
	{
	  if (first != last)
	    {
	      count = (first + last) / 2;
	      code = uni_cop_table[count].code;
	      if (code < u)
		{
		  first = count+1;
		}
	      else if (code > u)
		{
		  last = count-1;
		}
	      else
		{
		  return uni_cop_table[count].cop;
		}
	    }
	  else  /* first == last */
	    {
	      if (u == uni_cop_table[first].code)
		{
		  return uni_cop_table[first].cop;
		}
	      return 0;
	    }
	}
      return 0;
    }
}

unsigned char
uni_cop(unichar u)
{
  return GSPrivateUniCop(u);
}

// uni_isnonsp(unichar u) now implemented in NSString.m

unichar*
uni_is_decomp(unichar u)
{
  if (u < uni_dec_table[0].code)
    {
      return 0;		// Special case for latin1
    }
  else
    {
      unichar	code;
      unichar	count = 0;
      unichar	first = 0;
      unichar	last = uni_dec_table_size - 1;

      while (first <= last)
	{
	  if (first != last)
	    {
	      count = (first + last) / 2;
	      code = uni_dec_table[count].code;
	      if (code < u)
		{
		  first = count+1;
		}
	      else if (code > u)
		{
		  last = count-1;
		}
	      else
		{
		  return uni_dec_table[count].decomp;
		}
	    }
	  else  /* first == last */
	    {
	      if (u == uni_dec_table[first].code)
		{
		  return uni_dec_table[first].decomp;
		}
	      return 0;
	    }
	}
      return 0;
    }
}

static inline int
octdigit(int c)
{
  return (c >= '0' && c < '8');
}

/**
 * Function to check a block of data for validity as a unicode string and
 * say whether it contains solely ASCII or solely Latin1 data.<br />
 * Any leading BOM must already have been removed and the data must already
 * be in native byte order.<br />
 * Returns the number of characters which were found valid.
 */
unsigned
GSUnicode(const unichar *chars, unsigned length,
  BOOL *isASCII, BOOL *isLatin1)
{
  unsigned	i = 0;
  unichar	c;

  if (isASCII) *isASCII = YES;
  if (isLatin1) *isLatin1 = YES;
  while (i < length)
    {
      if ((c = chars[i++]) > 127)
        {
	  if (isASCII) *isASCII = NO;
	  i--;
	  while (i < length)
	    {
	      if ((c = chars[i++]) > 255)
		{
		  if (isLatin1) *isLatin1 = NO;
		  i--;
		  while (i < length)
		    {
		      c = chars[i++];
		      if (c >= 0xdc00 && c <= 0xdfff)
		        {
			  return i - 1;	// Second half of a surrogate pair.
		        }
		      if (c >= 0xd800 && c <= 0xdbff)
		        {
			  // First half of a surrogate pair.
			  if (i >= length)
			    {
			      return i - 1;	// Second half missing
			    }
			  c = chars[i];
			  if (c < 0xdc00 || c > 0xdfff)
			    {
			      return i - 1;	// Second half missing
			    }
			  i++;		// Step past second half
		        }
		    }
		}
	    }
        }
    }
  return i;
}

#define	GROW() \
if (dst == 0) \
  { \
    /* \
     * Data is just being discarded anyway, so we can \
     * reset the offset into the local buffer on the \
     * stack and pretend the buffer has grown. \
     */ \
    ptr = buf - dpos; \
    bsize = dpos + BUFSIZ; \
    if (extra != 0) \
      { \
	bsize--; \
      } \
  } \
else if (zone == 0) \
  { \
    result = NO; /* No buffer growth possible ... fail. */ \
    goto done; \
  } \
else \
  { \
    unsigned	grow = slen; \
\
    if (grow < bsize + BUFSIZ) \
      { \
	grow = bsize + BUFSIZ; \
      } \
    grow *= sizeof(unichar); \
\
    if (ptr == buf || ptr == *dst) \
      { \
	unichar	*tmp; \
\
	tmp = NSZoneMalloc(zone, grow + extra); \
	if (tmp != 0) \
	  { \
	    memcpy(tmp, ptr, bsize * sizeof(unichar)); \
	  } \
	ptr = tmp; \
      } \
    else \
      { \
	ptr = NSZoneRealloc(zone, ptr, grow + extra); \
      } \
    if (ptr == 0) \
      { \
        result = NO; /* No buffer growth possible ... fail. */ \
        goto done; \
      } \
    bsize = grow / sizeof(unichar); \
  }

#define UTF8DECODE      1

#if     defined(UTF8DECODE)
/* This next data (utf8d) and function (decode()) copyright ...
Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

static const uint8_t utf8d[] = {
  // The first part of the table maps bytes to character classes that
  // to reduce the size of the transition table and create bitmasks.
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

  // The second part is a transition table that maps a combination
  // of a state of the automaton and a character class to a state.
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12, 
};

static uint32_t inline
decode(uint32_t* state, uint32_t* codep, uint32_t byte)
{
  uint32_t type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT)
    ?  (byte & 0x3fu) | (*codep << 6)
    : (0xff >> type) & (byte);

  *state = utf8d[256 + *state + type];
  return *state;
}

/* End of separately copyrighted section.
 */
#endif


/**
 * Function to convert from 8-bit data to 16-bit unicode characters.
 * <p>The dst argument is a pointer to a pointer to a buffer in which the
 * converted string is to be stored.  If it is a null pointer, this function
 * discards converted data, and is used only to determine the length of the
 * converted string.  If the zone argument is non-nul, the function is free
 * to allocate a larger buffer if necessary, and store this new buffer in
 * the dst argument.  It will *NOT* deallocate the original buffer!
 * </p>
 * <p>The size argument is a pointer to the initial size of the destination
 * buffer.  If the function changes the buffer size, this value will be
 * altered to the new size.  This is measured in 16-bit unicode characters,
 * not bytes.
 * </p>
 * <p>The src argument is a pointer to the byte sequence which is
 * to be converted to 16-bit unicode.
 * </p>
 * <p>The slen argument is the length of the byte sequence
 * which is to be converted to 16-bit unicode.
 * This is measured in bytes.
 * </p>
 * <p>The enc argument specifies the encoding type of the 8-bit byte sequence
 * which is to be converted to 16-bit unicode.
 * </p>
 * <p>The zone argument specifies a memory zone in which the function may
 * allocate a buffer to return data in.
 * If this is nul, the function will fail if the originally supplied buffer
 * is not big enough (unless dst is a null pointer ... indicating that
 * converted data is to be discarded).<br />
 * If the library is built for garbage collecting, the zone argument is used
 * only as a marker to say whether the function may allocate memory (zone
 * is non-null) or not (zone is null).
 * </p>
 * The options argument controls some special behavior.
 * <list>
 * <item>If GSUniTerminate is set, the function is expected to null terminate
 * the output string, and will assume that it is safe to place the nul
 * just beyond the end of the stated buffer size.
 * Also, if the function grows the buffer, it will allow for an extra
 * termination character.</item>
 * <item>If GSUniTemporary is set, the function will return the results in
 * an autoreleased buffer rather than in a buffer that the caller must
 * release.</item>
 * <item>If GSUniBOM is set, the function will write the first unicode
 * character as a byte order marker.</item>
 * <item>If GSUniShortOk is set, the function will return a buffer containing
 * any decoded characters even if the whole conversion fails.</item>
 * </list>
 * <p>On return, the function result is a flag indicating success (YES)
 * or failure (NO), and on success, the value stored in size is the number
 * of characters in the converted string.  The converted string itself is
 * stored in the location given by dst.<br />
 * NB. If the value stored in dst has been changed, it is a pointer to
 * allocated memory which the caller is responsible for freeing, and the
 * caller is <em>still</em> responsible for freeing the original buffer.
 * </p>
 */
BOOL
GSToUnicode(unichar **dst, unsigned int *size, const unsigned char *src,
  unsigned int slen, NSStringEncoding enc, NSZone *zone,
  unsigned int options)
{
  unichar	buf[BUFSIZ];
  unichar	*ptr;
  unsigned	bsize;
  unsigned	dpos = 0;	// Offset into destination buffer.
  unsigned	spos = 0;	// Offset into source buffer.
  unsigned	extra = (options & GSUniTerminate) ? sizeof(unichar) : 0;
  unichar	base = 0;
  unichar	*table = 0;
  BOOL		result = YES;
#ifdef HAVE_ICONV
  iconv_t	cd = (iconv_t)-1;
#endif

  /*
   * Ensure we have an initial buffer set up to decode data into.
   */
  if (dst == 0 || *size == 0)
    {
      ptr = buf;
      bsize = (extra != 0) ? BUFSIZ - 1 : BUFSIZ;
    }
  else
    {
      ptr = *dst;
      bsize = *size;
    }

  if (options & GSUniBOM)
    {
      while (dpos >= bsize)
	{
	  GROW();
	}
      ptr[dpos++] = (unichar)0xFEFF;	// Insert byte order marker.
    }

  switch (enc)
    {
      case NSUTF8StringEncoding:
	{
          uint32_t  u = 0;
#if     defined(UTF8DECODE)
          uint32_t      state = 0;
#endif
	  while (spos < slen)
	    {

#if     defined(UTF8DECODE)
              if (decode(&state, &u, src[spos++]))
                {
                  continue;
                }
#else
	      uint8_t   c = src[spos];

	      u = c;
	      if (c > 0x7f)
                {
                  int i, sle = 0;

		  /* legal first byte of a multibyte character?
                   */
                  if (c <= 0xc1 || c >= 0xf5)
                    {
                      /* (0x7f <= c < 0xc0) means this is a continuation
                       * of a multibyte character without the first byte.
                       *
                       * (0xc0 == c || 0xc1 == c) are always illegal because
                       *
                       * (c >= 0xf5) would be for a multibyte character
                       * outside the unicode range.
                       */
	              result = NO;
		      goto done;
                    }

		  /* calculated the expected sequence length */
                  while (c & 0x80)
                    {
                      c = c << 1;
                      sle++;
                    }

		  /* do we have enough bytes ? */
		  if ((spos + sle) > slen)
                    {
	              result = NO;
		      goto done;
	            }

		  /* get the codepoint */
		  for (i = 1; i < sle; i++)
		    {
		      if (src[spos + i] < 0x80 || src[spos + i] >= 0xc0)
			break;
		      u = (u << 6) | (src[spos + i] & 0x3f);
		    }
		  if (i < sle)
		    {
		      result = NO;
		      goto done;
		    }
	          u = u & ~(0xffffffff << ((5 * sle) + 1));
		  spos += sle;

                  /* How many bytes needed to encode this character?
                   */
                  if (u < 0x80)
                    {
                      i = 1;
                    }
                  else if (u < 0x800)
                    {
                      i = 2;
                    }
                  else if (u < 0x10000)
                    {
                      i = 3;
                    }
                  else 
                    {
                      i = 4;
                    }
                  if (0 && i < sle)
                    {
		      result = NO;	// Character was not minimally encoded.
		      goto done;
                    }

		  if ((u >= 0xd800) && (u <= 0xdfff))
		    {
		      result = NO;	// Unmatched half of surrogate pair.
		      goto done;
		    }
                  if (u > 0x10ffff)
                    {
		      result = NO;	// Outside the unicode range.
		      goto done;
                    }
                }
              else
		{
		  spos++;
		}
#endif

	      /*
	       * Add codepoint as either a single unichar for BMP
	       * or as a pair of surrogates for codepoints over 16 bits.
	       */

	      if (dpos >= bsize)
		{
		  GROW();
		}
	      if (u < 0x10000)
	        {
	          ptr[dpos++] = u;
	        }
	      else
	        {
                  unichar ul, uh;

                  u -= 0x10000;
                  ul = u & 0x3ff;
                  uh = (u >> 10) & 0x3ff;

	          ptr[dpos++] = uh + 0xd800;
	          if (dpos >= bsize)
		    {
		      GROW();
		    }
	          ptr[dpos++] = ul + 0xdc00;
//                  NSLog(@"Adding uh %d ul %d", uh + 0xd800, ul + 0xdc00);
	        }
	    }
#if     defined(UTF8DECODE)
          if (state != UTF8_ACCEPT)
            {
              result = NO;	// Parse failure
              goto done;
            }
#endif
	}
	break;

      case NSNonLossyASCIIStringEncoding:
        {
          unsigned int  index = 0;
          unsigned int  count = 0;

          while (index < slen)
            {
              uint8_t	c = (uint8_t)((unc)src[index++]);

              if ('\\' == c)
                {
                  if (index < slen)
                    {
                      c = (uint8_t)((unc)src[index++]);
                      if ('\\' == c)
                        {
                          count++;      // Escaped backslash
                        }
                      else if (octdigit(c)
                        && (index < slen && octdigit(src[index++]))
                        && (index < slen && octdigit(src[index++])))
                        {
                          count++;  // Octal escape
                        }
                      else if (('u' == c)
                        && (index < slen && isxdigit(src[index++]))
                        && (index < slen && isxdigit(src[index++]))
                        && (index < slen && isxdigit(src[index++]))
                        && (index < slen && isxdigit(src[index++])))
                        {
                          count++;      // Hex escape for unicode
                        }
                      else
                        {
                          result = NO;	// illegal backslash escape
                          goto done;
                        }
                    }
                  else
                    {
                      result = NO;	// unbalanced backslash
                      goto done;
                    }
                }
              else
                {
                  count++;
                }
            }
          
          if (dst == 0)
            {
              /* Just counting bytes.
               */
              dpos += count;
            }
          else
            {
              if (dpos + count + (extra ? 1 : 0) > bsize)
                {
                  if (zone == 0)
                    {
                      result = NO; /* No buffer growth possible ... fail. */
                      goto done;
                    }
                  else
                    {
                      unsigned	grow = (dpos + count) * sizeof(unichar);
                      unichar	*tmp;

                      tmp = NSZoneMalloc(zone, grow + extra * sizeof(unichar));
                      if ((ptr == buf || ptr == *dst) && (tmp != 0))
                        {
                          memcpy(tmp, ptr, bsize * sizeof(unichar));
                        }
                      if (ptr != buf && ptr != *dst)
                        {
                          NSZoneFree(zone, ptr);
                        }
                      ptr = tmp;
                      if (ptr == 0)
                        {
                          return NO;	/* Not enough memory */
                        }
                      bsize = grow / sizeof(unichar);
                    }
                }
              while (spos < slen)
                {
                  uint8_t	c = (uint8_t)((unc)src[spos++]);

                  if ('\\' == c)
                    {
                      c = (uint8_t)((unc)src[spos++]);
                      if ('\\' == c)
                        {
                          ptr[dpos++] = c;
                        }
                      else if ('u' == c)
                        {
                          int   i = 0;

                          for (count = 0; count < 4; count++)
                            {
                              c = (uint8_t)((unc)src[spos++]);
                              i *= 16;
                              if (isdigit(c))
                                {
                                  i += c - '0';
                                }
                              else if (isupper(c))
                                {
                                  i += 10 + c - 'A';
                                }
                              else
                                {
                                  i += 10 + c - 'a';
                                }
                            }
                          ptr[dpos++] = i;
                        }
                      else
                        {
                          int   i = c - '0';

                          c = (uint8_t)((unc)src[spos++]);
                          i = i * 8 + c - '0';
                          c = (uint8_t)((unc)src[spos++]);
                          i = i * 8 + c - '0';
                          ptr[dpos++] = i;
                        }
                    }
                  else
                    {
                      ptr[dpos++] = c;
                    }
                }
            }
          }
	break;

      case NSASCIIStringEncoding:
	if (dst == 0)
	  {
	    /* Just counting bytes, and we know there is exactly one
	     * unicode codepoint needed for each ascii character.
	     */
	    dpos += slen;
	  }
        else
	  {
	    /* Because we know that each ascii character is exactly
	     * one unicode character, we can check the destination
	     * buffer size and allocate more space in one go, before
	     * entering the loop where we deal with each character.
	     */
	    if (dpos + slen + (extra ? 1 : 0) > bsize)
	      {
		if (zone == 0)
		  {
		    result = NO; /* No buffer growth possible ... fail. */
		    goto done;
		  }
		else
		  {
		    unsigned	grow = (dpos + slen) * sizeof(unichar);
		    unichar	*tmp;

		    tmp = NSZoneMalloc(zone, grow + extra * sizeof(unichar));
		    if ((ptr == buf || ptr == *dst) && (tmp != 0))
		      {
			memcpy(tmp, ptr, bsize * sizeof(unichar));
		      }
		    if (ptr != buf && ptr != *dst)
		      {
			NSZoneFree(zone, ptr);
		      }
		    ptr = tmp;
		    if (ptr == 0)
		      {
			return NO;	/* Not enough memory */
		      }
		    bsize = grow / sizeof(unichar);
		  }
	      }
	    while (spos < slen)
	      {
		unichar	c = (unichar)((unc)src[spos++]);

		if (c > 127)
		  {
		    result = NO;	// Non-ascii data found in input.
		    goto done;
		  }
		ptr[dpos++] = c;
	      }
	  }
	break;

      case NSISOLatin1StringEncoding:
	if (dst == 0)
	  {
	    /* Just counting bytes, and we know there is exactly one
	     * unicode codepoint needed for each latin1 character.
	     */
	    dpos += slen;
	  }
        else
	  {
	    /* Because we know that each latin1 chartacter is exactly
	     * one unicode character, we can check the destination
	     * buffer size and allocate more space in one go, before
	     * entering the loop where we deal with each character.
	     */
	    if (dpos + slen + (extra ? 1 : 0) > bsize)
	      {
		if (zone == 0)
		  {
		    result = NO; /* No buffer growth possible ... fail. */
		    goto done;
		  }
		else
		  {
		    unsigned	grow = (dpos + slen) * sizeof(unichar);
		    unichar	*tmp;

		    tmp = NSZoneMalloc(zone, grow + extra * sizeof(unichar));
		    if ((ptr == buf || ptr == *dst) && (tmp != 0))
		      {
			memcpy(tmp, ptr, bsize * sizeof(unichar));
		      }
		    if (ptr != buf && ptr != *dst)
		      {
			NSZoneFree(zone, ptr);
		      }
		    ptr = tmp;
		    if (ptr == 0)
		      {
			return NO;	/* Not enough memory */
		      }
		    bsize = grow / sizeof(unichar);
		  }
	      }
	    while (spos < slen)
	      {
		ptr[dpos++] = (unichar)((unc)src[spos++]);
	      }
	  }
	break;

      case NSNEXTSTEPStringEncoding:
	base = Next_conv_base;
	table = Next_char_to_uni_table;
	goto tables;

      case NSISOCyrillicStringEncoding:
	base = Cyrillic_conv_base;
	table = Cyrillic_char_to_uni_table;
	goto tables;

      case NSISOLatin2StringEncoding:
	base = Latin2_conv_base;
	table = Latin2_char_to_uni_table;
	goto tables;

      case NSISOLatin9StringEncoding:
	base = Latin9_conv_base;
	table = Latin9_char_to_uni_table;
	goto tables;

      case NSISOThaiStringEncoding:
        base = Thai_conv_base;
	table = Thai_char_to_uni_table;
	goto tables;
	
#if 0
      case NSSymbolStringEncoding:
	base = Symbol_conv_base;
	table = Symbol_char_to_uni_table;
	goto tables;
#endif

tables:
	if (dst == 0)
	  {
	    /* Just counting bytes, and we know there is exactly one
	     * unicode codepoint needed for each character.
	     */
	    dpos += slen;
	  }
        else
	  {
	    /* Because we know that each character in the table is exactly
	     * one unicode character, we can check the destination
	     * buffer size and allocate more space in one go, before
	     * entering the loop where we deal with each character.
	     */
	    if (dpos + slen + (extra ? 1 : 0) > bsize)
	      {
		if (zone == 0)
		  {
		    result = NO; /* No buffer growth possible ... fail. */
		    goto done;
		  }
		else
		  {
		    unsigned	grow = (dpos + slen) * sizeof(unichar);
		    unichar	*tmp;

		    tmp = NSZoneMalloc(zone, grow + extra * sizeof(unichar));
		    if ((ptr == buf || ptr == *dst) && (tmp != 0))
		      {
			memcpy(tmp, ptr, bsize * sizeof(unichar));
		      }
		    if (ptr != buf && ptr != *dst)
		      {
			NSZoneFree(zone, ptr);
		      }
		    ptr = tmp;
		    if (ptr == 0)
		      {
			return NO;	/* Not enough memory */
		      }
		    bsize = grow / sizeof(unichar);
		  }
	      }
	    while (spos < slen)
	      {
		unc c = (unc)src[spos];

		if (c < base)
		  {
		    ptr[dpos++] = c;
		  }
		else
		  {
		    ptr[dpos++] = table[c - base];
		  }
		spos++;
	      }
	  }
	break;

      case NSGSM0338StringEncoding:
	while (spos < slen)
	  {
	    unc c = (unc)src[spos];

	    if (dpos >= bsize)
	      {
		GROW();
	      }
	    ptr[dpos] = GSM0338_char_to_uni_table[c];
	    if (c == 0x1b && spos < slen + 1)
	      {
		unsigned	i = 0;

		c = (unc)src[spos + 1];
		while (i < sizeof(GSM0338_escapes)/sizeof(GSM0338_escapes[0]))
		  {
		    if (GSM0338_escapes[i].to == c)
		      {
			ptr[dpos] = GSM0338_escapes[i].from;
			spos++;
			break;
		      }
		    i++;
		  }
	      }
	    dpos++;
	    spos++;
	  }
	break;

      default:
#ifdef HAVE_ICONV
	{
	  struct _strenc_	*encInfo;
	  unsigned char	*inbuf;
	  unsigned char	*outbuf;
	  size_t	inbytesleft;
	  size_t	outbytesleft;
	  size_t	rval;
	  const char	*estr = 0;
	  BOOL		done = NO;

	  if ((encInfo = EntrySupported(enc)) != 0)
	    {
	      estr = encInfo->iconv;
	    }
	  /* explicitly check for empty encoding name since some systems
	   * have buggy iconv_open() code which succeeds on an empty name.
	   */
	  if (estr == 0)
	    {
	      NSLog(@"GSToUnicode() No iconv for encoding x%02x", enc);
	      result = NO;
	      goto done;
	    }
	  if (slen == 0)
	    {
	      break;	// Nothing to do
	    }
	  cd = iconv_open(UNICODE_ENC, estr);
	  if (cd == (iconv_t)-1)
	    {
	      NSLog(@"GSToUnicode() No iconv for encoding %@ tried to use %s",
		GSPrivateEncodingName(enc), estr);
	      result = NO;
	      goto done;
	    }

	  inbuf = (unsigned char*)src;
	  inbytesleft = slen;
	  outbuf = (unsigned char*)ptr;
	  outbytesleft = bsize * sizeof(unichar);
	  do
	    {
	      if (inbytesleft == 0)
		{
		  done = YES;	// Flush iconv
		  rval = iconv(cd, 0, 0, (void*)&outbuf, &outbytesleft);
		}
	      else
		{
		  rval = iconv(cd,
		    (void*)&inbuf, &inbytesleft, (void*)&outbuf, &outbytesleft);
		}
	      dpos = (bsize * sizeof(unichar) - outbytesleft) / sizeof(unichar);
	      if (rval == (size_t)-1)
		{
		  if (errno == E2BIG)
		    {
		      unsigned	old = bsize;

		      GROW();
		      outbuf = (unsigned char*)&ptr[dpos];
		      outbytesleft += (bsize - old) * sizeof(unichar);
		    }
		  else
		    {
		      result = NO;
		      goto done;
		    }
		}
	    } while (!done || rval != 0);
	}
#else
	result = NO;
#endif
    }

done:
#ifdef HAVE_ICONV
  if (cd != (iconv_t)-1)
    {
      iconv_close(cd);
    }
#endif

  if (NULL == ptr)
    {
      *size = 0;
    }
  else
    {
      /*
       * Post conversion ... terminate if needed, and set output values.
       */
      if (extra != 0 && dst != 0)
        {
          ptr[dpos] = (unichar)0;
        }
      *size = dpos;
      if (dst != 0 && (result == YES || (options & GSUniShortOk)))
        {
          if (options & GSUniTemporary)
            {
              unsigned	bytes = dpos * sizeof(unichar) + extra;
              void		*r;

              /*
               * Temporary string was requested ... make one.
               */
              r = GSAutoreleasedBuffer(bytes);
              memcpy(r, ptr, bytes);
              if (ptr != buf && ptr != *dst)
                {
                  NSZoneFree(zone, ptr);
                }
              ptr = r;
              *dst = ptr;
            }
          else if (zone != 0 && (ptr == buf || bsize > dpos))
            {
              unsigned	bytes = dpos * sizeof(unichar) + extra;

              /*
               * Resizing is permitted, try ensure we return a buffer which
               * is just big enough to hold the converted string.
               */
              if (ptr == buf || ptr == *dst)
                {
                  unichar	*tmp;

                  tmp = NSZoneMalloc(zone, bytes);
                  if (tmp != 0)
                    {
                      memcpy(tmp, ptr, bytes);
                    }
                  ptr = tmp;
                }
              else
                {
                  ptr = NSZoneRealloc(zone, ptr, bytes);
                }
              *dst = ptr;
            }
          else if (ptr == buf)
            {
              ptr = NULL;
              result = NO;
            }
          else
            {
              *dst = ptr;
            }
        }
      else if (ptr != buf && dst != 0 && ptr != *dst)
        {
          NSZoneFree(zone, ptr);
        }
    }
  if (dst)
    NSCAssert(*dst != buf, @"attempted to pass out pointer to internal buffer");

  return result;
}

#undef	GROW

#define	GROW() \
if (dst == 0) \
  { \
    /* \
     * Data is just being discarded anyway, so we can \
     * reset the offset into the local buffer on the \
     * stack and pretend the buffer has grown. \
     */ \
    ptr = buf - dpos; \
    bsize = dpos + BUFSIZ; \
    if (extra != 0) \
      { \
	bsize--; \
      } \
  } \
else if (zone == 0) \
  { \
    result = NO; /* No buffer growth possible ... fail. */ \
    goto done; \
  } \
else \
  { \
    unsigned	grow = slen; \
\
    if (grow < bsize + BUFSIZ) \
      { \
	grow = bsize + BUFSIZ; \
      } \
\
    if (ptr == buf || ptr == *dst) \
      { \
	unsigned char	*tmp; \
\
	tmp = NSZoneMalloc(zone, grow + extra); \
	if (tmp != 0) \
	  { \
	    memcpy(tmp, ptr, bsize); \
	  } \
	ptr = tmp; \
      } \
    else \
      { \
	ptr = NSZoneRealloc(zone, ptr, grow + extra); \
      } \
    if (ptr == 0) \
      { \
        result = NO; /* No buffer growth possible ... fail. */ \
	goto done; \
      } \
    bsize = grow; \
  }

static inline int chop(unichar c, _ucc_ *table, int hi)
{
  int	lo = 0;

  while (hi > lo)
    {
      int	i = (hi + lo) / 2;
      unichar	from = table[i].from;

      if (from < c)
        {
	  lo = i + 1;
	}
      else if (from > c)
        {
	  hi = i;
	}
      else
        {
	  return i;	// Found
	}
    }
  return -1;		// Not found
}

/**
 * Function to convert from 16-bit unicode to 8-bit data.
 * <p>The dst argument is a pointer to a pointer to a buffer in which the
 * converted data is to be stored.  If it is a null pointer, this function
 * discards converted data, and is used only to determine the length of the
 * converted data.  If the zone argument is non-nul, the function is free
 * to allocate a larger buffer if necessary, and store this new buffer in
 * the dst argument.  It will *NOT* deallocate the original buffer!
 * </p>
 * <p>The size argument is a pointer to the initial size of the destination
 * buffer.  If the function changes the buffer size, this value will be
 * altered to the new size.  This is measured in bytes.
 * </p>
 * <p>The src argument is a pointer to the 16-bit unicode string which is
 * to be converted to 8-bit data.
 * </p>
 * <p>The slen argument is the length of the 16-bit unicode string
 * which is to be converted to 8-bit data.
 * This is measured in 16-bit characters, not bytes.
 * </p>
 * <p>The enc argument specifies the encoding type of the 8-bit byte sequence
 * which is to be produced from the 16-bit unicode.
 * </p>
 * <p>The zone argument specifies a memory zone in which the function may
 * allocate a buffer to return data in.
 * If this is nul, the function will fail if the originally supplied buffer
 * is not big enough (unless dst is a null pointer ... indicating that
 * converted data is to be discarded).<br />
 * If the library is built for garbage collecting, the zone argument is used
 * only as a marker to say whether the function may allocate memory (zone
 * is non-null) or not (zone is null).
 * </p>
 * The options argument controls some special behavior.
 * <list>
 * <item>If GSUniStrict is set, the function will fail if a character is
 * encountered in the source which can't be converted.  Otherwise, some
 * approximation or marker will be placed in the destination.</item>
 * <item>If GSUniTerminate is set, the function is expected to nul terminate
 * the output data, and will assume that it is safe to place the nul
 * just beyond the end of the stated buffer size.
 * Also, if the function grows the buffer, it will allow for an extra
 * termination byte.</item>
 * <item>If GSUniTemporary is set, the function will return the results in
 * an autoreleased buffer rather than in a buffer that the caller must
 * release.</item>
 * <item>If GSUniBOM is set, the function will read the first unicode
 * character as a byte order marker.</item>
 * <item>If GSUniShortOk is set, the function will return a buffer containing
 * any decoded characters even if the whole conversion fails.</item>
 * </list>
 * <p>On return, the function result is a flag indicating success (YES)
 * or failure (NO), and on success, the value stored in size is the number
 * of bytes in the converted data.  The converted data itself is
 * stored in the location given by dst.<br />
 * NB. If the value stored in dst has been changed, it is a pointer to
 * allocated memory which the caller is responsible for freeing, and the
 * caller is <em>still</em> responsible for freeing the original buffer.
 * </p>
 */
BOOL
GSFromUnicode(unsigned char **dst, unsigned int *size, const unichar *src,
  unsigned int slen, NSStringEncoding enc, NSZone *zone,
  unsigned int options)
{
  unsigned char	buf[BUFSIZ];
  unsigned char	*ptr;
  unsigned	bsize;
  unsigned	dpos = 0;	// Offset into destination buffer.
  unsigned	spos = 0;	// Offset into source buffer.
  unsigned	extra = (options & GSUniTerminate) ? 1 : 0;
  BOOL		strict = (options & GSUniStrict) ? YES : NO;
  unichar	base = 0;
  _ucc_		*table = 0;
  unsigned	tsize = 0;
  unsigned char	escape = 0;
  _ucc_		*etable = 0;
  unsigned	etsize = 0;
  _ucc_		*ltable = 0;
  unsigned	ltsize = 0;
  BOOL		swapped = NO;
  BOOL		result = YES;
#ifdef HAVE_ICONV
  iconv_t	cd = (iconv_t)-1;
#endif

  if (options & GSUniBOM)
    {
      if (slen == 0)
	{
	  *size = 0;
	  result = NO;	// Missing byte order marker.
	}
      else
	{
	  unichar	c;

	  c = *src++;
	  slen--;
	  if (c != 0xFEFF)
	    {
	      if (c == 0xFFFE)
		{
		  swapped = YES;
		}
	      else
		{
		  *size = 0;
		  result = NO;	// Illegal byte order marker.
		}
	    }
	}
    }

  /*
   * Ensure we have an initial buffer set up to decode data into.
   */
  if (dst == 0 || *size == 0)
    {
      ptr = buf;
      bsize = (extra != 0) ? BUFSIZ - 1 : BUFSIZ;
    }
  else
    {
      ptr = *dst;
      bsize = *size;
    }

  if (result == NO)
    {
      goto done;
    }

#ifdef HAVE_ICONV
  if (strict == NO
    && enc != NSUTF8StringEncoding
    && enc != NSGSM0338StringEncoding)
    {
      goto iconv_start;	// For lossy conversion
    }
#endif

  switch (enc)
    {
      case NSUTF8StringEncoding:
	{
	  if (swapped == YES)
	    {
	      while (spos < slen)
		{
		  unichar 	u1, u2;
		  unsigned char	reversed[8];
		  unsigned long	u;
		  int		sl;
		  int		i;

		  /* get first unichar */
		  u1 = src[spos++];
		  u1 = (((u1 & 0xff00) >> 8) + ((u1 & 0x00ff) << 8));

		  /* Fast track ... if this is actually an ascii character
		   * it just converts straight to utf-8
		   */
		  if (u1 <= 0x7f)
		    {
		      if (dpos >= bsize)
			{
			  GROW();
			}
		      ptr[dpos++] = (unsigned char)u1;
		      continue;
		    }

		  // 0xfeff is a zero-width-no-break-space inside text
		  if (u1 >= 0xdc00 && u1 <= 0xdfff)	// bad pairing
		    {
		      if (strict)
			{
			  result = NO;
			  goto done;
			}
		      continue;	// Skip invalid character.
		    }

		  /* possibly get second character and calculate 'u' */
		  if ((u1 >= 0xd800) && (u1 < 0xdc00))
		    {
		      if (spos >= slen)
			{
			  if (strict)
			    {
			      result = NO;
			      goto done;
			    }
			  continue;	// At end.
			}

		      /* get second unichar */
		      u2 = src[spos++];
		      u2 = (((u2 & 0xff00) >> 8) + ((u2 & 0x00ff) << 8));

		      if ((u2 < 0xdc00) || (u2 > 0xdfff))
			{
			  spos--;
			  if (strict)
			    {
			      result = NO;
			      goto done;
			    }
			  continue;	// Skip bad half of surrogate pair.
			}

		      /* make the full value */
		      u = ((unsigned long)(u1 - 0xd800) * 0x400)
			+ (u2 - 0xdc00) + 0x10000;
		    }
		  else
		    {
		      u = u1;
		    }

		  /* calculate the sequence length
		   * a length of 1 was dealt with earlier
		   */
		  if (u <= 0x7ff)
		    {
		      sl = 2;
		    }
		  else if (u <= 0xffff)
		    {
		      sl = 3;
		    }
		  else if (u <= 0x1fffff)
		    {
		      sl = 4;
		    }
		  else if (u <= 0x3ffffff)
		    {
		      sl = 5;
		    }
		  else
		    {
		      sl = 6;
		    }

		  /* make sure we have enough space for it */
		  while (dpos + sl >= bsize)
		    {
		      GROW();
		    }

		  /* split value into reversed array */
		  for (i = 0; i < sl; i++)
		    {
		      reversed[i] = (u & 0x3f);
		      u = u >> 6;
		    }

		  ptr[dpos++] = reversed[sl-1] | ((0xff << (8-sl)) & 0xff);
		  /* add bytes into the output sequence */
		  for (i = sl - 2; i >= 0; i--)
		    {
		      ptr[dpos++] = reversed[i] | 0x80;
		    }
		}
	    }
	  else
	    {
	      while (spos < slen)
		{
		  unichar 	u1, u2;
		  unsigned char	reversed[8];
		  unsigned long	u;
		  int		sl;
		  int		i;

		  /* get first unichar */
		  u1 = src[spos++];

		  /* Fast track ... if this is actually an ascii character
		   * it just converts straight to utf-8
		   */
		  if (u1 <= 0x7f)
		    {
		      if (dpos >= bsize)
			{
			  GROW();
			}
		      ptr[dpos++] = (unsigned char)u1;
		      continue;
		    }

		  // 0xfeff is a zero-width-no-break-space inside text
		  if (u1 >= 0xdc00 && u1 <= 0xdfff)	// bad pairing
		    {
		      if (strict)
			{
			  result = NO;
			  goto done;
			}
		      continue;	// Skip invalid character.
		    }

		  /* possibly get second character and calculate 'u' */
		  if ((u1 >= 0xd800) && (u1 < 0xdc00))
		    {
		      if (spos >= slen)
			{
			  if (strict)
			    {
			      result = NO;
			      goto done;
			    }
			  continue;	// At end.
			}

		      /* get second unichar */
		      u2 = src[spos++];

		      if ((u2 < 0xdc00) || (u2 > 0xdfff))
			{
			  spos--;
			  if (strict)
			    {
			      result = NO;
			      goto done;
			    }
			  continue;	// Skip bad half of surrogate pair.
			}

		      /* make the full value */
		      u = ((unsigned long)(u1 - 0xd800) * 0x400)
			+ (u2 - 0xdc00) + 0x10000;
		    }
		  else
		    {
		      u = u1;
		    }

		  /* calculate the sequence length
		   * a length of 1 was dealt with earlier
		   */
		  if (u <= 0x7ff)
		    {
		      sl = 2;
		    }
		  else if (u <= 0xffff)
		    {
		      sl = 3;
		    }
		  else if (u <= 0x1fffff)
		    {
		      sl = 4;
		    }
		  else if (u <= 0x3ffffff)
		    {
		      sl = 5;
		    }
		  else
		    {
		      sl = 6;
		    }

		  /* make sure we have enough space for it */
		  while (dpos + sl >= bsize)
		    {
		      GROW();
		    }

		  /* split value into reversed array */
		  for (i = 0; i < sl; i++)
		    {
		      reversed[i] = (u & 0x3f);
		      u = u >> 6;
		    }

		  ptr[dpos++] = reversed[sl-1] | ((0xff << (8-sl)) & 0xff);
		  /* add bytes into the output sequence */
		  for (i = sl - 2; i >= 0; i--)
		    {
		      ptr[dpos++] = reversed[i] | 0x80;
		    }
		}
	    }
        }
        break;

      case NSNonLossyASCIIStringEncoding:
        {
          unsigned int  index = 0;
          unsigned int  count = 0;

          if (YES == swapped)
            {
              while (index < slen)
                {
                  unichar	u = src[index++];

                  u = (((u & 0xff00) >> 8) + ((u & 0x00ff) << 8));
                  if (u < 256)
                    {
                      if ((u >= ' ' && u < 127)
                        || '\r' == u || '\n' == u || '\t' == u)
                        {
                          count++;
                          if ('\\' == u)
                            {
                              count++;
                            }
                        }
                      else
                        {
                          count += 4;
                        }
                    }
                  else
                    {
                      count += 12;
                    }
                }
            }
          else
            {
              while (index < slen)
                {
                  unichar	u = src[index++];

                  if (u < 256)
                    {
                      if ((u >= ' ' && u < 127)
                        || '\r' == u || '\n' == u || '\t' == u)
                        {
                          count++;
                          if ('\\' == u)
                            {
                              count++;
                            }
                        }
                      else
                        {
                          count += 4;
                        }
                    }
                  else
                    {
                      count += 6;
                    }
                }
            }
          if (dst == 0)
            {
              /* Just counting bytes ...
               */
              dpos = count;
            }
          else
            {
              /* We can now check the destination buffer size and allocate
               * more space in one go, before entering the loop where we
               * deal with each character.
               */
              if (count > bsize)
                {
                  if (zone == 0)
                    {
                      result = NO; /* No buffer growth possible ... fail. */
                      goto done;
                    }
                  else
                    {
                      uint8_t	*tmp;

                      tmp = NSZoneMalloc(zone, count + extra);
                      if (ptr != buf && ptr != *dst)
                        {
                          NSZoneFree(zone, ptr);
                        }
                      ptr = tmp;
                      if (ptr == 0)
                        {
                          return NO;	/* Not enough memory */
                        }
                      bsize = count;
                    }
                }
              index = 0;
              while (index < slen)
                {
                  unichar	u = src[index++];

                  if (YES == swapped)
                    {
                      u = (((u & 0xff00) >> 8) + ((u & 0x00ff) << 8));
                    }
                  if (u < 256)
                    {
                      if ((u >= ' ' && u < 127)
                        || '\r' == u || '\n' == u || '\t' == u)
                        {
                          ptr[dpos++] = (unsigned char)u;
                          if ('\\' == u)
                            {
                              ptr[dpos++] = (unsigned char)u;
                            }
                        }
                      else
                        {
                          dpos += sprintf((char*)&ptr[dpos], "\\%03o", u);
                        }
                    }
                  else
                    {
                      dpos += sprintf((char*)&ptr[dpos], "\\u%04x", u);
                    }
                }
            }
          }
        goto done;

      case NSASCIIStringEncoding:
	base = 128;
	goto bases;

      case NSISOLatin1StringEncoding:
      case NSUnicodeStringEncoding:
	base = 256;
	goto bases;

bases:
	if (dst == 0)
	  {
	    /* Just counting bytes, and we know there is exactly one
	     * unicode codepoint needed for each character.
	     */
	    dpos = slen;
	  }
        else
	  {
	    /* Because we know that each ascii character is exactly
	     * one unicode character, we can check the destination
	     * buffer size and allocate more space in one go, before
	     * entering the loop where we deal with each character.
	     */
	    if (slen > bsize)
	      {
		if (zone == 0)
		  {
		    result = NO; /* No buffer growth possible ... fail. */
		    goto done;
		  }
		else
		  {
		    uint8_t	*tmp;

		    tmp = NSZoneMalloc(zone, slen + extra);
		    if (ptr != buf && ptr != *dst)
		      {
			NSZoneFree(zone, ptr);
		      }
		    ptr = tmp;
		    if (ptr == 0)
		      {
			return NO;	/* Not enough memory */
		      }
		    bsize = slen;
		  }
	      }
	  }
	if (strict == NO)
	  {
	    if (swapped == YES)
	      {
		while (spos < slen)
		  {
		    unichar	u = src[spos++];

		    u = (((u & 0xff00) >> 8) + ((u & 0x00ff) << 8));
		    if (u < base)
		      {
			ptr[dpos++] = (unsigned char)u;
		      }
		    else
		      {
			ptr[dpos++] =  '?';
		      }
		  }
	      }
	    else
	      {
		while (spos < slen)
		  {
		    unichar	u = src[spos++];

		    if (u < base)
		      {
			ptr[dpos++] = (unsigned char)u;
		      }
		    else
		      {
			ptr[dpos++] =  '?';
		      }
		  }
	      }
	  }
	else
	  {
	    if (swapped == YES)
	      {
		while (spos < slen)
		  {
		    unichar	u = src[spos++];

		    u = (((u & 0xff00) >> 8) + ((u & 0x00ff) << 8));
		    if (u < base)
		      {
			ptr[dpos++] = (unsigned char)u;
		      }
		    else
		      {
			result = NO;
			goto done;
		      }
		  }
	      }
	    else
	      {
		while (spos < slen)
		  {
		    unichar	u = src[spos++];

		    if (u < base)
		      {
			ptr[dpos++] = (unsigned char)u;
		      }
		    else
		      {
			result = NO;
			goto done;
		      }
		  }
	      }
	  }
	break;

      case NSNEXTSTEPStringEncoding:
	base = Next_conv_base;
	table = Next_uni_to_char_table;
	tsize = Next_uni_to_char_table_size;
	goto tables;

      case NSISOCyrillicStringEncoding:
	base = Cyrillic_conv_base;
	table = Cyrillic_uni_to_char_table;
	tsize = Cyrillic_uni_to_char_table_size;
	goto tables;

      case NSISOLatin2StringEncoding:
	base = Latin2_conv_base;
	table = Latin2_uni_to_char_table;
	tsize = Latin2_uni_to_char_table_size;
	goto tables;

      case NSISOLatin9StringEncoding:
	base = Latin9_conv_base;
	table = Latin9_uni_to_char_table;
	tsize = Latin9_uni_to_char_table_size;
	goto tables;

      case NSISOThaiStringEncoding:
        base = Thai_conv_base;
	table = Thai_uni_to_char_table;
	tsize = Thai_uni_to_char_table_size;
	goto tables;

#if 0
      case NSSymbolStringEncoding:
	base = Symbol_conv_base;
	table = Symbol_uni_to_char_table;
	tsize = Symbol_uni_to_char_table_size;
	goto tables;
#endif

      case NSGSM0338StringEncoding:
	base = 0;
	table = GSM0338_uni_to_char_table;
	tsize = GSM0338_tsize;
        escape = 0x1b;
	etable = GSM0338_escapes;
	etsize = GSM0338_esize;
	if (strict == NO)
	  {
	    ltable = GSM0338_lossy;
	    ltsize = GSM0338_lsize;
	  }
	goto tables;

tables:
	while (spos < slen)
	  {
	    unichar	u = src[spos++];
	    int	i;

	    /* Swap byte order if necessary */
	    if (swapped == YES)
	      {
		u = (((u & 0xff00) >> 8) + ((u & 0x00ff) << 8));
	      }

	    /* Grow output buffer to make room if necessary */
	    if (dpos >= bsize)
	      {
		GROW();
	      }

	    if (u < base)
	      {
		/*
		 * The character set has a lower section whose contents
		 * are identical to unicode, so no mapping is needed.
		 */
		ptr[dpos++] = (unsigned char)u;
	      }
	    else if (table != 0 && (i = chop(u, table, tsize)) >= 0)
	      {
		/*
		 * The character mapping is found in a basic table.
		 */
		ptr[dpos++] = table[i].to;
	      }
	    else if (etable != 0 && (i = chop(u, etable, etsize)) >= 0)
	      {
		/*
		 * The character mapping is found in a table of simple
		 * escape sequences consisting of an escape byte followed
		 * by another single byte.
		 */
		ptr[dpos++] = escape;
		if (dpos >= bsize)
		  {
		    GROW();
		  }
		ptr[dpos++] = etable[i].to;
	      }
	    else if (ltable != 0 && (i = chop(u, ltable, ltsize)) >= 0)
	      {
		/*
		 * The character is found in a lossy mapping table.
		 */
		ptr[dpos++] = ltable[i].to;
	      }
	    else if (strict == NO)
	      {
		/*
		 * The default lossy mapping generates a question mark.
		 */
		ptr[dpos++] = '?';
	      }
	    else
	      {
		/*
		 * No mapping has been found.
		 */
		result = NO;
		goto done;
	      }
	  }
	break;

      default:
#ifdef HAVE_ICONV
iconv_start:
	{
	  struct _strenc_	*encInfo;
	  unsigned char	*inbuf;
	  unsigned char	*outbuf;
	  size_t	inbytesleft;
	  size_t	outbytesleft;
	  size_t	rval;
	  const char	*estr = 0;
	  BOOL		done = NO;

	  if ((encInfo = EntrySupported(enc)) != 0)
	    {
	      if (strict == NO)
		{
		  /*
		   * Try to transliterate where no direct conversion
		   * is available.
		   */
		  estr = encInfo->lossy;
		}
	      if (estr == 0)
		{
		  estr = encInfo->iconv;
		}
	    }
	  
	  /* explicitly check for empty encoding name since some systems
	   * have buggy iconv_open() code which succeeds on an empty name.
	   */
	  if (estr == 0)
	    {
	      NSLog(@"GSFromUnicode() No iconv for encoding x%02x", enc);
	      result = NO;
	      goto done;
	    }
	  if (slen == 0)
	    {
	      break;	// Nothing to convert.
	    }
	  cd = iconv_open(estr, UNICODE_ENC);
	  if (cd == (iconv_t)-1)
	    {
	      NSLog(@"GSFromUnicode() No iconv for encoding %@ tried to use %s",
		GSPrivateEncodingName(enc), estr);
	      result = NO;
	      goto done;
	    }

	  inbuf = (unsigned char*)src;
	  inbytesleft = slen * sizeof(unichar);
	  outbuf = (unsigned char*)ptr;
	  outbytesleft = bsize;
	  do
	    {
	      if (inbytesleft == 0)
		{
		  done = YES; // Flush buffer
		  rval = iconv(cd, 0, 0, (void*)&outbuf, &outbytesleft);
		}
	      else
		{
		  rval = iconv(cd,
		    (void*)&inbuf, &inbytesleft, (void*)&outbuf, &outbytesleft);
		}
	      dpos = bsize - outbytesleft;
	      if (rval != 0)
		{
		  if (rval == (size_t)-1)
		    {
		      if (errno == E2BIG)
			{
			  unsigned	old = bsize;

			  GROW();
			  outbuf = (unsigned char*)&ptr[dpos];
			  outbytesleft += (bsize - old);
			}
		      else if (errno == EILSEQ)
			{
			  if (strict == YES)
			    {
			      result = NO;
			      goto done;
			    }
			  /*
			   * If we are allowing lossy conversion, we replace any
			   * unconvertable character with a question mark.
			   */
			  if (outbytesleft > 0)
			    {
			      *outbuf++ = '?';
			      outbytesleft--;
			      inbuf += sizeof(unichar);
			      inbytesleft -= sizeof(unichar);
			    }
			}
		      else
			{
			  result = NO;
			  goto done;
			}
		    }
		  else if (strict == YES)
		    {
		      /*
		       * A positive return from iconv indicates some
		       * irreversible (ie lossy) conversions took place,
		       * so if we are doing strict conversions we must fail.
		       */
		      result = NO;
		      goto done;
		    }
		}
	    } while (!done || rval != 0);
	}
#else
	result = NO;
	goto done;
#endif
    }

  done:
#ifdef HAVE_ICONV
  if (cd != (iconv_t)-1)
    {
      iconv_close(cd);
    }
#endif

  if (NULL == ptr)
    {
      *size = 0;
    }
  else
    {
      /*
       * Post conversion ... set output values.
       */
      if (extra != 0)
        {
          ptr[dpos] = (unsigned char)0;
        }
      *size = dpos;
      if (dst != 0 && (result == YES || (options & GSUniShortOk)))
        {
          if (options & GSUniTemporary)
            {
              unsigned	bytes = dpos + extra;
              void		*r;

              /*
               * Temporary string was requested ... make one.
               */
              r = GSAutoreleasedBuffer(bytes);
              memcpy(r, ptr, bytes);
              if (ptr != buf && ptr != *dst)
                {
                  NSZoneFree(zone, ptr);
                }
              ptr = r;
              *dst = ptr;
            }
          else if (zone != 0 && (ptr == buf || bsize > dpos))
            {
              unsigned	bytes = dpos + extra;

              /*
               * Resizing is permitted - try ensure we return a buffer
               * which is just big enough to hold the converted string.
               */
              if (ptr == buf || ptr == *dst)
                {
                  unsigned char	*tmp;

                  tmp = NSZoneMalloc(zone, bytes);
                  if (tmp != 0)
                    {
                      memcpy(tmp, ptr, bytes);
                    }
                  ptr = tmp;
                }
              else
                {
                  ptr = NSZoneRealloc(zone, ptr, bytes);
                }
              *dst = ptr;
            }
          else if (ptr == buf)
            {
              ptr = NULL;
              result = NO;
            }
          else
            {
              *dst = ptr;
            }
        }
      else if (ptr != buf && dst != 0 && ptr != *dst)
        {
          NSZoneFree(zone, ptr);
        }
    }

  if (dst)
    NSCAssert(*dst != buf, @"attempted to pass out pointer to internal buffer");

  return result;
}

#undef	GROW



NSStringEncoding*
GSPrivateAvailableEncodings()
{
  if (_availableEncodings == 0)
    {
      GSSetupEncodingTable();
      (void)pthread_mutex_lock(&local_lock);
      if (_availableEncodings == 0)
	{
	  NSStringEncoding	*encodings;
	  unsigned		pos;
	  unsigned		i;

	  /*
	   * Now build up a list of supported encodings ... in the
	   * format needed to support [NSString+availableStringEncodings]
	   * Check to see what iconv support we have as we go along.
	   * This is also the place where we determine the name we use
	   * for iconv to support unicode.
	   */
	  encodings = malloc(sizeof(NSStringEncoding) * (encTableSize+1));
	  pos = 0;
	  for (i = 0; i < encTableSize+1; i++)
	    {
	      if (GSPrivateIsEncodingSupported(i) == YES)
		{
		  encodings[pos++] = i;
		}
	    }
	  encodings[pos] = 0;
	  _availableEncodings = encodings;
	}
      (void)pthread_mutex_unlock(&local_lock);
    }
  return _availableEncodings;
}

NSStringEncoding
GSPrivateCStringEncoding(const char *encoding)
{
  NSStringEncoding  enc = GSUndefinedEncoding;

  if (0 != encoding)
    {
      GSSetupEncodingTable();

      if (strcmp(encoding, "ANSI_X3.4-1968") == 0 /* glibc */
        || strcmp(encoding, "ISO_646.IRV:1983") == 0 /* glibc */
        || strcmp(encoding, "646") == 0 /* Solaris NetBSD */)
        enc = NSISOLatin1StringEncoding;
      else if (strcmp(encoding, "EUC-JP") == 0 /* glibc */
        /* HP-UX IRIX OSF/1 Solaris NetBSD */
        || strcmp(encoding, "eucJP") == 0
        || strcmp(encoding, "IBM-eucJP") == 0 /* AIX */)
        enc = NSJapaneseEUCStringEncoding;
      else if (strcmp(encoding, "UTF-8") == 0 /* glibc AIX OSF/1 Solaris */
        || strcmp(encoding, "utf8") == 0 /* HP-UX */)
        enc = NSUTF8StringEncoding;
      else if (strcmp(encoding, "ISO-8859-1") == 0 /* glibc */
        /* AIX IRIX OSF/1 Solaris NetBSD */
        || strcmp(encoding, "ISO8859-1") == 0
        || strcmp(encoding, "iso88591") == 0 /* HP-UX */)
        enc = NSISOLatin1StringEncoding;
      else if (strcmp(encoding, "IBM-932") == 0 /* AIX */
        || strcmp(encoding, "SJIS") == 0 /* HP-UX OSF/1 NetBSD */
        || strcmp(encoding, "PCK") == 0 /* Solaris */)
        enc = NSShiftJISStringEncoding;
      else if (strcmp(encoding, "ISO-8859-2") == 0 /* glibc */
        /* AIX IRIX OSF/1 Solaris NetBSD */
        || strcmp(encoding, "ISO8859-2") == 0
        || strcmp(encoding, "iso88592") == 0 /* HP-UX */)
        enc = NSISOLatin2StringEncoding;
      else if (strcmp(encoding, "CP1251") == 0 /* glibc */
        || strcmp(encoding, "ansi-1251") == 0 /* Solaris */)
        enc = NSWindowsCP1251StringEncoding;
      else if (strcmp(encoding, "CP1252") == 0 /*  */
        || strcmp(encoding, "IBM-1252") == 0 /* AIX */)
        enc = NSWindowsCP1252StringEncoding;
      else if (strcmp(encoding, "ISO-8859-5") == 0 /* glibc */
        /* AIX IRIX OSF/1 Solaris NetBSD */
        || strcmp(encoding, "ISO8859-5") == 0
        || strcmp(encoding, "iso88595") == 0 /* HP-UX */)
        enc = NSISOCyrillicStringEncoding;
      else if (strcmp(encoding, "KOI8-R") == 0 /* glibc */
        || strcmp(encoding, "koi8-r") == 0 /* Solaris */)
        enc = NSKOI8RStringEncoding;
      else if (strcmp(encoding, "ISO-8859-3") == 0 /* glibc */
        || strcmp(encoding, "ISO8859-3") == 0 /* Solaris */)
        enc = NSISOLatin3StringEncoding;
      else if (strcmp(encoding, "ISO-8859-4") == 0 /*  */
        || strcmp(encoding, "ISO8859-4") == 0 /* OSF/1 Solaris NetBSD */)
        enc = NSISOLatin4StringEncoding;
      else if (strcmp(encoding, "ISO-8859-6") == 0 /* glibc */
        || strcmp(encoding, "ISO8859-6") == 0 /* AIX Solaris */
        || strcmp(encoding, "iso88596") == 0 /* HP-UX */)
        enc = NSISOArabicStringEncoding;
      else if (strcmp(encoding, "ISO-8859-7") == 0 /* glibc */
        || strcmp(encoding, "ISO8859-7") == 0 /* AIX IRIX OSF/1 Solaris */
        || strcmp(encoding, "iso88597") == 0 /* HP-UX */)
        enc = NSISOGreekStringEncoding;
      else if (strcmp(encoding, "ISO-8859-8") == 0 /* glibc */
        || strcmp(encoding, "ISO8859-8") == 0 /* AIX OSF/1 Solaris */
        || strcmp(encoding, "iso88598") == 0 /* HP-UX */)
        enc = NSISOHebrewStringEncoding;
      else if (strcmp(encoding, "ISO-8859-9") == 0 /* glibc */
        || strcmp(encoding, "ISO8859-9") == 0 /* AIX IRIX OSF/1 Solaris */
        || strcmp(encoding, "iso88599") == 0 /* HP-UX */)
        enc = NSISOLatin5StringEncoding;
      else if (strcmp(encoding, "ISO-8859-10") == 0 /*  */
        || strcmp(encoding, "ISO8859-10") == 0 /*  */)
        enc = NSISOLatin6StringEncoding;
      else if (strcmp(encoding, "TIS-620") == 0 /* glibc AIX */
        || strcmp(encoding, "tis620") == 0 /* HP-UX */
        || strcmp(encoding, "TIS620.2533") == 0 /* Solaris */
        || strcmp(encoding, "TACTIS") == 0 /* OSF/1 */)
        enc = NSISOThaiStringEncoding;
      else if (strcmp(encoding, "ISO-8859-13") == 0 /* glibc */
        || strcmp(encoding, "ISO8859-13") == 0 /*  */
        || strcmp(encoding, "IBM-921") == 0 /* AIX */)
        enc = NSISOLatin7StringEncoding;
      else if (strcmp(encoding, "ISO-8859-14") == 0 /* glibc */
        || strcmp(encoding, "ISO8859-14") == 0 /*  */)
        enc = NSISOLatin8StringEncoding;
      else if (strcmp(encoding, "ISO-8859-15") == 0 /* glibc */
        /* AIX OSF/1 Solaris NetBSD */
        || strcmp(encoding, "ISO8859-15") == 0
        || strcmp(encoding, "iso885915") == 0 /* HP-UX */)
        enc = NSISOLatin9StringEncoding;
      else if (strcmp(encoding, "GB2312") == 0 /* glibc */
        || strcmp(encoding, "gb2312") == 0 /* Solaris */
        || strcmp(encoding, "eucCN") == 0 /* IRIX NetBSD */
        || strcmp(encoding, "IBM-eucCN") == 0 /* AIX */
        || strcmp(encoding, "hp15CN") == 0 /* HP-UX */)
        enc = NSGB2312StringEncoding;
      else if (strcmp(encoding, "BIG5") == 0 /* glibc Solaris NetBSD */
        || strcmp(encoding, "big5") == 0 /* AIX HP-UX OSF/1 */)
        enc = NSBIG5StringEncoding;
      else if (strcmp(encoding, "EUC-KR") == 0 /* glibc */
        || strcmp(encoding, "eucKR") == 0 /* HP-UX IRIX OSF/1 NetBSD */
        || strcmp(encoding, "IBM-eucKR") == 0 /* AIX */
        || strcmp(encoding, "5601") == 0 /* Solaris */)
        enc = NSKoreanEUCStringEncoding;
    }

  if (enc == GSUndefinedEncoding)
    {
#ifdef __ANDROID__
      // Android uses UTF-8 as default encoding (e.g. for file paths)
      enc = NSUTF8StringEncoding;
#else
      enc = NSISOLatin1StringEncoding;
#endif
    }
  else if (GSPrivateIsEncodingSupported(enc) == NO)
    {
      fprintf(stderr, "WARNING: %s - encoding not implemented as "
              "c string encoding.\n", encoding);
      fprintf(stderr, "  NSISOLatin1StringEncoding used instead.\n");
      enc = NSISOLatin1StringEncoding;
    }

  return enc;
}

NSStringEncoding
GSPrivateDefaultCStringEncoding()
{
  if (defEnc == GSUndefinedEncoding)
    {
      const char	*encoding = 0;
      unsigned int	count;

      GSSetupEncodingTable();

      (void)pthread_mutex_lock(&local_lock);
      if (defEnc != GSUndefinedEncoding)
	{
	  (void)pthread_mutex_unlock(&local_lock);
	  return defEnc;
	}

      encoding = getenv("GNUSTEP_STRING_ENCODING");
      if (encoding != 0)
	{
	  count = 0;
	  while (str_encoding_table[count].enc
	    && strcasecmp(str_encoding_table[count].ename, encoding)
	    && strcasecmp(str_encoding_table[count].iconv, encoding))
	    {
	      count++;
	    }
	  if (str_encoding_table[count].enc)
	    {
	      defEnc = str_encoding_table[count].enc;
	    }
	  else
	    {
	      fprintf(stderr,
		      "WARNING: %s - encoding not supported.\n", encoding);
	      fprintf(stderr,
		      "  NSISOLatin1StringEncoding set as default.\n");
	      defEnc = NSISOLatin1StringEncoding;
	    }
	}
      if (defEnc == GSUndefinedEncoding)
	{
	  defEnc = GSPrivateICUCStringEncoding();
	}
      if (defEnc == GSUndefinedEncoding)
	{
	  defEnc = NSISOLatin1StringEncoding;
	}
      else if (GSPrivateIsEncodingSupported(defEnc) == NO)
	{
	  fprintf(stderr, "WARNING: %s - encoding not implemented as "
		  "default c string encoding.\n", encoding);
	  fprintf(stderr,
		  "  NSISOLatin1StringEncoding set as default.\n");
	  defEnc = NSISOLatin1StringEncoding;
	}

      (void)pthread_mutex_unlock(&local_lock);
    }
  return defEnc;
}

NSString*
GSPrivateEncodingName(NSStringEncoding encoding)
{
  struct _strenc_	*encInfo;

  if ((encInfo = EntrySupported(encoding)) == NULL)
    {
      return @"Unknown encoding";
    }
  return [NSString stringWithUTF8String: encInfo->ename];
}

BOOL
GSPrivateIsByteEncoding(NSStringEncoding encoding)
{
  struct _strenc_	*encInfo;

  if ((encInfo = EntrySupported(encoding)) == NULL)
    {
      return NO;
    }
  return encInfo->eightBit;
}

/* Returns the C-String encoding as used by native locale functions.
 * We can use this to convert strings produced by those functions to
 * NSString objects.
 */
NSStringEncoding
GSPrivateNativeCStringEncoding()
{
  if (natEnc == GSUndefinedEncoding)
    {
      char	encbuf[BUFSIZ];
#if HAVE_LANGINFO_CODESET
      char      *old;

      /* Take it from the system locale information.  */
      [gnustep_global_lock lock];
      /* Initialise locale system by setting current locale from
       * environment and then resetting it.  Must be done before
       * any call to nl_langinfo()
       */
      if (0 != (old = setlocale(LC_CTYPE, "")))
        {
          setlocale(LC_CTYPE, old);
        }
      strncpy(encbuf, nl_langinfo(CODESET), sizeof(encbuf)-1);
      encbuf[sizeof(encbuf)-1] = '\0';
      [gnustep_global_lock unlock];
#else
      encbuf[0] = '\0';
#endif
      natEnc = GSPrivateCStringEncoding(encbuf);
    }
  return natEnc;
}

/* Returns the C-String encoding as used by ICU library functions.
 * We can use this to convert strings produced by those functions
 * to NSString objects.
 */
NSStringEncoding
GSPrivateICUCStringEncoding()
{
  if (icuEnc == GSUndefinedEncoding)
    {
      const char        *encoding = 0;
#if HAVE_UNICODE_UCNV_H
      const char *defaultName;
      UErrorCode err = U_ZERO_ERROR;

      defaultName = ucnv_getDefaultName ();
      encoding = ucnv_getStandardName (defaultName, "MIME", &err);
      if (0 == encoding)
        {
          encoding = ucnv_getStandardName (defaultName, "IANA", &err);
        }
#endif
      icuEnc = GSPrivateCStringEncoding(encoding);
    }
  return icuEnc;
}

