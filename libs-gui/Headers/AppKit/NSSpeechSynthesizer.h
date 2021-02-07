/** <title>NSSpeechSynthesizer</title>

   <abstract>abstract base class for speech synthesis</abstract>

   Copyright <copy>(C) 2009 Free Software Foundation, Inc.</copy>

   Author: Gregory Casamento <greg.casamento@gmail.com>
   Date: Mar 2009

   This file is part of the GNUstep GUI Library.

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

#ifndef _GNUstep_H_NSSpeechSynthesizer
#define _GNUstep_H_NSSpeechSynthesizer

#import <Foundation/NSObject.h>

// NSSpeechBoundary enumerated type...
typedef enum 
  {
    NSSpeechImmediateBoundary = 0,
    NSSpeechWordBoundary,
    NSSpeechSentenceBoundary
  }
NSSpeechBoundary;

// forward declarations...
@class NSString, NSMutableDictionary, NSMutableArray, NSError;

// Keys for properties...
extern NSString *NSVoiceIdentifier;
extern NSString *NSVoiceName;
extern NSString *NSVoiceAge;
extern NSString *NSVoiceGender;
extern NSString *NSVoiceDemoText;
extern NSString *NSVoiceLanguage;
extern NSString *NSVoiceLocaleIdentifier;
extern NSString *NSVoiceSupportedCharacters;
extern NSString *NSVoiceIndividuallySpokenCharacters;

// Values for gender
extern NSString *NSVoiceGenderNeuter;
extern NSString *NSVoiceGenderMale;
extern NSString *NSVoiceGenderFemale;

// values for speech mode
extern NSString *NSSpeechModeText;
extern NSString *NSSpeechModePhoneme;
extern NSString *NSSpeechModeNormal;
extern NSString *NSSpeechModeLiteral;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
extern NSString *NSSpeechResetProperty;
extern NSString *NSSpeechOutputToFileURLProperty;
extern NSString *NSSpeechPitchBaseProperty;
#endif

// values for speech status...
extern NSString *NSSpeechStatusOutputBusy;
extern NSString *NSSpeechStatusOutputPaused;
extern NSString *NSSpeechStatusNumberOfCharactersLeft;
extern NSString *NSSpeechStatusPhonemeCode;

// values for error
extern NSString *NSSpeechErrorCount;
extern NSString *NSSpeechErrorOldestCode;
extern NSString *NSSpeechErrorOldestCharacterOffset;
extern NSString *NSSpeechErrorNewestCode;
extern NSString *NSSpeechErrorNewestCharacterOffset;

// values for info
extern NSString *NSSpeechSynthesizerInfoIdentifier;
extern NSString *NSSpeechSynthesizerInfoVersion;

// values for command delimiter
extern NSString *NSSpeechCommandPrefix;
extern NSString *NSSpeechCommandSuffix;

// values for dictionaries.
extern NSString *NSSpeechDictionaryLanguage;
extern NSString *NSSpeechDictionaryModificationDate;
extern NSString *NSSpeechDictionaryPronunciations;
extern NSString *NSSpeechDictionaryAbreviations;
extern NSString *NSSpeechDictionaryEntrySpelling;
extern NSString *NSSpeechDictionaryEntryPhonemes;

// class declaration...
@interface NSSpeechSynthesizer : NSObject

// init...
- (id) initWithVoice: (NSString *)voice;

// configuring speech synthesis
- (BOOL) usesFeebackWindow;
- (void) setUsesFeebackWindow: (BOOL)flag;

- (NSString *) voice;
- (void) setVoice: (NSString *)voice;

- (float) rate;
- (void) setRate: (float)rate;

- (float) volume;
- (void) setVolume: (float)volume;

- (void) addSpeechDictionary: (NSDictionary *)speechDictionary;

- (id) objectForProperty: (NSString *)property error: (NSError **)error;

- (id) setObject: (id) object 
     forProperty: (NSString *)property 
           error: (NSError **)error;

- (id) delegate;
- (void) setDelegate: (id)delegate;

// Getting information...
+ (NSArray *) availableVoices;

+ (NSDictionary *) attributesForVoice: (NSString *)voice;

+ (NSString *) defaultVoice;

// Getting state...
+ (BOOL) isAnyApplicationSpeaking;

// Synthesizing..
- (BOOL) isSpeaking;

- (BOOL) startSpeakingString: (NSString *)text;

- (BOOL) startSpeakingString: (NSString *)text toURL: (NSURL *)url;

- (void) stopSpeaking;

- (void) stopSpeakingAtBoundary: (NSSpeechBoundary)boundary;

- (void) pauseSpeakingAtBoundary: (NSSpeechBoundary)boundary;

- (void) continueSpeaking;

- (NSString *) phonemesFromText: (NSString *)text;
@end

@protocol NSSpeechSynthesizerDelegate <NSObject>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSSpeechSynthesizerDelegate)
#endif
- (void)speechSynthesizer: (NSSpeechSynthesizer *)sender
 didEncounterErrorAtIndex: (NSUInteger)index 
                 ofString: (NSString *)string 
                  message: (NSString *)error;

- (void)speechSynthesizer: (NSSpeechSynthesizer *)sender 
  didEncounterSyncMessage: (NSString *)error;

- (void)speechSynthesizer: (NSSpeechSynthesizer *)sender 
        didFinishSpeaking: (BOOL)success;

- (void)speechSynthesizer: (NSSpeechSynthesizer *)sender 
         willSpeakPhoneme: (short)phoneme;

- (void) speechSynthesizer: (NSSpeechSynthesizer *)sender 
             willSpeakWord: (NSRange)range 
                  ofString: (NSString *)string;
@end

#endif // _GNUstep_H_NSSpeechSynthesizer

