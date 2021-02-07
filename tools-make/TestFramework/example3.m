#import	"Testing.h"

/* A third test ... using test macros.
 *
 * If you run the test with 'gnustep-tests example3.m' it should
 * report two test passes, and a test fail.
 */

/* Import a header because we want to use a method from it.
 */
#import	<Foundation/NSDictionary.h>

int
main()
{
  /* We start a set here ...
   * Having a set means we do not need to bother creating an autorelease pool.
   */
  START_SET("example set")

  /* We use a macro here so that any exception in the expression we use
   * will not break out of the set, and the two remaining tests will be
   * run.
   */
  PASS([(NSDictionary*)@"abc" objectForKey: @"xxx"],
    "sending a bad message")

  pass(1 == 1, "integer equality works");

  /* And let's use a macro here too ... the expression is not going to
   * raise an exception unless NSObject is somehow broken, and even if
   * it did, this is the last test in the set, so it wouldn't matter,
   * but it's good practice to code safely in case we move the code
   * around in a later version of the program.
   */
  PASS([[NSObject new] autorelease] != nil, "+new creates an object")

  END_SET("example set")

  return 0;
}
