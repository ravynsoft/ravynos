/* Implementation of class PocketsphinxSpeechRecognitionEngine
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Fri Dec  6 04:55:59 EST 2019

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

#import "GSSpeechRecognitionEngine.h"
#import <Foundation/NSDistributedNotificationCenter.h>

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>
#include <pocketsphinx/pocketsphinx.h>

/**
 * Implementation of a speech engine using pocketsphinx.  This should be the default
 * for resource-constrained platforms.
 */

#define MODELDIR "/share/pocketsphinx/model"

static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    /* Argument file. */
    {"-argfile",
     ARG_STRING,
     NULL,
     "Argument file giving extra arguments."},
    {"-adcdev",
     ARG_STRING,
     NULL,
     "Name of audio device to use for input."},
    {"-infile",
     ARG_STRING,
     NULL,
     "Audio file to transcribe."},
    {"-inmic",
     ARG_BOOLEAN,
     "yes",
     "Transcribe audio from microphone."},
    {"-time",
     ARG_BOOLEAN,
     "no",
     "Print word times in file transcription."},
    CMDLN_EMPTY_OPTION
};

@interface PocketsphinxSpeechRecognitionEngine : GSSpeechRecognitionEngine
{
  ps_decoder_t *ps;
  cmd_ln_t *config;
  FILE *fh;
  char const *uttid;
  int16 buf[512];
  int rv;
  int32 score;
  NSThread *_listeningThread;
  id<NSSpeechRecognizerDelegate> _delegate;
}
@end

@implementation PocketsphinxSpeechRecognitionEngine

- (id)init
{
  if ((self = [super init]) != nil)
    {
      char *arg[3];
      arg[0] = "";
      arg[1] = "-inmic";
      arg[2] = "yes";

      config = cmd_ln_parse_r(NULL, cont_args_def, 3, arg, TRUE);
      // turn off pocketsphinx output
      err_set_logfp(NULL);
      err_set_debug_level(0);
      ps_default_search_args(config);
      ps = ps_init(config);
      if (ps == NULL)
        {
          cmd_ln_free_r(config);
          NSLog(@"Could not start server");
          return nil;
        }
      _listeningThread = nil;
    }
  return self;
}

- (void) _recognizedWord: (NSString *)word
{
  [[NSDistributedNotificationCenter defaultCenter]
    postNotificationName: GSSpeechRecognizerDidRecognizeWordNotification
                  object: word
                userInfo: nil];
}

/*
 * NOTE: This code is derived from continuous.c under pocketsphinx 
 *       which is MIT licensed
 * Main utterance processing loop:
 *     while (YES) {
 *        start utterance and wait for speech to process
 *        decoding till end-of-utterance silence will be detected
 *        print utterance result;
 *     }
 */
- (void) recognize
{
  ad_rec_t *ad;
  int16 adbuf[2048];
  BOOL utt_started, in_speech;
  int32 k;
  char const *hyp;

  NSDebugLog(@"** Starting speech recognition loop");
  if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
                        (int) cmd_ln_float32_r(config,
                                               "-samprate"))) == NULL)
    {
      NSLog(@"Failed to open audio device");
      return;
    }
  
  if (ad_start_rec(ad) < 0)
    {
      NSLog(@"Failed to start recording");
      return;
    }
  
  if (ps_start_utt(ps) < 0)
    {
      NSLog(@"Failed to start utterance");
      return;
    }
  
  utt_started = NO;
  NSDebugLog(@"Ready.... <%@, %d>", _listeningThread, [_listeningThread isCancelled]);
  
  while([_listeningThread isCancelled] == NO && _listeningThread != nil)
    {
      if ((k = ad_read(ad, adbuf, 2048)) < 0)
        {
          NSLog(@"Failed to read audio");
          break;
        }
      
      ps_process_raw(ps, adbuf, k, FALSE, FALSE);
      in_speech = ps_get_in_speech(ps);

      if (in_speech && !utt_started)
        {
          utt_started = YES;
          NSDebugLog(@"Listening...");
        }
      
      if (!in_speech && utt_started)
        {
          /* speech -> silence transition, time to start new utterance  */
          ps_end_utt(ps);
          hyp = ps_get_hyp(ps, NULL);
          if (hyp != NULL)
            {
              NSString *recognizedString = [NSString stringWithCString: hyp
                                                              encoding: NSUTF8StringEncoding];
              [self performSelectorOnMainThread: @selector(_recognizedWord:)
                                     withObject: recognizedString
                                  waitUntilDone: NO];
              NSDebugLog(@"Word: %s", hyp);
            }
          
          if (ps_start_utt(ps) < 0)
            {
              NSLog(@"Failed to start utterance");
            }
          
          utt_started = NO;
          NSDebugLog(@"Ready.... <%@, %d>", _listeningThread, [_listeningThread isCancelled]);
        }
      [NSThread sleepForTimeInterval: 0.01];
    }

  // Close everything...
  ps_end_utt(ps);
  ad_close(ad);
}

- (void) start
{
  _listeningThread =
    [[NSThread alloc] initWithTarget: self
                            selector: @selector(recognize)
                              object: nil];
  [_listeningThread setName: @"Speech Recognition Loop"];
  NSLog(@"Starting - Thread info for speech recognition server %@", _listeningThread);
  [_listeningThread start];
}

- (void) stop
{
  NSLog(@"Stop listening thread %@", _listeningThread);
  [_listeningThread cancel];
  RELEASE(_listeningThread);
  _listeningThread = nil;
}

@end

@implementation GSSpeechRecognitionEngine (Pocketsphinx)

+ (GSSpeechRecognitionEngine*)defaultSpeechRecognitionEngine
{
  return AUTORELEASE([[PocketsphinxSpeechRecognitionEngine alloc] init]);
}

@end
