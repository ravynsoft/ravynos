#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"


int main()
{
  NSURLComponents *components = nil;

  START_SET("components");

  components = [NSURLComponents componentsWithURL:
    [NSURL URLWithString: @"https://user:password@some.host.com"]
      resolvingAgainstBaseURL: NO];
  
  [components setQueryItems: [NSArray arrayWithObjects:
    [NSURLQueryItem queryItemWithName: @"lang" value: @"en"],
    [NSURLQueryItem queryItemWithName: @"response_type" value: @"code"],
    [NSURLQueryItem queryItemWithName: @"uri" value:
      [[NSURL URLWithString: @"https://some.url.com/path?param1=one&param2=two"]
	absoluteString]], nil]];
  // URL
  PASS_EQUAL([components string],
    @"https://user:password@some.host.com?lang=en&response_type=code"
    @"&uri=https://some.url.com/path?param1%3Done%26param2%3Dtwo",
    "URL string is correct")
  
  // encoded...
  PASS_EQUAL([components percentEncodedQuery],
    @"lang=en&response_type=code&uri=https://some.url.com/path"
    @"?param1%3Done%26param2%3Dtwo",
    "percentEncodedQuery is correct")
  PASS_EQUAL([components percentEncodedHost], @"some.host.com",
    "percentEncodedHost is correct")
  PASS_EQUAL([components percentEncodedUser], @"user",
    "percentEncodedUser is correct")
  PASS_EQUAL([components percentEncodedPassword], @"password",
    "percentEncodedPassword is correct")
  
  // unencoded...
  PASS_EQUAL([components query],
    @"lang=en&response_type=code&uri=https://some.url.com/path?"
    @"param1=one&param2=two",
    "query is correct")
  PASS_EQUAL([components host], @"some.host.com",
    "host is correct")
  PASS_EQUAL([components user], @"user",
    "user is correct")
  PASS_EQUAL([components password], @"password",
    "password is correct")
    
  [components setQuery: nil];
  PASS_EQUAL([components query], nil,
    "set query to nil")
  PASS_EQUAL([components percentEncodedQuery], nil,
    "percent encoded query is nil")
  PASS_EQUAL([components queryItems], nil,
    "query items is nil")
  PASS_EQUAL([components percentEncodedQueryItems], nil,
    "percent encoded query items is nil")
 
  [components setQuery: @""];
  NSArray *emptyArray = [NSArray array];
  PASS_EQUAL([components query], @"",
    "set query to empty")
  PASS_EQUAL([components percentEncodedQuery], @"",
    "percent encoded query is empty")
  PASS_EQUAL([components queryItems], emptyArray,
    "query items is empty")
  PASS_EQUAL([components percentEncodedQueryItems], emptyArray,
    "percent encoded query items is empty")

  [components setQuery: @"&"];
  PASS_EQUAL([components query], @"&",
    "set query to ampersand")
  PASS_EQUAL([components percentEncodedQuery], @"&",
    "percent encoded ampersand query is still ampersand")

  [components setPercentEncodedQuery: @"%26"];
  PASS_EQUAL([components query], @"&",
    "set query to item containing ampersand")
  PASS_EQUAL([components percentEncodedQuery], @"%26",
    "percent encoded query item encodes ampersand")
    
  //NSURLQueryItem percent encoding/decoding test
  NSString* urlString = @"http://domain/?%D0%90%D0%B0%D0%91%D0%B1=%D0%90%D0%B0%D0%91%D0%B1";
  NSString* cyrillicStr = @"\U00000410\U00000430\U00000411\U00000431";
  NSURLComponents* components = [NSURLComponents componentsWithString:urlString];
  NSURLQueryItem* item = [[components queryItems] lastObject];
  PASS_EQUAL(item.name, cyrillicStr,  "Should return decoded query item name from url");
  PASS_EQUAL(item.value, cyrillicStr, "Should return decoded query item value from url");
  
  item = [NSURLQueryItem queryItemWithName:cyrillicStr value:cyrillicStr];
  [components setQueryItems:[NSArray arrayWithObject:item]];
  PASS_EQUAL([components string], urlString, "Encoded url string from unencoded item");
  PASS_EQUAL([components URL], [NSURL URLWithString:urlString], "Encoded url query part from unencoded item");
    
  NSString* invalidUrlString = @"\U00000410\U00000430\U00000411\U00000431";
  PASS_EQUAL([NSURL URLWithString:invalidUrlString], nil, "nil NSURL from invalid string")
  PASS_EQUAL([NSURLComponents componentsWithString:invalidUrlString], nil, "nil NSComponents from invalid string")

  END_SET("components")

  return 0;
}
