/* Interface of class NSTextFinder
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

#ifndef _NSTextFinder_h_GNUSTEP_GUI_INCLUDE
#define _NSTextFinder_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSGeometry.h>

#import <AppKit/AppKitDefines.h>
#import <AppKit/NSNibDeclarations.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSArray, NSView; 
@protocol NSTextFinderClient, NSTextFinderBarContainer;
  
enum
{
 NSTextFinderActionShowFindInterface = 1,
 NSTextFinderActionNextMatch,
 NSTextFinderActionPreviousMatch,
 NSTextFinderActionReplaceAll,
 NSTextFinderActionReplace,
 NSTextFinderActionReplaceAndFind,
 NSTextFinderActionSetSearchString,
 NSTextFinderActionReplaceAllInSelection,
 NSTextFinderActionSelectAll,
 NSTextFinderActionSelectAllInSelection,
 NSTextFinderActionHideFindInterface,
 NSTextFinderActionShowReplaceInterface,
 NSTextFinderActionHideReplaceInterface
};
typedef NSInteger NSTextFinderAction;

enum
{
 NSTextFinderMatchingTypeContains = 0,
 NSTextFinderMatchingTypeStartsWith,
 NSTextFinderMatchingTypeFullWord,
 NSTextFinderMatchingTypeEndsWith
};
typedef NSInteger NSTextFinderMatchingType;

typedef NSString* NSPasteboardTypeTextFinderOptionKey;

APPKIT_EXPORT NSPasteboardTypeTextFinderOptionKey const NSTextFinderCaseInsensitiveKey;
APPKIT_EXPORT NSPasteboardTypeTextFinderOptionKey const NSTextFinderMatchingTypeKey;

@interface NSTextFinder : NSObject <NSCoding>
{
  IBOutlet id<NSTextFinderClient> _client;
  IBOutlet id<NSTextFinderBarContainer> _findBarContainer;
  
  BOOL _findIndicatorNeedsUpdate;
  BOOL _incrementalSearchingEnabled;
  BOOL _incrementalSearchingShouldDimContentView;
  NSArray *_incrementalMatchRanges;

  id _finder;
  NSInteger _tag;
}
  
// Validating and performing
- (void) performAction: (NSTextFinderAction)op;
- (BOOL) validateAction: (NSTextFinderAction)op;
- (void) cancelFindIndicator;

// Properties
- (id<NSTextFinderClient>) client;
- (void) setClient: (id<NSTextFinderClient>) client;
- (id<NSTextFinderBarContainer>) findBarContainer;
- (void) setFindBarContainer: (id<NSTextFinderBarContainer>) findBarContainer;
- (BOOL) findIndicatorNeedsUpdate;
- (void) setFindIndicatorNeedsUpdate: (BOOL)flag;
- (BOOL) isIncrementalSearchingEnabled;
- (void) setIncrementalSearchingEnabled: (BOOL)flag;
- (BOOL) incrementalSearchingShouldDimContentView;
- (void) setIncrementalSearchingShouldDimContentView: (BOOL)flag;
- (NSArray *) incrementalMatchRanges;

+ (void) drawIncrementalMatchHighlightInRect: (NSRect)rect;

- (void) noteClientStringWillChange;  

@end

// PROTOCOLS

@protocol NSTextFinderClient <NSObject>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#endif
- (BOOL) isSelectable;
- (BOOL) allowsMultipleSelection;
- (BOOL) isEditable;
- (NSString *) string;
- (NSString *) stringAtIndex: (NSUInteger)characterIndex
              effectiveRange: (NSRangePointer)outRange
      endsWithSearchBoundary: (BOOL *)outFlag;
- (NSUInteger) stringLength;
- (NSRange) firstSelectedRange;
- (NSArray *) selectedRanges;
- (void) setSelectedRanges: (NSArray *)ranges;  
- (void) scrollRangeToVisible:(NSRange)range;
- (BOOL) shouldReplaceCharactersInRanges: (NSArray *)ranges withStrings: (NSArray *)strings;
- (void) replaceCharactersInRange: (NSRange)range withString: (NSString *)string;
- (void) didReplaceCharacters;
- (NSView *) contentViewAtIndex: (NSUInteger)index effectiveCharacterRange: (NSRangePointer)outRange;
- (NSArray *) rectsForCharacterRange: (NSRange)range;
- (NSArray *) visibleCharacterRanges;
- (void) drawCharactersInRange: (NSRange)range forContentView: (NSView *)view;

@end


@protocol NSTextFinderBarContainer <NSObject>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@required
#endif
- (NSView *) findBarView;
- (void) setFindBarView: (NSView *)view;
- (BOOL) isfindBarVisible;
- (void) setFindBarVisible: (BOOL)flag;  
- (void) findBarViewDidChangeHeight;

#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#endif
- (NSView *) contentView;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSTextFinder_h_GNUSTEP_GUI_INCLUDE */

