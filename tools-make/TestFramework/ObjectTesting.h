/* ObjectTesting - Include basic object tests for the GNUstep Testsuite

   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by: Matt Rice?
 
   This package is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
*/
#import <Testing.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSData.h>
#import <Foundation/NSArchiver.h>
#import <Foundation/NSKeyedArchiver.h>

/* This file contains macros of testing some basic protocol implementation
 * common to almost all classes.
 *
 * The reason for using macros rather than functions is that it allows the
 * preprocessor and compiler to generate messages containign the file name
 * and line number of the location of problems in your own testcase code.
 *
 * Sometimes, with a complex testing process, you want the location of the
 * problem  within that process ... so to aid that we also have function
 * equivalents of the macros.
 */

/* Macro to perform basic allocation tests
 */
#define TEST_ALLOC(CN) \
{ \
  NSString *className = (CN); \
  Class theClass = NSClassFromString(className); \
  id obj0 = nil; \
  id obj1 = nil; \
  const char *prefix = [[NSString stringWithFormat: @"Class '%@'", className] \
    UTF8String]; \
  NSZone *testZone = NSCreateZone(1024, 1024, 1); \
  PASS(theClass != Nil, "%s exists", prefix); \
\
  obj0 = [theClass alloc]; \
  PASS(obj0 != nil, "%s has working alloc", prefix); \
  PASS([obj0 isKindOfClass: theClass], \
    "%s alloc gives the correct class", prefix); \
  obj0 = [[theClass alloc] init]; \
  PASS([obj0 isKindOfClass: theClass], "%s has working init", prefix); \
\
  obj0 = [theClass new]; \
  PASS([obj0 isKindOfClass: theClass], "%s has working new", prefix); \
\
  obj1 = [theClass allocWithZone: testZone]; \
  PASS([obj1 isKindOfClass: theClass],"%s has working allocWithZone",prefix); \
}
static void test_alloc(NSString *CN) __attribute__ ((unused));
static void test_alloc(NSString *CN)
{
  TEST_ALLOC(CN);
}

/* Macro to perform basic allocation tests without initialisation
 */
#define TEST_ALLOC_ONLY(CN) \
{ \
  NSString *className = (CN); \
  Class theClass = NSClassFromString(className); \
  id obj0 = nil; \
  id obj1 = nil; \
  const char *prefix = [[NSString stringWithFormat: @"Class '%@'", className] \
    UTF8String]; \
  NSZone *testZone = NSCreateZone(1024, 1024, 1); \
  PASS(theClass != Nil, "%s exists", prefix); \
   \
  obj0 = [theClass alloc]; \
  PASS(obj0 != nil, "%s has working alloc", prefix); \
  PASS([obj0 isKindOfClass: theClass], \
    "%s alloc gives the correct class", prefix); \
  PASS_EXCEPTION([obj0 description], NSInvalidArgumentException, \
    "raises NSInvalidArgumentException in description") \
\
  PASS_EXCEPTION(if([obj0 init]==nil)[NSException raise: NSInvalidArgumentException format: @""], \
    NSInvalidArgumentException, \
    "returns nil or raises NSInvalidArgumentException in init") \
\
  PASS_EXCEPTION(if([theClass new]==nil)[NSException raise: NSInvalidArgumentException format: @""], \
    NSInvalidArgumentException, \
    "returns nil or raises NSInvalidArgumentException in new") \
\
  obj1 = [theClass allocWithZone: testZone]; \
  PASS([obj1 isKindOfClass: theClass],"%s has working allocWithZone",prefix); \
}
static void test_alloc_only(NSString *CN) __attribute__ ((unused));
static void test_alloc_only(NSString *CN)
{
  TEST_ALLOC_ONLY(CN);
}


/* Macro to test for the NSObject protocol
 * Arguments are:
 * CN   The name of the class to be tested
 * OJS  An arraayof objects to be tested
 */
#define TEST_NSOBJECT(CN, OJS) \
{ \
  NSString *className = (CN); \
  NSArray *objects = (OJS); \
  int i; \
  Class theClass = Nil; \
  theClass = NSClassFromString(className); \
  PASS(theClass != Nil, "%s is a known className", [className UTF8String]); \
   \
  for (i = 0; i < [objects count]; i++) \
    { \
      id theObj = [objects objectAtIndex: i]; \
      id mySelf = nil; \
      Class myClass = Nil; \
      int count1; \
      int count2; \
      Class sup = Nil; \
      const char *prefix; \
      id r; \
 \
      prefix = [[NSString stringWithFormat: @"Object %i of class '%@'", \
        i, className] UTF8String]; \
      PASS([theObj conformsToProtocol: @protocol(NSObject)], \
	"%s conforms to NSObject", prefix); \
      mySelf = [theObj self]; \
      PASS(mySelf == theObj, "%s can return self", prefix); \
      myClass = [theObj class]; \
      PASS(myClass != Nil, "%s can return own class", prefix); \
      PASS([theObj isKindOfClass: theClass], \
	"%s object %.160s is of correct class", prefix, \
	[[theObj description] UTF8String]); \
      PASS(mySelf == myClass ? ![theObj isMemberOfClass: myClass] \
	: [theObj isMemberOfClass: myClass], \
        "%s isMemberOfClass works", prefix); \
      sup = [theObj superclass]; \
      PASS(theClass == NSClassFromString(@"NSObject") ? sup == nil \
	: (sup != nil && sup != myClass), "%s can return superclass", prefix); \
      PASS([theObj respondsToSelector: @selector(hash)], \
	"%s responds to hash", prefix); \
      PASS([theObj isEqual: theObj], "%s isEqual: to self", prefix); \
      PASS([theObj respondsToSelector: @selector(self)], \
	"%s respondsToSelector: ", prefix); \
      [theObj isProxy]; \
      r = [theObj retain]; \
      PASS(theObj == r, "%s handles retain", prefix);  \
      [theObj release]; \
      [theObj retain]; \
      [theObj autorelease]; \
 \
      count1 = [theObj retainCount]; \
      [theObj retain]; \
      [theObj release]; \
      count2 = [theObj retainCount]; \
      PASS((count1 == count2), "%s has working retainCount", prefix); \
      PASS([[theObj description] isKindOfClass: [NSString class]], \
	"%s has NSString description", prefix); \
      PASS([theObj performSelector: @selector(self)] == theObj, \
	"%s handles performSelector", prefix);     \
    } \
}
static void test_NSObject(NSString *CN, NSArray *OJS) __attribute__ ((unused));
static void test_NSObject(NSString *CN, NSArray *OJS)
{
  TEST_NSOBJECT(CN, OJS)
}


/* Archives each object in the array, then unarchives it and checks that
 * the two are equal (using the PASS_EQUAL macro).
 */
#define TEST_NSCODING(OJS) \
{ \
  NSArray *objects = (OJS); \
  int i; \
  for (i = 0; i < [objects count]; i++) \
    { \
      char buf[100]; \
      id obj = [objects objectAtIndex: i]; \
      const char *prefix; \
      NSMutableData *data; \
      NSArchiver *archiver; \
      id decoded; \
\
      snprintf(buf, sizeof(buf), "test_NSCoding object %u", i); \
      START_SET(buf) \
	PASS([[[obj class] description] length], \
	  "I can extract a class name for object"); \
\
	prefix = [[NSString stringWithFormat: @"Object %i of class '%s'", i, \
	  [NSStringFromClass([obj class]) UTF8String]] UTF8String]; \
	PASS([obj conformsToProtocol: @protocol(NSCoding)], \
	  "conforms to NSCoding protocol"); \
	data = (NSMutableData *)[NSMutableData data]; \
	archiver = [[NSArchiver alloc] initForWritingWithMutableData: data]; \
	PASS(archiver != nil, "I am able to set up an archiver"); \
	data = nil; \
	[archiver encodeRootObject: obj]; \
	data = [archiver archiverData]; \
	PASS(data && [data length] > 0, "%s can be encoded", prefix); \
	decoded = [NSUnarchiver unarchiveObjectWithData: data]; \
	PASS(decoded != nil, "can be decoded"); \
        PASS_EQUAL(decoded, obj, "decoded object equals the original"); \
      END_SET(buf) \
    } \
}
static void test_NSCoding(NSArray *OJS) __attribute__ ((unused));
static void test_NSCoding(NSArray *OJS)
{
  TEST_NSCODING(OJS);
}


/* Archives each object in the argument array,
 * then unarchives it and checks that the two are
 * equal using the PASS_EQUAL macro.
 */
#define TEST_KEYED_NSCODING(OJS) \
{ \
  NSArray       *objects = (OJS); \
  int i; \
  for (i = 0; i < [objects count]; i++) \
    { \
      char buf[100]; \
      id obj = [objects objectAtIndex: i]; \
      const char *prefix; \
      NSData *data; \
      id decoded; \
\
      snprintf(buf, sizeof(buf), "test_keyed_NSCoding object %u", i); \
      START_SET(buf) \
	PASS([[[obj class] description] length], \
	  "I can extract a class name for object"); \
\
	prefix = [[NSString stringWithFormat: @"Object %i of class '%s'", i, \
	  [NSStringFromClass([obj class]) UTF8String]] UTF8String]; \
	PASS([obj conformsToProtocol: @protocol(NSCoding)], \
	  "conforms to NSCoding protocol"); \
	data = [NSKeyedArchiver archivedDataWithRootObject: obj]; \
	PASS([data length] > 0, "%s can be encoded", prefix); \
	decoded = [NSKeyedUnarchiver unarchiveObjectWithData: data]; \
	PASS(decoded != nil, "can be decoded"); \
        PASS_EQUAL(decoded, obj, "decoded object equals the original") \
      END_SET(buf) \
    } \
}
static void test_keyed_NSCoding(NSArray *OJS) __attribute__ ((unused));
static void test_keyed_NSCoding(NSArray *OJS)
{
  TEST_KEYED_NSCODING(OJS);
}


/* A macro for testing that objects conform to, and
 * implement the NSCopying protocol.
 * Macro arguments are:
 * ICN  An NSString object containing the name of the
 *      immutable class to be immutably copied.
 * MCN  An NSString object containing the name of the
 *      mutable class corresponding to the immutable
 *      class.
 * OJS  An NSArray object containg one or more objects 
 *      to be tested.
 * MRT  A flag saying whether copies of an immutable
 *      instance in the same zone must be implemented
 *      by simply retaining the original.
 * MCP  A flag saying whether copies of an immutable
 *      instance must always be made as real copies
 *      rather than by retaining the original.
 */
#define TEST_NSCOPYING(ICN, MCN, OJS, MRT, MCP) \
{ \
  NSString *iClassName = (ICN); \
  NSString *mClassName = (MCN); \
  NSArray *objects = (OJS); \
  BOOL mustRetain = (MRT); \
  BOOL mustCopy = (MCP); \
  Class iClass = NSClassFromString(iClassName); \
  Class mClass = NSClassFromString(mClassName); \
  int i; \
  NSZone *testZone = NSCreateZone(1024, 1024, 1); \
\
  PASS(iClass != Nil, "%s is a known class", [iClassName UTF8String]); \
  PASS(mClass != Nil, "%s is a known class", [mClassName UTF8String]); \
\
  for (i = 0; i < [objects count]; i++) \
    { \
      char buf[100]; \
      BOOL immutable; \
      NSString *theName; \
      const char *prefix; \
      id theCopy = nil; \
      Class theClass = Nil; \
      id theObj = [objects objectAtIndex: i]; \
\
      snprintf(buf, sizeof(buf), "test_NSCopying object %u", i); \
      START_SET(buf) \
	if (iClass != mClass && [theObj isKindOfClass: mClass]) \
	  { \
	    immutable = NO; \
	    theName = iClassName; \
	    theClass = iClass; \
	  } \
	else \
	  { \
	    immutable = YES; \
	    theName = mClassName; \
	    theClass = mClass; \
	  } \
\
	prefix = [[NSString stringWithFormat: @"Object %i of class '%s'", \
	  i, [theName UTF8String]] UTF8String]; \
	PASS([theObj conformsToProtocol: @protocol(NSCopying)], \
	  "conforms to NSCopying"); \
	theCopy = [theObj copy]; \
	PASS(theCopy != nil, "%s understands -copy", prefix); \
	PASS([theCopy isKindOfClass: iClass], \
	  "%s copy is of correct type", prefix); \
	PASS_EQUAL(theCopy, theObj, \
	  "%s original and copy are equal", prefix); \
	if (immutable) \
	  { \
	    if (YES == mustRetain) \
	      { \
		PASS(theCopy == theObj, \
		  "%s is retained by copy with same zone", prefix); \
	      } \
	    else if (YES == mustCopy) \
	      { \
		PASS(theCopy != theObj, \
		  "%s is not retained by copy with same zone", prefix); \
	      } \
	  } \
	if (theClass != iClass) \
	  { \
	    PASS(![theCopy isKindOfClass: theClass], \
	      "%s result of copy is not immutable", prefix); \
	  } \
\
	theCopy = [theObj copyWithZone: testZone]; \
	PASS(theCopy != nil, "%s understands -copyWithZone", prefix); \
	PASS([theCopy isKindOfClass: iClass], \
	  "%s zCopy has correct type", prefix); \
	PASS_EQUAL(theCopy, theObj, \
	  "%s copy and original are equal", prefix); \
	if (immutable) \
	  { \
	     if (YES == mustRetain) \
	       { \
		 PASS(theCopy == theObj, \
		   "%s is retained by copy with other zone", prefix); \
	       } \
	     else if (YES == mustCopy) \
	       { \
		 PASS(theCopy != theObj, \
		   "%s is not retained by copy with other zone", prefix); \
	       } \
	  } \
       if (theClass != iClass) \
	 PASS(![theCopy isKindOfClass: theClass], \
	   "%s result of copyWithZone: is not immutable", prefix); \
      END_SET(buf) \
    } \
}
static void test_NSCopying(
  NSString *ICN, NSString *MCN, NSArray *OJS, BOOL MRT, BOOL MCP)
  __attribute__ ((unused));
static void test_NSCopying(
  NSString *ICN, NSString *MCN, NSArray *OJS, BOOL MRT, BOOL MCP)
{
  TEST_NSCOPYING(ICN, MCN, OJS, MRT, MCP);
}



/* A macro for testing that objects conform to, and
 * implement the mutable copying protocol.
 * Macro arguments are:
 * ICN  An NSString object containing the name of the
 *      immutable class to be mutably copied.
 * MCN  An NSString object containing the name of the
 *      mutable class to be produced as a result of
 *      the copy.
 * OJS  An NSArray object containg one or more objects 
 *      to be tested.
 */
#define TEST_NSMUTABLECOPYING(ICN, MCN, OJS) \
{ \
  NSString *iClassName = (ICN); \
  NSString *mClassName = (MCN); \
  NSArray *objects = (OJS); \
  int i; \
  Class iClass = Nil; \
  Class mClass = Nil; \
  NSZone *testZone = NSCreateZone(1024, 1024, 1); \
  iClass = NSClassFromString(iClassName); \
  PASS(iClass != Nil, "%s is a known class", [iClassName UTF8String]); \
\
  mClass = NSClassFromString(mClassName); \
  PASS(mClass != Nil, "%s is a known class", [mClassName UTF8String]); \
\
  for (i = 0; i < [objects count]; i++) \
    { \
      char buf[100]; \
      id theObj = [objects objectAtIndex: i]; \
      NSString *theName = nil; \
      const char *prefix; \
      BOOL immutable; \
      id theCopy = nil; \
\
      snprintf(buf, sizeof(buf), "test_NSMutableCopying object %u", i); \
      START_SET(buf); \
	if (iClass == mClass && [theObj isKindOfClass: mClass]) \
	  immutable = NO; \
	else \
	  immutable = YES; \
\
	if (YES == immutable) \
	  { \
	    theName = iClassName; \
	  } \
	else \
	  { \
	    theName = mClassName; \
	  } \
\
	prefix = [[NSString stringWithFormat: \
	  @"Object %i of class '%s'", i, [theName UTF8String]] UTF8String]; \
	PASS([theObj conformsToProtocol: @protocol(NSMutableCopying)], \
	  "%s conforms to NSMutableCopying protocol", prefix); \
	theCopy = [theObj mutableCopy]; \
	PASS(theCopy != nil, "%s understands -mutableCopy", prefix); \
	PASS([theCopy isKindOfClass: mClass], \
	  "%s mutable copy is of correct type", prefix); \
        PASS_EQUAL(theCopy, theObj, \
          "%s copy object equals the original", prefix); \
	PASS(theCopy != theObj, \
	  "%s not retained by mutable copy in the same zone", \
	  [mClassName UTF8String]); \
\
	theCopy = [theObj mutableCopyWithZone: testZone]; \
	PASS(theCopy != nil, \
	  "%s understands mutableCopyWithZone", [mClassName UTF8String]); \
	PASS(theCopy != theObj, \
          "%s not retained by mutable copy in other zone", \
	  [mClassName UTF8String]); \
      END_SET(buf) \
    } \
}
static void test_NSMutableCopying(
  NSString *ICN, NSString *MCN, NSArray *OJS) __attribute__ ((unused));
static void test_NSMutableCopying(NSString *ICN, NSString *MCN, NSArray *OJS)
{
  TEST_NSMUTABLECOPYING(ICN, MCN, OJS);
}




/* DEPRECATED ... please use the START_SET/END_SET and PASS macros instead.
   START_TEST/END_TEST can be used if the code being tested could raise
   and the exception should be considered a test failure.  The exception
   is not reraised to allow subsequent tests to execute.  The START_TEST
   macro takes an argument which will skip the test as Skipped if it
   evaluates to 0, allowing runtime control of whether the code block 
   should be executed.
 */
#define START_TEST(supported) if ((supported)) { NS_DURING 
#define END_TEST(result, desc, args...) \
  pass(result, desc, ## args); \
  NS_HANDLER \
    fprintf(stderr, "EXCEPTION: %s %s %s\n", \
      [[localException name] UTF8String], \
      [[localException reason] UTF8String], \
      [[[localException userInfo] description] UTF8String]); \
    pass (NO, desc, ## args); NS_ENDHANDLER } \
  else { fprintf(stderr, "Failed test: " desc, ## args); \
    fprintf(stderr, "\n"); }


/* Quick test to check that we have the class we expect.
 */
#define TEST_FOR_CLASS(aClassName, aClass, TestDescription) \
  PASS([aClass isKindOfClass: NSClassFromString(aClassName)], TestDescription)

/* Quick test to check for a non-empty string in the case where we don't
 * actually know what value we should be expecting.
 */
#define TEST_STRING(code, description) \
  { \
    NSString *_testString = code; \
    pass(_testString != nil \
      && [_testString isKindOfClass: [NSString class]] \
      && [_testString length], description); \
  }

