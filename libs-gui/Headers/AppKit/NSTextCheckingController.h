/* Interface of class NSTextCheckingController
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

#ifndef _NSTextCheckingController_h_GNUSTEP_GUI_INCLUDE
#define _NSTextCheckingController_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>

#import <AppKit/NSTextCheckingClient.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_15, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray, NSDictionary, NSMenu;

@interface NSTextCheckingController : NSObject
{
  id<NSTextCheckingClient> _client;
  NSInteger _spellCheckerDocumentTag;
}
  
// initializer
- (instancetype) initWithClient: (id<NSTextCheckingClient>)client;

// properties...
- (id<NSTextCheckingClient>) client;
- (NSInteger) spellCheckerDocumentTag;
- (void) setSpellCheckerDocumentTag: (NSInteger)tag;

// instance methods...
- (void) changeSpelling: (id)sender;
- (void) checkSpelling: (id)sender;
- (void) checkTextInRange: (NSRange)range 
                    types: (NSTextCheckingTypes)checkingTypes 
                  options: (NSDictionary *)options;
- (void) checkTextInSelection: (id)sender;
- (void) checkTextInDocument: (id)sender;
- (void) didChangeTextInRange: (NSRange)range;
- (void) considerTextCheckingForRange: (NSRange)range;
- (void) didChangeSelectedRange;
- (void) ignoreSpelling: (id)sender;
- (void) insertedTextInRange: (NSRange)range;
- (void) invalidate;
- (NSMenu *) menuAtIndex: (NSUInteger)location
      clickedOnSelection: (BOOL)clickedOnSelection 
          effectiveRange: (NSRangePointer)effectiveRange;
- (void) orderFrontSubstitutionsPanel: (id)sender;
- (void) showGuessPanel: (id)sender;
- (void) updateCandidates;
- (NSArray *) validAnnotations;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSTextCheckingController_h_GNUSTEP_GUI_INCLUDE */

