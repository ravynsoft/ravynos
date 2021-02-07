/*
   diningPhilosophers.h

   Five hungry philosophers testing locks and threads
   This program loops indefinitely.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996

   This file is part of the GNUstep Application Kit Library.

   This file is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this file; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA.
*/

#include <Foundation/NSLock.h>
#include <Foundation/NSThread.h>
#include    <Foundation/NSAutoreleasePool.h>
#include    <Foundation/NSValue.h>

// Conditions
#define NO_FOOD 1
#define FOOD_SERVED 2

// NSLocks ... umm I mean forks
id forks[5];
id fork_lock;

//
// A class of hungry philosophers
//
@interface Philosopher : NSObject

{
  int chair;
}

// Instance methods
- (void)sitAtChair:(NSNumber*)position;
- (int)chair;

@end

@implementation Philosopher

// Instance methods
- (void)sitAtChair:(NSNumber*)position
{
	int i;

	// Sit down
	chair = [position intValue];

	// Its a constant battle to feed yourself
	while (1)
	{

		[fork_lock lockWhenCondition:FOOD_SERVED];

		// Get the fork to our left
		[forks[chair] lockWhenCondition:FOOD_SERVED];

		// Get the fork to our right
		[forks[(chair + 1) % 5] lockWhenCondition:FOOD_SERVED];

		[fork_lock unlock];

		// Start eating!
		printf("Philosopher %d can start eating.\n", chair); fflush(stdout);

		for (i = 0;i < 100000; ++i)
		{
			if ((i % 10000) == 0)
				printf("Philosopher %d is eating.\n", chair); fflush(stdout);
		}

		// Done eating
		printf("Philosopher %d is done eating.\n", chair); fflush(stdout);

		// Drop the fork to our left
		[forks[chair] unlock];

		// Drop the fork to our right
		[forks[(chair + 1) % 5] unlock];

		// Wait until we are hungry again
		for (i = 0;i < 1000000 * (chair + 1); ++i) ;
	}

	// We never get here, but this is what we should do
	[NSThread exit];
}

- (int)chair
{
	return chair;
}

@end

//
// my main for the test app
//
int main()
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  int i;
  id p[5];

  // Create the locks
  for (i = 0;i < 5; ++i)
  {
	  forks[i] = [[NSConditionLock alloc]
		  initWithCondition:NO_FOOD];
	  [forks[i] lock];
  }

	fork_lock = [[NSConditionLock alloc]
		initWithCondition:NO_FOOD];
	[fork_lock lock];

  // Create the philosophers
  for (i = 0;i < 5; ++i)
	  p[i] = [[Philosopher alloc] init];

  // Have them sit at the table
  for (i = 0;i < 5; ++i)
	  [NSThread detachNewThreadSelector:@selector(sitAtChair:)
		  toTarget:p[i] withObject: [NSNumber numberWithInt: i]];

  // Now let them all eat
	[fork_lock unlockWithCondition:FOOD_SERVED];
  for (i = 0;i < 5; ++i)
	  [forks[i] unlockWithCondition:FOOD_SERVED];

  while (1);
  [arp release];
}

