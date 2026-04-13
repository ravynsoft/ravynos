//
// Copyright (c) 2019 Apple Inc. All rights reserved.
//
// @APPLE_LICENSE_HEADER_START@
//
// This file contains Original Code and/or Modifications of Original Code
// as defined in and that are subject to the Apple Public Source License
// Version 2.0 (the 'License'). You may not use this file except in
// compliance with the License. Please obtain a copy of the License at
// http://www.opensource.apple.com/apsl/ and read it before using this
// file.
//
// The Original Code and all software distributed under the License are
// distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
// INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
// Please see the License for the specific language governing rights and
// limitations under the License.
//
// @APPLE_LICENSE_HEADER_END@
//
// livefiles_cs_tester.c - Implements unit tests for livefiles
//                         Apple_CoreStorage plugin.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <UserFS/UserVFS.h>

extern UVFSFSOps cs_fsops;

//
// Enums describing file-system formats.
//
typedef enum {
	JHFS = 1,
	APFS,
	FAT32,
	EXFAT,
	APPLE_CS,

	INVALID_FS_TYPE = INT8_MAX,
} lf_cspt_fstype_t;

//
// Array describing file-system name and analogous types.
//
const struct {
	const char *const fs_name;
	lf_cspt_fstype_t  fs_type;
} lf_cspt_fstype_arr_t[] = {

	{"JHFS", JHFS},
	{"APFS", APFS},
	{"EXFAT",EXFAT},
	{"FAT32",FAT32},
	{"APPLE_CS", APPLE_CS},

	{NULL, INVALID_FS_TYPE}
};

//
// Validate file-system types that is supported by this program.
//
static bool
is_fstype_valid(const char *fs_name, lf_cspt_fstype_t *fs_type)
{
	int idx;

	for (idx = 0; lf_cspt_fstype_arr_t[idx].fs_name != NULL; idx++) {
		if (strcmp(fs_name, lf_cspt_fstype_arr_t[idx].fs_name) == 0) {
			*fs_type = lf_cspt_fstype_arr_t[idx].fs_type;
			return true;
		}
	}

	return false;
}

//
// Usage string returned to user.
//
static int
usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s filesystem-format device-path\n",
			prog_name);
	return EINVAL;
}

int
main(int argc, char *argv[])
{
	int fd, error;
	lf_cspt_fstype_t fs_type;

	if (argc != 3) {
		return usage(argv[0]);
	}

	if (!is_fstype_valid(argv[1], &fs_type)) {
		fprintf(stderr, "Unknown file-system type %s\n", argv[1]);
		return EINVAL;
	}

	fd = open(argv[2], O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open device [%s]: %d\n",
				argv[2], errno);
		return EBADF;
	}

	error = cs_fsops.fsops_init();
	printf("Init for fs_type %s returned [%d]\n", argv[1], error);
	if (error) {
		goto test_end;
	}

	error = cs_fsops.fsops_taste(fd);
	switch(fs_type) {
		case JHFS:
		case APFS:
		case EXFAT:
		case FAT32:

			//
			// Taste with these disk-image is expected to retrun
			// ENOTSUP. Thus supress the error here.
			//
			if (error == ENOTSUP) {
				error = 0;
			}
			break;

		case APPLE_CS:
			break;

		//
		// Control should not come here.
		//
		default:
			fprintf(stderr, "Bug in test.\n");
	}
	printf("Taste for fs_type %s returned [%d]\n", argv[1], error);
	cs_fsops.fsops_fini();

test_end:
	printf("Test result [%d]\n", error);
	close(fd);
	return EXIT_SUCCESS;
}
