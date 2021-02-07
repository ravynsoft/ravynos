/** Interface for NSArchiver for GNUStep
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Rewrite by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: March 1995
   
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

   AutogsdocSource:NSUnarchiver.m
   AutogsdocSource:NSArchiver.m
*/ 

#ifndef __NSArchiver_h_GNUSTEP_BASE_INCLUDE
#define __NSArchiver_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSCoder.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSMutableArray, NSMutableDictionary, NSMutableData, NSData, NSString;

#if	OS_API_VERSION(GS_API_OSSPEC,GS_API_LATEST)

GS_EXPORT_CLASS
@interface NSArchiver : NSCoder
{
#if	GS_EXPOSE(NSArchiver)
@private
  NSMutableData	*_data;		/* Data to write into.		*/
  id		_dst;		/* Serialization destination.	*/
  IMP		_serImp;	/* Method to serialize with.	*/
  IMP		_tagImp;	/* Serialize a type tag.	*/
  IMP		_xRefImp;	/* Serialize a crossref.	*/
  IMP		_eObjImp;	/* Method to encode an id.	*/
  IMP		_eValImp;	/* Method to encode others.	*/
#ifndef	_IN_NSARCHIVER_M
#define	GSIMapTable	void*
#endif
  GSIMapTable	_clsMap;	/* Class cross references.	*/
  GSIMapTable	_cIdMap;	/* Conditionally coded.		*/
  GSIMapTable	_uIdMap;	/* Unconditionally coded.	*/
  GSIMapTable	_ptrMap;	/* Constant pointers.		*/
  GSIMapTable	_namMap;	/* Mappings for class names.	*/
  GSIMapTable	_repMap;	/* Mappings for objects.	*/
#ifndef	_IN_NSARCHIVER_M
#undef	GSIMapTable
#endif
  unsigned	_xRefC;		/* Counter for cross-reference.	*/
  unsigned	_xRefO;		/* Counter for cross-reference.	*/
  unsigned	_xRefP;		/* Counter for cross-reference.	*/
  unsigned	_startPos;	/* Where in data we started.	*/
  BOOL		_encodingRoot;
  BOOL		_initialPass;
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

/* Initializing an archiver */
- (id) initForWritingWithMutableData: (NSMutableData*)mdata;

/* Archiving Data */
+ (NSData*) archivedDataWithRootObject: (id)rootObject;
+ (BOOL) archiveRootObject: (id)rootObject toFile: (NSString*)path;

/* Getting data from the archiver */
- (NSMutableData*) archiverData;

/* Substituting Classes */
- (NSString*) classNameEncodedForTrueClassName: (NSString*) trueName;
- (void) encodeClassName: (NSString*)trueName
           intoClassName: (NSString*)inArchiveName;

#if	OS_API_VERSION(GS_API_MACOSX,GS_API_LATEST)
/* Substituting Objects */
- (void) replaceObject: (id)object
	    withObject: (id)newObject;
#endif
@end

#if	GS_API_VERSION(GS_API_NONE,011700)
@interface	NSArchiver (GNUstep)

/*
 *	Re-using the archiver - the 'resetArchiver' method resets the internal
 *	state of the archiver so that you can re-use it rather than having to
 *	destroy it and create a new one.
 *	NB. you would normally want to issue a 'setLength:0' message to the
 *	mutable data object used by the archiver as well, othewrwise the next
 *	root object encoded will be appended to data.
 */
- (void) resetArchiver;

/*
 *	Subclassing with different output format.
 *	NSArchiver normally writes directly to an NSMutableData object using
 *	the methods -
 *		[-serializeTypeTag:]
 *		    to encode type tags for data items, the tag is the
 *		    first byte of the character encoding string for the
 *		    data type (as provided by '@encode(xxx)'), possibly
 *		    with the top bit set to indicate that what follows is
 *		    a crossreference to an item already encoded.
 *		[-serializeCrossRef:],
 *		    to encode a crossreference number either to identify the
 *		    following item, or to refer to a previously encoded item.
 *		    Objects, Classes, Selectors, CStrings and Pointer items
 *		    have crossreference encoding, other types do not.
 *		[-serializeData:ofObjCType:context:]
 *		    to encode all other information.
 *
 *	And uses other NSMutableData methods to write the archive header
 *	information from within the method:
 *		[-serializeHeaderAt:version:classes:objects:pointers:]
 *		    to write a fixed size header including archiver version
 *		    (obtained by [self systemVersion]) and crossreference
 *		    table sizes.  The archiver will do this twice, once with
 *		    dummy values at initialisation time and once with the real
 *		    values.
 *
 *	To subclass NSArchiver, you must implement your own versions of the
 *	four methods above, and override the 'directDataAccess' method to
 *	return NO so that the archiver knows to use your serialization
 *	methods rather than those in the NSMutableData object.
 */
- (BOOL) directDataAccess;
- (void) serializeHeaderAt: (unsigned)positionInData
		   version: (unsigned)systemVersion
		   classes: (unsigned)classCount
		   objects: (unsigned)objectCount
		  pointers: (unsigned)pointerCount;
@end
#endif



GS_EXPORT_CLASS
@interface NSUnarchiver : NSCoder
{
#if	GS_EXPOSE(NSUnarchiver)
@private
  NSData		*data;		/* Data to write into.		*/
  Class			dataClass;	/* What sort of data is it?	*/
  id			src;		/* Deserialization source.	*/
  IMP			desImp;		/* Method to deserialize with.	*/
  void			(*tagImp)(id, SEL, unsigned char*, unsigned*,unsigned*);
  IMP			dValImp;	/* Method to decode data with.	*/
#ifndef	_IN_NSUNARCHIVER_M
#define	GSIArray	void*
#endif
  GSIArray		clsMap;		/* Class crossreference map.	*/
  GSIArray		objMap;		/* Object crossreference map.	*/
  GSIArray		ptrMap;		/* Pointer crossreference map.	*/
#ifndef	_IN_NSUNARCHIVER_M
#undef	GSIArray
#endif
  unsigned		cursor;		/* Position in data buffer.	*/
  unsigned		version;	/* Version of archiver used.	*/
  NSZone		*zone;		/* Zone for allocating objs.	*/
  NSMutableDictionary	*objDict;	/* Class information store.	*/
  NSMutableArray	*objSave;
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

/* Initializing an unarchiver */
- (id) initForReadingWithData: (NSData*)anObject;

/* Decoding objects */
+ (id) unarchiveObjectWithData: (NSData*)anObject;
+ (id) unarchiveObjectWithFile: (NSString*)path;

/* Managing */
- (BOOL) isAtEnd;
- (NSZone*) objectZone;
- (void) setObjectZone: (NSZone*)aZone;
- (unsigned int) systemVersion;

/* Substituting Classes */
+ (NSString*) classNameDecodedForArchiveClassName: (NSString*)nameInArchive;
+ (void) decodeClassName: (NSString*)nameInArchive
	     asClassName: (NSString*)trueName;
- (NSString*) classNameDecodedForArchiveClassName: (NSString*)nameInArchive;
- (void) decodeClassName: (NSString*)nameInArchive 
	     asClassName: (NSString*)trueName;

#if	OS_API_VERSION(GS_API_MACOSX,GS_API_LATEST)
/* Substituting objects */
- (void) replaceObject: (id)anObject withObject: (id)replacement;
#endif
@end

#if OS_API_VERSION(GS_API_NONE,GS_API_NONE) && GS_API_VERSION(1,GS_API_LATEST)
@interface	NSUnarchiver (GNUstep)

- (unsigned) cursor;
- (void) resetUnarchiverWithData: (NSData*)anObject
			 atIndex: (unsigned)pos;

- (BOOL) directDataAccess;
- (void) deserializeHeaderAt: (unsigned*)pos
		     version: (unsigned*)v
		     classes: (unsigned*)c
		     objects: (unsigned*)o
		    pointers: (unsigned*)p;
@end
#endif


/* Exceptions */

/**
 *  Specified in OpenStep to be raised by [NSArchiver] or subclasses if there
 *  are problems initializing or encoding.  <em>Not currently used.
 *  NSInternalInconsistencyException usually raised instead.</em>
 */
GS_EXPORT NSString * const NSInconsistentArchiveException;

#endif	/* OS_API_VERSION */

#if	defined(__cplusplus)
}
#endif

#endif	/* __NSArchiver_h_GNUSTEP_BASE_INCLUDE */
