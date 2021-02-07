/** -*- objc -*-
 *
 * Author: Sergei Golovin <Golovin.SV@gmail.com>
 *
 */

/*
 *  TODO: there is lack of session handling
 */

#import "TestWebServer.h"

/**
 *  The protocol RequestHandler describes how a request receiver should interact
 *  with a request handler.
 */
@protocol RequestHandler

/**
 *  Does all job needed to prepare for request handling. Returns YES if
 *  the handler still can proceed the supplied request later.
 *  The reason this method was introduced is possible session
 *  handling in the future.
 */
- (BOOL)prehandleRequest:(GSMimeDocument *)request
		response:(GSMimeDocument *)response
		     for:(TestWebServer *)server;

/**
 *  Makes a custom response. Returns YES if the handler has recognized
 *  it is it's responsibility to handle the request.
 */ 
- (BOOL) handleRequest:(GSMimeDocument *)request
	      response:(GSMimeDocument *)response
		   for:(TestWebServer *)server;

/**
 *  Does all job needed to be done after request handling. Returns NO
 *  if the handler has discovered some inconsistency after handling.
 *  The reason this method was introduced is possible session
 *  handling in the future.
 */
- (BOOL)posthandleRequest:(GSMimeDocument *)request
		 response:(GSMimeDocument *)response
		      for:(TestWebServer *)server;

@end /* RequestHandler */

/**
 *  The abstract class TestHandler. Custom handlers must be derived from it.
 *  The class implements the protocol RequestHandler's methods that are called 
 *  by an instance of TestWebServer. The protocol's method -[handleRequest:response:for:]
 *  produces a custom response. It also calls it's delegate's callback methods of
 *  the protocol TestWebServerDelegate (if implemented) during proceeding of a request.
 *
 *  On use of the -[handleRequest:response:for:]
 *  ---------------------------------------------
 *
 *  The TestHandler's implementation of the -[handleRequest:response:for:] checks
 *  whether the supplied request is authorized. It returns YES if no processing is
 *  required (by a child) so the response must be returned intact. If the method
 *  returns NO then the child MAY process the request on it's own. If the request is
 *  authorized the method also sets the flag _isAuthorized to YES.
 *  
 *  Child's implementation MUST call this super's one as in the following
 *  example code:
 *  ------------------------------------------------------------------------
 *  - (BOOL) handleRequest: (GSMimeDocument*)request
 *                response: (GSMimeDocument*)response
 *                     for: (TestWebServer *)server
 *  {
 *     BOOL ret = [super handleRequest: request response: response for: server];
 *     if(NO == ret)
 *       {
 *	    // process on it's own with possible checking _isAuthorized if needed
 *	    ...
 *          _done = YES;
 *	    ret = YES;
 *       }
 *
 *    return ret;
 *  }
 *  ------------------------------------------------------------------------
 *
 *  The method recognizes the case when the request's path contains '/withoutauth'.
 *  In this case no authorization is required and the method -[handleRequest:response:for:]
 *  just returns NO and sets the flag _isAuthorized to YES.
 *
 */
@interface TestHandler : NSObject <RequestHandler>
{
  /* the debug mode switch */
  BOOL _debug; 
  /* the handler's delegate... NOT RETAINED */
  id _delegate;
  /* the flag signifies the handler has got an authorized request */
  BOOL _isAuthorized;
  /* the login for basic authentication */
  NSString *_login;
  /* the password for basic authentication */
  NSString *_password;
  /* the flag for successful handling...
     can be used in the -[posthandleRequest:response:for:] */
  BOOL _done;
}

- (id)init;
- (void)dealloc;

/* getters */
/* end of getters */

/* setters */
/**
 *  Switches the debug mode on/off.
 */
- (void)setDebug:(BOOL)flag;

/**
 *  Sets the delegate implementing TestWebServerDelegate protocol.
 */
- (void)setDelegate:(id)delegate;

/**
 *  Sets the login for requests with authorization.
 */
- (void)setLogin:(NSString *)login;

/**
 *  Sets the password for requests with authorization.
 */
- (void)setPassword:(NSString *)password;

/* end of setters */

@end /* TestHandler */

/* The handler collection */

/**
 *  The handler returns the status code 200 for any request.
 *  The response's content is 'OK'. The status line is the default one
 *  returned by the TestWebServer. The header 'Content-Type' is set to 'plain/text'.
 */
@interface Handler200 : TestHandler
{
}

@end /* Handler200 */

/**
 *  The handler returns the status code 204 for any request.
 *  The response's content is empty. The status line is 'HTTP/1.1 204 No Content'.
 */
@interface Handler204 : TestHandler
{
}

@end /* Handler204 */

/**
 *  The handler returns the status code 301 for any request.
 *  The response's content is an html page like 'Redirect to <a href="URL">'.
 *  The status line is 'HTTP/1.1 301 Moved permanently'. It also sets
 *  the response's header 'Location' to the custom URL stored by the
 *  ivar _URLString. If the -[setURLString:] wasn't called with a proper URL
 *  string before the first request arrives then the handler will raise
 *  NSInternalInconsistencyException.
 */
@interface Handler301 : TestHandler
{
  NSString *_URLString;
}

- (void)dealloc;

/**
 *  Sets the URL to which the handler should redirect.
 */
- (void)setURLString:(NSString *)URLString;

@end /* Handler301 */

/**
 *  The handler returns unconditionally the status code 400 for any request.
 *  The response's content is 'You have issued a request with invalid data'.
 *  The status line is 'HTTP/1.1 400 Bad Request'.
 */
@interface Handler400 : TestHandler
{
}

@end /* Handler400 */

/**
 *  The handler returns unconditionally the status code 401 for any request.
 *  The response's content is 'Invalid login or password'.
 *  The status line is 'HTTP/1.1 401 Unauthorized'. It also sets the response's
 *  header 'WWW-Authenticate' to the value "Basic realm='TestSuit'".
 */
@interface Handler401 : TestHandler
{
}

@end /* Handler401 */

/**
 *  The handler returns unconditionally the status code 402 for any request.
 *  The response's content is 'Check your balance'.
 *  The status line is 'HTTP/1.1 402 Payment required'.
 */
@interface Handler402 : TestHandler
{
}

@end /* Handler402 */

/**
 *  The handler returns unconditionally the status code 404 for any request.
 *  The response's content is 'The resource you are trying to access is not found'.
 *  The status line is 'HTTP/1.1 404 Not found'.
 */
@interface Handler404 : TestHandler
{
}

@end /* Handler404 */

/**
 *  The handler returns unconditionally the status code 405 for any request.
 *  The response's content is 'You can't do that'.
 *  The status line is 'HTTP/1.1 405 Method not allowed'.
 */
@interface Handler405 : TestHandler
{
}

@end /* Handler405 */

/**
 *  The handler returns unconditionally the status code 409 for any request.
 *  The response's content is 'The resource you are trying to access is busy'.
 *  The status line is 'HTTP/1.1 409 Conflict'.
 */
@interface Handler409 : TestHandler
{
}

@end /* Handler409 */

/**
 *  The handler returns unconditionally the status code 500 for any request.
 *  The response's content is 'System error'.
 *  The status line is 'HTTP/1.1 500 Internal Server Error'.
 */
@interface Handler500 : TestHandler
{
}

@end /* Handler500 */

/**
 *  The handler returns unconditionally the status code 505 for any request.
 *  The response's content is 'There is network protocol inconsistency'.
 *  The status line is 'HTTP/1.1 505 Network Protocol Error'.
 */
@interface Handler505 : TestHandler
{
}

@end /* Handler505 */

/**
 *  The handler returns unconditionally the status code 507 for any request.
 *  The response's content is 'Insufficient storage'.
 *  The status line is 'HTTP/1.1 507 Insufficient storage'.
 */
@interface Handler507 : TestHandler
{
}

@end /* Handler507 */

/**
 *  The handler returns the index page with a list of possible URLs for any request.
 *  The URLs are constructed from the base URL stored by the ivar _URLString. If
 *  the -[setURLString:] wasn't called with a proper URL string before the first
 *  request arrives then the handler will raise NSInternalInconsistencyException.
 */
@interface HandlerIndex : TestHandler
{
  NSString *_URLString;
}

- (void)dealloc;

/**
 *  Sets the URL to which the handler should redirect.
 */
- (void)setURLString:(NSString *)URLString;

@end /* HandlerIndex */

/* The end of the handler collection */
