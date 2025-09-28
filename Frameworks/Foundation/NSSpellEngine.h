//
//  NSSpellEngine.h
//  Foundation
//
//  Created by Christopher Lloyd on 8/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSObject.h>
#import <Foundation/NSSpellServer.h>

/* Private class which implements the delegate behavior of NSSpellServer directly, used for in-process spell checking. */

@interface NSSpellEngine : NSObject <NSSpellServerDelegate>

+ (NSArray *)allSpellEngines;
+ (NSArray *)spellEngines;

- (NSString *)vendor;
- (NSArray *)languages;

- (NSRange)checkGrammarInString:(NSString *)string language:(NSString *)language details:(NSArray **)outDetails;

- (NSArray *)checkString:(NSString *)stringToCheck offset:(NSUInteger)offset types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options orthography:(NSOrthography *)orthography wordCount:(NSInteger *)wordCount;

- (void)didForgetWord:(NSString *)word inLanguage:(NSString *)language;

- (void)didLearnWord:(NSString *)word inLanguage:(NSString *)language;

- (NSRange)findMisspelledWordInString:(NSString *)stringToCheck language:(NSString *)language wordCount:(NSInteger *)wordCount countOnly:(BOOL)countOnly;

- (NSArray *)suggestCompletionsForPartialWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language;

- (NSArray *)suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language;

@end
