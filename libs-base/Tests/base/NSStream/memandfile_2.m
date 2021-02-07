/**
 * This test tests asynchronized copying between mem and file using a runloop
 */
#import "ObjectTesting.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSStream.h>

static NSOutputStream *defaultOutput = nil;
static NSInputStream *defaultInput = nil;

@interface Listener1 : NSObject
@end

@interface Listener2 : NSObject
@end

@implementation Listener1

- (void)stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
  static uint8_t buffer[4096];
  switch (streamEvent) 
    {
    case NSStreamEventHasBytesAvailable: 
      {
        int len = [(NSInputStream*)theStream read: buffer maxLength: 4096];
        uint8_t *p = buffer;
        
        if (len==0)
          {
            [theStream close];
	    [theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
				 forMode: NSDefaultRunLoopMode];
          }
        else 
          {
            while(len>0)
              {
                int written = [defaultOutput write: p maxLength: len];
                p = p + written;
                len = len - written;
              }
          }
        break;
      }
    case NSStreamEventEndEncountered: 
      {
	[theStream close];
	[theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
			     forMode: NSDefaultRunLoopMode];
        break;
      }
    default: 
      {
        NSAssert1(1, @"Error! code is %ld",
          (long int)[[theStream streamError] code]);
        break;
      }  
    }
} 

@end

@implementation Listener2

- (void)stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
  static uint8_t *p;
  static uint8_t buffer[4096];
  static int len = 0;

  switch (streamEvent) 
    {
    case NSStreamEventHasSpaceAvailable: 
      {
        if (len==0)
          {
            len = [defaultInput read: buffer maxLength: 4096];
            p = buffer;
          }
        if (len==0)
          {
	    [theStream close];
	    [theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
				 forMode: NSDefaultRunLoopMode];
          }
        else 
          {
            int written = [(NSOutputStream*)theStream write: p maxLength: len];
            p = p + written;
            len = len - written;
          }
        break;
      }
    case NSStreamEventEndEncountered: 
      {
	[theStream close];
	[theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
			     forMode: NSDefaultRunLoopMode];
        break;
      }
    default: 
      {
        NSAssert1(1, @"Error! code is %ld",
          (long int)[[theStream streamError] code]);
        break;
      }  
    }
} 

@end

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSRunLoop     *rl = [NSRunLoop currentRunLoop];
  NSData        *answer = nil;
  NSDate        *end;

  // first test, file to memory copy
  NSString *path = @"memandfile.m";
  NSData *goldData = [NSData dataWithContentsOfFile: path];
  NSInputStream *input = [NSInputStream inputStreamWithFileAtPath: path];
  NSOutputStream *output = [NSOutputStream outputStreamToMemory];
  Listener1 *l1 = [[Listener1 new] autorelease];

  [input setDelegate: l1];
  [input scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [input open];
  [output open];
  defaultOutput = output;
  end = [NSDate dateWithTimeIntervalSinceNow: 1.0];
  while (NO == [goldData isEqualToData: answer]
    && [end timeIntervalSinceNow] > 0.0)
    {
      [rl runMode: NSDefaultRunLoopMode beforeDate: end];
      answer = [output propertyForKey: NSStreamDataWrittenToMemoryStreamKey];
    }

  answer = [output propertyForKey: NSStreamDataWrittenToMemoryStreamKey];
  PASS([goldData isEqualToData: answer], "file to memory copy ok");

  // second test, memory to file copy
  NSString *pathO = @"temp";
  NSInputStream *input2 = [NSInputStream inputStreamWithData: goldData];
  NSOutputStream *output2 = [NSOutputStream outputStreamToFileAtPath: pathO append: NO];
  Listener1 *l2 = [[Listener2 new] autorelease];

  [output2 setDelegate: l2];
  [output2 scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [input2 open];
  [output2 open];
  defaultInput = input2;

  end = [NSDate dateWithTimeIntervalSinceNow: 1.0];
  NSData *answer2 = nil;
  while (NO == [goldData isEqualToData: answer2]
    && [end timeIntervalSinceNow] > 0.0)
    {
      [rl runMode: NSDefaultRunLoopMode beforeDate: end];
      answer2 = [NSData dataWithContentsOfFile: pathO];
    }

  answer2 = [NSData dataWithContentsOfFile: pathO];
  PASS([goldData isEqualToData: answer2], "memory to file copy ok");

  [[NSFileManager defaultManager] removeFileAtPath: pathO handler: nil];
    
  [arp release];
  return 0;
}

