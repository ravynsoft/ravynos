#import <Foundation/Foundation.h>
#import "Testing.h"

int main()
{ 
  NSAutoreleasePool   *pool = [NSAutoreleasePool new];
  id	o;

  /* The path utilities provide information about the current user and
   * the current filesystem layout ... and are inherently system dependent
   * so we can't know if they are operating correctly.  The best we can do
   * are trivial checks to see if their results look sane.
   */
  NSLog(@"NSUserName() %@", o = NSUserName());
  PASS([o length] > 0, "we can get a user name");
  NSLog(@"NSFullUserName() %@", o = NSFullUserName());
  PASS([o length] > 0, "we can get a full user name");
  NSLog(@"NSHomeDirectory() %@", o = NSHomeDirectory());
  PASS([o length] > 0, "we can get a home directory");
  NSLog(@"NSTemporaryDirectory() %@", o = NSTemporaryDirectory());
  PASS([o length] > 0, "we can get a temporary directory");
  NSLog(@"NSOpenStepRootDirectory() %@", o = NSOpenStepRootDirectory());
  PASS([o length] > 0, "we can get a root directory");
  NSLog(@"NSHomeDirectoryForUser(@\"non-existent-user\") %@",
    o = NSHomeDirectoryForUser(@"non-existent-user"));
  PASS(nil == o, "home directory for non existent user is nil");

  /* These functions have been removed in recent OSX but are retained in GNUstep
   */
#if     defined(GNUSTEP_BASE_LIBRARY)
  NSLog(@"NSStandardApplicationPaths() %@", o = NSStandardApplicationPaths());
  PASS([o count] > 0, "we have application paths");
  NSLog(@"NSStandardLibraryPaths() %@", o = NSStandardLibraryPaths());
  PASS([o count] > 0, "we have library paths");
#endif

  [pool release]; pool = nil;
 
  return 0;
}
