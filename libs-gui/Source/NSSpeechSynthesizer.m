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

#import <Foundation/NSObject.h>
#import <Foundation/NSDistantObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSError.h>
#import <Foundation/NSConnection.h>
#import "AppKit/NSWorkspace.h"
#import "AppKit/NSSpeechSynthesizer.h"

// Keys for properties...
NSString *NSVoiceIdentifier = @"NSVoiceIdentifier";
NSString *NSVoiceName = @"NSVoiceName";
NSString *NSVoiceAge = @"NSVoiceAge";
NSString *NSVoiceGender = @"NSVoiceGender";
NSString *NSVoiceDemoText = @"NSVoiceDemoText";
NSString *NSVoiceLanguage = @"NSVoiceLanguage";
NSString *NSVoiceLocaleIdentifier = @"NSVoiceLocaleIdentifier";
NSString *NSVoiceSupportedCharacters = @"NSVoiceSupportedCharacters";
NSString *NSVoiceIndividuallySpokenCharacters = @"NSVoiceIndividuallySpokenCharacters";

// Values for gender
NSString *NSVoiceGenderNeuter = @"NSVoiceGenderNeuter";
NSString *NSVoiceGenderMale = @"NSVoiceGenderMale";
NSString *NSVoiceGenderFemale = @"NSVoiceGenderFemale";

// values for speech mode
NSString *NSSpeechModeText = @"NSSpeechModeText";
NSString *NSSpeechModePhoneme = @"NSSpeechModePhoneme";
NSString *NSSpeechModeNormal = @"NSSpeechModeNormal";
NSString *NSSpeechModeLiteral = @"NSSpeechModeLiteral";

NSString *NSSpeechResetProperty = @"NSSpeechResetProperty";
NSString *NSSpeechOutputToFileURLProperty = @"NSSpeechOutputToFileURLProperty";
NSString *NSSpeechPitchBaseProperty = @"NSSpeechPitchBaseProperty";

// values for speech status...
NSString *NSSpeechStatusOutputBusy = @"NSSpeechStatusOutputBusy";
NSString *NSSpeechStatusOutputPaused = @"NSSpeechStatusOutputPaused";
NSString *NSSpeechStatusNumberOfCharactersLeft = @"NSSpeechStatusNumberOfCharactersLeft";
NSString *NSSpeechStatusPhonemeCode = @"NSSpeechStatusPhonemeCode";

// values for error
NSString *NSSpeechErrorCount = @"NSSpeechErrorCount";
NSString *NSSpeechErrorOldestCode = @"NSSpeechErrorOldestCode";
NSString *NSSpeechErrorOldestCharacterOffset = @"NSSpeechErrorOldestCharacterOffset";
NSString *NSSpeechErrorNewestCode = @"NSSpeechErrorNewestCode";
NSString *NSSpeechErrorNewestCharacterOffset = @"NSSpeechErrorNewestCharacterOffset";

// values for info
NSString *NSSpeechSynthesizerInfoIdentifier = @"NSSpeechSynthesizerInfoIdentifier";
NSString *NSSpeechSynthesizerInfoVersion = @"NSSpeechSynthesizerInfoVersion";

// values for command delimiter
NSString *NSSpeechCommandPrefix = @"NSSpeechCommandPrefix";
NSString *NSSpeechCommandSuffix = @"NSSpeechCommandSuffix";

// values for dictionaries.
NSString *NSSpeechDictionaryLanguage = @"NSSpeechDictionaryLanguage";
NSString *NSSpeechDictionaryModificationDate = @"NSSpeechDictionaryModificationDate";
NSString *NSSpeechDictionaryPronunciations = @"NSSpeechDictionaryPronunciations";
NSString *NSSpeechDictionaryAbreviations = @"NSSpeechDictionaryAbreviations";
NSString *NSSpeechDictionaryEntrySpelling = @"NSSpeechDictionaryEntrySpelling";
NSString *NSSpeechDictionaryEntryPhonemes = @"NSSpeechDictionaryEntryPhonemes";

// Speech daemon
static id server;
// Flag indicating whether we should wait for the daemon to finish launching.
static BOOL serverLaunchTested;
// Class of the NSSpeechSynthesizer
static Class NSSpeechSynthesizerClass;
// Informal protocol used for the server.
@interface NSObject (GSSpeechServer)
- (NSSpeechSynthesizer*)newSynthesizer;
@end

@implementation NSSpeechSynthesizer

- (id) initWithVoice: (NSString *)voice 
{
  return self;
}

+ (void)initialize
{
  NSSpeechSynthesizerClass = [NSSpeechSynthesizer class];
  server = [[NSConnection rootProxyForConnectionWithRegisteredName: @"GSSpeechServer"
                                                              host: nil] retain];
  if (nil == server)
    {
      NSWorkspace *ws = [NSWorkspace sharedWorkspace];
      [ws launchApplication: @"GSSpeechServer"
                   showIcon: NO
                 autolaunch: NO];
    }
}

+ (BOOL)isAnyApplicationSpeaking
{
  return [server isSpeaking];
}

// Never really allocate one of these.  
+ (id)allocWithZone: (NSZone*)aZone
{
  if (self == NSSpeechSynthesizerClass)
    {
      if (nil == server && !serverLaunchTested)
        {
          unsigned int i = 0;
          // Wait for up to five seconds  for the server to launch, then give up.
          for (i=0 ; i < 50 ; i++)
            {
              server = [NSConnection rootProxyForConnectionWithRegisteredName: @"GSSpeechServer"
                                                                         host: nil];
              RETAIN(server);
              if (nil != server)
                {
                  break;
                }
              [NSThread sleepForTimeInterval: 0.1];
            }
          // Set a flag so we don't bother waiting for the speech server to
          // launch the next time if it didn't work this time.
          serverLaunchTested = YES;
        }
      // If there is no server, this will return nil
      return [server newSynthesizer];
    }
  return [super allocWithZone: aZone];
}

// configuring speech synthesis
- (BOOL) usesFeebackWindow 
{
  return NO;
}

- (void) setUsesFeebackWindow: (BOOL)flag 
{
}

- (NSString *) voice 
{
  return nil;
}

- (void) setVoice: (NSString *)voice 
{
}

- (float) rate 
{
  return 0;
}

- (void) setRate: (float)rate 
{
}

- (float) volume 
{
  return 0;
}

- (void) setVolume: (float)volume 
{
}

- (void) addSpeechDictionary: (NSDictionary *)speechDictionary 
{
}

- (id) objectForProperty: (NSString *)property error: (NSError **)error 
{
  return nil;
}

- (id) setObject: (id) object 
     forProperty: (NSString *)property 
           error: (NSError **)error 
{
  return nil;
}

- (id) delegate 
{
  return nil;
}

- (void) setDelegate: (id)delegate
{
}

// Getting information...
+ (NSArray *) availableVoices 
{
  return [NSArray array];
}

+ (NSDictionary *) attributesForVoice: (NSString *)voice 
{
  return [NSDictionary dictionary];
}

+ (NSString *) defaultVoice 
{ 
  return nil;
}

// Synthesizing..
- (BOOL) isSpeaking 
{
  return NO;
}

- (BOOL) startSpeakingString: (NSString *)text 
{
  return NO;
}

- (BOOL) startSpeakingString: (NSString *)text toURL: (NSURL *)url 
{
  return NO;
}

- (void) stopSpeaking 
{
}

- (void) stopSpeakingAtBoundary: (NSSpeechBoundary)boundary 
{
}

- (void) pauseSpeakingAtBoundary: (NSSpeechBoundary)boundary 
{ 
}

- (void) continueSpeaking 
{
}

- (NSString *) phonemesFromText: (NSString *)text
{
  return nil;
}
@end

