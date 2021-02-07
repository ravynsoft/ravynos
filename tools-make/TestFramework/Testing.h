/* Testing - Include basic tests macros for the GNUstep Testsuite

   Copyright (C) 2005-2011 Free Software Foundation, Inc.

   Written by: Alexander Malmberg <alexander@malmberg.org>
   Updated by: Richard Frith-Macdonald <rfm@gnu.org>
 
   This package is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
*/

#ifndef Testing_h
#define Testing_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSException.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSRegularExpression.h>
#import <Foundation/NSString.h>

/* A flag indicating that the testsuite is currently processing tests
 * which are actually not expected to pass, but where we hope someone
 * might have committed a bugfix.
 * You should set this to YES at the start of any set of tests which
 * are actually unlikely to pass on all systems.
 * The state of this flag is preserved by sets ... on exit from a set
 * it is restored to the state it had on entry.
 * This flag is ignored if the tests are performed in 'developer' mode
 * (ie run with the gnustep-tests --developer option and therefore
 * compiled with the TESTDEV preprocessor macro defined).
 */
static BOOL testHopeful __attribute__((unused)) = NO;

/* A flag indicating whether the most recently executed test passed.
 * This is set by the pass() function (and therefore by any test macro).
 * Do not modify this directly.
 */
static BOOL testPassed __attribute__((unused)) = NO;

/* A variable storing the line number of the test currently being run.
 * Do not modify this directly.
 */
static unsigned testLineNumber __attribute__((unused)) = 0;

/* A variable storing the indentation of the set currently being run.
 * Do not modify this directly.
 */
static unsigned testIndentation __attribute__((unused)) = 0;
static inline void testIndent(void) __attribute__((unused));
static inline void testIndent(void)
{
  unsigned	i = testIndentation;
  while (i-- > 0)
    {
      fprintf(stderr, "  ");
    }
}

/* A variable set whenever a test macro is executed.  This contains
 * the exception which terminated the test macro, or nil if no exception
 * was raised.
 */
static NSException *testRaised __attribute__((unused)) = nil;

/* The setEnded function pointer may be set to a function which is to be
 * executed when the set ends, with its three parameters being the name
 * of the set, a flag to say whether the set completed successfully, and
 * the duration of the set.
 * The SET_TIMER() macro turns on/off timing and, if timeing was
 * already on, adds the time of the current period to the duration.
 */
static void (*setEnded)(const char *name, BOOL completed, double duration)
  __attribute__((unused)) = 0;
#define SET_TIMER(active) \
({ \
  double        started = _setTiming; \
  _setTiming = [NSDate timeIntervalSinceReferenceDate]; \
  if (started > 0.0) setDuration += _setTiming - started; \
  if (NO == active) _setTiming = 0.0; \
})\


/* The pass() function is the low-level core of the testsuite.
 *
 * You call this with two arguments ... an integer expression indicating the
 * success or failure of the testcase (0 is a failure) and a string which
 * describes the testcase.
 *
 * The global variable 'testHopeful' can be set to a non-zero value before
 * calling this function in order to specify that if the condition is
 * not true it should be treated as a dashed hope rather than a failure
 * (unless the tests are bing performed in 'developer' mode).
 *
 * If there is a better higher-level test macro available, please use
 * that instead.  In particular, please use the PASS_EQUAL() macro wherever
 * you wish to test the equality of a pair of objective-c objects.
 *
 * If you are calling the function directly, please use a format string
 * beginning "%s:%d" and pass __FILE__ and __LINE__ as the first two arguments
 * so that this function will print out the location it was called from.
 *
 * This function is the most efficient option for general use, but
 * please don't use it if there is any change that the evaluation of
 * the expression used as its first argument might cause an exception
 * in any context where that might be a problem.
 */
static void pass(int passed, const char *format, ...)  __attribute__((unused)) __attribute__ ((format(printf, 2, 3)));
static void pass(int passed, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  if (passed)
    {
      fprintf(stderr, "Passed test:     ");
      testPassed = YES;
    }
#if	!defined(TESTDEV)
  else if (YES == testHopeful)
    {
      fprintf(stderr, "Dashed hope:     ");
      testPassed = NO;
    }
#endif
  else
    {
      fprintf(stderr, "Failed test:     ");
      testPassed = NO;
    }
  testIndent();
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
#if	defined(FAILFAST)
  if (NO == testPassed && NO == testHopeful)
    {
      exit(1);	// Abandon testing now.
    }
#endif
}

/* The testStart() function is used by the PASS macros to provide a break
 * point in the source code after the current test line has been stored in
 * testLineNumber.
 * This is provided for when debugging ... you can set a breakpoint in the
 * testStart() function for the line number reported in a test failure and
 * have the debugger stop in just the right place.
 */
static void testStart()  __attribute__((unused));
static void testStart()
{
  return;
}

/* Tests a code expression which evaluates to an integer value.
 * The expression may not contain commas unless it is bracketed.
 * The format must be a literal string printf style format.
 * If the expression evaluates to zero the test does not pass.
 * If the expression causes an exception to be raised, the exception
 * is caught and logged but the test does not pass.
 * Otherwise, the test passes.
 * Basically equivalent to pass() but with exception handling.
 */
#define PASS(testExpression__, testFormat__, ...) \
  NS_DURING \
    { \
      int _cond; \
      id _tmp = testRaised; testRaised = nil; [_tmp release]; \
      testLineNumber = __LINE__; \
      testStart(); \
      _cond = (int)(testExpression__); \
      pass(_cond, "%s:%d ... " testFormat__, __FILE__, \
	__LINE__, ## __VA_ARGS__); \
    } \
  NS_HANDLER \
    testRaised = [localException retain]; \
    pass(0, "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
    printf("%s: %s", [[testRaised name] UTF8String], \
      [[testRaised description] UTF8String]); \
  NS_ENDHANDLER

/* This category declaration should keep the compiler happy ...
 * it defines an informal protocol specifying methods your test
 * classes may implement to aid with testing.
 */ 
@interface      NSObject(TestFramework)
/* The -isEqualForTestcase: method may be implemented in order to have
 * the PASS_EQUAL macro perform special equality testing rather than
 * using the normal equality test method (-isEqual).
 */
- (BOOL) isEqualForTestcase: (id)otherObject;
@end

/* Tests a code expression which evaluates to an object value.
 * The expression may not contain commas unless it is bracketed.
 * The expected value may not contain commas unless it is bracketed.
 * The format must be a literal string printf style format.
 *
 * Where the expression evaluates to an object which is identical to
 * the expect value, or where the expect value responds to -isEqualForTestcase:
 * and calling [expact -isEqualForTestcase: object] return YES,
 * or where the expect value does not respond to -isEqualForTestcase: and
 * calling [expect isEqual: object] returns YES, then the test has passed.
 *
 * The particularly useful thing about this macro is that, if the
 * results of the expression and the expected object are not equal,
 * the string representation of both values is logged so that you
 * can get a better idea of what went wrong.
 */
#define PASS_EQUAL(testExpression__, testExpect__, testFormat__, ...) \
  NS_DURING \
    { \
      int _cond; \
      id _obj; \
      id _exp; \
      id _tmp = testRaised; testRaised = nil; [_tmp release]; \
      testLineNumber = __LINE__; \
      testStart(); \
      _obj = (id)(testExpression__);\
      _exp = (id)(testExpect__);\
      if (_obj == _exp) \
        { \
          _cond = YES; \
        } \
      else if ([_obj respondsToSelector: @selector(isEqualForTestcase:)]) \
        { \
          _cond = (BOOL)[(id)_exp isEqualForTestcase: _obj]; \
        } \
      else \
        { \
          _cond = [_exp isEqual: _obj]; \
        } \
      pass(_cond, "%s:%d ... " testFormat__, __FILE__, \
        __LINE__, ## __VA_ARGS__); \
      if (0 == _cond) \
	{ \
          NSString  *s = [_obj description]; \
          if ([s length] == 1) \
            { \
              fprintf(stderr, \
		"Expected '%s' and got '%s' (unicode codepoint %d)\n", \
                [[_exp description] UTF8String], [s UTF8String], \
		[s characterAtIndex: 0]); \
            } \
	  else if (nil == s) \
	    { \
	      fprintf(stderr, "Expected '%s' and got (nil)\n", \
                [[_exp description] UTF8String]); \
	    } \
	  else \
	    { \
	      fprintf(stderr, "Expected '%s' and got '%s'\n", \
                [[_exp description] UTF8String], [s UTF8String]); \
	    } \
	} \
    } \
  NS_HANDLER \
    testRaised = [localException retain]; \
    pass(0, "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
    printf("%s: %s", [[testRaised name] UTF8String], \
      [[testRaised description] UTF8String]); \
  NS_ENDHANDLER

/* Tests a code expression which evaluates to an object value.
 * The expression may not contain commas unless it is bracketed.
 * The expected pattern may not contain commas unless it is bracketed.
 * The format must be a literal string printf style format.
 *
 * Where the expression evaluates to an object whose description matches
 * the expect value reguilar expression, the test has passed.
 *
 * If the results of the expression and the expected pattern o not match,
 * the string representation of both values is logged so that you
 * can get a better idea of what went wrong.
 */
#define PASS_MATCH(testExpression__, testExpect__, testFormat__, ...) \
  NS_DURING \
    { \
      int _cond; \
      id _obj; \
      id _dsc; \
      id _exp; \
      id _pat; \
      id _tmp = testRaised; testRaised = nil; [_tmp release]; \
      testLineNumber = __LINE__; \
      testStart(); \
      _obj = (id)(testExpression__);\
      _dsc = [_obj description];\
      _pat = (id)(testExpect__);\
      _exp = [[[NSRegularExpression alloc] initWithPattern: _pat \
        options: 0 error: 0] autorelease];\
      if (nil != _dsc && nil != _exp) \
        { \
          NSRange       r = NSMakeRange(0, [_dsc length]);\
          r = [_exp rangeOfFirstMatchInString: _dsc options: 0 range: r];\
          if (r.length > 0)\
            { \
              _cond = YES; \
            } \
        } \
      else \
        { \
          _cond = NO; \
        } \
      pass(_cond, "%s:%d ... " testFormat__, __FILE__, \
        __LINE__, ## __VA_ARGS__); \
      if (0 == _cond) \
	{ \
          if ([_dsc length] == 1) \
            { \
              fprintf(stderr, \
		"Expected '%s' and got '%s' (unicode codepoint %d)\n", \
                [[_pat description] UTF8String], [_dsc UTF8String], \
		[_dsc characterAtIndex: 0]); \
            } \
	  else if (nil == _dsc) \
	    { \
	      fprintf(stderr, "Expected '%s' and got (nil)\n", \
                [[_pat description] UTF8String]); \
	    } \
	  else \
	    { \
	      fprintf(stderr, "Expected '%s' and got '%s'\n", \
                [[_pat description] UTF8String], [_dsc UTF8String]); \
	    } \
	} \
    } \
  NS_HANDLER \
    testRaised = [localException retain]; \
    pass(0, "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
    printf("%s: %s", [[testRaised name] UTF8String], \
      [[testRaised description] UTF8String]); \
  NS_ENDHANDLER

/* Please use the PASS_EXCEPTION() macro to handle any code where you
 * want an exception to be thrown.  The macro checks that the supplied
 * code throws  an expection with the specified name.  If the code fails
 * to throw, or throws the wrong exception, then the code does not pass.
 * You can supply nil for expected exception name if you don't care about
 * the exact type of exception thrown.
 * The code fragment may not contain commas unless it is surrounded by
 * brackets. eg. PASS_EXCEPTION(({code here}), name, "hello")
 * The format must be a literal string printf style format.
 */
#define PASS_EXCEPTION(testCode__, testExpect__, testFormat__, ...) \
  NS_DURING \
    id _tmp = testRaised; testRaised = nil; [_tmp release]; \
    { \
      testLineNumber = __LINE__; \
      testStart(); \
      testCode__; \
    } \
    pass(0, "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  NS_HANDLER \
    testRaised = [localException retain]; \
    pass((nil == (testExpect__) \
      || [[testRaised name] isEqual: (testExpect__)]), \
      "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
    if (nil != (testExpect__) \
      && NO == [(testExpect__) isEqual: [testRaised name]]) \
      fprintf(stderr, "Expected '%s' and got '%s'\n", \
        [(testExpect__) UTF8String], \
        [[testRaised name] UTF8String]); \
  NS_ENDHANDLER

/* Please use the PASS_RUNS() macro to handle any code where you want the
 * code to run to completion without an exception being thrown, but you don't
 * have a particular expression to be checked.
 * The code fragment may not contain commas unless it is surrounded by
 * brackets. eg. PASS_EXCEPTION(({code here}), name, "hello")
 * The format must be a literal string printf style format.
 */
#define PASS_RUNS(testCode__, testFormat__, ...) \
  NS_DURING \
    id _tmp = testRaised; testRaised = nil; [_tmp release]; \
    { \
      testLineNumber = __LINE__; \
      testStart(); \
      testCode__; \
    } \
    pass(1, "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  NS_HANDLER \
    testRaised = [localException retain]; \
    pass(0, "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
    printf("%s: %s", [[testRaised name] UTF8String], \
      [[testRaised description] UTF8String]); \
  NS_ENDHANDLER


/* SETs are used to group multiple testcases or code which is outside of
 * the scope of the current test but could raise exceptions that should
 * be caught to allow further tests to run.
 *
 * You must pass a short description to identify the set at both its
 * start and its end.  This allows the seat to be easily identified in the
 * log, and also allows for checking to be sure that each start if a set
 * is matched by a corresponding end.
 *
 * The state of the 'testHopeful' flag is saved at the start of the set and
 * restored at the end of the set, so you can start your code by setting
 * 'testHopeful=YES;' to mark any tests within the set as being part of a
 * group of tests we don't expect to pass.
 *
 * Importantly, you may skip some or all of the tests in a set if those
 * tests are not supported in the package being tested (eg. testing of
 * functionality which depends on some external library which was not
 * available when the package being tested was buit).
 *
 * Any uncaught exception occurring inside a set will abort the entire set
 * so that remaining tests in the set will not be executed, but you may
 * also abandon remaining tests upon any test failure.
 *
 * The tests within the set are enclosed in an autorelease pool, and any
 * temporary objects are cleaned up at the end of the set.
 */

/* The START_SET() macro starts a set of grouped tests. It must be matched
 * by a corresponding END_SET() with the same string as an argument.
 * The argument is a short description to be printed in the log on entry.
 * The duration of each set is automatically timed (you can suspend/resume
 * timing using the SET_TIMER macro).  Each timed period is added to the
 * setDuration local variable while a set is executing (you can of course
 * modify this variable using code inside the set).
 */
#define START_SET(setName) \
  { \
    double setDuration = 0.0; \
    BOOL _setSuccess = YES; \
    double _setTiming = [NSDate timeIntervalSinceReferenceDate]; \
    BOOL _save_hopeful = testHopeful; \
    unsigned _save_indentation = testIndentation; \
    int	_save_line = __LINE__; \
    char *_save_set = (char*)malloc(strlen(setName) + 1); \
    strncpy(_save_set, setName, strlen(setName) + 1); \
    fprintf(stderr, "Start set:       "); \
    testIndent(); \
    fprintf(stderr, "%s:%d ... %s\n", __FILE__, __LINE__, _save_set); \
    testIndentation++; \
    NS_DURING \
      NSAutoreleasePool *_setPool = [NSAutoreleasePool new]; \
      {

/* Helper macro for END_SET() ... do not use directly.
 */
#if	defined(TESTDEV)
# define	OMITTED \
	{ \
	  fprintf(stderr, "Skipped set:     "); \
          testIndent(); \
	  fprintf(stderr, "%s\n", [[localException reason] UTF8String]); \
	}
#else
# define	OMITTED ;
#endif

/* The END_SET() macro terminates a set of grouped tests.  It must be matched
 * by a corresponding START_SET() with the same string as an argument.
 * The argument is a short description to be printed in the log on entry.
 * When a set ends, the function pointed to by the setEnded function is
 * called with three arguments which allow you to perform extra reporting
 * or cleanup etc.  The three arguments are the set name, a flag to say
 * whether the set completed successfully, and the duration of the set.
 */
#define END_SET(setName) \
      } \
    [_setPool release]; \
    NS_HANDLER \
      _setSuccess = NO; \
      if (YES == [[localException name] isEqualToString: @"SkipSet"]) \
	{ \
	  fprintf(stderr, "Skipped set:     "); \
          testIndent(); \
	  fprintf(stderr, "%s\n", [[localException reason] UTF8String]); \
	} \
      else if (YES == [[localException name] isEqualToString: @"OmitSet"]) \
	OMITTED \
      else \
	{ \
	  if (YES == [[localException name] isEqualToString: @"FailSet"]) \
	    { \
	      fprintf(stderr, "Failed set:      "); \
              testIndent(); \
	      fprintf(stderr, "%s:%d ... need not met in %s.\n", \
	        __FILE__, _save_line, _save_set); \
	    } \
	  else \
	    { \
	      fprintf(stderr, "EXCEPTION: %s %s %s\n", \
		[[localException name] UTF8String], \
		[[localException reason] UTF8String], \
		[[[localException userInfo] description] UTF8String]); \
	      fprintf(stderr, "Failed set:      "); \
              testIndent(); \
	      fprintf(stderr, "%s:%d ... problem in %s.\n", \
	        __FILE__, _save_line, _save_set); \
	    } \
	} \
    NS_ENDHANDLER \
    SET_TIMER(NO); \
    if (0 != setEnded) (*setEnded)(setName, _setSuccess, setDuration); \
    if (strcmp(_save_set, setName) != 0) \
      fprintf(stderr, "Error:      %s:%d ... END(%s) with START(%s).\n", \
        __FILE__, __LINE__, setName, _save_set); \
    testIndentation = _save_indentation; \
    fprintf(stderr, "End set:         "); \
    testIndent(); \
    fprintf(stderr, "%s:%d ... %s\n", __FILE__, __LINE__, _save_set); \
    free(_save_set); \
    testHopeful = _save_hopeful; \
  }

/* The NEED macro takes a test macro as an argument and breaks out of a set
 * and reports it as failed if the test does not pass.
 */
#define	NEED(testToTry) \
  {testToTry;} \
  if (NO == testPassed) \
    { \
      if (nil != testRaised) \
	{ \
	  [testRaised raise]; \
	} \
      else \
	{ \
	  [NSException raise: @"FailSet" format: @"Test did not pass"]; \
	} \
    }

/* The SKIP() macro skips the remainder of a set of grouped tests.
 * Its argument is a literal printf style format string and variable
 * arguments to print a message giving the reason for skipping the set.
 * This should be a short one line message (for immediate display),
 * preferably with a more detailed explanation on subsequent lines.
 */
#define	SKIP(testFormat__, ...) \
  [NSException raise: @"SkipSet" format: @"%s %d ... " testFormat__, \
  __FILE__, __LINE__, ## __VA_ARGS__];

/* The OMIT() macro acts just like SKIP() except that it only reports the
 * set if running in developer mode.  The idea is that it should be used for
 * groups of tests which are not expected to be available on most platforms
 * yet, so only developers should see them reported.
 */
#define	OMIT(testFormat__, ...) \
  [NSException raise: @"OmitSet" format: @"%s %d ... " testFormat__, \
  __FILE__, __LINE__, ## __VA_ARGS__];


/* some good macros to compare floating point numbers */
#import <math.h>
#import <float.h>
#define EQ(x, y) \
  (((x) >= ((y) - FLT_EPSILON*100)) && ((x) <= ((y) + FLT_EPSILON*100)))
#define LE(x, y) ((x)<(y) || EQ(x, y))
#define GE(x, y) ((y)<(x) || EQ(x, y))
#define LT(x, y) (!GE(x, y))
#define GT(x, y) (!LE(x, y))

/* A convenience macro to pass an object as a string to a print function. 
 */
#define POBJECT(obj)      [[(obj) description] UTF8String]

#endif

#ifndef	CREATE_AUTORELEASE_POOL
#define	RETAIN(object)		[object retain]
#define	RELEASE(object)		[object release]
#define	AUTORELEASE(object)	[object autorelease]
#define	TEST_RETAIN(object)	({\
id __object = (id)(object); (__object != nil) ? [__object retain] : nil; })
#define	TEST_RELEASE(object)	({\
id __object = (id)(object); if (__object != nil) [__object release]; })
#define	TEST_AUTORELEASE(object)	({\
id __object = (id)(object); (__object != nil) ? [__object autorelease] : nil; })
#define	ASSIGN(object,value)	({\
id __value = (id)(value); \
id __object = (id)(object); \
if (__value != __object) \
  { \
    if (__value != nil) \
      { \
	[__value retain]; \
      } \
    object = __value; \
    if (__object != nil) \
      { \
	[__object release]; \
      } \
  } \
})
#define	ASSIGNCOPY(object,value)	({\
id __value = (id)(value); \
id __object = (id)(object); \
if (__value != __object) \
  { \
    if (__value != nil) \
      { \
	__value = [__value copy]; \
      } \
    object = __value; \
    if (__object != nil) \
      { \
	[__object release]; \
      } \
  } \
})
#define	DESTROY(object) 	({ \
  if (object) \
    { \
      id __o = object; \
      object = nil; \
      [__o release]; \
    } \
})
#define	CREATE_AUTORELEASE_POOL(X)	\
  NSAutoreleasePool *(X) = [NSAutoreleasePool new]
#define RECREATE_AUTORELEASE_POOL(X)  \
  if (X == nil) \
    (X) = [NSAutoreleasePool new]
#endif

