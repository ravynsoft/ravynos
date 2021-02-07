/*
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 */

#import "NSURLConnectionTest.h"

/* the runloop's time slice */
#define TIMING 0.1
/* the max duration of a test */
#define MAXDURATION 3.0

@interface NSURLConnectionTest (Private)

/**
 *  The method tries to guess the request which the test case should make.
 *  It recognizes as an argument an NSDictionary with custom variable parts
 *  of connecting process (see the class's description). It also can be supplied
 *  with the exact request which must be made during test execution.
 *  The result of the method is stored in the ivar _request.
 */
- (void)_makeRequest:(id)extra;

/**
 *  The method analyzes the ivar _request and tries to guess the execution path
 *  inferring from it the reference flag set. If there is a deviation from the guessed
 *  execution path the caller can supply NSDictionary as the argument 'extra' with
 *  the key 'ReferenceFlags' is set to have a custom NSDictionary as a value explicitly
 *  setting the particular flags (see the class's description).
 */
- (void)_makeReferenceFlags:(id)extra;

@end /* NSURLConnectionTest (Private) */

@implementation NSURLConnectionTest

+ (Class) testWebServerClass
{
  return [TestWebServer class];
}

/* The flag map connects flags's string names with flags... used to log actions */
static NSMapTable *_flagMap = nil;

+ (void)initialize
{
  if (nil == _flagMap)
    {
      _flagMap = NSCreateMapTable(NSObjectMapKeyCallBacks, 
				   NSIntegerMapValueCallBacks,
				   0);
      NSMapInsert(_flagMap, @"SENTREQUEST",     (void *)1);
      NSMapInsert(_flagMap, @"AUTHORIZED",      (void *)2);
      NSMapInsert(_flagMap, @"NOTAUTHORIZED",   (void *)4);
      NSMapInsert(_flagMap, @"GOTUNAUTHORIZED", (void *)8);
      NSMapInsert(_flagMap, @"GOTREQUEST",      (void *)16);
      NSMapInsert(_flagMap, @"SENTRESPONSE",    (void *)32);
      NSMapInsert(_flagMap, @"GOTRESPONSE",     (void *)64);
      NSMapInsert(_flagMap, @"GOTCONTENT",      (void *)128);
      NSMapInsert(_flagMap, @"GOTFINISH",       (void *)256);
      NSMapInsert(_flagMap, @"GOTFAIL",         (void *)512);
      NSMapInsert(_flagMap, @"GOTREDIRECT",     (void *)1024);
    }
}

- (id)init
{
  if ((self = [super init]) != nil)
    {
      _conn = nil;
      _error = nil;
      _server = nil;
      _auxServer = nil;
      _received = nil;
      _request = nil;
      _redirectRequest = nil;
      _expectedStatusCode = 204;
      _expectedContent = nil;
    }

  return self;
}

// super's -[dealloc] calls the -[tearDownTest:]
// so place clearing there

- (void)setUpTest:(id)extra
{
  [super setUpTest: extra];

  [self _makeRequest: extra];
  [self _makeReferenceFlags: extra];
  _received = [NSMutableData new];
}

- (void)startTest:(id)extra
{
  CREATE_AUTORELEASE_POOL(arp);
  NSTimeInterval duration = 0.0;

  if (YES == _debug)
    {
      NSLog(@"%@: started with request:\n%@", self, _request);
      [self logFlags];

      NSMutableSet	*s = [[NSProcessInfo processInfo] debugSet];
      [s addObject: @"NSURLConnection"];
      [s addObject: @"NSURLProtocol"];
      [s addObject: @"NSStream"];
      [s addObject: @"NSRunLoop"];
    }

  _conn = [[NSURLConnection alloc] initWithRequest: _request
					  delegate: self];

  while (!_done && !_failed)
    {
      [[NSRunLoop currentRunLoop]
		runUntilDate: [NSDate dateWithTimeIntervalSinceNow: TIMING]];
      duration += TIMING;
      if (duration >= MAXDURATION)
	{
	  _failed = YES;
	}
    }
  if (YES == _debug)
    {
      NSLog(@"%@: stopped with request:\n%@", self, _request);
      [self logFlags];
    }
  DESTROY(arp);
}

- (void)tearDownTest:(id)extra
{
  BOOL isToStop = YES; // whether to stop the main TestWebServer instance
  BOOL isToStopAux = YES; // whether to stop the auxilliary TestWebServer instance
  if ([_extra isKindOfClass: [NSDictionary class]])
    {
      NSDictionary *d = _extra;
      isToStop = ([d objectForKey: @"Instance"] == nil);
      isToStopAux = ([d objectForKey: @"AuxInstance"] == nil &&
		     [[d objectForKey: @"IsAuxilliary"] isEqualToString: @"YES"]);
    }

  if (isToStop)
    {
      if (nil != _server)
	{
	  [_server stop];
	}
    }
  if (isToStopAux)
    {
      if (nil != _auxServer)
	{
	  [_auxServer stop];
	}
    }

  DESTROY(_server);
  DESTROY(_auxServer);
  DESTROY(_received);
  DESTROY(_request);
  DESTROY(_redirectRequest);
  DESTROY(_expectedContent);
  [_conn cancel];
  DESTROY(_conn);
  DESTROY(_error);
  
  [super tearDownTest: extra];
}

- (BOOL)isSuccess
{
  BOOL ret = [super isSuccess];


  if (!ret)
    {
      /* log the flags that differ */
      NSString *key;
      NSUInteger flag;
      NSMapEnumerator en = NSEnumerateMapTable(_flagMap);
      while (NSNextMapEnumeratorPair(&en, (void **)&key, (void **)&flag))
	{
	  if ([self isReferenceFlagSet: flag] != [self isFlagSet: flag])
	    {
	      NSLog(@"ERR: %@", key);
	    }
	}
      NSEndMapTableEnumeration(&en);

      if (_failed)
	{
	  NSLog(@"FAILED for unknown reason possibly not related to the test (e.g. a timer has expired)");
	}
    }

  return ret;
}

- (void)logFlags
{
  NSString *value;
  NSString *key;
  NSUInteger flag;
  NSMapEnumerator en = NSEnumerateMapTable(_flagMap);
  while (NSNextMapEnumeratorPair(&en, (void **)&key, (void **)&flag))
    {
      if ([self isFlagSet: flag])
	{
	  value = @"YES";
	}
      else
	{
	  value = @"NO";
	}

      NSLog(@"ACTUAL:    %@ -> %@", key, value);

      if ([self isReferenceFlagSet: flag])
	{
	  value = @"YES";
	}
      else
	{
	  value = @"NO";
	}

      NSLog(@"REFERENCE: %@ -> %@", key, value);
    }
  NSEndMapTableEnumeration(&en);  
}

- (NSError*) error
{
  return _error;
}

/* TestWebServerDelegate */
- (void)handler:(id)handler
     gotRequest:(GSMimeDocument *)request
	   with:(TestWebServer *)server
{
  NSString *method = [_request HTTPMethod];
  NSData *content = [request convertToData];

  [self setFlags: SENTREQUEST];
  if (YES == _debug)
    {
      NSLog(@"%@: set SENTREQUEST (-[%@])", self, NSStringFromSelector(_cmd));
    }

  // TODO: more comparisons of _request and request
  if ([method isEqualToString: @"POST"] ||
     [method isEqualToString: @"PUT"])
    {
      if ([content isEqualToData: [_request HTTPBody]])
	{
	  [self setFlags: GOTREQUEST];
	  if (YES == _debug)
	    {
	      NSLog(@"%@: set GOTREQUEST (-[%@])", self, NSStringFromSelector(_cmd));
	    }
	}
    }
  else
    {
      [self setFlags: GOTREQUEST];
      if (YES == _debug)
	{
	  NSLog(@"%@: set GOTREQUEST (-[%@])", self, NSStringFromSelector(_cmd));
	}
    }
}

- (void)handler:(id)handler
willSendUnauthorized:(GSMimeDocument *)response
	   with:(TestWebServer *)server
{
  [self setFlags: NOTAUTHORIZED];
  if (YES == _debug)
    {
      NSLog(@"%@: set NOTAUTHORIZED (-[%@])", self, NSStringFromSelector(_cmd));
    }
}

- (void)handler:(id)handler
  gotAuthorized:(GSMimeDocument *)request
	   with:(TestWebServer *)server
{
  [self setFlags: AUTHORIZED];
  if (YES == _debug)
    {
      NSLog(@"%@: set AUTHORIZED (-[%@])", self, NSStringFromSelector(_cmd));
    }
}

- (void)handler:(id)handler
       willSend:(GSMimeDocument *)response
	   with:(TestWebServer *)server
{
  [self setFlags: SENTRESPONSE];
  if (YES == _debug)
    {
      NSLog(@"%@: set SENTRESPONSE (-[%@])", self, NSStringFromSelector(_cmd));
    }
}

- (void)timeoutExceededByHandler:(id)handler
{
  _failed = YES;
}
/* end of TestWebServerDelegate */

/* NSURLConnectionDelegate */
- (NSURLRequest *)connection:(NSURLConnection *)connection
             willSendRequest:(NSURLRequest *)request
            redirectResponse:(NSURLResponse *)redirectResponse
{
  if ([redirectResponse isKindOfClass: [NSHTTPURLResponse class]])
    {
      if ([(NSHTTPURLResponse *)redirectResponse statusCode] == _redirectStatusCode)
	{
	  [self setFlags: GOTREDIRECT];
	  if (YES == _debug)
	    {
	      NSLog(@"%@: set GOTREDIRECT (-[%@])", self, NSStringFromSelector(_cmd));
	    }
	}
    }
  else
    {
      [self setFlags: GOTREDIRECT];
      if (YES == _debug)
	{
	  NSLog(@"%@: set GOTREDIRECT (-[%@])", self, NSStringFromSelector(_cmd));
	}
    }

  return _redirectRequest;
}

- (void)connection:(NSURLConnection *)connection
didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
  [self setFlags: GOTUNAUTHORIZED];
  if (YES == _debug)
    {
      NSLog(@"%@: set GOTUNAUTHORIZED (-[%@])", self, NSStringFromSelector(_cmd));
    }

  if ([challenge previousFailureCount] == 0)
    {
      NSURLCredential *cred;
      // TODO: _auxServer?
      NSString *login = [_server login];
      NSString *password = [_server password];

      if (nil == login)
	{
	  if ([_extra isKindOfClass: [NSDictionary class]])
	    {
	      login = [(NSDictionary *)_extra objectForKey: @"Login"];
	    }
	}
      if (nil == password)
	{
	  if ([_extra isKindOfClass: [NSDictionary class]])
	    {
	      password = [(NSDictionary *)_extra objectForKey: @"Password"];
	    }
	}
      cred = [NSURLCredential credentialWithUser: login
					password: password
				     persistence: NSURLCredentialPersistenceNone];
      [[challenge sender] useCredential: cred
	     forAuthenticationChallenge: challenge];
    }
  else
    {
      [[challenge sender] cancelAuthenticationChallenge: challenge];
    }
}

- (void)connection:(NSURLConnection *)connection
didReceiveResponse:(NSURLResponse *)response
{
  if (YES == _debug)
    {
      if ([response isKindOfClass: [NSHTTPURLResponse class]])
	{
	  NSLog(@"%@: received response (-[%@]):\nStatus Code: %i",
            self, NSStringFromSelector(_cmd),
            (int)[(NSHTTPURLResponse *)response statusCode]);
	}
    }

  if ([response isKindOfClass: [NSHTTPURLResponse class]])
    {
      if ([(NSHTTPURLResponse *)response statusCode] == _expectedStatusCode)
	{
	  [self setFlags: GOTRESPONSE];
	  if (YES == _debug)
	    {
	      NSLog(@"%@: set GOTRESPONSE (-[%@])", self, NSStringFromSelector(_cmd));
	    }
	}
    }
  else
    {
      [self setFlags: GOTRESPONSE];
      if (YES == _debug)
	{
	  NSLog(@"%@: set GOTRESPONSE (-[%@])", self, NSStringFromSelector(_cmd));
	}
    }
}

- (void)connection:(NSURLConnection *)connection
    didReceiveData:(NSData *)data
{
  [_received appendData: data];
  if (YES == _debug)
    {
      NSLog(@"%@: received data '%@' (-[%@])", self, data, NSStringFromSelector(_cmd));
    }
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
  if (nil != _expectedContent &&
     [_received isEqualToData: _expectedContent])
    {
      [self setFlags: GOTCONTENT];
      if (YES == _debug)
	{
	  NSLog(@"%@: set GOTCONTENT (-[%@])", self, NSStringFromSelector(_cmd));
	}
    }

  [self setFlags: GOTFINISH];
  if (YES == _debug)
    {
      NSLog(@"%@: set GOTFINISH (-[%@])", self, NSStringFromSelector(_cmd));
    }

  _done = YES;
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
  ASSIGN(_error, error);

  /*  if ((nil != _expectedContent &&
      [_received isEqualToData: _expectedContent]) ||
     (nil == _expectedContent && [_received length] == 0))
    {
      [self setFlags: GOTCONTENT];
    }
  */

  [self setFlags: GOTFAIL];
  if (YES == _debug)
    {
      NSLog(@"%@: set GOTFAIL (-[%@])", self, NSStringFromSelector(_cmd));
      NSLog(@"%@: error %@", self, error);
    }

  _done = YES;
}

/* end of NSURLConnectionDelegate */

@end /* NSURLConnectionTest */

@implementation NSURLConnectionTest (Private)

- (void)_makeRequest:(id)extra
{
  BOOL isOwnServer = YES;
  BOOL isOwnAuxServer = NO;
  NSString *tmp;
  BOOL isDetached = NO;
  TestWebServer *instance = nil;
  TestWebServer *auxInstance = nil;
  NSString *protocol = nil;
  NSString *address = nil;
  NSString *auxPort = nil;
  NSString *port = nil;
  //  NSString *login = nil;
  //  NSString *password = nil;
  NSString *path = nil;
  NSString *redirectPath = nil;
  NSString *statusCode = nil;
  NSString *redirectCode = nil;
  NSString *method = nil;
  id payload = nil;
  id content = nil;
  NSURL *url;
  NSDictionary *d = nil;

  if ([extra isKindOfClass: [NSDictionary class]])
    {
      d = extra;

      instance = [d objectForKey: @"Instance"];
      if (nil != instance)
	{
	  NSString *value;

	  protocol = [instance isSecure] ? @"https" : @"http";
	  if ((value = [d objectForKey: @"Protocol"]) == nil ||
	     ![[value lowercaseString] isEqualToString: protocol])
	    {
	      d = [d mutableCopy];
	      [(NSMutableDictionary *)d setObject: protocol forKey: @"Protocol"];
	      [d autorelease];
	    }

	  address = [instance address];
	  port = [instance port];
	  ASSIGN(_server, instance);
	  [_server setDelegate: self];
	  [_server setDebug: _debug];
	  isOwnServer = NO;
	}
      else
	{
	  protocol = [[d objectForKey: @"Protocol"] lowercaseString];
	  address = [d objectForKey: @"Address"];
	  port = [d objectForKey: @"Port"];
	}

      auxInstance = [d objectForKey: @"AuxInstance"];
      if (nil != auxInstance)
	{
	  auxPort = [auxInstance port];
	  ASSIGN(_auxServer, auxInstance);
	  [_auxServer setDelegate: self];
	  [_auxServer setDebug: _debug];
	  isOwnAuxServer = NO;
	}

      if (isOwnServer)
	{
	  if ((tmp = [d objectForKey: @"IsDetached"]) != nil)
	    {
	      if ([tmp isEqualToString: @"YES"])
		{
		  isDetached = YES;
		}
	      else
		{
		  isDetached = NO;
		}
	    }
	}

      isOwnAuxServer
	= [[d objectForKey: @"IsAuxilliary"] isEqualToString: @"YES"]
	&& nil == _auxServer;
      if (isOwnAuxServer)
	{
	  auxPort = [d objectForKey: @"AuxPort"];
	}

      path = [d objectForKey: @"Path"];
      if (isOwnAuxServer || nil != _auxServer)
	{
	  redirectPath = [d objectForKey: @"RedirectPath"];
	}
      statusCode = [d objectForKey: @"StatusCode"];
      // TODO: conditions?
      redirectCode = [d objectForKey: @"RedirectCode"];
      method = [d objectForKey: @"Method"];
      payload = [d objectForKey: @"Payload"];
      content = [d objectForKey: @"Content"];
    } // Is extra NSDictionary?
  else if ([extra isKindOfClass: [NSURLRequest class]])
    {
      ASSIGN(_request, extra);
    }

  if (nil == _request)
    {
      if (nil == protocol)
	{
	  protocol = @"http";
	}
      if (nil == address)
	{
	  address = @"localhost";
	}
      if (nil == port)
	{
	  port = @"1234";
	}
      if (nil == auxPort)
	{
	  auxPort = @"1235";
	}
      if (nil == path)
	{
	  path = @"/";
	}
      if ((isOwnAuxServer || nil != _auxServer) && nil == redirectPath)
	{
	  redirectPath = path;
	}
      if (nil != redirectPath && nil == redirectCode)
	{
	  _redirectStatusCode = 301;
	}
      if (nil == statusCode)
	{
	  _expectedStatusCode = 204;
	}
      else
	{
	  _expectedStatusCode = [statusCode intValue];
	  if (_expectedStatusCode == 0)
	    {
	      [NSException raise: NSInvalidArgumentException
	  	format: @"Invalid expected 'StatusCode' supplied %@",
		statusCode];
	    }
	}
      if (nil == redirectCode)
	{
	  _redirectStatusCode = 301;
	}
      else
	{
	  _redirectStatusCode = [redirectCode intValue];
	  if (_redirectStatusCode == 0)
	    {
	      [NSException raise: NSInvalidArgumentException
		format: @"Invalid expected 'RedirectCode' supplied %@",
		redirectCode];
	    }
	}

      if (nil == method)
	{
	  method = @"GET";
	}

      if (![path hasPrefix: @"/"])
	{
	  // path MUST begin with '/'
	  path = [@"/" stringByAppendingString: path];
	}

      if (![redirectPath hasPrefix: @"/"])
	{
	  // path MUST begin with '/'
	  redirectPath = [@"/" stringByAppendingString: redirectPath];
	}

      url = [NSURL URLWithString: 
        [NSString stringWithFormat:
	  @"%@://%@:%@%@", protocol, address, auxPort, redirectPath]];
      _redirectRequest = [NSMutableURLRequest requestWithURL: url];
      RETAIN(_redirectRequest);

      url = [NSURL URLWithString: 
	[NSString stringWithFormat:
	  @"%@://%@:%@%@", protocol, address, port, path]];
      _request = [NSMutableURLRequest requestWithURL: url];
      RETAIN(_request);

      [(NSMutableURLRequest *)_request setHTTPMethod: method];
      if (nil != payload)
	{
	  if ([payload isKindOfClass: [NSString class]])
	    {
	      payload = [payload dataUsingEncoding: NSUTF8StringEncoding];
	    }
	  if (![payload isKindOfClass: [NSData class]])
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"invalid payload"];
	    }
	  [(NSMutableURLRequest *)_request setHTTPBody: payload];
	}

      if (nil != content)
	{
	  if ([content isKindOfClass: [NSString class]])
	    {
	      content = [content dataUsingEncoding: NSUTF8StringEncoding];
	    }
	  if (![content isKindOfClass: [NSData class]])
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"invalid content"];
	    }
	  ASSIGN(_expectedContent, content);
	}
      if (nil != _expectedContent && nil == statusCode)
	{
	  _expectedStatusCode = 200;
	}
    }

  if (isOwnServer)
    {
      _server = [[TestWebServer alloc] initWithAddress: address
						  port: port
						  mode: isDetached
						 extra: d];
      [_server setDebug: _debug];
      [_server setDelegate: self];
      [_server start: d];
    }
  if (isOwnAuxServer)
    {
      _auxServer = [[TestWebServer alloc] initWithAddress: address
						     port: auxPort
						     mode: isDetached
						    extra: d];
      [_auxServer setDebug: _debug];
      [_auxServer setDelegate: self];
      [_auxServer start: d];
    }
}

- (void)_makeReferenceFlags:(id)extra
{
  // trying to guess the execution path
  // and to infer the reference flag set
  if (nil != _request)
    {
      // default reference set
      [self setReferenceFlags: SENTREQUEST];
      [self setReferenceFlags: NOTAUTHORIZED];
      [self setReferenceFlags: AUTHORIZED];
      [self setReferenceFlags: GOTUNAUTHORIZED];
      [self setReferenceFlags: GOTREQUEST];
      [self setReferenceFlags: SENTRESPONSE];
      [self setReferenceFlags: GOTRESPONSE];
      if (nil != _expectedContent)
	{
	  [self setReferenceFlags: GOTCONTENT];
	}
      [self setReferenceFlags: GOTFINISH];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"no request"];
    }

  if ([extra isKindOfClass: [NSDictionary class]])
    {
      // make correction in the reference flag set
      NSDictionary *d = extra;
      if ((d = [d objectForKey: @"ReferenceFlags"]) != nil)
	{
	  NSEnumerator *en = [d keyEnumerator];
	  NSString *key;
	  NSString *value;
	  SEL sel;

	  while ((key = [en nextObject]) != nil)
	    {
	      value = [d objectForKey: key];
	      NSString *originalKey = nil;
	      NSUInteger flag = NORESULTS;
	      // NSUInteger flag = (NSUInteger)NSMapGet(_flagMap, key);
	      if (NSMapMember(_flagMap,
                key, (void **)&originalKey, (void **)&flag))
		{
		  if ([value isEqualToString: @"YES"])
		    {
		      sel = @selector(setReferenceFlags:);
		    }
		  else
		    {
		      sel = @selector(unsetReferenceFlags:);
		    }

		  [self performSelector: sel withObject: (void *)flag];
		}
	      else
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"flag codes corrupted (key %@)", key];
		}

	    }
	}
    }
}

@end /* NSURLConnectionTest (Private) */
