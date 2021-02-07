/* 
   NSSpellServer.h

   Class to allow a spell checker to be available to other apps

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2001

   Author of previous version: Scott Christley <scottc@net-community.com>
   Date: 1996
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
    If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_NSSpellServer
#define _GNUstep_H_NSSpellServer

#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>

#if     defined(__cplusplus)
extern "C" {
#endif

// Forward declarations
@class NSConnection;
@class NSMutableArray;
@class NSMutableDictionary;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
GS_EXPORT NSString *const NSGrammarCorrections;
GS_EXPORT NSString *const NSGrammarRange;
GS_EXPORT NSString *const NSGrammarUserDescription;
#endif

GS_EXPORT_CLASS
@interface NSSpellServer : NSObject
{
#if	GS_EXPOSE(NSSpellServer)
@private
  id _delegate;
  BOOL _caseSensitive GS_UNUSED_IVAR; 
  unsigned char _dummy[3] GS_UNUSED_IVAR;
  NSMutableDictionary *_userDictionaries;
  NSString *_currentLanguage;
  NSArray *_ignoredWords;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

// Checking in Your Service 
- (BOOL) registerLanguage: (NSString *)language
		 byVendor: (NSString *)vendor;

// Assigning a Delegate 
- (id) delegate;
- (void) setDelegate: (id)anObject;

// Running the Service 
- (void) run;

// Checking User Dictionaries 
- (BOOL) isWordInUserDictionaries: (NSString *)word
		    caseSensitive: (BOOL)flag;
@end

/**
  This is an informal protocol since the
  NSSpellChecker will need to use a proxy object
  to call these methods.  

  These methods need to be implemented by the spell service
  so that the NSSpellServer instance call call them when
  necessary.
*/
@interface NSObject (NSSpellServerDelegate)
/**
 * <p>
 * This method is called when the user begins spell checking the document.
 * The parameters are: <code>sender</code> the spell server instance which
 * invoked this method, <code>stringToCheck</code> this is the string which
 * the spell service is going to attempt to find misspelled words in,
 * <code>language</code> the language to check in, <code>wordCount</code> the
 * number of words checked, and <code>countOnly</code> a flag which dictates
 * if them method checks the spelling or just counts the words in the given
 * string.
 * </p>
 * <p>
 * Returns a range for any word it finds that is misspelled.
 * </p>
 */
- (NSRange) spellServer: (NSSpellServer *)sender
findMisspelledWordInString: (NSString *)stringToCheck
                  language: (NSString *)language
                 wordCount: (int32_t *)wordCount
                 countOnly: (BOOL)countOnly;

/**
 * Attempts to guess the correct spelling of <code>word</code>. 
 */
- (NSArray *) spellServer: (NSSpellServer *)sender
    suggestGuessesForWord: (NSString *)word
               inLanguage: (NSString *)language;

/**
 * Records the new word in the user's dictionary for the given language.
 */
- (void) spellServer: (NSSpellServer *)sender
        didLearnWord: (NSString *)word
          inLanguage: (NSString *)language;

/**
 * Forgets the given word in the user's dictionary for the given language.
 */
- (void) spellServer: (NSSpellServer *)sender
       didForgetWord: (NSString *)word
          inLanguage: (NSString *)language;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3,GS_API_LATEST) 
/** Not implemented */
- (NSArray *) spellServer: (NSSpellServer *)sender
  suggestCompletionsForPartialWordRange: (NSRange)range
  inString: (NSString *)string
  language: (NSString *)language;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
/** Not implemented */
- (NSRange) spellServer: (NSSpellServer *)sender
  checkGrammarInString: (NSString *)stringToCheck
  language: (NSString *)language
  details: (NSArray **)details;

#endif

@end

#if     defined(__cplusplus)
}
#endif

#endif // _GNUstep_H_NSSpellServer
