/* Definition of class NSTextCheckingClient
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

#ifndef _NSTextCheckingClient_h_GNUSTEP_GUI_INCLUDE
#define _NSTextCheckingClient_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSGeometry.h>

#import <AppKit/NSTextInputClient.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_15, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

enum
{
 NSTextInputTraitTypeDefault,
 NSTextInputTraitTypeNo,
 NSTextInputTraitTypeYes
};
typedef NSInteger NSTextInputTraitType;

@class NSDictionary, NSCandidateListTouchBarItem;

@protocol NSTextInputTraits

#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#endif
- (NSTextInputTraitType) autocorrectionType;
- (void) setAutocorrectionType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) spellCheckingType;
- (void) setSpellCheckingType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) grammarCheckingType;
- (void) setGrammarCheckingType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) smartQuotesType;
- (void) setSmartQuotesType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) smartDashesType;
- (void) setSmartDashesType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) smartInsertDeleteType;
- (void) setSmartInsertDeleteType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) textReplacementType;
- (void) setTextReplacementType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) dataDetectionType;
- (void) setDataDetectionType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) linkDetectionType;
- (void) setLinkDetectionType: (NSTextInputTraitType)type;  
- (NSTextInputTraitType) textCompletionType;
- (void) setTextCompletionType: (NSTextInputTraitType)type;
  
@end


@protocol NSTextCheckingClient <NSTextInputClient, NSTextInputTraits>

#if GS_PROTOCOLS_HAVE_OPTIONAL
@required
#endif
- (void) addAnnotations: (NSDictionary *)annotations 
                  range: (NSRange)range;

- (NSAttributedString *) annotatedSubstringForProposedRange: (NSRange)range 
                                                actualRange: (NSRangePointer)actualRange;

- (NSCandidateListTouchBarItem *) candidateListTouchBarItem;
  
- (void) removeAnnotation: (NSAttributedStringKey)annotationName 
                    range: (NSRange)range;
  
- (void) replaceCharactersInRange: (NSRange)range 
              withAnnotatedString: (NSAttributedString *)annotatedString;
  
- (void) selectAndShowRange: (NSRange)range;
  
- (void) setAnnotations: (NSDictionary *)annotations 
                  range: (NSRange)range;
  
- (NSView *) viewForRange: (NSRange)range 
                firstRect: (NSRectPointer)firstRect 
              actualRange: (NSRangePointer)actualRange;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSTextCheckingClient_h_GNUSTEP_GUI_INCLUDE */

