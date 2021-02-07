#import "GSTaskRegistry.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSException.h"
#import "Foundation/NSURLSession.h"


@implementation GSTaskRegistry
{
  NSMutableDictionary  *_tasks;
  void                 (^_tasksCompletion)(void);
}

- (instancetype) init 
{
  if (nil != (self = [super init])) 
    {
      _tasks = [[NSMutableDictionary alloc] init];
    }
  
  return self;
}

- (void) dealloc
{
  DESTROY(_tasks);
  [super dealloc];
}

- (NSArray*) allTasks 
{
  return [_tasks allValues];
}

- (BOOL) isEmpty 
{
  return [_tasks count] == 0;
}

- (void) notifyOnTasksCompletion: (void (^)(void))tasksCompletion 
{
  _tasksCompletion = tasksCompletion;
}

- (void) addTask: (NSURLSessionTask*)task
{
  NSString          *identifier;
  NSUInteger        taskIdentifier;
  NSURLSessionTask  *t;

  taskIdentifier = [task taskIdentifier];

  NSAssert(taskIdentifier != 0, @"Invalid task identifier");
  
  identifier = [NSString stringWithFormat: @"%lu", taskIdentifier];

  if (nil != (t = [_tasks objectForKey: identifier])) 
    {
      if ([t isEqual: task]) 
        {
          NSAssert(NO,
            @"Trying to re-insert a task that's already in the registry.");
        } 
      else 
        {
          NSAssert(NO,
            @"Trying to insert a task, but a different task with the same"
            @" identifier is already in the registry.");
        }
    }
  
  [_tasks setObject: task forKey: identifier];
}

- (void) removeTask: (NSURLSessionTask*)task 
{
  NSString          *identifier;
  NSUInteger        taskIdentifier;

  taskIdentifier = [task taskIdentifier];

  NSAssert(taskIdentifier != 0, @"Invalid task identifier");
  
  identifier = [NSString stringWithFormat: @"%lu", taskIdentifier];

  if (nil == [_tasks objectForKey: identifier])
    {
      NSAssert(NO, @"Trying to remove task, but it's not in the registry."); 
    }

  [_tasks removeObjectForKey: identifier];

  if (nil != _tasksCompletion && [self isEmpty]) 
    {
      _tasksCompletion();
    }
}

@end
