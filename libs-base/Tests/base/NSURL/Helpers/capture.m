#if	GNUSTEP
#include	<Foundation/Foundation.h>

@interface	TestClass : NSObject
{
  NSOutputStream *op;
  NSInputStream *ip;
  NSMutableData	*capture;
  unsigned	written;
  BOOL		readable;
  BOOL		writable;
  BOOL          isSecure; /* whether to use a secure TLS/SSL connection */
  BOOL         doRespond; /* the request is read */
  BOOL              done; /* the response is written */
  NSString         *file; /* the file to write the captured request */
}
- (id)initWithSecure:(BOOL)flag;
- (int) runTest;
@end

@implementation	TestClass

- (void) dealloc
{
  RELEASE(capture);
  RELEASE(op);
  RELEASE(ip);
  DESTROY(file);
  [super dealloc];
}

- (id)initWithSecure:(BOOL)flag
{
  if((self = [super init]) != nil)
    {
      isSecure = flag;
      capture = [NSMutableData new];
      doRespond = NO;
      done = NO;
      file = nil;
    }

 return self;
}

- (id) init
{
  return [self initWithSecure: NO];
}

- (int) runTest
{
  NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
  NSRunLoop		*rl = [NSRunLoop currentRunLoop];
  NSHost		*host = [NSHost hostWithName: @"localhost"];
  NSStream		*serverStream;
  int			port = [[defs stringForKey: @"Port"] intValue];

  isSecure = [[defs stringForKey: @"Secure"] boolValue];

  if (port == 0) port = 1234;

  file = [defs stringForKey: @"FileName"];
  RETAIN(file);
  if (file == nil) file = @"Capture.dat";

  serverStream = [GSServerStream serverStreamToAddr: [host address] port: port];
  if (serverStream == nil)
    {
      NSLog(@"Failed to create server stream");
      return 1;
    }
  if(isSecure)
    {
      [serverStream setProperty: NSStreamSocketSecurityLevelTLSv1 forKey: NSStreamSocketSecurityLevelKey];
      [serverStream setProperty: @"testCert.pem" forKey: GSTLSCertificateFile];
      [serverStream setProperty: @"testKey.pem" forKey: GSTLSCertificateKeyFile];
    }

  [serverStream setDelegate: self];
  [serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
  [serverStream open];

  while(!done)
    {
      [rl runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]];
    }

  return 0;
}

- (void) stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
  NSRunLoop	*rl = [NSRunLoop currentRunLoop];
  NSString      *resp = @"HTTP/1.0 204 Empty success response\r\n\r\n";
// NSLog(@"Event %p %d", theStream, streamEvent);

  switch (streamEvent) 
    {
      case NSStreamEventHasBytesAvailable: 
	{
	  if (ip == nil)
	    {
	      [(GSServerStream*)theStream acceptWithInputStream: &ip
						   outputStream: &op];
	      if (ip)   // it is ok to accept nothing
		{
		  RETAIN(ip);
		  RETAIN(op);
		  [ip scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
		  [op scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
		  [ip setDelegate: self];
		  [op setDelegate: self];
		  [ip open];
		  [op open];
		  [theStream close];
		  [theStream removeFromRunLoop: rl
				       forMode: NSDefaultRunLoopMode];
		}
	    }
	  if (theStream == ip)
	    {
	      NSRange r1;
	      NSRange r2;
	      NSString *headers;
	      NSString *tmp1;
	      NSString *tmp2;
	      NSUInteger contentLength;

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
		  else
		    {
		      [capture appendBytes: buffer length: readSize];
		    }
		}

	      // the following chunk ensures that the captured data are written only
	      // when all request's bytes are read... it waits for full headers and
	      // reads the Content-Length's value then waits for the number of bytes
	      // equal to that value is read
	      tmp1 = [[NSString alloc] initWithData: capture 
					   encoding: NSUTF8StringEncoding];
	      // whether the headers are read
	      if((r1 = [tmp1 rangeOfString: @"\r\n\r\n"]).location != NSNotFound)
		{
		  headers = [tmp1 substringToIndex: r1.location + 2];
		  if((r2 = [[headers lowercaseString] rangeOfString: @"content-length:"]).location != NSNotFound)
		    {
		      tmp2 = [headers substringFromIndex: r2.location + r2.length]; // content-length:<tmp2><end of headers>
		      if((r2 = [tmp2 rangeOfString: @"\r\n"]).location != NSNotFound)
			{
			  // full line with content-length is present
			  tmp2 = [tmp2 substringToIndex: r2.location]; // number of content's bytes
			  contentLength = [tmp2 intValue];
			  if(r1.location + 4 + contentLength == [capture length]) // Did we get headers + body?
			    {
			      // full request is read so write it
			      if ([capture writeToFile: file atomically: YES] == NO)
				{
				  NSLog(@"Unable to write captured data to '%@'", file);
				}

			      doRespond = YES; // allows to write the response
			      theStream = op;
			    }
			}
		    }
		}
	      DESTROY(tmp1);
	    }
	  if(!doRespond) break;
	}
      case NSStreamEventHasSpaceAvailable: 
	{
	  if(doRespond)
	    {
	      // if we have read all request's bytes
	      NSData	*data;

	      NSAssert(theStream == op, @"Wrong stream for writing");
	      writable = YES;

	      data = [resp dataUsingEncoding: NSASCIIStringEncoding];
	      while (writable == YES && written < [data length])
		{
		  int	result = [op write: [data bytes] + written
				     maxLength: [data length] - written];
		  
		  if (result <= 0)
		    {
		      writable = NO;
		    }
		  else
		    {
		      written += result;
		    }
		}
	      if (written == [data length])
		{
		  [ip close];
		  [ip removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];

		  [op close];
		  [op removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];

		  done = YES;
		}
	    }
	  break;
	}

      case NSStreamEventEndEncountered: 
        {
	  [theStream close];
	  [theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	  NSLog(@"Server close %p", theStream);
	  break;
	}

      case NSStreamEventErrorOccurred: 
	{
	  int	code = [[theStream streamError] code];

	  [theStream close];
	  [theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	  NSAssert1(1, @"Error! code is %d", code);
	  break;
	}  

      default: 
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
