/** Implementation for NSNotificationQueue for GNUStep
   Copyright (C) 1995-1999 Free Software Foundation, Inc.

   Author: Mircea Oancea <mircea@jupiter.elcom.pub.ro>
   Date: 1995
   Modified by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1997
   Rewritten: 1999

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   <title>NSNotificationQueue class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#define	EXPOSE_NSNotificationQueue_IVARS	1
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSNotificationQueue.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSThread.h"

#import "GSPrivate.h"
/* NotificationQueueList by Richard Frith-Macdonald
   These objects are used to maintain lists of NSNotificationQueue objects.
   There is one list per NSThread, with the first object in the list stored
   in the thread dictionary and accessed using the key below.
   */

static	NSString*	lkey = @"NotificationQueueListThreadKey";
static	NSString*	qkey = @"NotificationQueueThreadKey";


@interface	NotificationQueueList : NSObject
{
@public
  NotificationQueueList	*next;
  NSNotificationQueue	*queue;
}
+ (void) registerQueue: (NSNotificationQueue*)q;
+ (void) unregisterQueue: (NSNotificationQueue*)q;
@end

static NotificationQueueList*
currentList(void)
{
  NotificationQueueList	*list;
  NSMutableDictionary	*d;

  d = GSCurrentThreadDictionary();
  list = (NotificationQueueList*)[d objectForKey: lkey];
  if (list == nil)
    {
      list = [NotificationQueueList new];
      [d setObject: list forKey: lkey];
      RELEASE(list);	/* retained in dictionary.	*/
    }
  return list;
}

@implementation	NotificationQueueList

- (void) dealloc
{
  while (next != nil)
    {
      NotificationQueueList	*tmp = next;

      next = tmp->next;
      RELEASE(tmp);
    }
  [super dealloc];
}

+ (void) registerQueue: (NSNotificationQueue*)q
{
  NotificationQueueList	*list;
  NotificationQueueList	*elem;

  list = currentList();	/* List of queues for thread.	*/

  if (list->queue == nil)
    {
      list->queue = q;		/* Make this the default.	*/
    }

  while (list->queue != q && list->next != nil)
    {
      list = list->next;
    }

  if (list->queue == q)
    {
      return;			/* Queue already registered.	*/
    }

  elem = (NotificationQueueList*)NSAllocateObject(self, 0,
    NSDefaultMallocZone());
  elem->queue = q;
  list->next = elem;
}

+ (void) unregisterQueue: (NSNotificationQueue*)q
{
  NotificationQueueList	*list;

  list = currentList();

  if (list->queue == q)
    {
      NSMutableDictionary	*d = GSCurrentThreadDictionary();
      NotificationQueueList	*tmp = list->next;

      if (tmp != nil)
        {
          [d setObject: tmp forKey: lkey];
	  RELEASE(tmp);			/* retained in dictionary.	*/
        }
      else
	{
	  [d removeObjectForKey: lkey];
	}
    }
  else
    {
      while (list->next != nil)
	{
	  if (list->next->queue == q)
	    {
	      NotificationQueueList	*tmp = list->next;

	      list->next = tmp->next;
	      RELEASE(tmp);
	      break;
	    }
	}
    }
}

@end

/*
 * NSNotificationQueue queue
 */

typedef struct _NSNotificationQueueRegistration
{
  struct _NSNotificationQueueRegistration	*next;
  struct _NSNotificationQueueRegistration	*prev;
  NSNotification				*notification;
  id						name;
  id						object;
  NSArray					*modes;
} NSNotificationQueueRegistration;

struct _NSNotificationQueueList;

typedef struct _NSNotificationQueueList
{
  struct _NSNotificationQueueRegistration	*head;
  struct _NSNotificationQueueRegistration	*tail;
} NSNotificationQueueList;

/*
 * Queue functions
 *
 *  Queue             Elem              Elem              Elem
 *    head ---------> next -----------> next -----------> next --> nil
 *            nil <-- prev <----------- prev <----------- prev
 *    tail --------------------------------------------->
 */

static inline void
remove_from_queue_no_release(NSNotificationQueueList *queue,
  NSNotificationQueueRegistration *item)
{
  if (item->next)
    {
      item->next->prev = item->prev;
    }
  else
    {
      NSCAssert(queue->tail == item, @"tail item not at tail of queue!");
      queue->tail = item->prev;
    }

  if (item->prev)
    {
      item->prev->next = item->next;
    }
  else
    {
      NSCAssert(queue->head == item, @"head item not at head of queue!");
      queue->head = item->next;
    }
}

static void
remove_from_queue(NSNotificationQueueList *queue,
  NSNotificationQueueRegistration *item, NSZone *_zone)
{
  remove_from_queue_no_release(queue, item);
  RELEASE(item->notification);
  RELEASE(item->modes);
  NSZoneFree(_zone, item);
}

static void
add_to_queue(NSNotificationQueueList *queue, NSNotification *notification,
  NSArray *modes, NSZone *_zone)
{
  NSNotificationQueueRegistration	*item;

  item = NSZoneCalloc(_zone, 1, sizeof(NSNotificationQueueRegistration));
  if (item == 0)
    {
      [NSException raise: NSMallocException
      		  format: @"Unable to add to notification queue"];
    }

  item->notification = RETAIN(notification);
  item->name = [notification name];
  item->object = [notification object];
  item->modes = [modes copyWithZone: [modes zone]];

  item->next = NULL;
  item->prev = queue->tail;
  queue->tail = item;
  if (item->prev)
    {
      item->prev->next = item;
    }
  if (!queue->head)
    {
      queue->head = item;
    }
}



/*
 * NSNotificationQueue class implementation
 */

@interface NSNotificationQueue (Private)
- (NSNotificationCenter*) _center;
@end

/**
 * This class supports asynchronous posting of [NSNotification]s to an
 * [NSNotificationCenter].  The method to add a notification to the queue
 * returns immediately.  The queue will periodically post its oldest
 * notification to the notification center.  In a multithreaded process,
 * notifications are always sent on the thread that they are posted from.
 */
@implementation NSNotificationQueue

static NSArray	*defaultMode = nil;

+ (void) initialize
{
  if (defaultMode == nil)
    {
      defaultMode = [[NSArray alloc] initWithObjects: (id*)&NSDefaultRunLoopMode
					       count: 1];
      [[NSObject leakAt: &defaultMode] release];
    }
}

/**
 * Returns the default notification queue for use in this thread.  It will
 * always post notifications to the default notification center (for the
 * entire task, which may have multiple threads and therefore multiple
 * notification queues).
 */
+ (NSNotificationQueue*) defaultQueue
{
  NotificationQueueList	*list;
  NSNotificationQueue	*item;

  list = currentList();
  item = list->queue;
  if (item == nil)
    {
      item = (NSNotificationQueue*)NSAllocateObject(self,
	0, NSDefaultMallocZone());
      item = [item initWithNotificationCenter:
	[NSNotificationCenter defaultCenter]];
      if (item != nil)
	{
	  NSMutableDictionary	*d;

	  d = GSCurrentThreadDictionary();
	  [d setObject: item forKey: qkey];
	  RELEASE(item);	/* retained in dictionary.	*/
	}
    }
  return item;
}

- (id) init
{
  return [self initWithNotificationCenter:
	  [NSNotificationCenter defaultCenter]];
}

/**
 *  Initialize a new instance to post notifications to the given
 *  notificationCenter (instead of the default).
 */
- (id) initWithNotificationCenter: (NSNotificationCenter*)notificationCenter
{
  _zone = [self zone];

  // init queue
  _center = RETAIN(notificationCenter);
  _asapQueue = NSZoneCalloc(_zone, 1, sizeof(NSNotificationQueueList));
  _idleQueue = NSZoneCalloc(_zone, 1, sizeof(NSNotificationQueueList));

  if (_asapQueue == 0 || _idleQueue == 0)
    {
      DESTROY(self);
    }
  else
    {
      /*
       * insert in global queue list
       */
      [NotificationQueueList registerQueue: self];
    }
  return self;
}

- (void) dealloc
{
  NSNotificationQueueRegistration	*item;

  /*
   * remove from class instances list
   */
  [NotificationQueueList unregisterQueue: self];

  /*
   * release items from our queues
   */
  while ((item = _asapQueue->head) != 0)
    {
      remove_from_queue(_asapQueue, item, _zone);
    }
  NSZoneFree(_zone, _asapQueue);

  while ((item = _idleQueue->head) != 0)
    {
      remove_from_queue(_idleQueue, item, _zone);
    }
  NSZoneFree(_zone, _idleQueue);

  RELEASE(_center);
  [super dealloc];
}

/* Inserting and Removing Notifications From a Queue */

/**
 * Immediately remove all notifications from queue matching notification on
 * name and/or object as specified by coalesce mask, which is an OR
 * ('<code>|</code>') of the options
 * <code>NSNotificationCoalescingOnName</code>,
 * <code>NSNotificationCoalescingOnSender</code> (object), and
 * <code>NSNotificationNoCoalescing</code> (match only the given instance
 * exactly).  If both of the first options are specified, notifications must
 * match on both attributes (not just either one).  Removed notifications are
 * <em>not</em> posted.
 */
- (void) dequeueNotificationsMatching: (NSNotification*)notification
			 coalesceMask: (NSUInteger)coalesceMask
{
  NSNotificationQueueRegistration	*item;
  NSNotificationQueueRegistration	*prev;
  id					name   = [notification name];
  id					object = [notification object];

  if ((coalesceMask & NSNotificationCoalescingOnName)
    && (coalesceMask & NSNotificationCoalescingOnSender))
    {
      /*
       * find in ASAP notification in queue matching both
       */
      for (item = _asapQueue->tail; item; item = prev)
	{
          prev = item->prev;
          //PENDING: should object comparison be '==' instead of isEqual?!
          if ((object == item->object) && [name isEqual: item->name])
	    {
              remove_from_queue(_asapQueue, item, _zone);
	    }
	}
      /*
       * find in idle notification in queue matching both
       */
      for (item = _idleQueue->tail; item; item = prev)
	{
          prev = item->prev;
          if ((object == item->object) && [name isEqual: item->name])
	    {
              remove_from_queue(_idleQueue, item, _zone);
	    }
	}
    }
  else if ((coalesceMask & NSNotificationCoalescingOnName))
    {
      /*
       * find in ASAP notification in queue matching name
       */
      for (item = _asapQueue->tail; item; item = prev)
	{
          prev = item->prev;
          if ([name isEqual: item->name])
	    {
              remove_from_queue(_asapQueue, item, _zone);
	    }
	}
      /*
       * find in idle notification in queue matching name
       */
      for (item = _idleQueue->tail; item; item = prev)
	{
          prev = item->prev;
          if ([name isEqual: item->name])
	    {
              remove_from_queue(_idleQueue, item, _zone);
	    }
	}
    }
  else if ((coalesceMask & NSNotificationCoalescingOnSender))
    {
      /*
       * find in ASAP notification in queue matching sender
       */
      for (item = _asapQueue->tail; item; item = prev)
	{
          prev = item->prev;
          if (object == item->object)
	    {
              remove_from_queue(_asapQueue, item, _zone);
	    }
	}
      /*
       * find in idle notification in queue matching sender
       */
      for (item = _idleQueue->tail; item; item = prev)
	{
          prev = item->prev;
          if (object == item->object)
	    {
              remove_from_queue(_idleQueue, item, _zone);
	    }
	}
    }
}

/**
 *  Sets notification to be posted to notification center at time dependent on
 *  postingStyle, which may be either <code>NSPostNow</code> (synchronous post),
 *  <code>NSPostASAP</code> (post soon), or <code>NSPostWhenIdle</code> (post
 *  when runloop is idle).
 */
- (void) enqueueNotification: (NSNotification*)notification
		postingStyle: (NSPostingStyle)postingStyle	
{
  [self enqueueNotification: notification
	       postingStyle: postingStyle
	       coalesceMask: NSNotificationCoalescingOnName
			      + NSNotificationCoalescingOnSender
		   forModes: nil];
}

/**
 *  Sets notification to be posted to notification center at time dependent on
 *  postingStyle, which may be either <code>NSPostNow</code> (synchronous
 *  post), <code>NSPostASAP</code> (post soon), or <code>NSPostWhenIdle</code>
 *  (post when runloop is idle).  coalesceMask determines whether this
 *  notification should be considered same as other ones already on the queue,
 *  in which case they are removed through a call to
 *  -dequeueNotificationsMatching:coalesceMask: .  The modes argument
 *  determines which [NSRunLoop] mode notification may be posted in (nil means
 *  NSDefaultRunLoopMode).
 */
- (void) enqueueNotification: (NSNotification*)notification
		postingStyle: (NSPostingStyle)postingStyle
		coalesceMask: (NSUInteger)coalesceMask
		    forModes: (NSArray*)modes
{
  if (modes == nil)
    {
      modes = defaultMode;
    }
  if (coalesceMask != NSNotificationNoCoalescing)
    {
      [self dequeueNotificationsMatching: notification
			    coalesceMask: coalesceMask];
    }
  switch (postingStyle)
    {
      case NSPostNow:
	{
	  NSString	*mode;

	  mode = [[NSRunLoop currentRunLoop] currentMode];
	  if (mode == nil || [modes indexOfObject: mode] != NSNotFound)
	    {
	      [_center postNotification: notification];
	    }
	}
	break;

      case NSPostASAP:
	add_to_queue(_asapQueue, notification, modes, _zone);
	break;

      case NSPostWhenIdle:
	add_to_queue(_idleQueue, notification, modes, _zone);
	break;
    }
}

@end

@implementation NSNotificationQueue (Private)

- (NSNotificationCenter*) _center
{
  return _center;
}

@end

static void
notify(NSNotificationCenter *center, NSNotificationQueueList *list,
  NSString *mode, NSZone *zone)
{
  BOOL					allocated = NO;
  void					*buf[100];
  void					**ptr = buf;
  unsigned				len = sizeof(buf) / sizeof(*buf);
  unsigned				pos = 0;
  NSNotificationQueueRegistration	*item = list->head;

  /* Gather matching items into a buffer.
   */
  while (item != 0)
    {
      if (mode == nil || [item->modes indexOfObject: mode] != NSNotFound)
	{
	  if (pos == len)
	    {
	      unsigned	want;

	      want = (len == 0) ? 2 : len * 2;
	      if (NO == allocated)
		{
		  void		*tmp;
		  
		  tmp = NSZoneMalloc(NSDefaultMallocZone(),
		    want * sizeof(void*));
		  memcpy(tmp, (void*)ptr, len * sizeof(void*));
		  ptr = tmp;
		  allocated = YES;
		}
	      else
		{
		  ptr = NSZoneRealloc(NSDefaultMallocZone(),
		    ptr, want * sizeof(void*));
		}
	      len = want;
	    }
	  ptr[pos++] = item;
	}
      item = item->next;	// head --> tail uses next link
    }
  len = pos;	// Number of items found

  /* Posting a notification catches exceptions, so it's OK to use
   * retain/release of objects here as we won't get an exception
   * causing a leak.
   */
  if (len > 0)
    {
      /* First, we make a note of each notification while removing the
       * corresponding list item from the queue ... so that when we get
       * round to posting the notifications we will not get problems
       * with another notif() trying to use the same items.
       */
      for (pos = 0; pos < len; pos++)
	{
	  item = ptr[pos];
	  ptr[pos] = RETAIN(item->notification);
	  remove_from_queue(list, item, zone);
	}

      /* Now that we no longer need to worry about r-entrancy,
       * we step through our notifications, posting each one in turn.
       */
      for (pos = 0; pos < len; pos++)
	{
	  NSNotification	*n = (NSNotification*)ptr[pos];

	  [center postNotification: n];
	  RELEASE(n);
	}

      if (allocated)
	{
	  NSZoneFree(NSDefaultMallocZone(), ptr);
	}
    }
}

/*
 *	The following code handles sending of queued notifications by
 *	NSRunLoop.
 */

void
GSPrivateNotifyASAP(NSString *mode)
{
  NotificationQueueList	*item;

  GSPrivateCheckTasks();

  for (item = currentList(); item; item = item->next)
    {
      if (item->queue)
	{
	  notify(item->queue->_center,
	    item->queue->_asapQueue,
	    mode,
	    item->queue->_zone);
	}
    }
}

void
GSPrivateNotifyIdle(NSString *mode)
{
  NotificationQueueList	*item;

  for (item = currentList(); item; item = item->next)
    {
      if (item->queue)
	{
	  notify(item->queue->_center,
	    item->queue->_idleQueue,
	    mode,
	    item->queue->_zone);
	}
    }
}

BOOL
GSPrivateNotifyMore(NSString *mode)
{
  NotificationQueueList	*item;

  for (item = currentList(); item; item = item->next)
    {
      if (item->queue != nil)
	{
          NSNotificationQueueRegistration	*r;

	  r = item->queue->_idleQueue->head;
	  while (r != 0)
	    {
	      if (mode == nil || [r->modes indexOfObject: mode] != NSNotFound)
		{
		  return YES;
		}
	      r = r->next;
	    }
	}
    }
  return NO;
}

