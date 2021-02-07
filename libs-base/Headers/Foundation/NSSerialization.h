/* Protocol for NSSerialization for GNUStep
   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: 1995
   Updated by:	Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1998
   
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

#ifndef __NSSerialization_h_GNUSTEP_BASE_INCLUDE
#define __NSSerialization_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData, NSMutableData;

/**
 *  Objects that are not standard property list constituents can adopt this
 *  protocol to allow themselves to be serialized by an [NSSerializer] and
 *  deserialized by an [NSDeserializer].  <em>Note, this mechanism has been
 *  deprecated and you should instead use [NSArchiver] and related facilities
 *  to serialize objects that are not ordinary property lists.</em>
 */
@protocol NSObjCTypeSerializationCallBack

/**
 *  Decodes an object of given type from data at position cursor.
 */
- (void) deserializeObjectAt: (id*)object
		  ofObjCType: (const char *)type
		    fromData: (NSData*)data
		    atCursor: (unsigned*)cursor;

/**
 *  Encode the given object of given type into data, using a string not a
 *  binary representation.
 */
- (void) serializeObjectAt: (id*)object
		ofObjCType: (const char *)type
		  intoData: (NSMutableData*)data;
@end

/**
 *  <p><em>This class is deprecated in favor of
 *  [NSPropertyListSerialization].</em></p>
 *
 *  <p>It provides a means of producing a byte-array (actually string)
 *  representation of a property list (NSArray or NSDictionary plus limited
 *  contents).</p>
 */
GS_EXPORT_CLASS
@interface NSSerializer: NSObject

/**
 *  <p>Serialize given property list (NSArray or NSDictionary plus limited
 *  contents) into byte array.</p>
 *  <p><em>Deprecated in favor of
 *  [NSPropertyListSerialization+dataFromPropertyList:format:errorDescription:].
 *  </em></p>
 */
+ (NSData*) serializePropertyList: (id)propertyList;

/**
 *  <p>Serialize given property list (NSArray or NSDictionary plus limited
 *  contents) into given mutable byte array.</p>
 *  <p><em>Deprecated in favor of
 *  [NSPropertyListSerialization+dataFromPropertyList:format:errorDescription:].
 *  </em></p>
 */
+ (void) serializePropertyList: (id)propertyList
		      intoData: (NSMutableData*)d;
@end

#if GS_API_VERSION(GS_API_NONE, 011700)
/**
 *	GNUstep extends serialization by having the option to make the
 *	resulting data more compact by ensuring that repeated strings
 *	are only stored once.  If the property-list has a lot of repeated
 *	strings in it, this will be both faster and more space efficient
 *	but it will be slower if the property-list has few repeated
 *	strings.  The default is NOT to generate compact versions of the data.
 *
 *	The [+shouldBeCompact:] method sets default behavior.
 *	The [+serializePropertyList:intoData:compact:] method lets you
 *	override the default behavior.
 */
@interface NSSerializer (GNUstep)

/**
 *  Specify whether to produce compacted format, with repeated strings only
 *  written once.
 */
+ (void) shouldBeCompact: (BOOL)flag;

/**
 *  As [NSSerializer+serializePropertyList:intoData:] but specify whether to
 *  produce compacted format.
 */
+ (void) serializePropertyList: (id)propertyList
		      intoData: (NSMutableData*)d
		       compact: (BOOL)flag;
@end
#endif

/**
 *  <em>This class is deprecated in favor of
 *  [NSPropertyListSerialization].</em> It provides a means of recovering a
 *  property list (NSArray or NSDictionary plus limited contents) from a
 *  byte-array (actually string) representation.
 */
GS_EXPORT_CLASS
@interface NSDeserializer: NSObject

/**
 *  Recover a property list (NSArray or NSDictionary plus limited
 *  contents) from a byte array.
 *  <em>Deprecated in favor of
 *  [NSPropertyListSerialization
 *  +propertyListFromData:mutabilityOption:format:errorDescription:].</em>
 */
+ (id) deserializePropertyListFromData: (NSData*)data
			      atCursor: (unsigned int*)cursor
		     mutableContainers: (BOOL)flag;

/**
 *  Recover a property list (NSArray or NSDictionary plus limited
 *  contents) from a byte array.
 *  <em>Deprecated in favor of
 *  [NSPropertyListSerialization
 *  +propertyListFromData:mutabilityOption:format:errorDescription:].</em>
 */
+ (id) deserializePropertyListFromData: (NSData*)data
		     mutableContainers: (BOOL)flag;

/**
 *  Recover a property list (NSArray or NSDictionary plus limited contents)
 *  from a byte array.  If the data at cursor has a length greater than
 *  length, a proxy is substituted for the actual property list as long as the
 *  constituent objects of that property list are not accessed.
 *  <em>Deprecated in favor of
 *  [NSPropertyListSerialization
 *  +propertyListFromData:mutabilityOption:format:errorDescription:].</em>
 */
+ (id) deserializePropertyListLazilyFromData: (NSData*)data
				    atCursor: (unsigned*)cursor
				      length: (unsigned)length
			   mutableContainers: (BOOL)flag;

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
/**
 *	<p>GNUstep extends deserialization by having the option to make the
 *	resulting data more compact by ensuring that repeated strings
 *	are only stored once.  If the property-list has a lot of repeated
 *	strings in it, this will be more space efficient but it will be
 *	slower (though other parts of your code may speed up through more
 *	efficient equality testing of uniqued strings).
 *	The default is NOT to deserialize uniqued strings.</p>
 *
 *	<p>The [+uniquing:] method turns uniquing on/off.
 *	Uniquing is done using a global [NSCountedSet] - see its documentation
 *	for details.</p>
 */
@interface NSDeserializer (GNUstep)
/**
 * Turns uniquing (collapsing of multiple instances of a single string in the
 * output to one full copy plus references) on/off.
 */
+ (void) uniquing: (BOOL)flag;
@end

#endif

#if	defined(__cplusplus)
}
#endif

#endif /* __NSSerialization_h_GNUSTEP_BASE_INCLUDE */
