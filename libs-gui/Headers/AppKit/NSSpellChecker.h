/* 
   NSSpellChecker.h

   Class which is interface to spell-checking service

   Copyright (C) 1996, 2001 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2001

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_NSSpellChecker
#define _GNUstep_H_NSSpellChecker

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>

@class NSProxy;
@class NSString;
@class NSArray;
@class NSView;
@class NSPanel;
@class NSDictionary;
@class NSMutableDictionary;

typedef NSInteger NSCorrectionIndicatorType;
enum
{
  NSCorrectionIndicatorTypeDefault = 0,
  NSCorrectionIndicatorTypeReversion,
  NSCorrectionIndicatorTypeGuesses
};

@interface NSSpellChecker : NSObject
{
@private
  /* Non-GUI attributes */
  id _serverProxy;
  NSString *_language;
  NSMutableDictionary *_ignoredWords;

  // Variables to keep state...
  int _position; 
  int _currentTag;
  BOOL _wrapFlag;

  // GUI ...
  id _wordField;
  id _accessoryView;
  id _dictionaryPulldown;
  id _spellPanel;

  // Buttons... (so we can enable/disable, if necessary)
  id _learnButton;
  id _forgetButton;
  id _ignoreButton;
  id _guessButton;
  id _findNextButton;
  id _correctButton;
}

+ (BOOL)isAutomaticTextReplacementEnabled;
+ (BOOL)isAutomaticDashSubstitutionEnabled;
+ (BOOL)isAutomaticQuoteSubstitutionEnabled;

//
// Making a Checker available 
//
+ (NSSpellChecker *)sharedSpellChecker;
+ (BOOL)sharedSpellCheckerExists;

//
// Managing the Spelling Panel 
//
- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)aView;
- (NSPanel *)spellingPanel;

//
// Checking Spelling 
//
- (int)countWordsInString:(NSString *)aString
		 language:(NSString *)language;
- (NSRange)checkSpellingOfString:(NSString *)stringToCheck
		      startingAt:(int)startingOffset;
- (NSRange)checkSpellingOfString:(NSString *)stringToCheck
		      startingAt:(int)startingOffset
			language:(NSString *)language
			    wrap:(BOOL)wrapFlag
	  inSpellDocumentWithTag:(int)tag
		       wordCount:(int *)wordCount;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSRange)checkGrammarOfString:(NSString *)stringToCheck
		     startingAt:(NSInteger)startingOffset
		       language:(NSString *)language
		           wrap:(BOOL)wrapFlag
	 inSpellDocumentWithTag:(NSInteger)tag
			details:(NSArray **)details;
#endif
- (NSArray *)guessesForWord:(NSString *)word;

//
// Setting the Language 
//
- (NSString *)language;
- (BOOL)setLanguage:(NSString *)aLanguage;

//
// Managing the Spelling Process 
//
+ (int)uniqueSpellDocumentTag;
- (void)closeSpellDocumentWithTag:(int)tag;
- (void)ignoreWord:(NSString *)wordToIgnore
  inSpellDocumentWithTag:(int)tag;
- (NSArray *)ignoredWordsInSpellDocumentWithTag:(int)tag;
- (void)setIgnoredWords:(NSArray *)someWords
  inSpellDocumentWithTag:(int)tag;
- (void)setWordFieldStringValue:(NSString *)aString;
- (void)updateSpellingPanelWithMisspelledWord:(NSString *)word;

@end

typedef NSInteger NSCorrectionResponse;
enum
{
  NSCorrectionResponseNone,
  NSCorrectionResponseAccepted,
  NSCorrectionResponseRejected,
  NSCorrectionResponseIgnored,
  NSCorrectionResponseEdited,
  NSCorrectionResponseReverted
};

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
APPKIT_EXTERN NSString *NSSpellCheckerDidChangeAutomaticSpellingCorrectionNotification;
APPKIT_EXTERN NSString *NSSpellCheckerDidChangeAutomaticTextReplacementNotification;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)
APPKIT_EXTERN NSString *NSSpellCheckerDidChangeAutomaticQuoteSubstitutionNotification;
APPKIT_EXTERN NSString *NSSpellCheckerDidChangeAutomaticDashSubstitutionNotification;
#endif

#endif // _GNUstep_H_NSSpellChecker

