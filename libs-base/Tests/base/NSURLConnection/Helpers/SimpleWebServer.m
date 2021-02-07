/*
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 */

#import "SimpleWebServer.h"

/* the time step for the runloop */                                                                                                   
#define TIMING 0.1 

@interface SimpleWebServer (Private)
/**
 *  Starts a listening (server) stream.
 */
- (void)_openServerStream;

/**
 *  Stops the listening (server) stream.
 */
- (void)_closeServerStream;

/**
 *  Reset to prepare for the next request-response cycle.
 */
- (void)_resetCycle;

/**
 *  Opens the input _ip and output _op streams, schedules them
 *  on the current runloop. Requires the _ip and _op are not nil.
 */
- (void)_openIOStreams;

/**
 *  Closes the input _ip and output _op streams, unschedules them
 *  on the current runloop and clears that ivars.
 */
- (void)_closeIOStreams;

/**
 *  Tries to recognise if the request's bytes have been read (HTTP message's
 *  headers and body) and if so then it proceeds the request and produces
 *  a response. If the response is ready it returns YES.
 */
- (BOOL)_tryCaptured;

@end /* SimpleWebServer (Private) */

@implementation SimpleWebServer

- (void)dealloc
{
  _delegate = nil;
  DESTROY(_capture);
  DESTROY(_request);
  DESTROY(_response);
  [self _closeIOStreams];
  [self _closeServerStream];
  DESTROY(_address);
  DESTROY(_port);
  DESTROY(_secure);
  [super dealloc];
}

- (id)init
{
  if ((self = [super init]) != nil)
    {
      _debug = NO;
      _delegate = nil;
      _doRespond = NO;
      _done = NO;
      _canRespond = NO;
      _ip = nil;
      _op = nil;
    }

  return self;
}

/* getters */
- (NSString *)port
{
  NSStream *s = _serverStream;

  if (nil == s) s = _ip;
  if (nil == s) s = _op;
  if (nil == s) return nil;

  return [s streamStatus] == NSStreamStatusOpen ? _port : nil;
}
/* end of getters */

/* setters */
- (BOOL)setAddress:(NSString *)address
	      port:(NSString *)port
	    secure:(NSDictionary *)dict
{
  BOOL ret = NO;

  if (nil == _serverStream)
    {
      ASSIGN(_address, address);
      ASSIGN(_port, port);
      ASSIGN(_secure, dict);
      [self _openServerStream];
      if (nil != _serverStream)
	{
	  ret = YES;
	}
    }
  else
    {
      NSLog(@"%@: already started '%@' on '%@'",
	self, _serverStream, [NSThread currentThread]);
    }

  return ret;
}

- (void)setDebug:(BOOL)flag
{
  _debug = flag;
}

- (void)setDelegate:(id)delegate
{
  _delegate = delegate;
}

/* end of setters */

- (void)stop
{
  NSRunLoop *rl = [NSRunLoop currentRunLoop];;

  if (nil != _serverStream)
    {
      [self _closeServerStream];
      DESTROY(_address);
      DESTROY(_port);
      DESTROY(_secure);
    }

  [self _closeIOStreams];

  // give a time slice to free all resources
  [rl runUntilDate: [NSDate dateWithTimeIntervalSinceNow: TIMING]];
}

/* GSStream's delegate */

/* The method is derived/stolen from NSURL/Helpers/capture.m */
- (void) stream: (NSStream *)theStream handleEvent: (NSStreamEvent)streamEvent
{
  NSRunLoop	*rl = [NSRunLoop currentRunLoop];

// NSLog(@"Event %p %d", theStream, streamEvent);

  switch (streamEvent) 
    {
      case NSStreamEventHasBytesAvailable: 
	{
	  if (_ip == nil)
	    {
	      [self _resetCycle];
	      [(GSServerStream*)_serverStream acceptWithInputStream: &_ip
						       outputStream: &_op];
	      if (_ip)   // it is ok to accept nothing
		{
		  RETAIN(_ip);
		  RETAIN(_op);
		  [self _openIOStreams];
		  [self _closeServerStream];
		}
	    }
	  if (theStream == _ip)
	    {
	      _readable = YES;
	      while (_readable == YES)
		{
		  unsigned char	buffer[BUFSIZ];
		  int		readSize;

		  readSize = [_ip read: buffer maxLength: sizeof(buffer)];
		  if (readSize <= 0)
		    {
		      _readable = NO;
		    }
		  else
		    {
		      [_capture appendBytes: buffer length: readSize];
		    }
		}

	      _doRespond = [self _tryCaptured];
	      if (_doRespond)
		{
		  // reset the output stream to trigger polling
		  [_op write: NULL maxLength: 0];
		  if (YES == _debug)
		    {
		      NSLog(@"%@: about to send response\n%@", self, _response);
		    }
		}
	    }
	  break;
	}
      case NSStreamEventHasSpaceAvailable: 
	{
	  if (_doRespond && _canRespond)
	    {
	      NSMutableData *data;
	      NSString      *status;
	      NSData        *statusData;
	      char          *crlf = "\r\n";
	      id            content;
	      NSData        *contentData = nil;
	      NSUInteger    cLength = 0; // content-length
	      NSString      *connection;
	      BOOL          close = YES;

	      NSAssert(theStream == _op, @"Wrong stream for writing");
	      _writable = YES;

	      // adding the 'Connection' to the response
	      connection = [[_request headerNamed: @"Connection"] value];
	      if (nil == connection)
		{
		  connection = [[_request headerNamed: @"connection"] value];
		}
	      // if the client didn't supply the header 'Connection' or
	      // explicitly stated to close the current connection
	      close = (nil == connection
		|| [[connection lowercaseString] isEqualToString: @"close"]);

	      // adding the 'Content-Length' to the response
	      content = [_response content];
	      if ([content isKindOfClass: [NSString class]])
		{
		  contentData = [(NSString *)content
		    dataUsingEncoding: NSUTF8StringEncoding];
		}
	      else if ([content isKindOfClass: [NSData class]])
		{
		  contentData = (NSData *)content;
		}
	      else
		{
		  // yet unsupported
		}
	      if (nil != content)
		{
		  cLength = [contentData length];
		  if (cLength > 0)
		    {
                      NSString  *l;

                      l = [NSString stringWithFormat: @"%u", (unsigned)cLength];
		      [_response setHeader: @"Content-Length" 
				     value: l
				parameters: nil];
		    }
		}
	      if (cLength == 0)
		{
		  [_response setHeader: @"Content-Length" 
				 value: @"0"
			    parameters: nil];
		}

	      // adding the status line
	      status = [[_response headerNamed: @"http"] value];
	      if (nil == status)
		{
		  status = [[_response headerNamed: @"HTTP"] value];
		}
	      if (nil == status)
		{
		  status = [[_response headerNamed: @"Http"] value];
		}	      
	      statusData = [status dataUsingEncoding: NSUTF8StringEncoding];
	      data = [NSMutableData dataWithData: statusData];
	      [_response deleteHeaderNamed: @"http"];
	      [data appendBytes: crlf length: 2];

	      // actual sending
	      [data appendData: [_response rawMimeData]];
	      while (_writable == YES && _written < [data length])
		{
		  int	result = [_op write: [data bytes] + _written
				      maxLength: [data length] - _written];
		  
		  if (result <= 0)
		    {
		      _writable = NO;
		    }
		  else
		    {
		      _written += result;
		    }
		}
	      if (_written == [data length])
		{
		  if (close)
		    {
		      // if the client didn't supply the header 'Connection' or explicitly stated
		      // to close the current connection
		      [self _closeIOStreams];
		      [self _openServerStream];
		    }
		  // ready for another request-response cycle
		  [self _resetCycle];
		}
	    }

	  _canRespond = YES;

	  break;
	}
      case NSStreamEventEndEncountered: 
	{
	  if (theStream == _ip || theStream == _op)
	    {
	      [self _closeIOStreams];
	      [self _resetCycle];
	      [self _openServerStream];
	    }
	  else
	    {
	      [theStream close];
	      [theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	    }
	  break;
	}

      case NSStreamEventErrorOccurred: 
	{
	  int	code = [[theStream streamError] code];

	  if (theStream == _ip || theStream == _op)
	    {
	      [self _closeIOStreams];
	      [self _resetCycle];
	      [self _openServerStream];
	    }
	  else
	    {
	      [theStream close];
	      [theStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
	    }

	  NSAssert1(1, @"Error! code is %d", code);

	  break;
	}  

      default: 
	break;
    }
} 

/* end of GSStream's delegate */

@end /* SimpleWebServer */

@implementation SimpleWebServer (Private)

- (void)_openServerStream
{
  NSString *certFile = nil;
  NSString *keyFile = nil;
  NSRunLoop *rl;

  if (nil == _serverStream)
    {
      rl = [NSRunLoop currentRunLoop];

      _serverStream = [GSServerStream serverStreamToAddr: _address port: [_port intValue]];
      RETAIN(_serverStream);
      if (_serverStream == nil)
	{
	  NSLog(@"Failed to create server stream (address: %@, port: %@)", _address, _port);
	  return;
	}

      if (nil != _secure &&
	 (certFile = [_secure objectForKey: @"CertificateFile"]) != nil &&
	 (keyFile = [_secure objectForKey: @"KeyFile"]) != nil)
	{
	  _isSecure = YES;
	  [_serverStream setProperty: NSStreamSocketSecurityLevelTLSv1 forKey: NSStreamSocketSecurityLevelKey];
	  [_serverStream setProperty: certFile forKey: GSTLSCertificateFile];
	  [_serverStream setProperty: keyFile forKey: GSTLSCertificateKeyFile];
	}

      _capture = [NSMutableData new];

      [_serverStream setDelegate: self];
      [_serverStream scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
      [_serverStream open];
      if (YES == _debug)
	{
	  NSLog(@"%@: started '%@' on '%@'", self, _serverStream, [NSThread currentThread]);
	}
    }
  else
    {
      NSLog(@"%@: already started '%@' on '%@'", self, _serverStream, [NSThread currentThread]);
    }
}

- (void)_closeServerStream
{
  NSRunLoop *rl;

  if (nil != _serverStream)
    {
      rl = [NSRunLoop currentRunLoop];

      [_serverStream close];
      [_serverStream removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
      if (YES == _debug)
	{
	  NSLog(@"%@: stopped server stream %@", self, _serverStream);
	}
      DESTROY(_serverStream);
    }
}

- (void)_resetCycle
{
  _written = 0;
  _doRespond = NO;
  _done = NO;
  _canRespond = NO;
  [_capture setLength: 0];
}

- (void)_openIOStreams
{
  NSRunLoop *rl;

  if (_ip != nil && _op != nil)
    {
      rl = [NSRunLoop currentRunLoop];

      [_ip scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
      [_op scheduleInRunLoop: rl forMode: NSDefaultRunLoopMode];
      [_ip setDelegate: self];
      [_op setDelegate: self];
      [_ip open];
      [_op open];
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"%@: IO streams not properly initialized", self];
    }
}

- (void)_closeIOStreams
{
  NSRunLoop *rl;

  if (nil != _ip || nil != _op)
    {
      rl = [NSRunLoop currentRunLoop];

      [_ip close];
      [_ip removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];

      [_op close];
      [_op removeFromRunLoop: rl forMode: NSDefaultRunLoopMode];
      
      DESTROY(_ip);
      DESTROY(_op);
    }
}

- (BOOL)_tryCaptured
{
  BOOL ret = NO;
  NSRange r1;
  NSRange r2;
  NSString *headers;
  NSString *tmp1;
  NSString *tmp2;
  NSUInteger contentLength = 0;

  // the following chunk ensures that the captured data are written only
  // when all request's bytes are read... it waits for full headers and
  // reads the Content-Length's value then waits for the number of bytes
  // equal to that value is read
  tmp1 = [[NSString alloc] initWithData: _capture 
			       encoding: NSUTF8StringEncoding];
  // whether the headers are read
  if ((r1 = [tmp1 rangeOfString: @"\r\n\r\n"]).location != NSNotFound)
    {
      headers = [tmp1 substringToIndex: r1.location + 2];
      if ((r2 = [[headers lowercaseString] rangeOfString: @"content-length:"]).location != NSNotFound)
	{
	  tmp2 = [headers substringFromIndex: r2.location + r2.length]; // content-length:<tmp2><end of headers>
	  if ((r2 = [tmp2 rangeOfString: @"\r\n"]).location != NSNotFound)
	    {
	      // full line with content-length is present
	      tmp2 = [tmp2 substringToIndex: r2.location]; // number of content's bytes
	      contentLength = [tmp2 intValue];
	    }
	}
      else
	{
	  contentLength = 0; // no header 'content-length'
	}
      if (r1.location + 4 + contentLength == [_capture length]) // Did we get headers + body?
	{
	  // The request has been received
	  NSString *method = @"";
	  NSString *query = @"";
	  NSString *version = @"";
	  NSString *scheme = _isSecure ? @"https" : @"http";
	  NSString *path = @"";
	  NSData   *data;

	  // TODO: currently no checks
	  r2 = [headers rangeOfString: @"\r\n"]; 
	  while (r2.location == 0)
	    {
	      // ignore an empty line before the request line
	      headers = [headers substringFromIndex: 2];
	      r2 = [headers rangeOfString: @"\r\n"];
	    }
	  // the request line has been caught
	  tmp2 = [tmp1 substringFromIndex: r2.location + 2]; // whole request without the first line
	  data = [tmp2 dataUsingEncoding: NSUTF8StringEncoding];
	  _request = [GSMimeParser documentFromData: data];
	  RETAIN(_request);

	  // x-http-...
	  tmp2 = [headers substringToIndex: r2.location]; // the request line
	  tmp2 = [tmp2 stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceCharacterSet]];

	  // find the method
	  r2 = [tmp2 rangeOfString: @" "]; 
	  method = [[tmp2 substringToIndex: r2.location] uppercaseString];
	  tmp2 = [[tmp2 substringFromIndex: r2.location + 1]
		   stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceCharacterSet]];
	  
	  r2 = [tmp2 rangeOfString: @"?"]; // path?query
	  if (r2.location != NSNotFound)
	    {
	      // path?query
	      path = [tmp2 substringToIndex: r2.location];
	      tmp2 = [tmp2 substringFromIndex: r2.location + 1]; // without '?'
	      r2 = [tmp2 rangeOfString: @" "];
	      query = [tmp2 substringToIndex: r2.location];
	    }
	  else
	    {
	      // only path
	      r2 = [tmp2 rangeOfString: @" "];
	      path = [tmp2 substringToIndex: r2.location];
	    }
	  tmp2 = [[tmp2 substringFromIndex: r2.location + 1]
		       stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceCharacterSet]];
	  // tmp2 == 'HTTP/<version>'
	  version = [tmp2 substringFromIndex: 5];


	  [_request setHeader: @"x-http-method"
			value: method
		   parameters: nil];
	  
	  [_request setHeader: @"x-http-path"
			value: path
		   parameters: nil];
	  
	  [_request setHeader: @"x-http-query"
			value: query
		   parameters: nil];
	  
	  [_request setHeader: @"x-http-scheme"
			value: scheme
		   parameters: nil];

	  [_request setHeader: @"x-http-version"
			value: version
		   parameters: nil];
	  
	  if (YES == _debug)
	    {
	      NSLog(@"%@: got request\n%@", self, _request);
	    }
	  _response = [GSMimeDocument new];
	  if (nil != _delegate && [_delegate respondsToSelector: @selector(processRequest:response:for:)])
	    {
	      ret = [_delegate processRequest: _request response: _response for: self];
	    }
	  if (!ret)
	    {
	      DESTROY(_response);
	      _response = [GSMimeDocument new];
	      [_response setHeader: @"HTTP" value: @"HTTP/1.1 204 No Content" parameters: nil];
	      [_response setHeader: @"Content-Length" value: @"0" parameters: nil];
	      ret = YES;
	    }
	}
    }
  DESTROY(tmp1);

  return ret;
}

@end /* SimpleWebServer (Private) */
