#if	defined(GNUSTEP_BASE_LIBRARY)
/**
 * This test tests client and server socket
 */
#import "ObjectTesting.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSStream.h>

static GSServerStream *serverStream; 
static NSOutputStream *serverOutput = nil;
static NSOutputStream *clientOutput = nil;
static NSInputStream *serverInput = nil;
static NSInputStream *clientInput = nil;
static NSData *goldData;
static NSMutableData *testData;
static NSString *prefix = @"";

static NSString *
eventString(NSStream *stream, NSStreamEvent event)
{
  switch (event)
    {
      case NSStreamEventOpenCompleted: return @"open completed";
      case NSStreamEventHasSpaceAvailable: return @"space available";
      case NSStreamEventHasBytesAvailable: return @"bytes available";
      case NSStreamEventEndEncountered: return @"end encountered"; 
      case NSStreamEventErrorOccurred: 
        return [NSString stringWithFormat: @"error %ld (%@)",
          (long int)[[stream streamError] code], [stream streamError]];
      default:
      return [NSString stringWithFormat: @"unknown event %ld", (long int)event];
    }
}

@interface ClientListener : NSObject
{
  uint8_t buffer[4096];
  int writePointer;
}
@end

@implementation ClientListener

- (void)stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
NSLog(@"%@ Client %p %@", prefix, theStream, eventString(theStream, streamEvent));
  switch (streamEvent) 
    {
    case NSStreamEventOpenCompleted: 
      {
        if (theStream==clientOutput)
	  {
	    NSLog(@"%@ Client %p set write pointer to zero", prefix, theStream);
            writePointer = 0;
	  }
        break;
      }
    case NSStreamEventHasSpaceAvailable: 
      {
        NSAssert(theStream==clientOutput, @"Wrong stream for writing");
        if (writePointer < [goldData length])
          {
            int writeReturn;

            writeReturn = [clientOutput write: [goldData bytes]+writePointer 
				    maxLength: [goldData length]-writePointer];
	    NSLog(@"%@ Client %p wrote %d", prefix, theStream, writeReturn);
            if (writeReturn < 0)
              NSLog(@"%@ Error ... %@", prefix, [theStream streamError]);
            writePointer += writeReturn;
          }          
        else
	  {
	    writePointer = 0;
            [theStream close];          
	    [theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
				 forMode: NSDefaultRunLoopMode];
            NSLog(@"%@ Client close %p", prefix, theStream);
	  }
        break;
      }
    case NSStreamEventHasBytesAvailable: 
      {
        int readSize;

        NSAssert(theStream==clientInput, @"Wrong stream for reading");
        readSize = [clientInput read: buffer maxLength: 4096];
        NSLog(@"%@ Client %p read %d", prefix, theStream, readSize);
        if (readSize < 0)
          {
            NSLog(@"%@ Error ... %@", prefix, [theStream streamError]);
            // it is possible that readSize<0 but not an Error.
	    // For example would block
          }
        else if (readSize == 0)
	  {
            [theStream close];
	    [theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
				   forMode: NSDefaultRunLoopMode];
            NSLog(@"%@ Client close %p", prefix, theStream);
	  }
        else
	  {
            [testData appendBytes: buffer length: readSize];
	  }
        break;
      }
    case NSStreamEventEndEncountered: 
      {
        [theStream setDelegate: nil];
        [theStream close];
	[theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
			     forMode: NSDefaultRunLoopMode];
        break;
      }
    case NSStreamEventErrorOccurred: 
      {
        break;
      }  
    default: 
      break;
    }
}

@end

@interface ServerListener : NSObject
{
  uint8_t buffer[4096];
  int readSize;
  int writeSize;
  BOOL readable;
  BOOL writable;
}
@end

@implementation ServerListener

- (void)stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
NSLog(@"%@ Server %p %@", prefix, theStream, eventString(theStream, streamEvent));
  switch (streamEvent) 
    {
    case NSStreamEventHasBytesAvailable: 
      {
        if (theStream==serverStream)
          {
            NSAssert(serverInput==nil, @"accept twice");
NSLog(@"%@ Server %p accepting incoming connection", prefix, theStream);
            [serverStream acceptWithInputStream: &serverInput
				   outputStream: &serverOutput];
            if (nil == serverInput)   // it is ok to accept nothing
	      {
NSLog(@"%@ Server %p accept failed (no connection)", prefix, theStream);
	      }
	    else
              {
                NSRunLoop *rl = [NSRunLoop currentRunLoop];

                [serverInput scheduleInRunLoop: rl
				       forMode: NSDefaultRunLoopMode];
                [serverOutput scheduleInRunLoop: rl
					forMode: NSDefaultRunLoopMode];
		NSLog(@"%@ Server input stream is %p", prefix, serverInput);
		NSLog(@"%@ Server output stream is %p", prefix, serverOutput);
                RETAIN(serverInput);
                RETAIN(serverOutput);
                [serverInput setDelegate: self];
                [serverOutput setDelegate: self];
                [serverInput open];
                [serverOutput open];
                [serverInput scheduleInRunLoop: rl
				       forMode: NSDefaultRunLoopMode];
                [serverOutput scheduleInRunLoop: rl
					forMode: NSDefaultRunLoopMode];
                readSize = 0;
                writeSize = 0;
		[serverStream setDelegate: nil];
                [serverStream close];
		[serverStream removeFromRunLoop: [NSRunLoop currentRunLoop]
					forMode: NSDefaultRunLoopMode];
              }
          }
        if (theStream == serverInput)
          {
	    readable = YES;
	  }
        break;
      }
    case NSStreamEventHasSpaceAvailable: 
      {
        NSAssert(theStream==serverOutput, @"Wrong stream for writing");
	writable = YES;
        break;
      }
    case NSStreamEventEndEncountered: 
      {
	[theStream setDelegate: nil];
        [theStream close];
	[theStream removeFromRunLoop: [NSRunLoop currentRunLoop]
			     forMode: NSDefaultRunLoopMode];
        NSLog(@"%@ Server close %p", prefix, theStream);
	if (theStream == serverInput && writeSize == readSize)
	  {
	    [serverOutput setDelegate: nil];
	    [serverOutput close];
	    [serverOutput removeFromRunLoop: [NSRunLoop currentRunLoop]
				    forMode: NSDefaultRunLoopMode];
	    NSLog(@"%@ Server output close %p", prefix, serverOutput);
	  }
        break;
      }
    case NSStreamEventErrorOccurred: 
        break;
    default: 
      break;
    }

  while ((readable == YES && writeSize == readSize)
    || (writable == YES && writeSize < readSize))
    {
      if (readable == YES && writeSize == readSize)
	{
	  readSize = [serverInput read: buffer maxLength: 4096];
	  readable = NO;
	  NSLog(@"%@ Server %p read %d", prefix, serverInput, readSize);
	  writeSize = 0;
	  if (readSize == 0)
	    {
	      [serverInput setDelegate: nil];
	      [serverInput close];
	      [serverInput removeFromRunLoop: [NSRunLoop currentRunLoop]
				     forMode: NSDefaultRunLoopMode];
	      NSLog(@"%@ Server input close %p", prefix, serverInput);
	      [serverOutput setDelegate: nil];
	      [serverOutput close];
	      [serverOutput removeFromRunLoop: [NSRunLoop currentRunLoop]
				      forMode: NSDefaultRunLoopMode];
	      NSLog(@"%@ Server output close %p", prefix, serverOutput);
	    }
	  else if (readSize < 0)
	    {
              NSLog(@"%@ Error ... %@", prefix, [clientInput streamError]);
	      readSize = 0;
	    }
	}
      if (writable == YES && writeSize < readSize)
	{
	  int writeReturn = [serverOutput write: buffer+writeSize 
					  maxLength: readSize-writeSize];
	  NSLog(@"%@ Server %p wrote %d", prefix, serverOutput, writeReturn);
	  writable = NO;
	  if (writeReturn == 0)
	    {
	      [serverOutput setDelegate: nil];
	      [serverOutput close];
	      [serverOutput removeFromRunLoop: [NSRunLoop currentRunLoop]
				      forMode: NSDefaultRunLoopMode];
	      NSLog(@"%@ Server close %p", prefix, serverOutput);
	      [serverInput setDelegate: nil];
	      [serverInput close];
	      [serverInput removeFromRunLoop: [NSRunLoop currentRunLoop]
				     forMode: NSDefaultRunLoopMode];
	      NSLog(@"%@ Server input close %p", prefix, serverInput);
	    }
	  else if (writeReturn > 0)
	    {
	      writeSize += writeReturn;
	    }
	  else if (writeReturn < 0)
	    {
	      NSLog(@"%@ Error ... %@", prefix, [serverOutput streamError]);
	    }

	  /* If we have finished writing and there is no more data coming,
	   * we can close the output stream.
	   */
	  if (writeSize == readSize
	    && [serverInput streamStatus] == NSStreamStatusClosed)
	    {
	      [serverOutput setDelegate: nil];
	      [serverOutput close];
	      [serverOutput removeFromRunLoop: [NSRunLoop currentRunLoop]
				      forMode: NSDefaultRunLoopMode];
	      NSLog(@"%@ Server output close %p", prefix, serverOutput);
	    }
	}
    }
} 

@end

int main()
{
  CREATE_AUTORELEASE_POOL(arp);
  NSRunLoop *rl = [NSRunLoop currentRunLoop];
  NSHost *host = [NSHost hostWithAddress: @"127.0.0.1"];
  ServerListener *sli;
  ClientListener *cli;
  NSString *path = @"socket_cs.m";
  NSString *socketPath = @"test-socket";
  NSDate *end;

  [[NSFileManager defaultManager] removeFileAtPath: socketPath handler: nil];
  NSLog(@"sending and receiving on %@: %@", host, [host address]);
  goldData = [NSData dataWithContentsOfFile: path];
  testData = [NSMutableData dataWithCapacity: 4096];

{
  CREATE_AUTORELEASE_POOL(inner);

  prefix = @"Test1";
  [testData setLength: 0];
  sli = AUTORELEASE([ServerListener new]);
  cli = AUTORELEASE([ClientListener new]);
  serverStream
    = [GSServerStream serverStreamToAddr: [host address] port: 1234];
  [serverStream setDelegate: sli];
  [serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [serverStream open];
  [NSStream getStreamsToHost: host
			port: 1234
		 inputStream: &clientInput
		outputStream: &clientOutput];
  NSLog(@"%@ Client input stream is %p", prefix, clientInput);
  NSLog(@"%@ Client output stream is %p", prefix, clientOutput);
  [clientInput setDelegate: cli];
  [clientOutput setDelegate: cli];
  [clientInput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [clientOutput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [clientInput open];
  [clientOutput open];

  end = [NSDate dateWithTimeIntervalSinceNow: 5];
  while (NO == [goldData isEqualToData: testData]
    && [end timeIntervalSinceNow] > 0.0)
    {
      [rl runMode: NSDefaultRunLoopMode beforeDate: end];
    }
  PASS([goldData isEqualToData: testData], "Local tcp");
  if ([end timeIntervalSinceNow] < 0.0)
    NSLog(@"%@ timed out.\n", prefix);

  [clientInput setDelegate: nil];
  [clientOutput setDelegate: nil];
  DESTROY(serverInput);
  DESTROY(serverOutput);
  RELEASE(inner);
}

{
  CREATE_AUTORELEASE_POOL(inner);

  prefix = @"Test2";
  [testData setLength: 0];
  sli = AUTORELEASE([ServerListener new]);
  cli = AUTORELEASE([ClientListener new]);
  serverStream
    = [GSServerStream serverStreamToAddr: [host address] port: 1234];
  [serverStream setDelegate: sli];
  [serverStream open];
  [serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [NSStream getStreamsToHost: host
			port: 1234
		 inputStream: &clientInput
		outputStream: &clientOutput];
  NSLog(@"%@ Client input stream is %p", prefix, clientInput);
  NSLog(@"%@ Client output stream is %p", prefix, clientOutput);
  [clientInput setDelegate: cli];
  [clientOutput setDelegate: cli];
  [clientInput open];
  [clientOutput open];
  [clientInput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [clientOutput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];

  end = [NSDate dateWithTimeIntervalSinceNow: 5];
  while (NO == [goldData isEqualToData: testData]
    && [end timeIntervalSinceNow] > 0.0)
    {
      [rl runMode: NSDefaultRunLoopMode beforeDate: end];
    }
  PASS([goldData isEqualToData: testData], "Local tcp (blocking open)");
  if ([end timeIntervalSinceNow] < 0.0)
    NSLog(@"%@ timed out.\n", prefix);

  [clientInput setDelegate: nil];
  [clientOutput setDelegate: nil];
  DESTROY(serverInput);
  DESTROY(serverOutput);
  RELEASE(inner);
}

{
  CREATE_AUTORELEASE_POOL(inner);

  prefix = @"Test3";
  [testData setLength: 0];
  sli = AUTORELEASE([ServerListener new]);
  cli = AUTORELEASE([ClientListener new]);
  serverStream = [GSServerStream serverStreamToAddr: socketPath];
  [serverStream setDelegate: sli];
  [serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [serverStream open];
  [NSStream getLocalStreamsToPath: socketPath
		      inputStream: &clientInput
		     outputStream: &clientOutput];
  NSLog(@"%@ Client input stream is %p", prefix, clientInput);
  NSLog(@"%@ Client output stream is %p", prefix, clientOutput);
  [clientInput setDelegate: cli];
  [clientOutput setDelegate: cli];
  [clientInput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [clientOutput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [clientInput open];
  [clientOutput open];

  end = [NSDate dateWithTimeIntervalSinceNow: 5];
  while (NO == [goldData isEqualToData: testData]
    && [end timeIntervalSinceNow] > 0.0)
    {
      [rl runMode: NSDefaultRunLoopMode beforeDate: end];
    }
  PASS([goldData isEqualToData: testData], "Local socket");
  if ([end timeIntervalSinceNow] < 0.0)
    NSLog(@"%@ timed out.\n", prefix);

  [clientInput setDelegate: nil];
  [clientOutput setDelegate: nil];
  DESTROY(serverInput);
  DESTROY(serverOutput);
  RELEASE(inner);
}

{
  CREATE_AUTORELEASE_POOL(inner);

  prefix = @"Test4";
  [testData setLength: 0];
  sli = AUTORELEASE([ServerListener new]);
  cli = AUTORELEASE([ClientListener new]);

  [[NSFileManager defaultManager] removeFileAtPath: socketPath handler: nil];

  serverStream = [GSServerStream serverStreamToAddr: socketPath];
  [serverStream setDelegate: sli];
  [serverStream open];
  [serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [NSStream getLocalStreamsToPath: socketPath
		      inputStream: &clientInput
		     outputStream: &clientOutput];
  NSLog(@"%@ Client input stream is %p", prefix, clientInput);
  NSLog(@"%@ Client output stream is %p", prefix, clientOutput);
  [clientInput setDelegate: cli];
  [clientOutput setDelegate: cli];
  [clientInput open];
  [clientOutput open];
  [clientInput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [clientOutput scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];

  end = [NSDate dateWithTimeIntervalSinceNow: 5];
  while (NO == [goldData isEqualToData: testData]
    && [end timeIntervalSinceNow] > 0.0)
    {
      [rl runMode: NSDefaultRunLoopMode beforeDate: end];
    }
  PASS([goldData isEqualToData: testData], "Local socket (blocking open)");
  if ([end timeIntervalSinceNow] < 0.0)
    NSLog(@"%@ timed out.\n", prefix);

  [clientInput setDelegate: nil];
  [clientOutput setDelegate: nil];
  DESTROY(serverInput);
  DESTROY(serverOutput);
  RELEASE(inner);
}

  [[NSFileManager defaultManager] removeFileAtPath: socketPath handler: nil];

  RELEASE(arp);
  return 0;
}
#else
int main()
{
  return 0;
}
#endif
