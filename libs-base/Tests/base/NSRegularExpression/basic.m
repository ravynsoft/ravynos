
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSRegularExpression.h>
#import <Foundation/NSTextCheckingResult.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSValue.h>

@interface DegeneratePatternTest : NSObject
{
  NSRegularExpression *expression;
  NSString* input;
}
@end

@implementation DegeneratePatternTest

- (instancetype) init
{
  if (nil == (self = [super init]))
    {
      return nil;
    }
  expression =
    [[NSRegularExpression alloc] initWithPattern: @"^(([a-z])+.)+[A-Z]([a-z])+$"
                                         options: 0
                                          error: NULL];
  ASSIGN(input, @"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!");
  return self;
}

- (void) runTest: (id)obj
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  [expression matchesInString: input
                      options: 0
                        range: NSMakeRange(0, [input length])];
  DESTROY(pool);
}

- (void) dealloc
{
  DESTROY(expression);
  DESTROY(input);
  [super dealloc];
}
@end


int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  START_SET("NSRegularExpression")

#if !(__APPLE__ || GS_USE_ICU)
  SKIP("NSRegularExpression not built\nThe ICU library was not available when GNUstep-base was built")
#else
# ifdef GNUSTEP
    // Ensure that a deterministic limit is set up for this process
    NSUserDefaults *dflts = [NSUserDefaults standardUserDefaults];
    NSDictionary *domain = [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithInt: 1500], @"GSRegularExpressionWorkLimit", nil];
    [dflts setVolatileDomain: domain
                     forName: @"GSTestDomain"];
# endif
  id testObj = [[NSRegularExpression alloc] initWithPattern: @"^a"
                                                    options: 0
                                                      error: NULL];

  test_NSObject(@"NSRegularExpression",
                [NSArray arrayWithObject:
                  [[NSRegularExpression alloc] initWithPattern: @"^a"
                                                       options: 0
                                                         error: NULL]]);
  test_NSCopying(@"NSRegularExpression",@"NSRegularExpression",
                 [NSArray arrayWithObject:testObj],NO,NO);

  NSAutoreleasePool *iPool = [NSAutoreleasePool new];
    {
      PASS_EQUAL([testObj pattern], @"^a", "Correct pattern returned");
    }
  DESTROY(iPool);
  /* Regression test: We had a double-free bug on retrieving pattern,
   * which is also the reason for wrapping the previous one in an ARP */
  PASS_RUNS([testObj pattern], "Calling -pattern twice runs");

  // The pattern does not include a capture group $1, so this should return 
  // nil;
  NSString *replacement = @"should be unset on return";
  replacement =  [testObj stringByReplacingMatchesInString: @"ab"
                                                   options: 0
                                                     range: NSMakeRange(0,2)
                                              withTemplate: @"$1c"];
  PASS(replacement == nil, "Replacement: returns nil on capture group error");
  replacement = [testObj stringByReplacingMatchesInString: @"ab"
                                                  options: 0
                                                    range: NSMakeRange(0, 2)
                                             withTemplate: @"c"];
  PASS_EQUAL(replacement, @"cb", "Replacement correct");

  NSString *replMut = [NSMutableString stringWithString: @"ab"];
  [testObj replaceMatchesInString: replMut
                          options: 0
                            range: NSMakeRange(0,2)
                     withTemplate: @"$1c"];
  PASS_EQUAL(replMut, @"ab", 
             "Mutable replacement: Incorrect template does not change string");
  replMut = [NSMutableString stringWithString: @"ab"];
  [testObj replaceMatchesInString: replMut
                          options: 0
                            range: NSMakeRange(0,2)
                     withTemplate: @"c"];
  PASS_EQUAL(replMut, @"cb",
             "Mutable replacement: Correct replacement for template");

  NSTextCheckingResult *r = [testObj firstMatchInString: @"ab"
                                                options: 0
                                                  range: NSMakeRange(0,2)];
  PASS(r, "Found NSTextCheckingResult");
  replacement = @"should be unset on return";
  replacement = [testObj replacementStringForResult: r
                                           inString: @"ab"
                                             offset: 0
                                           template: @"$1c"];
  PASS(replacement == nil,
       "Custom replacement: returns nil on capture group error");
  replacement = nil;
  replacement = [testObj replacementStringForResult: r
                                           inString: @"ab"
                                             offset: 0
                                           template: @"c"];
  PASS_EQUAL(replacement, @"c",
             "Custom replacement: Returns correct replacement");

  NSRegularExpression *testObj2 =
    [[NSRegularExpression alloc] initWithPattern: @"bc"
					 options: 0
					   error: NULL];
  r = [testObj2 firstMatchInString: @"abcdeabcde"
			   options: 0
			     range: NSMakeRange(5, 5)];
  PASS(r != nil && NSEqualRanges([r range], NSMakeRange(6, 2)),
       "Restricting the range for firstMatchInString: works");

  /* To test whether we correctly bail out of processing degenerate patterns,
   * we spin up a new thread and evaluate an expression there. The expectation
   * is that the thread should terminate within a few seconds.
   *
   * NOTE: Since we cannot terminate the thread in case of a failure, this
   * test should be run last.
   */
  DegeneratePatternTest *test = [DegeneratePatternTest new];
  NSThread *thread = [[NSThread alloc] initWithTarget: test
                                              selector: @selector(runTest:)
                                                object: nil];
  [thread start];
  [thread setName: @"PatternTestRunner"];
  NSDate *started = [NSDate date];
  NSRunLoop *rl = [NSRunLoop currentRunLoop];
  /* We spin the runloop for a bit while we wait for the other thread to bail
   * out */
  while ([thread isExecuting] && abs([started timeIntervalSinceNow] < 10.0f))
    {
      [rl runMode: NSDefaultRunLoopMode
       beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
    }
  PASS(NO == [thread isExecuting], "Faulty regular expression terminated");
#endif

  END_SET("NSRegularExpression")
  [arp release]; arp = nil;
  return 0;
}
