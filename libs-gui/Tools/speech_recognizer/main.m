#import <Foundation/Foundation.h>

@interface GSSpeechRecognitionServer
+ (void)start;
@end

int main(void)
{
  [NSAutoreleasePool new];
  [GSSpeechRecognitionServer start];
  return 0;
}
