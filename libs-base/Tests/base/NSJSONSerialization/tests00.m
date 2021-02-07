//
//  Created by Larry Campbell on 12/12/08.
//

#import <Foundation/Foundation.h>

#import "ObjectTesting.h"


@interface GnustepBaseTests : NSObject {
    unsigned         _successes;
    unsigned         _failures;
}
@end

@implementation GnustepBaseTests

- (BOOL) performTest: (NSString *)name
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  BOOL  result;
  
  NSLog(@"Performing test %@", name);
  NS_DURING
    {
      [self performSelector: NSSelectorFromString(name)];
      _successes++;
      result = YES;
    }
  NS_HANDLER
    {
      NSLog(@"Test %@ failed: %@", name, [localException reason]);
      _failures++;
      result = NO;
    }
  NS_ENDHANDLER

  [pool release];
  return result;
}

- (unsigned) successes
{
  return _successes;
}

- (unsigned) failures
{
  return _failures;
}

@end

@interface GnustepBaseTests(TheTests)
@end

@implementation GnustepBaseTests(TheTests)

- (void) cr39118
{
  int x = 1999999999;
  NSString *s = @"1999999999";
  NSAssert([s intValue] == x, @"intValue botch");
}

// verify that distinct strings get distinct hashes even if strings are very long
- (void) cr48439
{
  NSMutableSet *hashes = [NSMutableSet set];
  NSArray *strings = [NSArray arrayWithObjects:
    @"",
    @"a",
    @"aa",
    @"123456789012345678901234567890123456789012345678901234567890123",
    @"123456789012345678901234567890123456789012345678901234567890123a",
    @"123456789012345678901234567890123456789012345678901234567890123b",
    @"123456789012345678901234567890123456789012345678901234567890123c",
    @"123456789012345678901234567890123456789012345678901234567890123d",
    @"1234567890123456789012345678901234567890123456789012345678901234",
    @"1234567890123456789012345678901234567890123456789012345678901234a",
    @"1234567890123456789012345678901234567890123456789012345678901234b",
    @"1234567890123456789012345678901234567890123456789012345678901234c",
    @"1234567890123456789012345678901234567890123456789012345678901234d",
    @"12345678901234567890123456789012345678901234567890123456789012345",
    @"12345678901234567890123456789012345678901234567890123456789012345a",
    @"12345678901234567890123456789012345678901234567890123456789012345b",
    @"12345678901234567890123456789012345678901234567890123456789012345c",
    @"12345678901234567890123456789012345678901234567890123456789012345d",
    @"123456789012345678901234567890123456789012345678901234567890123456789012345",
    @"123456789012345678901234567890123456789012345678901234567890123456789012345a",
    @"123456789012345678901234567890123456789012345678901234567890123456789012345b",
    @"123456789012345678901234567890123456789012345678901234567890123456789012345c",
    @"123456789012345678901234567890123456789012345678901234567890123456789012345d",
    nil];
  NSString *s;
  NSEnumerator *e;
  
  e = [strings objectEnumerator];
  while ((s = [e nextObject]) != nil)
    [hashes addObject:[NSNumber numberWithUnsignedInt:[s hash]]];
  
  NSAssert([hashes count] == [strings count], @"hash botch");
}

// this also covers CR 150661
- (void)cr153594
{
  NSDate        *t1 = [NSDate date];
  NSDate        *t2;
  NSData        *d;
    
  d = [NSKeyedArchiver archivedDataWithRootObject: t1];
  t2 = [NSKeyedUnarchiver unarchiveObjectWithData: d];
  NSAssert([t1 isEqual:t2], @"equality botch");
}

- (void)cr1524466
{
    NSURL *u = [[[NSURL alloc] initWithScheme: @"http" host: @"1.2.3.4" path: @"/a?b;foo"] autorelease];
    NSString *s = [[u absoluteURL] description];
    NSAssert([s isEqual: @"http://1.2.3.4/a?b;foo"], @"NSURL encoding botch");
}

- (void)cr2096767
{
  static struct {
    NSString        *inputString;
    NSTimeInterval   increment;
      NSString        *expectedOutput;
  } tests[] = {
    { @"2006-04-22 22:22:22:901",  0.0,         @"2006-04-22 22:22:22:901" },    // gnustep bug https://savannah.gnu.org/bugs/?func=detailitem&item_id=16426
    { @"2006-04-22 22:22:22:999",  0.0,         @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.0005,      @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.0006,      @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.0007,      @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.0008,      @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.00089,     @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.000899,    @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.0008999,   @"2006-04-22 22:22:22:999" },
    { @"2006-04-22 22:22:22:999",  0.00089999,  @"2006-04-22 22:22:23:000" },
    { @"2006-04-22 22:22:22:999",  0.0009,      @"2006-04-22 22:22:23:000" },    // CR https://bugzilla.akamai.com/show_bug.cgi?id=2096767
  };
  unsigned i;
  NSString *fmt = [NSString stringWithFormat: @"%%Y-%%m-%%d %%H:%%M:%%S:%%F"];

  for (i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
    {
      NSString *inputString = tests[i].inputString;
      NSString *expectedOutput = tests[i].expectedOutput;
      NSCalendarDate *d1 = [NSCalendarDate dateWithString:inputString calendarFormat:fmt];
      NSCalendarDate *d2 = [[[NSCalendarDate alloc] initWithTimeInterval:tests[i].increment sinceDate:d1] autorelease];
      NSString *result = [d2 descriptionWithCalendarFormat:fmt];
      NSLog(@"input string: %@, increment: %.4f", inputString, tests[i].increment);
      NSLog(@"input date: %@", d1);
      NSLog(@"output date: %@", result);
      NSLog(@"expect: %@", expectedOutput);
      NSAssert([result isEqualToString:expectedOutput], @"mismatch");
  }
}

- (void)cr2549370
{
  NSString *path = @"./json.data";
  NSString *string = @"Hello garçon! ¡Ola! A little more coöperation please!";
  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
  NSMutableArray *array = [NSMutableArray array];
  NSData *d;
  NSInputStream *stream;

  [array addObject:[NSDictionary dictionaryWithObjectsAndKeys:string, @"nickname", nil]];
  [dict setObject:array forKey: @"datacenters"];

  d = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted error:0];
  [d writeToFile:path atomically:NO];
  
  stream = [NSInputStream inputStreamWithFileAtPath:path];
  [stream open];
  dict = [NSJSONSerialization JSONObjectWithStream:stream options:0 error:0];
  dict = [[dict objectForKey: @"datacenters"] objectAtIndex:0];
  [[NSFileManager defaultManager] removeFileAtPath: path handler: nil];
  NSAssert(dict != nil, @"JSON returned nil");
  NSAssert([string isEqualToString:[dict objectForKey: @"nickname"]], @"data mismatch");
}

void checkstr(NSString *expected, NSString *got)
{
  if ([expected caseInsensitiveCompare:got] != NSOrderedSame) {
      NSLog(@"expected %@", expected);
      NSLog(@"     got %@", got);
      [NSException raise: @"error" format: @"test failed"];
  }
}

// This is copied from servermonitor. We want to use the same quoting rules it does.
// We use this only for the username and password; we assume the path/query/fragment
// is already escaped.
static NSString *percentQuoted(NSString *s)
{
  NSMutableString *retval = [NSMutableString stringWithCapacity:[s length]];
  const unsigned char *p = (const unsigned char *)[s UTF8String];
  unsigned char c;
  const char *allowedNonalnumChars = "-_,.'~!$&*();";
  
  while ((c = *p++) != '\0')
    {
      if (isalnum(c) || strchr(allowedNonalnumChars, c) != 0)
	{
          [retval appendFormat: @"%c", c];
	}
      else
	{
          [retval appendFormat: @"%%%02x", c];
	}
    }
  return retval;
}

- (void)urlTest
{
  static NSString *schemes[] = { @"ftp", @"http", @"https" };
  static NSString *hosts[] = { @"www.akamai.com", @"10.0.1.1", @"localhost" };
  static NSString *usernames[] = { nil, @"username", @"foo", @"a_username" };
  static NSString *passwords[] = { nil, @"password", @"M@nx2y#" };
  
  // The path/query/fragment part here must be percent-quoted
  // as it would have to be if pasted into a curl command.
  struct {
    NSString *pqf;  // path, query, fragment
    NSString *path; // expected path returned from NSURL (nil if same as pqf)
  } pqfs[] = {
      { @"/", nil },
      { @"/foo/bar/zot", nil },
      { @"/a%20dillar%20a%20dollar", nil },
      { @"/foo:bar:zot", nil },
      { @"/a?b;foo", @"/a" },
      { @"/a%25b", nil },
      { @"/MHcwdTBOMEwwSjAJBgUrDgMCGgUABBScDhI23ZEqCpe2zGqbkoJVvUaDTgQUf/ZMNigUrs0eN6/eWvJbw6CsK/4CEQCbL3Mm0kXS8q7t0jI7E5gdoiMwITAfBgkrBgEFBQcwAQIEEgQQi7kTQMz7QK60KI2PnRR3mg==",
          nil },
      { @"/interface/6/?mode=rc3&outputType=json&url=http%3A%2F%2Fwww.ehow.com%2Fhow_2324118_take-care-paper-cut.html&category=Health&subCategory=Family%20Health&subSubCategory=General%20Family%20Health&pageLocations=PLATFORM_TL|0&flushPage=1",
          @"/interface/6" },
      { @"/d/search/p/enom/xml/domain/multiset/v4/?url=http%3A%2F%2Fnuseek.com&Partner=enom_internal_d2r_derp&config=1234567890&affilData=ip%3d127.0.0.1%26ua%3dIE|6.0|WinNT%26ur%3d&maxListings=10&maxRT=20&maxRTL=20&maxWeb=4&serveUrl=http://parkingweb30/default.pk",
          @"/d/search/p/enom/xml/domain/multiset/v4" },
  };
  unsigned i, j, k, l, m;
  
  for (i = 0; i < sizeof(schemes)/sizeof(schemes[0]); i++)
    {
      NSString *scheme = schemes[i];

      for (j = 0; j < sizeof(hosts)/sizeof(hosts[0]); j++)
        {
          NSString *host = hosts[j];

          for (k = 0; k < sizeof(usernames)/sizeof(usernames[0]); k++)
            {
              NSString *username = usernames[k];

              for (l = 0; l < sizeof(passwords)/sizeof(passwords[0]); l++)
                {
                  NSString *password = passwords[l];

                  for (m = 0; m < sizeof(pqfs)/sizeof(pqfs[0]); m++)
                    {
                      NSMutableString *hostpart = [NSMutableString string];
                      NSString *pqf = pqfs[m].pqf;
                      NSString *expectedPath = [(pqfs[m].path == nil ? pqf : pqfs[m].path) stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
                      NSString *urlstring;
                      NSURL *url;
                      
                      expectedPath = [(pqfs[m].path == nil ? pqf : pqfs[m].path) stringByReplacingPercentEscapesUsingEncoding: NSUTF8StringEncoding];

                      if (username != nil)
                        {
                          if (password != nil)
                            {
                              [hostpart appendFormat: @"%@:%@@",
                                percentQuoted(username),
                                percentQuoted(password)];
                            }
                          else
                            {
                              [hostpart appendFormat: @"%@@",
                                percentQuoted(username)];
                            }
                        }
                      [hostpart appendString:host];
                      
                      urlstring = [NSString stringWithFormat: @"%@://%@%@",
                        scheme, hostpart, pqf];
                      url = [[[NSURL alloc] initWithString: urlstring]
                        autorelease];
                      
                      // work around Apple bug, which rejects
                      // objects containing vertical bar (turns out
                      // gnustep is bug-for-bug compatible here,
                      // so no need to make this Apple-only)
                      if (url == nil && [pqf rangeOfString: @"|"].length != 0)
                        {
                          NSMutableString *fixedObject = [[pqf mutableCopy] autorelease];
                          unsigned n;
                          do {
                              n = [fixedObject replaceOccurrencesOfString: @"|" withString: @"%7C" options:0 range:NSMakeRange(0, [fixedObject length])];
                          } while (n != 0);
                          urlstring = [NSString stringWithFormat: @"%@://%@%@", scheme, hostpart, fixedObject];
                          url = [[[NSURL alloc] initWithString:urlstring] autorelease];
                        }
                      
                      NSAssert(url != nil, @"URL creation failed");

                      NSAssert([[url scheme] isEqual:schemes[i]], @"scheme botch");
                      NSAssert((username == nil) == ([url user] == nil), @"username existence botch");
                      if (username != nil) {
                          NSAssert([[url user] isEqual:username], @"username botch");
                          NSAssert((password == nil) == ([url password] == nil), @"password existence botch");
                          if (password != nil) {
                              NSString *urlpassword = [[url password] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
                              NSAssert([urlpassword isEqual:password], @"password botch");
                          }
                      }
                      checkstr(expectedPath, [url path]);
                      checkstr(urlstring, [[url absoluteURL] description]);
                      NSAssert([[url host] isEqual:host], @"host botch");
                      NSAssert([[url path] isEqual:expectedPath],
                        @"path botch");
                    }
                }
            }
        }
    }
}

- (void)longLongOverflow
{
  NSNumber *big = [NSNumber numberWithUnsignedLongLong: 0xffffffffffffffff];
  NSNumber *small = [NSNumber numberWithLongLong: 0xffffffffffffffff];

  NSData *data = [NSJSONSerialization dataWithJSONObject: big
						 options: 0
						   error: NULL];
  NSString *string = [[NSString alloc] initWithData: data
					   encoding: NSUTF8StringEncoding];
  NSAssert([string isEqualToString:@"18446744073709551615"], @"unsigned long long");
  [string release];

  data = [NSJSONSerialization dataWithJSONObject: small
					 options: 0
					   error: NULL];
  string = [[NSString alloc] initWithData: data
				 encoding: NSUTF8StringEncoding];
  NSAssert([string isEqualToString:@"-1"], @"signed long long");
  [string release];
}

@end


int main(int argc, char *argv[])
{
  int status = 0;
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  GnustepBaseTests *gtb = [GnustepBaseTests new];
  
  PASS([gtb performTest: @"cr39118"], "cr39118");
  PASS([gtb performTest: @"cr48439"], "cr48439");
  PASS([gtb performTest: @"cr153594"], "cr153594");
  PASS([gtb performTest: @"urlTest"], "urlTest");
  PASS([gtb performTest: @"cr1524466"], "cr1524466");
  PASS([gtb performTest: @"cr2096767"], "cr2096767");
  PASS([gtb performTest: @"cr2549370"], "cr2549370");
  PASS([gtb performTest: @"longLongOverflow"], "longLongOverflow");
  [gtb release];

  [pool release];
  return status;
}
