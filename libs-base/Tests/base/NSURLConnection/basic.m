#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableURLRequest *mutable;
  NSURLConnection *connection;
  NSURLResponse *response;
  NSURLRequest *request;
  NSError *error;
  NSData *data;
  NSURL *httpURL;
  NSString *path;

  httpURL = [NSURL URLWithString: @"http://www.gnustep.org"];

  TEST_FOR_CLASS(@"NSURLConnection", [NSURLConnection alloc],
    "NSURLConnection +alloc returns an NSURLConnection");

  mutable = [NSMutableURLRequest requestWithURL: httpURL];
  PASS([NSURLConnection canHandleRequest: mutable],
    "NSURLConnection can handle an valid HTTP request (GET)");
  [mutable setHTTPMethod: @"WRONGMETHOD"];
  PASS([NSURLConnection canHandleRequest: mutable],
    "NSURLConnection can handle an invalid HTTP request (WRONGMETHOD)");

  [mutable setHTTPMethod: @"GET"];
  connection = [NSURLConnection connectionWithRequest: mutable delegate: nil];
  PASS(connection != nil,
    "NSURLConnection +connectionWithRequest: delegate: with nil as delegate returns a instance");

  response = nil;
  data = [NSURLConnection sendSynchronousRequest: mutable
                               returningResponse: &response
                                           error: &error];
  PASS(data != nil && [data length] > 0,
    "NSURLConnection synchronously load data from an http URL");
  PASS(response != nil && [(NSHTTPURLResponse*)response statusCode] > 0,
    "NSURLConnection synchronous load returns a response");

  path = [[NSFileManager defaultManager] currentDirectoryPath];
  path = [path stringByAppendingPathComponent: @"basic.m"];
  [mutable setURL: [NSURL fileURLWithPath: path]];
  data = [NSURLConnection sendSynchronousRequest: mutable
                               returningResponse: &response
                                           error: &error];
  PASS(data != nil && [data length] > 0,
    "NSURLConnection synchronously load data from a local file");

  request = [NSURLRequest requestWithURL:
    [NSURL URLWithString:@"https://www.google.com/"]];
  response = nil;
  error = nil;
  data = [NSURLConnection sendSynchronousRequest: request
                               returningResponse: &response
                                           error: &error];

  PASS(nil == error, "https://www.google.com/ does not return an error")
  PASS(nil != data, "https://www.google.com/ returns data")

  [arp release]; arp = nil;
  return 0;
}
