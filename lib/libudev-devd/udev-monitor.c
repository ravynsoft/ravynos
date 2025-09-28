/*
 * Copyright (c) 2015, 2021 Vladimir Kondratyev <vladimir@kondratyev.su>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libudev.h"
#include "udev.h"
#include "udev-device.h"
#include "udev-utils.h"
#include "udev-filter.h"
#include "udev-utils.h"
#include "utils.h"

#define	DEVD_SOCK_PATH		"/var/run/devd.seqpacket.pipe"
#define	DEVD_RECONNECT_INTERVAL	1000	/* reconnect after 1 second */

#define	DEVD_EVENT_ATTACH	'+'
#define	DEVD_EVENT_DETACH	'-'
#define	DEVD_EVENT_NOTICE	'!'
#define	DEVD_EVENT_UNKNOWN	'?'

STAILQ_HEAD(udev_monitor_queue_head, udev_monitor_queue_entry);
struct udev_monitor_queue_entry {
	struct udev_device *ud;
	STAILQ_ENTRY(udev_monitor_queue_entry) next;
};

struct udev_monitor {
	int refcount;
	int fds[2];
	struct udev_filter_head filters;
	struct udev *udev;
	struct udev_monitor_queue_head queue;
	pthread_mutex_t mtx;
	pthread_t thread;
};

LIBUDEV_EXPORT struct udev_device *
udev_monitor_receive_device(struct udev_monitor *um)
{
	struct udev_monitor_queue_entry *umqe;
	struct udev_device *ud;
	char buf[1];

	TRC("(%p)", um);
	if (read(um->fds[0], buf, 1) < 0)
		return (NULL);

	if (STAILQ_EMPTY(&um->queue))
		return (NULL);

	pthread_mutex_lock(&um->mtx);
	umqe = STAILQ_FIRST(&um->queue);
	STAILQ_REMOVE_HEAD(&um->queue, next);
	pthread_mutex_unlock(&um->mtx);
	ud = umqe->ud;
	free(umqe);

	return (ud);
}

static int
udev_monitor_send_device(struct udev_monitor *um, const char *syspath,
    int action)
{
	struct udev_monitor_queue_entry *umqe;

	umqe = calloc(1, sizeof(struct udev_monitor_queue_entry));
	if (umqe == NULL)
		return (-1);

	umqe->ud = udev_device_new_common(um->udev, syspath, action);
	if (umqe->ud == NULL) {
		free(umqe);
		return (-1);
	}

	pthread_mutex_lock(&um->mtx);
	STAILQ_INSERT_TAIL(&um->queue, umqe, next);
	pthread_mutex_unlock(&um->mtx);

	if (write(um->fds[1], "*", 1) != 1) {
		pthread_mutex_lock(&um->mtx);
		STAILQ_REMOVE(&um->queue, umqe, udev_monitor_queue_entry, next);
		pthread_mutex_unlock(&um->mtx);
		udev_device_unref(umqe->ud);
		free(umqe);
		return (-1);
	}

	return (0);
}

static int
parse_devd_message(char *msg, char *syspath, size_t syspathlen)
{
	char devpath[DEV_PATH_MAX] = DEV_PATH_ROOT "/";
	const char *type, *dev_name;
	size_t type_len, dev_len, root_len;
	int action;

	root_len = strlen(devpath);
	action = UD_ACTION_NONE;

	switch (msg[0]) {
#ifdef HAVE_DEVINFO_H
	case DEVD_EVENT_ATTACH:
		action = UD_ACTION_ADD;
		/* FALLTHROUGH */
	case DEVD_EVENT_DETACH:
		if (action == UD_ACTION_NONE)
			action = UD_ACTION_REMOVE;
		*(strchrnul(msg + 1, ' ')) = '\0';
		strlcpy(syspath, msg + 1, syspathlen);
		break;
#endif /* HAVE_DEVINFO_H */
	case DEVD_EVENT_NOTICE:
		if (!(match_kern_prop_value(msg + 1, "system", "DEVFS")
			&& match_kern_prop_value(msg + 1, "subsystem", "CDEV"))
			&& !match_kern_prop_value(msg + 1, "system", "DRM"))
			break;
		type = get_kern_prop_value(msg + 1, "type", &type_len);
		dev_name = get_kern_prop_value(msg + 1, "cdev", &dev_len);
		if (type == NULL ||
		    dev_name == NULL ||
		    dev_len > (sizeof(devpath) - root_len - 1))
			break;
		if (type_len == 6 &&
		    strncmp(type, "CREATE", type_len) == 0)
			action = UD_ACTION_ADD;
		else if (type_len == 7 &&
		    strncmp(type, "DESTROY", type_len) == 0)
			action = UD_ACTION_REMOVE;
		else if (type_len == 7 &&
		    strncmp(type, "HOTPLUG", type_len) == 0)
			action = UD_ACTION_HOTPLUG;
		else
			break;
		memcpy(devpath + root_len, dev_name, dev_len);
		devpath[dev_len + root_len] = 0;
		strlcpy(syspath, get_syspath_by_devpath(devpath), syspathlen);
		break;
	case DEVD_EVENT_UNKNOWN:
	default:
		break;
	}

	return (action);
}

static void *
udev_monitor_thread(void *args)
{
	struct udev_monitor *um = args;
	char ev[1024], syspath[DEV_PATH_MAX];
	struct pollfd fds[2];
	nfds_t nfds;
	ssize_t len;
	int devd_fd = -1, ret, action, timeout;
	sigset_t set;
	const static struct sockaddr_un sa = {
		.sun_family = AF_UNIX,
		.sun_path = DEVD_SOCK_PATH,
	};

	sigfillset(&set);
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	fds[0].fd = um->fds[1];
	fds[0].events = 0;
	fds[1].events = POLLIN;

	for (;;) {
		if (devd_fd < 0 &&
		    (devd_fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) >= 0 &&
		    connect(devd_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			close(devd_fd);
			devd_fd = -1;
		}

		if (devd_fd < 0) {
			nfds = 1;
			timeout = DEVD_RECONNECT_INTERVAL;
		} else {
			fds[1].fd = devd_fd;
			nfds = 2;
			timeout = -1;
		}

		ret = poll(fds, nfds, timeout);
		if (ret == -1 && errno == EINTR)
			continue;
		if (ret == -1)
			break;

		/* edev_monitor is finishing */
		if (fds[0].revents & POLLHUP)
			break;

		/* connection respawn timer expired */
		if (ret == 0 || devd_fd < 1)
			continue;

		if (fds[1].revents & POLLIN) {
			if ((len = recv(devd_fd, ev, sizeof(ev), MSG_WAITALL))
			    <= 0) {
				close(devd_fd);
				devd_fd = -1;
				continue;
			}
			/* Replace terminating LF with 0 to make C-string */
			ev[len - 1] = '\0';
			action = parse_devd_message(ev, syspath, sizeof(syspath));
			if (action != UD_ACTION_NONE &&
			    udev_filter_match(um->udev, &um->filters, syspath))
				udev_monitor_send_device(um, syspath, action);
		}

		if (fds[1].revents & POLLHUP) {
			close(devd_fd);
			devd_fd = -1;
		}
	}

	if (devd_fd >= 0)
		close(devd_fd);

	return (NULL);
}

LIBUDEV_EXPORT struct udev_monitor *
udev_monitor_new_from_netlink(struct udev *udev, const char *name)
{
	struct udev_monitor *um;
	
	TRC("(%p, %s)", udev, name);
	um = calloc(1, sizeof(struct udev_monitor));
	if (!um)
		return (NULL);

	if (pipe2(um->fds, O_CLOEXEC) == -1) {
		ERR("pipe2 failed");
		free(um);
		return (NULL);
	}

	um->udev = udev;
	_udev_ref(udev);
	um->refcount = 1;
	udev_filter_init(&um->filters);
	STAILQ_INIT(&um->queue);
	pthread_mutex_init(&um->mtx, NULL);

	return (um);
}

LIBUDEV_EXPORT int
udev_monitor_filter_add_match_subsystem_devtype(struct udev_monitor *um,
    const char *subsystem, const char *devtype)
{

	TRC("(%p, %s, %s)", um, subsystem, devtype);
	return (udev_filter_add(&um->filters, UDEV_FILTER_TYPE_SUBSYSTEM, 0,
	    subsystem, NULL));
}

LIBUDEV_EXPORT int
udev_monitor_filter_add_match_tag(struct udev_monitor *um, const char *tag)
{
	TRC("(%p, %s)", um, tag);
	UNIMPL();
	return (0);
}

LIBUDEV_EXPORT int
udev_monitor_enable_receiving(struct udev_monitor *um)
{

	TRC("(%p)", um);

	if (pthread_create(&um->thread, NULL, udev_monitor_thread, um) != 0) {
		ERR("thread_create failed");
		return (-1);
	}

	return (0);
}

LIBUDEV_EXPORT int
udev_monitor_get_fd(struct udev_monitor *um)
{

	/* TRC("(%p)", um); */
	return (um->fds[0]);
}

LIBUDEV_EXPORT struct udev_monitor *
udev_monitor_ref(struct udev_monitor *um)
{

	TRC("(%p) refcount=%d", um, um->refcount);
	++um->refcount;
	return (um);
}

static void
udev_monitor_queue_drop(struct udev_monitor_queue_head *umqh)
{
	struct udev_monitor_queue_entry *umqe;

	while (!STAILQ_EMPTY(umqh)) {
		umqe = STAILQ_FIRST(umqh);
		STAILQ_REMOVE_HEAD(umqh, next);
		udev_device_unref(umqe->ud);
		free(umqe);
	}
}

LIBUDEV_EXPORT void
udev_monitor_unref(struct udev_monitor *um)
{
	TRC("(%p) refcount=%d", um, um->refcount);
	if (--um->refcount == 0) {
		close(um->fds[0]);
		pthread_join(um->thread, NULL);
		close(um->fds[1]);
		udev_filter_free(&um->filters);
		udev_monitor_queue_drop(&um->queue);
		pthread_mutex_destroy(&um->mtx);
		_udev_unref(um->udev);
		free(um);
	}
}

LIBUDEV_EXPORT
struct udev *udev_monitor_get_udev(struct udev_monitor *um)
{

	TRC();
	return (um->udev);
}

LIBUDEV_EXPORT int
udev_monitor_set_receive_buffer_size(struct udev_monitor *um, int size)
{
	TRC("(%d)", size);
	UNIMPL();
	return (0);
}

LIBUDEV_EXPORT int
udev_monitor_filter_update(struct udev_monitor *um)
{
	TRC();
	UNIMPL();
	return (0);
}

LIBUDEV_EXPORT int
udev_monitor_filter_remove(struct udev_monitor *um)
{
	TRC();
	UNIMPL();
	return (0);
}
