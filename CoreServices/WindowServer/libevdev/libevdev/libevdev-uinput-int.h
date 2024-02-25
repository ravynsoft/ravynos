// SPDX-License-Identifier: MIT
/*
 * Copyright © 2013 Red Hat, Inc.
 */

struct libevdev_uinput {
	int fd; /**< file descriptor to uinput */
	int fd_is_managed; /**< do we need to close it? */
	char *name; /**< device name */
	char *syspath; /**< /sys path */
	char *devnode; /**< device node */
	time_t ctime[2]; /**< before/after UI_DEV_CREATE */
};
