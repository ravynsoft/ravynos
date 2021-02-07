/*
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 */

#import "TestWebServer.h"
#import "RequestHandler.h"

@interface TestWebServer (Private)

/**
 *  Starts the detached thread. It is the entry point of the thread.
 */
- (void)_startDetached:(id)extra;

/**
 *  Starts the SimpleWebServer.
 */
- (void)_startHTTPServer:(id)extra;

/**
 *  Stops the SimpleWebServer.
 */
- (void)_stopHTTPServer;


@end

/* default 'constants' */
#define DEFAULTADDRESS @"localhost"
#define DEFAULTPORT @"1234"
#define DEFAULTMODE NO
#define DEFAULTLOGIN @"login"
#define DEFAULTPASSWORD @"password"

/* lock's conditions */
#define READY   0 /* the initial condition */
#define STARTED 1 /* the SimpleWebServer has been started */
#define STOPPED 2 /* the SimpleWebServer has been stopped */

/* the time step for the runloop */
#define TIMING 0.1
/* the maximum duration of running of the SimpleWebServer instance
   after which the server must be shut down */
#define MAXDURATION 3.0

@implementation TestWebServer

- (id) init
{
  return [self initWithAddress: DEFAULTADDRESS
			  port: DEFAULTPORT
			  mode: DEFAULTMODE
			 extra: nil];
}

- (id)initWithAddress:(NSString *)address
		 port:(NSString *)port
		 mode:(BOOL)detached
		extra:(id)extra
{
  if ((self = [super init]) != nil)
    {
      _debug = NO;
      _address = [[NSHost hostWithName: address] address];
      RETAIN(_address);
      ASSIGN(_port, port);
      ASSIGN(_extra, extra);
      _password = nil;
      _login = nil;
      _isSecure = NO;

      if ([extra isKindOfClass: [NSDictionary class]])
	{
	  NSDictionary *d = extra;
	  NSString *proto;

	  if ((_password = [d objectForKey: @"Password"]) != nil)
	    {
	      RETAIN(_password);
	    }
	  if ((_login = [d objectForKey: @"Login"]) != nil)
	    {
	      RETAIN(_login);
	    }
	  if ((proto = [d objectForKey: @"Protocol"]) != nil &&
	     [[proto lowercaseString] isEqualToString: @"https"])
	    {
	      _isSecure = YES;
	    }
	}

      if (nil == _login)
	{
	  _login = DEFAULTLOGIN; RETAIN(_login);
	}
      if (nil == _password)
	{
	  _password = DEFAULTPASSWORD; RETAIN(_password);
	}

      _traversalMap = [[NSMutableDictionary alloc]
			initWithObjectsAndKeys:
			  [Handler200 class], @"200",
			[Handler204 class], @"204",
			[Handler301 class], @"301",
			[Handler400 class], @"400",
			[Handler401 class], @"401",
			[Handler402 class], @"402",
			[Handler404 class], @"404",
			[Handler405 class], @"405",
			[Handler409 class], @"409",
			[Handler500 class], @"500",
			[Handler505 class], @"505",
			[Handler507 class], @"507",
			[HandlerIndex class], @"index",
			nil];

      _lock = [[NSConditionLock alloc] initWithCondition: READY];
      _threadToQuit = NO;
      if (detached)
	{
	  _serverThread = [[NSThread alloc] initWithTarget: self
						  selector: @selector(_startDetached:)
						    object: extra];
	}
      else
	{
	  _serverThread = nil;
	}
    }

  return self;
}

- (void)dealloc
{
  DESTROY(_lock);
  DESTROY(_address);
  DESTROY(_port);
  DESTROY(_extra);
  DESTROY(_login);
  DESTROY(_password);
  _delegate = nil;
  DESTROY(_traversalMap);
  [super dealloc];
}

- (void) start: (id)extra
{
  if ([_server port] != nil)
    {
      if (_debug)
	{
	  NSWarnMLog(@"SimpleWebServer already started");
	}
      return;
    }

  if (nil != _serverThread)
    {
      if (_debug)
	NSLog(@"Waiting for startup");
      if (![_serverThread isExecuting])
	{
	  NSTimeInterval duration = 0.0;

	  [_serverThread start];

	  // wait for the SimpleWebServer is started
	  while (![_lock tryLockWhenCondition: STARTED]
	    && duration < MAXDURATION)
	    {
	      [[NSRunLoop currentRunLoop]
		runUntilDate: [NSDate dateWithTimeIntervalSinceNow: TIMING]];
	      duration += TIMING;
	    }
	  [_lock unlock];
	  if (duration >= MAXDURATION
	    && nil != _delegate
	    && [_delegate respondsToSelector:
	      @selector(timeoutExceededByHandler:)])
	    {
	      [_delegate timeoutExceededByHandler: self];
	      [self stop];
	    }
	}
      else
	{
	  NSWarnMLog(@"the detached thread is already started");
	}
    }
  else
    {
      [self _startHTTPServer: extra];
    }
}

- (void) stop
{
  if ([_server port] == nil)
    {
      if (YES == _debug)
	{
	  NSWarnMLog(@"SimpleWebServer already stopped");
	}
      return;
    }
  if (nil != _serverThread)
    {
      if ([_serverThread isExecuting])
	{
	  NSTimeInterval duration = 0.0;
	  _threadToQuit = YES; // this makes the detached thread quiting from it's runloop

	  // wait for the SimpleWebServer is stopped
	  while(![_lock tryLockWhenCondition: STOPPED] &&
		duration < MAXDURATION)
	    {
	      [[NSRunLoop currentRunLoop]
		runUntilDate: [NSDate dateWithTimeIntervalSinceNow: TIMING]];
	      duration += TIMING;
	    }
	  [_lock unlockWithCondition: READY];
	  if (duration >= MAXDURATION &&
	     nil != _delegate &&
	     [_delegate respondsToSelector: @selector(timeoutExceededByHandler:)])
	    {
	      [_delegate timeoutExceededByHandler: self];
	    }
	}
    }
  else
    {
      [self _stopHTTPServer];
    }
}

- (BOOL) processRequest:(GSMimeDocument *)request
               response:(GSMimeDocument *)response
		    for:(SimpleWebServer *)server
{
  NSString *path;
  NSString *component;
  NSEnumerator *en;
  BOOL ret = NO;
  id handler;

  // analyze the path to infer what the client wants
  path = [[request headerNamed:@"x-http-path"] value];

  // traverse over the path to search the right handler
  en = [[path pathComponents] objectEnumerator];
  while((component = [en nextObject]) != nil)
    {
      if ((handler = [_traversalMap objectForKey: component]) != nil)
	{
	  // the handler has been found
	  break; // while
	}
    }

  if (nil == component)
    {
      // no component found... default 204
      component = @"204";
      handler = [_traversalMap objectForKey: component];
    }

  if (handler == [handler class])
    {
      // it is a Class not an instance... instantiate and substitute
      // instead of the Class
      Class cls = (Class)handler;
      handler = [[cls alloc] init];
      [_traversalMap setObject: handler forKey: component];
    }

  // set the handler
  [handler setDelegate: _delegate];
  [handler setLogin: _login];
  [handler setPassword: _password];
  if ([handler respondsToSelector: @selector(setURLString:)])
    {
      if ([handler isKindOfClass: [HandlerIndex class]])
	{
	  NSString *urlString = [NSString stringWithFormat: @"%@://%@:%@/",
				     _isSecure ? @"https" : @"http",
					  _address,
					  _port];
	
	  [(HandlerIndex *)handler setURLString: urlString];
	}
      else if ([handler isKindOfClass: [Handler301 class]])
	{
	  // by default http://localhost:1235/
	  NSString *port = [NSString stringWithFormat: @"%u", [_port intValue] + 1]; // the TestWebServer's port + 1
	  NSString *urlString = [NSString stringWithFormat: @"%@://%@:%@/",
				     _isSecure ? @"https" : @"http",
					  _address,
					  port];

	  [(Handler301 *)handler setURLString: urlString];
	}
    }

  // TODO: add session-related conditions here
  [handler prehandleRequest: request
		   response: response
			for: self];

  // the main job
  ret = [handler handleRequest: request
		      response: response
			   for: self];
  // TODO: analyze 'ret'... if NO a default handler must be called

  // TODO: add session-related conditions here
  [handler posthandleRequest: request
		    response: response
			 for: self];


  return ret;
}

/* end of SimpleWebServer delegate */


/* getters */

- (NSString *)address
{
  return _address;
}

- (NSString *)port
{
  return _port;
}

- (NSString *)login
{
  return _login;
}

- (NSString *)password
{
  return _password;
}

- (BOOL)isSecure
{
  return _isSecure;
}

/* end of getters */

/* setters */
- (void)setDelegate:(id)delegate
{
  _delegate = delegate;
}

- (void)setDebug:(BOOL)mode
{
  _debug = mode;
}

/* end of setters */

@end /* TestWebServer */


@implementation TestWebServer (Private)

- (void) _startHTTPServer: (id)extra
{
  NSString *certPath;
  NSString *keyPath;
  NSDictionary *secure = nil;
  BOOL status;

  _server = [SimpleWebServer new];
  [_server setDebug: _debug];
  [_server setDelegate: self];
  if (_isSecure)
    {
      NSHost *h = [NSHost hostWithAddress: _address];
      NSHost *l = [NSHost hostWithName: @"localhost"];

      if ([h isEqual: l])
	{
	  certPath = [[NSBundle bundleForClass: [self class]]
		       pathForResource: @"testCert"
				ofType: @"pem"];
	  keyPath = [[NSBundle bundleForClass: [self class]]
		      pathForResource: @"testKey"
			       ofType: @"pem"];
	  secure = [NSDictionary dictionaryWithObjectsAndKeys:
				   certPath, @"CertificateFile",
				 keyPath, @"KeyFile",
				 nil];
	}
      else
	{
	  [NSException raise: NSInternalInconsistencyException
	    format: @"The server hasn't run. Address %@ is not localhost (%@)",
	    _address, l];
	  // NOTE: generate corresponding certificates for any address differing
	  //       from localhost
	}
    }

  if (_debug)
    {
      NSLog(@"Starting web server with address %@, port %@ %@",
	_address, _port, secure ? @" with TLS" : @"");
    }
  status = [_server setAddress: _address port: _port secure: secure];
  if (!status)
    {
      [NSException raise: NSInternalInconsistencyException
	format: @"The server hasn't run. Perhaps the port %@ is invalid",
	DEFAULTPORT];
    }
}

- (void)_stopHTTPServer
{
  if (nil != _server && [_server port] != nil)
    {
      [_server stop]; // shut down the server
      if (YES == _debug)
	{
	  NSLog(@"%@: stopped SimpleWebServer %@", self, _server);
	}
      DESTROY(_server);
    }
}

- (void)_startDetached:(id)extra
{
  CREATE_AUTORELEASE_POOL(arp);
  NSTimeInterval duration = 0.0;

  [_lock lockWhenCondition: READY];
  [self _startHTTPServer: extra];
  [_lock unlockWithCondition: STARTED];

  if (YES == _debug)
    {
      NSLog(@"%@: enter into runloop in detached thread %@", self, [NSThread currentThread]);
    }

  while(!_threadToQuit && duration < MAXDURATION)
    {
      [[NSRunLoop currentRunLoop]
	runUntilDate: [NSDate dateWithTimeIntervalSinceNow: TIMING]];
      duration += TIMING;
    }

  if (YES == _debug)
    {
      NSLog(@"%@: exit from runloop in detached thread %@", self, [NSThread currentThread]);
    }

  if (duration >= MAXDURATION &&
     nil != _delegate &&
     [_delegate respondsToSelector: @selector(timeoutExceededByHandler:)])
    {
      [_delegate timeoutExceededByHandler: self];
    }

  [_lock lockWhenCondition: STARTED];
  [self _stopHTTPServer];
  [_lock unlockWithCondition: STOPPED];

  DESTROY(arp);
}



@end /* TestWebServer (Private) */
