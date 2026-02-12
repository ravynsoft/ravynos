/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS

void
od_passthru_set_node_availability(const char *nodename, bool available);

int32_t
od_passthru_register_node(const char *nodename, bool hidden);

void
od_passthru_unregister_node(const char *nodename);

bool
od_passthru_log_message(int32_t level, const char *message);

uid_t
od_passthru_get_uid(void);

uid_t
passthru_get_euid(void);

void
od_passthru_localonly_exit(void);

dispatch_source_t
od_passthru_create_source(mach_port_t port);

void
od_passthru_set_plugin_enabled(const char *plugin_name, bool enabled);

__END_DECLS
