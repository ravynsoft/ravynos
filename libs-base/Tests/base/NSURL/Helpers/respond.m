#if	GNUSTEP
#include	<Foundation/Foundation.h>

@interface	TestClass : NSObject
{
  NSOutputStream *op;
  NSInputStream *ip;
  NSData	*content;
  unsigned	written;
  BOOL		readable;
  BOOL		writable;
  unsigned	writeLen;
  unsigned	count;
  double        pause;
}
- (int) runTest;
@end

@implementation	TestClass

- (void) dealloc
{
  RELEASE(content);
  RELEASE(op);
  RELEASE(ip);
  [super dealloc];
}

- (id) init
{
  return self;
}

- (int) runTest
{
  NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
  NSRunLoop		*rl = [NSRunLoop currentRunLoop];
  NSHost		*host = [NSHost hostWithName: @"localhost"];
  NSStream		*serverStream;
  NSString		*file;
  int			port = [[defs stringForKey: @"Port"] intValue];

  if (port == 0) port = 1234;

  count = [[defs stringForKey: @"Count"] intValue];
  if (count <= 0) count = 1;

  pause = [defs floatForKey: @"Pause"];

  file = [defs stringForKey: @"FileName"];
  if (file == nil) file = @"Respond.dat";
  content = [[NSData alloc] initWithContentsOfFile: file];
  if (content == nil)
    {
      NSLog(@"Unable to load data from '%@'", file);
      return 1;
    }

  if ([defs boolForKey: @"Shrink"] == YES)
    {
      writeLen = [content length];
    }
  else
    {
      writeLen = 0;
    }

  serverStream = [GSServerStream serverStreamToAddr: [host address] port: port];
  if (serverStream == nil)
    {
      NSLog(@"Failed to create server stream");
      return 1;
    }
  [serverStream setDelegate: self];
  [serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [serverStream open];

  /* Run for up to 5 minutes to allow slow/large tests to complete.
   */
  [rl runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 300]];

  return 0;
}

- (void) stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
  NSRunLoop	*rl = [NSRunLoop currentRunLoop];

NSLog(@"Server %p %u", theStream, (unsigned)streamEvent);
  switch (streamEvent) 
    {
    case NSStreamEventHasBytesAvailable: 
      {
        if (theStream != ip)
          {
	    if (ip != nil)
	      {
		[ip close];
		[ip removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
		ip = nil;
	      }
	    if (op != nil)
	      {
		[op close];
		[op removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
		op = nil;
	      }
            [(GSServerStream*)theStream acceptWithInputStream: &ip
						 outputStream: &op];
            if (ip)   // it is ok to accept nothing
              {
	        written = 0;	// Nothing written yet on this connection.
                RETAIN(ip);
                RETAIN(op);
                [ip scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
                [op scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
                [ip setDelegate: self];
                [op setDelegate: self];
                [ip open];
                [op open];
		NSLog(@"Accept %d", count);
		if (count > 0)
		  {
		    count--;
		  }
		if (count == 0)
		  {
		    /*
		     * Handled enough requests ... close down
		     */
                    [theStream close];
		    [theStream removeFromRunLoop: rl
		   			 forMode: NSDefaultRunLoopMode];
		  }
		if (writeLen > 0 && [[NSUserDefaults standardUserDefaults]
		  boolForKey: @"Shrink"] == YES)
		  {
		    /* Want to write in slightly smaller chunks for each
		     * connection so that the remote end can check that
		     * it handles different chunk sizes properly.
		     */
		    writeLen--;
		  }
              }
	    else
	      {
	        NSLog(@"Accept returned nothing");
	      }
          }
        else if (theStream == ip)
          {
	    readable = YES;
	    while (readable == YES)
	      {
		unsigned char	buffer[BUFSIZ];
		int		readSize;

		readSize = [ip read: buffer maxLength: sizeof(buffer)];
		if (readSize <= 0)
		  {
		    readable = NO;
		  }
	      }
	  }
        break;
      }
    case NSStreamEventHasSpaceAvailable: 
      {
        NSAssert(theStream == op, @"Wrong stream for writing");
	writable = YES;
	while (writable == YES && written < [content length])
	  {
	    int	length = [content length] - written;

	    /* If we have a write length limit set, don't try to write
	     * more than that in one chunk.
	     */
	    if (writeLen > 0 && length > writeLen)
	      {
	        length = writeLen;
	      }

	    /* We now pause for the time (in sec) specified.  Even if
	     * the time is zero or less we still have a small pause, to try to
	     * coerce the OS into separating the write events.
             */
	    if (pause <= 0)
	      {
		[NSThread sleepUntilDate:
		  [NSDate dateWithTimeIntervalSinceNow: 0.00001]];
	      }
	    else
	      {
		[NSThread sleepUntilDate:
		  [NSDate dateWithTimeIntervalSinceNow: pause]];
	      }

	    length = [op write: [content bytes] + written maxLength: length];
	    if (length <= 0)
	      {
	        writable = NO;
	      }
	    else
	      {
	        written += length;
	      }
	  }
	if (written == [content length])
	  {
	    [op close];
	    [op removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	    op = nil;
	    if (ip != nil)
	      {
		[ip close];
		[ip removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
		ip = nil;
	        NSLog(@"Done reading %d", count+1);
	      }
	    NSLog(@"Done writing %d", count+1);
	  }
        break;
      }
    case NSStreamEventEndEncountered: 
      {
        NSAssert(ip == theStream || op == theStream,
	  NSInternalInconsistencyException);
        [theStream close];
	[theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	if (theStream == ip) ip = nil;
	if (theStream == op) op = nil;
      NSLog(@"Server close %p", theStream);
        break;
      }

    case NSStreamEventErrorOccurred: 
      {
        int	code = [[theStream streamError] code];

        NSLog(@"Error on %p code is %d", theStream, code);
        [theStream close];
	[theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	if (theStream == ip) ip = nil;
	if (theStream == op) op = nil;
        break;
      }  

    case NSStreamEventOpenCompleted: 
      break;

    default: 
      NSLog(@"Unexpected event %u on %p", (unsigned)streamEvent, theStream);
      break;
    }
} 

@end

int
main(int argc, char **argv)
{
  int	result;
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

  result = [[[[TestClass alloc] init] autorelease] runTest];

  RELEASE(arp);
  return result;
}

#else

int main()
{
  return 0;
}

#endif
