#ifndef	INCLUDED_GSTASKREGISTRY_H
#define	INCLUDED_GSTASKREGISTRY_H

#import "common.h"

@class NSArray;
@class NSURLSessionTask;

/*
 * This helper class keeps track of all tasks.
 *
 * Each `NSURLSession` has a `GSTaskRegistry` for its running tasks. 
 *
 * - Note: This must **only** be accessed on the owning session's work queue.
 */
@interface GSTaskRegistry : NSObject

- (void ) addTask: (NSURLSessionTask*)task;

- (void) removeTask: (NSURLSessionTask*)task;

- (void) notifyOnTasksCompletion: (void (^)(void))tasksCompletion;

- (NSArray*) allTasks;

- (BOOL) isEmpty;

@end

#endif
