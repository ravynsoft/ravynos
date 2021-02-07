/* Implementation of class NSLinguisticTagger
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: heron
   Date: Sat Nov  2 21:37:50 EDT 2019

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#include <Foundation/NSLinguisticTagger.h>
#include <Foundation/NSRange.h>
#include <Foundation/NSString.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSOrthography.h>

NSLinguisticTagScheme const NSLinguisticTagSchemeTokenType = @"NSLinguisticTagSchemeTokenType";
NSLinguisticTagScheme const NSLinguisticTagSchemeLexicalClass = @"NSLinguisticTagSchemeLexicalClass";
NSLinguisticTagScheme const NSLinguisticTagSchemeNameType = @"NSLinguisticTagSchemeNameType";
NSLinguisticTagScheme const NSLinguisticTagSchemeNameTypeOrLexicalClass = @"NSLinguisticTagSchemeNameTypeOrLexicalClass";
NSLinguisticTagScheme const NSLinguisticTagSchemeLemma = @"NSLinguisticTagSchemeLemma";
NSLinguisticTagScheme const NSLinguisticTagSchemeLanguage = @"NSLinguisticTagSchemeLanguage";
NSLinguisticTagScheme const NSLinguisticTagSchemeScript = @"NSLinguisticTagSchemeScript";

/* Tags for NSLinguisticTagSchemeTokenType */
NSLinguisticTag const NSLinguisticTagWord = @"NSLinguisticTagWord";                          
NSLinguisticTag const NSLinguisticTagPunctuation = @"NSLinguisticTagPunctuation";                   
NSLinguisticTag const NSLinguisticTagWhitespace = @"NSLinguisticTagWhitespae";                    
NSLinguisticTag const NSLinguisticTagOther = @"NSLinguisticTagOther";

/* Tags for NSLinguisticTagSchemeLexicalClass */
NSLinguisticTag const NSLinguisticTagNoun = @"NSLinguisticTagNoun";
NSLinguisticTag const NSLinguisticTagVerb = @"NSLinguisticTagVerb";  
NSLinguisticTag const NSLinguisticTagAdjective = @"NSLinguisticTagAdjective";  
NSLinguisticTag const NSLinguisticTagAdverb  = @"NSLinguisticTagAdverb";  
NSLinguisticTag const NSLinguisticTagPronoun = @"NSLinguisticTagPronoun";  
NSLinguisticTag const NSLinguisticTagDeterminer  = @"NSLinguisticTagDeterminer";  
NSLinguisticTag const NSLinguisticTagParticle  = @"NSLinguisticTagParticle";  
NSLinguisticTag const NSLinguisticTagPreposition  = @"NSLinguisticTagPrepostion";  
NSLinguisticTag const NSLinguisticTagNumber  = @"NSLinguisticTagNumber";  
NSLinguisticTag const NSLinguisticTagConjunction  = @"NSLinguisticTagConjunction";  
NSLinguisticTag const NSLinguisticTagInterjection  = @"NSLinguisticTagInterjection";  
NSLinguisticTag const NSLinguisticTagClassifier  = @"NSLinguisticTagClassifier";  
NSLinguisticTag const NSLinguisticTagIdiom = @"NSLinguisticTagIdiom";  
NSLinguisticTag const NSLinguisticTagOtherWord = @"NSLinguisticTagOtherWord";  
NSLinguisticTag const NSLinguisticTagSentenceTerminator = @"NSLinguisticTagSentenceTerminator";  
NSLinguisticTag const NSLinguisticTagOpenQuote = @"NSLinguisticTagOpenQuote";  
NSLinguisticTag const NSLinguisticTagCloseQuote = @"NSLinguisticTagCloseQuote";  
NSLinguisticTag const NSLinguisticTagOpenParenthesis = @"NSLinguisticTagOpenParenthesis";  
NSLinguisticTag const NSLinguisticTagCloseParenthesis = @"NSLinguisticTagCloseParenthesis";  
NSLinguisticTag const NSLinguisticTagWordJoiner = @"NSLinguisticTagWordJoiner";  
NSLinguisticTag const NSLinguisticTagDash = @"NSLinguisticTagDash";  
NSLinguisticTag const NSLinguisticTagOtherPunctuation = @"NSLinguisticTagOtherPunctuation";  
NSLinguisticTag const NSLinguisticTagParagraphBreak = @"NSLinguisticTagParagraphBreak";  
NSLinguisticTag const NSLinguisticTagOtherWhitespace = @"NSLinguisticTagOtherWhitespace";  

/* Tags for NSLinguisticTagSchemeNameType */
NSLinguisticTag const NSLinguisticTagPersonalName = @"NSLinguisticTagPersonalName";  
NSLinguisticTag const NSLinguisticTagPlaceName = @"NSLinguisticTagPlaceName";  
NSLinguisticTag const NSLinguisticTagOrganizationName = @"NSLinguisticTagOrganizationName";  

@implementation NSLinguisticTagger

- (instancetype) initWithTagSchemes: (NSArray *)tagSchemes
                            options: (NSUInteger)opts
{
  self = [super init];
  if(self != nil)
    {
      ASSIGNCOPY(_schemes, tagSchemes);
      _options = opts;
      _string = nil;
      _dominantLanguage = nil;
      _tokenArray = nil;
      _orthographyArray = nil;
    }
  return self; 
}

- (void) dealloc
{
  RELEASE(_schemes);
  RELEASE(_string);
  RELEASE(_dominantLanguage);
  RELEASE(_tokenArray);
  RELEASE(_orthographyArray);
  [super dealloc];
}

- (NSArray *) tagSchemes
{
  return _schemes;
}

- (NSString *) string
{
  return _string;
}

- (void) setString: (NSString *)string
{
  ASSIGNCOPY(_string, string);
}
  
+ (NSArray *) availableTagSchemesForUnit: (NSLinguisticTaggerUnit)unit
                                language: (NSString *)language
{
  return nil;
}

+ (NSArray *) availableTagSchemesForLanguage: (NSString *)language
{
  return nil;
}

- (void) setOrthography: (NSOrthography *)orthography
                  range: (NSRange)range
{
}
  
- (NSOrthography *) orthographyAtIndex: (NSUInteger)charIndex
                        effectiveRange: (NSRangePointer)effectiveRange
{
  return nil;
}

- (void) stringEditedInRange: (NSRange)newRange
              changeInLength: (NSInteger)delta
{
}
  
- (NSRange) tokenRangeAtIndex: (NSUInteger)charIndex
                         unit: (NSLinguisticTaggerUnit)unit
{
  return NSMakeRange(0,0);
}
  
- (NSRange) sentenceRangeForRange: (NSRange)range
{
  return NSMakeRange(0,0);
}

- (void) enumerateTagsInRange: (NSRange)range
                         unit: (NSLinguisticTaggerUnit)unit
                       scheme: (NSLinguisticTagScheme)scheme
                      options: (NSLinguisticTaggerOptions)options
                   usingBlock: (GSLinguisticTagRangeBoolBlock)block
{
}

- (NSLinguisticTag) tagAtIndex: (NSUInteger)charIndex
                          unit: (NSLinguisticTaggerUnit)unit
                        scheme: (NSLinguisticTagScheme)scheme
                    tokenRange: (NSRangePointer)tokenRange
{
  return nil;
}

- (NSArray *) tagsInRange: (NSRange)range
                     unit: (NSLinguisticTaggerUnit)unit
                   scheme: (NSLinguisticTagScheme)scheme
                  options: (NSLinguisticTaggerOptions)options
              tokenRanges: (NSArray **)tokenRanges
{
  return nil;
}

- (void) enumerateTagsInRange: (NSRange)range
                       scheme: (NSLinguisticTagScheme)tagScheme
                      options: (NSLinguisticTaggerOptions)opts
                   usingBlock: (GSLinguisticTagRangeRangeBoolBlock)block
{
}

- (NSLinguisticTag) tagAtIndex: (NSUInteger)charIndex
                        scheme: (NSLinguisticTagScheme)scheme
                    tokenRange: (NSRangePointer)tokenRange
                 sentenceRange: (NSRangePointer)sentenceRange
{
  return nil;
}
  
- (NSArray *) tagsInRange: (NSRange)range
                   scheme: (NSString *)tagScheme
                  options: (NSLinguisticTaggerOptions)opts
              tokenRanges: (NSArray **)tokenRanges
{
  return nil;
}

- (NSString *) dominantLanguage
{
  return nil;
}

+ (NSString *) dominantLanguageForString: (NSString *)string
{
  return nil;
}

+ (NSLinguisticTag) tagForString: (NSString *)string
                         atIndex: (NSUInteger)charIndex
                            unit: (NSLinguisticTaggerUnit)unit
                          scheme: (NSLinguisticTagScheme)scheme
                     orthography: (NSOrthography *)orthography
                      tokenRange: (NSRangePointer)tokenRange
{
  return nil;
}
  
+ (NSArray *)tagsForString: (NSString *)string
                     range: (NSRange)range
                      unit: (NSLinguisticTaggerUnit)unit
                    scheme: (NSLinguisticTagScheme)scheme
                   options: (NSLinguisticTaggerOptions)options
               orthography: (NSOrthography *)orthography
               tokenRanges: (NSArray **)tokenRanges
{
  return nil;
}

+ (void) enumerateTagsForString: (NSString *)string
                          range: (NSRange)range
                           unit: (NSLinguisticTaggerUnit)unit
                         scheme: (NSLinguisticTagScheme)scheme
                        options: (NSLinguisticTaggerOptions)options
                    orthography: (NSOrthography *)orthography
                     usingBlock: (GSLinguisticTagRangeBoolBlock)block
{
}
  

- (NSArray *) possibleTagsAtIndex: (NSUInteger)charIndex
                           scheme: (NSString *)tagScheme
                       tokenRange: (NSRangePointer)tokenRange
                    sentenceRange: (NSRangePointer)sentenceRange
                           scores: (NSArray **)scores
{
  return nil;
}
@end

