/*
 * Copyright (c) 2006-2018 Apple Computer, Inc. All rights reserved.
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

#ifndef SYS_MEMORYSTATUS_H
#define SYS_MEMORYSTATUS_H

#include <sys/time.h>
#include <mach_debug/zone_info.h>
#include <sys/proc.h>

#define MEMORYSTATUS_ENTITLEMENT "com.apple.private.memorystatus"

#define JETSAM_PRIORITY_REVISION                  2

#define JETSAM_PRIORITY_IDLE_HEAD                -2
/* The value -1 is an alias to JETSAM_PRIORITY_DEFAULT */
#define JETSAM_PRIORITY_IDLE                      0
#define JETSAM_PRIORITY_IDLE_DEFERRED             1 /* Keeping this around till all xnu_quick_tests can be moved away from it.*/
#define JETSAM_PRIORITY_AGING_BAND1               JETSAM_PRIORITY_IDLE_DEFERRED
#define JETSAM_PRIORITY_BACKGROUND_OPPORTUNISTIC  2
#define JETSAM_PRIORITY_AGING_BAND2               JETSAM_PRIORITY_BACKGROUND_OPPORTUNISTIC
#define JETSAM_PRIORITY_BACKGROUND                3
#define JETSAM_PRIORITY_ELEVATED_INACTIVE         JETSAM_PRIORITY_BACKGROUND
#define JETSAM_PRIORITY_MAIL                      4
#define JETSAM_PRIORITY_PHONE                     5
#define JETSAM_PRIORITY_UI_SUPPORT                8
#define JETSAM_PRIORITY_FOREGROUND_SUPPORT        9
#define JETSAM_PRIORITY_FOREGROUND               10
#define JETSAM_PRIORITY_AUDIO_AND_ACCESSORY      12
#define JETSAM_PRIORITY_CONDUCTOR                13
#define JETSAM_PRIORITY_DRIVER_APPLE             15
#define JETSAM_PRIORITY_HOME                     16
#define JETSAM_PRIORITY_EXECUTIVE                17
#define JETSAM_PRIORITY_IMPORTANT                18
#define JETSAM_PRIORITY_CRITICAL                 19

#define JETSAM_PRIORITY_MAX                      21

/* TODO - tune. This should probably be lower priority */
#define JETSAM_PRIORITY_DEFAULT                  18
#define JETSAM_PRIORITY_TELEPHONY                19

/* Compatibility */
#define DEFAULT_JETSAM_PRIORITY                  18

/*
 * The deferral time used by default for apps and daemons in all aging
 * policies except kJetsamAgingPolicySysProcsReclaimedFirst is
 * DEFERRED_IDLE_EXIT_TIME_SECS.
 *
 * For kJetsamAgingPolicySysProcsReclaimedFirst,
 *
 * Daemons: The actual idle deferred time for the daemon is based on
 * the relaunch behavior of the daemon. The relaunch behavior determines
 * the scaling factor applied to DEFERRED_IDLE_EXIT_TIME_SECS. See
 * kJetsamSysProcsIdleDelayTime* ratios defined in kern_memorystatus.c
 *
 * Apps: The apps are aged for DEFERRED_IDLE_EXIT_TIME_SECS factored
 * by kJetsamAppsIdleDelayTimeRatio.
 */
#define DEFERRED_IDLE_EXIT_TIME_SECS             10

#define KEV_MEMORYSTATUS_SUBCLASS                 3

enum {
	kMemorystatusLevelNote = 1,
	kMemorystatusSnapshotNote = 2,
	kMemorystatusFreezeNote = 3,
	kMemorystatusPressureNote = 4
};

enum {
	kMemorystatusLevelAny = -1,
	kMemorystatusLevelNormal = 0,
	kMemorystatusLevelWarning = 1,
	kMemorystatusLevelUrgent = 2,
	kMemorystatusLevelCritical = 3
};

typedef struct memorystatus_priority_entry {
	pid_t pid;
	int32_t priority;
	uint64_t user_data;
	int32_t limit;  /* MB */
	uint32_t state;
} memorystatus_priority_entry_t;

/*
 * This should be the structure to specify different properties
 * for processes (group or single) from user-space. Unfortunately,
 * we can't move to it completely because the priority_entry structure
 * above has been in use for a while now. We'll have to deprecate it.
 *
 * To support new fields/properties, we will add a new structure with a
 * new version and a new size.
 */
#define MEMORYSTATUS_MPE_VERSION_1              1

#define MEMORYSTATUS_MPE_VERSION_1_SIZE         sizeof(struct memorystatus_properties_entry_v1)

typedef struct memorystatus_properties_entry_v1 {
	int version;
	pid_t pid;
	int32_t priority;
	int use_probability;
	uint64_t user_data;
	int32_t limit;  /* MB */
	uint32_t state;
	char proc_name[MAXCOMLEN + 1];
	char __pad1[3];
} memorystatus_properties_entry_v1_t;

typedef struct memorystatus_kernel_stats {
	uint32_t free_pages;
	uint32_t active_pages;
	uint32_t inactive_pages;
	uint32_t throttled_pages;
	uint32_t purgeable_pages;
	uint32_t wired_pages;
	uint32_t speculative_pages;
	uint32_t filebacked_pages;
	uint32_t anonymous_pages;
	uint32_t compressor_pages;
	uint64_t compressions;
	uint64_t decompressions;
	uint64_t total_uncompressed_pages_in_compressor;
	uint64_t zone_map_size;
	uint64_t zone_map_capacity;
	uint64_t largest_zone_size;
	char     largest_zone_name[MACH_ZONE_NAME_MAX_LEN];
} memorystatus_kernel_stats_t;

/*
** This is a variable-length struct.
** Allocate a buffer of the size returned by the sysctl, cast to a memorystatus_snapshot_t *
*/

typedef struct jetsam_snapshot_entry {
	pid_t    pid;
	char     name[(2 * MAXCOMLEN) + 1];
	int32_t  priority;
	uint32_t state;
	uint32_t fds;
	uint8_t  uuid[16];
	uint64_t user_data;
	uint64_t killed;
	uint64_t pages;
	uint64_t max_pages_lifetime;
	uint64_t purgeable_pages;
	uint64_t jse_internal_pages;
	uint64_t jse_internal_compressed_pages;
	uint64_t jse_purgeable_nonvolatile_pages;
	uint64_t jse_purgeable_nonvolatile_compressed_pages;
	uint64_t jse_alternate_accounting_pages;
	uint64_t jse_alternate_accounting_compressed_pages;
	uint64_t jse_iokit_mapped_pages;
	uint64_t jse_page_table_pages;
	uint64_t jse_memory_region_count;
	uint64_t jse_gencount;                  /* memorystatus_thread generation counter */
	uint64_t jse_starttime;                 /* absolute time when process starts */
	uint64_t jse_killtime;                  /* absolute time when jetsam chooses to kill a process */
	uint64_t jse_idle_delta;                /* time spent in idle band */
	uint64_t jse_coalition_jetsam_id;       /* we only expose coalition id for COALITION_TYPE_JETSAM */
	struct timeval64 cpu_time;
	uint64_t jse_thaw_count;
} memorystatus_jetsam_snapshot_entry_t;

typedef struct jetsam_snapshot {
	uint64_t snapshot_time;                 /* absolute time snapshot was initialized */
	uint64_t notification_time;             /* absolute time snapshot was consumed */
	uint64_t js_gencount;                   /* memorystatus_thread generation counter */
	memorystatus_kernel_stats_t stats;      /* system stat when snapshot is initialized */
	size_t entry_count;
	memorystatus_jetsam_snapshot_entry_t entries[];
} memorystatus_jetsam_snapshot_t;

/* TODO - deprecate; see <rdar://problem/12969599> */
#define kMaxSnapshotEntries 192

/*
 * default jetsam snapshot support
 */
extern memorystatus_jetsam_snapshot_t *memorystatus_jetsam_snapshot;
extern memorystatus_jetsam_snapshot_t *memorystatus_jetsam_snapshot_copy;
extern unsigned int memorystatus_jetsam_snapshot_count;
extern unsigned int memorystatus_jetsam_snapshot_copy_count;
extern unsigned int memorystatus_jetsam_snapshot_max;
extern unsigned int memorystatus_jetsam_snapshot_size;
extern uint64_t memorystatus_jetsam_snapshot_last_timestamp;
extern uint64_t memorystatus_jetsam_snapshot_timeout;
#define memorystatus_jetsam_snapshot_list memorystatus_jetsam_snapshot->entries
#define JETSAM_SNAPSHOT_TIMEOUT_SECS 30

/* General memorystatus stuff */

extern uint64_t memorystatus_sysprocs_idle_delay_time;
extern uint64_t memorystatus_apps_idle_delay_time;

/* State */
#define kMemorystatusSuspended        0x01
#define kMemorystatusFrozen           0x02
#define kMemorystatusWasThawed        0x04
#define kMemorystatusTracked          0x08
#define kMemorystatusSupportsIdleExit 0x10
#define kMemorystatusDirty            0x20
#define kMemorystatusAssertion        0x40

/*
 * Jetsam exit reason definitions - related to memorystatus
 *
 * When adding new exit reasons also update:
 *	JETSAM_REASON_MEMORYSTATUS_MAX
 *	kMemorystatusKilled... Cause enum
 *	memorystatus_kill_cause_name[]
 */
#define JETSAM_REASON_INVALID                                                           0
#define JETSAM_REASON_GENERIC                                                           1
#define JETSAM_REASON_MEMORY_HIGHWATER                                          2
#define JETSAM_REASON_VNODE                                                                     3
#define JETSAM_REASON_MEMORY_VMPAGESHORTAGE                                     4
#define JETSAM_REASON_MEMORY_PROCTHRASHING                                      5
#define JETSAM_REASON_MEMORY_FCTHRASHING                                        6
#define JETSAM_REASON_MEMORY_PERPROCESSLIMIT                            7
#define JETSAM_REASON_MEMORY_DISK_SPACE_SHORTAGE                        8
#define JETSAM_REASON_MEMORY_IDLE_EXIT                                          9
#define JETSAM_REASON_ZONE_MAP_EXHAUSTION                                       10
#define JETSAM_REASON_MEMORY_VMCOMPRESSOR_THRASHING                     11
#define JETSAM_REASON_MEMORY_VMCOMPRESSOR_SPACE_SHORTAGE        12
#define JETSAM_REASON_LOWSWAP                                   13
#define JETSAM_REASON_MEMORYSTATUS_MAX  JETSAM_REASON_LOWSWAP

/*
 * Jetsam exit reason definitions - not related to memorystatus
 */
#define JETSAM_REASON_CPULIMIT                  100

/* Cause */
enum {
	kMemorystatusInvalid                                                    = JETSAM_REASON_INVALID,
	kMemorystatusKilled                                                             = JETSAM_REASON_GENERIC,
	kMemorystatusKilledHiwat                                                = JETSAM_REASON_MEMORY_HIGHWATER,
	kMemorystatusKilledVnodes                                               = JETSAM_REASON_VNODE,
	kMemorystatusKilledVMPageShortage                               = JETSAM_REASON_MEMORY_VMPAGESHORTAGE,
	kMemorystatusKilledProcThrashing                                = JETSAM_REASON_MEMORY_PROCTHRASHING,
	kMemorystatusKilledFCThrashing                                  = JETSAM_REASON_MEMORY_FCTHRASHING,
	kMemorystatusKilledPerProcessLimit                              = JETSAM_REASON_MEMORY_PERPROCESSLIMIT,
	kMemorystatusKilledDiskSpaceShortage                    = JETSAM_REASON_MEMORY_DISK_SPACE_SHORTAGE,
	kMemorystatusKilledIdleExit                                             = JETSAM_REASON_MEMORY_IDLE_EXIT,
	kMemorystatusKilledZoneMapExhaustion                    = JETSAM_REASON_ZONE_MAP_EXHAUSTION,
	kMemorystatusKilledVMCompressorThrashing                = JETSAM_REASON_MEMORY_VMCOMPRESSOR_THRASHING,
	kMemorystatusKilledVMCompressorSpaceShortage    = JETSAM_REASON_MEMORY_VMCOMPRESSOR_SPACE_SHORTAGE,
	kMemorystatusKilledLowSwap                      = JETSAM_REASON_LOWSWAP,
};

/*
 * For backwards compatibility
 * Keeping these around for external users (e.g. ReportCrash, Ariadne).
 * TODO: Remove once they stop using these.
 */
#define kMemorystatusKilledDiagnostic           kMemorystatusKilledDiskSpaceShortage
#define kMemorystatusKilledVMThrashing          kMemorystatusKilledVMCompressorThrashing
#define JETSAM_REASON_MEMORY_VMTHRASHING        JETSAM_REASON_MEMORY_VMCOMPRESSOR_THRASHING

/* Memorystatus control */
#define MEMORYSTATUS_BUFFERSIZE_MAX 65536

int memorystatus_get_level(user_addr_t level);
int memorystatus_control(uint32_t command, int32_t pid, uint32_t flags, void *buffer, size_t buffersize);

/* Commands */
#define MEMORYSTATUS_CMD_GET_PRIORITY_LIST            1
#define MEMORYSTATUS_CMD_SET_PRIORITY_PROPERTIES      2
#define MEMORYSTATUS_CMD_GET_JETSAM_SNAPSHOT          3
#define MEMORYSTATUS_CMD_GET_PRESSURE_STATUS          4
#define MEMORYSTATUS_CMD_SET_JETSAM_HIGH_WATER_MARK   5    /* Set active memory limit = inactive memory limit, both non-fatal	*/
#define MEMORYSTATUS_CMD_SET_JETSAM_TASK_LIMIT        6    /* Set active memory limit = inactive memory limit, both fatal	*/
#define MEMORYSTATUS_CMD_SET_MEMLIMIT_PROPERTIES      7    /* Set memory limits plus attributes independently			*/
#define MEMORYSTATUS_CMD_GET_MEMLIMIT_PROPERTIES      8    /* Get memory limits plus attributes					*/
#define MEMORYSTATUS_CMD_PRIVILEGED_LISTENER_ENABLE   9    /* Set the task's status as a privileged listener w.r.t memory notifications  */
#define MEMORYSTATUS_CMD_PRIVILEGED_LISTENER_DISABLE  10   /* Reset the task's status as a privileged listener w.r.t memory notifications  */
#define MEMORYSTATUS_CMD_AGGRESSIVE_JETSAM_LENIENT_MODE_ENABLE  11   /* Enable the 'lenient' mode for aggressive jetsam. See comments in kern_memorystatus.c near the top. */
#define MEMORYSTATUS_CMD_AGGRESSIVE_JETSAM_LENIENT_MODE_DISABLE 12   /* Disable the 'lenient' mode for aggressive jetsam. */
#define MEMORYSTATUS_CMD_GET_MEMLIMIT_EXCESS          13   /* Compute how much a process's phys_footprint exceeds inactive memory limit */
#define MEMORYSTATUS_CMD_ELEVATED_INACTIVEJETSAMPRIORITY_ENABLE         14 /* Set the inactive jetsam band for a process to JETSAM_PRIORITY_ELEVATED_INACTIVE */
#define MEMORYSTATUS_CMD_ELEVATED_INACTIVEJETSAMPRIORITY_DISABLE        15 /* Reset the inactive jetsam band for a process to the default band (0)*/
#define MEMORYSTATUS_CMD_SET_PROCESS_IS_MANAGED       16   /* (Re-)Set state on a process that marks it as (un-)managed by a system entity e.g. assertiond */
#define MEMORYSTATUS_CMD_GET_PROCESS_IS_MANAGED       17   /* Return the 'managed' status of a process */
#define MEMORYSTATUS_CMD_SET_PROCESS_IS_FREEZABLE     18   /* Is the process eligible for freezing? Apps and extensions can pass in FALSE to opt out of freezing, i.e.,
	                                                    *  if they would prefer being jetsam'ed in the idle band to being frozen in an elevated band. */
#define MEMORYSTATUS_CMD_GET_PROCESS_IS_FREEZABLE     19   /* Return the freezable state of a process. */

#if CONFIG_FREEZE
#if DEVELOPMENT || DEBUG
#define MEMORYSTATUS_CMD_FREEZER_CONTROL              20
#endif /* DEVELOPMENT || DEBUG */
#endif /* CONFIG_FREEZE */

#define MEMORYSTATUS_CMD_GET_AGGRESSIVE_JETSAM_LENIENT_MODE      21   /* Query if the lenient mode for aggressive jetsam is enabled. */

#define MEMORYSTATUS_CMD_INCREASE_JETSAM_TASK_LIMIT   22   /* Used by DYLD to increase the jetsam active and inactive limits, when using roots */

/* Commands that act on a group of processes */
#define MEMORYSTATUS_CMD_GRP_SET_PROPERTIES           100

/* Test commands */

/* Trigger forced jetsam */
#define MEMORYSTATUS_CMD_TEST_JETSAM            1000
#define MEMORYSTATUS_CMD_TEST_JETSAM_SORT       1001

/* Panic on jetsam options */
typedef struct memorystatus_jetsam_panic_options {
	uint32_t data;
	uint32_t mask;
} memorystatus_jetsam_panic_options_t;

#define MEMORYSTATUS_CMD_SET_JETSAM_PANIC_BITS        1002

/* Select priority band sort order */
#define JETSAM_SORT_NOSORT      0
#define JETSAM_SORT_DEFAULT     1


/* memorystatus_control() flags */

#define MEMORYSTATUS_FLAGS_SNAPSHOT_ON_DEMAND           0x1     /* A populated snapshot buffer is returned on demand */
#define MEMORYSTATUS_FLAGS_SNAPSHOT_AT_BOOT             0x2     /* Returns a snapshot with memstats collected at boot */
#define MEMORYSTATUS_FLAGS_SNAPSHOT_COPY                0x4     /* Returns the previously populated snapshot created by the system */
#define MEMORYSTATUS_FLAGS_GRP_SET_PRIORITY             0x8     /* Set jetsam priorities for a group of pids */
#define MEMORYSTATUS_FLAGS_GRP_SET_PROBABILITY          0x10    /* Set probability of use for a group of processes */

/*
 * For use with memorystatus_control:
 * MEMORYSTATUS_CMD_GET_JETSAM_SNAPSHOT
 *
 * A jetsam snapshot is initialized when a non-idle
 * jetsam event occurs.  The data is held in the
 * buffer until it is reaped. This is the default
 * behavior.
 *
 * Flags change the default behavior:
 *	Demand mode - this is an on_demand snapshot,
 *	meaning data is populated upon request.
 *
 *	Boot mode - this is a snapshot of
 *	memstats collected before loading the
 *	init program.  Once collected, these
 *	stats do not change.  In this mode,
 *	the snapshot entry_count is always 0.
 *
 *	Copy mode - this returns the previous snapshot
 *      collected by the system. The current snaphshot
 *	might be only half populated.
 *
 * Snapshots are inherently racey between request
 * for buffer size and actual data compilation.
 */

/* These definitions are required for backwards compatibility */
#define MEMORYSTATUS_SNAPSHOT_ON_DEMAND         MEMORYSTATUS_FLAGS_SNAPSHOT_ON_DEMAND
#define MEMORYSTATUS_SNAPSHOT_AT_BOOT           MEMORYSTATUS_FLAGS_SNAPSHOT_AT_BOOT
#define MEMORYSTATUS_SNAPSHOT_COPY              MEMORYSTATUS_FLAGS_SNAPSHOT_COPY

/*
 * For use with memorystatus_control:
 * MEMORYSTATUS_CMD_SET_PRIORITY_PROPERTIES
 */
typedef struct memorystatus_priority_properties {
	int32_t  priority;
	uint64_t user_data;
} memorystatus_priority_properties_t;

/*
 * Inform the kernel that setting the priority property is driven by assertions.
 */
#define MEMORYSTATUS_SET_PRIORITY_ASSERTION     0x1

/*
 * For use with memorystatus_control:
 * MEMORYSTATUS_CMD_SET_MEMLIMIT_PROPERTIES
 * MEMORYSTATUS_CMD_GET_MEMLIMIT_PROPERTIES
 */
typedef struct memorystatus_memlimit_properties {
	int32_t memlimit_active;                /* jetsam memory limit (in MB) when process is active */
	uint32_t memlimit_active_attr;
	int32_t memlimit_inactive;              /* jetsam memory limit (in MB) when process is inactive */
	uint32_t memlimit_inactive_attr;
} memorystatus_memlimit_properties_t;

typedef struct memorystatus_memlimit_properties2 {
	memorystatus_memlimit_properties_t v1;
	uint32_t memlimit_increase;             /* jetsam memory limit increase (in MB) for active and inactive states */
	uint32_t memlimit_increase_bytes;       /* bytes used to determine the jetsam memory limit increase, for active and inactive states */
} memorystatus_memlimit_properties2_t;

#define MEMORYSTATUS_MEMLIMIT_ATTR_FATAL        0x1     /* if set, exceeding the memlimit is fatal */


#endif /* SYS_MEMORYSTATUS_H */
