/* Definition of class NSPersonNameComponentsFormatter
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

#import "Foundation/NSString.h"
#import "Foundation/NSAttributedString.h"
#import "Foundation/NSPersonNameComponents.h"
#import "Foundation/NSPersonNameComponentsFormatter.h"

@implementation NSPersonNameComponentsFormatter

- (instancetype) init
{
  self = [super init];
  if(self != nil)
    {
      _phonetic = NO;
      _style = NSPersonNameComponentsFormatterStyleDefault;
      _nameOptions = 0L;
    }
  return self;
}

- (void) _setNameOptions: (NSPersonNameComponentsFormatterOptions)opts
{
  _nameOptions = opts;
}

// Designated init...
+ (NSString *) localizedStringFromPersonNameComponents: (NSPersonNameComponents *)components
                                                 style: (NSPersonNameComponentsFormatterStyle)nameFormatStyle
                                               options: (NSPersonNameComponentsFormatterOptions)nameOptions
{
  NSPersonNameComponentsFormatter *fmt = [[NSPersonNameComponentsFormatter alloc] init];
  [fmt setStyle: nameFormatStyle];
  [fmt _setNameOptions: nameOptions];
  return [fmt stringForObjectValue: components] ;
}

// Setters
- (NSPersonNameComponentsFormatterStyle) style
{
  return _style;
}

- (void) setStyle: (NSPersonNameComponentsFormatterStyle)style
{
  _style = style;
}

- (BOOL) isPhonetic
{
  return _phonetic;
}

- (void) setPhonetic: (BOOL)flag
{
  _phonetic = flag;
}

// Convenience methods...
- (NSString *) stringFromPersonNameComponents: (NSPersonNameComponents *)components
{
  NSString *result = @"";
  
  switch (_style)
    {
    case NSPersonNameComponentsFormatterStyleDefault:
    case NSPersonNameComponentsFormatterStyleMedium:
      result = [result stringByAppendingString: [components givenName]];
      result = [result stringByAppendingString: @" "];
      result = [result stringByAppendingString: [components familyName]];
      break;
    case NSPersonNameComponentsFormatterStyleShort:
      result = [result stringByAppendingString: [components givenName]];
      break;
    case NSPersonNameComponentsFormatterStyleLong:      
      result = [result stringByAppendingString: [components namePrefix]];
      result = [result stringByAppendingString: @" "];
      result = [result stringByAppendingString: [components givenName]];
      result = [result stringByAppendingString: @" "];
      result = [result stringByAppendingString: [components familyName]];
      result = [result stringByAppendingString: @" "];
      result = [result stringByAppendingString: [components nameSuffix]];
      break;
    case NSPersonNameComponentsFormatterStyleAbbreviated:
      result = [result stringByAppendingString: [[components givenName] substringToIndex: 1]];
      result = [result stringByAppendingString: [[components familyName] substringToIndex: 1]];
      break;
    }
  
  return result;
}

- (NSAttributedString *) annotatedStringFromPersonNameComponents: (NSPersonNameComponents *)components
{
  NSAttributedString *result =  AUTORELEASE([[NSAttributedString alloc] initWithString:
                                                  [self stringFromPersonNameComponents: components]]);
  return result;
}

- (NSPersonNameComponents *) personNameComponentsFromString: (NSString *)string
{
  NSPersonNameComponents *pnc = AUTORELEASE([[NSPersonNameComponents alloc] init]);
  NSArray *nameArray = [string componentsSeparatedByString: @" "];
  NSUInteger count = [nameArray count];

  switch(count)
    {
    case 1:
      [pnc setNickname: [nameArray objectAtIndex: 0]];
      break;
    case 2:
      [pnc setGivenName: [nameArray objectAtIndex: 0]];
      [pnc setFamilyName: [nameArray objectAtIndex: 1]];
      break;
    case 3:
      {
        NSString *first = [[nameArray objectAtIndex: 0] lowercaseString];
        if([first isEqualToString: @"mr"] ||
           [first isEqualToString: @"ms"] ||
           [first isEqualToString: @"mrs"] ||
           [first isEqualToString: @"dr"])
          {
            [pnc setNamePrefix: [nameArray objectAtIndex: 0]];
            [pnc setGivenName:  [nameArray objectAtIndex: 1]];
            [pnc setFamilyName: [nameArray objectAtIndex: 2]];
          }
        else
          {
            [pnc setGivenName:  [nameArray objectAtIndex: 0]];
            [pnc setMiddleName: [nameArray objectAtIndex: 1]];
            [pnc setFamilyName: [nameArray objectAtIndex: 2]];
          }
      }
      break;
    case 4:
      {
        NSString *first = [[nameArray objectAtIndex: 0] lowercaseString];
        if([first isEqualToString: @"mr."] ||
           [first isEqualToString: @"ms."] ||
           [first isEqualToString: @"mrs."] ||
           [first isEqualToString: @"dr."])
          {
            [pnc setNamePrefix: [nameArray objectAtIndex: 0]];
            [pnc setGivenName:  [nameArray objectAtIndex: 1]];
            [pnc setMiddleName: [nameArray objectAtIndex: 2]];
            [pnc setFamilyName: [nameArray objectAtIndex: 3]];
          }
        else
          {
            [pnc setGivenName:  [nameArray objectAtIndex: 0]];
            [pnc setMiddleName: [nameArray objectAtIndex: 1]];
            [pnc setFamilyName: [nameArray objectAtIndex: 2]];
            [pnc setNameSuffix: [nameArray objectAtIndex: 3]];
          }
      }
      break;
    case 5:
      {
        NSString *first = [[nameArray objectAtIndex: 0] lowercaseString];
        if([first isEqualToString: @"mr."] ||
           [first isEqualToString: @"ms."] ||
           [first isEqualToString: @"mrs."] ||
           [first isEqualToString: @"dr."])
          {
            [pnc setNamePrefix: [nameArray objectAtIndex: 0]];
            [pnc setGivenName:  [nameArray objectAtIndex: 1]];
            [pnc setMiddleName: [nameArray objectAtIndex: 2]];
            [pnc setFamilyName: [nameArray objectAtIndex: 3]];
            [pnc setNameSuffix: [nameArray objectAtIndex: 4]];
          }
      }
      break;
    default:
      NSLog(@"Not sure how to parse %@", string);
      pnc = nil;
      break;
    }
  return pnc;
}

- (BOOL) getObjectValue: (id *)obj
              forString: (NSString *)string
       errorDescription: (NSString **)error
{
  NSPersonNameComponents *pnc = [self personNameComponentsFromString: string];
  if(pnc != nil)
    {
      *obj = pnc;
      *error = nil;
    }
  else
    {
      *obj = nil;
      *error = @"Could not parse string into NSPersonNameComponents object";
    }
  return NO;
}

- (NSString *)stringForObjectValue: (id)obj
{
  NSPersonNameComponents *pnc = (NSPersonNameComponents *)obj;
  return [self stringFromPersonNameComponents: pnc];
}

@end

// components for attributed strings;
NSString * const NSPersonNameComponentKey = @"NSPersonNameComponentKey";
NSString * const NSPersonNameComponentGivenName = @"NSPersonNameComponentGivenName";
NSString * const NSPersonNameComponentFamilyName = @"NSPersonNameComponentFamilyName";
NSString * const NSPersonNameComponentMiddleName = @"NSPersonNameComponentMiddleName";
NSString * const NSPersonNameComponentPrefix = @"NSPersonNameComponentPrefix";
NSString * const NSPersonNameComponentSuffix = @"NSPersonNameComponentSuffix";
NSString * const NSPersonNameComponentNickname = @"NSPersonNameComponentNickname";
NSString * const NSPersonNameComponentDelimiter = @"NSPersonNameComponentDelimiter";
