/** -*- objc -*-
 *
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 * 
 *  The class is intended to test the class NSURLConnection. It is designed to start
 *  a TestWebServer instance and make NSURLConnection to it getting TestWebServerDelegate's
 *  and NSURLConnection's callbacks. As TestCase's child it has two flag sets, the actual
 *  one and the reference one (See TestCase.h). It sets/unsets flags of the actual set on
 *  the execution path at the 'checkpoints' listed below as macros.
 *
 *  The method -[isSuccess] is called when the NSURLConnection finishes (fails). It makes
 *  a comparison between the two flag sets. The result signifies a success/failure of the
 *  test.
 *
 *  The test case which the NSURLConnectionTest implements by default is connecting
 *  to the http://localhost:1234/ whith the HTTP method 'GET'. You can change variable
 *  parts of process by supplying a custom dictionary to the method -[setUpTest:]
 *  before the test case is started by a call of the -[startTest:]. The use pattern:
 *  --------------------------------------------------------------------------
 *  #import "NSURLConnectionTest.h"
 *  #import "Testing.h"
 *  
 *  NSURLConnectionTest *testCase = [NSURLConnectionTest new];
 *  // the extra dictionary with test cases's parameters
 *  NSDictionary *d = [NSDictionary dictionaryWithObjectsAndKeys:
 *                                  @"/somepath", @"Path",
 *                                  @"POST", @"Method",
 *                                  @"https", @"Protocol",
 *                                  nil];
 *  [testCase setUpTest: d];
 *  [testCase startTest: d];
 *  PASS([testCase isSuccess], "a diagnostic message about the test");
 *  [testCase tearDownTest: d];
 *  DESTROY(testCase);
 *  --------------------------------------------------------------------------
 *
 *  The method -[setUpTest:] recognises the following variable parts (supplied as key-value
 *  pairs):
 *
 *     'Instance'
 *     'AuxInstance'  - holds an already running main/auxilliary TestWebServer instance.
 *                      If nil the class will run one main instance of TestWebServer
 *                      and no auxilliary one. The key 'IsAuxilliary' with the value 'YES'
 *                      should be supplied to run an own auxilliary instance.
 *                      Useful to run several tests consequently. The test won't stop
 *                      on its own the supplied instance(s). It leaves the calling code
 *                      to make the decision. The auxilliary instance 'AuxInstance' is
 *                      used in tests on redirecting where two web servers are needed.
 *                      Default: nil
 *     'IsAuxilliary' - whether the NSURLConnectionTest should run it's own auxilliary
 *                      TestWebServer instance. You must supply YES as a value to run it.
 *                      It will be run with the same parameters (address/protocol/detached)
 *                      as the main one except of the port which is assigned from the value
 *                      of the key 'AuxPort'.
 *                      Default: NO
 *     'IsDetached'   - whether to run the own(and auxilliary) instance in the detached mode
 *                      (the instance will be run within a detached thread).
 *                      Default: NO
 *     'Address'      - the address of the remote side.
 *                      Default: localhost
 *     'Port'         - the port of the remote side.
 *                      Default: 1234
 *     'AuxPort'      - the port of the auxilliary remote side (where the connection
 *                      to be redirected).
 *                      Default: 1235
 *     'Protocol'     - the network protocol (supports currently http, https).
 *                      Default: http
 *     'Path'         - the path of the URL.
 *                      Default: '/'
 *     'RedirectPath' - the path where request should be redirected in corresponding tests.
 *                      Default: '/'
 *     'StatusCode'   - the status code expected from the remote side if the test
 *                      is successful.
 *                      Default: 204
 *     'RedirectCode' - the status code expected from the remote side on the connection's first
 *                      stage in a test with redirect.
 *                      Default: 301
 *     'Method'       - the request's method.
 *                      Default: GET
 *     'Payload'      - the request's payload. It can be of NSString or of NSData class.
 *                      The class produces NSData from NSString using NSUTF8StringEncoding.
 *                      Default: nil
 *     'Content'      - the expected content. It can be of NSString or of NSData class.
 *                      The class produces NSData from NSString using NSUTF8StringEncoding.
 *                      Default: nil
 *     'ReferenceFlags' - the dictionary to correct the guessed reference flag set.
 *                        Default: nil
 *                        The class tries to infer the reference flag set from
 *                        the test request. If that guessed (default) set is wrong then this 
 *                        dictionary allows to correct it.
 *                        The class recognises the following keys:
 *
 *                            'SENTREQUEST'
 *                            'AUTHORIZED'
 *                            'NOTAUTHORIZED'
 *                            'GOTUNAUTHORIZED'
 *                            'GOTREQUEST'
 *                            'SENTRESPONSE'
 *                            'GOTRESPONSE'
 *                            'GOTCONTENT'
 *                            'GOTFINISH'
 *                            'GOTFAIL'
 *                            'GOTREDIRECT'
 *
 *                        the 'YES' as a value means the corresponding reference flag
 *                        must be set, otherwise 'NO' means it must not be set.
 *
 *  The NSURLConnectionTest can raise it's verbosity level by the method call -[setDebug: YES].
 *  In this case whole connection session will be showed in the log.
 *
 */

#import "TestWebServer.h"
#import "TestCase.h"

/* the test's checkpoint list */

/* the request has been sent by the client and reaches the server */
#define SENTREQUEST          1
/* the client's request has been authorized */
#define AUTHORIZED           2
/* the client's request hasn't been authorized */
#define NOTAUTHORIZED        4
/* the client has got Unauthorized response */
#define GOTUNAUTHORIZED      8
/* the server has got a right request */
#define GOTREQUEST           16
/* the server is ready to send the response */
#define SENTRESPONSE         32
/* the client has got the response from the server with valid headers/properties */
#define GOTRESPONSE          64
/* the client has got the expected content from the server's response */
#define GOTCONTENT           128
/* the test's execution has reached client's -[connectionDidFinishLoading:] */
#define GOTFINISH            256
/* the test's execution has reached client's -[connection:didFailWithError:] */
#define GOTFAIL              512
/* the test's execution has reached client's -[connection:willSendRequest:redirectResponse:]*/
#define GOTREDIRECT          1024
/* the end of the test's checkpoint list */

@interface NSURLConnectionTest : TestCase <TestWebServerDelegate>
{
  /* the main TestWebServer instance */
  TestWebServer *_server;
  /* tha auxilliary TestWebServer instance needed in tests on redirecting */
  TestWebServer *_auxServer;
  /* the custom request (made by the instance or supplied externally) */
  NSURLRequest *_request;
  /* the redirect request (made by the instance) */
  NSURLRequest *_redirectRequest;
  /* the data accumulator for the received response's content */
  NSMutableData *_received;
  /* the expected status code */
  NSUInteger _expectedStatusCode;
  /* the expected redirect status code of the connection's first stage
   * in tests with redirect... the resulting (on the second stage)
   * expected status code is stored in the ivar _expectedStatusCode */
  NSUInteger _redirectStatusCode;
  /* the expected response's content */
  NSData *_expectedContent;
  /* the connection */
  NSURLConnection *_conn;
  /* to store the error supplied with the -[connection:didFailWithError:] */
  NSError *_error;
}

/**
 *  Returns a TestWebServer-like class used by this test case class. A descendant can
 *  return more advanced implementation of TestWebServer's functionality.
 */
+ (Class)testWebServerClass;

+ (void)initialize;

- (id)init;

/* See the super's description */
- (void)setUpTest:(id)extra;
- (void)startTest:(id)extra;
- (void)tearDownTest:(id)extra;

/**
 *  The only difference from the super's one is issuing of a diagnostic message
 *  if the test is failed.
 */
- (BOOL)isSuccess;

/**
 *  Issues log messages describing the actual and reference flag sets' states.
 */
- (void)logFlags;

/**
 *  Returns the error stored by the -[connection:didFailWithError:].
 */
- (NSError *)error;

/* NSURLConnectionDelegate */
- (NSURLRequest *)connection:(NSURLConnection *)connection
             willSendRequest:(NSURLRequest *)request
            redirectResponse:(NSURLResponse *)redirectResponse;

- (void)connection:(NSURLConnection *)connection
didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge;

- (void)connection:(NSURLConnection *)connection
didReceiveResponse:(NSURLResponse *)response;

- (void)connection:(NSURLConnection *)connection
    didReceiveData:(NSData *)data;

- (void)connectionDidFinishLoading:(NSURLConnection *)connection;

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error;

/* end of NSURLConnectionDelegate */

@end /* NSURLConnectionTest */
