/**
 * This test tests synchronized copying between mem and file
 */
#import "ObjectTesting.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSStream.h>

static void copyStream(NSInputStream *input, NSOutputStream *output)
{
  uint8_t buffer[4096];

  [input open];
  [output open];
  while([input hasBytesAvailable])
    {
      int len = [input read:buffer maxLength:4096];
      uint8_t *p = buffer;
      while(len>0)
        {
          int written = [output write:p maxLength:len];
          p = p + written;
          len = len - written;
        }
    }
  [input close];
  [output close];
}

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

  // first test, file to memory copy
  NSString *path = @"memandfile.m";
  NSData *goldData = [NSData dataWithContentsOfFile:path];
  NSInputStream *input = [NSInputStream inputStreamWithFileAtPath:path];
  NSOutputStream *output = [NSOutputStream outputStreamToMemory];

  copyStream(input, output);

  NSData *answer = [output propertyForKey:NSStreamDataWrittenToMemoryStreamKey];
  PASS([goldData isEqualToData:answer], "file to memory copy ok");

  // second test, memory to file copy
  NSString *pathO = @"temp";
  NSInputStream *input2 = [NSInputStream inputStreamWithData:goldData];
  NSOutputStream *output2 = [NSOutputStream outputStreamToFileAtPath:pathO append:NO];

  copyStream(input2, output2);

  NSData *answer2 = [NSData dataWithContentsOfFile:pathO];
  PASS([goldData isEqualToData:answer2], "memory to file copy ok");

  [[NSFileManager defaultManager] removeFileAtPath: pathO handler: nil];

  [arp release]; arp = nil;
  return 0;
}
