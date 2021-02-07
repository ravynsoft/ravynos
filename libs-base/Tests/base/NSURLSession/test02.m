#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"
#import "../NSURLConnection/Helpers/TestWebServer.h"

#if GS_HAVE_NSURLSESSION
#import "delegate.g"
#endif

int main()
{
  START_SET("NSURLSession http")

#if !GS_HAVE_NSURLSESSION
    SKIP("library built without NSURLSession support")
#else
  NSFileManager	*fm;
  NSBundle	*bundle;
  NSString	*helperPath;

  // load the test suite's classes
  fm = [NSFileManager defaultManager];
  helperPath = [[fm currentDirectoryPath] stringByAppendingPathComponent:
    @"../NSURLConnection/Helpers/TestConnection.bundle"];
  bundle = [NSBundle bundleWithPath: helperPath];
  NSCAssert([bundle load], NSInternalInconsistencyException);

  TestWebServer	*server;
  Class		c;
  BOOL 		debug = YES;

  // create a shared TestWebServer instance for performance
  c = [bundle principalClass];
  server = [[c testWebServerClass] new];
  NSCAssert(server != nil, NSInternalInconsistencyException);
  [server setDebug: debug];
  [server start: nil]; // localhost:1234 HTTP


  NSURLSessionConfiguration     *defaultConfigObject;
  NSURLSession                  *defaultSession;
  NSURLSessionDataTask          *dataTask;
  NSMutableURLRequest           *urlRequest;
  NSURL                         *url;
  NSOperationQueue              *mainQueue;
  NSString                      *params;
  MyDelegate                    *object;

  object = AUTORELEASE([MyDelegate new]);
  mainQueue = [NSOperationQueue mainQueue];
  defaultConfigObject = [NSURLSessionConfiguration defaultSessionConfiguration];
  defaultSession = [NSURLSession sessionWithConfiguration: defaultConfigObject
                                                 delegate: object
                                            delegateQueue: mainQueue];
  url = [NSURL URLWithString: @"http://localhost:1234/xxx"];
  params = @"dummy=true";
  urlRequest = [NSMutableURLRequest requestWithURL: url];
  [urlRequest setHTTPMethod: @"POST"];
  [urlRequest setHTTPBody: [params dataUsingEncoding: NSUTF8StringEncoding]];
  if ([urlRequest respondsToSelector: @selector(setDebug:)])
    {
      [urlRequest setDebug: YES];
    }

  dataTask = [defaultSession dataTaskWithRequest: urlRequest];
  [dataTask resume];

  NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: 60.0];
  while ([object finished] == NO
    && [limit timeIntervalSinceNow] > 0.0)
    {
      ENTER_POOL
      NSDate    *when = [NSDate dateWithTimeIntervalSinceNow: 0.1];

      [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                               beforeDate: when];
      LEAVE_POOL
    }

  PASS(YES == [object finished], "request completed")
  PASS_EQUAL([object taskError], nil, "request did not error")

  NSString *expect = @"Please give login and password";
  PASS_EQUAL([object taskText], expect, "request returned text")

#endif
  END_SET("NSURLSession http")
  return 0;
}
