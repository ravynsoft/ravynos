/** <title>NSBundleAdditions</title>

   <abstract>Implementation of NSBundle Additions</abstract>

   Copyright (C) 2005 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2005
   
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

#ifndef _GNUstep_H_GSModelLoaderFactory
#define _GNUstep_H_GSModelLoaderFactory

#import <Foundation/NSObject.h>
#import <Foundation/NSZone.h>

@class NSArray;
@class NSData;
@class NSDictionary;
@class NSString;
@class NSBundle;

@interface GSModelLoader : NSObject
+ (NSString *) type;
+ (float) priority;
- (BOOL) loadModelData: (NSData *)data
     externalNameTable: (NSDictionary *)context
              withZone: (NSZone *)zone;
- (BOOL) loadModelFile: (NSString *)fileName
     externalNameTable: (NSDictionary *)context
              withZone: (NSZone *)zone;
- (NSData *)dataForFile: (NSString *)fileName;
@end

@interface GSModelLoaderFactory : NSObject
+ (void) registerModelLoaderClass: (Class)aClass;
+ (Class) classForType: (NSString *)type;
+ (NSArray *) supportedTypes;
+ (NSString *) supportedModelFileAtPath: (NSString *)modelPath;
+ (GSModelLoader *) modelLoaderForFileType: (NSString *)type;
+ (GSModelLoader *) modelLoaderForFileName: (NSString *)modelPath;
@end

#endif
