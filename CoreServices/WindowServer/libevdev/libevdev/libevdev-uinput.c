// SPDX-License-Identifier: MIT
/*
 * Copyright © 2013 Red Hat, Inc.
 */

#include "config.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "libevdev-int.h"
#include "libevdev-uinput-int.h"
#include "libevdev-uinput.h"
#include "libevdev-util.h"
#include "libevdev.h"

#ifndef UINPUT_IOCTL_BASE
#define UINPUT_IOCTL_BASE       'U'
#endif

#ifndef UI_SET_PROPBIT
#define UI_SET_PROPBIT _IOW(UINPUT_IOCTL_BASE, 110, int)
#endif

static struct libevdev_uinput *
alloc_uinput_device(const char *name)
{
	struct libevdev_uinput *uinput_dev;

	uinput_dev = calloc(1, sizeof(struct libevdev_uinput));
	if (uinput_dev) {
		uinput_dev->name = strdup(name);
		uinput_dev->fd = -1;
	}

	return uinput_dev;
}

static inline int
set_abs(const struct libevdev *dev, int fd, unsigned int code)
{
	const struct input_absinfo *abs = libevdev_get_abs_info(dev, code);
	struct uinput_abs_setup abs_setup = {0};
	int rc;

	abs_setup.code = code;
	abs_setup.absinfo = *abs;
	rc = ioctl(fd, UI_ABS_SETUP, &abs_setup);
	return rc;
}

static int
set_evbits(const struct libevdev *dev, int fd, struct uinput_user_dev *uidev)
{
	int rc = 0;
	unsigned int type;

	for (type = 0; type < EV_CNT; type++) {
		unsigned int code;
		int max;
		int uinput_bit;
		const unsigned long *mask;

		if (!libevdev_has_event_type(dev, type))
			continue;

		rc = ioctl(fd, UI_SET_EVBIT, type);
		if (rc == -1)
			break;

		/* uinput can't set EV_REP */
		if (type == EV_REP)
			continue;

		max = type_to_mask_const(dev, type, &mask);
		if (max == -1)
			continue;

		switch(type) {
			case EV_KEY: uinput_bit = UI_SET_KEYBIT; break;
			case EV_REL: uinput_bit = UI_SET_RELBIT; break;
			case EV_ABS: uinput_bit = UI_SET_ABSBIT; break;
			case EV_MSC: uinput_bit = UI_SET_MSCBIT; break;
			case EV_LED: uinput_bit = UI_SET_LEDBIT; break;
			case EV_SND: uinput_bit = UI_SET_SNDBIT; break;
			case EV_FF: uinput_bit = UI_SET_FFBIT; break;
			case EV_SW: uinput_bit = UI_SET_SWBIT; break;
			default:
				    rc = -1;
				    errno = EINVAL;
				    goto out;
		}

		for (code = 0; code <= (unsigned int)max; code++) {
			if (!libevdev_has_event_code(dev, type, code))
				continue;

			rc = ioctl(fd, uinput_bit, code);
			if (rc == -1)
				goto out;

			if (type == EV_ABS) {
				if (uidev == NULL) {
					rc = set_abs(dev, fd, code);
					if (rc != 0)
						goto out;
				} else {
					const struct input_absinfo *abs =
						libevdev_get_abs_info(dev, code);

					uidev->absmin[code] = abs->minimum;
					uidev->absmax[code] = abs->maximum;
					uidev->absfuzz[code] = abs->fuzz;
					uidev->absflat[code] = abs->flat;
					/* uinput has no resolution in the
					 * device struct */
				}
			}
		}

	}

out:
	return rc;
}

static int
set_props(const struct libevdev *dev, int fd)
{
	unsigned int prop;
	int rc = 0;

	for (prop = 0; prop <= INPUT_PROP_MAX; prop++) {
		if (!libevdev_has_property(dev, prop))
			continue;

		rc = ioctl(fd, UI_SET_PROPBIT, prop);
		if (rc == -1) {
			/* If UI_SET_PROPBIT is not supported, treat -EINVAL
			 * as success. The kernel only sends -EINVAL for an
			 * invalid ioctl, invalid INPUT_PROP_MAX or if the
			 * ioctl is called on an already created device. The
			 * last two can't happen here.
			 */
			if (errno == EINVAL)
				rc = 0;
			break;
		}
	}
	return rc;
}

LIBEVDEV_EXPORT int
libevdev_uinput_get_fd(const struct libevdev_uinput *uinput_dev)
{
	return uinput_dev->fd;
}

#ifdef __FreeBSD__
/*
 * FreeBSD does not have anything similar to sysfs.
 * Set libevdev_uinput->syspath to NULL unconditionally.
 * Look up the device nodes directly instead of via sysfs, as this matches what
 * is returned by the UI_GET_SYSNAME ioctl() on FreeBSD.
 */
static int
fetch_syspath_and_devnode(struct libevdev_uinput *uinput_dev)
{
#define DEV_INPUT_DIR "/dev/input/"
	int rc;
	char buf[sizeof(DEV_INPUT_DIR) + 64] = DEV_INPUT_DIR;

	rc = ioctl(uinput_dev->fd,
	           UI_GET_SYSNAME(sizeof(buf) - strlen(DEV_INPUT_DIR)),
		   &buf[strlen(DEV_INPUT_DIR)]);
	if (rc == -1)
		return -1;

	uinput_dev->syspath = NULL;
	uinput_dev->devnode = strdup(buf);

	return 0;
#undef DEV_INPUT_DIR
}

#else /* !__FreeBSD__ */

static int is_event_device(const struct dirent *dent) {
	return strncmp("event", dent->d_name, 5) == 0;
}

static char *
fetch_device_node(const char *path)
{
	char *devnode = NULL;
	struct dirent **namelist;
	int ndev, i;

	ndev = scandir(path, &namelist, is_event_device, alphasort);
	if (ndev <= 0)
		return NULL;

	/* ndev should only ever be 1 */

	for (i = 0; i < ndev; i++) {
		if (!devnode && asprintf(&devnode, "/dev/input/%s", namelist[i]->d_name) == -1)
			devnode = NULL;
		free(namelist[i]);
	}

	free(namelist);

	return devnode;
}

static int is_input_device(const struct dirent *dent) {
	return strncmp("input", dent->d_name, 5) == 0;
}

static int
fetch_syspath_and_devnode(struct libevdev_uinput *uinput_dev)
{
#define SYS_INPUT_DIR "/sys/devices/virtual/input/"
	struct dirent **namelist;
	int ndev, i;
	int rc;
	char buf[sizeof(SYS_INPUT_DIR) + 64] = SYS_INPUT_DIR;

	rc = ioctl(uinput_dev->fd,
		   UI_GET_SYSNAME(sizeof(buf) - strlen(SYS_INPUT_DIR)),
		   &buf[strlen(SYS_INPUT_DIR)]);
	if (rc != -1) {
		uinput_dev->syspath = strdup(buf);
		uinput_dev->devnode = fetch_device_node(buf);
		return 0;
	}

	ndev = scandir(SYS_INPUT_DIR, &namelist, is_input_device, alphasort);
	if (ndev <= 0)
		return -1;

	for (i = 0; i < ndev; i++) {
		int fd, len;
		struct stat st;

		rc = snprintf(buf, sizeof(buf), "%s%s/name",
			      SYS_INPUT_DIR,
			      namelist[i]->d_name);
		if (rc < 0 || (size_t)rc >= sizeof(buf)) {
			continue;
		}

		/* created within time frame */
		fd = open(buf, O_RDONLY);
		if (fd < 0)
			continue;

		/* created before UI_DEV_CREATE, or after it finished */
		if (fstat(fd, &st) == -1 ||
		    st.st_ctime < uinput_dev->ctime[0] ||
		    st.st_ctime > uinput_dev->ctime[1]) {
			close(fd);
			continue;
		}

		len = read(fd, buf, sizeof(buf));
		close(fd);
		if (len <= 0)
			continue;

		buf[len - 1] = '\0'; /* file contains \n */
		if (strcmp(buf, uinput_dev->name) == 0) {
			if (uinput_dev->syspath) {
				/* FIXME: could descend into bit comparison here */
				log_info(NULL, "multiple identical devices found. syspath is unreliable\n");
				break;
			}

			rc = snprintf(buf, sizeof(buf), "%s%s",
				      SYS_INPUT_DIR,
				      namelist[i]->d_name);

			if (rc < 0 || (size_t)rc >= sizeof(buf)) {
				log_error(NULL, "Invalid syspath, syspath is unreliable\n");
				break;
			}

			uinput_dev->syspath = strdup(buf);
			uinput_dev->devnode = fetch_device_node(buf);
		}
	}

	for (i = 0; i < ndev; i++)
		free(namelist[i]);
	free(namelist);

	return uinput_dev->devnode ? 0 : -1;
#undef SYS_INPUT_DIR
}
#endif /* __FreeBSD__*/

static int
uinput_create_write(const struct libevdev *dev, int fd)
{
	int rc;
	struct uinput_user_dev uidev;

	memset(&uidev, 0, sizeof(uidev));

	strncpy(uidev.name, libevdev_get_name(dev), UINPUT_MAX_NAME_SIZE - 1);
	uidev.id.vendor = libevdev_get_id_vendor(dev);
	uidev.id.product = libevdev_get_id_product(dev);
	uidev.id.bustype = libevdev_get_id_bustype(dev);
	uidev.id.version = libevdev_get_id_version(dev);

	if (set_evbits(dev, fd, &uidev) != 0)
		goto error;
	if (set_props(dev, fd) != 0)
		goto error;

	rc = write(fd, &uidev, sizeof(uidev));
	if (rc < 0) {
		goto error;
	} else if ((size_t)rc < sizeof(uidev)) {
		errno = EINVAL;
		goto error;
	}

	errno = 0;

error:
	return -errno;
}

static int
uinput_create_DEV_SETUP(const struct libevdev *dev, int fd,
			struct libevdev_uinput *new_device)
{
	int rc;
	struct uinput_setup setup;

	if (set_evbits(dev, fd, NULL) != 0)
		goto error;
	if (set_props(dev, fd) != 0)
		goto error;

	memset(&setup, 0, sizeof(setup));
	strncpy(setup.name, libevdev_get_name(dev), UINPUT_MAX_NAME_SIZE - 1);
	setup.id.vendor = libevdev_get_id_vendor(dev);
	setup.id.product = libevdev_get_id_product(dev);
	setup.id.bustype = libevdev_get_id_bustype(dev);
	setup.id.version = libevdev_get_id_version(dev);
	setup.ff_effects_max = libevdev_has_event_type(dev, EV_FF) ? 10 : 0;

	rc = ioctl(fd, UI_DEV_SETUP, &setup);
	if (rc == 0)
		errno = 0;
error:
	return -errno;
}

LIBEVDEV_EXPORT int
libevdev_uinput_create_from_device(const struct libevdev *dev, int fd, struct libevdev_uinput** uinput_dev)
{
	int rc;
	struct libevdev_uinput *new_device;
	int close_fd_on_error = (fd == LIBEVDEV_UINPUT_OPEN_MANAGED);
	unsigned int uinput_version = 0;

	new_device = alloc_uinput_device(libevdev_get_name(dev));
	if (!new_device)
		return -ENOMEM;

	if (fd == LIBEVDEV_UINPUT_OPEN_MANAGED) {
		fd = open("/dev/uinput", O_RDWR|O_CLOEXEC);
		if (fd < 0)
			goto error;

		new_device->fd_is_managed = 1;
	} else if (fd < 0) {
		log_bug(NULL, "Invalid fd %d\n", fd);
		errno = EBADF;
		goto error;
	}

	if (ioctl(fd, UI_GET_VERSION, &uinput_version) == 0 &&
	    uinput_version >= 5)
		rc = uinput_create_DEV_SETUP(dev, fd, new_device);
	else
		rc = uinput_create_write(dev, fd);

	if (rc != 0)
		goto error;

	/* ctime notes time before/after ioctl to help us filter out devices
	   when traversing /sys/devices/virtual/input to find the device
	   node.

	   this is in seconds, so ctime[0]/[1] will almost always be
	   identical but /sys doesn't give us sub-second ctime so...
	 */
	new_device->ctime[0] = time(NULL);

	rc = ioctl(fd, UI_DEV_CREATE, NULL);
	if (rc == -1)
		goto error;

	new_device->ctime[1] = time(NULL);
	new_device->fd = fd;

	if (fetch_syspath_and_devnode(new_device) == -1) {
		log_error(NULL, "unable to fetch syspath or device node.\n");
		errno = ENODEV;
		goto error;
	}

	*uinput_dev = new_device;

	return 0;

error:
	rc = -errno;
	libevdev_uinput_destroy(new_device);
	if (fd != -1 && close_fd_on_error)
		close(fd);
	return rc;
}

LIBEVDEV_EXPORT void
libevdev_uinput_destroy(struct libevdev_uinput *uinput_dev)
{
	if (!uinput_dev)
		return;

	if (uinput_dev->fd >= 0) {
		(void)ioctl(uinput_dev->fd, UI_DEV_DESTROY, NULL);
		if (uinput_dev->fd_is_managed)
			close(uinput_dev->fd);
	}
	free(uinput_dev->syspath);
	free(uinput_dev->devnode);
	free(uinput_dev->name);
	free(uinput_dev);
}

LIBEVDEV_EXPORT const char*
libevdev_uinput_get_syspath(struct libevdev_uinput *uinput_dev)
{
	return uinput_dev->syspath;
}

LIBEVDEV_EXPORT const char*
libevdev_uinput_get_devnode(struct libevdev_uinput *uinput_dev)
{
	return uinput_dev->devnode;
}

LIBEVDEV_EXPORT int
libevdev_uinput_write_event(const struct libevdev_uinput *uinput_dev,
			    unsigned int type,
			    unsigned int code,
			    int value)
{
	struct input_event ev = { {0,0}, type, code, value };
	int fd = libevdev_uinput_get_fd(uinput_dev);
	int rc, max;

	if (type > EV_MAX)
		return -EINVAL;

	max = libevdev_event_type_get_max(type);
	if (max == -1 || code > (unsigned int)max)
		return -EINVAL;

	rc = write(fd, &ev, sizeof(ev));

	return rc < 0 ? -errno : 0;
}
