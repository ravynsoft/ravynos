#!/usr/sbin/dtrace -s

/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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
 * Usage: voucher_trace.d -p [pid]
 *        traced process must have been executed with
 *        DYLD_LIBRARY_PATH=/usr/lib/system/introspection or with
 *        DYLD_IMAGE_SUFFIX=_profile or DYLD_IMAGE_SUFFIX=_debug
 */

#pragma D option quiet
#pragma D option zdefs
#pragma D option bufsize=16m

BEGIN {
	printf("Starting to trace voucher operations...\n");
}

voucher$target:libdispatch*.dylib::create
{
	printf("ALLOC   voucher 0x%p, thread %#llx, ref 1, port %#x, aid %#llx", arg0, tid, arg1, arg2);
	ustack(10);
	printf("\n")
}

voucher$target:libdispatch*.dylib::dispose
{
	printf("FREE    voucher 0x%p, thread %#llx, ref 0", arg0, tid);
	ustack(10);
	printf("\n")
}

voucher$target:libdispatch*.dylib::retain
{
	printf("RETAIN  voucher 0x%p, thread %#llx, ref %d", arg0, tid, arg1);
	ustack(10);
	printf("\n")
}

voucher$target:libdispatch*.dylib::release
{
	printf("RELEASE voucher 0x%p, thread %#llx, ref %d", arg0, tid, arg1);
	ustack(10);
	printf("\n")
}

voucher$target:libdispatch*.dylib::adopt
{
	printf("ADOPT   voucher 0x%p, thread %#llx", arg0, tid);
	ustack(10);
	printf("\n")
}

voucher$target:libdispatch*.dylib::orphan
{
	printf("ORPHAN  voucher 0x%p, thread %#llx", arg0, tid);
	ustack(10);
	printf("\n")
}
