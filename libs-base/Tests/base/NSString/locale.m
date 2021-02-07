#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSLocale.h>

#if	defined(GS_USE_ICU)
#define	NSLOCALE_SUPPORTED	GS_USE_ICU
#else
#define	NSLOCALE_SUPPORTED	1 /* Assume Apple support */
#endif

static void testBasic(void)
{
  NSComparisonResult compRes;
  NSRange range;

  range = [@"ababa" rangeOfString: @"b"
			  options: 0
			    range: NSMakeRange(2, 3)
			   locale: [[[NSLocale alloc] initWithLocaleIdentifier: @"en_US"] autorelease]];

  PASS(NSEqualRanges(NSMakeRange(3,1), range), "expected {3,1}, got {%d, %d}", (int)range.location,
       (int)range.length);


  compRes = [@"hello" compare: @"Hello"];		
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);

  compRes = [@"H" compare: @"i"];		
  PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);
  
  compRes = [@"h" compare: @"H"];
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);

  compRes = [@"hello" compare: @"Hello" options: 0 range: NSMakeRange(0, 5) locale: nil];
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);
  
  compRes = [@"h" compare: @"H" options: 0 range: NSMakeRange(0, 1) locale: nil];
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);

  compRes = [@"hello" compare: @"H" options: 0 range: NSMakeRange(0, 1) locale: nil];
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);

  compRes = [@"a" compare: @"Z"];
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);

  compRes = [@"z" compare: @"A"];
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);
  
  compRes = [@"a" localizedCompare: @"Z"];
  PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);
  
  compRes = [@"z" localizedCompare: @"A"];
  PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);

  
  // These two may be considered implementation details
  {
    BOOL wasHopeful = testHopeful;
    testHopeful = YES;
    
    compRes = [@"hello" compare: @"Hello" options: 0 range: NSMakeRange(0, 5)
			 locale: [[[NSLocale alloc] initWithLocaleIdentifier: @"en_US_POSIX"] autorelease]];
    PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);	
    
    
    compRes = [@"hello" compare: @"Hello" options: 0 range: NSMakeRange(0, 5)
			 locale: [NSLocale systemLocale]];
    PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);

    testHopeful = wasHopeful;
  }

  compRes = [@"hello" compare: @"Hello" options: 0 range: NSMakeRange(0, 5)
		       locale: [[[NSLocale alloc] initWithLocaleIdentifier: @"en_US"] autorelease]];
  PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);

  compRes = [@"hello" localizedCompare: @"Hello"];
  PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);

  compRes = [@"H" localizedCompare: @"i"];
  PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);
}

static void testEszett(void)
{
  NSComparisonResult compRes;
  NSRange range;
    
  const unichar EszettChar = 0x00df;
  NSString *EszettStr = [[[NSString alloc] initWithCharacters: &EszettChar
						       length: 1] autorelease];

  NSString *EszettPrefixStr = [EszettStr stringByAppendingString: @"abcdef"];
  NSString *EszettSuffixStr = [@"abcdef" stringByAppendingString: EszettStr];
  NSString *EszettPrefixSuffixStr = [NSString stringWithFormat: @"%@abcdef%@", EszettStr, EszettStr];
    
  // test compare:
    
  compRes = [EszettStr compare: @"Ss"
		       options: NSCaseInsensitiveSearch
			 range: NSMakeRange(0, 1)];

  PASS(compRes == 0, "Ss compares equal to Eszett character with"
       " NSCaseInsensitiveSearch. got %d", (int)compRes);
    
  compRes = [EszettStr compare: @"S"
		       options: 0
			 range: NSMakeRange(0, 1)];

  PASS(compRes == 1, "Eszett compare: S is NSOrderedDescending. got %d", (int)compRes);
    
  compRes = [EszettStr compare: @"s"
		       options: 0
			 range: NSMakeRange(0, 1)];

  PASS(compRes == 1, "Eszett compare: s is NSOrderedDescending. got %d", (int)compRes);
    
  // test rangeOfString:
    
  range = [EszettPrefixStr rangeOfString: @"sS"
				 options: NSCaseInsensitiveSearch
				   range: NSMakeRange(0, 7)];
    
  PASS(NSEqualRanges(range, NSMakeRange(0, 1)), "with NSCaseInsensitiveSearch range of sS"
       " in <Eszett>abcdef is {0,1}. got {%d,%d}", (int)range.location, (int)range.length);
    
  range = [EszettPrefixStr rangeOfString: @"sS"
				 options: 0
				   range: NSMakeRange(0, 7)];
    
  PASS(NSEqualRanges(range, NSMakeRange(NSNotFound, 0)), "without NSCaseInsensitiveSearch, "
       "range of sS in <Eszett>abcdef is {NSNotFound, 0}. got {%d,%d}",
       (int)range.location, (int)range.length);
    
  range = [EszettPrefixStr rangeOfString: @"sS"
				 options: NSCaseInsensitiveSearch | NSAnchoredSearch | NSBackwardsSearch
				   range: NSMakeRange(0, 7)];
    
  PASS(NSEqualRanges(range, NSMakeRange(NSNotFound, 0)), "for anchored backwards search, "
       "range of sS in <Eszett>abcdef is {NSNotFound, 0}. got {%d,%d}",
       (int)range.location, (int)range.length);

  range = [EszettPrefixStr rangeOfString: @"sS"
				 options: NSCaseInsensitiveSearch | NSAnchoredSearch
				   range: NSMakeRange(0, 7)];
    
  PASS(NSEqualRanges(range, NSMakeRange(0, 1)), "for anchored forwards search, "
       "range of sS in <Eszett>abcdef is {0, 1}. got {%d,%d}",
       (int)range.location, (int)range.length);
    
  range = [EszettSuffixStr rangeOfString: @"sS"
				 options: NSCaseInsensitiveSearch | NSAnchoredSearch | NSBackwardsSearch
				   range: NSMakeRange(0, 7)];

  PASS(NSEqualRanges(range, NSMakeRange(6, 1)), "for anchored backwards search, "
       "range of sS in abcdef<Eszett> is {6, 1}. got {%d,%d}",
       (int)range.location, (int)range.length);
    
  range = [EszettPrefixSuffixStr rangeOfString: @"sS"
				       options: NSCaseInsensitiveSearch 
					 range: NSMakeRange(0, 8)];
    
  PASS(NSEqualRanges(range, NSMakeRange(0, 1)), "for forward search, "
       "range of sS in <Eszett>abcdef<Eszett> is {0, 1}. got {%d,%d}",
       (int)range.location, (int)range.length);    

  range = [EszettPrefixSuffixStr rangeOfString: @"sS"
				       options: NSCaseInsensitiveSearch | NSBackwardsSearch
					 range: NSMakeRange(0, 8)];
    
  PASS(NSEqualRanges(range, NSMakeRange(7, 1)), "for backward search, "
       "range of sS in <Eszett>abcdef<Eszett> is {7, 1}. got {%d,%d}",
       (int)range.location, (int)range.length); 
}

static void testLithuanian(void)
{
  // "in Lithuanian, "y" is sorted between "i" and "k"."
  //   -- http://userguide.icu-project.org/collation

  NSComparisonResult compRes;

  compRes = [@"y" compare: @"k" options: 0 range: NSMakeRange(0,1) locale: nil];

  PASS(compRes == NSOrderedDescending, "y>k in nil locale. comparison result: %d",
       (int)compRes);

  compRes = [@"y" compare: @"k" options: 0 range: NSMakeRange(0,1) locale: 
		[[[NSLocale alloc] initWithLocaleIdentifier: @"lt"] autorelease]];
  
  PASS(compRes == NSOrderedAscending, "y<k in 'lt' locale. comparison result: %d",
       (int)compRes);
}

static void testDiacritics(void)
{
	NSComparisonResult compRes;

	const unichar eAcuteChar = 0x00e9;
	NSString *eAcuteStr = [[[NSString alloc] initWithCharacters: &eAcuteChar
														 length: 1] autorelease];
	const unichar EAcuteChar = 0x00c9;
	NSString *EAcuteStr = [[[NSString alloc] initWithCharacters: &EAcuteChar
														 length: 1] autorelease];

	
	compRes = [@"e" compare: eAcuteStr];
	PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);

	compRes = [@"e" compare: EAcuteStr];
	PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);
	
	compRes = [@"e" localizedCompare: eAcuteStr];
	PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);
	
	compRes = [@"e" localizedCompare: EAcuteStr];
	PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);
	
	compRes = [eAcuteStr compare: EAcuteStr];
	PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);	
	
	compRes = [eAcuteStr localizedCompare: EAcuteStr];
	PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);

	// test NSCaseInsensitiveSearch
	
	compRes = [@"e" compare: eAcuteStr options: NSCaseInsensitiveSearch];
	PASS(compRes == NSOrderedAscending, "expected -1 got %d", (int)compRes);

	compRes = [eAcuteStr compare: EAcuteStr options: NSCaseInsensitiveSearch];
	PASS(compRes == NSOrderedSame, "expected 0 got %d", (int)compRes);	

	// test NSDiacriticInsensitiveSearch
	
	compRes = [@"e" compare: eAcuteStr options: NSDiacriticInsensitiveSearch];
	PASS(compRes == NSOrderedSame, "expected 0 got %d", (int)compRes);
	
	compRes = [@"e" compare: EAcuteStr options: NSDiacriticInsensitiveSearch];
	PASS(compRes == NSOrderedDescending, "expected 1 got %d", (int)compRes);

	// test NSCaseInsensitiveSearch | NSDiacriticInsensitiveSearch
	
	compRes = [@"e" compare: eAcuteStr options: NSCaseInsensitiveSearch | NSDiacriticInsensitiveSearch];
	PASS(compRes == NSOrderedSame, "expected 0 got %d", (int)compRes);
	
	compRes = [@"e" compare: EAcuteStr options: NSCaseInsensitiveSearch | NSDiacriticInsensitiveSearch];
	PASS(compRes == NSOrderedSame, "expected 0 got %d", (int)compRes);
	
	compRes = [@"E" compare: eAcuteStr options: NSCaseInsensitiveSearch | NSDiacriticInsensitiveSearch];
	PASS(compRes == NSOrderedSame, "expected 0 got %d", (int)compRes);
	
	compRes = [@"E" compare: EAcuteStr options: NSCaseInsensitiveSearch | NSDiacriticInsensitiveSearch];
	PASS(compRes == NSOrderedSame, "expected 0 got %d", (int)compRes);
	
	compRes = [EAcuteStr compare: eAcuteStr options: NSCaseInsensitiveSearch | NSDiacriticInsensitiveSearch];
	PASS(compRes == NSOrderedSame, "expected 0 got %d", (int)compRes);
}

int main()
{
  START_SET("NSString + locale")
  
  if (!NSLOCALE_SUPPORTED)
    SKIP("NSLocale not supported\nThe ICU library was not available when GNUstep-base was built")

  testHopeful = YES;
  
  testBasic();
  testEszett();
  testLithuanian();
  testDiacritics();

  END_SET("NSString + locale")

  return 0;
}
