/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>

portions:
copyright (C) 2003 Free Software Foundation, Inc.
Author: Alexander Malmberg


Test whether a class is a working concrete subclass of NSString. This file
should be included _once_ in a test that wants to test a particular class.
*/



#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSData.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>

/*
This is the main entry point to this file. Call it with a class that's
supposed to be a concrete NSString subclass.
*/
void TestNSStringClass(Class stringClass);

/* Solaris, in particular, can't handle a NULL string in a printf statement */
#define FORMAT_STRING(str) ((str) ? str : "NULL")


#define IS_VALID_OBJECT(obj) (object_getClass((id)obj) != zombieClass)


Class stringClass;


/*
Basic sanity test.
*/
BOOL test_initWithCString(void)
{
  NSString *test1 = [[stringClass alloc] initWithCString: "ascii"];
  NSString *sanity = @"ascii";

  if (!test1)
    return NO;

  if (![sanity isEqualToString: test1])
    return NO;
  if (![test1 isEqualToString: sanity])
    return NO;
  if (![test1 isEqual: sanity])
    return NO;

  return YES;
}


/*
Test encoding and decoding in various character encodings.
*/
void
test_encodings_helper(NSStringEncoding encoding, 
  unsigned char *bytes, int bytes_length, 
  unichar *characters, int characters_length)
{
  NSData *encodedData = nil;
  NSString *decodedString = nil;
  NSData *referenceData = nil;
  NSString *referenceString = nil;
  NSStringEncoding *encodings;
  const char *enc;

  encodings = [NSString availableStringEncodings];
  if (0 != encodings)
    {
      while (*encodings != 0 && *encodings != encoding)
	{
	  encodings++;
	}
    }
  PASS(encodings != 0 && encoding == *encodings,
    "String encoding %d is supported", encoding);
  if (NO == testPassed)
    {
      return;
    }
  
  enc = [[NSString localizedNameOfStringEncoding: encoding] UTF8String];

  referenceString = [[stringClass alloc] initWithCharacters: characters
						     length: characters_length];
  referenceData = [[NSData alloc] initWithBytes: bytes
					 length: bytes_length];

  PASS(nil != (decodedString = [[stringClass alloc]
    initWithData: referenceData encoding: encoding]),
    "can instantiate string from %s", enc);
  if (YES == testPassed)
    {
      PASS_EQUAL(decodedString, referenceString,
	"new %s string matches reference", enc);
    }

  PASS_EQUAL((encodedData = [referenceString dataUsingEncoding: encoding]),
    referenceData, "data using encoding %s matches reference data", enc);

  DESTROY(decodedString);
  DESTROY(referenceData);
  DESTROY(referenceString);
}

void
test_encoding(void)
{
  {
    NSData *d = [[NSData alloc] initWithBytes: "foo"  length: 3];
    NSString *s = [[stringClass alloc] initWithData: d  encoding: 0];

    PASS(s == nil, "-initWithData:encoding: gives nil for invalid encodings")

    DESTROY(d);
  }

  test_encodings_helper(NSASCIIStringEncoding, 
    (unsigned char[]){65, 66, 67}, 3, 
    (unichar[]){65, 66, 67}, 3);

  test_encodings_helper(NSUTF8StringEncoding, 
    (unsigned char[]){65, 66, 67}, 3, 
    (unichar[]){65, 66, 67}, 3);

  test_encodings_helper(NSUTF8StringEncoding, 
    (unsigned char[]){0xc3, 0xa5, 0xc3, 0xa4, 0xc3, 0xb6, 
    0xd7, 0xa9, 0xd7, 0x9c, 0xd7, 0x95, 0xd7, 0x9d}, 14, 
    (unichar[]){0xe5, 0xe4, 0xf6, 0x5e9, 0x5dc, 0x5d5, 0x5dd}, 7);

  /* Codepoint U+2F801 CJK Compatiblity Ideograph */
  test_encodings_helper(NSUTF8StringEncoding, 
    (unsigned char[]){0xf0, 0xaf, 0xa0, 0x81}, 4, 
    (unichar[]){0xd87e, 0xdc01}, 2);

#if	defined(GNUSTEP_BASE_LIBRARY)
  test_encodings_helper(NSISOHebrewStringEncoding, 
    (unsigned char[]){0xf9, 0xec, 0xe5, 0xed}, 4, 
    (unichar[]){0x5e9, 0x5dc, 0x5d5, 0x5dd}, 4);
#endif

  test_encodings_helper(NSISOLatin1StringEncoding, 
    (unsigned char[]){116, 101, 115, 116, 45, 229, 228, 246}, 8, 
    (unichar[]){116, 101, 115, 116, 45, 229, 228, 246}, 8);

  test_encodings_helper(NSUTF8StringEncoding, 
    (unsigned char[]){0xe0, 0xb8, 0xa0, 0xe0, 0xb8, 0xb2, 0xe0, 0xb8, 0xa9, 
    0xe0, 0xb8, 0xb2, 0xe0, 0xb9, 0x84, 0xe0, 0xb8, 0x97, 
    0xe0, 0xb8, 0xa2}, 21, 
    (unichar[]){0xe20, 0xe32, 0xe29, 0xe32, 0xe44, 0xe17, 0xe22}, 7);

/*
;  (test-data-string
;    '(#xc0 #xd2 #xc9 #xd2 #xe4 #xb7 #xc2) 59 ; iso-8859-11, not yet implemented
;    '(#xe20 #xe32 #xe29
;      #xe32 #xe44 #xe17
;      #xe22) #t #t)
*/

#if	defined(GNUSTEP_BASE_LIBRARY)
  test_encodings_helper(NSBIG5StringEncoding, 
    (unsigned char[]){0x41, 0x42, 0x43, 0x20, 0xa7, 0x41, 0xa6, 0x6e, 0x21}, 9, 
    (unichar[]){0x41, 0x42, 0x43, 0x20, 0x4f60, 0x597d, 0x21}, 7);
#endif
}


BOOL test_getCString_maxLength_range_remainingRange(void)
{
  NS_DURING
    unsigned char *referenceBytes;
    int referenceBytesLength;
    NSString *referenceString;
    unsigned char buffer[16];
    NSRange remainingRange;
    int i, j;
    BOOL ok = YES;

    switch ([NSString defaultCStringEncoding])
      {
	case NSUTF8StringEncoding:
	  referenceBytes =(unsigned char []){0x41, 0xc3, 0xa5, 0x42};
	  referenceBytesLength = 4;
	  referenceString = [stringClass stringWithCharacters:
	    (unichar []){0x41, 0xe5, 0x42}
		  length: 3];
	  break;
	default:
	  printf("Have no reference string for c-string encoding %i,"
	    " skipping test.\n", [NSString defaultCStringEncoding]);
	  NS_VALUERETURN(YES, BOOL);
      }

    for (i = 0; i < referenceBytesLength; i++)
      {
	[referenceString getCString: buffer
		maxLength: i
		range: NSMakeRange(0, [referenceString length])
		remainingRange: &remainingRange];

	for (j = 0; j <= i ; j++)
	  if (buffer[j] == 0 || buffer[j] != referenceBytes[j])
	    break;
	if (buffer[j]!= 0)
	  {
	    PASS(0, "-getCString: maxLength: %i range: remainingRange: failed",
	      i);
	    ok = NO;
	  }
      }
    NS_VALUERETURN(ok, BOOL);
  NS_HANDLER
    printf("%s\n", POBJECT(localException));
    return NO;
  NS_ENDHANDLER
}


void test_return_self_optimizations(void)
{
  NSAutoreleasePool *arp;
  NSString *string, *returnValue;
  Class	zombieClass = NSClassFromString(@"NSZombie");


  arp = [NSAutoreleasePool new];
  string = [[stringClass alloc] initWithCharacters: NULL
	  length: 0];
  returnValue = [string lowercaseString];
  [string release];
  PASS((IS_VALID_OBJECT(returnValue) && [@"" isEqual: returnValue]), 
       "-lowercaseString returns a valid instance");
  DESTROY(arp);

  arp = [NSAutoreleasePool new];
  string = [[stringClass alloc] initWithCharacters: NULL
	  length: 0];
  returnValue = [string uppercaseString];
  [string release];
  PASS((IS_VALID_OBJECT(returnValue) && [@"" isEqual: returnValue]), 
       "-uppercaseString returns a valid instance");
  DESTROY(arp);

  arp = [NSAutoreleasePool new];
  string = [[stringClass alloc] initWithCharacters: NULL
	  length: 0];
  returnValue = [string capitalizedString];
  RELEASE(string);
  PASS((IS_VALID_OBJECT(returnValue) && [@"" isEqual: returnValue]), 
       "-capitalizedString returns a valid instance");
  DESTROY(arp);

  ENTER_POOL
  string = [[stringClass alloc] initWithCString: "helvetica"
	  length: 9];
  returnValue = [string capitalizedString];
  RELEASE(string);
  PASS_EQUAL(returnValue, @"Helvetica", "-capitalizedString for helvetica")
  LEAVE_POOL

  ENTER_POOL
  string = [[stringClass alloc] initWithCString: "HeLveTica"
	  length: 9];
  returnValue = [string capitalizedString];
  RELEASE(string);
  PASS_EQUAL(returnValue, @"Helvetica", "-capitalizedString for HeLveTica")
  LEAVE_POOL

  ENTER_POOL
  string = [[stringClass alloc] initWithCString: "HELVETICA"
	  length: 9];
  returnValue = [string capitalizedString];
  RELEASE(string);
  PASS_EQUAL(returnValue, @"Helvetica", "-capitalizedString for HELVETICA")
  LEAVE_POOL

  ENTER_POOL
  string = [[stringClass alloc] initWithCString: "HeLLO worLd"
	  length: 11];
  returnValue = [string capitalizedString];
  RELEASE(string);
  PASS_EQUAL(returnValue, @"Hello World", "-capitalizedString for HeLLO worLd")
  LEAVE_POOL

  arp = [NSAutoreleasePool new];
  string = [[stringClass alloc] initWithCharacters: NULL
	  length: 0];
  returnValue = [string description];
  PASS((IS_VALID_OBJECT(returnValue) && [@"" isEqual: returnValue]), 
       "-description returns a valid instance");
  [string release];
  DESTROY(arp);

  arp = [NSAutoreleasePool new];
  string = [[stringClass alloc] initWithCharacters: NULL
	  length: 0];
  returnValue = [string stringByExpandingTildeInPath];
  [string release];
  PASS([@"" isEqual: returnValue], "-stringByExpandingTildeInPath returns a valid instance (1)");
  DESTROY(arp);

  arp = [NSAutoreleasePool new];
  string = [[stringClass alloc] initWithCharacters: (unichar[]){0x41}
	  length: 1];
  returnValue = [string stringByExpandingTildeInPath];
  [string release];
  PASS((IS_VALID_OBJECT(returnValue) && [@"A" isEqual: returnValue]), 
       "-stringByExpandingTildeInPath returns a valid instance (2)");
  DESTROY(arp);

  arp = [NSAutoreleasePool new];
  string = [[stringClass alloc] initWithCharacters: (unichar[]){0x41}
	  length: 1];
  returnValue = [string stringByAbbreviatingWithTildeInPath];
  [string release];
  PASS((IS_VALID_OBJECT(returnValue) && [@"A" isEqual: returnValue]), 
       "-stringByAbbreviatingWithTildeInPath returns a valid instance");
  DESTROY(arp);

  /*
  TODO:
  -stringByPaddingToLength:...
  -stringByResolvingSymlinksInPath
  -stringByTrimmingCharactersInSet:
  */
}


void TestNSStringClass(Class aStringClass)
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

  stringClass = aStringClass;

  PASS(test_initWithCString(), "-initWithCString: works");
  test_encoding();
  PASS(test_getCString_maxLength_range_remainingRange(), "-getCString:maxLength:range:remainingRange: works");

  test_return_self_optimizations();

  [arp release]; arp = nil;
}

