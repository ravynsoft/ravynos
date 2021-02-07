/**
 * The GSRunLoopCtxt stores context information to handle polling for
 * events.  This information is associated with a particular runloop
 * mode, and persists throughout the life of the runloop instance.
 *
 *	NB.  This class is private to NSRunLoop and must not be subclassed.
 */

#import "common.h"

#import "Foundation/NSError.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSNotificationQueue.h"
#import "Foundation/NSPort.h"
#import "Foundation/NSStream.h"
#import "../GSRunLoopCtxt.h"
#import "../GSRunLoopWatcher.h"
#import "../GSPrivate.h"

#define	FDCOUNT	1024

static SEL	wRelSel;
static SEL	wRetSel;
static IMP	wRelImp;
static IMP	wRetImp;

static void
wRelease(NSMapTable* t, void* w)
{
  (*wRelImp)((id)w, wRelSel);
}

static void
wRetain(NSMapTable* t, const void* w)
{
  (*wRetImp)((id)w, wRetSel);
}

static const NSMapTableValueCallBacks WatcherMapValueCallBacks = 
{
  wRetain,
  wRelease,
  0
};

@implementation	GSRunLoopCtxt

+ (void) initialize
{
  wRelSel = @selector(release);
  wRetSel = @selector(retain);
  wRelImp = [[GSRunLoopWatcher class] instanceMethodForSelector: wRelSel];
  wRetImp = [[GSRunLoopWatcher class] instanceMethodForSelector: wRetSel];
}

- (void) dealloc
{
  RELEASE(mode);
  GSIArrayEmpty(performers);
  NSZoneFree(performers->zone, (void*)performers);
  GSIArrayEmpty(timers);
  NSZoneFree(timers->zone, (void*)timers);
  GSIArrayEmpty(watchers);
  NSZoneFree(watchers->zone, (void*)watchers);
  if (handleMap != 0)
    {
      NSFreeMapTable(handleMap);
    }
  if (winMsgMap != 0)
    {
      NSFreeMapTable(winMsgMap);
    }
  GSIArrayEmpty(_trigger);
  NSZoneFree(_trigger->zone, (void*)_trigger);
  [super dealloc];
}

/**
 * Remove any callback for the specified event which is set for an
 * uncompleted poll operation.<br />
 * This is called by nested event loops on contexts in outer loops
 * when they handle an event ... removing the event from the outer
 * loop ensures that it won't get handled twice, once by the inner
 * loop and once by the outer one.
 */
- (void) endEvent: (void*)data
              for: (GSRunLoopWatcher*)watcher
{
  if (completed == NO)
    {
      unsigned i = GSIArrayCount(_trigger);

      while (i-- > 0)
	{
	  GSIArrayItem	item = GSIArrayItemAtIndex(_trigger, i);

	  if (item.obj == (id)watcher)
	    {
	      GSIArrayRemoveItemAtIndex(_trigger, i);
	    }
	}
      switch (watcher->type)
	{
	  case ET_RPORT:
	  case ET_HANDLE:
	    NSMapRemove(handleMap, data);
	    break;
	  case ET_WINMSG:
	    NSMapRemove(winMsgMap, data);
	    break;
	  case ET_TRIGGER:
	    // Already handled
	    break;
	  default:
	    NSLog(@"Ending an event of unexpected type (%d)", watcher->type);
	    break;
	}
    }
}

/**
 * Mark this poll context as having completed, so that if we are
 * executing a re-entrant poll, the enclosing poll operations
 * know they can stop what they are doing because an inner
 * operation has done the job.
 */
- (void) endPoll
{
  completed = YES;
}

- (id) init
{
  [NSException raise: NSInternalInconsistencyException
	      format: @"-init may not be called for GSRunLoopCtxt"];
  return nil;
}

- (id) initWithMode: (NSString*)theMode extra: (void*)e
{
  self = [super init];
  if (self != nil)
    {
      NSZone	*z;

      mode = [theMode copy];
      extra = e;
      z = [self zone];
      performers = NSZoneMalloc(z, sizeof(GSIArray_t));
      timers = NSZoneMalloc(z, sizeof(GSIArray_t));
      watchers = NSZoneMalloc(z, sizeof(GSIArray_t));
      _trigger = NSZoneMalloc(z, sizeof(GSIArray_t));
      GSIArrayInitWithZoneAndCapacity(performers, z, 8);
      GSIArrayInitWithZoneAndCapacity(timers, z, 8);
      GSIArrayInitWithZoneAndCapacity(watchers, z, 8);
      GSIArrayInitWithZoneAndCapacity(_trigger, z, 8);

      handleMap = NSCreateMapTable(NSIntegerMapKeyCallBacks,
              WatcherMapValueCallBacks, 0);
      winMsgMap = NSCreateMapTable(NSIntegerMapKeyCallBacks,
              WatcherMapValueCallBacks, 0);
    }
  return self;
}

/*
 * If there is a generic watcher (watching hwnd == 0),
 * loop through all events, and send them to the correct
 * watcher (if there are any) and then process the rest right here.
 * Return a flag to say whether any messages were handled.
 */
- (BOOL) processAllWindowsMessages:(int)num_winMsgs within: (NSArray*)contexts
{
  MSG			msg;
  GSRunLoopWatcher	*generic = nil;
  unsigned		i;
  BOOL			handled = NO;

  if (num_winMsgs > 0)
    {
      generic = NSMapGet(winMsgMap,0);
    }
  
  if (generic != nil && generic->_invalidated == NO)
    {
      while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
	  if (num_winMsgs > 0)
	    {
	      HANDLE		handle;
	      GSRunLoopWatcher	*watcher;

	      handle = msg.hwnd;
	      watcher = (GSRunLoopWatcher*)NSMapGet(winMsgMap,
		(void*)handle);
	      if (watcher == nil || watcher->_invalidated == YES)
		{
		  handle = 0;	// Generic
		  watcher
		    = (GSRunLoopWatcher*)NSMapGet(winMsgMap, (void*)handle);
		}
	      if (watcher != nil && watcher->_invalidated == NO)
		{
		  i = [contexts count];
		  while (i-- > 0)
		    {
		      GSRunLoopCtxt *c = [contexts objectAtIndex: i];

		      if (c != self)
			{ 
			  [c endEvent: (void*)handle for: watcher];
			}
		    }
		  handled = YES;
		  /*
		   * The watcher is still valid - so call the
		   * receiver's event handling method.
		   */
		  [watcher->receiver receivedEvent: watcher->data
					      type: watcher->type
					     extra: (void*)&msg
					   forMode: mode];
		  continue;
		}
	    }
	  TranslateMessage(&msg); 
	  DispatchMessage(&msg);
	}
    }
  else
    {
      if (num_winMsgs > 0)
	{
	  unsigned		num = num_winMsgs;
	  NSMapEnumerator	hEnum;
	  HANDLE		handle;
	  GSRunLoopWatcher	*watcher;

	  hEnum = NSEnumerateMapTable(winMsgMap);
	  while (NSNextMapEnumeratorPair(&hEnum, &handle, (void**)&watcher))
	    {
	      if (watcher->_invalidated == NO)
		{
		  while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
		    {
		      i = [contexts count];
		      while (i-- > 0)
			{
			  GSRunLoopCtxt *c = [contexts objectAtIndex: i];
			      
			  if (c != self)
			    {
			      [c endEvent: (void*)handle for: watcher];
			    }
			}
		      handled = YES;
		      [watcher->receiver receivedEvent: watcher->data
						  type: watcher->type
						 extra: (void*)&msg
					       forMode: mode];
		    }
		}
	      num--;
	    }
	  NSEndMapTableEnumeration(&hEnum);
	} 
    }
  return handled;
}

- (BOOL) pollUntil: (int)milliseconds within: (NSArray*)contexts
{
  GSRunLoopThreadInfo   *threadInfo = GSRunLoopInfoForThread(nil);
  NSMapEnumerator	hEnum;
  GSRunLoopWatcher	*watcher;
  HANDLE		handleArray[MAXIMUM_WAIT_OBJECTS-1];
  int			num_handles;
  int			num_winMsgs;
  unsigned		count;
  unsigned		i;
  void			*handle;
  int			wait_timeout;
  DWORD			wait_return;
  BOOL			immediate = NO;
  BOOL			existingMessages = NO;

  // Set timeout how much time should wait
  if (milliseconds >= 0)
    {
      wait_timeout = milliseconds;
    }
  else
    {
      wait_timeout = INFINITE;
    }

  NSResetMapTable(handleMap);
  NSResetMapTable(winMsgMap);
  GSIArrayRemoveAllItems(_trigger);

  i = GSIArrayCount(watchers);
  num_handles = 1;              // One handle for signals from other threads
  num_winMsgs = 0;

  while (i-- > 0)
    {
      GSRunLoopWatcher	*info;
      BOOL		trigger;
      
      info = GSIArrayItemAtIndex(watchers, i).obj;
      if (info->_invalidated == YES)
	{
	  GSIArrayRemoveItemAtIndex(watchers, i);
	}
      else if ([info runLoopShouldBlock: &trigger] == NO)
	{
	  if (trigger == YES)
	    {
	      immediate = YES;
	      GSIArrayAddItem(_trigger, (GSIArrayItem)(id)info);
	    }
	}
      else
	{
	  HANDLE	handle;

	  switch (info->type)
	    {
	      case ET_HANDLE:
		handle = (HANDLE)(size_t)info->data;
		NSMapInsert(handleMap, (void*)handle, info);
		num_handles++;
		break;
	      case ET_RPORT:
		{
		  id port = info->receiver;
		  NSInteger port_hd_size = FDCOUNT;
		  NSInteger port_hd_count = FDCOUNT;
		  NSInteger port_hd_buffer[FDCOUNT];
		  NSInteger *port_hd_array = port_hd_buffer;

		  [port getFds: port_hd_array count: &port_hd_count];
                  while (port_hd_count > port_hd_size)
                    {
                      if (port_hd_array != port_hd_buffer) free(port_hd_array);
                      port_hd_size = port_hd_count;
                      port_hd_count = port_hd_size;
                      port_hd_array = malloc(sizeof(NSInteger)*port_hd_size);
                      [port getFds: port_hd_array count: &port_hd_count];
                    }
		  NSDebugMLLog(@"NSRunLoop", @"listening to %d port handles",
		    port_hd_count);
		  while (port_hd_count--)
		    {
		      NSMapInsert(handleMap, 
			(void*)(size_t) port_hd_array[port_hd_count],
			info);
		      num_handles++;
		    }
                  if (port_hd_array != port_hd_buffer) free(port_hd_array);
		}
		break;
	      case ET_WINMSG:
		handle = (HANDLE)(size_t)info->data;
		NSMapInsert(winMsgMap, (void*)handle, info);
		num_winMsgs++;
		break;
	      case ET_TRIGGER:
		break;
	    }
	}
    }
    
  /*
   * If there are notifications in the 'idle' queue, we try an
   * instantaneous select so that, if there is no input pending,
   * we can service the queue.  Similarly, if a task has completed,
   * we need to deliver its notifications.
   */
  if (GSPrivateCheckTasks() || GSPrivateNotifyMore(mode) || immediate == YES)
    {
      wait_timeout = 0;
    }

  handleArray[0] = threadInfo->event; // Signal from other thread
  num_handles = NSCountMapTable(handleMap) + 1;
  if (num_handles >= MAXIMUM_WAIT_OBJECTS)
    {
      NSLog(@"Too many handles to wait for ... only using %d of %d",
        MAXIMUM_WAIT_OBJECTS-1, num_handles);
      num_handles = MAXIMUM_WAIT_OBJECTS-1;
    }
  count = num_handles - 1;	// Count of handles excluding thread event
  if (count > 0)
    {
      i = 1 + (fairStart++ % count);
      hEnum = NSEnumerateMapTable(handleMap);
      while (count-- > 0
	&& NSNextMapEnumeratorPair(&hEnum, &handle, (void**)&watcher))
	{
	  if (i >= num_handles)
	    {
	      i = 1;
	    }
	  handleArray[i++] = (HANDLE)handle;
	}
      NSEndMapTableEnumeration(&hEnum);
    }

  completed = NO;

  /* Clear all the windows messages first before we wait,
   * since MsgWaitForMultipleObjects only signals on NEW messages
   */
  if ([self processAllWindowsMessages: num_winMsgs within: contexts] == YES)
    {
      // Processed something ... no need to wait.
      wait_timeout = 0;
      num_winMsgs = 0;
      existingMessages = YES;
    }

  if (num_winMsgs > 0)
    {
      NSDebugMLLog(@"NSRunLoop",
	@"wait for messages and %d handles for %d milliseconds",
	num_handles, wait_timeout);
	
      /*
       * Wait for signalled events or window messages.
       */
      wait_return = MsgWaitForMultipleObjects(num_handles, handleArray, 
        NO, wait_timeout, QS_ALLINPUT);
    }
  else if (num_handles > 0)
    {
      NSDebugMLLog(@"NSRunLoop",
	@"wait for %d handles for %d milliseconds", num_handles, wait_timeout);

      /*
       * We are not interested in windows messages ... just wait for
       * signalled events.
       */
      wait_return = WaitForMultipleObjects(num_handles, handleArray, 
        NO, wait_timeout);
    }
  else
    {
      NSDebugMLLog(@"NSRunLoop",
	@"wait for %d milliseconds", wait_timeout);
      SleepEx(wait_timeout, TRUE);
      wait_return = WAIT_TIMEOUT;
    }

  // check wait errors
  if (WAIT_FAILED == wait_return
    || (wait_return >= WAIT_ABANDONED_0 
      && wait_return < WAIT_ABANDONED_0 + num_handles))
    {
      int	i;
      BOOL	found = NO;

      NSDebugMLLog(@"NSRunLoop", @"WaitForMultipleObjects() error in "
	@"-pollUntil:within: %@", [NSError _last]);
      /*
       * Check each handle in turn until either we find one which has an
       * event signalled, or we find the one which caused the original
       * wait to fail ... so the callback routine for that handle can
       * deal with the problem.
       */
      for (i = 0; i < num_handles; i++)
	{
	  handleArray[0] = handleArray[i];
	  wait_return = WaitForMultipleObjects(1, handleArray, NO, 0);
	  if (wait_return != WAIT_TIMEOUT)
	    {
	      wait_return = WAIT_OBJECT_0;
	      found = YES;
	      break;
	    }
	}
      if (found == NO)
	{
	  NSLog(@"WaitForMultipleObjects() error in "
	    @"-pollUntil:within: %@", [NSError _last]);
	  abort ();        
	}
    }

  /*
   * Trigger any watchers which are set up to trigger for every runloop wait.
   */
  count = GSIArrayCount(_trigger);
  completed = NO;
  while (count-- > 0)
    {
      GSRunLoopWatcher	*watcher;

      watcher = (GSRunLoopWatcher*)GSIArrayItemAtIndex(_trigger, count).obj;
      if (watcher->_invalidated == NO)
	{
	  NSDebugMLLog(@"NSRunLoop", @"trigger watcher %@", watcher);
	  i = [contexts count];
	  while (i-- > 0)
	    {
	      GSRunLoopCtxt	*c = [contexts objectAtIndex: i];

	      if (c != self)
		{
		  [c endEvent: (void*)watcher for: watcher];
		}
	    }
	  /*
	   * The watcher is still valid - so call its
	   * receivers event handling method.
	   */
	  [watcher->receiver receivedEvent: watcher->data
				      type: watcher->type
				     extra: watcher->data
				   forMode: mode];
	}
      GSPrivateNotifyASAP(mode);
    }

  if (WAIT_TIMEOUT == wait_return)
    {
      // there is no event to handle
      if (existingMessages)
	{
	  NSDebugMLLog(@"NSRunLoop", @"processed windows messages");
	}
      else
	{
	  NSDebugMLLog(@"NSRunLoop", @"timeout without events");
	  completed = YES;
	  return NO;        
	}
    }
  else if (WAIT_OBJECT_0 + num_handles == wait_return)
    {
      // one or more windows message
      NSDebugMLLog(@"NSRunLoop", @"processing windows messages");
      [self processAllWindowsMessages: num_winMsgs within: contexts];
    }
  else if ((i = wait_return - WAIT_OBJECT_0) >= 0 && i < num_handles)
    {
      /* Look the event that WaitForMultipleObjects() says is ready;
       * get the corresponding fd for that handle event and notify
       * the corresponding object for the ready fd.
       */
      NSDebugMLLog(@"NSRunLoop", @"Handle signalled %d", i);
      
      handle = handleArray[i];

      if (handle == threadInfo->event)
	{
	  watcher = nil;
	  NSDebugMLLog(@"NSRunLoop", @"Fire perform on thread");
	  [threadInfo fire];
	}
      else
	{
	  watcher = (GSRunLoopWatcher*)NSMapGet(handleMap, (void*)handle);
	  NSDebugMLLog(@"NSRunLoop", @"Fire watcher %@", watcher);
	}
      if (watcher != nil && watcher->_invalidated == NO)
	{
	  i = [contexts count];
	  while (i-- > 0)
	    {
	      GSRunLoopCtxt *c = [contexts objectAtIndex: i];

	      if (c != self)
		{ 
		  [c endEvent: (void*)handle for: watcher];
		}
	    }
	  /*
	   * The watcher is still valid - so call its receivers
	   * event handling method.
	   */
	  [watcher->receiver receivedEvent: watcher->data
				      type: watcher->type
				     extra: (void*)handle
				   forMode: mode];
	}
    }
  else
    {
      NSDebugMLLog(@"NSRunLoop", @"unexpected result %d", wait_return);
      GSPrivateNotifyASAP(mode);
      completed = NO;
      return NO;        
    }

  GSPrivateNotifyASAP(mode);
  completed = YES;
  return YES;
}

+ (BOOL) awakenedBefore: (NSDate*)when
{
  GSRunLoopThreadInfo   *threadInfo = GSRunLoopInfoForThread(nil);
  NSTimeInterval	ti = (when == nil) ? 0.0 : [when timeIntervalSinceNow];
  int			milliseconds = (ti <= 0.0) ? 0 : (int)(ti*1000);
  HANDLE		h = threadInfo->event;

  if (WaitForMultipleObjects(1, &h, NO, milliseconds) != WAIT_TIMEOUT)
    {
      NSDebugMLLog(@"NSRunLoop", @"Fire perform on thread");
      [threadInfo fire];
      return YES;
    }
  return NO;
}

@end
