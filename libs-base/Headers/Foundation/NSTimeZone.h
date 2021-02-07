/* Interface for NSTimeZone for GNUStep
   Copyright (C) 1994, 1996, 1999 Free Software Foundation, Inc.

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

#ifndef __NSTimeZone_h_GNUSTEP_BASE_INCLUDE
#define __NSTimeZone_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSArray;
@class	NSDate;
@class	NSDictionary;
@class  NSLocale;
@class	NSString;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
enum {
  NSTimeZoneNameStyleStandard,
  NSTimeZoneNameStyleShortStandard,
  NSTimeZoneNameStyleDaylightSaving,
  NSTimeZoneNameStyleShortDaylightSaving
};
typedef NSInteger NSTimeZoneNameStyle;
GS_EXPORT NSString * const NSSystemTimeZoneDidChangeNotification;
#endif

GS_EXPORT_CLASS
@interface NSTimeZone : NSObject

//Creating and Initializing an NSTimeZone
+ (NSTimeZone*) localTimeZone;
+ (NSTimeZone*) timeZoneForSecondsFromGMT: (NSInteger)seconds;
+ (NSTimeZone*) timeZoneWithName: (NSString*)aTimeZoneName;

//Managing Time Zones
+ (void) setDefaultTimeZone: (NSTimeZone*)aTimeZone;

// Getting Time Zone Information
+ (NSDictionary*) abbreviationDictionary;
+ (NSArray*) knownTimeZoneNames;

//Getting Arrays of Time Zones
+ (NSArray*) timeZoneArray;
- (NSArray*) timeZoneDetailArray;

#if	OS_API_VERSION(GS_API_NONE, GS_API_LATEST)
/* Returns an dictionary that maps abbreviations to the array
   containing all the time zone names that use the abbreviation. */
+ (NSDictionary*) abbreviationMap;
#endif

#if	OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (void) resetSystemTimeZone;
+ (NSTimeZone*) systemTimeZone;
+ (NSTimeZone*) timeZoneWithName: (NSString*)name data: (NSData*)data;
- (NSString*) abbreviation;
- (NSString*) abbreviationForDate: (NSDate*)aDate;
- (NSData*) data;
- (id) initWithName: (NSString*)name;
- (id) initWithName: (NSString*)name data: (NSData*)data;
- (BOOL) isDaylightSavingTime;
- (BOOL) isDaylightSavingTimeForDate: (NSDate*)aDate;
- (BOOL) isEqualToTimeZone: (NSTimeZone*)aTimeZone;
- (NSString*) name;
- (NSInteger) secondsFromGMT;
- (NSInteger) secondsFromGMTForDate: (NSDate*)aDate;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
- (NSTimeInterval) daylightSavingTimeOffsetForDate: (NSDate *)aDate;
/** Not implemented */
- (NSDate *) nextDaylightSavingTimeTransitionAfterDate: (NSDate *)aDate;

- (NSTimeInterval) daylightSavingTimeOffset;
/** Not implemented */
- (NSDate *) nextDaylightSavingTimeTransition;

- (NSString *)localizedName: (NSTimeZoneNameStyle)style
                     locale: (NSLocale *)locale;
#endif

#if	OS_API_VERSION(GS_API_OPENSTEP, GS_API_MACOSX)
- (NSTimeZoneDetail*) timeZoneDetailForDate: (NSDate*)date;
- (NSString*) timeZoneName;
#endif

/*
 * The next two methods are a problem ... they are present in both
 * OpenStep and MacOS-X, but return different types!
 * We resort to the MaxOS-X version.
 */
+ (NSTimeZone*) defaultTimeZone;
+ (NSTimeZone*) timeZoneWithAbbreviation: (NSString*)abbreviation;  

@end

#if	OS_API_VERSION(GS_API_OPENSTEP, GS_API_MACOSX)
GS_EXPORT_CLASS
@interface NSTimeZoneDetail : NSTimeZone
- (BOOL) isDaylightSavingTimeZone;
- (NSString*) timeZoneAbbreviation;
- (NSInteger) timeZoneSecondsFromGMT;
@end
#endif

#if	defined(__cplusplus)
}
#endif

#endif  /* __NSTimeZone_h_GNUSTEP_BASE_INCLUDE*/

