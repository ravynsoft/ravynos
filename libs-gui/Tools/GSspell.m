/* 
   GSspell.m

   GNUstep spell checker facility.

   Copyright (C) 2001, 2010 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: May 2001

   Author:  Wolfgang Lux <wolfgang.lux@gmail.com>
   Date: January 2010
   
   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.
    
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public  
   License along with this library; see the file COPYING.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.

*/ 

// get the configuration.
#include "config.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#ifdef HAVE_ASPELL_H
#import <GNUstepBase/GSLocale.h>
#import <GNUstepBase/Unicode.h>
#include <aspell.h>
#endif

// A minor category for NSData so that we can convert NSStrings
// into data.
@interface NSData (MethodsForSpellChecker)
+ (id)dataWithString: (NSString *)string;
@end

@implementation NSData (MethodsForSpellChecker)
+ (id)dataWithString: (NSString *)string
{
  NSData *data = [NSData dataWithBytes: (char *)[string cString]
			        length: [string length]];
  return data;
}
@end

// A category for NSBundle so that we can determine the languages
// vended by a service bundle
@interface NSBundle (MethodsForSpellChecker)
- (NSArray *) serviceLanguages;
@end

@implementation NSBundle (MethodsForSpellChecker)
- (NSArray *) serviceLanguages
{
  NSDictionary *infoDict = [self infoDictionary];
  if ([infoDict isKindOfClass: [NSDictionary class]])
    {
      NSArray *services = [infoDict objectForKey: @"NSServices"];
      if ([services isKindOfClass: [NSArray class]] && [services count] > 0)
	{
	  NSDictionary *serviceDict = [services objectAtIndex: 0];
	  if ([serviceDict isKindOfClass: [NSDictionary class]])
	    {
	      NSArray *languages = [serviceDict objectForKey: @"NSLanguages"];
	      if ([languages isKindOfClass: [NSArray class]])
		{
		  return languages;
		}
	    }
	}
    }
  return nil;
}
@end

// The base class.  Its spell checker just provides a dumb spell checker
// for American English as fallback if aspell is not available.

@interface GNUSpellChecker : NSObject
- (BOOL) registerLanguagesWithServer: (NSSpellServer *)aServer;
- (NSArray *) languages;
@end

@implementation GNUSpellChecker

- (BOOL) registerLanguagesWithServer: (NSSpellServer *)aServer
{
  BOOL success = NO;
  NSEnumerator *langEnum;
  NSString *language;

  langEnum = [[self languages] objectEnumerator];
  while ((language = [langEnum nextObject]) != nil)
    {
      if ([aServer registerLanguage: language byVendor: @"GNU"])
	{
	  NSLog(@"Registered spell server for language %@", language);
	  success = YES;
	}
      else
	{
	  NSLog(@"Could not register spell server for language %@", language);
	}
    }
  return success;
}

- (NSArray *) languages
{
  return [NSArray arrayWithObject: @"AmericanEnglish"];
}

- (BOOL) createBundleAtPath: (NSString *)path languages: (NSArray *)languages
{
  NSDictionary *infoDict, *serviceDict;
  NSFileManager *fm = [NSFileManager defaultManager];
  NSString *execPath;

  if ([fm fileExistsAtPath: path] && ![fm removeFileAtPath: path handler: nil])
    {
      NSLog(@"cannot remove %@", path);
      return NO;
    }

  path = [path stringByAppendingPathComponent: @"Resources"];
  if (![fm createDirectoryAtPath: path
     withIntermediateDirectories: YES
                      attributes: nil
                           error: NULL])
    {
      NSLog(@"cannot not create bundle directory %@", path);
      return NO;
    }

  path = [path stringByAppendingPathComponent: @"Info-gnustep"];
  path = [path stringByAppendingPathExtension: @"plist"];

  /* FIXME Not sure if the executable path is needed in the service dictionary.
     However, GSspellInfo.plist has it and so we include it here too. */
  execPath = [[NSBundle mainBundle] executablePath];
  serviceDict =
    [NSDictionary dictionaryWithObjectsAndKeys:
		    execPath, @"NSExecutable",
		    languages, @"NSLanguages",
		    @"GNU", @"NSSpellChecker",
		    nil];
  infoDict =
    [NSDictionary dictionaryWithObjectsAndKeys:
		    execPath, @"NSExecutable",
		    [NSArray arrayWithObject: serviceDict], @"NSServices",
		    nil];
  if (![infoDict writeToFile: path atomically: YES])
    {
      NSLog(@"cannot save info dictionary to %@", path);
      return NO;
    }
  return YES;
}

- (BOOL) removeBundleAtPath: (NSString *)path
{
  NSFileManager *fm = [NSFileManager defaultManager];

  if (![fm fileExistsAtPath: path])
    {
      return NO;
    }
  if (![fm removeFileAtPath: path handler: nil])
    {
      NSLog(@"cannot remove %@", path);
      return NO;
    }
  return YES;
}

/* The installed services bundle only vends a spelling service for the
   AmericanEnglish language. In order to make other languages available,
   we maintain a bundle in the user's Services directory that vends those
   languages. The bundle shares our server executable through its info
   dictionary. */
- (void) synchronizeLanguages
{
  NSArray *paths;
  NSString *path;
  NSMutableArray *otherLanguages;

  paths =
    NSSearchPathForDirectoriesInDomains (NSLibraryDirectory,
					 NSUserDomainMask,
					 YES);
  path = [paths objectAtIndex:0];
  path = [path stringByAppendingPathComponent: @"Services"];
  path = [path stringByAppendingPathComponent: @"GSspell"];
  path = [path stringByAppendingPathExtension: @"service"];

  otherLanguages = [[[self languages] mutableCopy] autorelease];
  [otherLanguages removeObject: @"AmericanEnglish"];
  [otherLanguages sortUsingSelector: @selector(compare:)];
  if ([otherLanguages count])
    {
      if (![otherLanguages isEqual:
	     [[NSBundle bundleWithPath: path] serviceLanguages]])
	{
	  if ([self createBundleAtPath: path languages: otherLanguages])
	    {
	      [[NSWorkspace sharedWorkspace] findApplications];
	    }
	}
    }
  else
    {
      if ([self removeBundleAtPath: path])
	{
	  [[NSWorkspace sharedWorkspace] findApplications];
	}
    }
}

- (NSRange) spellServer: (NSSpellServer *)sender
findMisspelledWordInString: (NSString *)stringToCheck
	       language: (NSString *)language
	      wordCount: (int *)wordCount
	      countOnly: (BOOL)countOnly
{
  NSRange r = NSMakeRange(0,0);

  if (countOnly)
    {
      NSScanner *inputScanner = [NSScanner scannerWithString: stringToCheck];
      [inputScanner setCharactersToBeSkipped:
		      [NSCharacterSet whitespaceAndNewlineCharacterSet]];      
      while (![inputScanner isAtEnd])
        {
          [inputScanner scanUpToCharactersFromSet:
			  [NSCharacterSet whitespaceAndNewlineCharacterSet]
			intoString: NULL];
          (*wordCount)++;
	}
    }
  else
    {
      NSLog(@"spellServer:findMisspelledWordInString:...  invoked, "
	    @"spell server not configured.");
    }

  return r;
}

- (NSArray *) spellServer: (NSSpellServer *)sender
    suggestGuessesForWord: (NSString *)word
	       inLanguage: (NSString *)language
{
  NSMutableArray *array = [NSMutableArray array];

  NSLog(@"spellServer:suggestGuessesForWord:... invoked, "
	@"spell server not configured");
  
  return array;
}

- (void) spellServer: (NSSpellServer *)sender
	didLearnWord: (NSString *)word
	  inLanguage: (NSString *)language
{
  NSLog(@"spellServer:didLearnWord:inLanguage: invoked, "
	@"spell server not configured");
}

- (void) spellServer: (NSSpellServer *)sender
       didForgetWord: (NSString *)word
	  inLanguage: (NSString *)language
{
  NSLog(@"spellServer:didForgetWord:inLanguage: invoked, "
	@"spell server not configured");
}

@end

#ifdef HAVE_ASPELL_H

// The real speller checker class provides spelling services for all
// languages that aspell has dictionaries installed.

#define GNU_SPELL_CHECKER_CLASS GNUAspellSpellChecker
@interface GNUAspellSpellChecker : GNUSpellChecker
{
  NSDictionary *dictionaries;
  NSMutableDictionary *spellers, *documentCheckers;
}
@end

@implementation GNUAspellSpellChecker

static NSDictionary *
aspell_dictionaries()
{
  AspellConfig *config;
  AspellDictInfoList *dictList;
  AspellDictInfoEnumeration *dictEnum;
  NSMutableDictionary *dictionaries;

  config = new_aspell_config();
  dictList = get_aspell_dict_info_list(config);
  delete_aspell_config(config);

  dictionaries = [[NSMutableDictionary alloc] initWithCapacity: 1];
  dictEnum = aspell_dict_info_list_elements(dictList);
  while (!aspell_dict_info_enumeration_at_end(dictEnum))
    {
      const AspellDictInfo *dict = aspell_dict_info_enumeration_next(dictEnum);
      /* The string encoding does not really matter here, since Aspell
	 represents dictionary languages by a two letter ISO 639 language
	 code followed by an optional two letter ISO 3166 country code,
	 all of which are plain ASCII characters.
	 Note that there may be multiple dictionaries for a language,
	 but we are interested only in the supported languages.
	 FIXME How can the user choose a particular dictionary variant
	 from the Spelling panel? */
      NSString *dictLang = [NSString stringWithUTF8String: dict->code];
      NSString *language = GSLanguageFromLocale(dictLang);
      if (!language)
	language = dictLang;
      [dictionaries setObject: dictLang forKey: language];
    }
  delete_aspell_dict_info_enumeration(dictEnum);

  return dictionaries;
}

- (id) init
{
  if (![super init])
    return nil;

  dictionaries = aspell_dictionaries();
  spellers = [[NSMutableDictionary alloc] initWithCapacity: 1];
  documentCheckers = [[NSMutableDictionary alloc] initWithCapacity: 1];

  return self;
}

- (NSArray *) languages
{
  return [dictionaries allKeys];
}

- (AspellSpeller *) spellerForLanguage: (NSString *)language
{
  AspellSpeller *speller = [[spellers objectForKey: language] pointerValue];
  if (!speller)
    {
      NSString *dictLang = [dictionaries objectForKey: language];
      if (dictLang)
	{
	  AspellConfig *config = new_aspell_config();
	  aspell_config_replace(config, "lang", [dictLang UTF8String]);
	  aspell_config_replace(config, "encoding", "UTF-8");
	  speller = to_aspell_speller(new_aspell_speller(config));
	  [spellers setObject: [NSValue valueWithPointer: speller]
		       forKey: language];
	}
    }
  return speller;
}

- (AspellDocumentChecker *) documentCheckerForLanguage: (NSString *)language
{
  AspellDocumentChecker *checker =
    [[documentCheckers objectForKey: language] pointerValue];
  if (!checker)
    {
      AspellSpeller *speller = [self spellerForLanguage: language];
      checker =
	to_aspell_document_checker(new_aspell_document_checker(speller));
      [documentCheckers setObject: [NSValue valueWithPointer: checker]
			   forKey: language];
    }
  return checker;
}

static inline unsigned int
uniLength(unsigned char *buf, unsigned int len)
{
  unsigned int i, size;

  for (i = 0; i < len; i++)
    {
      if (buf[i] >= 0x80)
	{
	  if (GSToUnicode(0, &size, buf, len, NSUTF8StringEncoding, 0, 0))
	    {
	      len = size;
	    }
	  break;
	}
    }
  return len;
}

- (NSRange) spellServer: (NSSpellServer *)sender
findMisspelledWordInString: (NSString *)stringToCheck
	       language: (NSString *)language
	      wordCount: (int *)wordCount
	      countOnly: (BOOL)countOnly
{
  const char *p;
  AspellToken token;
  AspellDocumentChecker *checker;
  NSRange r;
  NSString *word;
  int length;

  if (countOnly)
    {
      return [super spellServer: sender
		    findMisspelledWordInString: stringToCheck
		       language: language
		      wordCount: wordCount
		      countOnly: countOnly];
    }

  p = [stringToCheck UTF8String];
  length = strlen(p);

  checker = [self documentCheckerForLanguage: language];
  aspell_document_checker_process(checker, p, length);

  /* Even though we add learned words to aspell's user dictionary, we must
     ask the server for words in its user dictionaries so that words that
     the user has ignored won't be returned as misspelled. */
  do
    {
      token = aspell_document_checker_next_misspelling(checker);
      if (token.len == 0)
	return NSMakeRange(NSNotFound, 0);

      r = NSMakeRange(uniLength((unsigned char *)p, token.offset),
		      uniLength((unsigned char *)p + token.offset, token.len));
      word = [stringToCheck substringWithRange: r];
    }
  while ([sender isWordInUserDictionaries: word caseSensitive: YES]);

  return r;
}

- (NSArray *) spellServer: (NSSpellServer *)sender
    suggestGuessesForWord: (NSString *)word
	       inLanguage: (NSString *)language
{
  NSMutableArray *array = [NSMutableArray array];

  const char *p = [word UTF8String];
  int len = strlen(p);
  int words = 0;
  AspellSpeller *speller = [self spellerForLanguage: language];
  const struct AspellWordList *list = aspell_speller_suggest(speller, p, len);
  AspellStringEnumeration *en;

  words = aspell_word_list_size(list);
  en = aspell_word_list_elements(list);

  // add them to the array.
  while (!aspell_string_enumeration_at_end(en))
    {
      const char *string = aspell_string_enumeration_next(en);
      NSString *word = [NSString stringWithUTF8String: string];
      [array addObject: word];
    }

  // cleanup.
  delete_aspell_string_enumeration(en);
  
  return array;
}

- (void) spellServer: (NSSpellServer *)sender
	didLearnWord: (NSString *)word
	  inLanguage: (NSString *)language
{
  const char *aword = [word UTF8String];
  AspellSpeller *speller = [self spellerForLanguage: language];
  aspell_speller_add_to_personal(speller, aword, strlen(aword));
}

- (void) spellServer: (NSSpellServer *)sender
       didForgetWord: (NSString *)word
	  inLanguage: (NSString *)language
{
  NSLog(@"Not implemented");
}

@end

#endif

// The main program
#ifndef GNU_SPELL_CHECKER_CLASS
#define GNU_SPELL_CHECKER_CLASS GNUSpellChecker
#endif

#ifdef GNUSTEP
int main(int argc, char** argv, char **env)
#else
int main(int argc, char** argv)
#endif
{
  CREATE_AUTORELEASE_POOL (_pool);
  NSSpellServer *aServer = [[NSSpellServer alloc] init];
  GNUSpellChecker *aSpellChecker = [[GNU_SPELL_CHECKER_CLASS alloc] init];

  NSLog(@"NSLanguages = %@", [aSpellChecker languages]);
  [aSpellChecker synchronizeLanguages];
  if ([aSpellChecker registerLanguagesWithServer: aServer])
    {
      [aServer setDelegate: aSpellChecker];
      NSLog(@"Spell server started and waiting.");
      [aServer run];
      NSLog(@"Unexpected death of spell checker");
    }
  else
    {
      NSLog(@"Cannot create spell checker instance");
    }
  RELEASE(aSpellChecker);
  RELEASE(aServer);
  [_pool drain];
  return 0;
}
