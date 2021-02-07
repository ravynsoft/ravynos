#import <Foundation/Foundation.h>

#if     GNUSTEP

#define PORT_LISTEN 1234
#define LOCATION200 @"http://localhost:1234/200"

@interface StatusServer : NSObject
{
  NSInputStream *ip;
  NSOutputStream *op;
  NSDictionary *responses;
  NSDictionary *headers;
  NSDictionary *commonHeaders;
  NSString *body;
  NSMutableData *request;
}

- (int) runTest;

@end

@implementation StatusServer

- (id)init
{
  if ((self = [super init]))
    {
      body = @"Hello\r\n";
      unsigned bodyLength = [body length];
      NSString *bl = [[NSNumber numberWithInt: bodyLength]
			    stringValue];
      NSString *date = [[NSCalendarDate date] description];
      // set up the dictionaries of responses and headers

      commonHeaders = [NSDictionary dictionaryWithObjectsAndKeys:
				    @"close",                @"Connection",
				    @"GNUstep test harness", @"Server",
				    @"text/plain",           @"Content-Type",
				    date,                    @"Date",
				    nil];
      [commonHeaders retain];
      
      responses = [NSDictionary dictionaryWithObjectsAndKeys:
				@"HTTP/1.1 200 OK", @"200",
				@"HTTP/1.1 201 Created", @"201",
				@"HTTP/1.1 204 No Content", @"204",
				@"HTTP/1.1 301 Moved Permanently", @"301",
				@"HTTP/1.1 302 Found", @"302",
				@"HTTP/1.1 303 See Other", @"303",
				@"HTTP/1.1 307 Temporary Redirect", @"307",
				@"HTTP/1.1 400 Bad Request", @"400",
				@"HTTP/1.1 401 Unauthorized", @"401",
				@"HTTP/1.1 403 Forbidden", @"403",
				@"HTTP/1.1 404 Not Found", @"404",
				@"HTTP/1.1 416 Requested Range Not Satisfiable",
				@"416",
				@"HTTP/1.1 500 Internal Server Error", @"500",
				@"HTTP/1.1 501 Not Implemented", @"501",
				nil];
      [responses retain];

      NSDictionary *header200 = [NSDictionary dictionaryWithObjectsAndKeys:
					      bl, @"Content-Length",
					      nil];
      //we can use this Location: header in header201 for the 30x, too
      NSDictionary *header201 = [NSDictionary dictionaryWithObjectsAndKeys:
					      LOCATION200, @"Location",
					      nil];
      NSDictionary *header401 = [NSDictionary dictionaryWithObjectsAndKeys:
					      @"Basic realm=\"GNUstep\"", 
					      @"WWW-Authenticate",
					      nil];

      headers = [NSDictionary dictionaryWithObjectsAndKeys:
			      header200, @"200",
			      header201, @"201",
			      header201, @"301",
			      header201, @"302",
			      header201, @"303",
			      header201, @"307",
			      header401, @"401",
			      nil];
      [headers retain];

      ip = nil;
      op = nil;
      request = nil;
    }
  return self;
}

- (void)dealloc
{
  [headers release];
  [commonHeaders release];
  [responses release];
  if (nil != ip)
    {
      [ip release];
    }
  if (nil != op)
    {
      [op release];
    }
  if (nil != request)
    {
      [request release];
    }

  [super dealloc];
}

- (int)runTest
{
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  NSRunLoop      *rl = [NSRunLoop currentRunLoop];
  NSHost         *host = [NSHost hostWithName: @"localhost"];
  NSStream       *serverStream;
  unsigned       port = [[defs stringForKey: @"Port"] intValue];

  if (0 == port) port = PORT_LISTEN;

  serverStream = [GSServerStream serverStreamToAddr: [host address]
				 port: port];

  if (nil == serverStream)
    {
      NSLog(@"Failed to create server stream");
      return 1;
    }

  [serverStream setDelegate: self];
  [serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [serverStream open];

  // run for one minute, then quit
  [rl runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 60]];
  return 0;
}

- (void) stream: (NSStream *)theStream handleEvent: (NSStreamEvent) streamEvent
{
  NSRunLoop *rl = [NSRunLoop currentRunLoop];

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
	    [(GSServerStream *)theStream acceptWithInputStream: &ip
			       outputStream: &op];

	    if (ip)
	      {
		RETAIN(ip);
		RETAIN(op);
		[ip scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
		[ip setDelegate: self];
		[op setDelegate: self];
		[ip open];
	      }
	    else
	      {
		NSLog(@"Received nothing from accept");
	      }
	  }
	else if (theStream == ip)
	  {
	    uint8_t	buf[1024];
	    unsigned	len;
	    BOOL	done = NO;

	    if (nil != request)
	      {
		[request release];
	      }
	    request = [[NSMutableData alloc] initWithCapacity: sizeof(buf)];
	    len = [ip read: buf maxLength: sizeof(buf)];
	    if (len > 0)
	      {
		const char	*bytes;

	        [request appendBytes: buf length: len];
	        len = [request length];
		bytes = (const char*)[request bytes];
		if (len > 3 && memcmp(bytes+len-4, "\r\n\r\n", 4) == 0)
		  {
		    done = YES;
		  }
	      }
	    else
	      {
		done = YES; 	// EOF or error
	      }
	    if (done == YES)
	      {
		[op open];
		[op scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
		[ip close];
		[ip removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
		ip = nil;
	      }
	  }
	break;
      }

    case NSStreamEventHasSpaceAvailable:
      {
	NSString *wholeReq, *req, *retCode, *hdr;
	NSMutableData *response;
	NSArray *components;
	NSDictionary *specificHeaders;
	NSEnumerator *en;

	int statusCode;

	NSAssert(op == theStream, NSInternalInconsistencyException);
	if (nil == request)
	  {
	    NSLog(@"Attempt to send response without a request");
	    return;
	  }
	wholeReq = [[[NSString alloc] initWithData: request 
				      encoding: NSASCIIStringEncoding]
				      autorelease];
	response = [[[NSMutableData alloc] init] autorelease];
	components = [wholeReq componentsSeparatedByString: @"\r\n"];
	// the actual request is the first line
	req = [components objectAtIndex: 0];

	if ([req rangeOfString: @"GET"].location == NSNotFound
	  && [req rangeOfString: @"HEAD"].location == NSNotFound)
	  {
	    retCode = @"501"; // HTTP 501: Not Implemented
	  }
	else
	  {
	    retCode = [[req componentsSeparatedByString: @" "]
			objectAtIndex: 1];
	    // trim the leading slash
	    retCode = [[retCode pathComponents] objectAtIndex: 1];
	  }
	if ([responses objectForKey: retCode] == nil)
	  {
	    retCode = @"404"; // HTTP 404: Not Found
	  }
	// build the response
	[response appendBytes: [[responses objectForKey: retCode] cString]
		  length: [[responses objectForKey: retCode] length]];
	[response appendBytes: "\r\n" length: 2];

	en = [commonHeaders keyEnumerator];
	while ((hdr = [en nextObject]) != nil)
	  {
	    [response appendBytes: [hdr cString] length: [hdr length]];
	    [response appendBytes: ": " length: 2];
	    hdr = [commonHeaders objectForKey: hdr];
	    [response appendBytes: [hdr cString] length: [hdr length]];
	    [response appendBytes: "\r\n" length: 2];
	  }

	specificHeaders = [headers objectForKey: retCode];
	if (specificHeaders != nil)
	  {
	    en = [specificHeaders keyEnumerator];
	    while ((hdr = [en nextObject]) != nil)
	      {
		[response appendBytes: [hdr cString] length: [hdr length]];
		[response appendBytes: ": " length: 2];
		hdr = [specificHeaders objectForKey: hdr];
		[response appendBytes: [hdr cString] length: [hdr length]];
		[response appendBytes: "\r\n" length: 2];
	      }
	  }
	[response appendBytes: "\r\n" length: 2];
	//do we need to add the body part?

	if([req rangeOfString: @"HEAD"].location == NSNotFound)
	  {
	    statusCode = [retCode intValue];
	
	    switch (statusCode)
	      {
	      case 200:
	      case 400:
	      case 401:
	      case 403:
	      case 404:
	      case 500:
	      case 501:
		[response appendBytes: [body cString] length: [body length]];
		break;
	      default:
		break;
	      }
	  }
	// send this response and close the stream
	[op write: [response bytes] maxLength: [response length]];
	[op close];
	[op removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	op = nil;
	break;
      }

    case NSStreamEventEndEncountered:
      {
	[theStream close];
	[theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	if (theStream == ip) ip = nil;
	if (theStream == op) op = nil;
	break;
      }

    case NSStreamEventErrorOccurred:
      {
	int code = [[theStream streamError] code];

	NSLog(@"Error %d on stream %p", code, theStream);
	[theStream close];
	[theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	if (theStream == ip) ip = nil;
	if (theStream == op) op = nil;
	break;
      }

    case NSStreamEventOpenCompleted:
      break;

    default:
      NSLog (@"Event %d on stream %p unknown", streamEvent, theStream);
      break;
    }
}

@end

int main (int argc, char **argv)
{
  int result;
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  
  result = [[[[StatusServer alloc] init] autorelease] runTest];

  [arp release]; arp = nil;
  return result;
}

#else

int main (int argc, char **argv)
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  
  NSLog(@"StatesServer not implemented on non-GNUstep systems");

  [arp release]; arp = nil;
  return 0;
}

#endif
