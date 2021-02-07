/** Interface for NSUbiquitousKeyValueStore
   Copyright (C) 2019 Free Software Foundation, Inc.

   Written by: Gregory John Casamento <greg.casamento@gmail.com>
   Created: July 3 2019

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

#ifndef _NSUbiquitousKeyValueStore_h_GNUSTEP_BASE_INCLUDE
#define _NSUbiquitousKeyValueStore_h_GNUSTEP_BASE_INCLUDE

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST)

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSObject.h>

@class GS_GENERIC_CLASS(NSArray, ElementT);
@class GS_GENERIC_CLASS(NSDictionary, KeyT:id<NSCopying>, ValT);
@class NSData;
@class NSString;

#if	defined(__cplusplus)
extern "C" {
#endif

GS_EXPORT_CLASS
@interface NSUbiquitousKeyValueStore : NSObject

// Getting the Shared Instance
+ (NSUbiquitousKeyValueStore *) defaultStore;
  
// Getting Values
// Returns the array associated with the specified key.
- (NSArray *) arrayForKey: (NSString *)key;
  
// Returns the Boolean value associated with the specified key.
- (BOOL) boolForKey: (NSString *)key;

// Returns the data object associated with the specified key.
- (NSData*) dataForKey: (NSString *)key;

// Returns the dictionary object associated with the specified key.
- (NSDictionary *) dictionaryForKey: (NSString *)key;

// Returns the double value associated with the specified key.
- (double) doubleForKey: (NSString *)key;

// Returns the long long value associated with the specified key.
- (long long) longLongForKey: (NSString *)key;

// Returns the object associated with the specified key.
- (id) objectForKey: (NSString *)key;

//  Returns the string associated with the specified key.
- (NSString *) stringForKey: (NSString *)key;

// Setting Values
// Sets an array object for the specified key in the key-value store.
- (void) setArray: (NSArray *)array forKey: (NSString *)key;

// Sets a Boolean value for the specified key in the key-value store.
- (void) setBool: (BOOL)flag forKey: (NSString *)key;

// Sets a data object for the specified key in the key-value store.
- (void) setData: (NSData *)data forKey: (NSString *)key;

// Sets a dictionary object for the specified key in the key-value store.
- (void) setDictionary: (NSDictionary *)dict forKey: (NSString *)key;

// Sets a double value for the specified key in the key-value store.
- (void) setDouble: (double)val forKey: (NSString *)key;

// Sets a long long value for the specified key in the key-value store.
- (void) setLongLong: (long long)val forKey: (NSString *)key;

// Sets an object for the specified key in the key-value store.
- (void) setObject: (id) obj forKey: (NSString *)key;

// Sets a string object for the specified key in the key-value store.
- (void) setString: (NSString *)string forKey: (NSString *)key;

// Explicitly Synchronizing In-Memory Key-Value Data to Disk
// Explicitly synchronizes in-memory keys and values with those stored on disk.
- (void) synchronize;

// Removing Keys
// Removes the value associated with the specified key from the key-value store.
- (void) removeObjectForKey: (NSString *)key;

// Retrieving the Current Keys and Values
// A dictionary containing all of the key-value pairs in the key-value store.
- (NSDictionary *) dictionaryRepresentation;

@end

// Notifications & constants
GS_EXPORT NSString* const NSUbiquitousKeyValueStoreDidChangeExternallyNotification;
GS_EXPORT NSString* const NSUbiquitousKeyValueStoreChangeReasonKey;

#if	defined(__cplusplus)
}
#endif

#endif /* OS_API_VERSION check */

#endif /* _NSUbiquitousKeyValueStore_h_GNUSTEP_BASE_INCLUDE */
