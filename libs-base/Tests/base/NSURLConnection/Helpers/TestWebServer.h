
/** -*- objc -*-
 *
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 *
 *  TestWebServer is intended to be used in the tests where a web service is
 *  needed. The class's instance is instantiated with the method
 *  -[initWithAddress:port:mode:extra] and then started with the call -[start:].
 *  When started the TestWebServer starts in it's turn a SimpleWebServer instance
 *  in the same thread unless it is initialized with the argument 'mode' set to
 *  'YES' to run it in a detached thread.
 *
 *  It is designed to call the delegate's callbacks (if implemented) of
 *  the TestWebServerDelegate protocol for every request made to
 *  the SimpleWebServer's assigned address on the SimpleWebServer's assigned port
 *  (by default localhost and 1234 respectively). However it currently doesn't
 *  handle any request on it's own. Instead the class uses a collection of
 *  handlers. Every handler makes a custom response. So the TestWebServer only
 *  has to dispatch a request to a corresponding handler and then to send back
 *  the handler's response to the SimpleWebServer instance. See RequestHandler
 *  for more information.
 *
 *  TestWebServer can be supplied with additional parameters via the argument
 *  'extra' of the initializer -[initWithAddress:port:mode:extra].
 *  See that method's description for the currently supported key-value pairs.
 *
 *  The pattern of use:
 *  ----------------------------------------------------------------------------
 *  NSDictionary *extra = [NSDictionary dictionaryWithObjectsAndKeys:
 *                                      @"https", @"Protocol",
 *                                      nil];
 *  TestWebServer *server = [[TestWebServer alloc] initWithAddress: @"localhost"
 *                                                            port: @"1234"
 *                                                            mode: NO
 *                                                           extra: extra];
 *  ADelegateClass *del = [ADelegateClass new];
 *  [server setDelegate: del];
 *  [server start: extra];
 *  ....
 *  <runloop>
 *  ....
 *  [server stop];
 *  DESTROY(server);
 *  DESTROY(del);
 *  ----------------------------------------------------------------------------
 *
 *
 *  The TestWebServer can response to the following resource paths:
 *
 *      /index
 *            displays the page with all supported resources
 *      /withoutauth/
 *            if it is a part of the request's URL then the TestWebServer
 *            will NOT check authentication/authorization of the client.
 *
 *      /204/
 *      /301/
 *      /400/
 *      /401/
 *      /402/
 *      /404/
 *      /405/
 *      /409/
 *      /500/
 *      /505/
 *      /507/
 *            a try to access to this resources will lead the TestWebServer
 *            to produce one of the pre-defined responses with the status code
 *            is equal to the corresponding number.
 *            The pre-defined responses are:
 *              204 "" (empty string)
 *              301 "Redirect to <URL>"
 *                  Returns in the header 'Location' a new <URL> which by default
 *                  has the same protocol, address but the port is incremented by
 *                  1 (e.g. http://localhost:1235/ with other parameters set to
 *                  their default values).
 *              400 "You have issued a request with invalid data"
 *              401 "Invalid login or password"
 *              403 "Check your balance"
 *              404 "The resource you are trying to access is not found"
 *              405 "You can't do that"
 *              409 "The resource you are trying to access is busy"
 *              500 "System error"
 *              505 "There is network protocol inconsistency"
 *              507 "Insufficient storage"
 *
 *  If you want more verbosity call the method -[setDebug: YES].
 *
 */ 

#import "SimpleWebServer.h"
#import <Foundation/Foundation.h>

@class RequestHandler;

@interface TestWebServer : NSObject <SimpleWebServerDelegate>
{
  /* the debug mode flag */
  BOOL _debug;
  /* the IP address to attach to... see DEFAULTADDRESS at the beginning
   * of the implementaion */
  NSString *_address;
  /* the port to listen on... see DEFAULTPORT in the beginning
   * of the implementaion */
  NSString *_port;
  /* whether the TestWebServer listens for HTTPS requests */
  BOOL _isSecure;
  /* the login for basic authentication */
  NSString *_login;
  /* the password for basic authentication */
  NSString *_password;
  /* holds the extra argument which is for a future use...
   * Currently it is expected to be a dictionary with predetermined 
   * keys ( see the description of -[initWithAddress:port:mode:extra:]) */
  id _extra;
  /* the flag used as a trigger to stop the detached thread */
  BOOL _threadToQuit;
  /* holds the SimpleWebServer accepting incoming connections */
  SimpleWebServer *_server;
  /* holds the detached thread the SimpleWebServer is running on...
   * nil if it runs in the same thread as the calling code */
  NSThread *_serverThread;
  /* the delegate ... NOT RETAINED...
   * see below the protocol TestWebServerDelegate */
  id _delegate;
  /* the lock used for synchronization between threads mainly
   * to wait for the SimpleWebServer is started/stopped...
   * the condition list:
   *     READY   - the initial condition
   *     STARTED - the SimpleWebServer has been started;
   *     STOPPED - the SimpleWebServer has been stopped;
   */
  NSConditionLock *_lock;
  /* the traversal map used to pick a right handler from the request's path */
  NSMutableDictionary *_traversalMap;
}

/**
 *  Initializes the intance with default parameters.
 */
- (id)init;

/**
 *  Initializes an instance with it's SimpleWebServer accepting connection on
 *  the specified address and port. The mode decides whether to run
 *  the SimpleWebServer on the detached thread (NO means it won't be detached).
 *  The argument extra is for future enhancement and could be of the class
 *  NSDictionary whose key-value pairs set various parameters.
 *  The current code supports the following extra keys:
 *     'Login'        - the login for basic authentication
 *     'Password'     - the password for basic authentication
 *     'Protocol'     - 'http' means waiting for HTTP requests
 *                      'https' - for HTTPS requests
 */
- (id)initWithAddress:(NSString *)address
                 port:(NSString *)port
                 mode:(BOOL)detached
                extra:(id)extra;
- (void)dealloc;

/**
 *  Starts the SimpleWebServer accepting incoming connections.
 *  The supplied argument is for future enhancement and currently has no meaning.
 *  It isn't retained by the method.
 */
- (void)start:(id)extra;

/**
 *  Stops the SimpleWebServer. The instance ceases to accept incoming connections.
 */
- (void)stop;

/* SimpleWebServerDelegate */
/**
 *  The main job of request-response cycle is done here. The method is called
 *  when all request's bytes are read. It must be supplied with the incoming
 *  request and the response document which is about to be sent back to the web
 *  client. The handling code within the method would change the supplied response.
 */
- (BOOL) processRequest:(GSMimeDocument *)request
               response:(GSMimeDocument *)response
		    for:(SimpleWebServer *)server;
/* end of SimpleWebServerDelegate */

/* getters */

/**
 *  Returns the address which SimpleWebServer is bound to.
 */
- (NSString *)address;

/**
 *  Returns the port which the SimpleWebServer listens on.
 */
- (NSString *)port;

/**
 *  Returns the login for basic authentication.
 */
- (NSString *)login;

/**
 *  Returns the password for basic authentication.
 */
- (NSString *)password;

/**
 *  Returns YES if the instance waits for HTTPS requests.
 */
- (BOOL)isSecure;

/* end of getters */

/* setters */

/**
 *  Sets the delegate implementing TestWebServerDelegate protocol.
 *  The argument isn't retained by the method.
 */
- (void)setDelegate:(id)delegate;

/**
 *  Sets the debug mode (more verbose).
 */
- (void)setDebug:(BOOL)mode;

/* end of setters */

@end /* TestWebServer */

/**
 *  The protocol's methods are called during processing of a request.
 */
@protocol TestWebServerDelegate

/**
 *  Called by the handler when it has got the supplied request
 *  from the supplied TestWebServer instance.
 */
- (void)handler:(id)handler
     gotRequest:(GSMimeDocument *)request
	   with:(TestWebServer *)server;

/**
 *  Called by the handler when it is going to send the response
 *  on an unauthorized request (an invalid request or no credentials are supplied)
 *  via the supplied TestWebServer.
 */
- (void)handler:(id)handler
willSendUnauthorized:(GSMimeDocument *)response
	   with:(TestWebServer *)server;

/**
 *  Called by the handler when it has got the supplied request
 *  with valid credentials from the supplied TestWebServer instance.
 */
- (void)handler:(id)handler
  gotAuthorized:(GSMimeDocument *)request
	   with:(TestWebServer *)server;

/**
 *  Called by the handler when it is going to send the response
 *  on any request except an unauthorized one via the supplied TestWebServer.
 */
- (void)handler:(id)handler
       willSend:(GSMimeDocument *)response
	   with:(TestWebServer *)server;

/**
 *  Called when the timeout is exceeded.
 */
- (void)timeoutExceededByHandler:(id)handler;

@end /* TestWebServerDelegate */
