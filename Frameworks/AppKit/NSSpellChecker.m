/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSSpellChecker.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSRaise.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSOrthography.h>
#import <Foundation/NSSpellEngine.h>
#import "NSSpellCheckerTagData.h"
#import "NSSpellingViewController.h"

NSString * const NSSpellCheckerDidChangeAutomaticTextReplacementNotification=@"NSSpellCheckerDidChangeAutomaticTextReplacementNotification";
NSString * const NSSpellCheckerDidChangeAutomaticSpellingCorrectionNotification=@"NSSpellCheckerDidChangeAutomaticSpellingCorrectionNotification";

NSString * const NSTextCheckingOrthographyKey=@"NSTextCheckingOrthographyKey";
NSString * const NSTextCheckingQuotesKey=@"NSTextCheckingQuotesKey";
NSString * const NSTextCheckingReplacementsKey=@"NSTextCheckingReplacementsKey";
NSString * const NSTextCheckingReferenceDateKey=@"NSTextCheckingReferenceDateKey";
NSString * const NSTextCheckingReferenceTimeZoneKey=@"NSTextCheckingReferenceTimeZoneKey";
NSString * const NSTextCheckingDocumentURLKey=@"NSTextCheckingDocumentURLKey";
NSString * const NSTextCheckingDocumentTitleKey=@"NSTextCheckingDocumentTitleKey";
NSString * const NSTextCheckingDocumentAuthorKey=@"NSTextCheckingDocumentAuthorKey";

#define SPELLCHECK_DEBUG 0

@implementation NSSpellChecker

-init
{
	_tagToData=[[NSMutableDictionary alloc] init];
	_learnedWords=[[NSMutableSet alloc] init];
	return self;
}

-(void)dealloc
{
	[_tagToData release];
	[_learnedWords release];
	[_language release];
	[super dealloc];
}

#pragma mark -
#pragma mark Getting the Spell Checker

+(NSSpellEngine *)_currentSpellEngine {
	return [[NSSpellEngine allSpellEngines] objectAtIndex:0];
}

static NSSpellChecker *shared=nil;

+(NSSpellChecker *)sharedSpellChecker
{
	if(shared==nil) {
		shared=[[NSSpellChecker alloc] init];
	}
	
	return shared;
}

+(BOOL)sharedSpellCheckerExists
{
	return (shared != nil) ? YES : NO;
}

#pragma mark -
#pragma mark Configuring Spell Checkers Languages

-(NSString *)currentLanguage
{
	return [[NSLocale currentLocale] localeIdentifier];
}

-(NSArray *)availableLanguages
{
	static NSMutableArray *availableLanguages = nil;
	if (availableLanguages == nil) {
		availableLanguages = [[NSMutableArray alloc] init];
	
		for(NSSpellEngine *engine in [NSSpellEngine allSpellEngines]){
			[availableLanguages addObjectsFromArray:[engine languages]];
		}
		
		[availableLanguages sortUsingSelector:@selector(caseInsensitiveCompare:)];
	}
	
	return availableLanguages;
}

-(NSArray *)userPreferredLanguages
{
	static NSMutableArray *userPreferredLanguages = nil;
	if (userPreferredLanguages == nil) {
		NSMutableArray* userPreferredLanguages = [[NSMutableArray alloc] init];
		NSArray* availableLanguages = [self availableLanguages];
		NSArray* allPreferredLanguages = [NSLocale preferredLanguages];

		for (NSString* language in allPreferredLanguages) {
			if ([availableLanguages containsObject: language]) {
				[userPreferredLanguages addObject: language];
			}
		}
	}
	return userPreferredLanguages;
}

-(BOOL)automaticallyIdentifiesLanguages
{
	return _automaticallyIdentifiesLanguages;
}

-(void)setAutomaticallyIdentifiesLanguages:(BOOL)flag
{
	_automaticallyIdentifiesLanguages = flag;
}

-(NSString *)language
{
	return _language;
}

-(BOOL)setLanguage:(NSString *)language
{
	BOOL inPopup = [[self availableLanguages] containsObject: language];
	if ([[self userPreferredLanguages] containsObject: language]) {
		[_language autorelease];
		_language=[language copy];
	}
	return inPopup;
}

#pragma mark -
#pragma mark Managing Panels

-(NSSpellingViewController *)_spellingViewController
{
	if(_spellingViewController==nil) {
		_spellingViewController=[[NSSpellingViewController alloc] initWithNibName:@"NSSpellingViewController" bundle:[NSBundle bundleForClass:[NSSpellingViewController class]]];
	}
	return (NSSpellingViewController *)_spellingViewController;
}

-(NSPanel *)spellingPanel
{
    if(_spellingPanel==nil){
        
        NSSpellingViewController *vc=[self _spellingViewController];
		
        NSView *view=[vc view];
		NSRect frame = [view frame];
        _spellingPanel=[[NSPanel alloc] initWithContentRect: frame styleMask:NSUtilityWindowMask | NSResizableWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
		
		[_spellingPanel setTitle: NSLocalizedStringFromTableInBundle(@"Spelling", nil, [NSBundle bundleForClass: [NSSpellChecker class]], @"The title of the spelling dialog")];
		[view setFrameOrigin: NSMakePoint(0, 10)];
        [[_spellingPanel contentView] addSubview: view];
		[_spellingPanel setMinSize: frame.size];
        [_spellingPanel center];
    }
	
    return _spellingPanel;
}

-(NSPanel *)substitutionsPanel {
	return _substitutionsPanel;
}

-(void)updateSpellingPanelWithGrammarString:(NSString *)problemString detail:(NSDictionary *)detail {
	NSUnimplementedMethod();
}

-(void)updatePanels {
	NSUnimplementedMethod();
}

-(NSView *)accessoryView {
	return _accessoryView;
}

-(void)setAccessoryView:(NSView *)view {
	view=[view retain];
	[_accessoryView release];
	_accessoryView=view;
	NSUnimplementedMethod();
}

-(NSViewController *)substitutionsPanelAccessoryViewController {
	NSUnimplementedMethod();
	return 0;
}

-(void)setSubstitutionsPanelAccessoryViewController:(NSViewController *)viewController {
	NSUnimplementedMethod();
}

#pragma mark -
#pragma mark Checking Strings for Spelling and Grammar

-(NSInteger)countWordsInString:(NSString *)string language:(NSString *)language {
	NSUnimplementedMethod();
	return 0;
}

-(NSRange)checkSpellingOfString:(NSString *)string startingAt:(NSInteger)offset {
	return [self checkSpellingOfString:string startingAt:offset language:[[NSLocale currentLocale] localeIdentifier] wrap:NO inSpellDocumentWithTag:0 wordCount:NULL];
}

-(NSRange)checkSpellingOfString:(NSString *)string startingAt:(NSInteger)offset language:(NSString *)language wrap:(BOOL)wrap inSpellDocumentWithTag:(NSInteger)tag wordCount:(NSInteger *)wordCount {
    
	NSMutableDictionary *options=[NSMutableDictionary dictionary];
	
	if(language==nil){
		language=[self currentLanguage];
	}
	
	if(language!=nil){
		NSDictionary  *languageMap=[NSDictionary dictionaryWithObject:[NSArray arrayWithObject:language] forKey:@"Latn"];
		NSOrthography *orthography=[NSOrthography orthographyWithDominantScript:@"Latn" languageMap:languageMap];
		
		[options setObject:orthography forKey:NSTextCheckingOrthographyKey];
	}

	NSArray *checking=[self checkString:string range:NSMakeRange(offset,[string length]-offset) types:NSTextCheckingTypeSpelling options:options inSpellDocumentWithTag:tag orthography:NULL wordCount:wordCount];
    
	if([checking count]==0)
		return NSMakeRange(0,0);
    
	NSTextCheckingResult *first=[checking objectAtIndex:0];
	
	return [first range];
}

-(NSRange)checkGrammarOfString:(NSString *)string startingAt:(NSInteger)start language:(NSString *)language wrap:(BOOL)wrap inSpellDocumentWithTag:(NSInteger)documentTag details:(NSArray **)outDetails {
	NSUnimplementedMethod();
	return NSMakeRange(0,0);
}

-(NSArray *)checkString:(NSString *)string range:(NSRange)range types:(NSTextCheckingTypes)types options:(NSDictionary *)options inSpellDocumentWithTag:(NSInteger)tag orthography:(NSOrthography **)orthography wordCount:(NSInteger *)wordCount {
	NSSpellEngine *spellEngine=[[self class] _currentSpellEngine];
	
	/* NSSpellChecker and NSSpellServer have inconsistent API, we accept a range but the server only takes an offset. */
	/* NSSpellChecker returns by ref an orthography, yet NSSpellServer accepts one as argument. */
	/* I guess this isn't one to one and there is some extra work being done in NSSpellChecker. */

#if SPELLCHECK_DEBUG
    NSLog(@"checkString: %@ range: %@ types: %d options: %@", string, NSStringFromRange(range), types, options);
#define DEBUG_CHECKSTRING 0
#endif
    
	NSString *substring=[string substringToIndex:NSMaxRange(range)];

	NSArray *results = [spellEngine checkString:substring offset:range.location types:types options:options orthography:[options objectForKey:NSTextCheckingOrthographyKey] wordCount:wordCount];

#if DEBUG_CHECKSTRING
    NSLog(@"    substring: %@", substring);
    NSLog(@"    results: %@", results);
    if (wordCount) {
        NSLog(@"    wordCount: %d", *wordCount);
    }
#endif
    
    return results;
}

#ifdef NS_BLOCKS
-(NSInteger)requestCheckingOfString:(NSString *)stringToCheck range:(NSRange)range types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options inSpellDocumentWithTag:(NSInteger)tag completionHandler:(void (^)(NSInteger sequenceNumber, NSArray *results, NSOrthography *orthography, NSInteger wordCount))completionHandler {
	NSUnimplementedMethod();
	return 0;
}
#endif

-(NSArray *)guessesForWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag {
    NSSpellEngine *spellEngine=[[self class] _currentSpellEngine];
	
    NSString *word = [string substringWithRange: range];
    
    return [spellEngine suggestGuessesForWord: word inLanguage: language];
}

#pragma mark -
#pragma mark Managing the Spell-Checking Process

+(NSInteger)uniqueSpellDocumentTag {
	/* These start at 1, don't change */
	static NSInteger tag=1;
    
	return tag++;
}

-(NSSpellCheckerTagData *)_dataForDocumentTag:(NSInteger)tagInt {
	NSNumber *tag=[NSNumber numberWithInteger:tagInt];
	NSSpellCheckerTagData *result=[_tagToData objectForKey:tag];
	
	if(result==nil){
		result=[[[NSSpellCheckerTagData alloc] init] autorelease];
		[_tagToData setObject:result forKey:tag];
	}
	
	return result;
}

-(void)closeSpellDocumentWithTag:(NSInteger)tag { 
	NSUnimplementedMethod();
}

-(void)ignoreWord:(NSString *)word inSpellDocumentWithTag:(NSInteger)tag {
	[[self _dataForDocumentTag:tag] ignoreWord:word];
}

-(NSArray *)ignoredWordsInSpellDocumentWithTag:(NSInteger)tag {
	return [[self _dataForDocumentTag:tag] ignoredWords];
}

-(void)setIgnoredWords:(NSArray *)ignoredWords inSpellDocumentWithTag:(NSInteger)tag {
	[[self _dataForDocumentTag:tag] setIgnoredWords:ignoredWords];
}

-(void)setWordFieldStringValue:(NSString *)string {
	NSUnimplementedMethod();
}

-(void)updateSpellingPanelWithMisspelledWord:(NSString *)word
{
    NSSpellingViewController *vc=[self _spellingViewController];
    
    [vc updateSpellingPanelWithMisspelledWord:word];
	
	// From the docs: If checkSpellingOfString:startingAt: does not find a misspelled word,
	// you should call updateSpellingPanelWithMisspelledWord: with the empty string.
	// This causes the system to beep, letting the user know that the spell check is complete
	// and no misspelled words were found.
	
	if ([word isEqualToString: @""]) {
		NSBeep();
	}
}

-(NSArray *)completionsForPartialWordRange:(NSRange)partialWordRange inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag {
	NSUnimplementedMethod();
	return 0;
}

-(BOOL)hasLearnedWord:(NSString *)word {
	return [_learnedWords containsObject:word];
}

-(void)unlearnWord:(NSString *)word {
	NSUnimplementedMethod();
}

-(void)learnWord:(NSString *)word {
	[_learnedWords addObject:[[word copy] autorelease]];
}

-(NSArray *)userQuotesArrayForLanguage:(NSString *)language {
	NSUnimplementedMethod();
	return 0;
}

-(NSDictionary *)userReplacementsDictionary {
	NSUnimplementedMethod();
	return 0;
}

#pragma mark -
#pragma mark Data Detector Interaction

-(NSMenu *)menuForResult:(NSTextCheckingResult *)checkingResult string:(NSString *)checkedString options:(NSDictionary *)options atLocation:(NSPoint)location inView:(NSView *)view {
	NSSpellEngine *engine=[[self class] _currentSpellEngine];
	
	NSMenu *result=[[NSMenu alloc] initWithTitle:@""];
    
	NSRange range=[checkingResult range];
	NSString *word=[checkedString substringWithRange:range];
    
	NSArray *guesses=[engine suggestGuessesForWord:word inLanguage:[self currentLanguage]];
    
	if([guesses count]==0){
		NSMenuItem *item=[result addItemWithTitle: NSLocalizedStringFromTableInBundle(@"< No Suggestions >", nil, [NSBundle bundleForClass: [NSSpellChecker class]], @"Shown when there are no selections from the spell checker") action:NULL keyEquivalent:@""];
		[item setEnabled:NO];
	}
	else {
		for(NSString *guess in guesses)
			[result addItemWithTitle:guess action:@selector(changeSpelling:) keyEquivalent:@""];
	}
	
	[result addItem:[NSMenuItem separatorItem]];
	
	return result;
}

#pragma mark -
#pragma mark Automatic Spelling Correction

+(BOOL)isAutomaticSpellingCorrectionEnabled {
	NSUnimplementedMethod();
	return 0;
}

#ifdef NS_BLOCKS
-(void)showCorrectionIndicatorOfType:(NSCorrectionIndicatorType)type primaryString:(NSString *)primaryString alternativeStrings:(NSArray *)alternativeStrings forStringInRect:(NSRect)rect view:(NSView *)view completionHandler:(void (^)(NSString *acceptedString))completionBlock {
	NSUnimplementedMethod();
}
#endif

-(NSString *)correctionForWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language inSpellDocumentWithTag:(NSInteger)tag {
	NSUnimplementedMethod();
	return 0;
}

-(void)recordResponse:(NSCorrectionResponse)response toCorrection:(NSString *)correction forWord:(NSString *)word language :(NSString *)language inSpellDocumentWithTag :(NSInteger)tag {
	NSUnimplementedMethod();
}

-(void)dismissCorrectionIndicatorForView:(NSView *)view {
	NSUnimplementedMethod();
}

#pragma mark -
#pragma mark Automatic Text Replacement

+(BOOL)isAutomaticTextReplacementEnabled {
   NSUnimplementedMethod();
   return 0;
}

@end
