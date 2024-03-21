// SPDX-License-Identifier: MIT
/*
 * Copyright © 2014 Red Hat, Inc.
 */

#include "config.h"
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/input.h>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include "test-common.h"

START_TEST(test_revoke)
{
	struct uinput_device* uidev;
	struct libevdev *dev, *dev2;
	int rc, fd;
	struct input_event ev1, ev2;
	int dev_fd;

	test_create_device(&uidev, &dev,
			   EV_SYN, SYN_REPORT,
			   EV_REL, REL_X,
			   EV_REL, REL_Y,
			   EV_REL, REL_WHEEL,
			   EV_KEY, BTN_LEFT,
			   EV_KEY, BTN_MIDDLE,
			   EV_KEY, BTN_RIGHT,
			   -1);

	fd = open(uinput_device_get_devnode(uidev), O_RDONLY|O_NONBLOCK);
	ck_assert_int_gt(fd, -1);
	rc = libevdev_new_from_fd(fd, &dev2);
	ck_assert_msg(rc == 0, "Failed to create second device: %s", strerror(-rc));

	uinput_device_event(uidev, EV_REL, REL_X, 1);
	uinput_device_event(uidev, EV_SYN, SYN_REPORT, 0);

	for (int i = 0; i < 2; i++) {
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev1);
		ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);

		rc = libevdev_next_event(dev2, LIBEVDEV_READ_FLAG_NORMAL, &ev2);
		ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);

		ck_assert_int_eq(ev1.type, ev2.type);
		ck_assert_int_eq(ev1.code, ev2.code);
		ck_assert_int_eq(ev1.value, ev2.value);
	}

	/* revoke first device, expect it closed, second device still open */
	dev_fd = libevdev_get_fd(dev);
	ck_assert_int_ge(dev_fd, 0);
	rc = ioctl(dev_fd, EVIOCREVOKE, NULL);
	if (rc == -1 && errno == EINVAL) {
		fprintf(stderr, "WARNING: skipping EVIOCREVOKE test, not suported by current kernel\n");
		goto out;
	}
	ck_assert_msg(rc == 0, "Failed to revoke device: %s", strerror(errno));

	uinput_device_event(uidev, EV_REL, REL_X, 1);
	uinput_device_event(uidev, EV_SYN, SYN_REPORT, 0);

	rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev1);
	ck_assert_int_eq(rc, -ENODEV);

	rc = libevdev_next_event(dev2, LIBEVDEV_READ_FLAG_NORMAL, &ev2);
	ck_assert_int_eq(rc, LIBEVDEV_READ_STATUS_SUCCESS);

out:
	uinput_device_free(uidev);
	libevdev_free(dev);
	libevdev_free(dev2);
	close(fd);
}
END_TEST

START_TEST(test_revoke_invalid)
{
	struct uinput_device* uidev;
	struct libevdev *dev;
	int rc;
	int dev_fd;

	test_create_device(&uidev, &dev,
			   EV_SYN, SYN_REPORT,
			   EV_REL, REL_X,
			   EV_REL, REL_Y,
			   EV_REL, REL_WHEEL,
			   EV_KEY, BTN_LEFT,
			   EV_KEY, BTN_MIDDLE,
			   EV_KEY, BTN_RIGHT,
			   -1);

	dev_fd = libevdev_get_fd(dev);
	ck_assert_int_ge(dev_fd, 0);
	/* ioctl requires 0 as value */
	rc = ioctl(dev_fd, EVIOCREVOKE, 1);
	ck_assert_int_eq(rc, -1);
	ck_assert_int_eq(errno, EINVAL);

	uinput_device_free(uidev);
	libevdev_free(dev);
}
END_TEST

START_TEST(test_revoke_fail_after)
{
	struct uinput_device* uidev;
	struct libevdev *dev, *dev2 = NULL;
	int rc, fd;

	test_create_device(&uidev, &dev,
			   EV_SYN, SYN_REPORT,
			   EV_REL, REL_X,
			   EV_REL, REL_Y,
			   EV_REL, REL_WHEEL,
			   EV_KEY, BTN_LEFT,
			   EV_KEY, BTN_MIDDLE,
			   EV_KEY, BTN_RIGHT,
			   -1);

	fd = open(uinput_device_get_devnode(uidev), O_RDONLY|O_NONBLOCK);
	ck_assert_int_gt(fd, -1);

	rc = ioctl(fd, EVIOCREVOKE, NULL);
	if (rc == -1 && errno == EINVAL) {
		fprintf(stderr, "WARNING: skipping EVIOCREVOKE test, not suported by current kernel\n");
		goto out;
	}
	ck_assert_msg(rc == 0, "Failed to revoke device: %s", strerror(errno));

	rc = libevdev_new_from_fd(fd, &dev2);
	ck_assert_int_eq(rc, -ENODEV);

out:
	uinput_device_free(uidev);
	libevdev_free(dev);
	close(fd);
}
END_TEST

TEST_SUITE_ROOT_PRIVILEGES(kernel)
{
	Suite *s = suite_create("kernel");

	add_test(s, test_revoke);
	add_test(s, test_revoke_invalid);
	add_test(s, test_revoke_fail_after);

	return s;
}
