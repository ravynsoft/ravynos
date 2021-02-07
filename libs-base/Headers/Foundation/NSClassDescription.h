/* Interface for NSClassDescription for GNUStep
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date:	2000
   
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

#ifndef __NSClassDescription_h_GNUSTEP_BASE_INCLUDE
#define __NSClassDescription_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSException.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

@class NSArray;
@class NSDictionary;
@class NSString;

/**
 * Posted by [NSClassDescription+classDescriptionForClass:] when a class
 * description cannot be found for a class.  The implementation will check
 * again after the notification is (synchronously) processed, allowing
 * descriptions to be registered lazily.
 */
GS_EXPORT NSString* const NSClassDescriptionNeededForClassNotification;

GS_EXPORT_CLASS
@interface NSClassDescription : NSObject

+ (NSClassDescription*) classDescriptionForClass: (Class)aClass;
+ (void) invalidateClassDescriptionCache;
+ (void) registerClassDescription: (NSClassDescription*)aDescription
			 forClass: (Class)aClass;

- (NSArray*) attributeKeys;
- (NSString*) inverseForRelationshipKey: (NSString*)aKey;
- (NSArray*) toManyRelationshipKeys;
- (NSArray*) toOneRelationshipKeys;

@end

@interface NSObject (NSClassDescriptionPrimitives)

- (NSArray*) attributeKeys;
- (NSClassDescription*) classDescription;
- (NSString*) inverseForRelationshipKey: (NSString*)aKey;
- (NSArray*) toManyRelationshipKeys;
- (NSArray*) toOneRelationshipKeys;

@end

#endif

#if	defined(__cplusplus)
}
#endif

#endif

