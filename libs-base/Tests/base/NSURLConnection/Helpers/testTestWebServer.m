/**
 *
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 *
 *  Runs two TestWebServer instances to check how the class TestWebServer
 *  behaves. Visit http://localhost:1234/index to see all supported resources.
 *  
 *  If you visit the main TestWebServer instance with the following command:
 *
 *       wget  -O - --user=login --password=password http://localhost:1234/301 2>&1
 *
 *  you should get a session log like this:
 *
 *       --2014-08-13 12:08:01--  http://localhost:1234/301
 *       Resolving 127.0.0.1 (localhost)... 127.0.0.1
 *       Connecting to 127.0.0.1 (localhost)|127.0.0.1|:1234... connected.
 *       HTTP request sent, awaiting response... 401 Unauthorized
 *       Reusing existing connection to 127.0.0.1:1234.
 *       HTTP request sent, awaiting response... 301 Moved Permanently
 *       Location: http://127.0.0.1:1235/ [following]
 *       --2014-08-13 12:08:01--  http://127.0.0.1:1235/
 *       Connecting to 127.0.0.1:1235... connected.
 *       HTTP request sent, awaiting response... 401 Unauthorized
 *       Reusing existing connection to 127.0.0.1:1235.
 *       HTTP request sent, awaiting response... 204 No Content
 *       Length: 0
 *       Saving to: ‘STDOUT’
 *
 *            0K                                                        0.00 =0s
 *
 *       2014-08-13 12:08:01 (0.00 B/s) - written to stdout [0/0]
 *
 */
#import <Foundation/Foundation.h>
#import "TestWebServer.h"
#import "NSURLConnectionTest.h"

#define TIMING 0.1

int main(int argc, char **argv, char **env)
{
  CREATE_AUTORELEASE_POOL(arp);
  NSFileManager *fm;
  NSBundle *bundle;
  BOOL loaded;
  NSString *helperPath;

  fm = [NSFileManager defaultManager];
  helperPath = [[fm currentDirectoryPath]
    stringByAppendingString: @"/TestConnection.bundle"];
  bundle = [NSBundle bundleWithPath: helperPath];
  loaded = [bundle load];

  if (loaded)
    {
      TestWebServer *server1;
      TestWebServer *server2;
      Class testClass;
      BOOL debug = YES;
      NSDictionary *d;

      testClass = [bundle principalClass]; // NSURLConnectionTest
      d = [NSDictionary dictionaryWithObjectsAndKeys:
        //			  @"https", @"Protocol",
        nil];
      server1 = [[[testClass testWebServerClass] alloc]
        initWithAddress: @"localhost"
        port: @"1234"
        mode: NO
        extra: d];
      [server1 setDebug: debug];
      [server1 start: d]; // localhost:1234 HTTP

      server2 = [[[testClass testWebServerClass] alloc]
        initWithAddress: @"localhost"
        port: @"1235"
        mode: NO
        extra: d];
      [server2 setDebug: debug];
      [server2 start: d]; // localhost:1235 HTTP
      
      while (YES)
	{
	  [[NSRunLoop currentRunLoop]
      		runUntilDate: [NSDate dateWithTimeIntervalSinceNow: TIMING]];
	}
      //      [server1 stop];
      //      DESTROY(server1);
      //      [server2 stop];
      //      DESTROY(server2);

    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"can't load bundle TestConnection"];
    }


  DESTROY(arp);

  return 0;
}
