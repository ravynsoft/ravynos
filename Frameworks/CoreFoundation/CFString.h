/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFString.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFSTRING__)
#define __COREFOUNDATION_CFSTRING__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFCharacterSet.h>
#include <CoreFoundation/CFLocale.h>
#include <stdarg.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

/*
Please note: CFStrings are conceptually an array of Unicode characters.
However, in general, how a CFString stores this array is an implementation
detail. For instance, CFString might choose to use an array of 8-bit characters
to store its contents, or it might use multiple blocks of memory, or whatever.
This is especially true since CFString is toll-free bridged with NSString, enabling
any NSString instance to be used as a CFString. Furthermore, the implementation
may change depending on the default system encoding, the user's language, 
or even a release or update of the OS.

What this means is that you should use the following advanced functions with care:

  CFStringGetPascalStringPtr()
  CFStringGetCStringPtr()
  CFStringGetCharactersPtr()

These functions are provided for optimization only. They will either return the desired
pointer quickly, in constant time, or they return NULL. They might choose to return NULL
for many reasons; for instance it's possible that for users running in different
languages these sometimes return NULL; or in a future OS release the first two might
switch to always returning NULL. Never observing NULL returns in your usages of these
functions does not mean they won't ever return NULL. (But note the CFStringGetCharactersPtr()
exception mentioned further below.)

In your usages of these functions, if you get a NULL return, use the non-Ptr version
of the functions as shown in this example:

  char buffer[BUFSIZE];
  const char *ptr = CFStringGetCStringPtr(str, encoding);
  if (ptr == NULL) {
      if (CFStringGetCString(str, buffer, BUFSIZE, encoding)) ptr = buffer;
  }

Note that CFStringGetCString() or CFStringGetPascalString() calls might still fail --- but
that will happen in two circumstances only: The conversion from the UniChar contents of CFString
to the specified encoding fails, or the buffer is too small. If they fail, that means
the conversion was not possible.

If you need a copy of the buffer in the above example, you might consider simply calling
CFStringGetCString() in all cases --- CFStringGetCStringPtr() is simply an optimization.

In addition, the following functions, which create immutable CFStrings from developer
supplied buffers without copying the buffers, might have to actually copy
under certain circumstances (If they do copy, the buffer will be dealt with by the
"contentsDeallocator" argument.):

  CFStringCreateWithPascalStringNoCopy()
  CFStringCreateWithCStringNoCopy()
  CFStringCreateWithCharactersNoCopy()

You should of course never depend on the backing store of these CFStrings being
what you provided, and in other no circumstance should you change the contents
of that buffer (given that would break the invariant about the CFString being immutable).

Having said all this, there are actually ways to create a CFString where the backing store
is external, and can be manipulated by the developer or CFString itself:

  CFStringCreateMutableWithExternalCharactersNoCopy()
  CFStringSetExternalCharactersNoCopy()

A "contentsAllocator" is used to realloc or free the backing store by CFString.
kCFAllocatorNull can be provided to assure CFString will never realloc or free the buffer.
Developer can call CFStringSetExternalCharactersNoCopy() to update
CFString's idea of what's going on, if the buffer is changed externally. In these
strings, CFStringGetCharactersPtr() is guaranteed to return the external buffer.

These functions are here to allow wrapping a buffer of UniChar characters in a CFString,
allowing the buffer to passed into CFString functions and also manipulated via CFString
mutation functions. In general, developers should not use this technique for all strings,
as it prevents CFString from using certain optimizations.
*/

/* Identifier for character encoding; the values are the same as Text Encoding Converter TextEncoding.
*/
typedef UInt32 CFStringEncoding;

/* Platform-independent built-in encodings; always available on all platforms.
   Call CFStringGetSystemEncoding() to get the default system encoding.
*/
#define kCFStringEncodingInvalidId (0xffffffffU)
typedef CF_ENUM(CFStringEncoding, CFStringBuiltInEncodings) {
    kCFStringEncodingMacRoman = 0,
    kCFStringEncodingWindowsLatin1 = 0x0500, /* ANSI codepage 1252 */
    kCFStringEncodingISOLatin1 = 0x0201, /* ISO 8859-1 */
    kCFStringEncodingNextStepLatin = 0x0B01, /* NextStep encoding*/
    kCFStringEncodingASCII = 0x0600, /* 0..127 (in creating CFString, values greater than 0x7F are treated as corresponding Unicode value) */
    kCFStringEncodingUnicode = 0x0100, /* kTextEncodingUnicodeDefault  + kTextEncodingDefaultFormat (aka kUnicode16BitFormat) */
    kCFStringEncodingUTF8 = 0x08000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF8Format */
    kCFStringEncodingNonLossyASCII = 0x0BFF, /* 7bit Unicode variants used by Cocoa & Java */

    kCFStringEncodingUTF16 = 0x0100, /* kTextEncodingUnicodeDefault + kUnicodeUTF16Format (alias of kCFStringEncodingUnicode) */
    kCFStringEncodingUTF16BE = 0x10000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF16BEFormat */
    kCFStringEncodingUTF16LE = 0x14000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF16LEFormat */

    kCFStringEncodingUTF32 = 0x0c000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF32Format */
    kCFStringEncodingUTF32BE = 0x18000100, /* kTextEncodingUnicodeDefault + kUnicodeUTF32BEFormat */
    kCFStringEncodingUTF32LE = 0x1c000100 /* kTextEncodingUnicodeDefault + kUnicodeUTF32LEFormat */
};


/* CFString type ID */
CF_EXPORT
CFTypeID CFStringGetTypeID(void);

/* CFSTR() allows creation of compile-time constant CFStringRefs; the argument 
should be a constant C-string.

CFSTR(), not being a "Copy" or "Create" function, does not return a new
reference for you. So, you should not release the return value. This is
much like constant C or Pascal strings --- when you use "hello world"
in a program, you do not free it.

However, strings returned from CFSTR() can be retained and released in a
properly nested fashion, just like any other CF type. That is, if you pass
a CFSTR() return value to a function such as SetMenuItemWithCFString(), the
function can retain it, then later, when it's done with it, it can release it.

Non-7 bit characters (that is, above 127) in CFSTR() are supported, although care must
be taken in dealing with files containing them. If you can trust your editor and tools
to deal with non-ASCII characters in the source code, then you can use them directly 
in CFSTR(); otherwise, you can represent such characters with their escaped octal 
equivalents in the encoding the compiler will use to interpret them (for instance, 
O-umlaut is \303\226 in UTF-8). UTF-8 is the recommended encoding here, 
since it is the default choice with Mac OS X developer tools.
*/
#if TARGET_OS_WIN32 || TARGET_OS_LINUX
#undef __CONSTANT_CFSTRINGS__
#endif

#ifdef __CONSTANT_CFSTRINGS__
#define CFSTR(cStr)  ((CFStringRef) __builtin___CFStringMakeConstantString ("" cStr ""))
#else
#define CFSTR(cStr)  __CFStringMakeConstantString("" cStr "")
#endif

#if defined(__GNUC__) && (__GNUC__*10+__GNUC_MINOR__ >= 42) && defined(__APPLE_CC__) && (__APPLE_CC__ > 1) && !defined(__INTEL_COMPILER) && (TARGET_OS_MAC || TARGET_OS_EMBEDDED)
#define CF_FORMAT_FUNCTION(F,A) __attribute__((format(CFString, F, A)))
#define CF_FORMAT_ARGUMENT(A) __attribute__((format_arg(A)))
#else
#define CF_FORMAT_FUNCTION(F,A)
#define CF_FORMAT_ARGUMENT(A)
#endif

/*** Immutable string creation functions ***/

/* Functions to create basic immutable strings. The provided allocator is used for all memory activity in these functions.
*/

/* The following four functions copy the provided buffer into CFString's internal storage. */
CF_EXPORT
CFStringRef CFStringCreateWithPascalString(CFAllocatorRef alloc, ConstStr255Param pStr, CFStringEncoding encoding);

CF_EXPORT
CFStringRef CFStringCreateWithCString(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding);

/* The following takes an explicit length, and allows you to specify whether the data is an external format --- that is, whether to pay attention to the BOM character (if any) and do byte swapping if necessary
*/
CF_EXPORT
CFStringRef CFStringCreateWithBytes(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalRepresentation);

CF_EXPORT
CFStringRef CFStringCreateWithCharacters(CFAllocatorRef alloc, const UniChar *chars, CFIndex numChars);


/* These functions try not to copy the provided buffer. The buffer will be deallocated 
with the provided contentsDeallocator when it's no longer needed; to not free
the buffer, specify kCFAllocatorNull here. As usual, NULL means default allocator.

NOTE: Do not count on these buffers as being used by the string; 
in some cases the CFString might free the buffer and use something else
(for instance if it decides to always use Unicode encoding internally). 

NOTE: If you are not transferring ownership of the buffer to the CFString
(for instance, you supplied contentsDeallocator = kCFAllocatorNull), it is your
responsibility to assure the buffer does not go away during the lifetime of the string.
If the string is retained or copied, its lifetime might extend in ways you cannot
predict. So, for strings created with buffers whose lifetimes you cannot
guarantee, you need to be extremely careful --- do not hand it out to any
APIs which might retain or copy the strings.
*/
CF_EXPORT
CFStringRef CFStringCreateWithPascalStringNoCopy(CFAllocatorRef alloc, ConstStr255Param pStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator);

CF_EXPORT
CFStringRef CFStringCreateWithCStringNoCopy(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator);

/* The following takes an explicit length, and allows you to specify whether the data is an external format --- that is, whether to pay attention to the BOM character (if any) and do byte swapping if necessary
*/
CF_EXPORT
CFStringRef CFStringCreateWithBytesNoCopy(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalRepresentation, CFAllocatorRef contentsDeallocator);

CF_EXPORT
CFStringRef CFStringCreateWithCharactersNoCopy(CFAllocatorRef alloc, const UniChar *chars, CFIndex numChars, CFAllocatorRef contentsDeallocator);

/* Create copies of part or all of the string.
*/
CF_EXPORT
CFStringRef CFStringCreateWithSubstring(CFAllocatorRef alloc, CFStringRef str, CFRange range);

CF_EXPORT
CFStringRef CFStringCreateCopy(CFAllocatorRef alloc, CFStringRef theString);

/* These functions create a CFString from the provided printf-like format string and arguments.
*/
CF_EXPORT
CFStringRef CFStringCreateWithFormat(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef format, ...) CF_FORMAT_FUNCTION(3,4);

CF_EXPORT
CFStringRef CFStringCreateWithFormatAndArguments(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef format, va_list arguments) CF_FORMAT_FUNCTION(3,0);

/* Functions to create mutable strings. "maxLength", if not 0, is a hard bound on the length of the string. If 0, there is no limit on the length.
*/
CF_EXPORT
CFMutableStringRef CFStringCreateMutable(CFAllocatorRef alloc, CFIndex maxLength);

CF_EXPORT
CFMutableStringRef CFStringCreateMutableCopy(CFAllocatorRef alloc, CFIndex maxLength, CFStringRef theString);

/* This function creates a mutable string that has a developer supplied and directly editable backing store.
The string will be manipulated within the provided buffer (if any) until it outgrows capacity; then the
externalCharactersAllocator will be consulted for more memory. When the CFString is deallocated, the
buffer will be freed with the externalCharactersAllocator. Provide kCFAllocatorNull here to prevent the buffer
from ever being reallocated or deallocated by CFString. See comments at top of this file for more info.
*/
CF_EXPORT
CFMutableStringRef CFStringCreateMutableWithExternalCharactersNoCopy(CFAllocatorRef alloc, UniChar *chars, CFIndex numChars, CFIndex capacity, CFAllocatorRef externalCharactersAllocator);

/*** Basic accessors for the contents ***/

/* Number of 16-bit Unicode characters in the string.
*/
CF_EXPORT
CFIndex CFStringGetLength(CFStringRef theString);

/* Extracting the contents of the string. For obtaining multiple characters, calling
CFStringGetCharacters() is more efficient than multiple calls to CFStringGetCharacterAtIndex().
If the length of the string is not known (so you can't use a fixed size buffer for CFStringGetCharacters()),
another method is to use is CFStringGetCharacterFromInlineBuffer() (see further below).
*/
CF_EXPORT
UniChar CFStringGetCharacterAtIndex(CFStringRef theString, CFIndex idx);

CF_EXPORT
void CFStringGetCharacters(CFStringRef theString, CFRange range, UniChar *buffer);


/*** Conversion to other encodings ***/

/* These two convert into the provided buffer; they return false if conversion isn't possible
(due to conversion error, or not enough space in the provided buffer). 
These functions do zero-terminate or put the length byte; the provided bufferSize should include
space for this (so pass 256 for Str255). More sophisticated usages can go through CFStringGetBytes().
These functions are equivalent to calling CFStringGetBytes() with 
the range of the string; lossByte = 0; and isExternalRepresentation = false; 
if successful, they then insert the leading length or terminating zero, as desired.
*/
CF_EXPORT
Boolean CFStringGetPascalString(CFStringRef theString, StringPtr buffer, CFIndex bufferSize, CFStringEncoding encoding);

CF_EXPORT
Boolean CFStringGetCString(CFStringRef theString, char *buffer, CFIndex bufferSize, CFStringEncoding encoding);

/* These functions attempt to return in O(1) time the desired format for the string.
Note that although this means a pointer to the internal structure is being returned,
this can't always be counted on. Please see note at the top of the file for more
details.
*/
CF_EXPORT
ConstStringPtr CFStringGetPascalStringPtr(CFStringRef theString, CFStringEncoding encoding);	/* May return NULL at any time; be prepared for NULL */

CF_EXPORT
const char *CFStringGetCStringPtr(CFStringRef theString, CFStringEncoding encoding);		/* May return NULL at any time; be prepared for NULL */

CF_EXPORT
const UniChar *CFStringGetCharactersPtr(CFStringRef theString);					/* May return NULL at any time; be prepared for NULL */

/* The primitive conversion routine; allows you to convert a string piece at a time
       into a fixed size buffer. Returns number of characters converted. 
   Characters that cannot be converted to the specified encoding are represented
       with the byte specified by lossByte; if lossByte is 0, then lossy conversion
       is not allowed and conversion stops, returning partial results.
   Pass buffer==NULL if you don't care about the converted string (but just the convertability,
       or number of bytes required). 
   maxBufLength indicates the maximum number of bytes to generate. It is ignored when buffer==NULL.
   Does not zero-terminate. If you want to create Pascal or C string, allow one extra byte at start or end. 
   Setting isExternalRepresentation causes any extra bytes that would allow 
       the data to be made persistent to be included; for instance, the Unicode BOM. Note that
       CFString prepends UTF encoded data with the Unicode BOM <http://www.unicode.org/faq/utf_bom.html> 
       when generating external representation if the target encoding allows. It's important to note that
       only UTF-8, UTF-16, and UTF-32 define the handling of the byte order mark character, and the "LE"
       and "BE" variants of UTF-16 and UTF-32 don't.
*/
CF_EXPORT
CFIndex CFStringGetBytes(CFStringRef theString, CFRange range, CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation, UInt8 *buffer, CFIndex maxBufLen, CFIndex *usedBufLen);

/* Convenience functions String <-> Data. These generate "external" formats, that is, formats that
   can be written out to disk. For instance, if the encoding is Unicode,
   CFStringCreateFromExternalRepresentation() pays attention to the BOM character (if any) 
   and does byte swapping if necessary. Similarly CFStringCreateExternalRepresentation() will  
   include a BOM character if appropriate. See CFStringGetBytes() for more on this and lossByte.
*/
CF_EXPORT
CFStringRef CFStringCreateFromExternalRepresentation(CFAllocatorRef alloc, CFDataRef data, CFStringEncoding encoding);	/* May return NULL on conversion error */

CF_EXPORT
CFDataRef CFStringCreateExternalRepresentation(CFAllocatorRef alloc, CFStringRef theString, CFStringEncoding encoding, UInt8 lossByte);	/* May return NULL on conversion error */	

/* Hints about the contents of a string
*/
CF_EXPORT
CFStringEncoding CFStringGetSmallestEncoding(CFStringRef theString);	/* Result in O(n) time max */

CF_EXPORT
CFStringEncoding CFStringGetFastestEncoding(CFStringRef theString);	/* Result in O(1) time max */

/* General encoding info
*/
CF_EXPORT
CFStringEncoding CFStringGetSystemEncoding(void);		/* The default encoding for the system; untagged 8-bit characters are usually in this encoding */

CF_EXPORT
CFIndex CFStringGetMaximumSizeForEncoding(CFIndex length, CFStringEncoding encoding);	/* Max bytes a string of specified length (in UniChars) will take up if encoded */


/*** FileSystem path conversion functions ***/

/* Extract the contents of the string as a NULL-terminated 8-bit string appropriate for passing to POSIX APIs (for example, normalized for HFS+).  The string is zero-terminated. false will be returned if the conversion results don't fit into the buffer.  Use CFStringGetMaximumSizeOfFileSystemRepresentation() if you want to make sure the buffer is of sufficient length.
*/
CF_EXPORT
Boolean CFStringGetFileSystemRepresentation(CFStringRef string, char *buffer, CFIndex maxBufLen);

/* Get the upper bound on the number of bytes required to hold the file system representation for the string. This result is returned quickly as a very rough approximation, and could be much larger than the actual space required. The result includes space for the zero termination. If you are allocating a buffer for long-term keeping, it's recommended that you reallocate it smaller (to be the right size) after calling CFStringGetFileSystemRepresentation(). 
*/
CF_EXPORT
CFIndex CFStringGetMaximumSizeOfFileSystemRepresentation(CFStringRef string);

/* Create a CFString from the specified zero-terminated POSIX file system representation.  If the conversion fails (possible due to bytes in the buffer not being a valid sequence of bytes for the appropriate character encoding), NULL is returned.
*/
CF_EXPORT
CFStringRef CFStringCreateWithFileSystemRepresentation(CFAllocatorRef alloc, const char *buffer);


/*** Comparison functions. ***/

/* Find and compare flags; these are OR'ed together and provided as CFStringCompareFlags in the various functions. 
*/
typedef CF_OPTIONS(CFOptionFlags, CFStringCompareFlags) {
    kCFCompareCaseInsensitive = 1,	
    kCFCompareBackwards = 4,		/* Starting from the end of the string */
    kCFCompareAnchored = 8,		/* Only at the specified starting point */
    kCFCompareNonliteral = 16,		/* If specified, loose equivalence is performed (o-umlaut == o, umlaut) */
    kCFCompareLocalized = 32,		/* User's default locale is used for the comparisons */
    kCFCompareNumerically = 64,		/* Numeric comparison is used; that is, Foo2.txt < Foo7.txt < Foo25.txt */
    kCFCompareDiacriticInsensitive CF_ENUM_AVAILABLE(10_5, 2_0) = 128, /* If specified, ignores diacritics (o-umlaut == o) */
    kCFCompareWidthInsensitive CF_ENUM_AVAILABLE(10_5, 2_0) = 256, /* If specified, ignores width differences ('a' == UFF41) */
    kCFCompareForcedOrdering CF_ENUM_AVAILABLE(10_5, 2_0) = 512 /* If specified, comparisons are forced to return either kCFCompareLessThan or kCFCompareGreaterThan if the strings are equivalent but not strictly equal, for stability when sorting (e.g. "aaa" > "AAA" with kCFCompareCaseInsensitive specified) */
};

/* The main comparison routine; compares specified range of the first string to (the full range of) the second string.
   locale == NULL indicates canonical locale (the return value from CFLocaleGetSystem()).
   kCFCompareNumerically, added in 10.2, does not work if kCFCompareLocalized is specified on systems before 10.3
   kCFCompareBackwards and kCFCompareAnchored are not applicable.
   rangeToCompare applies to the first string; that is, only the substring of theString1 specified by rangeToCompare is compared against all of theString2.
*/
CF_EXPORT
CFComparisonResult CFStringCompareWithOptionsAndLocale(CFStringRef theString1, CFStringRef theString2, CFRange rangeToCompare, CFStringCompareFlags compareOptions, CFLocaleRef locale) CF_AVAILABLE(10_5, 2_0);

/* Comparison convenience. Uses the current user locale (the return value from CFLocaleCopyCurrent()) if kCFCompareLocalized. Refer to CFStringCompareWithOptionsAndLocale() for more info.
*/
CF_EXPORT
CFComparisonResult CFStringCompareWithOptions(CFStringRef theString1, CFStringRef theString2, CFRange rangeToCompare, CFStringCompareFlags compareOptions);

/* Comparison convenience suitable for passing as sorting functions.
   kCFCompareNumerically, added in 10.2, does not work if kCFCompareLocalized is specified on systems before 10.3
   kCFCompareBackwards and kCFCompareAnchored are not applicable.
*/
CF_EXPORT
CFComparisonResult CFStringCompare(CFStringRef theString1, CFStringRef theString2, CFStringCompareFlags compareOptions);

/* CFStringFindWithOptionsAndLocale() returns the found range in the CFRange * argument; you can pass NULL for simple discovery check.
   locale == NULL indicates canonical locale (the return value from CFLocaleGetSystem()).
   If stringToFind is the empty string (zero length), nothing is found.
   Ignores the kCFCompareNumerically option.
   Only the substring of theString specified by rangeToSearch is searched for stringToFind.
*/
CF_EXPORT
Boolean CFStringFindWithOptionsAndLocale(CFStringRef theString, CFStringRef stringToFind, CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFLocaleRef locale, CFRange *result) CF_AVAILABLE(10_5, 2_0);

/* Find convenience. Uses the current user locale (the return value from CFLocaleCopyCurrent()) if kCFCompareLocalized. Refer to CFStringFindWithOptionsAndLocale() for more info.
*/
CF_EXPORT
Boolean CFStringFindWithOptions(CFStringRef theString, CFStringRef stringToFind, CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFRange *result);

/* CFStringCreateArrayWithFindResults() returns an array of CFRange pointers, or NULL if there are no matches.
   Overlapping instances are not found; so looking for "AA" in "AAA" finds just one range.
   Post 10.1: If kCFCompareBackwards is provided, the scan is done from the end (which can give a different result), and
      the results are stored in the array backwards (last found range in slot 0).
   If stringToFind is the empty string (zero length), nothing is found.
   kCFCompareAnchored causes just the consecutive instances at start (or end, if kCFCompareBackwards) to be reported. So, searching for "AB" in "ABABXAB..." you just get the first two occurrences.
   Ignores the kCFCompareNumerically option.
   Only the substring of theString specified by rangeToSearch is searched for stringToFind.
*/
CF_EXPORT
CFArrayRef CFStringCreateArrayWithFindResults(CFAllocatorRef alloc, CFStringRef theString, CFStringRef stringToFind, CFRange rangeToSearch, CFStringCompareFlags compareOptions);

/* Find conveniences; see comments above concerning empty string and options.
*/
CF_EXPORT
CFRange CFStringFind(CFStringRef theString, CFStringRef stringToFind, CFStringCompareFlags compareOptions);

CF_EXPORT
Boolean CFStringHasPrefix(CFStringRef theString, CFStringRef prefix);

CF_EXPORT
Boolean CFStringHasSuffix(CFStringRef theString, CFStringRef suffix);

/*!
	@function CFStringGetRangeOfComposedCharactersAtIndex
	Returns the range of the composed character sequence at the specified index.
	@param theString The CFString which is to be searched.  If this
                		parameter is not a valid CFString, the behavior is
              		undefined.
	@param theIndex The index of the character contained in the
			composed character sequence.  If the index is
			outside the index space of the string (0 to N-1 inclusive,
			where N is the length of the string), the behavior is
			undefined.
	@result The range of the composed character sequence.
*/
CF_EXPORT CFRange CFStringGetRangeOfComposedCharactersAtIndex(CFStringRef theString, CFIndex theIndex);

/*!
	@function CFStringFindCharacterFromSet
	Query the range of the first character contained in the specified character set.
	@param theString The CFString which is to be searched.  If this
                		parameter is not a valid CFString, the behavior is
              		undefined.
	@param theSet The CFCharacterSet against which the membership
			of characters is checked.  If this parameter is not a valid
			CFCharacterSet, the behavior is undefined.
	@param range The range of characters within the string to search. If
			the range location or end point (defined by the location
			plus length minus 1) are outside the index space of the
			string (0 to N-1 inclusive, where N is the length of the
			string), the behavior is undefined. If the range length is
			negative, the behavior is undefined. The range may be empty
			(length 0), in which case no search is performed.
	@param searchOptions The bitwise-or'ed option flags to control
			the search behavior.  The supported options are
			kCFCompareBackwards andkCFCompareAnchored.
			If other option flags are specified, the behavior
                        is undefined.
	@param result The pointer to a CFRange supplied by the caller in
			which the search result is stored.  Note that the length
			of this range can be more than 1, if for instance the 
			result is a composed character. If a pointer to an invalid
			memory is specified, the behavior is undefined.
	@result true, if at least a character which is a member of the character
			set is found and result is filled, otherwise, false.
*/
CF_EXPORT Boolean CFStringFindCharacterFromSet(CFStringRef theString, CFCharacterSetRef theSet, CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFRange *result);

/* Find range of bounds of the line(s) that span the indicated range (startIndex, numChars),
   taking into account various possible line separator sequences (CR, CRLF, LF, and Unicode NextLine, LineSeparator, ParagraphSeparator).
   All return values are "optional" (provide NULL if you don't want them)
     lineBeginIndex: index of first character in line
     lineEndIndex: index of first character of the next line (including terminating line separator characters)
     contentsEndIndex: index of the first line separator character
   Thus, lineEndIndex - lineBeginIndex is the number of chars in the line, including the line separators
         contentsEndIndex - lineBeginIndex is the number of chars in the line w/out the line separators
*/
CF_EXPORT
void CFStringGetLineBounds(CFStringRef theString, CFRange range, CFIndex *lineBeginIndex, CFIndex *lineEndIndex, CFIndex *contentsEndIndex); 

/* Same as CFStringGetLineBounds(), however, will only look for paragraphs. Won't stop at Unicode NextLine or LineSeparator characters.
*/
CF_EXPORT
void CFStringGetParagraphBounds(CFStringRef string, CFRange range, CFIndex *parBeginIndex, CFIndex *parEndIndex, CFIndex *contentsEndIndex) CF_AVAILABLE(10_5, 2_0);

/*!
	@function CFStringGetHyphenationLocationBeforeIndex
	Retrieve the first potential hyphenation location found before the specified location.
	@param string The CFString which is to be hyphenated.  If this
                		parameter is not a valid CFString, the behavior is
              		undefined.
	@param location An index in the string.  If a valid hyphen index is returned, it 
	                will be before this index.
	@param limitRange The range of characters within the string to search. If
			the range location or end point (defined by the location
			plus length minus 1) are outside the index space of the
			string (0 to N-1 inclusive, where N is the length of the
			string), the behavior is undefined. If the range length is
			negative, the behavior is undefined. The range may be empty
			(length 0), in which case no hyphen location is generated.
	@param options Reserved for future use.
	@param locale Specifies which language's hyphenation conventions to use.
			This must be a valid locale.  Hyphenation data is not available
			for all locales.  You can use CFStringIsHyphenationAvailableForLocale
			to test for availability of hyphenation data.
	@param character The suggested hyphen character to insert.  Pass NULL if you
			do not need this information.
	@result an index in the string where it is appropriate to insert a hyphen, if
			one exists; else kCFNotFound
*/
CF_EXPORT
CFIndex CFStringGetHyphenationLocationBeforeIndex(CFStringRef string, CFIndex location, CFRange limitRange, CFOptionFlags options, CFLocaleRef locale, UTF32Char *character) CF_AVAILABLE(10_7, 4_2);

CF_EXPORT
Boolean CFStringIsHyphenationAvailableForLocale(CFLocaleRef locale) CF_AVAILABLE(10_7, 4_3);

/*** Exploding and joining strings with a separator string ***/

CF_EXPORT
CFStringRef CFStringCreateByCombiningStrings(CFAllocatorRef alloc, CFArrayRef theArray, CFStringRef separatorString);	/* Empty array returns empty string; one element array returns the element */

CF_EXPORT
CFArrayRef CFStringCreateArrayBySeparatingStrings(CFAllocatorRef alloc, CFStringRef theString, CFStringRef separatorString);	/* No separators in the string returns array with that string; string == sep returns two empty strings */


/*** Parsing non-localized numbers from strings ***/

CF_EXPORT
SInt32 CFStringGetIntValue(CFStringRef str);		/* Skips whitespace; returns 0 on error, MAX or -MAX on overflow */

CF_EXPORT
double CFStringGetDoubleValue(CFStringRef str);	/* Skips whitespace; returns 0.0 on error */


/*** MutableString functions ***/

/* CFStringAppend("abcdef", "xxxxx") -> "abcdefxxxxx"
   CFStringDelete("abcdef", CFRangeMake(2, 3)) -> "abf"
   CFStringReplace("abcdef", CFRangeMake(2, 3), "xxxxx") -> "abxxxxxf"
   CFStringReplaceAll("abcdef", "xxxxx") -> "xxxxx"
*/
CF_EXPORT
void CFStringAppend(CFMutableStringRef theString, CFStringRef appendedString);

CF_EXPORT
void CFStringAppendCharacters(CFMutableStringRef theString, const UniChar *chars, CFIndex numChars);

CF_EXPORT
void CFStringAppendPascalString(CFMutableStringRef theString, ConstStr255Param pStr, CFStringEncoding encoding);

CF_EXPORT
void CFStringAppendCString(CFMutableStringRef theString, const char *cStr, CFStringEncoding encoding);

CF_EXPORT
void CFStringAppendFormat(CFMutableStringRef theString, CFDictionaryRef formatOptions, CFStringRef format, ...) CF_FORMAT_FUNCTION(3,4);

CF_EXPORT
void CFStringAppendFormatAndArguments(CFMutableStringRef theString, CFDictionaryRef formatOptions, CFStringRef format, va_list arguments) CF_FORMAT_FUNCTION(3,0);

CF_EXPORT
void CFStringInsert(CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr);

CF_EXPORT
void CFStringDelete(CFMutableStringRef theString, CFRange range);

CF_EXPORT
void CFStringReplace(CFMutableStringRef theString, CFRange range, CFStringRef replacement);

CF_EXPORT
void CFStringReplaceAll(CFMutableStringRef theString, CFStringRef replacement);	/* Replaces whole string */

/* Replace all occurrences of target in rangeToSearch of theString with replacement.
   Pays attention to kCFCompareCaseInsensitive, kCFCompareBackwards, kCFCompareNonliteral, and kCFCompareAnchored.
   kCFCompareBackwards can be used to do the replacement starting from the end, which could give a different result.
     ex. AAAAA, replace AA with B -> BBA or ABB; latter if kCFCompareBackwards
   kCFCompareAnchored assures only anchored but multiple instances are found (the instances must be consecutive at start or end)
     ex. AAXAA, replace A with B -> BBXBB or BBXAA; latter if kCFCompareAnchored
   Returns number of replacements performed.
*/
CF_EXPORT
CFIndex CFStringFindAndReplace(CFMutableStringRef theString, CFStringRef stringToFind, CFStringRef replacementString, CFRange rangeToSearch, CFStringCompareFlags compareOptions);


/* This function will make the contents of a mutable CFString point directly at the specified UniChar array.
   It works only with CFStrings created with CFStringCreateMutableWithExternalCharactersNoCopy().
   This function does not free the previous buffer.
   The string will be manipulated within the provided buffer (if any) until it outgrows capacity; then the
     externalCharactersAllocator will be consulted for more memory.
   See comments at the top of this file for more info.
*/
CF_EXPORT
void CFStringSetExternalCharactersNoCopy(CFMutableStringRef theString, UniChar *chars, CFIndex length, CFIndex capacity);	/* Works only on specially created mutable strings! */

/* CFStringPad() will pad or cut down a string to the specified size.
   The pad string is used as the fill string; indexIntoPad specifies which character to start with.
     CFStringPad("abc", " ", 9, 0) ->  "abc      "
     CFStringPad("abc", ". ", 9, 1) -> "abc . . ."
     CFStringPad("abcdef", ?, 3, ?) -> "abc"

     CFStringTrim() will trim the specified string from both ends of the string.
     CFStringTrimWhitespace() will do the same with white space characters (tab, newline, etc)
     CFStringTrim("  abc ", " ") -> "abc"
     CFStringTrim("* * * *abc * ", "* ") -> "*abc "
*/
CF_EXPORT
void CFStringPad(CFMutableStringRef theString, CFStringRef padString, CFIndex length, CFIndex indexIntoPad);

CF_EXPORT
void CFStringTrim(CFMutableStringRef theString, CFStringRef trimString);

CF_EXPORT
void CFStringTrimWhitespace(CFMutableStringRef theString);

CF_EXPORT
void CFStringLowercase(CFMutableStringRef theString, CFLocaleRef locale);

CF_EXPORT
void CFStringUppercase(CFMutableStringRef theString, CFLocaleRef locale);

CF_EXPORT
void CFStringCapitalize(CFMutableStringRef theString, CFLocaleRef locale);

/*!
	@typedef CFStringNormalizationForm
	This is the type of Unicode normalization forms as described in
	Unicode Technical Report #15. To normalize for use with file
	system calls, use CFStringGetFileSystemRepresentation().
*/
typedef CF_ENUM(CFIndex, CFStringNormalizationForm) {
	kCFStringNormalizationFormD = 0, // Canonical Decomposition
	kCFStringNormalizationFormKD, // Compatibility Decomposition
	kCFStringNormalizationFormC, // Canonical Decomposition followed by Canonical Composition
	kCFStringNormalizationFormKC // Compatibility Decomposition followed by Canonical Composition
};

/*!
	@function CFStringNormalize
	Normalizes the string into the specified form as described in
	Unicode Technical Report #15.
	@param theString  The string which is to be normalized.  If this
		parameter is not a valid mutable CFString, the behavior is
		undefined.
	@param theForm  The form into which the string is to be normalized.
		If this parameter is not a valid CFStringNormalizationForm value,
		the behavior is undefined.
*/
CF_EXPORT void CFStringNormalize(CFMutableStringRef theString, CFStringNormalizationForm theForm);


/*!
	@function CFStringFold
	Folds the string into the form specified by the flags.
		Character foldings are operations that convert any of a set of characters
		sharing similar semantics into a single representative from that set.
		This function can be used to preprocess strings that are to be compared,
		searched, or indexed.
		Note that folding does not include normalization, so it is necessary
		to use CFStringNormalize in addition to CFStringFold in order to obtain
		the effect of kCFCompareNonliteral.
	@param theString  The string which is to be folded.  If this parameter is not
		a valid mutable CFString, the behavior is undefined.
	@param theFlag  The equivalency flags which describes the character folding form.
		Only those flags containing the word "insensitive" are recognized here; other flags are ignored.		
		Folding with kCFCompareCaseInsensitive removes case distinctions in accordance with the mapping
		specified by ftp://ftp.unicode.org/Public/UNIDATA/CaseFolding.txt.  Folding with
		kCFCompareDiacriticInsensitive removes distinctions of accents and other diacritics.  Folding
		with kCFCompareWidthInsensitive removes character width distinctions by mapping characters in
		the range U+FF00-U+FFEF to their ordinary equivalents.
	@param theLocale The locale tailoring the character folding behavior. If NULL,
		it's considered to be the system locale returned from CFLocaleGetSystem().
		If non-NULL and not a valid CFLocale object, the behavior is undefined.
*/

CF_EXPORT
void CFStringFold(CFMutableStringRef theString, CFStringCompareFlags theFlags, CFLocaleRef theLocale) CF_AVAILABLE(10_5, 2_0);

/* Perform string transliteration.  The transformation represented by transform is applied to the given range of string, modifying it in place. Only the specified range will be modified, but the transform may look at portions of the string outside that range for context. NULL range pointer causes the whole string to be transformed. On return, range is modified to reflect the new range corresponding to the original range. reverse indicates that the inverse transform should be used instead, if it exists. If the transform is successful, true is returned; if unsuccessful, false. Reasons for the transform being unsuccessful include an invalid transform identifier, or attempting to reverse an irreversible transform.

You can pass one of the predefined transforms below, or any valid ICU transform ID as defined in the ICU User Guide. Note that we do not support arbitrary set of ICU transform rules.
*/
CF_EXPORT
Boolean CFStringTransform(CFMutableStringRef string, CFRange *range, CFStringRef transform, Boolean reverse);

/* Transform identifiers for CFStringTransform()
*/
CF_EXPORT const CFStringRef kCFStringTransformStripCombiningMarks;
CF_EXPORT const CFStringRef kCFStringTransformToLatin;
CF_EXPORT const CFStringRef kCFStringTransformFullwidthHalfwidth;
CF_EXPORT const CFStringRef kCFStringTransformLatinKatakana;
CF_EXPORT const CFStringRef kCFStringTransformLatinHiragana;
CF_EXPORT const CFStringRef kCFStringTransformHiraganaKatakana;
CF_EXPORT const CFStringRef kCFStringTransformMandarinLatin;
CF_EXPORT const CFStringRef kCFStringTransformLatinHangul;
CF_EXPORT const CFStringRef kCFStringTransformLatinArabic;
CF_EXPORT const CFStringRef kCFStringTransformLatinHebrew;
CF_EXPORT const CFStringRef kCFStringTransformLatinThai;
CF_EXPORT const CFStringRef kCFStringTransformLatinCyrillic;
CF_EXPORT const CFStringRef kCFStringTransformLatinGreek;
CF_EXPORT const CFStringRef kCFStringTransformToXMLHex;
CF_EXPORT const CFStringRef kCFStringTransformToUnicodeName;
CF_EXPORT const CFStringRef kCFStringTransformStripDiacritics CF_AVAILABLE(10_5, 2_0);


/*** General encoding related functionality ***/

/* This returns availability of the encoding on the system
*/
CF_EXPORT
Boolean CFStringIsEncodingAvailable(CFStringEncoding encoding);

/* This function returns list of available encodings.  The returned list is terminated with kCFStringEncodingInvalidId and owned by the system.
*/
CF_EXPORT
const CFStringEncoding *CFStringGetListOfAvailableEncodings(void);

/* Returns name of the encoding; non-localized.
*/
CF_EXPORT
CFStringRef CFStringGetNameOfEncoding(CFStringEncoding encoding);

/* ID mapping functions from/to Cocoa NSStringEncoding.  Returns kCFStringEncodingInvalidId if no mapping exists.
*/
CF_EXPORT
unsigned long CFStringConvertEncodingToNSStringEncoding(CFStringEncoding encoding);

CF_EXPORT
CFStringEncoding CFStringConvertNSStringEncodingToEncoding(unsigned long encoding);

/* ID mapping functions from/to Microsoft Windows codepage (covers both OEM & ANSI).  Returns kCFStringEncodingInvalidId if no mapping exists.
*/
CF_EXPORT
UInt32 CFStringConvertEncodingToWindowsCodepage(CFStringEncoding encoding);

CF_EXPORT
CFStringEncoding CFStringConvertWindowsCodepageToEncoding(UInt32 codepage);

/* ID mapping functions from/to IANA registery charset names.  Returns kCFStringEncodingInvalidId if no mapping exists.
*/
CF_EXPORT
CFStringEncoding CFStringConvertIANACharSetNameToEncoding(CFStringRef theString);

CF_EXPORT
CFStringRef  CFStringConvertEncodingToIANACharSetName(CFStringEncoding encoding);

/* Returns the most compatible MacOS script value for the input encoding */
/* i.e. kCFStringEncodingMacRoman -> kCFStringEncodingMacRoman */
/*	kCFStringEncodingWindowsLatin1 -> kCFStringEncodingMacRoman */
/*	kCFStringEncodingISO_2022_JP -> kCFStringEncodingMacJapanese */
CF_EXPORT
CFStringEncoding CFStringGetMostCompatibleMacStringEncoding(CFStringEncoding encoding);



/* The next two functions allow fast access to the contents of a string, 
   assuming you are doing sequential or localized accesses. To use, call
   CFStringInitInlineBuffer() with a CFStringInlineBuffer (on the stack, say),
   and a range in the string to look at. Then call CFStringGetCharacterFromInlineBuffer()
   as many times as you want, with a index into that range (relative to the start
   of that range). These are INLINE functions and will end up calling CFString only 
   once in a while, to fill a buffer.  CFStringGetCharacterFromInlineBuffer() returns 0 if
   a location outside the original range is specified.
*/
#define __kCFStringInlineBufferLength 64
typedef struct {
    UniChar buffer[__kCFStringInlineBufferLength];
    CFStringRef theString;
    const UniChar *directUniCharBuffer;
    const char *directCStringBuffer;
    CFRange rangeToBuffer;		/* Range in string to buffer */
    CFIndex bufferedRangeStart;		/* Start of range currently buffered (relative to rangeToBuffer.location) */
    CFIndex bufferedRangeEnd;		/* bufferedRangeStart + number of chars actually buffered */
} CFStringInlineBuffer;

#if defined(CF_INLINE)
CF_INLINE void CFStringInitInlineBuffer(CFStringRef str, CFStringInlineBuffer *buf, CFRange range) {
    buf->theString = str;
    buf->rangeToBuffer = range;
    buf->directCStringBuffer = (buf->directUniCharBuffer = CFStringGetCharactersPtr(str)) ? NULL : CFStringGetCStringPtr(str, kCFStringEncodingASCII);
    buf->bufferedRangeStart = buf->bufferedRangeEnd = 0;
}

CF_INLINE UniChar CFStringGetCharacterFromInlineBuffer(CFStringInlineBuffer *buf, CFIndex idx) {
    if (idx < 0 || idx >= buf->rangeToBuffer.length) return 0;
    if (buf->directUniCharBuffer) return buf->directUniCharBuffer[idx + buf->rangeToBuffer.location];
    if (buf->directCStringBuffer) return (UniChar)(buf->directCStringBuffer[idx + buf->rangeToBuffer.location]);
    if (idx >= buf->bufferedRangeEnd || idx < buf->bufferedRangeStart) {
	if ((buf->bufferedRangeStart = idx - 4) < 0) buf->bufferedRangeStart = 0;
	buf->bufferedRangeEnd = buf->bufferedRangeStart + __kCFStringInlineBufferLength;
	if (buf->bufferedRangeEnd > buf->rangeToBuffer.length) buf->bufferedRangeEnd = buf->rangeToBuffer.length;
	CFStringGetCharacters(buf->theString, CFRangeMake(buf->rangeToBuffer.location + buf->bufferedRangeStart, buf->bufferedRangeEnd - buf->bufferedRangeStart), buf->buffer);
    }
    return buf->buffer[idx - buf->bufferedRangeStart];
}

#else
/* If INLINE functions are not available, we do somewhat less powerful macros that work similarly (except be aware that the buf argument is evaluated multiple times).
*/
#define CFStringInitInlineBuffer(str, buf, range) \
    do {(buf)->theString = str; (buf)->rangeToBuffer = range; (buf)->directCStringBuffer = ((buf)->directUniCharBuffer = CFStringGetCharactersPtr(str)) ? NULL : CFStringGetCStringPtr(str, kCFStringEncodingASCII);} while (0)

#define CFStringGetCharacterFromInlineBuffer(buf, idx) \
    (((idx) < 0 || (idx) >= (buf)->rangeToBuffer.length) ? 0 : ((buf)->directUniCharBuffer ? (buf)->directUniCharBuffer[(idx) + (buf)->rangeToBuffer.location] : ((buf)->directCStringBuffer ? (UniChar)((buf)->directCStringBuffer[(idx) + (buf)->rangeToBuffer.location]) : CFStringGetCharacterAtIndex((buf)->theString, (idx) + (buf)->rangeToBuffer.location))))

#endif /* CF_INLINE */



/* UTF-16 surrogate support
 */
CF_INLINE Boolean CFStringIsSurrogateHighCharacter(UniChar character) {
    return ((character >= 0xD800UL) && (character <= 0xDBFFUL) ? true : false);
}

CF_INLINE Boolean CFStringIsSurrogateLowCharacter(UniChar character) {
    return ((character >= 0xDC00UL) && (character <= 0xDFFFUL) ? true : false);
}

CF_INLINE UTF32Char CFStringGetLongCharacterForSurrogatePair(UniChar surrogateHigh, UniChar surrogateLow) {
    return (UTF32Char)(((surrogateHigh - 0xD800UL) << 10) + (surrogateLow - 0xDC00UL) + 0x0010000UL);
}

// Maps a UTF-32 character to a pair of UTF-16 surrogate characters. The buffer pointed by surrogates has to have space for at least 2 UTF-16 characters. Returns true if mapped to a surrogate pair.
CF_INLINE Boolean CFStringGetSurrogatePairForLongCharacter(UTF32Char character, UniChar *surrogates) {
    if ((character > 0xFFFFUL) && (character < 0x110000UL)) { // Non-BMP character
        character -= 0x10000;
        if (NULL != surrogates) {
            surrogates[0] = (UniChar)((character >> 10) + 0xD800UL);
            surrogates[1] = (UniChar)((character & 0x3FF) + 0xDC00UL);
        }
        return true;
    } else {
        if (NULL != surrogates) *surrogates = (UniChar)character;
        return false;
    }
}

/* Rest of the stuff in this file is private and should not be used directly
*/
/* For debugging only; output goes to stderr
   Use CFShow() to printf the description of any CFType;
   Use CFShowStr() to printf detailed info about a CFString
*/
CF_EXPORT
void CFShow(CFTypeRef obj);

CF_EXPORT
void CFShowStr(CFStringRef str);

/* This function is private and should not be used directly */
CF_EXPORT
CFStringRef  __CFStringMakeConstantString(const char *cStr) CF_FORMAT_ARGUMENT(1);	/* Private; do not use */

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFSTRING__ */
