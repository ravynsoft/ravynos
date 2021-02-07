#import "GSSpeechServer.h"
#import "GSSpeechEngine.h"
#import "GSSpeechSynthesizer.h"
#import <Foundation/Foundation.h>

static GSSpeechServer *sharedInstance;

@implementation GSSpeechServer
+ (void)initialize
{
  sharedInstance = [self new];
}

+ (void)start
{
  NSConnection *connection = [NSConnection defaultConnection];
  [connection setRootObject: sharedInstance];
  if (NO == [connection registerName: @"GSSpeechServer"])
    {
      return;
    }
  [[NSRunLoop currentRunLoop] run];
}

+ (id)sharedServer
{
  return sharedInstance;
}

- (id)init
{
  if (nil == (self = [super init]))
    {
      return nil;
    }
  
  engine = [GSSpeechEngine defaultSpeechEngine];
  if (nil == engine)
    {
      [self release];
      return nil;
    }
  return self;
}

- (id)newSynthesizer
{
  return [[GSSpeechSynthesizer new] autorelease];
}

- (BOOL)startSpeakingString: (NSString*)aString notifyWhenDone: (id)client
{
  [engine stopSpeaking];
  [engine startSpeaking: aString notifyWhenDone: client];
  return YES;
}

- (void)stopSpeaking
{
  [engine stopSpeaking];
}

- (BOOL)isSpeaking
{
  return [engine isSpeaking];
}

- (NSArray*)voices
{
  return [engine voices];
}

- (oneway void)setVoice: (NSString*)aVoice
{
  [engine setVoice: aVoice];
}

- (NSString*)voice
{
  return [engine voice];
}

- (NSString*)defaultVoice
{
  return [engine defaultVoice];
}
@end
