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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_WORKQUEUE_INTERNAL__
#define __DISPATCH_WORKQUEUE_INTERNAL__

void _dispatch_workq_worker_register(dispatch_queue_global_t root_q);
void _dispatch_workq_worker_unregister(dispatch_queue_global_t root_q);

#if defined(__linux__)
#define HAVE_DISPATCH_WORKQ_MONITORING 1
#else
#define HAVE_DISPATCH_WORKQ_MONITORING 0
#endif

#endif /* __DISPATCH_WORKQUEUE_INTERNAL__ */

