/* Interface for NSCoder for GNUStep
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.

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

#ifndef __NSCoder_h_GNUSTEP_BASE_INCLUDE
#define __NSCoder_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSGeometry.h>
#import	<Foundation/NSSet.h>
#import	<Foundation/NSZone.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSMutableData, NSData, NSString;

/**
 *  <p>Top-level class defining methods for use when archiving (encoding)
 *  objects to a byte array or file, and when restoring (decoding) objects.
 *  Generally only subclasses of this class are used directly - [NSArchiver],
 *  [NSUnarchiver], [NSKeyedArchiver], [NSKeyedUnarchiver], or [NSPortCoder].
 *  </p>
 *  <p><code>NSPortCoder</code> is used within the distributed objects
 *  framework.  For archiving to/from disk, the <em>Keyed...</em> classes are
 *  preferred for new implementations, since they provide greater
 *  forward/backward compatibility in the face of class changes.</p>
 */
GS_EXPORT_CLASS
@interface NSCoder : NSObject
// Encoding Data

/**
 *  Encodes array of count structures or objects of given type, which may be
 *  obtained through the '<code>@encode(...)</code>' compile-time operator.
 *  Usually this is used for primitives though it can be used for objects as
 *  well.
 */
- (void) encodeArrayOfObjCType: (const char*)type
			 count: (NSUInteger)count
			    at: (const void*)array;

/**
 *  Can be ignored.
 */
- (void) encodeBycopyObject: (id)anObject;
/**
 *  Can be ignored.
 */
- (void) encodeByrefObject: (id)anObject;

/**
 *  Stores bytes directly into archive.  
 */
- (void) encodeBytes: (void*)d length: (NSUInteger)l;

/**
 *  Encode object if it is/will be encoded unconditionally by this coder,
 *  otherwise store a nil.
 */
- (void) encodeConditionalObject: (id)anObject;

/**
 *  Encode an instance of [NSData].
 */
- (void) encodeDataObject: (NSData*)data;

/**
 *  Encodes a generic object.  This will usually result in an
 *  [(NSCoding)-encodeWithCoder:] message being sent to anObject so it
 *  can encode itself.
 */
- (void) encodeObject: (id)anObject;

/**
 *  Encodes a property list by calling [NSSerializer -serializePropertyList:],
 *  then encoding the resulting [NSData] object.
 */
- (void) encodePropertyList: (id)plist;

/**
 *  Encodes a point structure.
 */
- (void) encodePoint: (NSPoint)point;

/**
 *  Encodes a rectangle structure.
 */
- (void) encodeRect: (NSRect)rect;

/**
 *  Store object and objects it refers to in archive (i.e., complete object
 *  graph).
 */
- (void) encodeRootObject: (id)rootObject;

/**
 *  Encodes a size structure.
 */
- (void) encodeSize: (NSSize)size;

/**
 *  Encodes structure or object of given type, which may be obtained
 *  through the '<code>@encode(...)</code>' compile-time operator.  Usually
 *  this is used for primitives though it can be used for objects as well.
 */
- (void) encodeValueOfObjCType: (const char*)type
			    at: (const void*)address;

/**
 *  Multiple version of [-encodeValueOfObjCType:at:].
 */
- (void) encodeValuesOfObjCTypes: (const char*)types,...;

// Decoding Data

/**
 *  Decodes array of count structures or objects of given type, which may be
 *  obtained through the '<code>@encode(...)</code>' compile-time operator.
 *  Usually this is used for primitives though it can be used for objects as
 *  well.  Objects will be retained and you must release them.
 */
- (void) decodeArrayOfObjCType: (const char*)type
                         count: (NSUInteger)count
                            at: (void*)address;

/**
 *  Retrieve bytes directly from archive.
 */
- (void*) decodeBytesWithReturnedLength: (NSUInteger*)l;

/**
 *  Decode an instance of [NSData].
 */
- (NSData*) decodeDataObject;

/**
 *  Decodes a generic object.  Usually the class will be read from the
 *  archive, an object will be created through an <code>alloc</code> call,
 *  then that class will be sent an [(NSCoding)-initWithCoder:] message.
 */
- (id) decodeObject;

/**
 *  Decodes a property list from the archive previously stored through a call
 *  to [-encodePropertyList:].
 */
- (id) decodePropertyList;

/**
 *  Decodes a point structure.
 */
- (NSPoint) decodePoint;

/**
 *  Decodes a rectangle structure.
 */
- (NSRect) decodeRect;

/**
 *  Decodes a size structure.
 */
- (NSSize) decodeSize;

/**
 *  Decodes structure or object of given type, which may be obtained
 *  through the '<code>@encode(...)</code>' compile-time operator.  Usually
 *  this is used for primitives though it can be used for objects as well,
 *  in which case you are responsible for releasing them.
 */
- (void) decodeValueOfObjCType: (const char*)type
			    at: (void*)address;

/**
 *  Multiple version of [-decodeValueOfObjCType:at:].
 */
- (void) decodeValuesOfObjCTypes: (const char*)types,...;

// Managing Zones

/**
 *  Returns zone being used to allocate memory for decoded objects.
 */
- (NSZone*) objectZone;

/**
 *  Sets zone to use for allocating memory for decoded objects.
 */
- (void) setObjectZone: (NSZone*)zone;

// Getting a Version

/**
 *  Returns *Step version, which is not the release version, but a large number,
 *  by specification &lt;1000 for pre-OpenStep.  This implementation returns
 *  a number based on the GNUstep major, minor, and subminor versions.
 */
- (unsigned int) systemVersion;

/**
 *  Returns current version of class (when encoding) or version of decoded
 *  class (decoded).  Version comes from [NSObject -getVersion].
 *  
 */
- (NSInteger) versionForClassName: (NSString*)className;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/*
 * Include GSConfig.h for typedefs/defines of uint8_t, int32_t int64_t
 */
#import <GNUstepBase/GSConfig.h>


/** <override-subclass />
 * Returns a flag indicating whether the receiver supported keyed coding.
 * the default implementation returns NO.  Subclasses supporting keyed
 * coding must override this to return YES.
 */
- (BOOL) allowsKeyedCoding;

/** <override-subclass />
 * Returns a class indicating whether an encoded value corresponding
 * to aKey exists.
 */
- (BOOL) containsValueForKey: (NSString*)aKey;

/** <override-subclass />
 * Returns a boolean value associated with aKey.  This value must previously
 * have been encoded using -encodeBool:forKey:
 */
- (BOOL) decodeBoolForKey: (NSString*)aKey;

/** <override-subclass />
 * Returns a pointer to a byte array associated with aKey.<br />
 * Returns the length of the data in aLength.<br />
 * This value must previously have been encoded using
 * -encodeBytes:length:forKey:
 */
- (const uint8_t*) decodeBytesForKey: (NSString*)aKey
		      returnedLength: (NSUInteger*)alength;

/** <override-subclass />
 * Returns a double value associated with aKey.  This value must previously
 * have been encoded using -encodeDouble:forKey: or -encodeFloat:forKey:
 */
- (double) decodeDoubleForKey: (NSString*)aKey;

/** <override-subclass />
 * Returns a float value associated with aKey.  This value must previously
 * have been encoded using -encodeFloat:forKey: or -encodeDouble:forKey:<br />
 * Precision may be lost (or an exception raised if the value will not fit
 * in a float) if the value was encoded using -encodeDouble:forKey:,
 */
- (float) decodeFloatForKey: (NSString*)aKey;

/** <override-subclass />
 * Returns an integer value associated with aKey.  This value must previously
 * have been encoded using -encodeInt:forKey:, -encodeInt32:forKey:, or
 * -encodeInt64:forKey:.<br />
 * An exception will be raised if the value does not fit in an integer.
 */
- (int) decodeIntForKey: (NSString*)aKey;

/** <override-subclass />
 * Returns a 32-bit integer value associated with aKey.  This value must
 * previously have been encoded using -encodeInt:forKey:,
 * -encodeInt32:forKey:, or -encodeInt64:forKey:.<br />
 * An exception will be raised if the value does not fit in a 32-bit integer.
 */
- (int32_t) decodeInt32ForKey: (NSString*)aKey;

/** <override-subclass />
 * Returns a 64-bit integer value associated with aKey.  This value must
 * previously have been encoded using -encodeInt:forKey:,
 * -encodeInt32:forKey:, or -encodeInt64:forKey:.
 */
- (int64_t) decodeInt64ForKey: (NSString*)aKey;

/** <override-subclass />
 * Returns an object value associated with aKey.  This value must
 * previously have been encoded using -encodeObject:forKey: or
 * -encodeConditionalObject:forKey:
 */
- (id) decodeObjectForKey: (NSString*)aKey;



/** <override-subclass />
 * Encodes aBool and associates the encoded value with aKey.
 */
- (void) encodeBool: (BOOL) aBool forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes the data of the specified length and pointed to by aPointer,
 * and associates the encoded value with aKey.
 */
- (void) encodeBytes: (const uint8_t*)aPointer
	      length: (NSUInteger)length
	      forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes anObject and associates the encoded value with aKey, but only
 * if anObject has already been encoded using -encodeObject:forKey:
 */
- (void) encodeConditionalObject: (id)anObject forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes aDouble and associates the encoded value with aKey.
 */
- (void) encodeDouble: (double)aDouble forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes aFloat and associates the encoded value with aKey.
 */
- (void) encodeFloat: (float)aFloat forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes an int and associates the encoded value with aKey.
 */
- (void) encodeInt: (int)anInteger forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes 32 bit integer and associates the encoded value with aKey.
 */
- (void) encodeInt32: (int32_t)anInteger forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes a 64 bit integer and associates the encoded value with aKey.
 */
- (void) encodeInt64: (int64_t)anInteger forKey: (NSString*)aKey;

/** <override-subclass />
 * Encodes anObject and associates the encoded value with aKey.
 */
- (void) encodeObject: (id)anObject forKey: (NSString*)aKey;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/** <override-subclass />
 * Encodes an NSInteger and associates the encoded value with key.
 */

- (void) encodeInteger: (NSInteger)anInteger forKey: (NSString *)key;
/** <override-subclass />
 * Decodes an NSInteger associated with the key.
 */
- (NSInteger) decodeIntegerForKey: (NSString *)key;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)

#if GS_HAS_DECLARED_PROPERTIES
@property (nonatomic, assign) BOOL requiresSecureCoding;
#else
- (BOOL) requiresSecureCoding;
- (void) setRequiresSecureCoding: (BOOL)requires;
#endif

- (id) decodeObjectOfClass: (Class)cls forKey: (NSString *)key;
- (id) decodeObjectOfClasses: (NSSet *)classes forKey: (NSString *)key;

#endif
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* __NSCoder_h_GNUSTEP_BASE_INCLUDE */
