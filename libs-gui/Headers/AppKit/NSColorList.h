/* 
   NSColorList.h

   Manage named lists of NSColors.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: 2000
   
   This file is part of the GNUstep GUI Library.

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

#ifndef _GNUstep_H_NSColorList
#define _GNUstep_H_NSColorList
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSString;
@class NSArray;
@class NSMutableArray;
@class NSDictionary;
@class NSMutableDictionary;

@class NSColor;

@interface NSColorList : NSObject <NSCoding>

{
  NSString* _name;
  NSString* _fullFileName;
  BOOL _is_editable;

  // Color Lists are required to be a sort of ordered dictionary
  // For now it is implemented as follows (Scott Christley, 1996):

  // This object contains couples (keys (=color names), values (=colors))
  NSMutableDictionary* _colorDictionary;

  // This object contains the keys (=color names) in order
  NSMutableArray* _orderedColorKeys;
}

//
// Initializing an NSColorList
//
/**
 * Initializes a new, empty color list registered under given name.
 */
- (id) initWithName: (NSString *)name;

/**
 * <p>Initializes a new color list registered under given name, taking
 * contents from the file specified in path.  (Path should include the
 * filename with extension (usually ".clr"), and by convention name should be
 * the same as filename <em>without</em> the extension.)</p>
 *  
 * <p>The format of the file can be either an archive of an NSColorList
 * or an ASCII format.  ASCII files follow this format:</p>
 *
 * <p>first line  =  [#/colors] <br/>
 * each subsequent line describes a color as [int float+ string] <br/>
 * the first int describes the method (RGBA, etc.), the floats
 * provide its arguments (e.g., r, g, b, alpha), and string is name.</p>
 *
 * <p>The <em>method</em> corresponds to one of the [NSColor] initializers.
 * We are looking for documentation of the exact correspondence on OpenStep;
 * for now the only supported method is "0", which is an RGBA format with
 * the arguments in order R,G,B, A.</p>
 */
- (id) initWithName: (NSString *)name
	   fromFile: (NSString *)path;

//
// Getting All Color Lists
//
+ (NSArray *) availableColorLists;

/** Returns the first color list (from the array of available lists)
 * matching name.
 */
+ (NSColorList *) colorListNamed: (NSString *)name;

/** Returns an array containing all the keyus in the color list
 */
- (NSArray *) allKeys;

/** Returns the color for the specified key (if any).
 */
- (NSColor *) colorWithKey: (NSString *)key;

/**
 * Returns a flag indicating whether the receiver is editable.
 */
- (BOOL) isEditable;

/** Inserts a color into the color list at the specified index.
 * Removes any other color with the same name.
 */
- (void) insertColor: (NSColor *)color
		 key: (NSString *)key
	     atIndex: (unsigned)location;

/**
 * Returns the name of the receiver.
 */
- (NSString *) name;

/** Removes the color for the specified key from the list.
 */
- (void) removeColorWithKey: (NSString *)key;

/** Removes the on-disk representation of the list.
 */
- (void) removeFile;

/**
 * Sets the color for this key and appends it to the color list.
 */
- (void) setColor: (NSColor *)aColor
	   forKey: (NSString *)key;

/**
 * Writes the receiver to the specified path.<br />
 * If path is nil, writes to a file located in the current user's personal
 * Colors directory whose name is that of the list with the extension
 * 'clr' appended.<br />
 * If path is a directory, writes to a file in that directory whose name
 * is that of the list with the extension 'clr' appended.<br />
 * Otherwise (path is neither nil nor a directory), writes to the path
 * without appending the l.ist name.<br />
 * Returns YES on success, NO on failure.<br />
 * Writing with a path of nil will cause the receiver to be added to the
 * +availableColorLists if it is not already there.
 */
- (BOOL) writeToFile: (NSString *)path;

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder *)aCoder;
- (id) initWithCoder: (NSCoder *)aDecoder;

@end

/* Notifications */
APPKIT_EXPORT NSString *NSColorListDidChangeNotification;

#endif // _GNUstep_H_NSColorList
