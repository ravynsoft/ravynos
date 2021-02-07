#import "GSTimeoutSource.h"

@implementation GSTimeoutSource

- (instancetype) initWithQueue: (dispatch_queue_t)queue
                  milliseconds: (NSInteger)milliseconds
                       handler: (dispatch_block_t)handler 
{
  if (nil != (self = [super init])) 
    {
      _queue = queue;
      _handler = handler;
      _milliseconds = milliseconds;
      _rawSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _queue);

      uint64_t delay = MAX(1, milliseconds - 1);

      dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, delay * NSEC_PER_MSEC);

      dispatch_source_set_timer(_rawSource, start, delay * NSEC_PER_MSEC, _milliseconds == 1 ? 1 * NSEC_PER_USEC : 1 * NSEC_PER_MSEC);
      dispatch_source_set_event_handler(_rawSource, _handler);
      dispatch_resume(_rawSource);
    }
  return self;
}

- (void) dealloc 
{
  dispatch_source_cancel(_rawSource);
  [super dealloc];
}

- (NSInteger) milliseconds
{
  return _milliseconds;
}

- (dispatch_queue_t) queue
{
  return _queue;
}

- (dispatch_block_t) handler
{
  return _handler;
}

@end