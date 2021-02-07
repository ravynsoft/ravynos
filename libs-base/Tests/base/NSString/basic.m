#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>


static NSString*
makeFormattedString(NSString *theFormat, ...)
{
  NSString *aString;
  va_list args;

  va_start(args, theFormat);
  aString = [[NSString alloc] initWithFormat: theFormat  arguments: args];
  va_end(args);
  return AUTORELEASE(aString);
}

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  uint8_t       bytes[256];
  unichar	u0 = 'a';
  unichar	u1 = 0xfe66;
  int           i = 256;
  char          buf[32];
  NSString	*s;
  NSString *testObj = [NSString stringWithCString: "Hello\n"];

  while (i-- > 0)
    {
      bytes[i] = (uint8_t)i;
    }

  test_alloc(@"NSString");
  test_NSObject(@"NSString",[NSArray arrayWithObject:testObj]);
  test_NSCoding([NSArray arrayWithObject:testObj]);
  test_keyed_NSCoding([NSArray arrayWithObject:testObj]);
  test_NSCopying(@"NSString", @"NSMutableString", 
                 [NSArray arrayWithObject:testObj], NO, NO);
  test_NSMutableCopying(@"NSString", @"NSMutableString",
  			[NSArray arrayWithObject:testObj]);

  /* Test non-ASCII strings.  */
  testObj = [@"\"\\U00C4\\U00DF\"" propertyList];
  test_NSMutableCopying(@"NSString", @"NSMutableString",
  			[NSArray arrayWithObject:testObj]);

  PASS([(s = [[NSString alloc] initWithCharacters: &u0 length: 1])
    isKindOfClass: [NSString class]]
    && ![s isKindOfClass: [NSMutableString class]],
    "[NSString initWithCharacters:length:] creates immutable string for ascii");

  PASS([(s = [[NSMutableString alloc] initWithCharacters: &u0 length: 1])
    isKindOfClass: [NSString class]]
    && [s isKindOfClass: [NSMutableString class]],
    "[NSMutableString initWithCharacters:length:] creates mutable string for ascii");

  PASS([(s = [[NSString alloc] initWithCharacters: &u1 length: 1])
    isKindOfClass: [NSString class]]
    && ![s isKindOfClass: [NSMutableString class]],
    "[NSString initWithCharacters:length:] creates immutable string for unicode");

  PASS([(s = [[NSMutableString alloc] initWithCharacters: &u1 length: 1])
    isKindOfClass: [NSString class]]
    && [s isKindOfClass: [NSMutableString class]],
    "[NSMutableString initWithCharacters:length:] creates mutable string for unicode");

  PASS_EXCEPTION([[NSString alloc] initWithString: nil];,
  		 NSInvalidArgumentException,
		 "NSString -initWithString: does not allow nil argument");

  PASS([@"he" getCString: buf maxLength: 2 encoding: NSASCIIStringEncoding]==NO,
    "buffer exact length fails");
  PASS([@"hell" getCString: buf maxLength: 5 encoding: NSASCIIStringEncoding],
    "buffer length+1 works");
  PASS(strcmp(buf, "hell") == 0, "getCString:maxLength:encoding");

  PASS([(s = [[NSString alloc] initWithBytes: bytes
                                      length: 256
                                    encoding: NSISOLatin1StringEncoding])
    isKindOfClass: [NSString class]]
    && [s length] == 256,
    "can create latin1 string with 256 values");

  PASS([(s = [[NSString alloc] initWithBytes: bytes
                                      length: 128
                                    encoding: NSASCIIStringEncoding])
    isKindOfClass: [NSString class]]
    && [s length] == 128,
    "can create ascii string with 128 values");

  PASS(nil == [[NSString alloc] initWithBytes: bytes
                                       length: 256
                                     encoding: NSASCIIStringEncoding],
    "reject 8bit characters in ascii");

  s = [[NSString alloc] initWithBytes: bytes
                               length: 256
                             encoding: NSISOLatin1StringEncoding];
  s = [[NSString alloc]
    initWithData: [s dataUsingEncoding: NSNonLossyASCIIStringEncoding]
    encoding: NSASCIIStringEncoding];
  PASS_EQUAL(s, @"\\000\\001\\002\\003\\004\\005\\006\\007\\010\t\n\\013\\014\r\\016\\017\\020\\021\\022\\023\\024\\025\\026\\027\\030\\031\\032\\033\\034\\035\\036\\037 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\\177\\200\\201\\202\\203\\204\\205\\206\\207\\210\\211\\212\\213\\214\\215\\216\\217\\220\\221\\222\\223\\224\\225\\226\\227\\230\\231\\232\\233\\234\\235\\236\\237\\240\\241\\242\\243\\244\\245\\246\\247\\250\\251\\252\\253\\254\\255\\256\\257\\260\\261\\262\\263\\264\\265\\266\\267\\270\\271\\272\\273\\274\\275\\276\\277\\300\\301\\302\\303\\304\\305\\306\\307\\310\\311\\312\\313\\314\\315\\316\\317\\320\\321\\322\\323\\324\\325\\326\\327\\330\\331\\332\\333\\334\\335\\336\\337\\340\\341\\342\\343\\344\\345\\346\\347\\350\\351\\352\\353\\354\\355\\356\\357\\360\\361\\362\\363\\364\\365\\366\\367\\370\\371\\372\\373\\374\\375\\376\\377", "latin1 in lossy encoding");
  NSLog(@"%lu '%s'", [s length], [s UTF8String]);
  RELEASE(s);

  s = [[NSString alloc]
    initWithData: [@"€" dataUsingEncoding: NSNonLossyASCIIStringEncoding]
    encoding: NSASCIIStringEncoding];
  PASS_EQUAL(s, @"\\u20ac", "euro in lossy encoding");
  RELEASE(s);

  s = [[NSString alloc] initWithBytes: "\\"
                               length: 1
                             encoding: NSNonLossyASCIIStringEncoding];
  PASS(nil == s, "lossy single backslash is invalid");

  s = [[NSString alloc] initWithBytes: "\\r"
                               length: 2
                             encoding: NSNonLossyASCIIStringEncoding];
  PASS(nil == s, "lossy backslash-r is invalid");

  s = [[NSString alloc] initWithBytes: "\\\\"
                               length: 2
                             encoding: NSNonLossyASCIIStringEncoding];
  PASS_EQUAL(s, @"\\", "lossy backslash-backslash is backslash");
  RELEASE(s);

  s = [[NSString alloc] initWithBytes: "\\101"
                               length: 4
                             encoding: NSNonLossyASCIIStringEncoding];
  PASS_EQUAL(s, @"A", "lossy backslassh-101 is A");
  RELEASE(s);

  s = [[NSString alloc] initWithBytes: "\\u20ac"
                               length: 6
                             encoding: NSNonLossyASCIIStringEncoding];
  PASS_EQUAL(s, @"€", "lossy backslassh-u20ac is a euro");
  RELEASE(s);

  s = makeFormattedString(@"%d.%d%s", 10, 20, "hello");
  PASS_EQUAL(s, @"10.20hello", "simple intWithFormat: works");

  PASS([@"" isEqual: nil] == NO, "an empty string is not null");
  PASS([@"" isEqualToString: nil] == NO, "an empty string is not null");

  [arp release]; arp = nil;
  return 0;
}
