/**
 *  Tests for HTTPS without authorization (big request)
 */
#import <Foundation/Foundation.h>
#import "Helpers/NSURLConnectionTest.h"
#import "Helpers/TestWebServer.h"
#import <Testing.h>

int main(int argc, char **argv, char **env)
{
  CREATE_AUTORELEASE_POOL(arp);
  NSFileManager 	*fm;
  NSBundle 		*bundle;
  BOOL 			loaded;
  NSString 		*helperPath;

  // load the test suite's classes
  fm = [NSFileManager defaultManager];
  helperPath = [[fm currentDirectoryPath]
    stringByAppendingString: @"/Helpers/TestConnection.bundle"];
  bundle = [NSBundle bundleWithPath: helperPath];
  loaded = [bundle load];

  if (loaded)
    {
      NSDictionary 		*d;
      Class 			testClass;
      NSDictionary		*refs;
      TestWebServer		*server;
      NSMutableString		*payload;
      NSURLConnectionTest 	*testCase;
      BOOL 			debug = NO;
      int			i;

      testClass = [bundle principalClass]; // NSURLConnectionTest

      // the extra dictionary commanding to use HTTPS
      d = [NSDictionary dictionaryWithObjectsAndKeys:
	@"HTTPS", @"Protocol",
	nil];
      // create a shared TestWebServer instance for performance
      server = [[[testClass testWebServerClass] alloc]
	initWithAddress: @"localhost"
	port: @"1234"
	mode: NO
        extra: d];
      [server setDebug: debug];
      [server start: d]; // localhost:1234 HTTPS

      /* Simple POST via HTTPS with the response's status code 400 and
       * non-empty response's body
       */
      testCase = [testClass new];
      [testCase setDebug: debug];
      /* the reference set difference (from the default reference set)
       * we expect the flags must not be set because we request a path
       * with no authorization. See NSURLConnectionTest.h for details
       * (the key words are 'TestCase' and 'ReferenceFlags')
       */
      refs = [NSDictionary dictionaryWithObjectsAndKeys:
	@"NO", @"GOTUNAUTHORIZED", 
	@"NO", @"AUTHORIZED",        
	@"NO", @"NOTAUTHORIZED",     
	nil];

      // the extra dictionary with test case's parameters
      payload = [NSMutableString stringWithCapacity: 1024 * 128];
      for (i = 0; i < 2000; i++)
	{
	  [payload appendFormat:
	    @"%09daaaaaaaaaabbbbbbbbbbcccccccccccdddddddddd\n",
	    i * 50];
	}
      d = [NSDictionary dictionaryWithObjectsAndKeys:
	server, @"Instance", // we use the shared TestWebServer instance
	@"400/withoutauth", @"Path", // request the handler responding with 400
	@"400", @"StatusCode", // the expected status code
	@"You have issued a request with invalid data", @"Content", // expected
	payload, @"Payload", // the custom payload
	@"POST", @"Method",    // use POST
	refs, @"ReferenceFlags", // the expected reference set difference
	nil];
      [testCase setUpTest: d];
      [testCase startTest: d];
      PASS([testCase isSuccess], "HTTPS... big payload... response 400 .... POST https://localhost:1234/400/withoutauth");
      [testCase tearDownTest: d];
      DESTROY(testCase);

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
