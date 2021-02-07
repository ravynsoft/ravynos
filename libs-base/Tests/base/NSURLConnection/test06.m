/**
 *  Tests for HTTP synchronous requests.
 */
#import <Foundation/Foundation.h>
#import "Helpers/NSURLConnectionTest.h"
#import "Helpers/TestWebServer.h"
#import <Testing.h>

int main(int argc, char **argv, char **env)
{
  CREATE_AUTORELEASE_POOL(arp);
  NSFileManager *fm;
  NSBundle *bundle;
  BOOL loaded;
  NSString *helperPath;

  // load the test suite's classes
  fm = [NSFileManager defaultManager];
  helperPath = [[fm currentDirectoryPath]
		 stringByAppendingString: @"/Helpers/TestConnection.bundle"];
  bundle = [NSBundle bundleWithPath: helperPath];
  loaded = [bundle load];

  if (loaded)
    {
      Class testClass;
      TestWebServer *server;
      BOOL debug = NO;
      NSURL *url;
      NSError *error = nil;
      NSURLRequest *request;
      NSURLResponse *response = nil;
      NSData *data;

      testClass = [bundle principalClass]; // NSURLConnectionTest

      // create a shared TestWebServer instance for performance
      // by default it requires the basic authentication with the pair
      // login:password
      server = [[testClass testWebServerClass] new];
      [server setDebug: debug];
      [server start: nil]; // localhost:1234 HTTP

      /*
       *  Simple GET via HTTP with some response's body and
       *  the response's status code 200
       */
      url = [NSURL
        URLWithString: @"http://login:password@localhost:1234/index"];
      request = [NSURLRequest requestWithURL: url];
      data = [NSURLConnection sendSynchronousRequest: request
				   returningResponse: &response
					       error: &error];
      PASS(nil != data && [(NSHTTPURLResponse*)response statusCode] == 200,
        "NSURLConnection synchronous load with authentication returns a response");

      // cleaning
      [server stop];
      DESTROY(server);
    }
  else
    {
      // no classes no tests
      [NSException raise: NSInternalInconsistencyException
		  format: @"can't load bundle TestConnection"];
    }


  DESTROY(arp);

  return 0;
}
