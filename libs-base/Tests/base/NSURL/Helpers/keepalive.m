#if     GNUSTEP
#include <Foundation/Foundation.h>

@interface TestClass : NSObject
{
  NSData         *bodyData;
  unsigned       accum;
  unsigned       count;
  unsigned       closeFrequency;
  NSArray        *headers;
  NSInputStream  *inStream;
  NSOutputStream *outStream;
}
- (int) runTest;
@end

@implementation TestClass

- (void)dealloc
{
  [headers release];
  [bodyData release];
  [inStream release];
  [outStream release];
  [super dealloc];
}

- (id) init
{
  return self;
}

- (int)runTest
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  NSRunLoop      *runLoop = [NSRunLoop currentRunLoop];
  NSString       *file,
                 *lengthHeader;
  NSHost         *host = [NSHost hostWithName: @"localhost"];
  NSStream       *serverStream;
  int            port = [[defs stringForKey: @"Port"] intValue];
  int            lifetime = [[defs stringForKey: @"Lifetime"] intValue];
  
  if (0 == port) port = 4322;
  if (0 == lifetime) lifetime = 30;

  accum = 0;

  closeFrequency = [[defs stringForKey: @"CloseFreq"] intValue];
  count = [[defs stringForKey: @"Count"] intValue];
  if (0 >= count) count=1;

  file = [defs stringForKey: @"FileName"];
  if (nil == file) file = @"ResponseBody.dat";
  bodyData = [[NSData alloc] initWithContentsOfFile: file];
  if (nil == bodyData)
    {
      NSLog(@"Unable to load data from '%@'",file);
      return 1;
    }

  if (YES == [defs boolForKey: @"FileHdrs"])
    {
      headers = nil;	// Already in the file
    }
  else
    {
      lengthHeader = [NSString stringWithFormat: @"Content-Length: %u\r\n",
	(unsigned)[bodyData length]];
      headers = [[NSArray alloc] initWithObjects: @"HTTP/1.1 200 OK\r\n",
	@"Content-type: text/plain\r\n", lengthHeader, nil];
    }

  serverStream = [GSServerStream serverStreamToAddr: [host address] port: port];
  if (nil == serverStream)
    {
      NSLog(@"Could not create server stream");
      return 1;
    }
  [serverStream setDelegate: self];
  [serverStream scheduleInRunLoop: runLoop forMode: NSDefaultRunLoopMode];
  [serverStream open];

  // only run for a fixed time anyway
  [runLoop runUntilDate: [NSDate dateWithTimeIntervalSinceNow: lifetime]];
  
  return 0;
}

// delegate method for NSStream
- (void) stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
  NSRunLoop *runLoop = [NSRunLoop currentRunLoop];

  switch (streamEvent)
    {
    case NSStreamEventHasBytesAvailable:
      {
	if (theStream != inStream)
	  {
	    if (inStream != nil)
	      {
                [inStream close];
                [inStream removeFromRunLoop: runLoop
                                    forMode: NSDefaultRunLoopMode];
		inStream = nil;
	      }
	    if (outStream != nil)
	      {
		[outStream close];
		[outStream removeFromRunLoop: runLoop
                                     forMode: NSDefaultRunLoopMode];
		outStream = nil;
	      }
	    if (accum == count)
	      {
		[theStream close];
		[theStream removeFromRunLoop: runLoop
                                     forMode: NSDefaultRunLoopMode];
		break;
	      }
	    [(GSServerStream*)theStream acceptWithInputStream: &inStream
                                                 outputStream: &outStream];
	    if (inStream)
	      {
		RETAIN(inStream);
		RETAIN(outStream);
		[inStream scheduleInRunLoop: runLoop
                                    forMode: NSDefaultRunLoopMode];
		[inStream setDelegate: self];
		[inStream open];
	      }
	    else
	      {
		NSLog(@"Accept returned nothing");
	      }
	  }
	else
	  {
	    unsigned char buffer[BUFSIZ];

	    (void)[inStream read: buffer maxLength: BUFSIZ];
	    // make outStream available for writing
	    [outStream scheduleInRunLoop: runLoop
                                 forMode: NSDefaultRunLoopMode];
	    [outStream setDelegate: self];
	    [outStream open];
	  }
	break;
      }

    case NSStreamEventHasSpaceAvailable:
      {
	NSAssert(theStream == outStream, @"Wrong stream for writing");
	NSMutableData  *payload = [[[NSMutableData alloc] init] autorelease];
	NSMutableArray *headerArray = [[headers mutableCopy] autorelease];
	NSEnumerator  *headerEnumeration;
	id             header;
	BOOL           closeConnection = NO;
	int            written;

	accum++;
	if ((0 != closeFrequency) && (0 == accum % closeFrequency))
	  {
	    closeConnection = YES;
	    [headerArray addObject: @"Connection: close\r\n"];
	  }

	headerEnumeration = [headerArray objectEnumerator];
	if (nil != headerEnumeration)
	  {
	    while ((header = [headerEnumeration nextObject]))
	      [payload appendData:
		[header dataUsingEncoding: NSASCIIStringEncoding]];
	    [payload appendData:
	      [@"\n" dataUsingEncoding: NSASCIIStringEncoding]];
	  }
	[payload appendData: bodyData];

	// provide the reply
	written = [outStream write: [payload bytes]
			 maxLength: [payload length]];
	if (written <= 0)
	  {
	    // something seriously wrong....
	    closeConnection = YES;
	    NSLog(@"Unable to write the payload!");
	  }

	// remove from runloop, so we have to wait for another request
	[outStream removeFromRunLoop: runLoop
                             forMode: NSDefaultRunLoopMode];

	if (closeConnection)
	  {
	    [outStream close];
	    outStream = nil;

	    if (nil != inStream)
	      {
		[inStream close];
		[inStream removeFromRunLoop: runLoop
                                    forMode: NSDefaultRunLoopMode];
		inStream = nil;
	      }
	  }
	break;
      }

    case NSStreamEventEndEncountered:
      {
	NSAssert(inStream == theStream || outStream == theStream,
                 NSInternalInconsistencyException);
	[theStream close];
	[theStream removeFromRunLoop: runLoop forMode: NSDefaultRunLoopMode];
	if (theStream == inStream) inStream = nil;
	if (theStream == outStream) outStream = nil;
	break;
      }

    case NSStreamEventErrorOccurred:
      {
	int code = [[theStream streamError] code];
	NSLog(@"Received error %d on stream %p", code, theStream);
	[theStream close];
	[theStream removeFromRunLoop: runLoop forMode: NSDefaultRunLoopMode];
	if (theStream == inStream) inStream = nil;
	if (theStream == outStream) outStream = nil;
	break;
      }
    
    case NSStreamEventOpenCompleted:
      break;
    default:
      NSLog(@"Unknown event %u on stream %p", (unsigned)streamEvent, theStream);
      break;
    }
}

@end

int main(int argc, char **argv)
{
  int result;
  NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];

  result = [[[[TestClass alloc] init] autorelease] runTest];

  [arp release];
  return result;
}
#else
int main()
{
  return 0;
}
#endif

