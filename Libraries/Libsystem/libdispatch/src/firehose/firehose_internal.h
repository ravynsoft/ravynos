/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef __FIREHOSE_INTERNAL__
#define __FIREHOSE_INTERNAL__

#if OS_FIREHOSE_SPI

// make sure this is defined so that we get MIG_SERVER_DIED when a send once
// notification is sent back because of a crashed server
#ifndef __MigTypeCheck
#define __MigTypeCheck 1
#endif

#define fcp_quarntined fcp_quarantined

#include <limits.h>
#include <machine/endian.h>
#include <mach/mach_types.h>
#include <mach/std_types.h>
#include <os/object.h>
#include <firehose/private.h>
#include <mach/mach_types.h>
#include <mach/std_types.h>
#include <sys/qos.h>

#include "os/firehose_server_private.h"
#include "firehose_buffer_internal.h"
#ifdef FIREHOSE_SERVER
#include "firehose_server_internal.h"
#endif
#include "firehose_inline_internal.h"

#endif // OS_FIREHOSE_SPI

#endif // __FIREHOSE_INTERNAL__
