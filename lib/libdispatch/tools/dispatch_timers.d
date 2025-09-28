#!/usr/sbin/dtrace -s

/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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
 * Usage: dispatch_timers.d -p [pid]
 *        traced process must have been executed with
 *        DYLD_LIBRARY_PATH=/usr/lib/system/introspection or with
 *        DYLD_IMAGE_SUFFIX=_profile or DYLD_IMAGE_SUFFIX=_debug
 */

#pragma D option quiet
#pragma D option zdefs

typedef struct dispatch_trace_timer_params_s {
	int64_t deadline, interval, leeway;
} *dispatch_trace_timer_params_t;

dispatch$target:libdispatch*.dylib::timer-configure,
dispatch$target:libdispatch*.dylib::timer-program,
dispatch$target:libdispatch*.dylib::timer-wake,
dispatch$target:libdispatch*.dylib::timer-fire /!start/ {
	start = walltimestamp;
}

/*
 * Trace dispatch timer configuration and programming:
 * Timer configuration indicates that dispatch_source_set_timer() was called.
 * Timer programming indicates that the dispatch manager is about to sleep
 * for 'deadline' ns (but may wake up earlier if non-timer events occur).
 * Time parameters are in nanoseconds, a value of -1 means "forever".
 *
 * probe timer-configure/-program(dispatch_source_t source,
 *         dispatch_function_t function, dispatch_trace_timer_params_t params)
 */
dispatch$target:libdispatch*.dylib::timer-configure,
dispatch$target:libdispatch*.dylib::timer-program {
	this->p = (dispatch_trace_timer_params_t)copyin(arg2,
			sizeof(struct dispatch_trace_timer_params_s));
	printf("%8dus %-15s: 0x%0?p deadline: %11dns interval: %11dns leeway: %11dns",
			(walltimestamp-start)/1000, probename, arg0,
			this->p ? this->p->deadline : 0, this->p ? this->p->interval : 0,
			this->p ? this->p->leeway : 0);
	usym(arg1);
	printf("\n");
}
dispatch$target:libdispatch*.dylib::timer-configure {
	printf("              / --- Begin ustack");
	ustack();
	printf("              \ --- End ustack\n");
}

/*
 * Trace dispatch timer wakes and fires:
 * Timer wakes indicate that the dispatch manager woke up due to expiry of the
 * deadline for the specified timer.
 * Timer fires indicate that that the dispatch manager scheduled the event
 * handler of the specified timer for asynchronous execution (may occur without
 * a corresponding timer wake if the manager was awake processing other events
 * when the timer deadline expired).
 *
 * probe timer-wake/-fire(dispatch_source_t source,
 *         dispatch_function_t function)
 */
dispatch$target:libdispatch*.dylib::timer-wake,
dispatch$target:libdispatch*.dylib::timer-fire {
	printf("%8dus %-15s: 0x%0?p%-70s", (walltimestamp-start)/1000, probename,
			arg0, "");
	usym(arg1);
	printf("\n");
}
