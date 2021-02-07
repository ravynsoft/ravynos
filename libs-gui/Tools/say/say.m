#import <AppKit/AppKit.h>
#include <unistd.h>
#include <getopt.h>

@interface SpeechDelegate : NSObject
@end

@implementation SpeechDelegate
- (void)speechSynthesizer: (NSSpeechSynthesizer*)sender 
        didFinishSpeaking: (BOOL)success
{
  exit((int)success);
}
@end

int main(int argc, char **argv)
{
  NSAutoreleasePool * p = [NSAutoreleasePool new];
  NSMutableString *words = nil;
  NSString *outFile = nil;
  NSString *voice = nil;
  NSString *inFile = nil;
  NSSpeechSynthesizer *say;
  int ch;
  int i;

  while ((ch = getopt(argc, argv, "o:v:f:")) != -1)
    {
      switch (ch)
        {
        case 'o':
          outFile = [NSString stringWithUTF8String: optarg];
          break;
        case 'f':
          inFile = [NSString stringWithUTF8String: optarg];
          break;
        case 'v':
          voice = [NSString stringWithUTF8String: optarg];
          break;
        }
    }
  
  if (nil != inFile)
    {
      NSData *file = [NSData dataWithContentsOfFile: inFile];
      words = [NSString stringWithCString: [file bytes]];
    }
  else
    {
      words = [NSMutableString string];
      for (i = optind ; i < argc ; i++)
        {
          [words appendString: [NSString stringWithUTF8String: argv[i]]];
          [words appendString: @" "];
        }
    }

  // Don't interrupt other apps.
  while ([NSSpeechSynthesizer isAnyApplicationSpeaking])
    {
      [NSThread sleepForTimeInterval: 0.1];
    }

  say = [[NSSpeechSynthesizer alloc] initWithVoice: voice];
  [say setDelegate: [SpeechDelegate new]];
  if (nil != outFile)
    {
      [say startSpeakingString: words toURL: [NSURL fileURLWithPath: outFile]];
    }
  else
    {
      [say startSpeakingString: words];
    }
  [[NSRunLoop currentRunLoop] run];
  // Not reached.
  RELEASE(p);
  return 0;
}
