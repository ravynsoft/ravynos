/* Copyright (c) 2011 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSTextCheckingResult.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitExport.h>

@class NSView, NSMenu, NSViewController, NSPanel, NSMutableDictionary, NSMutableSet;

APPKIT_EXPORT NSString *const NSSpellCheckerDidChangeAutomaticTextReplacementNotification;
APPKIT_EXPORT NSString *const NSSpellCheckerDidChangeAutomaticSpellingCorrectionNotification;

APPKIT_EXPORT NSString *const NSTextCheckingOrthographyKey;
APPKIT_EXPORT NSString *const NSTextCheckingQuotesKey;
APPKIT_EXPORT NSString *const NSTextCheckingReplacementsKey;
APPKIT_EXPORT NSString *const NSTextCheckingReferenceDateKey;
APPKIT_EXPORT NSString *const NSTextCheckingReferenceTimeZoneKey;
APPKIT_EXPORT NSString *const NSTextCheckingDocumentURLKey;
APPKIT_EXPORT NSString *const NSTextCheckingDocumentTitleKey;
APPKIT_EXPORT NSString *const NSTextCheckingDocumentAuthorKey;

enum {
    NSCorrectionIndicatorTypeDefault = 0,
    NSCorrectionIndicatorTypeReversion,
    NSCorrectionIndicatorTypeGuesses,
};
typedef NSInteger NSCorrectionIndicatorType;

enum {
    NSCorrectionResponseNone,
    NSCorrectionResponseAccepted,
    NSCorrectionResponseRejected,
    NSCorrectionResponseIgnored,
    NSCorrectionResponseEdited,
    NSCorrectionResponseReverted,
};
typedef NSInteger NSCorrectionResponse;

@interface NSSpellChecker : NSObject {
    NSPanel *_spellingPanel;
    NSViewController *_spellingViewController;
    NSView *_accessoryView;
    NSPanel *_substitutionsPanel;
    NSMutableDictionary *_tagToData;
    NSMutableSet *_learnedWords;
    NSString *_language;
    BOOL _automaticallyIdentifiesLanguages;
}

#pragma mark -
#pragma mark Getting the Spell Checker

+ (NSSpellChecker *)sharedSpellChecker;
+ (BOOL)sharedSpellCheckerExists;

#pragma mark -
#pragma mark Configuring Spell Checkers Languages

- (NSArray *)availableLanguages;
- (NSArray *)userPreferredLanguages;
- (BOOL)automaticallyIdentifiesLanguages;
- (void)setAutomaticallyIdentifiesLanguages:(BOOL)flag;
- (NSString *)language;
- (BOOL)setLanguage:(NSString *)language;

#pragma mark -
#pragma mark Managing Panels

- (NSPanel *)spellingPanel;
- (NSPanel *)substitutionsPanel;
- (void)updateSpellingPanelWithGrammarString:(NSString *)problemString detail:(NSDictionary *)detail;
- (void)updatePanels;
- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)view;
- (NSViewController *)substitutionsPanelAccessoryViewController;
- (void)setSubstitutionsPanelAccessoryViewController:(NSViewController *)viewController;

#pragma mark -
#pragma mark Checking Strings for Spelling and Grammar

- (NSInteger)countWordsInString:(NSString *)string language:(NSString *)language;
- (NSRange)checkSpellingOfString:(NSString *)string startingAt:(NSInteger)offset;
- (NSRange)checkSpellingOfString:(NSString *)string startingAt:(NSInteger)offset language:(NSString *)language wrap:(BOOL)wrap inSpellDocumentWithTag:(NSInteger)tag wordCount:(NSInteger *)wordCount;
- (NSRange)checkGrammarOfString:(NSString *)string startingAt:(NSInteger)start language:(NSString *)language wrap:(BOOL)wrap inSpellDocumentWithTag:(NSInteger)documentTag details:(NSArray **)outDetails;
- (NSArray *)checkString:(NSString *)string range:(NSRange)range types:(NSTextCheckingTypes)types options:(NSDictionary *)options inSpellDocumentWithTag:(NSInteger)tag orthography:(NSOrthography **)orthography wordCount:(NSInteger *)wordCount;

#ifdef NS_BLOCKS
- (NSInteger)requestCheckingOfString:(NSString *)stringToCheck range:(NSRange)range types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options inSpellDocumentWithTag:(NSInteger)tag completionHandler:(void (^)(NSInteger sequenceNumber, NSArray *results, NSOrthography *orthography, NSInteger wordCount))completionHandler;
#endif

- (NSArray *)guessesForWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;

#pragma mark -
#pragma mark Managing the Spell-Checking Process

+ (NSInteger)uniqueSpellDocumentTag;
- (void)closeSpellDocumentWithTag:(NSInteger)tag;
- (void)ignoreWord:(NSString *)word inSpellDocumentWithTag:(NSInteger)tag;
- (void)setIgnoredWords:(NSArray *)ignoredWords inSpellDocumentWithTag:(NSInteger)tag;
- (NSArray *)ignoredWordsInSpellDocumentWithTag:(NSInteger)tag;
- (void)setWordFieldStringValue:(NSString *)string;
- (void)updateSpellingPanelWithMisspelledWord:(NSString *)word;
- (NSArray *)completionsForPartialWordRange:(NSRange)partialWordRange inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;
- (BOOL)hasLearnedWord:(NSString *)word;
- (void)unlearnWord:(NSString *)word;
- (void)learnWord:(NSString *)word;
- (NSArray *)userQuotesArrayForLanguage:(NSString *)language;
- (NSDictionary *)userReplacementsDictionary;

#pragma mark -
#pragma mark Data Detector Interaction

- (NSMenu *)menuForResult:(NSTextCheckingResult *)result string:(NSString *)checkedString options:(NSDictionary *)options atLocation:(NSPoint)location inView:(NSView *)view;

#pragma mark -
#pragma mark Automatic Spelling Correction

- (NSString *)correctionForWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;
+ (BOOL)isAutomaticSpellingCorrectionEnabled;

#ifdef NS_BLOCKS
- (void)showCorrectionIndicatorOfType:(NSCorrectionIndicatorType)type primaryString:(NSString *)primaryString alternativeStrings:(NSArray *)alternativeStrings forStringInRect:(NSRect)rect view:(NSView *)view completionHandler:(void (^)(NSString *acceptedString))completionBlock;
#endif

- (void)dismissCorrectionIndicatorForView:(NSView *)view;
- (void)recordResponse:(NSCorrectionResponse)response toCorrection:(NSString *)correction forWord:(NSString *)word language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag;

#pragma mark -
#pragma mark Automatic Text Replacement

+ (BOOL)isAutomaticTextReplacementEnabled;

@end
