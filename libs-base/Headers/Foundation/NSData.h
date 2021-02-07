/* Interface for NSData for GNUStep
   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: 1995

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
   */

#ifndef __NSData_h_GNUSTEP_BASE_INCLUDE
#define __NSData_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSRange.h>
#import	<Foundation/NSSerialization.h>
#import <GNUstepBase/GSBlocks.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
@class	NSError;
@class	NSURL;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST)
enum {
  NSDataSearchBackwards = (1UL << 0),
  NSDataSearchAnchored = (1UL << 1),
};
typedef NSUInteger NSDataSearchOptions;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)
enum {
  NSDataBase64DecodingIgnoreUnknownCharacters = (1UL << 0)
};
typedef NSUInteger NSDataBase64DecodingOptions;

enum {
  NSDataBase64Encoding64CharacterLineLength = (1UL << 0),
  NSDataBase64Encoding76CharacterLineLength = (1UL << 1),
  NSDataBase64EncodingEndLineWithCarriageReturn = (1UL << 4),
  NSDataBase64EncodingEndLineWithLineFeed = (1UL << 5),
};
typedef NSUInteger NSDataBase64EncodingOptions;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4,GS_API_LATEST)
enum {
  NSMappedRead = 1,
  NSUncachedRead = 2
};

enum {
  NSDataWritingAtomic = 1
};
/* The original name for this was NSAtomicWrite ... need for backward comapat
 */
#define NSAtomicWrite   NSDataWritingAtomic
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)
DEFINE_BLOCK_TYPE(GSDataDeallocatorBlock, void, void*, NSUInteger);
#endif

GS_EXPORT_CLASS
@interface NSData : NSObject <NSCoding, NSCopying, NSMutableCopying>

// Allocating and Initializing a Data Object

+ (id) data;
+ (id) dataWithBytes: (const void*)bytes
	      length: (NSUInteger)length;
+ (id) dataWithBytesNoCopy: (void*)bytes
		    length: (NSUInteger)length;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (id) dataWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree;
#endif
+ (id) dataWithContentsOfFile: (NSString*)path;
+ (id) dataWithContentsOfMappedFile: (NSString*)path;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (id) dataWithContentsOfURL: (NSURL*)url;
#endif
+ (id) dataWithData: (NSData*)data;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)
- (id) initWithBase64EncodedData: (NSData*)base64Data
                         options: (NSDataBase64DecodingOptions)options;
- (id) initWithBase64EncodedString: (NSString*)base64String
                           options: (NSDataBase64DecodingOptions)options;
/**
 * <override-subclass/>
 * Initialize the receiver to hold memory pointed to by bytes without copying.
 * When the receiver is deallocated, the memory will be freed using the user
 * supplied deallocBlock. Note that passing a block that (either directly or
 * indirectly) holds a strong reference the receiver will cause a retain cycle. 
 */
- (instancetype) initWithBytesNoCopy: (void*)bytes
                              length: (NSUInteger)length
                         deallocator: (GSDataDeallocatorBlock)deallocBlock;
#endif
- (id) initWithBytes: (const void*)aBuffer
	      length: (NSUInteger)bufferSize;
- (id) initWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (id) initWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree;
#endif
- (id) initWithContentsOfFile: (NSString*)path;
- (id) initWithContentsOfMappedFile: (NSString*)path;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (id) initWithContentsOfURL: (NSURL*)url;
#endif
- (id) initWithData: (NSData*)data;

// Accessing Data

- (const void*) bytes;
- (NSString*) description;
- (void) getBytes: (void*)buffer;
- (void) getBytes: (void*)buffer
	   length: (NSUInteger)length;
- (void) getBytes: (void*)buffer
	    range: (NSRange)aRange;
- (NSData*) subdataWithRange: (NSRange)aRange;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6,GS_API_LATEST)
- (NSRange) rangeOfData: (NSData *)dataToFind
                options: (NSDataSearchOptions)mask
                  range: (NSRange)searchRange;
#endif

// base64
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)
- (NSData *) base64EncodedDataWithOptions: (NSDataBase64EncodingOptions)options;
- (NSString *) base64EncodedStringWithOptions: (NSDataBase64EncodingOptions)options;
#endif

// Querying a Data Object

- (BOOL) isEqualToData: (NSData*)other;
- (NSUInteger) length;

/**
 * <p>Writes a copy of the data encapsulated by the receiver to a file
 * at path.  If the useAuxiliaryFile flag is YES, this writes to a
 * temporary file and then renames that to the file at path, thus
 * ensuring that path exists and does not contain partially written
 * data at any point.
 * </p>
 * <p>On success returns YES, on failure returns NO.
 * </p>
 */
- (BOOL) writeToFile: (NSString*)path
	  atomically: (BOOL)useAuxiliaryFile;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 * Writes a copy of the contents of the receiver to the specified URL.
 */
- (BOOL) writeToURL: (NSURL*)anURL atomically: (BOOL)flag;
#endif

// Deserializing Data

- (unsigned int) deserializeAlignedBytesLengthAtCursor: (unsigned int*)cursor;
- (void) deserializeBytes: (void*)buffer
		   length: (unsigned int)bytes
		 atCursor: (unsigned int*)cursor;
- (void) deserializeDataAt: (void*)data
		ofObjCType: (const char*)type
		  atCursor: (unsigned int*)cursor
		   context: (id <NSObjCTypeSerializationCallBack>)callback;
- (int) deserializeIntAtCursor: (unsigned int*)cursor;
- (int) deserializeIntAtIndex: (unsigned int)index;
- (void) deserializeInts: (int*)intBuffer
		   count: (unsigned int)numInts
		atCursor: (unsigned int*)cursor;
- (void) deserializeInts: (int*)intBuffer
		   count: (unsigned int)numInts
		 atIndex: (unsigned int)index;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4,GS_API_LATEST)
/**
 * <p>Writes a copy of the data encapsulated by the receiver to a file
 * at path.  If the NSDataWritingAtomic option is set, this writes to a
 * temporary file and then renames that to the file at path, thus
 * ensuring that path exists and does not contain partially written
 * data at any point.
 * </p>
 * <p>On success returns YES, on failure returns NO.
 * </p>
 */
- (BOOL) writeToFile: (NSString *)path
             options: (NSUInteger)writeOptionsMask
               error: (NSError **)errorPtr;

/**
 * Writes a copy of the contents of the receiver to the specified URL.
 */
- (BOOL) writeToURL: (NSURL *)url
            options: (NSUInteger)writeOptionsMask
              error: (NSError **)errorPtr;
#endif
@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

/*
 *	We include special support for coding/decoding - adding methods for
 *	serializing/deserializing type-tags and cross-references.
 *
 *	A type-tag is a byte containing -
 *	Bit7	Set to indicate that the tag is for a cross-reference.
 *	Bit5-6	A value for the size of the type or cross-reference.
 *	Bit0-4	A value representing an Objective-C type.
 */

#define	_GSC_NONE	0x00		/* No type information.		*/
#define	_GSC_XREF	0x80		/* Cross reference to an item.	*/
#define	_GSC_SIZE	0x60		/* Type-size info mask.		*/
#define	_GSC_MASK	0x1f		/* Basic type info mask.	*/

/*
 *	If the tag is for a cross-reference, the size field defines the
 *	size of the cross-reference value -
 *	_GSC_X_0 (no crossref), _GSC_X_1, _GSC_X_2, _GSC_X_4
 */
#define	_GSC_X_0	0x00		/* nil or null pointer		*/
#define	_GSC_X_1	0x20		/* 8-bit cross-ref		*/
#define	_GSC_X_2	0x40		/* 16-bit cross-ref		*/
#define	_GSC_X_4	0x60		/* 32-bit cross-ref		*/

/*
 *	If the tag is for an integer value, the size field defines the
 *	size of the the encoded integer -
 *	_GSC_I16, _GSC_I32, _GSC_I64, _GSC_I128
 *      The file GSConfig.h (produced by the configure script) defines the
 *	size codes for this machines 'natural' integers -
 *	_GSC_S_SHT, _GSC_S_INT, _GSC_S_LNG, _GSC_S_LNG_LNG
 */
#define	_GSC_I16	0x00
#define	_GSC_I32	0x20
#define	_GSC_I64	0x40
#define	_GSC_I128	0x60

/*
 *	For the first sixteen types, the size information applies to the
 *	size of the type, for the second sixteen it applies to the
 *	following cross-reference number (or is zero if no crossref follows).
 */
#define	_GSC_MAYX	0x10		/* Item may have crossref.	*/

/*
 *	These are the types that can be archived -
 */
#define	_GSC_CHR	0x01
#define	_GSC_UCHR	0x02
#define	_GSC_SHT	0x03
#define	_GSC_USHT	0x04
#define	_GSC_INT	0x05
#define	_GSC_UINT	0x06
#define	_GSC_LNG	0x07
#define	_GSC_ULNG	0x08
#define	_GSC_LNG_LNG	0x09
#define	_GSC_ULNG_LNG	0x0a
#define	_GSC_FLT	0x0b
#define	_GSC_DBL	0x0c
#define	_GSC_BOOL       0x0d

#define	_GSC_ID		0x10
#define	_GSC_CLASS	0x11
#define	_GSC_SEL	0x12
#define	_GSC_PTR	0x13
#define	_GSC_CHARPTR	0x14
#define	_GSC_ARY_B	0x15
#define	_GSC_STRUCT_B	0x16
#define	_GSC_CID	0x17	/* Class encoded as id	*/

@interface NSData (GNUstepExtensions)
+ (id) dataWithShmID: (int)anID length: (NSUInteger) length;
+ (id) dataWithSharedBytes: (const void*)bytes length: (NSUInteger) length;

/*
 *	-deserializeTypeTag:andCrossRef:atCursor:
 *	This method is provided in order to give the GNUstep version of
 *	NSUnarchiver maximum possible performance.
 */
- (void) deserializeTypeTag: (unsigned char*)tag
		andCrossRef: (unsigned int*)ref
		   atCursor: (unsigned int*)cursor;
@end
#endif

GS_EXPORT_CLASS
@interface NSMutableData :  NSData

+ (id) dataWithCapacity: (NSUInteger)numBytes;
+ (id) dataWithLength: (NSUInteger)length;
- (id) initWithCapacity: (NSUInteger)capacity;
- (id) initWithLength: (NSUInteger)length;

// Adjusting Capacity

- (void) increaseLengthBy: (NSUInteger)extraLength;
- (void) setLength: (NSUInteger)size;
- (void*) mutableBytes;

// Appending Data

- (void) appendBytes: (const void*)aBuffer
	      length: (NSUInteger)bufferSize;
- (void) appendData: (NSData*)other;

// Modifying Data

- (void) replaceBytesInRange: (NSRange)aRange
		   withBytes: (const void*)bytes;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) replaceBytesInRange: (NSRange)aRange
		   withBytes: (const void*)bytes
		      length: (NSUInteger)length;
#endif
- (void) resetBytesInRange: (NSRange)aRange;
- (void) setData: (NSData*)data;

// Serializing Data

- (void) serializeAlignedBytesLength: (unsigned int)length;
- (void) serializeDataAt: (const void*)data
	      ofObjCType: (const char*)type
		 context: (id <NSObjCTypeSerializationCallBack>)callback;
- (void) serializeInt: (int)value;
- (void) serializeInt: (int)value
	      atIndex: (unsigned int)index;
- (void) serializeInts: (int*)intBuffer
		 count: (unsigned int)numInts;
- (void) serializeInts: (int*)intBuffer
		 count: (unsigned int)numInts
	       atIndex: (unsigned int)index;

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

@interface NSMutableData (GNUstepExtensions)
/*
 *	Capacity management - GNUstep gives you control over the size of
 *	the data buffer as well as the 'length' of valid data in it.
 */
- (NSUInteger) capacity;
- (id) setCapacity: (NSUInteger)newCapacity;

- (int) shmID;	/* Shared memory ID for data buffer (if any)	*/

/*
 *	-serializeTypeTag:
 *	-serializeTypeTag:andCrossRef:
 *	These methods are provided in order to give the GNUstep version of
 *	NSArchiver maximum possible performance.
 */
- (void) serializeTypeTag: (unsigned char)tag;
- (void) serializeTypeTag: (unsigned char)tag
	      andCrossRef: (unsigned int)xref;

@end
#endif

#if	defined(__cplusplus)
}
#endif

#if     !NO_GNUSTEP && !defined(GNUSTEP_BASE_INTERNAL)
#import <GNUstepBase/NSData+GNUstepBase.h>
#endif

#endif /* __NSData_h_GNUSTEP_BASE_INCLUDE */
