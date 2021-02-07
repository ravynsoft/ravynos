#import	"Testing.h"

/* A fourth test ... testing for an exception.
 *
 * If you run the test with 'gnustep-tests example4.m' it should
 * report three test passes.
 */

/* Import a header because we want to use a method from it.
 */
#import	<Foundation/NSDictionary.h>

int
main()
{
  START_SET("example set")

  /* We test for the code fragment raising an exception.  We don't care
   * about the particular exception, so we pass nil as the expected exception
   * name.
   */
  PASS_EXCEPTION([(NSDictionary*)@"abc" objectForKey: @"xxx"], nil,
    "sending a bad message causes an exception")
  pass(1 == 1, "integer equality works");
  PASS([[NSObject new] autorelease] != nil, "+new creates an object")

  END_SET("example set")

  return 0;
}
