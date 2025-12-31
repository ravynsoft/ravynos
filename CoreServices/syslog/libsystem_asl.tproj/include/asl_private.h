/*
 * Copyright (c) 2007-2015 Apple Inc. All rights reserved.
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
#include <mach/vm_statistics.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <Availability.h>
#include <os/object.h>
#include <os/object_private.h>

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

#define ASL_OPT_SHIM_NO_ASL   0x10000000
#define ASL_OPT_SHIM_NO_TRACE 0x20000000

#define ASL_STORE_LOCATION_FILE 0
#define ASL_STORE_LOCATION_MEMORY 1

#define ASL_OPT_SYSLOG_LEGACY  0x00010000

#define ASL_KEY_FREE_NOTE "ASLFreeNotify"
#define ASL_KEY_MESSAGETRACER "com.apple.message.domain"
#define ASL_KEY_POWERMANAGEMENT "com.apple.iokit.domain"
#define ASL_KEY_CFLOG_LOCAL_TIME "CFLog Local Time"

#define FACILITY_LASTLOG "com.apple.system.lastlog"
#define FACILITY_UTMPX "com.apple.system.utmpx"

/* remote control bits */
#define EVAL_LEVEL_MASK   0x000000ff
#define EVAL_ACTION_MASK  0xffff0000
#define EVAL_ACTIVE       0x00010000
#define EVAL_SEND_ASL     0x00020000
#define EVAL_SEND_TRACE   0x00040000
#define EVAL_TEXT_FILE    0x00080000
#define EVAL_ASL_FILE     0x00100000
#define EVAL_TUNNEL       0x00200000
#define EVAL_QUOTA        0x00400000
#define EVAL_MT_SHIM      0x00800000

/*
 * Private types
 */
#define ASL_TYPE_STRING       6
#define ASL_TYPE_COUNT        7

/* SPI to enable ASL filter tunneling using asl_set_filter() */
#define ASL_FILTER_MASK_TUNNEL   0x100

#define NOQUOTA_FILE_PATH "/etc/asl/.noquota"

// TODO: this could move to vm_statistics.h
#ifndef VM_MEMORY_ASL
#define VM_MEMORY_ASL (VM_MEMORY_APPLICATION_SPECIFIC_1 + 7)
#endif

/*
 * Memory limits: work queue size for syslogd and max message size in libasl
 */
#if TARGET_OS_EMBEDDED
#define SYSLOGD_WORK_QUEUE_MEMORY 3072000
#define LIBASL_MAX_MSG_SIZE       2048000
#else
#define SYSLOGD_WORK_QUEUE_MEMORY 10240000
#define LIBASL_MAX_MSG_SIZE        8192000
#endif

__BEGIN_DECLS

int asl_store_location() __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
char *asl_remote_notify_name() __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
int asl_syslog_faciliy_name_to_num(const char *name) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
const char *asl_syslog_faciliy_num_to_name(int n) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.5,10.12), ios(2.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
int asl_trigger_aslmanager(void) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
int asl_get_filter(asl_object_t client, int *local, int *master, int *remote, int *active) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.7,10.12), ios(4.3,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_set_local_control(asl_object_t client, uint32_t filter) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.11,10.12), ios(9.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
uint32_t asl_get_local_control(asl_object_t client) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.11,10.12), ios(9.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

/* EXCLUSIVLY FOR USE BY DEV TOOLS */
/* DO NOT USE THIS INTERFACE OTHERWISE */

uint32_t asl_store_match_timeout(void *ignored, void *query_v1, void **result_v1, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, uint32_t usec) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.10,10.12), ios(7.0,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

__END_DECLS


#endif /* __ASL_PRIVATE_H__ */
