//
//  NSCondition_posix.m
//  Foundation
//
//  Created by Sven Weidauer on 08.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//
#ifdef PLATFORM_IS_POSIX

#import "NSCondition_posix.h"


@implementation NSCondition_posix

- init;
{
	if ([super init] == nil) return nil;
	
	pthread_mutex_init( &mutex, 0 );
	pthread_cond_init( &condition, 0 );
	
	return self;
}

- (void)dealloc;
{
	pthread_cond_destroy( &condition );
	pthread_mutex_destroy( &mutex );
	[super dealloc];
}

- (void) lock;
{
	pthread_mutex_lock( &mutex );
}

- (void) unlock;
{
	pthread_mutex_unlock( &mutex );
}

- (void) signal;
{
	pthread_cond_signal( &condition );
}

- (void) broadcast;
{
	pthread_cond_broadcast( &condition );
}

- (void) wait;
{
	pthread_cond_wait( &condition, &mutex );
}

@end
#endif
