/* Definition of class NSLinguisticTagger
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
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

#ifndef _NSLinguisticTagger_h_GNUSTEP_BASE_INCLUDE
#define _NSLinguisticTagger_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>
#include <Foundation/NSRange.h>
#include <Foundation/NSString.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString, NSArray, NSOrthography;
  
typedef NSString* NSLinguisticTagScheme;

GS_EXPORT NSLinguisticTagScheme const NSLinguisticTagSchemeTokenType;            
GS_EXPORT NSLinguisticTagScheme const NSLinguisticTagSchemeLexicalClass;         
GS_EXPORT NSLinguisticTagScheme const NSLinguisticTagSchemeNameType;             
GS_EXPORT NSLinguisticTagScheme const NSLinguisticTagSchemeNameTypeOrLexicalClass;
GS_EXPORT NSLinguisticTagScheme const NSLinguisticTagSchemeLemma;                 
GS_EXPORT NSLinguisticTagScheme const NSLinguisticTagSchemeLanguage;              
GS_EXPORT NSLinguisticTagScheme const NSLinguisticTagSchemeScript;                

typedef NSString* NSLinguisticTag;

/* Tags for NSLinguisticTagSchemeTokenType */
GS_EXPORT NSLinguisticTag const NSLinguisticTagWord;                          
GS_EXPORT NSLinguisticTag const NSLinguisticTagPunctuation;                   
GS_EXPORT NSLinguisticTag const NSLinguisticTagWhitespace;                    
GS_EXPORT NSLinguisticTag const NSLinguisticTagOther;                         

/* Tags for NSLinguisticTagSchemeLexicalClass */
GS_EXPORT NSLinguisticTag const NSLinguisticTagNoun;
GS_EXPORT NSLinguisticTag const NSLinguisticTagVerb;
GS_EXPORT NSLinguisticTag const NSLinguisticTagAdjective;
GS_EXPORT NSLinguisticTag const NSLinguisticTagAdverb;
GS_EXPORT NSLinguisticTag const NSLinguisticTagPronoun;
GS_EXPORT NSLinguisticTag const NSLinguisticTagDeterminer;
GS_EXPORT NSLinguisticTag const NSLinguisticTagParticle;
GS_EXPORT NSLinguisticTag const NSLinguisticTagPreposition;
GS_EXPORT NSLinguisticTag const NSLinguisticTagNumber;
GS_EXPORT NSLinguisticTag const NSLinguisticTagConjunction;
GS_EXPORT NSLinguisticTag const NSLinguisticTagInterjection;
GS_EXPORT NSLinguisticTag const NSLinguisticTagClassifier;
GS_EXPORT NSLinguisticTag const NSLinguisticTagIdiom;
GS_EXPORT NSLinguisticTag const NSLinguisticTagOtherWord;
GS_EXPORT NSLinguisticTag const NSLinguisticTagSentenceTerminator;
GS_EXPORT NSLinguisticTag const NSLinguisticTagOpenQuote;
GS_EXPORT NSLinguisticTag const NSLinguisticTagCloseQuote;
GS_EXPORT NSLinguisticTag const NSLinguisticTagOpenParenthesis;
GS_EXPORT NSLinguisticTag const NSLinguisticTagCloseParenthesis;
GS_EXPORT NSLinguisticTag const NSLinguisticTagWordJoiner;
GS_EXPORT NSLinguisticTag const NSLinguisticTagDash;
GS_EXPORT NSLinguisticTag const NSLinguisticTagOtherPunctuation;
GS_EXPORT NSLinguisticTag const NSLinguisticTagParagraphBreak;
GS_EXPORT NSLinguisticTag const NSLinguisticTagOtherWhitespace;

/* Tags for NSLinguisticTagSchemeNameType */
GS_EXPORT NSLinguisticTag const NSLinguisticTagPersonalName;
GS_EXPORT NSLinguisticTag const NSLinguisticTagPlaceName;
GS_EXPORT NSLinguisticTag const NSLinguisticTagOrganizationName;

enum
{
    NSLinguisticTaggerUnitWord,        
    NSLinguisticTaggerUnitSentence,    
    NSLinguisticTaggerUnitParagraph,   
    NSLinguisticTaggerUnitDocument     
};
typedef NSUInteger NSLinguisticTaggerUnit;

enum
{ 
    NSLinguisticTaggerOmitWords         = 1 << 0, 
    NSLinguisticTaggerOmitPunctuation   = 1 << 1, 
    NSLinguisticTaggerOmitWhitespace    = 1 << 2, 
    NSLinguisticTaggerOmitOther         = 1 << 3, 
    NSLinguisticTaggerJoinNames         = 1 << 4  
};
typedef NSUInteger NSLinguisticTaggerOptions;

DEFINE_BLOCK_TYPE(GSLinguisticTagRangeBoolBlock, void, NSLinguisticTag, NSRange, BOOL);
DEFINE_BLOCK_TYPE(GSLinguisticTagRangeRangeBoolBlock, void, NSLinguisticTag, NSRange, NSRange, BOOL);

GS_EXPORT_CLASS
@interface NSLinguisticTagger : NSObject
{
  NSArray *_schemes;
  NSString *_string;
  NSString *_dominantLanguage;
  NSArray *_tokenArray;
  NSArray *_orthographyArray;
  NSUInteger _options;
}
  
- (instancetype) initWithTagSchemes: (NSArray *)tagSchemes
                            options: (NSUInteger)opts;

- (NSArray *) tagSchemes;

- (NSString *) string;
- (void) setString: (NSString *)string;
  
+ (NSArray *) availableTagSchemesForUnit: (NSLinguisticTaggerUnit)unit
                                language: (NSString *)language;

+ (NSArray *) availableTagSchemesForLanguage: (NSString *)language;

- (void) setOrthography: (NSOrthography *)orthography
                  range: (NSRange)range;
  
- (NSOrthography *) orthographyAtIndex: (NSUInteger)charIndex
                        effectiveRange: (NSRangePointer)effectiveRange; 

- (void) stringEditedInRange: (NSRange)newRange
              changeInLength: (NSInteger)delta;
  
- (NSRange) tokenRangeAtIndex: (NSUInteger)charIndex
                         unit: (NSLinguisticTaggerUnit)unit;
  
- (NSRange) sentenceRangeForRange: (NSRange)range;

- (void) enumerateTagsInRange: (NSRange)range
                         unit: (NSLinguisticTaggerUnit)unit
                       scheme: (NSLinguisticTagScheme)scheme
                      options: (NSLinguisticTaggerOptions)options
                   usingBlock: (GSLinguisticTagRangeBoolBlock)block;
  
- (NSLinguisticTag) tagAtIndex: (NSUInteger)charIndex
                          unit: (NSLinguisticTaggerUnit)unit
                        scheme: (NSLinguisticTagScheme)scheme
                    tokenRange: (NSRangePointer)tokenRange;

- (NSArray *) tagsInRange: (NSRange)range
                     unit: (NSLinguisticTaggerUnit)unit
                   scheme: (NSLinguisticTagScheme)scheme
                  options: (NSLinguisticTaggerOptions)options
              tokenRanges: (NSArray **)tokenRanges;

- (void) enumerateTagsInRange: (NSRange)range
                       scheme: (NSLinguisticTagScheme)tagScheme
                      options: (NSLinguisticTaggerOptions)opts
                   usingBlock: (GSLinguisticTagRangeRangeBoolBlock)block;

- (NSLinguisticTag) tagAtIndex: (NSUInteger)charIndex
                        scheme: (NSLinguisticTagScheme)scheme
                    tokenRange: (NSRangePointer)tokenRange
                 sentenceRange: (NSRangePointer)sentenceRange;
  
- (NSArray *) tagsInRange: (NSRange)range
                   scheme: (NSString *)tagScheme
                  options: (NSLinguisticTaggerOptions)opts
              tokenRanges: (NSArray **)tokenRanges;

- (NSString *) dominantLanguage;

+ (NSString *) dominantLanguageForString: (NSString *)string;

+ (NSLinguisticTag) tagForString: (NSString *)string
                         atIndex: (NSUInteger)charIndex
                            unit: (NSLinguisticTaggerUnit)unit
                          scheme: (NSLinguisticTagScheme)scheme
                     orthography: (NSOrthography *)orthography
                      tokenRange: (NSRangePointer)tokenRange;
  
+ (NSArray *)tagsForString: (NSString *)string
                     range: (NSRange)range
                      unit: (NSLinguisticTaggerUnit)unit
                    scheme: (NSLinguisticTagScheme)scheme
                   options: (NSLinguisticTaggerOptions)options
               orthography: (NSOrthography *)orthography
               tokenRanges: (NSArray **)tokenRanges;

+ (void) enumerateTagsForString: (NSString *)string
                          range: (NSRange)range
                           unit: (NSLinguisticTaggerUnit)unit
                         scheme: (NSLinguisticTagScheme)scheme
                        options: (NSLinguisticTaggerOptions)options
                    orthography: (NSOrthography *)orthography
                     usingBlock: (GSLinguisticTagRangeBoolBlock)block;
  

- (NSArray *) possibleTagsAtIndex: (NSUInteger)charIndex
                           scheme: (NSString *)tagScheme
                       tokenRange: (NSRangePointer)tokenRange
                    sentenceRange: (NSRangePointer)sentenceRange
                           scores: (NSArray **)scores;

@end

@interface NSString (NSLinguisticAnalysis)

- (NSArray *) linguisticTagsInRange: (NSRange)range
                             scheme: (NSLinguisticTagScheme)scheme
                            options: (NSLinguisticTaggerOptions)options
                        orthography: (NSOrthography *)orthography
                        tokenRanges: (NSArray **)tokenRanges;

- (void) enumerateLinguisticTagsInRange: (NSRange)range
                                 scheme: (NSLinguisticTagScheme)scheme
                                options: (NSLinguisticTaggerOptions)options
                            orthography: (NSOrthography *)orthography
                             usingBlock: (GSLinguisticTagRangeRangeBoolBlock)block;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSLinguisticTagger_h_GNUSTEP_BASE_INCLUDE */

