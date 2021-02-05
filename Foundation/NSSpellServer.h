#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSTextCheckingResult.h>

@class NSSpellServer, NSArray, NSDictionary, NSOrthography;

FOUNDATION_EXPORT NSString *const NSGrammarRange;
FOUNDATION_EXPORT NSString *const NSGrammarUserDescription;
FOUNDATION_EXPORT NSString *const NSGrammarCorrections;

@protocol NSSpellServerDelegate
//@optional

- (NSRange)spellServer:(NSSpellServer *)sender checkGrammarInString:(NSString *)string language:(NSString *)language details:(NSArray **)outDetails;

- (NSArray *)spellServer:(NSSpellServer *)sender checkString:(NSString *)stringToCheck offset:(NSUInteger)offset types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options orthography:(NSOrthography *)orthography wordCount:(NSInteger *)wordCount;

- (void)spellServer:(NSSpellServer *)sender didForgetWord:(NSString *)word inLanguage:(NSString *)language;

- (void)spellServer:(NSSpellServer *)sender didLearnWord:(NSString *)word inLanguage:(NSString *)language;

- (NSRange)spellServer:(NSSpellServer *)sender findMisspelledWordInString:(NSString *)stringToCheck language:(NSString *)language wordCount:(NSInteger *)wordCount countOnly:(BOOL)countOnly;

- (NSArray *)spellServer:(NSSpellServer *)sender suggestCompletionsForPartialWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language;

- (NSArray *)spellServer:(NSSpellServer *)sender suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language;

@end

@interface NSSpellServer : NSObject {
}

- (id<NSSpellServerDelegate>)delegate;

- (BOOL)isWordInUserDictionaries:(NSString *)word caseSensitive:(BOOL)caseSensitive;

- (BOOL)registerLanguage:(NSString *)language byVendor:(NSString *)vendor;

- (void)run;

- (void)setDelegate:(id<NSSpellServerDelegate>)delegate;

@end
