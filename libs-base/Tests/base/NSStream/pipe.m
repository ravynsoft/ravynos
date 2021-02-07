#if	defined(GNUSTEP_BASE_LIBRARY)
/**
 * This test tests a pipe using NSStream
 */
#import "ObjectTesting.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSStream.h>

static NSOutputStream *defaultOutput = nil;
static NSInputStream *defaultInput = nil;
static NSData *goldData;
static NSMutableData *testData;

@interface Listener : NSObject
@end

@implementation Listener

- (void)stream:(NSStream *)theStream handleEvent:(NSStreamEvent)streamEvent
{
  static uint8_t buffer[4096];
  static int writePointer=0;

  switch (streamEvent) 
    {
    case NSStreamEventHasSpaceAvailable:
      {
        NSAssert(theStream==defaultOutput, @"Wrong stream for writing");
        if (writePointer<[goldData length])
          {
            int writeReturn = [defaultOutput write:[goldData bytes]+writePointer 
                                             maxLength:[goldData length]-writePointer];
            writePointer += writeReturn;
          }          
        else
            [defaultOutput close];          
        break;
      }
    case NSStreamEventHasBytesAvailable:
      {
        int readSize;
        NSAssert(theStream==defaultInput, @"Wrong stream for reading");
        readSize = [defaultInput read:buffer maxLength:4096];
        NSAssert(readSize>=0, @"read error");
        if (readSize==0)
          [defaultInput close];
        else
          [testData appendBytes:buffer length:readSize];
        break;
      }
    case NSStreamEventErrorOccurred:
      {
        NSAssert1(1, @"Error! code is %ld",
          (long int)[[theStream streamError] code]);
        break;
      }  
    default:
      break;
    }
} 

@end

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSRunLoop *rl = [NSRunLoop currentRunLoop];
  Listener *li = [[Listener new] autorelease];
  NSString *path = @"pipe.m";
  NSDate   *end;
  
  [NSStream pipeWithInputStream:&defaultInput outputStream:&defaultOutput];
  goldData = [NSData dataWithContentsOfFile:path];
  testData = [NSMutableData dataWithCapacity:4096];
  [defaultInput setDelegate:li];
  [defaultOutput setDelegate:li];
  [defaultInput scheduleInRunLoop:rl forMode:NSDefaultRunLoopMode];
  [defaultOutput scheduleInRunLoop:rl forMode:NSDefaultRunLoopMode];
  [defaultInput open];
  [defaultOutput open];
  end = [NSDate dateWithTimeIntervalSinceNow: 0.5];
  while (NO == [goldData isEqualToData:testData]
    && [end timeIntervalSinceNow] > 0.0)
    {
      [rl runMode: NSDefaultRunLoopMode beforeDate: end];
    }
  PASS([goldData isEqualToData:testData], "Local pipe");
  [arp release];
  return 0;
}
#else
int main()
{
  return 0;
}
#endif
