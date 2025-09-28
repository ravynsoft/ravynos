#!/usr/sbin/dtrace -s

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
 * Usage: dispatch_trace.d -p [pid]
 *        traced process must have been executed with
 *        DYLD_LIBRARY_PATH=/usr/lib/system/introspection or with
 *        DYLD_IMAGE_SUFFIX=_profile or DYLD_IMAGE_SUFFIX=_debug
 */

#pragma D option quiet
#pragma D option zdefs
#pragma D option bufsize=16m

BEGIN {
	printf("%-8s %-3s %-8s   %-35s%-15s%-?s   %-43s%-?s   %-14s%-?s    %s\n",
		"Time us", "CPU", "Thread", "Function", "Probe", "Queue", "Label",
		"Item", "Kind", "Context", "Symbol");
}

dispatch$target:libdispatch*.dylib::queue-push,
dispatch$target:libdispatch*.dylib::queue-pop,
dispatch$target:libdispatch*.dylib::callout-entry,
dispatch$target:libdispatch*.dylib::callout-return /!start/ {
	start = walltimestamp;
}

/*
 * Trace queue push and pop operations:
 *
 * probe queue-push/-pop(dispatch_queue_t queue, const char *label,
 *         dispatch_object_t item, const char *kind,
 *         dispatch_function_t function, void *context)
 */
dispatch$target:libdispatch*.dylib::queue-push,
dispatch$target:libdispatch*.dylib::queue-pop {
	printf("%-8d %-3d 0x%08p %-35s%-15s0x%0?p %-43s0x%0?p %-14s0x%0?p",
		(walltimestamp-start)/1000, cpu, tid, probefunc, probename, arg0,
		copyinstr(arg1, 42), arg2, copyinstr(arg3, 13), arg5);
	usym(arg4);
	printf("\n");
}

/*
 * Trace callouts to client functions:
 *
 * probe callout-entry/-return(dispatch_queue_t queue, const char *label,
 *         dispatch_function_t function, void *context)
 */
dispatch$target:libdispatch*.dylib::callout-entry,
dispatch$target:libdispatch*.dylib::callout-return {
	printf("%-8d %-3d 0x%08p %-35s%-15s0x%0?p %-43s%-?s   %-14s0x%0?p",
		(walltimestamp-start)/1000, cpu, tid, probefunc, probename, arg0,
		copyinstr(arg1, 42), "", "", arg3);
	usym(arg2);
	printf("\n");
}
