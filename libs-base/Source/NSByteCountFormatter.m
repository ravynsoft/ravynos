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

#define	GS_NSByteCountFormatter_IVARS \
 NSFormattingContext _formattingContext; \
 NSByteCountFormatterCountStyle _countStyle; \
 BOOL _allowsNonnumericFormatting; \
 BOOL _includesActualByteCount; \
 BOOL _adaptive; \
 NSByteCountFormatterUnits _allowedUnits; \
 BOOL _includesCount; \
 BOOL _includesUnit; \
 BOOL _zeroPadsFractionDigits; 

#define	EXPOSE_NSByteCountFormatter_IVARS	1

#import <Foundation/NSByteCountFormatter.h>
#import <Foundation/NSString.h>
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSValue.h>

#define	GSInternal		NSByteCountFormatterInternal
#include	"GSInternal.h"
GS_PRIVATE_INTERNAL(NSByteCountFormatter)

// Unit definitions...
#define KB (double)1024.0
#define MB (double)(1024.0 * 1024.0)
#define GB (double)(1024.0 * 1024.0 * 1024.0)
#define TB (double)(1024.0 * 1024.0 * 1024.0 * 1024.0)
#define PB (double)(1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0)
#define EB (double)(1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0)
#define ZB (double)(1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0)
#define YB (double)(1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0)

@implementation NSByteCountFormatter
  
+ (NSString *)stringFromByteCount: (long long)byteCount
                       countStyle: (NSByteCountFormatterCountStyle)countStyle
{
  NSByteCountFormatter *formatter = [[NSByteCountFormatter alloc] init];
  [formatter setCountStyle: countStyle];
  return [formatter stringFromByteCount: byteCount];
}

- (NSString *)stringForObjectValue: (id)obj
{
  long long byteCount = 0;
  
  if([obj respondsToSelector: @selector(longLongValue)])
    {
      byteCount = [obj longLongValue];
    }

  return [self stringFromByteCount: byteCount];
}

- (NSByteCountFormatterUnits) _adaptiveSettings: (double)byteCount
{
  NSByteCountFormatterUnits units = NSByteCountFormatterUseDefault;
  
  if (byteCount >= KB || byteCount == 0.0) 
    {
      units = NSByteCountFormatterUseKB;
    }
  if (byteCount >= MB) 
    {
      units = NSByteCountFormatterUseMB;
    }
  if (byteCount >= GB) 
    {
      units = NSByteCountFormatterUseGB;
    }
  if (byteCount >= TB) 
    {
      units = NSByteCountFormatterUseTB;
    }
  if (byteCount >= PB) 
    {
      units = NSByteCountFormatterUsePB;
    }
  if (byteCount >= EB) 
    {
      units = NSByteCountFormatterUseEB;
    }
  if (byteCount >= YB) 
    {
      units = NSByteCountFormatterUseYBOrHigher;
    }
  
  return units;
}

- (NSString *)stringFromByteCount: (long long)byteCount
{
  NSString *result = nil;
  double bc = (double)byteCount;
  double count = 0;
  NSString *outputFormat = @"";
  NSString *unitName = @"";
  NSByteCountFormatterUnits allowed = internal->_allowedUnits;

  if(internal->_adaptive)
    {
      allowed = [self _adaptiveSettings: bc];
    }
  else if (allowed == NSByteCountFormatterUseDefault)
    {
      allowed = NSByteCountFormatterUseMB;
    }

  if (allowed & NSByteCountFormatterUseYBOrHigher)
    {
      count = bc / YB;
      unitName = @"YB";
    }
  if (allowed & NSByteCountFormatterUseEB)
    {
      count = bc / EB;
      unitName = @"EB";
    }
  if (allowed & NSByteCountFormatterUsePB)
    {
      count = bc / PB;
      unitName = @"PB";
    }
  if (allowed & NSByteCountFormatterUseTB)
    {
      count = bc / TB;
      unitName = @"TB";
    }
  if (allowed & NSByteCountFormatterUseGB)
    {
      count = bc / GB;
      unitName = @"GB";
    }
  if (allowed & NSByteCountFormatterUseMB)
    {
      count = bc / MB;	    
      unitName = @"MB";
    }
  if (allowed & NSByteCountFormatterUseKB)
    {
      count = bc / KB;
      unitName = @"KB";      
    }
  if (allowed & NSByteCountFormatterUseBytes)
    {
      count = bc;
      unitName = @"bytes";
    }

  if(internal->_allowsNonnumericFormatting && count == 0.0)
    {
      outputFormat = [outputFormat stringByAppendingString: @"Zero"];
    }
  else
    {
      if(internal->_zeroPadsFractionDigits)
	{
	  outputFormat = [outputFormat stringByAppendingString: @"%01.08f"];
	}
      else
	{
	  NSInteger whole = (NSInteger)(count / 1);
	  double frac = (double)count - (double)whole;
	  if(frac > 0.0)
	    {
	      whole += 1;
	    }
	  count = (double)whole;
	  outputFormat = [outputFormat stringByAppendingString: @"%01.0f"];
	}
    }
  
  if(internal->_includesUnit)
    {
      NSString *paddedUnit = [NSString stringWithFormat: @" %@",unitName];
      outputFormat = [outputFormat stringByAppendingString: paddedUnit];
    }

  // Do the formatting...
  result = [NSString stringWithFormat: outputFormat, count];
  
  return result;
}

- (id) init
{
  self = [super init];
  if(self == nil)
    {
      return nil;
    }

  GS_CREATE_INTERNAL(NSByteCountFormatter);

  internal->_countStyle = NSByteCountFormatterCountStyleFile;
  internal->_allowedUnits = NSByteCountFormatterUseDefault;
  internal->_adaptive = YES;
  internal->_formattingContext = NSFormattingContextUnknown;
  internal->_allowsNonnumericFormatting = YES;
  internal->_includesUnit = YES;

  return self;
}

- (NSFormattingContext) formattingContext
{
  return internal->_formattingContext;
}

- (void) setFormattingContext: (NSFormattingContext)ctx
{
  internal->_formattingContext = ctx;
}

- (NSByteCountFormatterCountStyle) countStyle
{
  return internal->_countStyle;
}

- (void) setCountStyle: (NSByteCountFormatterCountStyle)style
{
  internal->_countStyle = style;
}

- (BOOL) allowsNonnumericFormatting
{
  return internal->_allowsNonnumericFormatting;
}

- (void) setAllowsNonnumericFormatting: (BOOL)flag
{
  internal->_allowsNonnumericFormatting = flag;
}

- (BOOL) includesActualByteCount
{
  return internal->_includesActualByteCount;
}

- (void) setIncludesActualByteCount: (BOOL)flag
{
  internal->_includesActualByteCount = flag;
}

- (BOOL) isAdaptive
{
  return internal->_adaptive;
}

- (void) setAdaptive: (BOOL)flag
{
  internal->_adaptive = flag;
}

- (NSByteCountFormatterUnits) allowedUnits
{
  return internal->_allowedUnits;
}

- (void) setAllowedUnits: (NSByteCountFormatterUnits)units
{
  internal->_allowedUnits = units;
}

- (BOOL) includesCount
{
  return internal->_includesCount;
}

- (void) setIncludesCount: (BOOL)flag
{
  internal->_includesCount = flag;
}

- (BOOL) includesUnit
{
  return internal->_includesUnit;
}

- (void) setIncludesUnit: (BOOL)flag
{
  internal->_includesUnit = flag;
}
  
- (BOOL) zeroPadsFractionDigits
{
  return internal->_zeroPadsFractionDigits;
}

- (void) setZeroPadsFractionDigits: (BOOL)flag
{
  internal->_zeroPadsFractionDigits = flag;
}

@end

