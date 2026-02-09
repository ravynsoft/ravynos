/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _SYS_COALITION_H_
#define _SYS_COALITION_H_

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <mach/coalition.h>

__BEGIN_DECLS

/* Userspace syscall prototypes */

/* Syscalls */
int coalition_create(uint64_t *cid_out, uint32_t flags);
int coalition_terminate(uint64_t cid, uint32_t flags);
int coalition_reap(uint64_t cid, uint32_t flags);

/* Wrappers around __coalition_info syscall (with proper struct types) */
int coalition_info_resource_usage(uint64_t cid, struct coalition_resource_usage *cru, size_t sz);
int coalition_info_set_name(uint64_t cid, const char *name, size_t size);
int coalition_info_set_efficiency(uint64_t cid, uint64_t flags);
int coalition_ledger_set_logical_writes_limit(uint64_t cid, int64_t limit);


__END_DECLS

#endif /* _SYS_COALITION_H_ */
