/** -*- objc -*-
 *
 * Author: Sergei Golovin <Golovin.SV@gmail.com>
 *
 */

#import "RequestHandler.h"
#import "TestWebServer.h"

/**
 *  The abstract class TestHandler. All custom handlers must be derived from it.
 */
@implementation TestHandler

- (id)init
{
  if((self = [super init]) != nil)
    {
      _debug = NO;
      _delegate = nil; 
      _isAuthorized = NO;
    }

  return self;
}

- (void)dealloc
{
  _delegate = nil;
  [super dealloc];
}

/* getters */
/* end of getters */

/* setters */
- (void)setDebug:(BOOL)flag
{
  _debug = flag;
}

- (void)setDelegate:(id)delegate
{
  _delegate = delegate;
}

- (void)setLogin:(NSString *)login
{
  ASSIGN(_login, login);
}

- (void)setPassword:(NSString *)password
{
  ASSIGN(_password, password);
}

/* end of setters */

/* RequestHandler */

- (BOOL)prehandleRequest:(GSMimeDocument *)request
		response:(GSMimeDocument *)response
		     for:(TestWebServer *)server
{
  _isAuthorized = NO; // TODO: move to something like -[reset]
  _done = NO;

  return YES;
}

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  NSString *auth;
  NSString *path;
  BOOL ret = NO;



  if(YES == _debug)
    {
      NSLog(@"%@: BEGIN\n%@", self, request);
    }

  if(nil != _delegate && [_delegate respondsToSelector: @selector(handler:gotRequest:with:)])
    {
      [_delegate handler: self gotRequest: request with: server];
    }

  // analyze what the client wants
  path = [[request headerNamed:@"x-http-path"] value];

  if([path rangeOfString: @"withoutauth"].location == NSNotFound)
    {
      auth = [[request headerNamed: @"Authorization"] value];
      if(!auth) // not authorized
	{
	  [response setHeader: @"http" value: @"HTTP/1.1 401 Unauthorized" parameters: nil];
	  [response setHeader: @"WWW-Authenticate" value: @"Basic realm='TestSuit'" parameters: nil];
	  [response setContent: @"Please give login and password"];

	  if(nil != _delegate && [_delegate respondsToSelector: @selector(handler:willSendUnauthorized:with:)])
	    {
	      [_delegate handler: self willSendUnauthorized: response with: server];
	    }

	  if(YES == _debug)
	    {
	      NSLog(@"%@: about to send Unauthorized\n%@", self, response);
	    }

	  return YES;
	}
      else // the auth header is set
	{
	  NSArray *credentials;
	  
	  credentials = [auth componentsSeparatedByString: @" "];
	  if([[credentials objectAtIndex:0] isEqualToString: @"Basic"])
	    {
	      credentials = [[GSMimeDocument decodeBase64String: [credentials objectAtIndex:1]]
			      componentsSeparatedByString: @":"];
	      if([[credentials objectAtIndex:0] isEqualToString: _login] &&
		 [[credentials objectAtIndex:1] isEqualToString: _password])
		{
		  if(YES == _debug)
		    {
		      NSLog(@"%@: got valid credentials", self);
		    }

		  if(nil != _delegate && [_delegate respondsToSelector: @selector(handler:gotAuthorized:with:)])
		    {
		      [_delegate handler: self gotAuthorized: request with: server];
		    }

		  // sets the flag
		  _isAuthorized = YES;
		}
	    }
	  else
	    {
	      [response setHeader: @"http" value: @"HTTP/1.1 401 Unauthorized" parameters: nil];
	      [response setHeader: @"WWW-Authenticate" value: @"Basic realm='TestSuit'" parameters: nil];
	      [response setContent: @"Try another login or password"];
	  
	      if(nil != _delegate && [_delegate respondsToSelector: @selector(handler:willSendUnauthorized:with:)])
		{
		  [_delegate handler: self willSendUnauthorized: response with: server];
		}

	      if(YES == _debug)
		{
		  NSLog(@"%@: about to send Unauthorized\n%@", self, response);
		}
      
	      return YES;
	    }
	}
    } // ! withoutauth
  else
    {
      // just sets the flag
      _isAuthorized = YES;
    }

  return ret;
}

- (BOOL)posthandleRequest:(GSMimeDocument *)request
		 response:(GSMimeDocument *)response
		      for:(TestWebServer *)server
{
  BOOL ret = NO;

  if(YES == _done && 
     nil != _delegate &&
     [_delegate respondsToSelector: @selector(handler:willSend:with:)])
    {
      [_delegate handler: self willSend: response with: server];

      ret = YES;
    }

  // TODO: may be to move at another place?
  if(YES == _debug)
    {
      NSLog(@"%@: about to send\n%@", self, response);
    }

  return ret;
}

/* end of RequestHandler */


@end /* TestHandler */

@implementation Handler200

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);
      if(isToSetResponse)
        {
	  [response setContent: @"OK"];
	}
      [response setHeader: @"HTTP" value: @"HTTP/1.1 200 OK" parameters:nil];
      [response setHeader: @"content-type" value: @"plain/text" parameters: nil];
      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler200 */

@implementation Handler204

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      [response setContent: @""];
      [response setHeader: @"HTTP" value: @"HTTP/1.1 204 No Content" parameters:nil];

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler204 */

@implementation Handler301

- (void)dealloc
{
  DESTROY(_URLString);
  [super dealloc];
}

- (void)setURLString:(NSString *)URLString
{
  ASSIGN(_URLString, URLString);
}

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      if(nil != _URLString)
	{
	  BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);
	  [response setHeader: @"HTTP" value: @"HTTP/1.1 301 Moved Permanently" parameters: nil];
	  [response setHeader: @"Location" value: _URLString parameters: nil];
	  if(isToSetResponse)
	    {
	      NSString *method = [[request headerNamed: @"x-http-method"] value];
	      if(![method isEqualToString: @"HEAD"])
		{
		  NSString *note = [NSString stringWithFormat: @"<html><head></head><body>Redirect to <a href='%@'/></body></html>",
					     _URLString];
		  [response setContent: note];
		}
	    }
	}
      else
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"%@: URL isn't set", self];
	}
      
      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler301 */

@implementation Handler400

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      if(isToSetResponse)
        {
	  [response setContent: @"You have issued a request with invalid data"];
        }
      [response setHeader:@"HTTP" value:@"HTTP/1.1 400 Bad Request" parameters:nil];

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler400 */

@implementation Handler401

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value: @"HTTP/1.1 401 Unauthorized" parameters:nil];
      [response setHeader:@"WWW-Authenticate" value: @"Basic realm='TestSuit'" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"Invalid login or password"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler401 */

@implementation Handler402

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value:@"HTTP/1.1 402 Payment required" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"Check your balance"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler402 */

@implementation Handler404

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value:@"HTTP/1.1 404 Not found" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"The resource you are trying to access is not found"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler404 */

@implementation Handler405

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value:@"HTTP/1.1 405 Method not allowed" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"You can't do that"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler405 */

@implementation Handler409

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value:@"HTTP/1.1 409 Conflict" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"The resource you are trying to access is busy"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler409 */

@implementation Handler500

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value:@"HTTP/1.1 500 Internal Server Error" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"System error"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler500 */

@implementation Handler505

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value:@"HTTP/1.1 505 Network Protocol Error" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"There is network protocol inconsistency"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler505 */

@implementation Handler507

- (BOOL) handleRequest: (GSMimeDocument*)request
	      response: (GSMimeDocument*)response
		   for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);

      [response setHeader:@"HTTP" value: @"HTTP/1.1 507 Insufficient storage" parameters:nil];
      if(isToSetResponse)
        {
          [response setContent: @"Insufficient storage"];
        }

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* Handler507 */

@implementation HandlerIndex

- (void)dealloc
{
  DESTROY(_URLString);
  [super dealloc];
}

- (void)setURLString:(NSString *)URLString
{
  ASSIGN(_URLString, URLString);
}

- (BOOL) handleRequest: (GSMimeDocument*)request
	       response: (GSMimeDocument*)response
		    for: (TestWebServer*)server
{
  BOOL ret = [super handleRequest: request response: response for: server];
  if(NO == ret)
    {
      if(nil != _URLString)
	{
	  BOOL isToSetResponse = (nil == [response content]) || ([[response content] length] == 0);
	  if(isToSetResponse)
	    {
	      NSString *pattern = @"<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'><head></head><body>\
                           <a href='%@%@'>This page</a><br>		\
                           <a href='%@%@'>Without authentication</a><br> \
                           <a href='%@%@'>301</a><br>			\
                           <a href='%@%@'>400</a><br>			\
                           <a href='%@%@'>401</a><br>			\
                           <a href='%@%@'>402</a><br>			\
                           <a href='%@%@'>404</a><br>			\
                           <a href='%@%@'>405</a><br>			\
                           <a href='%@%@'>409</a><br>			\
                           <a href='%@%@'>500</a><br>			\
                           <a href='%@%@'>505</a><br>			\
                           <a href='%@%@'>507</a><br>			\
                           </body></html>";
	      NSString *index = [NSString stringWithFormat: pattern,
					  _URLString, @"index",
					  _URLString, @"withoutauth",
					  _URLString, @"301",
					  _URLString, @"400",
					  _URLString, @"401",
					  _URLString, @"402",
					  _URLString, @"404",
					  _URLString, @"405",
					  _URLString, @"409",
					  _URLString, @"500",
					  _URLString, @"505",
					  _URLString, @"507"];
	      [response setContent: index];
	      [response setHeader: @"content-type" value: @"text/html" parameters: nil];
	      [response setHeader: @"HTTP" value: @"HTTP/1.1 200 OK" parameters:nil];
	    }
	}
      else
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"%@: URL isn't set", self];
	}

      _done = YES;
      ret = YES;
    }
 
  return ret;
}

@end /* HandlerIndex */

