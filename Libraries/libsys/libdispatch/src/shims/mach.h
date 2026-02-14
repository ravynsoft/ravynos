/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_SHIMS_MACH__
#define __DISPATCH_SHIMS_MACH__

/*
 * Stub out defines for some mach types and related macros
 */

typedef uint32_t mach_port_t;

#define  MACH_PORT_NULL (0)
#define  MACH_PORT_DEAD (-1)

typedef uint32_t mach_error_t;

typedef uint32_t mach_msg_return_t;

typedef uint32_t mach_msg_bits_t;

typedef void *dispatch_mach_msg_t;

typedef uint64_t firehose_activity_id_t;

typedef void *mach_msg_header_t;

#endif
