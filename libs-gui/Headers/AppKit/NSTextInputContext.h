/* Interface of class NSTextInputContext
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

#ifndef _NSTextInputContext_h_GNUSTEP_GUI_INCLUDE
#define _NSTextInputContext_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <AppKit/NSTextInputClient.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray, NSString;
  
typedef NSString* NSTextInputSourceIdentifier;
  
@interface NSTextInputContext : NSObject
{
  id<NSTextInputClient> _client;
  BOOL _acceptsGlyphInfo;
  NSArray *_allowedInputSourceLocales;
  NSArray *_keyboardInputSources;
  NSTextInputSourceIdentifier _selectedKeyboardInputSource;
}

+ (NSTextInputContext *) currentInputContext;

- (instancetype) initWithClient: (id<NSTextInputClient>)client;

- (id<NSTextInputClient>) client; 

- (BOOL) acceptsGlyphInfo;
- (void) setAcceptsGlyphInfo: (BOOL)flag;
  
- (NSArray *) allowedInputSourceLocales;
- (void) setAllowedInputSourceLocales: (NSArray *)locales;

- (void) activate;
- (void) deactivate;

- (BOOL) handleEvent: (NSEvent *)event;

- (void) discardMarkedText;

- (void) invalidateCharacterCoordinates;

- (NSArray *) keyboardInputSources;

- (NSTextInputSourceIdentifier) selectedKeyboardInputSource;

+ (NSString *) localizedNameForInputSource:(NSTextInputSourceIdentifier)inputSourceIdentifier;
  
@end

APPKIT_EXPORT NSNotificationName NSTextInputContextKeyboardSelectionDidChangeNotification;

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSTextInputContext_h_GNUSTEP_GUI_INCLUDE */

