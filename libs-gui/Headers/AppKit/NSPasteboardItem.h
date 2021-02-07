/** <title>NSPasteboardItem</title>

   <abstract>class for wrapping pasteboard content</abstract>

   Copyright <copy>(C) 2017 Free Software Foundation, Inc.</copy>

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: July 2017

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _GNUstep_H_NSPasteboardItem
#define _GNUstep_H_NSPasteboardItem 
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSPasteboard.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)

@class NSArray;
@class NSData;
@class NSMutableArray;
@class NSMutableDictionary;
@class NSString;

@protocol NSPasteboardItemDataProvider;

@interface NSPasteboardItem : NSObject <NSPasteboardWriting, NSPasteboardReading> {
  NSMutableDictionary *_providerMap;
  NSMutableDictionary *_dataMap;
  NSMutableArray *_types;
}
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, copy) NSArray *types;
#else
- (NSArray *)types;
#endif

- (NSString *)availableTypeFromArray:(NSArray *)types;
- (BOOL)setDataProvider:(id<NSPasteboardItemDataProvider>)dataProvider
               forTypes:(NSArray *)types;
- (BOOL)setData:(NSData *)data forType:(NSString *)type;
- (BOOL)setString:(NSString *)string forType:(NSString *)type;
- (BOOL)setPropertyList:(id)propertyList forType:(NSString *)type;

- (NSData *)dataForType:(NSString *)type;
- (NSString *)stringForType:(NSString *)type;
- (id)propertyListForType:(NSString *)type;
@end

@protocol NSPasteboardItemDataProvider <NSObject>
- (void) pasteboard: (NSPasteboard *)pasteboard
               item: (NSPasteboardItem *)item
 provideDataForType: (NSString *)type;

#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSPasteboardItemDataProvider)
#endif

- (void)pasteboardFinishedWithDataProvider:(NSPasteboard *)pasteboard;
@end

#endif

#endif
