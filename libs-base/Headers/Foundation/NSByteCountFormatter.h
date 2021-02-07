/* Definition of class NSByteCountFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   Written by: 	Gregory Casamento <greg.casamento@gmail.com>
   Date: 	July 2019
   
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

#ifndef _NSByteCountFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSByteCountFormatter_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSFormatter.h>
#import	<Foundation/NSDecimalNumber.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class	NSString, NSAttributedString, NSDictionary,
        NSError, NSLocale, NSNumber;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
enum {
  NSByteCountFormatterUseDefault = 0,
  NSByteCountFormatterUseBytes = 1UL << 0,
  NSByteCountFormatterUseKB = 1UL << 1,
  NSByteCountFormatterUseMB = 1UL << 2,
  NSByteCountFormatterUseGB = 1UL << 3,
  NSByteCountFormatterUseTB = 1UL << 4,
  NSByteCountFormatterUsePB = 1UL << 5,
  NSByteCountFormatterUseEB = 1UL << 6,
  NSByteCountFormatterUseZB = 1UL << 7,
  NSByteCountFormatterUseYBOrHigher = 0x0FFUL << 8,
  NSByteCountFormatterUseAll = 0x0FFFFUL
};
typedef NSInteger NSByteCountFormatterUnits;

enum {
  NSByteCountFormatterCountStyleFile = 0,
  NSByteCountFormatterCountStyleMemory = 1,
  NSByteCountFormatterCountStyleDecimal = 2,
  NSByteCountFormatterCountStyleBinary = 3,
};
typedef NSInteger NSByteCountFormatterCountStyle;

GS_EXPORT_CLASS
@interface NSByteCountFormatter : NSFormatter
{
#if	GS_EXPOSE(NSByteCountFormatter)
#endif
#if     GS_NONFRAGILE
#  if	defined(GS_NSByteCountFormatter_IVARS)
@public
GS_NSByteCountFormatter_IVARS;
#  endif
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

- (NSFormattingContext) formattingContext;
- (void) setFormattingContext: (NSFormattingContext)ctx;

- (NSByteCountFormatterCountStyle) countStyle;
- (void) setCountStyle: (NSByteCountFormatterCountStyle)style;

- (BOOL) allowsNonnumericFormatting;
- (void) setAllowsNonnumericFormatting: (BOOL)flag;

- (BOOL) includesActualByteCount;
- (void) setIncludesActualByteCount: (BOOL)flag;

- (BOOL) isAdaptive;
- (void) setAdaptive: (BOOL)flag;

- (NSByteCountFormatterUnits) allowedUnits;
- (void) setAllowedUnits: (NSByteCountFormatterUnits)units;

- (BOOL) includesCount;
- (void) setIncludesCount: (BOOL)flag;

- (BOOL) includesUnit;
- (void) setIncludesUnit: (BOOL)flag;
  
- (BOOL) zeroPadsFractionDigits;
- (void) setZeroPadsFractionDigits: (BOOL)flag;

- (NSString *) stringForObjectValue: (id)obj;

  /* Beta methods... 
- (NSString *) stringFromMeasurement: (NSMeasurement *)measurement;
   End beta methods. */

- (NSString *)stringFromByteCount: (long long)byteCount;
  
+ (NSString *)stringFromByteCount: (long long)byteCount
                       countStyle: (NSByteCountFormatterCountStyle)countStyle;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSByteCountFormatter_h_GNUSTEP_BASE_INCLUDE */

