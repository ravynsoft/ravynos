/* Definition of class NSPersonNameComponents
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   Implemented by: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sep 2019

   This file is part of the GNUstep Library.
   
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

#ifndef __NSPersonNameComponents_h_GNUSTEP_BASE_INCLUDE
#define __NSPersonNameComponents_h_GNUSTEP_BASE_INCLUDE

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7,GS_API_LATEST)

#import <Foundation/NSObject.h>

@class NSString;

GS_EXPORT_CLASS
@interface NSPersonNameComponents : NSObject <NSCopying, NSCoding>
{
@private
 NSString *_namePrefix;
 NSString *_givenName;
 NSString *_middleName;
 NSString *_familyName;
 NSString *_nameSuffix;
 NSString *_nickname;
 NSPersonNameComponents *_phoneticRepresentation;
}

- (NSString *) namePrefix;
- (void) setNamePrefix: (NSString *)namePrefix;
- (NSString *) givenName;
- (void) setGivenName: (NSString *)givenName;
- (NSString *) middleName;
- (void) setMiddleName: (NSString *)middleName;
- (NSString *) familyName;
- (void) setFamilyName: (NSString *)familyName;
- (NSString *) nameSuffix;
- (void) setNameSuffix: (NSString *)nameSuffix;
- (NSString *) nickname;
- (void) setNickname: (NSString *)nickname;

- (NSPersonNameComponents *) phoneticRepresentation;
- (void) setPhoneticRepresentation: (NSPersonNameComponents *)pr;

@end

#endif
#endif
