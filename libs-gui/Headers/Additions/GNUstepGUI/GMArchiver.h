/*
   GMArchiver.h

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: October 1997

   Copyright (C) 1997 Free Software Foundation, Inc.
   All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

/* Portions of the code are based on NSArchiver from libFoundation. See the
   COPYING file from libFoundation for copyright information. */

#ifndef __GMArchiver_h__
#define __GMArchiver_h__

#ifndef GNUSTEP
#import <Foundation/Foundation.h>
#else
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSZone.h>
#endif

@class NSString;
@class NSData;
@class NSArray;
@class NSDictionary;
@class NSHashTable;
@class NSMapTable;
@class NSMutableArray;
@class NSMutableDictionary;

@class GMArchiver;
@class GMUnarchiver;


/* The objects that want to be archived the following protocol. */

@protocol ModelCoding

/* These methods are much like those from the NSCoding protocol.
   The difference is that you can specify names for the instance
   variables or attributes you encode. The recommended way is not to encode
   instance variables but attributes so that an archive file does not
   depend on the particular version of a class or on different
   instance variable names of the class from another implementation. */

- (void)encodeWithModelArchiver:(GMArchiver*)archiver;
- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver;
@end


@interface NSObject (ModelArchivingMethods)
- (id)replacementObjectForModelArchiver:(GMArchiver*)archiver;
- (Class)classForModelArchiver;
+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver;
@end


@interface GMArchiver : NSObject
{
  NSMutableDictionary*	propertyList;
  NSMutableArray* topLevelObjects;
  id lastObjectRepresentation;
  NSMapTable*	objects;	// object -> name
  NSHashTable*	conditionals;	// conditional objects
  NSMapTable*	classes;	// real classname -> class info
  int		counter;
  int		level;
  BOOL 		writingRoot;	// YES if encodeRootObject:withName: was sent
  BOOL findingConditionals;	// YES if finding conditionals
}

/* Initializing an GMArchiver */
- (id)init;

/* Archiving Data */
+ (BOOL)archiveRootObject:(id)rootObject
  toFile:(NSString*)path;
- (BOOL)writeToFile:(NSString*)path;

/* Getting the property list representation from the GMArchiver */
- (id)propertyList;

/* Encoding objects */
- (id)encodeRootObject:(id)rootObject withName:(NSString*)name;
- (id)encodeConditionalObject:(id)object withName:(NSString*)name;
- (id)encodeObject:(id)anObject withName:(NSString*)name;
- (id)encodeString:(NSString*)anObject withName:(NSString*)name;
- (id)encodeArray:(NSArray*)array withName:(NSString*)name;
- (id)encodeDictionary:(NSDictionary*)dictionary withName:(NSString*)name;
- (id)encodeData:(NSData*)anObject withName:(NSString*)name;
- (id)encodeClass:(Class)class withName:(NSString*)name;
- (id)encodeSelector:(SEL)selector withName:(NSString*)name;

/* Encoding the most common C types */
- (void)encodeChar:(char)value withName:(NSString*)name;
- (void)encodeUnsignedChar:(unsigned char)value withName:(NSString*)name;
- (void)encodeBOOL:(BOOL)value withName:(NSString*)name;
- (void)encodeShort:(short)value withName:(NSString*)name;
- (void)encodeUnsignedShort:(unsigned short)value withName:(NSString*)name;
- (void)encodeInt:(int)value withName:(NSString*)name;
- (void)encodeUnsignedInt:(unsigned int)value withName:(NSString*)name;
- (void)encodeLong:(long)value withName:(NSString*)name;
- (void)encodeUnsignedLong:(unsigned long)value withName:(NSString*)name;
- (void)encodeFloat:(float)value withName:(NSString*)name;
- (void)encodeDouble:(double)value withName:(NSString*)name;

/* Encoding geometry types */
- (void)encodePoint:(NSPoint)point withName:(NSString*)name;
- (void)encodeSize:(NSSize)size withName:(NSString*)name;
- (void)encodeRect:(NSRect)rect withName:(NSString*)name;

/* Substituting One Class for Another */
- (NSString*)classNameEncodedForTrueClassName:(NSString*)trueName;
- (void)encodeClassName:(NSString*)trueName
	intoClassName:(NSString*)inArchiveName;

@end /* GMArchiver */


@interface GMUnarchiver : NSObject
{	
  NSMutableDictionary*	propertyList;
  id currentDecodedObjectRepresentation;
  NSMutableDictionary* namesToObjects; // object name -> object
  int level;
  int version;
  NSZone* objectZone;
}

/* Initializing an GMUnarchiver */
+ (id)unarchiverWithContentsOfFile:(NSString*)filename;
- (id)initForReadingWithPropertyList:(id)propertyList;

/* Decoding Objects */
+ (id)unarchiveObjectWithName:(NSString*)name
  fromPropertyList:(id)propertyList;
+ (id)unarchiveObjectWithName:(NSString*)name fromFile:(NSString*)path;

/* Decoding objects */
- (id)decodeObjectWithName:(NSString*)name;
- (NSString*)decodeStringWithName:(NSString*)name;
- (NSArray*)decodeArrayWithName:(NSString*)name;
- (NSDictionary*)decodeDictionaryWithName:(NSString*)name;
- (NSData*)decodeDataWithName:(NSString*)name;
- (Class)decodeClassWithName:(NSString*)name;
- (SEL)decodeSelectorWithName:(NSString*)name;

/* Decoding the most common C types */
- (char)decodeCharWithName:(NSString*)name;
- (unsigned char)decodeUnsignedCharWithName:(NSString*)name;
- (BOOL)decodeBOOLWithName:(NSString*)name;
- (short)decodeShortWithName:(NSString*)name;
- (unsigned short)decodeUnsignedShortWithName:(NSString*)name;
- (int)decodeIntWithName:(NSString*)name;
- (unsigned int)decodeUnsignedIntWithName:(NSString*)name;
- (long)decodeLongWithName:(NSString*)name;
- (unsigned long)decodeUnsignedLongWithName:(NSString*)name;
- (float)decodeFloatWithName:(NSString*)name;
- (double)decodeDoubleWithName:(NSString*)name;

/* Decoding geometry types */
- (NSPoint)decodePointWithName:(NSString*)name;
- (NSSize)decodeSizeWithName:(NSString*)name;
- (NSRect)decodeRectWithName:(NSString*)name;

/* Managing an GMUnarchiver */
- (BOOL)isAtEnd;
- (NSZone*)objectZone;
- (void)setObjectZone:(NSZone*)zone;
- (unsigned int)systemVersion;

/* Substituting One Class for Another */
+ (NSString*)classNameDecodedForArchiveClassName:(NSString*)nameInArchive;
+ (void)decodeClassName:(NSString*)nameInArchive
	asClassName:(NSString*)trueName;
- (NSString*)classNameDecodedForArchiveClassName:(NSString*)nameInArchive;
- (void)decodeClassName:(NSString*)nameInArchive
	asClassName:(NSString*)trueName;
- (unsigned int)versionForClassName:(NSString*)className;

@end

#endif /* __GMArchiver_h__ */
