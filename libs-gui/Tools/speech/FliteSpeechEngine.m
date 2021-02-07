#import "GSSpeechEngine.h"
#include <flite/flite.h>

cst_voice *register_cmu_us_kal16();

/**
 * Implementation of a speech engine using flite.  This should be the default
 * for resource-constrained platforms.
 */
@interface FliteSpeechEngine : GSSpeechEngine {
  /** The audio device used for sound output. */
  cst_audiodev *ad;
  /** The current voice.  Only one supported at the moment. */
  cst_voice *v;
  /** Flag set to tell the playback thread to exit. */
  volatile BOOL shouldEndSpeaking;
  /** Flag indicating whether the engine is currently speaking. */
  volatile BOOL isSpeaking;
}
@end

@implementation FliteSpeechEngine
+ (void)initialize
{
  flite_init(); 
}

- (id)init
{
  if (nil == (self = [super init])) { return nil; }
  
  // Only one voice supported by flite unless others are compiled in.
  v = register_cmu_us_kal16();
  if (NULL == v)
    {
      [self release];
      return nil;
    }
  
  // Each wave should be the same format.
  cst_wave *w = flite_text_to_wave("test", v);
  ad = audio_open(w->sample_rate, w->num_channels, CST_AUDIO_LINEAR16);
  delete_wave(w);
  if (NULL == ad)
    {
      [self release];
      return nil;
    }
  return self;
}

- (void)sayString: (NSArray*)args
{
  id pool = [NSAutoreleasePool new];
  NSString *aString = [args objectAtIndex: 0];
  int i,n,r;
  int num_shorts;
  BOOL didFinish = YES;
  cst_wave *w = flite_text_to_wave([aString UTF8String], v);
  
  num_shorts = w->num_samples * w->num_channels;
  for (i=0; i < num_shorts; i += r/2)
    {
      if (num_shorts > i+CST_AUDIOBUFFSIZE)
        {
          n = CST_AUDIOBUFFSIZE;
        }
      else
        {
          n = num_shorts-i;
        }
      r = audio_write(ad, &w->samples[i], n*2);
      if (shouldEndSpeaking)
        {
          didFinish = NO;
          break;
        }
    }
  isSpeaking = NO;
  NS_DURING
    [[args objectAtIndex: 1] didFinishSpeaking: didFinish];
  NS_HANDLER
  NS_ENDHANDLER;
  [args release];
  [pool release];
  delete_wave(w);
}

- (void)startSpeaking: (NSString*)aString notifyWhenDone: (id)aDelegate
{
  [[[aDelegate delegate] connectionForProxy] enableMultipleThreads];
  NSArray *arg = [[NSArray alloc] initWithObjects: aString, aDelegate, nil];
  shouldEndSpeaking = NO;
  isSpeaking = YES;
  [NSThread detachNewThreadSelector: @selector(sayString:)
                           toTarget: self
                         withObject: arg];
  
}

- (BOOL)isSpeaking
{
  return isSpeaking;
}

- (void)stopSpeaking
{
  shouldEndSpeaking = YES;
  // Spin until the other thread has died.
  while (isSpeaking) {}
}

- (void)dealloc
{
  [self stopSpeaking];
  audio_close(ad);
  [super dealloc];
}
@end

@implementation GSSpeechEngine (Flite)
+ (GSSpeechEngine*)defaultSpeechEngine
{
  return [[[FliteSpeechEngine alloc] init] autorelease];
}
@end
