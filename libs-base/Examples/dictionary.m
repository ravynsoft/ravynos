/* A simple demonstration of the NSDictionary object.

  Copyright (C) 2005 Free Software Foundation

  Copying and distribution of this file, with or without modification,
  are permitted in any medium without royalty provided the copyright
  notice and this notice are preserved.

   In this example the NSDictionary holds int's which are keyed by strings. */


#include <Foundation/Foundation.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSString.h>

int
main()
{
  NSMutableDictionary	*d;

  ENTER_POOL

  /* Create a Dictionary object. */
  d = [[NSMutableDictionary alloc] initWithCapacity: 32];

  /* Load the dictionary with some items */
  [d setObject: [NSNumber numberWithInt: 1] forKey: @"one"];
  [d setObject: [NSNumber numberWithInt: 2] forKey: @"two"];
  [d setObject: [NSNumber numberWithInt: 3] forKey: @"three"];
  [d setObject: [NSNumber numberWithInt: 4] forKey: @"four"];
  [d setObject: [NSNumber numberWithInt: 5] forKey: @"five"];
  [d setObject: [NSNumber numberWithInt: 6] forKey: @"six"];

  NSLog(@"There are %u elements stored in the dictionary\n", [d count]);

  NSLog(@"Element %d is stored at \"%s\"\n",
    [[d objectForKey: @"three"] intValue], "three");

  NSLog(@"Removing element stored at \"three\"\n");
  [d removeObjectForKey: @"three"];

  NSLog(@"Now there are %u elements stored in the dictionary\n", [d count]);

  LEAVE_POOL
  exit(0);
}
