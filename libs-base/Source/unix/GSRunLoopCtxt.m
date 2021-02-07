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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_POLL_F
#include <poll.h>
#endif

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
  if (_efdMap != 0)
    {
      NSFreeMapTable(_efdMap);
    }
  if (_rfdMap != 0)
    {
      NSFreeMapTable(_rfdMap);
    }
  if (_wfdMap != 0)
    {
      NSFreeMapTable(_wfdMap);
    }
  GSIArrayEmpty(_trigger);
  NSZoneFree(_trigger->zone, (void*)_trigger);
#ifdef	HAVE_POLL_F
  if (pollfds != 0)
    {
      NSZoneFree(NSDefaultMallocZone(), pollfds);
    }
#endif
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
	      return;
	    }
	}

      switch (watcher->type)
	{
	  case ET_RPORT: 
	  case ET_RDESC: 
	    NSMapRemove(_rfdMap, data);
	    break;
	  case ET_WDESC: 
	    NSMapRemove(_wfdMap, data);
	    break;
	  case ET_EDESC: 
	    NSMapRemove(_efdMap, data);
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

      _efdMap = NSCreateMapTable (NSIntegerMapKeyCallBacks,
				      WatcherMapValueCallBacks, 0);
      _rfdMap = NSCreateMapTable (NSIntegerMapKeyCallBacks,
				      WatcherMapValueCallBacks, 0);
      _wfdMap = NSCreateMapTable (NSIntegerMapKeyCallBacks,
				      WatcherMapValueCallBacks, 0);
    }
  return self;
}

#ifdef	HAVE_POLL_F

static void setPollfd(int fd, int event, GSRunLoopCtxt *ctxt)
{
  int		index;
  struct pollfd *pollfds = ctxt->pollfds;
  pollextra	*pe = (pollextra*)ctxt->extra;

  if (fd >= pe->limit)
    {
      int oldfd_limit = pe->limit;

      pe->limit = fd + 1;
      if (pe->index == 0)
	{
	  pe->index = NSZoneMalloc(NSDefaultMallocZone(),
	    pe->limit * sizeof(*(pe->index)));
	}
      else
	{
	  pe->index = NSZoneRealloc(NSDefaultMallocZone(),
	    pe->index, pe->limit * sizeof(*(pe->index)));
	}
      do
	{
	  pe->index[oldfd_limit++] = -1;
	}
      while (oldfd_limit < pe->limit);
    }
  index = pe->index[fd];
  if (index == -1)
    {
      if (ctxt->pollfds_count >= ctxt->pollfds_capacity)
	{
	  ctxt->pollfds_capacity += 8;
	  pollfds = NSZoneRealloc(NSDefaultMallocZone(),
	    pollfds, ctxt->pollfds_capacity * sizeof (*pollfds));
	  ctxt->pollfds = pollfds;
	}
      index = ctxt->pollfds_count++;
      pe->index[fd] = index;
      pollfds[index].fd = fd;
      pollfds[index].events = 0;
      pollfds[index].revents = 0;
    }
  pollfds[index].events |= event;
}

/**
 * Perform a poll for the specified runloop context.
 * If the method has been called re-entrantly, the contexts stack
 * will list all the contexts with polls in progress
 * and this method must tell those outer contexts not to handle events
 * which are handled by this context.
 */
- (BOOL) pollUntil: (int)milliseconds within: (NSArray*)contexts
{
  GSRunLoopThreadInfo   *threadInfo = GSRunLoopInfoForThread(nil);
  int		poll_return;
  int		fdEnd;	/* Number of descriptors being monitored. */
  int		fdIndex;
  int		fdFinish;
  unsigned	count;
  unsigned int	i;
  BOOL		immediate = NO;

  i = GSIArrayCount(watchers);

  /*
   * Get ready to listen to file descriptors.
   * The maps will not have been emptied by any previous call.
   */
  NSResetMapTable(_efdMap);
  NSResetMapTable(_rfdMap);
  NSResetMapTable(_wfdMap);
  GSIArrayRemoveAllItems(_trigger);

  /*
   * Do the pre-listening set-up for the file descriptors of this mode.
   */
  if (pollfds_capacity < i + 2)
    {
      pollfds_capacity = i + 2;
      if (pollfds == 0)
	{
	  pollfds = NSZoneMalloc(NSDefaultMallocZone(),
	    pollfds_capacity * sizeof(*pollfds));
	}
      else
	{
	  pollfds = NSZoneRealloc(NSDefaultMallocZone(),
	    pollfds, pollfds_capacity * sizeof(*pollfds));
	}
    }
  pollfds_count = 0;
  ((pollextra*)extra)->limit = 0;

  /* Watch for signals from other threads.
   */
  setPollfd(threadInfo->inputFd, POLLIN, self);

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
	  int	fd;

	  switch (info->type)
	    {
	      case ET_EDESC: 
		fd = (int)(intptr_t)info->data;
		setPollfd(fd, POLLPRI, self);
		NSMapInsert(_efdMap, (void*)(intptr_t)fd, info);
		break;

	      case ET_RDESC: 
		fd = (int)(intptr_t)info->data;
		setPollfd(fd, POLLIN, self);
		NSMapInsert(_rfdMap, (void*)(intptr_t)fd, info);
		break;

	      case ET_WDESC: 
		fd = (int)(intptr_t)info->data;
		setPollfd(fd, POLLOUT, self);
		NSMapInsert(_wfdMap, (void*)(intptr_t)fd, info);
		break;

	      case ET_TRIGGER:
		break;

	      case ET_RPORT: 
		{
		  id port = info->receiver;
                  NSInteger port_fd_size = FDCOUNT;
		  NSInteger port_fd_count = FDCOUNT;
		  NSInteger port_fd_buffer[FDCOUNT];
		  NSInteger *port_fd_array = port_fd_buffer;

		  [port getFds: port_fd_array count: &port_fd_count];
                  while (port_fd_count > port_fd_size)
                    {
                      if (port_fd_array != port_fd_buffer) free(port_fd_array);
                      port_fd_size = port_fd_count;
                      port_fd_count = port_fd_size;
                      port_fd_array = malloc(sizeof(NSInteger)*port_fd_size);
                      [port getFds: port_fd_array count: &port_fd_count];
                    }
		  NSDebugMLLog(@"NSRunLoop",
		    @"listening to %"PRIdPTR" port handles\n", port_fd_count);
		  while (port_fd_count--)
		    {
		      fd = port_fd_array[port_fd_count];
		      setPollfd(fd, POLLIN, self);
		      NSMapInsert(_rfdMap, (void*)(intptr_t)fd, info);
		    }
                  if (port_fd_array != port_fd_buffer) free(port_fd_array);
		}
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
      milliseconds = 0;
    }

#if 0
{
  unsigned int i;
  fprintf(stderr, "poll %d %d:", milliseconds, pollfds_count);
  for (i = 0; i < pollfds_count; i++)
    fprintf(stderr, " %d,%x", pollfds[i].fd, pollfds[i].events);
  fprintf(stderr, "\n");
}
#endif
  poll_return = poll (pollfds, pollfds_count, milliseconds);
#if 0
{
  unsigned int i;
  fprintf(stderr, "ret %d %d:", poll_return, pollfds_count);
  for (i = 0; i < pollfds_count; i++)
    fprintf(stderr, " %d,%x", pollfds[i].fd, pollfds[i].revents);
  fprintf(stderr, "\n");
}
#endif

  NSDebugMLLog(@"NSRunLoop", @"poll returned %d\n", poll_return);

  if (poll_return < 0)
    {
      if (errno == EINTR)
	{
	  GSPrivateCheckTasks();
	  poll_return = 0;
	}
      else if (errno == 0)
	{
	  /* Some systems returns an errno == 0. Not sure why */
	  poll_return = 0;
	}
      else
	{
	  /* Some exceptional condition happened. */
	  /* xxx We can do something with exception_fds, instead of
	     aborting here. */
	  NSLog (@"poll() error in -acceptInputForMode:beforeDate: '%@'",
	    [NSError _last]);
	  abort ();
	}
    }

  /*
   * Trigger any watchers which are set up to for every runloop wait.
   */
  count =  GSIArrayCount(_trigger);
  while (count-- > 0)
    {
      GSRunLoopWatcher	*watcher;

      watcher = (GSRunLoopWatcher*)GSIArrayItemAtIndex(_trigger, count).obj;
      if (watcher->_invalidated == NO)
	{
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

  /*
   * If the poll returned no descriptors with events, we have no more to do.
   */
  if (poll_return == 0)
    {
      completed = YES;
      return NO;
    }

  /*
   * Look at all the file descriptors poll() says are ready for action;
   * notify the corresponding object for each of the ready fd's.
   * NB. It is possible for a watcher to be missing from the map - if
   * the event handler of a previous watcher has 'run' the loop again
   * before returning.
   * NB. Each time this loop is entered, the starting position (fairStart)
   * is incremented - this is to ensure a fair distribution over all
   * inputs where multiple inputs are in use.  Note - fairStart can be
   * modified while we are in the loop (by recursive calls).
   */
  fdEnd = pollfds_count;
  if (++fairStart >= fdEnd)
    {
      fairStart = 0;
      fdIndex = 0;
      fdFinish = 0;
    }
  else
    {
      fdIndex = fairStart;
      fdFinish = fairStart;
    }
  completed = NO;
  while (completed == NO)
    {
      if (pollfds[fdIndex].revents != 0)
	{
	  int			fd = pollfds[fdIndex].fd;
	  GSRunLoopWatcher	*watcher;
	  BOOL			found = NO;
	  
	  /*
	   * The poll() call supports various error conditions - all
	   * errors should be handled by any available handler.
	   * The ET_EDSEC handler is the primary handler for exceptions
	   * though it is more generally used to deal with out-of-band data.
	   */
	  if (pollfds[fdIndex].revents & (POLLPRI|POLLERR|POLLHUP|POLLNVAL))
	    {
	      watcher
		= (GSRunLoopWatcher*)NSMapGet(_efdMap, (void*)(intptr_t)fd);
	      if (watcher != nil && watcher->_invalidated == NO)
		{
		  i = [contexts count];
		  while (i-- > 0)
		    {
		      GSRunLoopCtxt	*c = [contexts objectAtIndex: i];

		      if (c != self)
			{
			  [c endEvent: (void*)(intptr_t)fd for: watcher];
			}
		    }
		  /*
		   * The watcher is still valid - so call its
		   * receivers event handling method.
		   */
		  [watcher->receiver receivedEvent: watcher->data
					      type: watcher->type
					     extra: (void*)(uintptr_t)fd
					   forMode: mode];
		}
	      GSPrivateNotifyASAP(mode);
	      if (completed == YES)
		{
		  break;	// A nested poll has done the job.
		}
	      found = YES;
	    }
	  if (pollfds[fdIndex].revents & (POLLOUT|POLLERR|POLLHUP|POLLNVAL))
	    {
	      watcher
		= (GSRunLoopWatcher*)NSMapGet(_wfdMap, (void*)(intptr_t)fd);
	      if (watcher != nil && watcher->_invalidated == NO)
		{
		  i = [contexts count];
		  while (i-- > 0)
		    {
		      GSRunLoopCtxt	*c = [contexts objectAtIndex: i];

		      if (c != self)
			{
			  [c endEvent: (void*)(intptr_t)fd for: watcher];
			}
		    }
		  /*
		   * The watcher is still valid - so call its
		   * receivers event handling method.
		   */
		  [watcher->receiver receivedEvent: watcher->data
					      type: watcher->type
					     extra: (void*)(uintptr_t)fd
					   forMode: mode];
		}
	      GSPrivateNotifyASAP(mode);
	      if (completed == YES)
		{
		  break;	// A nested poll has done the job.
		}
	      found = YES;
	    }
	  if (pollfds[fdIndex].revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL))
	    {
              if (fd == threadInfo->inputFd)
                {
	          NSDebugMLLog(@"NSRunLoop", @"Fire perform on thread");
                  [threadInfo fire];
                  watcher = nil;
                }
              else
                {
                  watcher = (GSRunLoopWatcher*)
                    NSMapGet(_rfdMap, (void*)(intptr_t)fd);
                }
	      if (watcher != nil && watcher->_invalidated == NO)
		{
		  i = [contexts count];
		  while (i-- > 0)
		    {
		      GSRunLoopCtxt	*c = [contexts objectAtIndex: i];

		      if (c != self)
			{
			  [c endEvent: (void*)(intptr_t)fd for: watcher];
			}
		    }
		  /*
		   * The watcher is still valid - so call its
		   * receivers event handling method.
		   */
		  [watcher->receiver receivedEvent: watcher->data
					      type: watcher->type
					     extra: (void*)(uintptr_t)fd
					   forMode: mode];
		}
	      GSPrivateNotifyASAP(mode);
	      if (completed == YES)
		{
		  break;	// A nested poll has done the job.
		}
	      found = YES;
	    }
	  if (found == YES && --poll_return == 0)
	    {
	      completed = YES;
	    }  
	}
      if (++fdIndex >= fdEnd)
	{
	  fdIndex = 0;
	}
      if (fdIndex == fdFinish)
	{
	  completed = YES;
	}
    }
  completed = YES;
  return YES;
}

+ (BOOL) awakenedBefore: (NSDate*)when
{
  GSRunLoopThreadInfo   *threadInfo = GSRunLoopInfoForThread(nil);
  NSTimeInterval	ti = (when == nil) ? 0.0 : [when timeIntervalSinceNow];
  int			milliseconds = (ti <= 0.0) ? 0 : (int)(ti*1000);
  struct pollfd		pollfds;

  /* Watch for signals from other threads.
   */
  pollfds.fd = threadInfo->inputFd;
  pollfds.events = POLLIN;
  pollfds.revents = 0;
  if (poll(&pollfds, 1, milliseconds) == 1)
    {
      NSDebugMLLog(@"NSRunLoop", @"Fire perform on thread");
      [threadInfo fire];
      return YES;
    }
  return NO;
}

#else

- (BOOL) pollUntil: (int)milliseconds within: (NSArray*)contexts
{
  GSRunLoopThreadInfo   *threadInfo = GSRunLoopInfoForThread(nil);
  struct timeval	timeout;
  void			*select_timeout;
  int			select_return;
  int			fdIndex;
  int			fdFinish;
  fd_set 		read_fds;	// Mask for read-ready fds.
  fd_set 		exception_fds;	// Mask for exception fds.
  fd_set 		write_fds;	// Mask for write-ready fds.
  int			fd;
  int			fdEnd = -1;
  unsigned		count;
  unsigned		i;
  BOOL			immediate = NO;

  i = GSIArrayCount(watchers);

  /* Find out how much time we should wait, and set SELECT_TIMEOUT. */
  if (milliseconds == 0)
    {
      /* Don't wait at all. */
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      select_timeout = &timeout;
    }
  else if (milliseconds > 0)
    {
      timeout.tv_sec = milliseconds/1000;
      timeout.tv_usec = (milliseconds - 1000 * timeout.tv_sec) * 1000;
      select_timeout = &timeout;
    }
  else 
    {
      timeout.tv_sec = -1;
      timeout.tv_usec = -1;
      select_timeout = NULL;
    }

  /*
   * Get ready to listen to file descriptors.
   * Initialize the set of FDS we'll pass to select(), and make sure we
   * have empty maps for keeping track of which watcher is associated
   * with which file descriptor.
   * The maps may not have been emptied if a previous call to this
   * method was terminated by an exception.
   */
  memset(&exception_fds, '\0', sizeof(exception_fds));
  memset(&read_fds, '\0', sizeof(read_fds));
  memset(&write_fds, '\0', sizeof(write_fds));
  NSResetMapTable(_efdMap);
  NSResetMapTable(_rfdMap);
  NSResetMapTable(_wfdMap);
  GSIArrayRemoveAllItems(_trigger);

  /* Watch for signals from other threads.
   */
  fd = threadInfo->inputFd;
  if (fd > fdEnd)
    fdEnd = fd;
  FD_SET (fd, &read_fds);

  while (i-- > 0)
    {
      GSRunLoopWatcher	*info;
      int		fd;
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
	  switch (info->type)
	    {
	      case ET_EDESC: 
		fd = (int)(intptr_t)info->data;
		if (fd > fdEnd)
		  fdEnd = fd;
		FD_SET (fd, &exception_fds);
		NSMapInsert(_efdMap, (void*)(intptr_t)fd, info);
		break;

	      case ET_RDESC: 
		fd = (int)(intptr_t)info->data;
		if (fd > fdEnd)
		  fdEnd = fd;
		FD_SET (fd, &read_fds);
		NSMapInsert(_rfdMap, (void*)(intptr_t)fd, info);
		break;

	      case ET_WDESC: 
		fd = (int)(intptr_t)info->data;
		if (fd > fdEnd)
		  fdEnd = fd;
		FD_SET (fd, &write_fds);
		NSMapInsert(_wfdMap, (void*)(intptr_t)fd, info);
		break;

	      case ET_RPORT: 
		{
		  id port = info->receiver;
                  NSInteger port_fd_size = FDCOUNT;
		  NSInteger port_fd_count = FDCOUNT;
		  NSInteger port_fd_buffer[FDCOUNT];
		  NSInteger *port_fd_array = port_fd_buffer;

		  [port getFds: port_fd_array count: &port_fd_count];
                  while (port_fd_count > port_fd_size)
                    {
                      if (port_fd_array != port_fd_buffer) free(port_fd_array);
                      port_fd_size = port_fd_count;
                      port_fd_count = port_fd_size;
                      port_fd_array = malloc(sizeof(NSInteger)*port_fd_size);
                      [port getFds: port_fd_array count: &port_fd_count];
                    }
		  NSDebugMLLog(@"NSRunLoop", @"listening to %d port sockets",
		    port_fd_count);
		  while (port_fd_count--)
		    {
		      fd = port_fd_array[port_fd_count];
		      if (fd > fdEnd)
			fdEnd = fd;
		      FD_SET (fd, &read_fds);
		      NSMapInsert(_rfdMap, (void*)(intptr_t)fd, info);
		    }
                  if (port_fd_array != port_fd_buffer) free(port_fd_array);
		}
		break;

	      case ET_TRIGGER:
		break;

	    }
	}
    }
  fdEnd++;

  /*
   * If there are notifications in the 'idle' queue, we try an
   * instantaneous select so that, if there is no input pending,
   * we can service the queue.  Similarly, if a task has completed,
   * we need to deliver its notifications.
   */
  if (GSPrivateCheckTasks() || GSPrivateNotifyMore(mode) || immediate == YES)
    {
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      select_timeout = &timeout;
    }

  // NSDebugMLLog(@"NSRunLoop", @"select timeout %d,%d", timeout.tv_sec, timeout.tv_usec);

  select_return = select (fdEnd, &read_fds, &write_fds,
    &exception_fds, select_timeout);

  NSDebugMLLog(@"NSRunLoop", @"select returned %d", select_return);

  if (select_return < 0)
    {
      if (errno == EINTR)
	{
	  GSPrivateCheckTasks();
	  select_return = 0;
	}
      else if (errno == 0)
	{
	  /* Some systems return an errno == 0. Not sure why */
	  select_return = 0;
	}
      else
	{
	  /* Some exceptional condition happened. */
	  /* xxx We can do something with exception_fds, instead of
	     aborting here. */
	  NSLog (@"select() error in -acceptInputForMode:beforeDate: '%@'",
	    [NSError _last]);
	  abort ();
	}
    }

  /*
   * Trigger any watchers which are set up to for every runloop wait.
   */
  count = GSIArrayCount(_trigger);
  while (count-- > 0)
    {
      GSRunLoopWatcher	*watcher;

      watcher = (GSRunLoopWatcher*)GSIArrayItemAtIndex(_trigger, count).obj;
      if (watcher->_invalidated == NO)
	{
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

  /*
   * If the select returned no descriptors with events, we have no more to do.
   */
  if (select_return == 0)
    {
      completed = YES;
      return NO;
    }

  /*
   * Look at all the file descriptors select() says are ready for action;
   * notify the corresponding object for each of the ready fd's.
   * NB. Each time this loop is entered, the starting position (fairStart)
   * is incremented - this is to ensure a fair distribution over all
   * inputs where multiple inputs are in use.  Note - fairStart can be
   * modified while we are in the loop (by recursive calls).
   */
  if (++fairStart >= fdEnd)
    {
      fairStart = 0;
      fdIndex = 0;
      fdFinish = 0;
    }
  else
    {
      fdIndex = fairStart;
      fdFinish = fairStart;
    }
  completed = NO;
  while (completed == NO)
    {
      BOOL	found = NO;

      if (FD_ISSET (fdIndex, &exception_fds))
	{
	  GSRunLoopWatcher	*watcher;

	  watcher
	    = (GSRunLoopWatcher*)NSMapGet(_efdMap, (void*)(intptr_t)fdIndex);
	  if (watcher != nil && watcher->_invalidated == NO)
	    {
	      i = [contexts count];
	      while (i-- > 0)
		{
		  GSRunLoopCtxt	*c = [contexts objectAtIndex: i];

		  if (c != self)
		    [c endEvent: (void*)(intptr_t)fdIndex for: watcher];
		}
	      /*
	       * The watcher is still valid - so call its receivers
	       * event handling method.
	       */
	      [watcher->receiver receivedEvent: watcher->data
					  type: watcher->type
					 extra: (void*)(intptr_t)fdIndex
				       forMode: mode];
	    }
	  GSPrivateNotifyASAP(mode);
	  if (completed == YES)
	    {
	      break;
	    }
	  found = YES;
	}
      if (FD_ISSET (fdIndex, &write_fds))
	{
	  GSRunLoopWatcher	*watcher;

	  watcher
	    = (GSRunLoopWatcher*)NSMapGet(_wfdMap, (void*)(intptr_t)fdIndex);
	  if (watcher != nil && watcher->_invalidated == NO)
	    {
	      i = [contexts count];
	      while (i-- > 0)
		{
		  GSRunLoopCtxt	*c = [contexts objectAtIndex: i];

		  if (c != self)
		    [c endEvent: (void*)(intptr_t)fdIndex for: watcher];
		}
	      /*
	       * The watcher is still valid - so call its receivers
	       * event handling method.
	       */
	      [watcher->receiver receivedEvent: watcher->data
					  type: watcher->type
					 extra: (void*)(intptr_t)fdIndex
				       forMode: mode];
	    }
	  GSPrivateNotifyASAP(mode);
	  if (completed == YES)
	    {
	      break;
	    }
	  found = YES;
	}
      if (FD_ISSET (fdIndex, &read_fds))
	{
	  GSRunLoopWatcher	*watcher;

          if (fdIndex == threadInfo->inputFd)
            {
	      NSDebugMLLog(@"NSRunLoop", @"Fire perform on thread");
              [threadInfo fire];
              watcher = nil;
            }
          else
            {
              watcher = (GSRunLoopWatcher*)
                NSMapGet(_rfdMap, (void*)(intptr_t)fdIndex);
            }
	  if (watcher != nil && watcher->_invalidated == NO)
	    {
	      i = [contexts count];
	      while (i-- > 0)
		{
		  GSRunLoopCtxt	*c = [contexts objectAtIndex: i];

		  if (c != self)
		    [c endEvent: (void*)(intptr_t)fdIndex for: watcher];
		}
	      /*
	       * The watcher is still valid - so call its receivers
	       * event handling method.
	       */
	      [watcher->receiver receivedEvent: watcher->data
					  type: watcher->type
					 extra: (void*)(intptr_t)fdIndex
				       forMode: mode];
	    }
	  GSPrivateNotifyASAP(mode);
	  if (completed == YES)
	    {
	      break;
	    }
	  found = YES;
	}
      if (found == YES && --select_return == 0)
	{
	  completed = YES;
	}
      if (++fdIndex >= fdEnd)
	{
	  fdIndex = 0;
	}
      if (fdIndex == fdFinish)
	{
	  completed = YES;
	}
    }
  completed = YES;
  return YES;
}

+ (BOOL) awakenedBefore: (NSDate*)when
{
  GSRunLoopThreadInfo   *threadInfo = GSRunLoopInfoForThread(nil);
  NSTimeInterval	ti = (when == nil) ? 0.0 : [when timeIntervalSinceNow];
  int			milliseconds = (ti <= 0.0) ? 0 : (int)(ti*1000);
  struct timeval	timeout;
  fd_set 		read_fds;	// Mask for read-ready fds.
  fd_set 		exception_fds;	// Mask for exception fds.
  fd_set 		write_fds;	// Mask for write-ready fds.

  memset(&exception_fds, '\0', sizeof(exception_fds));
  memset(&read_fds, '\0', sizeof(read_fds));
  memset(&write_fds, '\0', sizeof(write_fds));
  timeout.tv_sec = milliseconds/1000;
  timeout.tv_usec = (milliseconds - 1000 * timeout.tv_sec) * 1000;
  FD_SET (threadInfo->inputFd, &read_fds);
  if (select (threadInfo->inputFd, &read_fds, &write_fds,
    &exception_fds, &timeout) > 0)
    {
      NSDebugMLLog(@"NSRunLoop", @"Fire perform on thread");
      [threadInfo fire];
      return YES;
    }
  return NO;
}

#endif
@end
