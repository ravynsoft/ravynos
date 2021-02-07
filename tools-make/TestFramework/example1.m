#import	"Testing.h"

/* This is the absolute minimal test program ...
 * a single test case involving plain C and no Objective-C code.
 * 
 * If you run the test with 'gnustep-tests example1.m' it should
 * report a single test pass
 */
int
main()
{
  /* This is too simple to really be useful, but it's a start.
   */
  pass(1 == 1, "integer equality works");
  return 0;
}
