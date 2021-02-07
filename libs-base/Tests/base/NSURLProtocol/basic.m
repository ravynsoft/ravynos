#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSMutableURLRequest   *mutable, *copy;
  NSURLRequest          *canon;
  NSURL                 *httpURL;

  httpURL = [NSURL URLWithString: @"http://www.gnustep.org"];

  TEST_FOR_CLASS(@"NSURLProtocol", [NSURLProtocol alloc],
    "NSURLProtocol +alloc returns an NSURLProtocol");

  mutable = [[NSMutableURLRequest requestWithURL: httpURL] retain];
  PASS_EXCEPTION([NSURLProtocol canInitWithRequest: mutable], nil,
    "NSURLProtocol +canInitWithRequest throws an exeception (subclasses should be used)");

  canon = [NSURLProtocol canonicalRequestForRequest: mutable];
  TEST_FOR_CLASS(@"NSURLRequest", canon,
    "NSURLProtocol +canonicalRequestForRequest: returns an NSURLProtocol");

  copy = [mutable copy];
  PASS([NSURLProtocol requestIsCacheEquivalent: mutable toRequest: copy],
    "NSURLProtocol +requestIsCacheEquivalent:toRequest returns YES with a request and its copy");
  [copy setHTTPMethod: @"POST"];
  PASS([NSURLProtocol requestIsCacheEquivalent: mutable toRequest: copy] == NO,
    "NSURLProtocol +requestIsCacheEquivalent:toRequest returns NO after a method change");
  [copy release];

  [arp release]; arp = nil;
  return 0;
}
