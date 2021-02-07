/* Definition of class NSTextInputClient
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

#ifndef _NSTextInputClient_h_GNUSTEP_GUI_INCLUDE
#define _NSTextInputClient_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

#import <AppKit/NSAttributedString.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray;

@protocol NSTextInputClient

#if GS_PROTOCOLS_HAVE_OPTIONAL
@required
#endif
// Marked text
- (BOOL) hasMarkedText;
- (NSRange) markedRange;
- (NSRange) selectedRange;
- (void) setMarkedText: (id)string 
         selectedRange: (NSRange)selectedRange 
      replacementRange: (NSRange)replacementRange;
- (void)unmarkText;
- (NSArray *)validAttributesForMarkedText;

// Storing text
- (NSAttributedString *) attributedSubstringForProposedRange: (NSRange)range 
                                                 actualRange: (NSRangePointer)actualRange;
- (void) insertText: (id)string 
   replacementRange: (NSRange)replacementRange;

// Getting Character coordinates
- (NSUInteger) characterIndexForPoint: (NSPoint)point;
- (NSRect) firstRectForCharacterRange: (NSRange)range 
                          actualRange: (NSRangePointer)actualRange;

// Binding keystrokes
- (void) doCommandBySelector: (SEL)selector;

#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#endif
// Optional methods
- (NSAttributedString *) attributedString;
- (CGFloat) fractionOfDistanceThroughGlyphForPoint: (NSPoint)point;
- (CGFloat) baselineDeltaForCharacterAtIndex: (NSUInteger)anIndex;
- (NSInteger) windowLevel;
- (BOOL) drawsVerticallyForCharacterAtIndex: (NSUInteger)charIndex;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSTextInputClient_h_GNUSTEP_GUI_INCLUDE */

