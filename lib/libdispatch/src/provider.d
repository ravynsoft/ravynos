/*
 * Copyright (c) 2010-2013 Apple Inc. All rights reserved.
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

/*
 * DTrace Probes for libdispatch
 *
 * Only available in the introspection version of the library,
 * loaded by running a process with the environment variable
 * DYLD_LIBRARY_PATH=/usr/lib/system/introspection
 */

typedef struct dispatch_object_s *dispatch_object_t;
typedef struct dispatch_queue_s *dispatch_queue_t;
typedef struct dispatch_source_s *dispatch_source_t;
typedef void (*dispatch_function_t)(void *);
typedef struct dispatch_trace_timer_params_s {
	int64_t deadline, interval, leeway;
} *dispatch_trace_timer_params_t;


provider dispatch {

/*
 * Probes for dispatch queue push and pop operations
 *
 * dispatch$target:libdispatch*.dylib::queue-push
 * dispatch$target:libdispatch*.dylib::queue-pop
 */
	probe queue__push(dispatch_queue_t queue, const char *label,
			dispatch_object_t item, const char *kind,
			dispatch_function_t function, void *context);
	probe queue__pop(dispatch_queue_t queue, const char *label,
			dispatch_object_t item, const char *kind,
			dispatch_function_t function, void *context);

/*
 * Probes for dispatch callouts to client functions
 *
 * dispatch$target:libdispatch*.dylib::callout-entry
 * dispatch$target:libdispatch*.dylib::callout-return
 */
	probe callout__entry(dispatch_queue_t queue, const char *label,
			dispatch_function_t function, void *context);
	probe callout__return(dispatch_queue_t queue, const char *label,
			dispatch_function_t function, void *context);

/*
 * Probes for dispatch timer configuration and programming
 *
 * Timer configuration indicates that dispatch_source_set_timer() was called.
 * Timer programming indicates that the dispatch manager is about to sleep
 * for 'deadline' ns (but may wake up earlier if non-timer events occur).
 * Time parameters are in nanoseconds, a value of -1 means "forever".
 *
 * dispatch$target:libdispatch*.dylib::timer-configure
 * dispatch$target:libdispatch*.dylib::timer-program
 */
	probe timer__configure(dispatch_source_t source,
			dispatch_function_t handler, dispatch_trace_timer_params_t params);
	probe timer__program(dispatch_source_t source, dispatch_function_t handler,
			dispatch_trace_timer_params_t params);

/*
 * Probes for dispatch timer wakes and fires
 *
 * Timer wakes indicate that the dispatch manager woke up due to expiry of the
 * deadline for the specified timer.
 * Timer fires indicate that that the dispatch manager scheduled the event
 * handler of the specified timer for asynchronous execution (may occur without
 * a corresponding timer wake if the manager was awake processing other events
 * when the timer deadline expired).
 *
 * dispatch$target:libdispatch*.dylib::timer-wake
 * dispatch$target:libdispatch*.dylib::timer-fire
 */
	probe timer__wake(dispatch_source_t source, dispatch_function_t handler);
	probe timer__fire(dispatch_source_t source, dispatch_function_t handler);

};


#pragma D attributes Evolving/Evolving/Common provider dispatch provider
#pragma D attributes Private/Private/Common provider dispatch module
#pragma D attributes Private/Private/Common provider dispatch function
#pragma D attributes Evolving/Evolving/Common provider dispatch name
#pragma D attributes Evolving/Evolving/Common provider dispatch args
