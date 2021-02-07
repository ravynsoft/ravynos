/* Implementation of class NSTextInputContext
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 02-08-2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import "AppKit/NSTextInputContext.h"

NSTextInputContext *__currentInputContext;

@implementation NSTextInputContext

+ (NSTextInputContext *) currentInputContext
{
  return __currentInputContext;
}

- (instancetype) initWithClient: (id<NSTextInputClient>)client
{
  self = [super init];
  if (self != nil)
    {
      _client = client;
    }
  return self;
}

- (id<NSTextInputClient>) client
{
  return _client;
}

- (BOOL) acceptsGlyphInfo
{
  return _acceptsGlyphInfo;
}

- (void) setAcceptsGlyphInfo: (BOOL)flag
{
  _acceptsGlyphInfo = flag;
}
  
- (NSArray *) allowedInputSourceLocales
{
  return _allowedInputSourceLocales;
}

- (void) setAllowedInputSourceLocales: (NSArray *)locales
{
  ASSIGNCOPY(_allowedInputSourceLocales, locales);
}
  
- (void) activate
{
}

- (void) deactivate
{
}

- (BOOL) handleEvent: (NSEvent *)event
{
  return YES;
}

- (void) discardMarkedText
{
}

- (void) invalidateCharacterCoordinates
{
}

- (NSArray *) keyboardInputSources
{
  return _keyboardInputSources;
}

- (NSTextInputSourceIdentifier) selectedKeyboardInputSource
{
  return _selectedKeyboardInputSource;
}

+ (NSString *) localizedNameForInputSource:(NSTextInputSourceIdentifier)inputSourceIdentifier
{
  return nil;
}

@end

