/** <title>NSSpellServer</title>

   <abstract>Class to allow a spell checker to be available to other apps.</abstract>

   Copyright (C) 2001, 1996 Free Software Foundation, Inc.

   Author by: Gregory John Casamento <borgheron@yahoo.com>
   Date: 2001
   Author: Scott Christley <scottc@net-community.com> 
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
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/ 

#import "common.h"
#define	EXPOSE_NSSpellServer_IVARS	1
#import "Foundation/NSSpellServer.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSConnection.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSException.h"
#import "Foundation/NSSet.h"

NSString *const NSGrammarRange = @"NSGrammarRange";
NSString *const NSGrammarUserDescription = @"NSGrammarUserDescription";
NSString *const NSGrammarCorrections = @"NSGrammarCorrections";

static NSConnection *spellServerConnection = nil;

/* User dictionary location */
static NSString *GNU_UserDictionariesDir = @"Dictionaries";

// Function to create name for spell server....
NSString*
GSSpellServerName(NSString *vendor, NSString *language)
{
  NSString *serverName = nil;
  
  if (language == nil || vendor == nil) 
    {
      return nil;
    }

  serverName = [[vendor stringByAppendingString: language]
		 stringByAppendingString: @"SpellChecker"];

  return serverName;
}

@implementation NSSpellServer

// Class methods
+ (void) initialize
{
  if (self == [NSSpellServer class])
    {
      // Initial version
      [self setVersion: 1];
    }
}

// Non-private Instance methods
- (id) init
{
  NSArray *userPreference;
  NSString *currentLanguage;

  userPreference = [[NSUserDefaults standardUserDefaults]
    stringArrayForKey: @"NSLanguages"];  
  currentLanguage = [userPreference objectAtIndex: 0];
  if ((self = [super init]) != nil)
    {
      _delegate = nil;
      _ignoredWords = nil;
      ASSIGN(_userDictionaries, [NSMutableDictionary dictionary]);
      ASSIGN(_currentLanguage, currentLanguage);
    }

  return self;
}

// Cleanup when deallocated
- (void) dealloc
{
  RELEASE(_userDictionaries);
  RELEASE(_currentLanguage);
  [super dealloc];
}

// Checking in Your Service 

/**
 * This method vends the spell server to the Distributed Objects system
 * so that it can be connected to by clients.
 */
- (BOOL) registerLanguage: (NSString *)language
		 byVendor: (NSString *)vendor
{
  NSString *serverName = GSSpellServerName(vendor, language);
  BOOL result = NO;

  if (serverName == nil)
    {
      return NO;
    }

  spellServerConnection = [[NSConnection alloc] init];
  if (spellServerConnection)
    {
      [spellServerConnection setRootObject: self];
      result = [spellServerConnection registerName: serverName];
    }

  return result;
}

// Assigning a Delegate 

/**
 * Return the spell server delegate.
 */ 
- (id) delegate
{
  return _delegate;
}

/**
 * This method is used to set the delegate of the spellserver.
 * When a spelling service is run the spell server is vended out
 * to DO.  The spelling service must instantiate an instance of 
 * this class and set itself to be the delegate.   This allows
 * the service to respond to messages sent by the client.
 */
- (void) setDelegate: (id)anObject
{
  /* FIXME - we should not retain the delegate ! */
  IF_NO_GC(RETAIN(anObject);)
  ASSIGN(_delegate, anObject);
}

// Running the Service 
/**
 * Initiate the run loop of this service.  Once the spell server
 * object is vended, this method is called so that the server can
 * start responding to the messages sent by the client.  These
 * messages are passed on to the NSSpellServer instance's delegate.
 */
- (void) run
{
  // Start the runloop explicitly.
  [[NSRunLoop currentRunLoop] run];
}

// Private method
// Determine the path to the dictionary
/**
 * Path to the dictionary for the specified language.
 */
- (NSString *) _pathToDictionary: (NSString *)currentLanguage
{
  NSString *path = nil;
  NSString *user_gsroot = nil;
  
  user_gsroot = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
    NSUserDomainMask, YES) lastObject];

  if (currentLanguage != nil)
    {
      NSString *dirPath = nil;
      NSFileManager *mgr = [NSFileManager defaultManager];
      
      // Build the path and try to get the dictionary
      dirPath = [user_gsroot stringByAppendingPathComponent:
        GNU_UserDictionariesDir];
      path =  [dirPath stringByAppendingPathComponent: currentLanguage];
      
      if (![mgr fileExistsAtPath: path ])
	{
	  if ([mgr fileExistsAtPath: dirPath])
	    {
	      // The directory exists create the file.
	      NSArray *emptyDict = [NSArray array];

	      if (![emptyDict writeToFile: path atomically: YES])
		{
		  NSLog(@"Failed to create %@",path);
		  path = nil;
		}
	    }
	  else
	    {
	      // The directory does not exist create it.
	      if ([mgr createDirectoryAtPath: dirPath
                 withIntermediateDirectories: YES
                                  attributes: nil
                                       error: NULL])
		{
		  // Directory created. Now create the empty file.
		  NSArray *emptyDict = [NSArray array];
		  
		  if (![emptyDict writeToFile: path atomically: YES])
		    {
		      NSLog(@"Failed to create %@",path);
		      path = nil;
		    }
		}
	      else
		{
		  NSLog(@"Failed to create %@",dirPath);
		  path = nil;
		}
	    }
	}
    }
  
  return path;
}

// Private method
/** Open up dictionary stored in the user's directory.   */
- (NSMutableSet *) _openUserDictionary: (NSString *)language
{
  NSString *path = nil;
  NSMutableSet *words = nil;

  if ((words = [_userDictionaries objectForKey: language]) == nil)
    {
      if ((path = [self _pathToDictionary: language]) != nil)
	{
	  NSArray *wordarray = [NSArray arrayWithContentsOfFile: path];

	  if (wordarray == nil)
	    {
	      NSLog(@"Unable to load user dictionary from path %@",path);
	    }
	  else
	    {
	      words = [NSMutableSet setWithArray: wordarray];
	      [_userDictionaries setObject: words forKey: language];
	    }
	}
      else
	{
	  NSLog(@"Unable to find user dictionary at: %@", path);
	}
    }

  // successful in opening the desired dictionary..
  return words;
}

// Checking User Dictionaries
/** Check if word is in dict, flag determines if the search is case sensitive. */
- (BOOL) _isWord: (NSString *)word
    inDictionary: (NSSet *)dict
   caseSensitive: (BOOL)flag
{
  BOOL result = NO;
  NSString *dictWord = nil;
  NSEnumerator *setEnumerator = nil;

  // Catch the odd cases before they start trouble later on...
  if (word == nil || dict == nil) 
    {
      return NO; // avoid checking, if NIL.
    }

  if ([word length] == 0 || [dict count] == 0) 
    {
      return NO; // avoid checking, if has no length. 
    }

  // Check the dictionary for the word...
  setEnumerator = [dict objectEnumerator];
  while ((dictWord = [setEnumerator nextObject]) && result == NO)
    {
      // If the case is important then uppercase both strings
      // and compare, otherwise do the comparison.
      if (flag == NO)
	{
	  NSString *upperWord = [word uppercaseString];
	  NSString *upperDictWord = [dictWord uppercaseString];
	  
	  result = [upperWord isEqualToString: upperDictWord];
	}
      else
	{
	  result = [word isEqualToString: dictWord];
	}
    }
  
  return result;
}

// Checking User Dictionaries
/** 
Checks to see if the word is in the user's dictionary.  The user dictionary
is a set of words learned by the spell service for that particular user
combined with the set of ignored words in the current document.
*/
- (BOOL) isWordInUserDictionaries: (NSString *)word
		    caseSensitive: (BOOL)flag
{
  NSSet *userDict = [self _openUserDictionary: _currentLanguage];
  BOOL result = NO;

  if (userDict)
    {
      result = [self _isWord: word
	        inDictionary: userDict
	       caseSensitive: flag];
    }

  if (result == NO && _ignoredWords)
    {
      NSEnumerator *arrayEnumerator = [_ignoredWords objectEnumerator];
      NSString *iword = nil;

      while ((iword = [arrayEnumerator nextObject]) && result == NO)
	{
	  // If the case is important then uppercase both strings
	  // and compare, otherwise do the comparison.
	  if (flag == NO)
	    {
	      NSString *upperWord = [word uppercaseString];
	      NSString *upperIWord = [iword uppercaseString];
	      
	      result = [upperWord isEqualToString: upperIWord];
	    }
	  else
	    {
	      result = [word isEqualToString: iword];
	    }
	}      
    }

  return result;
}

/** Save the dictionary stored in user's directory. */
- (BOOL) _saveUserDictionary: (NSString *)language
{
  NSString *path = nil;

  if ((path = [self _pathToDictionary: language]) != nil)
    {
      NSMutableSet *set = [_userDictionaries objectForKey: language];      

      if (![[set allObjects] writeToFile: path atomically: YES])
	{
	  NSLog(@"Unable to save dictionary to path %@",path);
	  return NO;
	}
    }
  else
    {
      NSLog(@"Unable to save dictionary at: %@", path);
      return NO;
    }
  // successful in saving the desired dictionary..
  return YES; 
}

/** Learn a new word and put it into the dictionary. */
- (BOOL) _learnWord: (NSString *)word
       inDictionary: (NSString *)language
{
  NSMutableSet *set = [self _openUserDictionary: language];
  [set addObject: word];

  NS_DURING
    {
      [_delegate spellServer: self
		didLearnWord: word
		  inLanguage: language];
    }
  NS_HANDLER
    {
      NSLog(@"Call to delegate cause the following exception: %@",
	    [localException reason]);
    }
  NS_ENDHANDLER
  
  return [self _saveUserDictionary: language];
}

/** Forget a word and remove it from the dictionary. */
- (BOOL)_forgetWord: (NSString *)word
       inDictionary: (NSString *)language
{
  NSMutableSet *set = [self _openUserDictionary: language];
  [set removeObject: word];

  NS_DURING
    {
      [_delegate spellServer: self
	       didForgetWord: word
		  inLanguage: language];
    }
  NS_HANDLER
    {
      NSLog(@"Call to delegate caused following exception: %@",
	    [localException reason]);
    }
  NS_ENDHANDLER 

  return [self _saveUserDictionary: language];
}

/** Find a misspelled word. */
- (NSRange) _findMisspelledWordInString: (NSString *)stringToCheck
			       language: (NSString *)language
			   ignoredWords: (NSArray *)ignoredWords
			      wordCount: (int32_t *)wordCount
			      countOnly: (BOOL)countOnly
{
  NSRange r = NSMakeRange(0,0);

  // Forward to delegate
  NS_DURING
    {
      ASSIGN(_ignoredWords,ignoredWords); 
      r = [_delegate spellServer: self
		     findMisspelledWordInString: stringToCheck
		     language: language
		     wordCount: wordCount
		     countOnly: countOnly];
      _ignoredWords = nil;
    }
  NS_HANDLER
    {
      NSLog(@"Call to delegate caused the following exception: %@",
	    [localException reason]);
    }
  NS_ENDHANDLER

  return r;
}

/** Suggest a correction for the word. */
- (NSArray *) _suggestGuessesForWord: (NSString *)word
		          inLanguage: (NSString *)language
{
  NSArray *words = nil;

  // Forward to delegate
  NS_DURING
    {
      words = [_delegate spellServer: self
	       suggestGuessesForWord: word
			  inLanguage: language];
    }
  NS_HANDLER
    {
      NSLog(@"Call to delegate caused the following exception: %@",
	    [localException reason]);
    }
  NS_ENDHANDLER

  return words;
}

- (NSArray *) spellServer: (NSSpellServer *)sender
  suggestCompletionsForPartialWordRange: (NSRange)range
  inString: (NSString *)string
  language: (NSString *)language
{
  return nil;   // FIXME
}

- (NSRange) spellServer: (NSSpellServer *)sender
  checkGrammarInString: (NSString *)stringToCheck
  language: (NSString *)language
  details: (NSArray **)details
{
  return NSMakeRange(0, 0);     // FIXME
}
@end
