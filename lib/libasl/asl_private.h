/*
 * Copyright (c) 2007-2011 Apple Inc. All rights reserved.
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

#ifndef __ASL_PRIVATE_H__
#define __ASL_PRIVATE_H__

#include <asl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <Availability.h>
#include <os/object.h>
#include <os/object_private.h>

#define streq(A, B) (strcmp(A, B) == 0)
#define strcaseeq(A, B) (strcasecmp(A, B) == 0)

#define ASL_QUERY_OP_NULL          0x00000

#define ASLMANAGER_SERVICE_NAME "com.apple.aslmanager"
#define NOTIFY_SYSTEM_MASTER "com.apple.system.syslog.master"
#define NOTIFY_SYSTEM_ASL_FILTER "com.apple.system.syslog.asl_filter"
#define NOTIFY_PREFIX_SYSTEM "com.apple.system.syslog"
#define NOTIFY_PREFIX_USER "user.syslog"
#define NOTIFY_RC "com.apple.asl.remote"

#define ASL_OPT_IGNORE "ignore"
#define ASL_OPT_STORE "store"
#define ASL_OPT_CONTROL "control"

/* File and Store Open Option */
#define ASL_OPT_OPEN_READ   0x80000000

#define ASL_STORE_LOCATION_FILE 0
#define ASL_STORE_LOCATION_MEMORY 1

#define ASL_OPT_SYSLOG_LEGACY  0x00010000

#define ASL_KEY_FREE_NOTE "ASLFreeNotify"

/*
 * Private types
 */
#define ASL_TYPE_STRING       6
#define ASL_TYPE_COUNT        7

/* SPI to enable ASL filter tunneling using asl_set_filter() */
#define ASL_FILTER_MASK_TUNNEL   0x100

#define NOQUOTA_FILE_PATH "/etc/asl/.noquota"

__BEGIN_DECLS

int asl_store_location() __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
char *asl_remote_notify_name() __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);
int asl_syslog_faciliy_name_to_num(const char *name) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
const char *asl_syslog_faciliy_num_to_name(int n) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
int asl_trigger_aslmanager(void) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);
int asl_get_filter(asl_object_t client, int *local, int *master, int *remote, int *active) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3);

/* EXCLUSIVLY FOR USE BY DEV TOOLS */
/* DO NOT USE THIS INTERFACE OTHERWISE */

uint32_t asl_store_match_timeout(void *ignored, void *query_v1, void **result_v1, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, uint32_t usec) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_7_0);

__END_DECLS


#endif /* __ASL_PRIVATE_H__ */
