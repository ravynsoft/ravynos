/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#ifndef _SYS_PERSONA_H_
#define _SYS_PERSONA_H_

#include <sys/param.h>

enum {
	PERSONA_INVALID      = 0,
	PERSONA_GUEST        = 1,
	PERSONA_MANAGED      = 2,
	PERSONA_PRIV         = 3,
	PERSONA_SYSTEM       = 4,
	PERSONA_DEFAULT      = 5,
	PERSONA_SYSTEM_PROXY = 6,
	PERSONA_SYS_EXT      = 7,
	PERSONA_ENTERPRISE   = 8,

	PERSONA_TYPE_MAX     = PERSONA_ENTERPRISE,
};

#define PERSONA_ID_NONE ((uid_t)-1)

struct kpersona_info {
	uint32_t persona_info_version;

	uid_t    persona_id; /* overlaps with UID */
	int      persona_type;
	gid_t    persona_gid;
	uint32_t persona_ngroups;
	gid_t    persona_groups[NGROUPS];
	uid_t    persona_gmuid;
	char     persona_name[MAXLOGNAME + 1];

	/* TODO: MAC policies?! */
};

#define PERSONA_INFO_V1       1
#define PERSONA_INFO_V1_SIZE  (sizeof(struct kpersona_info))


#define PERSONA_OP_ALLOC    1
#define PERSONA_OP_PALLOC   2
#define PERSONA_OP_DEALLOC  3
#define PERSONA_OP_GET      4
#define PERSONA_OP_INFO     5
#define PERSONA_OP_PIDINFO  6
#define PERSONA_OP_FIND     7
#define PERSONA_OP_GETPATH  8
#define PERSONA_OP_FIND_BY_TYPE  9

#define PERSONA_MGMT_ENTITLEMENT "com.apple.private.persona-mgmt"

/*
 * user space persona interface
 */

/*
 * kpersona_alloc: Allocate a new in-kernel persona
 *
 * Parameters:
 *       info: Pointer to persona info structure describing the
 *             attributes of the persona to create / allocate.
 *
 *         id: output: set to the ID of the created persona
 *
 * Note:
 *      The 'persona_id' field of the 'info' parameter is ignored.
 *
 * Return:
 *        != 0: ERROR
 *        == 0: Success
 */
int kpersona_alloc(struct kpersona_info *info, uid_t *id);

/*
 * kpersona_palloc: Allocate a new in-kernel persona with a directory
 *                 pathname
 *
 * Parameters:
 *       info: Pointer to persona info structure describing the
 *             attributes of the persona to create / allocate.
 *
 *       path: Pointer to directory name that stores persona specific
 *             data. Assumes path buffer length = MAXPATHLEN and is a
 *             null-terminated string.
 *
 *         id: output: set to the ID of the created persona
 *
 * Note:
 *      The 'persona_id' field of the 'info' parameter is ignored.
 *
 * Return:
 *        != 0: ERROR
 *        == 0: Success
 */
int kpersona_palloc(struct kpersona_info *info, uid_t *id, char path[MAXPATHLEN]);

/*
 * kpersona_dealloc: delete / destroy an in-kernel persona
 *
 * Parameters:
 *         id: the ID of the persona to destroy
 *
 * Return:
 *        < 0: ERROR
 *          0: Success
 */
int kpersona_dealloc(uid_t id);

/*
 * kpersona_get: retrieve the persona with which the current thread is running
 *
 * To find the proc's persona id use kpersona_pidinfo
 *
 * Parameters:
 *         id: output: will be filled with the persona id from the voucher adopted
 *             on the current thread. If that voucher contains no persona information
 *             or there is no such voucher, then it defaults to the proc's persona id.
 *
 * Return:
 *        < 0: Thread is not running under any persona
 *          0: Success (uuid is filled with running persona UUID)
 */
int kpersona_get(uid_t *id);

/*
 * kpersona_get_path: retrieve the given persona's path
 *
 * Parameters:
 *         id: ID of the persona
 *
 *         path: output: filled in with path on success.
 *               Assumes path buffer length = MAXPATHLEN
 *
 * Return:
 *        < 0: Error
 *          0: Success
 */
int kpersona_getpath(uid_t id, char path[MAXPATHLEN]);

/*
 * kpersona_info: gather info about the given persona
 *
 * Parameters:
 *         id: ID of the persona to investigate
 *             If set to 0, it uses persona id from the voucher adopted on the current
 *             thread. If that voucher contains no persona information or there is no
 *             such voucher, then it defaults to the proc's persona id.
 *
 *       info: output: filled in with persona attributes on success.
 *
 * Return:
 *        < 0: ERROR
 *          0: Success
 */
int kpersona_info(uid_t id, struct kpersona_info *info);

/*
 * kpersona_pidinfo: gather persona info about the given PID
 *
 * Parameters:
 *        pid: PID of the process whose persona info we're to return
 *
 *       info: output: filled in with persona attributes on success.
 *
 * Return:
 *        < 0: ERROR
 *          0: Success
 */
int kpersona_pidinfo(pid_t pid, struct kpersona_info *info);

/*
 * kpersona_find: lookup the kernel's UUID of a persona
 *
 * Parameters:
 *       name: Local login name of the persona.
 *             Set this to NULL to find personas by 'uid'.
 *
 *        uid: UID of the persona.
 *             Set this to -1 to find personas by 'name'
 *
 *         id: output: the ID(s) matching the input parameters
 *             This can be NULL
 *
 *      idlen: input - size of 'id' buffer (in number of IDs)
 *             output - the total required size of the 'id' buffer
 *                      (in number of IDs) - may be larger than input size
 * Note:
 *      At least one of 'name' or 'uid' must be set.
 *
 * Return:
 *         < 0: ERROR
 *        >= 0: The number of IDs found to match the input parameters
 */
int kpersona_find(const char *name, uid_t uid, uid_t *id, size_t *idlen);

/*
 * kpersona_find_by_type: lookup the persona ids by type
 *
 * Parameters:
 *  persona_type: Type of persona id (see enum)
 *
 *           id: output: the ID(s) matching the input parameters
 *               This can be NULL
 *
 *        idlen: input - size of 'id' buffer (in number of IDs)
 *               output - the total required size of the 'id' buffer
 *                      (in number of IDs) - may be larger than input size
 * Return:
 *         < 0: ERROR
 *        >= 0: The number of IDs found to match the input parameters
 */
int kpersona_find_by_type(int persona_type, uid_t *id, size_t *idlen);


#endif /* _SYS_PERSONA_H_ */
