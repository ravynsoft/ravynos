/** <title>NSSpellChecker</title>

   <abstract>Class to provide the graphical interface to the spell checking
   service.</abstract>

   Copyright (C) 2001, 1996 Free Software Foundation, Inc.

   Author: Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2001,2003

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

#import "config.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSConnection.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSDistantObject.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSProxy.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSSpellServer.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSNib.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSPopUpButton.h"
#import "AppKit/NSSpellChecker.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSWindow.h"
#import "GSGuiPrivate.h"
#import "GNUstepGUI/GSServicesManager.h"

// prototype for function to create name for server
extern NSString *GSSpellServerName(NSString *checkerDictionary, NSString *language);

// These are methods which we only want the NSSpellChecker to call.
// The protocol is defined here so that the outside world does not
// have access to these internal methods.
@protocol NSSpellServerPrivateProtocol
- (NSRange) _findMisspelledWordInString: (NSString *)stringToCheck
			       language: (NSString *)language
			   ignoredWords: (NSArray *)ignoredWords
			      wordCount: (int *)wordCount
			      countOnly: (BOOL)countOnly;

- (BOOL) _learnWord: (NSString *)word
       inDictionary: (NSString *)language;

- (BOOL) _forgetWord: (NSString *)word
        inDictionary: (NSString *)language;

- (NSArray *) _suggestGuessesForWord: (NSString *)word
			  inLanguage: (NSString *)language;
@end

// Methods needed to get the GSServicesManager
@interface NSApplication(NSSpellCheckerMethods)
- (GSServicesManager *)_listener;
@end

@implementation NSApplication(NSSpellCheckerMethods)
- (GSServicesManager *)_listener
{
  return _listener;
}
@end

// Methods in the GSServicesManager to launch the spell server.
@interface GSServicesManager(NSSpellCheckerMethods)
- (id)_launchSpellCheckerForLanguage: (NSString *)language;
- (NSArray *)_languagesForPopUpButton;
@end

@implementation GSServicesManager(NSSpellCheckerMethods)
- (id)_launchSpellCheckerForLanguage: (NSString *)language
{
  id<NSSpellServerPrivateProtocol> proxy = nil;
  NSDictionary *spellCheckers = [_allServices objectForKey: @"BySpell"];
  NSDictionary *checkerDictionary = [spellCheckers objectForKey: language];
  NSString *spellServicePath = [checkerDictionary objectForKey: @"ServicePath"];
  NSString *vendor = [checkerDictionary objectForKey: @"NSSpellChecker"];
  NSDate *finishBy;

  NSString *port = GSSpellServerName(vendor, language);
  double seconds = 30.0;

  NSLog(@"Language: %@", language);
  NSLog(@"Service to start: %@", spellServicePath);
  NSLog(@"Port: %@",port);

  finishBy = [NSDate dateWithTimeIntervalSinceNow: seconds];
  proxy = GSContactApplication(spellServicePath, port, finishBy);
  if (proxy == nil)
    {
      NSLog(@"Failed to contact spell checker for language '%@'", language);
    }
  else
    {
      NSLog(@"Set proxy");
      [(NSDistantObject *)proxy setProtocolForProxy: 
			    @protocol(NSSpellServerPrivateProtocol)];
    }
			  
  return proxy;
}

- (NSArray *)_languagesForPopUpButton
{
  NSDictionary *spellCheckers = [_allServices objectForKey: @"BySpell"];
  NSArray *allKeys = [spellCheckers allKeys];

  return allKeys;
}
@end

// Shared spell checker instance....
static NSSpellChecker *__sharedSpellChecker = nil;
static int __documentTag = 0;

// Implementation of spell checker class
@implementation NSSpellChecker
//
// Class methods
//
+ (void)initialize
{
  if (self == [NSSpellChecker class])
    {
      // Initial version
      [self setVersion:1];
    }
}

+ (BOOL)isAutomaticTextReplacementEnabled
{
  return NO;
}

+ (BOOL)isAutomaticDashSubstitutionEnabled
{
  return NO;
}

+ (BOOL)isAutomaticQuoteSubstitutionEnabled
{
  return NO;
}

//
// Making a Checker available 
//
+ (NSSpellChecker *)sharedSpellChecker
{
  // Create the shared instance.
  if (__sharedSpellChecker == nil)
    {
      __sharedSpellChecker = [[NSSpellChecker alloc] init];
    }
  return __sharedSpellChecker;
}

+ (BOOL)sharedSpellCheckerExists
{
  // If the spell checker has been created, the 
  // variable will not be nil.
  return (__sharedSpellChecker != nil);
}

//
// Managing the Spelling Process 
//
+ (int)uniqueSpellDocumentTag
{
  return ++__documentTag;
}

//
// Internal methods for use by the spellChecker GUI
//
// Support function to start the spell server
- (id)_startServerForLanguage: (NSString *)language
{
  id<NSSpellServerPrivateProtocol> proxy = nil;
  
  // Start the service for this language  
  proxy = [[NSApp _listener] _launchSpellCheckerForLanguage: language];
  
  if (proxy == nil)
    {
      NSLog(@"Failed to get the spellserver");
    }
  else
    {
      // remove any previous notifications we are observing.
      [[NSNotificationCenter defaultCenter] removeObserver: self];
      
      // Make sure that we handle the death of the server correctly.
      [[NSNotificationCenter defaultCenter]
	addObserver: self
	selector: @selector(_handleServerDeath:)
	name: NSConnectionDidDieNotification
	object: [(NSDistantObject *)proxy connectionForProxy]];
    }

  return proxy;
}

- (id)_serverProxy
{
  if (_serverProxy == nil)
    {
      // Start the server and retain the reference to the
      // proxy.
      id<NSSpellServerPrivateProtocol> proxy = [self _startServerForLanguage: _language];
      if (proxy != nil)
	{
	  _serverProxy = proxy;
	  RETAIN(_serverProxy);
	}
    }
  return _serverProxy;
}

- (void)_populateDictionaryPulldown: (NSArray *)dictionaries
{
  [_dictionaryPulldown removeAllItems];
  [_dictionaryPulldown addItemsWithTitles:
    [dictionaries sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)]];
  [_dictionaryPulldown selectItemWithTitle: _language];
}

- (void)_populateAccessoryView
{
  // refresh the columns in the browser
  [_accessoryView reloadColumn: 0];
}

- (void)_handleServerDeath: (NSNotification *)notification
{
  NSLog(@"Spell server died");
  DESTROY(_serverProxy);
}

//
// Instance methods
//
- init
{
  NSArray *languages;

  self = [super init];
  if (self == nil)
    return nil;

  languages = [[NSUserDefaults standardUserDefaults]
    stringArrayForKey: @"NSLanguages"];
  // Set the language to the default for the user.
  _language = RETAIN([languages objectAtIndex: 0]);
  _wrapFlag = NO;
  _position = 0;
  _spellPanel = nil;
  _serverProxy = nil;
  _currentTag = 0;
  _ignoredWords = [NSMutableDictionary new];

  // Load the NIB file from the GUI bundle
  if (![GSGuiBundle() loadNibFile: @"GSSpellPanel"
	  externalNameTable: [NSDictionary dictionaryWithObject: self forKey: NSNibOwner]
	           withZone: [self zone]])
    {
      NSLog(@"Model file load failed for GSSpellPanel");
      return nil;
    }


  return self;
}

- (void)dealloc
{
  RELEASE(_language);
  RELEASE(_ignoredWords);
  RELEASE(_serverProxy);
  [super dealloc];
}

//
// Managing the Spelling Panel 
//
- (NSView *)accessoryView
{
  return _accessoryView;
}

- (void)setAccessoryView:(NSView *)aView
{
  _accessoryView = aView;
}

- (NSPanel *)spellingPanel
{
  return _spellPanel;
}


//
// Checking Spelling 
//
- (int)countWordsInString:(NSString *)aString
		 language:(NSString *)language
{
  int count = 0;
  id<NSSpellServerPrivateProtocol> proxy = [self _serverProxy];

  if (proxy != nil)
    [proxy _findMisspelledWordInString: aString
                              language: _language
                          ignoredWords: nil
                             wordCount: &count
                             countOnly: YES];

  return count;
}

- (NSRange)checkSpellingOfString:(NSString *)stringToCheck
		      startingAt:(int)startingOffset
{
  int wordCount = 0;
  NSRange r;

  r = [self checkSpellingOfString: stringToCheck
  	               startingAt: startingOffset
	                 language: _language
	                     wrap: NO
	   inSpellDocumentWithTag: _currentTag
	                wordCount: &wordCount];

  return r;
}

- (NSRange)checkSpellingOfString:(NSString *)stringToCheck
		      startingAt:(int)startingOffset
                        language:(NSString *)language
		            wrap:(BOOL)wrapFlag
          inSpellDocumentWithTag:(int)tag
		       wordCount:(int *)wordCount
{
  NSRange r;
  NSArray *dictForTag = [self ignoredWordsInSpellDocumentWithTag: tag];
  
  _currentTag = tag;
  // We have no string to work with
  if (stringToCheck == nil)
    {
      return NSMakeRange(0,0);
    }
  else
    // The string is zero length
    if ([stringToCheck length] == 0)
      {
	return NSMakeRange(0,0);
      }

  // Do this in an exception handling block in ensure that a failure of the
  // spellserver does not bring down the application.
  NS_DURING
    {
      id<NSSpellServerPrivateProtocol> proxy = [self _serverProxy];

      // Get the substring and check it.
      NSString *substringToCheck = [stringToCheck substringFromIndex: startingOffset];
      if (proxy == nil)
	NS_VALUERETURN(NSMakeRange(0,0), NSRange);

      r = [proxy _findMisspelledWordInString: substringToCheck
		 language: _language
		 ignoredWords: dictForTag
		 wordCount: wordCount
		 countOnly: NO];
      
      if (r.length != 0)
	{
	  // Adjust results relative to the original string
	  r.location += startingOffset;
	}
      else
	{
	  if (wrapFlag)
	    {
	      // Check the second half of the string
	      NSString *firstHalfOfString = [stringToCheck 
					      substringToIndex: startingOffset];
	      r = [proxy _findMisspelledWordInString: firstHalfOfString
			 language: _language
			 ignoredWords: dictForTag
			 wordCount: wordCount
			 countOnly: NO];
	    }
	}
      NS_VALUERETURN(r, NSRange);
    }
  NS_HANDLER
    {
      NSLog(@"%@",[localException reason]);
    }
  NS_ENDHANDLER
    
  return NSMakeRange(0,0);
}

- (NSArray *)guessesForWord:(NSString *)word
{
  NSArray   *guesses;
 
  // Make the call to the server to get the guesses.
  NS_DURING
    {
      guesses = [[self _serverProxy] _suggestGuessesForWord: word
				     inLanguage: _language];
      NS_VALUERETURN(guesses, id);
    }
  NS_HANDLER
    {
      NSLog(@"%@",[localException reason]);
    }
  NS_ENDHANDLER

  return nil;
}

- (NSRange)checkGrammarOfString:(NSString *)stringToCheck
		     startingAt:(NSInteger)startingOffset
		       language:(NSString *)language
		           wrap:(BOOL)wrapFlag
	 inSpellDocumentWithTag:(NSInteger)tag
			details:(NSArray **)details
{
  return NSMakeRange(NSNotFound, 0);
}

//
// Setting the Language 
//
- (NSString *)language
{
  return _language;
}

- (BOOL)setLanguage:(NSString *)aLanguage
{
  int index = 0;
  BOOL result = NO;

  index = [_dictionaryPulldown indexOfItemWithTitle: aLanguage];
  if (index != -1)
    {
      [_dictionaryPulldown selectItemAtIndex: index];
      result = YES;
    }

  return result;
}

//
// Managing the Spelling Process 
//

// Remove the ignored word list for this 
// document from the dictionary
- (void)closeSpellDocumentWithTag:(int)tag
{
  NSNumber *key = [NSNumber numberWithInt: tag];
  [_ignoredWords removeObjectForKey: key];
}

// Add a word to the ignored list.
- (void)    ignoreWord:(NSString *)wordToIgnore 
inSpellDocumentWithTag:(int)tag
{
  NSNumber *key = [NSNumber numberWithInt: tag];
  NSMutableSet *words = [_ignoredWords objectForKey: key];

  if (![wordToIgnore isEqualToString: @""])
    {
      // If there is a dictionary add to it, if not create one.
      if (words == nil)
	{
	  words = [NSMutableSet setWithObject: wordToIgnore];
	  [_ignoredWords setObject: words forKey: key];
	}
      else
	{
	  [words addObject: wordToIgnore];
	}
    }
}

// get the list of ignored words.
- (NSArray *)ignoredWordsInSpellDocumentWithTag:(int)tag
{
  NSNumber *key = [NSNumber numberWithInt: tag];
  NSSet *words = [_ignoredWords objectForKey: key];
  return [words allObjects];
}

// set the list of ignored words for a given document
- (void)setIgnoredWords:(NSArray *)someWords
 inSpellDocumentWithTag:(int)tag
{
  NSNumber *key = [NSNumber numberWithInt: tag];
  NSSet *words = [NSSet setWithArray: someWords];
  [_ignoredWords setObject: words forKey: key];
}

- (void)setWordFieldStringValue:(NSString *)aString
{
  [_wordField setStringValue: aString];
}

- (void)updateSpellingPanelWithMisspelledWord:(NSString *)word
{
  if ((word == nil) || ([word isEqualToString: @""]))
    {
      [_ignoreButton setEnabled: NO];
      [_guessButton setEnabled: NO];
      NSBeep();
      return;
    }

  [_ignoreButton setEnabled: YES];
  [_guessButton setEnabled: NO];
  [self setWordFieldStringValue: word];
  [self _populateAccessoryView];
}

- (void) _findNext: (id)sender
{
  BOOL processed = [[[NSApp mainWindow] firstResponder]
		       tryToPerform: @selector(checkSpelling:)
			       with: _spellPanel];

  if (!processed)
    {
      NSLog(@"No responder found");
    }
}

- (void) _learn: (id)sender
{
  NSString *word = [_wordField stringValue];

  // Call server and record the learned word.
  NS_DURING
    {
      [[self _serverProxy] _learnWord: word
                         inDictionary: _language];
    }
  NS_HANDLER
    {
      NSLog(@"%@",[localException reason]);
    }
  NS_ENDHANDLER

  [self _findNext: sender];
}

- (void) _forget: (id)sender
{
  NSString *word = [_wordField stringValue];

  // Call the server and remove the word from the learned
  // list.
  NS_DURING
    {
      [[self _serverProxy] _forgetWord: word
                          inDictionary: _language];
    }
  NS_HANDLER
    {
      NSLog(@"%@",[localException reason]);
    }
  NS_ENDHANDLER

  [self _findNext: sender];
}

- (void) _ignore: (id)sender
{
  BOOL processed = [[[NSApp mainWindow] firstResponder]
		       tryToPerform: @selector(ignoreSpelling:)
			       with: _wordField];

  if (!processed)
    {
      NSLog(@"_ignore: No responder found");
    }

  [self _findNext: sender];
}

- (void) _guess: (id)sender
{
  // Fill in the view...
  [self _populateAccessoryView];
}

- (void) _correct: (id)sender
{
  BOOL processed = [[[NSApp mainWindow] firstResponder]
		       tryToPerform: @selector(changeSpelling:)
			       with: _wordField];

  if (!processed)
    {
      NSLog(@"No responder found");
    }
  [self _findNext: sender];
}

- (void) _switchDictionary: (id)sender
{
  id<NSSpellServerPrivateProtocol> proxy = nil;
  NSString *language = nil;

  // Start the service for this language  
  language = [_dictionaryPulldown stringValue];
  if (![language isEqualToString: _language])
    {
      NSLog(@"Language = %@",language);
      proxy = [self _startServerForLanguage: language];
      if (proxy != nil)
	{
	  ASSIGN(_language, language);
	  ASSIGN(_serverProxy, (id)proxy);
	}
      else
	{
	  // Reset the pulldown to the proper language.
	  [_dictionaryPulldown selectItemWithTitle: _language];
	}
    }
}

- (void) _highlightGuess: (id)sender
{
  NSString *selectedGuess = nil;

  selectedGuess = [[_accessoryView selectedCell] stringValue];
  [_ignoreButton setEnabled: NO];
  [_guessButton setEnabled: YES];
  [_wordField setStringValue: selectedGuess];
}

- (void) awakeFromNib
{
  [self _populateDictionaryPulldown: 
	  [[NSApp _listener] _languagesForPopUpButton]];
  [_accessoryView setDelegate: self];
  [_accessoryView setDoubleAction: @selector(_correct:)];

  [_findNextButton setKeyEquivalent: @"n"];
  [_findNextButton setKeyEquivalentModifierMask: NSCommandKeyMask];
  [_ignoreButton setKeyEquivalent: @"i"];
  [_ignoreButton setKeyEquivalentModifierMask: NSCommandKeyMask];
  [_learnButton setKeyEquivalent: @"l"];
  [_learnButton setKeyEquivalentModifierMask: NSCommandKeyMask];
  [_forgetButton setKeyEquivalent: @"f"];
  [_forgetButton setKeyEquivalentModifierMask: NSCommandKeyMask];
  [_guessButton setKeyEquivalent: @"g"];
  [_guessButton setKeyEquivalentModifierMask: NSCommandKeyMask];
  [_correctButton setKeyEquivalent: @"c"];
  [_correctButton setKeyEquivalentModifierMask: NSCommandKeyMask];
  [_correctButton setImagePosition: NSImageRight];
  [_correctButton setImage: [NSImage imageNamed: @"common_ret"]];
  [_correctButton setAlternateImage: [NSImage imageNamed: @"common_retH"]];
  [_spellPanel makeFirstResponder: _correctButton];
  [_spellPanel setBecomesKeyOnlyIfNeeded: YES];
  [_spellPanel setFloatingPanel: YES];
}
@end

@interface NSSpellChecker(SpellBrowserDelegate)
- (BOOL) browser: (NSBrowser*)sender selectRow: (NSInteger)row inColumn: (NSInteger)column;

- (void) browser: (NSBrowser *)sender createRowsForColumn: (NSInteger)column
	inMatrix: (NSMatrix *)matrix;

- (NSString*) browser: (NSBrowser*)sender titleOfColumn: (NSInteger)column;

- (void) browser: (NSBrowser *)sender 
 willDisplayCell: (id)cell 
	   atRow: (NSInteger)row 
	  column: (NSInteger)column;

- (BOOL) browser: (NSBrowser *)sender isColumnValid: (NSInteger)column;
@end

@implementation NSSpellChecker(SpellBrowserDelegate)
- (BOOL) browser: (NSBrowser*)sender selectRow: (NSInteger)row inColumn: (NSInteger)column
{
  return YES;
}

- (void) browser: (NSBrowser *)sender createRowsForColumn: (NSInteger)column
	inMatrix: (NSMatrix *)matrix
{
  NSArray   *guesses = [self guessesForWord: [_wordField stringValue]];
  NSEnumerator    *e = [guesses objectEnumerator];
  NSString     *word = nil;
  NSBrowserCell *cell= nil;
  NSInteger i = 0;

  while ((word = [e nextObject]) != nil)
    {
      [matrix insertRow: i withCells: nil];
      cell = [matrix cellAtRow: i column: 0];
      [cell setLeaf: YES];
      i++;
      [cell setStringValue: word];
    }
}

- (NSString*) browser: (NSBrowser*)sender titleOfColumn: (NSInteger)column
{
  return _(@"Guess");
}

- (void) browser: (NSBrowser *)sender 
 willDisplayCell: (id)cell 
	   atRow: (NSInteger)row 
	  column: (NSInteger)column
{
}

- (BOOL) browser: (NSBrowser *)sender isColumnValid: (NSInteger)column
{
  return NO;
}
@end
