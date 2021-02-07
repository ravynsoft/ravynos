/** Declaration of extension methods for base additions

   Copyright (C) 2003-2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   and:         Adam Fedor <fedor@gnu.org>

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

#ifndef	INCLUDED_NSData_GNUstepBase_h
#define	INCLUDED_NSData_GNUstepBase_h

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSData.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

@interface NSData (GNUstepBase)

/** Returns an autoreleased data instance initialised with pseudo-random
 * bytes of the specified length.<br />
 * On failure returns nil.  This may be due to:<br />
 * A zero length or unreasonably large length argument or,<br />
 * Failure to allocate memory to hold the random data or,<br />
 * Failure of the underlying random data generation.
 */
+ (id) dataWithRandomBytesOfLength: (NSUInteger)length;

/** Returns an NSString object containing a backslash escaped representation
 * of the receiver.
 */
- (NSString*) escapedRepresentation;

/**
 * Returns a buffer containing an ASCII backslash escaped representation
 * of the receiver, (and optionally the size of the buffer excluding
 * the trailing nul terminator).
 */
- (char*) escapedRepresentation: (NSUInteger*)length;

/** Returns data formed by gunzipping the contents of the receiver.<br />
 * If the receiver did not contain data produced by gzip, this method
 * simply returns the receiver.<br />
 * If the gnuzipping failed, this method returns nil.<br />
 * Otherwise, the returned object is an autorelease mutable data object.
 */
- (NSData*) gunzipped;

/** Returns data formed by gzipping the contents of the receiver using
 * the specified compression level (from 0 to 9 with 0 meaning no compression
 * and 9 meaning maximum compression).  Use a value outside the 0 to 9
 * range (eg -1) to employ the default compression.<br />
 * NB. It is permissable to gzip an empty data object, and it is also
 * permissable to gzip a data object which already contains gzipped data.<br />
 * Returns nil on failure.
 */
- (NSData*) gzipped: (int)level;

/** Returns YES if the receiver is a non-empty data object with a gzip
 * header, NO otherwise.
 */
- (BOOL) isGzipped;

/** Returns an NSString object containing an ASCII hexadecimal representation
 * of the receiver.  This means that the returned object will contain
 * exactly twice as many characters as there are bytes as the receiver,
 * as each byte in the receiver is represented by two hexadecimal digits.<br />
 * The high order four bits of each byte is encoded before the low
 * order four bits.  Capital letters 'A' to 'F' are used to represent
 * values from 10 to 15.
 */
- (NSString*) hexadecimalRepresentation;

/** Returns a buffer containing an ASCII string with a nul terminated
 * hexadecimal representation of the receiver, (and optionally the size
 * of the buffer excluding the trailing nul terminator).
 */
- (char*) hexadecimalRepresentation: (NSUInteger*)length;

/**
 * Initialises the receiver with the supplied string data which contains
 * a hexadecimal coding of the bytes.  The parsing of the string is
 * fairly tolerant, ignoring whitespace and permitting both upper and
 * lower case hexadecimal digits (the -hexadecimalRepresentation method
 * produces a string using only uppercase digits with no white space).<br />
 * If the string does not contain one or more pairs of hexadecimal digits
 * then an exception is raised. 
 */
- (id) initWithHexadecimalRepresentation: (NSString*)string
  NS_CONSUMES_SELF NS_RETURNS_RETAINED;

/**
 * Creates an MD5 digest of the information stored in the receiver and
 * returns it as an autoreleased 16 byte NSData object.<br />
 * If you need to produce a digest of string information, you need to
 * decide what character encoding is to be used and convert your string
 * to a data object of that encoding type first using the
 * [NSString-dataUsingEncoding:] method -
 * <example>
 *   myDigest = [[myString dataUsingEncoding: NSUTF8StringEncoding] md5Digest];
 * </example>
 * If you need to use the digest in a human readable form, you will
 * probably want it to be seen as 32 hexadecimal digits, and can do that
 * using the -hexadecimalRepresentation method.
 */
- (NSData*) md5Digest;

/**
 * Decodes the source data from uuencoded and return the result.<br />
 * Returns the encoded file name in namePtr if it is not null.
 * Returns the encoded file mode in modePtr if it is not null.
 */
- (BOOL) uudecodeInto: (NSMutableData*)decoded
		 name: (NSString**)namePtr
		 mode: (NSInteger*)modePtr;

/**
 * Encode the source data to uuencoded.<br />
 * Uses the supplied name as the filename in the encoded data,
 * and says that the file mode is as specified.<br />
 * If no name is supplied, uses <code>untitled</code> as the name.
 */
- (BOOL) uuencodeInto: (NSMutableData*)encoded
		 name: (NSString*)name
		 mode: (NSInteger)mode;
@end

#endif	/* OS_API_VERSION */

#if	defined(__cplusplus)
}
#endif

#endif	/* INCLUDED_NSData_GNUstepBase_h */

