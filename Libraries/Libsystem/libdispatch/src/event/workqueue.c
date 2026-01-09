/*
 * Copyright (c) 2017-2017 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "internal.h"

#if DISPATCH_USE_INTERNAL_WORKQUEUE

/*
 * dispatch_workq monitors the thread pool that is
 * executing the work enqueued on libdispatch's pthread
 * root queues and dynamically adjusts its size.
 *
 * The dynamic monitoring could be implemented using either
 *   (a) low-frequency user-level approximation of the number of runnable
 *       worker threads via reading the /proc file system
 *   (b) a Linux kernel extension that hooks the process change handler
 *       to accurately track the number of runnable normal worker threads
 * This file provides an implementation of option (a).
 *
 * Using either form of monitoring, if (i) there appears to be
 * work available in the monitored pthread root queue, (ii) the
 * number of runnable workers is below the target size for the pool,
 * and (iii) the total number of worker threads is below an upper limit,
 * then an additional worker thread will be added to the pool.
 */

#pragma mark static data for monitoring subsystem

/*
 * State for the user-level monitoring of a workqueue.
 */
typedef struct dispatch_workq_monitor_s {
	/* The dispatch_queue we are monitoring */
	dispatch_queue_global_t dq;

	/* The observed number of runnable worker threads */
	int32_t num_runnable;

	/* The desired number of runnable worker threads */
	int32_t target_runnable;

	/*
	 * Tracking of registered workers; all accesses must hold lock.
	 * Invariant: registered_tids[0]...registered_tids[num_registered_tids-1]
	 *   contain the dispatch_tids of the worker threads we are monitoring.
	 */
	dispatch_unfair_lock_s registered_tid_lock;
	dispatch_tid *registered_tids;
	int num_registered_tids;
} dispatch_workq_monitor_s, *dispatch_workq_monitor_t;

#if HAVE_DISPATCH_WORKQ_MONITORING
static dispatch_workq_monitor_s _dispatch_workq_monitors[DISPATCH_QOS_NBUCKETS];
#endif

#pragma mark Implementation of the monitoring subsystem.

#define WORKQ_MAX_TRACKED_TIDS DISPATCH_WORKQ_MAX_PTHREAD_COUNT
#define WORKQ_OVERSUBSCRIBE_FACTOR 2

static void _dispatch_workq_init_once(void *context DISPATCH_UNUSED);
static dispatch_once_t _dispatch_workq_init_once_pred;

void
_dispatch_workq_worker_register(dispatch_queue_global_t root_q)
{
	dispatch_once_f(&_dispatch_workq_init_once_pred, NULL, &_dispatch_workq_init_once);

#if HAVE_DISPATCH_WORKQ_MONITORING
	dispatch_qos_t qos = _dispatch_priority_qos(root_q->dq_priority);
	if (qos == 0) qos = DISPATCH_QOS_DEFAULT;
	int bucket = DISPATCH_QOS_BUCKET(qos);
	dispatch_workq_monitor_t mon = &_dispatch_workq_monitors[bucket];
	dispatch_assert(mon->dq == root_q);
	dispatch_tid tid = _dispatch_tid_self();
	_dispatch_unfair_lock_lock(&mon->registered_tid_lock);
	dispatch_assert(mon->num_registered_tids < WORKQ_MAX_TRACKED_TIDS-1);
	int worker_id = mon->num_registered_tids++;
	mon->registered_tids[worker_id] = tid;
	_dispatch_unfair_lock_unlock(&mon->registered_tid_lock);
#else
	(void)root_q;
	(void)cls;
#endif // HAVE_DISPATCH_WORKQ_MONITORING
}

void
_dispatch_workq_worker_unregister(dispatch_queue_global_t root_q)
{
#if HAVE_DISPATCH_WORKQ_MONITORING
	dispatch_qos_t qos = _dispatch_priority_qos(root_q->dq_priority);
	if (qos == 0) qos = DISPATCH_QOS_DEFAULT;
	int bucket = DISPATCH_QOS_BUCKET(qos);
	dispatch_workq_monitor_t mon = &_dispatch_workq_monitors[bucket];
	dispatch_assert(mon->dq == root_q);
	dispatch_tid tid = _dispatch_tid_self();
	_dispatch_unfair_lock_lock(&mon->registered_tid_lock);
	for (int i = 0; i < mon->num_registered_tids; i++) {
		if (mon->registered_tids[i] == tid) {
			int last = mon->num_registered_tids - 1;
			mon->registered_tids[i] = mon->registered_tids[last];
			mon->registered_tids[last] = 0;
			mon->num_registered_tids--;
			break;
		}
	}
	_dispatch_unfair_lock_unlock(&mon->registered_tid_lock);
#else
	(void)root_q;
	(void)cls;
#endif // HAVE_DISPATCH_WORKQ_MONITORING
}


#if HAVE_DISPATCH_WORKQ_MONITORING
#if defined(__linux__)
/*
 * For each pid that is a registered worker, read /proc/[pid]/stat
 * to get a count of the number of them that are actually runnable.
 * See the proc(5) man page for the format of the contents of /proc/[pid]/stat
 */
static void
_dispatch_workq_count_runnable_workers(dispatch_workq_monitor_t mon)
{
	char path[128];
	char buf[4096];
	int running_count = 0;

	_dispatch_unfair_lock_lock(&mon->registered_tid_lock);

	for (int i = 0; i < mon->num_registered_tids; i++) {
		dispatch_tid tid = mon->registered_tids[i];
		int fd;
		ssize_t bytes_read = -1;

		int r = snprintf(path, sizeof(path), "/proc/%d/stat", tid);
		dispatch_assert(r > 0 && r < (int)sizeof(path));

		fd = open(path, O_RDONLY | O_NONBLOCK);
		if (unlikely(fd == -1)) {
			DISPATCH_CLIENT_CRASH(tid,
					"workq: registered worker exited prematurely");
		} else {
			bytes_read = read(fd, buf, sizeof(buf)-1);
			(void)close(fd);
		}

		if (bytes_read > 0) {
			buf[bytes_read] = '\0';
			char state;
			if (sscanf(buf, "%*d %*s %c", &state) == 1) {
				// _dispatch_debug("workq: Worker %d, state %c\n", tid, state);
				if (state == 'R') {
					running_count++;
				}
			} else {
				_dispatch_debug("workq: sscanf of state failed for %d", tid);
			}
		} else {
			_dispatch_debug("workq: Failed to read %s", path);
		}
	}

	mon->num_runnable = running_count;

	_dispatch_unfair_lock_unlock(&mon->registered_tid_lock);
}
#else
#error must define _dispatch_workq_count_runnable_workers
#endif

#define foreach_qos_bucket_reverse(name) \
		for (name = DISPATCH_QOS_BUCKET(DISPATCH_QOS_MAX); \
				name >= DISPATCH_QOS_BUCKET(DISPATCH_QOS_MAINTENANCE); name--)

static void
_dispatch_workq_monitor_pools(void *context DISPATCH_UNUSED)
{
	int global_soft_max = WORKQ_OVERSUBSCRIBE_FACTOR * (int)dispatch_hw_config(active_cpus);
	int global_runnable = 0, i;
	foreach_qos_bucket_reverse(i) {
		dispatch_workq_monitor_t mon = &_dispatch_workq_monitors[i];
		dispatch_queue_global_t dq = mon->dq;

		if (!_dispatch_queue_class_probe(dq)) {
			_dispatch_debug("workq: %s is empty.", dq->dq_label);
			continue;
		}

		_dispatch_workq_count_runnable_workers(mon);
		_dispatch_debug("workq: %s has %d runnable wokers (target is %d)",
				dq->dq_label, mon->num_runnable, mon->target_runnable);

		global_runnable += mon->num_runnable;

		if (mon->num_runnable == 0) {
			// We have work, but no worker is runnable.
			// It is likely the program is stalled. Therefore treat
			// this as if dq were an overcommit queue and call poke
			// with the limit being the maximum number of workers for dq.
			int32_t floor = mon->target_runnable - WORKQ_MAX_TRACKED_TIDS;
			_dispatch_debug("workq: %s has no runnable workers; poking with floor %d",
					dq->dq_label, floor);
			_dispatch_global_queue_poke(dq, 1, floor);
			global_runnable += 1; // account for poke in global estimate
		} else if (mon->num_runnable < mon->target_runnable &&
				   global_runnable < global_soft_max) {
			// We are below target, but some workers are still runnable.
			// We want to oversubscribe to hit the desired load target.
			// However, this under-utilization may be transitory so set the
			// floor as a small multiple of threads per core.
			int32_t floor = (1 - WORKQ_OVERSUBSCRIBE_FACTOR) * mon->target_runnable;
			int32_t floor2 = mon->target_runnable - WORKQ_MAX_TRACKED_TIDS;
			floor = MAX(floor, floor2);
			_dispatch_debug("workq: %s under utilization target; poking with floor %d",
					dq->dq_label, floor);
			_dispatch_global_queue_poke(dq, 1, floor);
			global_runnable += 1; // account for poke in global estimate
		}
	}
}
#endif // HAVE_DISPATCH_WORKQ_MONITORING

static void
_dispatch_workq_init_once(void *context DISPATCH_UNUSED)
{
#if HAVE_DISPATCH_WORKQ_MONITORING
	int i, target_runnable = (int)dispatch_hw_config(active_cpus);
	foreach_qos_bucket_reverse(i) {
		dispatch_workq_monitor_t mon = &_dispatch_workq_monitors[i];
		mon->dq = _dispatch_get_root_queue(i, false);
		void *buf = _dispatch_calloc(WORKQ_MAX_TRACKED_TIDS, sizeof(dispatch_tid));
		mon->registered_tids = buf;
		mon->target_runnable = target_runnable;
	}

	// Create monitoring timer that will periodically run on dispatch_mgr_q
	dispatch_source_t ds = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
			0, 0, &_dispatch_mgr_q);
	dispatch_source_set_timer(ds, dispatch_time(DISPATCH_TIME_NOW, 0),
			NSEC_PER_SEC, 0);
	dispatch_source_set_event_handler_f(ds, _dispatch_workq_monitor_pools);
	dispatch_set_context(ds, ds); // avoid appearing as leaked
	dispatch_activate(ds);
#endif // HAVE_DISPATCH_WORKQ_MONITORING
}

#endif // DISPATCH_USE_INTERNAL_WORKQUEUE
