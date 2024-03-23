#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/un.h>
#include <unistd.h>

#if defined(HAVE_LIBELOGIND)
#include <elogind/sd-bus.h>
#include <elogind/sd-login.h>
#elif defined(HAVE_LIBSYSTEMD)
#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#else
#error logind backend requires either elogind or systemd
#endif

#include "backend.h"
#include "drm.h"
#include "libseat.h"
#include "log.h"

static int dev_major_is_drm(unsigned int dev_major) {
	return dev_major == 226;
}

static int dev_is_drm(dev_t device) {
	return dev_major_is_drm(major(device));
}

struct backend_logind {
	struct libseat base;
	const struct libseat_seat_listener *seat_listener;
	void *seat_listener_data;

	sd_bus *bus;
	char *id;
	char *seat;
	char *path;
	char *seat_path;

	bool active;
	bool initial_setup;
	int has_drm;
};

const struct seat_impl logind_impl;
static struct backend_logind *backend_logind_from_libseat_backend(struct libseat *base);
static void release_control(struct backend_logind *backend);

static void destroy(struct backend_logind *backend) {
	assert(backend);
	if (backend->bus != NULL) {
		sd_bus_unref(backend->bus);
	}
	free(backend->id);
	free(backend->seat);
	free(backend->path);
	free(backend->seat_path);
	free(backend);
}

static int close_seat(struct libseat *base) {
	struct backend_logind *backend = backend_logind_from_libseat_backend(base);
	release_control(backend);
	destroy(backend);
	return 0;
}

static int ping_handler(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
	(void)ret_error;
	(void)userdata;
	if (sd_bus_message_is_method_error(m, NULL)) {
		const sd_bus_error *error = sd_bus_message_get_error(m);
		log_errorf("Ping failed: %s: %s", error->name, error->message);
		return -1;
	}
	return 0;
}

static int send_ping(struct backend_logind *backend) {
	int ret = sd_bus_call_method_async(backend->bus, NULL, "org.freedesktop.login1",
					   "/org/freedesktop/login1", "org.freedesktop.DBus.Peer",
					   "Ping", ping_handler, backend, "");
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static void check_pending_events(struct backend_logind *backend) {
	uint64_t queued_read, queued_write;
	sd_bus_get_n_queued_read(backend->bus, &queued_read);
	sd_bus_get_n_queued_write(backend->bus, &queued_write);

	if (queued_read == 0 && queued_write == 0) {
		return;
	}

	// The sd_bus instance has queued data, so a dispatch is required.
	// However, we likely already drained our socket, so there will not be
	// anything to read. Instead, send a ping request to logind so that the
	// user will be woken up by its response.
	int ret = send_ping(backend);
	if (ret < 0) {
		log_errorf("Could not send ping message: %s", strerror(-ret));
		return;
	}
}

static int open_device(struct libseat *base, const char *path, int *fd) {
	struct backend_logind *session = backend_logind_from_libseat_backend(base);

	int ret;
	int tmpfd = -1;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	struct stat st;
	if (stat(path, &st) < 0) {
		log_errorf("Could not stat path '%s'", path);
		return -1;
	}

	ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", session->path,
				 "org.freedesktop.login1.Session", "TakeDevice", &error, &msg, "uu",
				 major(st.st_rdev), minor(st.st_rdev));
	if (ret < 0) {
		log_errorf("Could not take device: %s", error.message);
		tmpfd = -1;
		goto out;
	}

	int paused = 0;
	ret = sd_bus_message_read(msg, "hb", &tmpfd, &paused);
	if (ret < 0) {
		log_errorf("Could not parse D-Bus response: %s", strerror(-ret));
		tmpfd = -1;
		goto out;
	}

	// The original fd seems to be closed when the message is freed
	// so we just clone it.
	tmpfd = fcntl(tmpfd, F_DUPFD_CLOEXEC, 0);
	if (tmpfd < 0) {
		log_errorf("Could not duplicate fd: %s", strerror(errno));
		tmpfd = -1;
		goto out;
	}

	if (dev_is_drm(st.st_rdev)) {
		session->has_drm++;
		log_debugf("DRM device opened, current total: %d", session->has_drm);
	}

	*fd = tmpfd;

out:
	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	check_pending_events(session);
	return tmpfd;
}

static int close_device(struct libseat *base, int device_id) {
	struct backend_logind *session = backend_logind_from_libseat_backend(base);
	if (device_id < 0) {
		errno = EINVAL;
		return -1;
	}

	int fd = device_id;

	struct stat st = {0};
	if (fstat(fd, &st) < 0) {
		log_errorf("Could not stat fd %d", fd);
		return -1;
	}
	if (dev_is_drm(st.st_rdev)) {
		session->has_drm--;
		log_debugf("DRM device closed, current total: %d", session->has_drm);
		assert(session->has_drm >= 0);
	}

	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	int ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", session->path,
				     "org.freedesktop.login1.Session", "ReleaseDevice", &error,
				     &msg, "uu", major(st.st_rdev), minor(st.st_rdev));
	if (ret < 0) {
		log_errorf("Could not close device: %s", error.message);
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	check_pending_events(session);
	return ret < 0 ? -1 : 0;
}

static int switch_session(struct libseat *base, int s) {
	struct backend_logind *session = backend_logind_from_libseat_backend(base);
	if (s < 0) {
		errno = EINVAL;
		return -1;
	}

	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	int ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", session->seat_path,
				     "org.freedesktop.login1.Seat", "SwitchTo", &error, &msg, "u",
				     (uint32_t)s);
	if (ret < 0) {
		log_errorf("Could not switch session: %s", error.message);
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	check_pending_events(session);
	return ret < 0 ? -1 : 0;
}

static int disable_seat(struct libseat *base) {
	(void)base;
	return 0;
}

static int get_fd(struct libseat *base) {
	struct backend_logind *backend = backend_logind_from_libseat_backend(base);
	int fd = sd_bus_get_fd(backend->bus);
	if (fd >= 0) {
		return fd;
	}

	errno = -fd;
	return -1;
}

static int poll_connection(struct backend_logind *backend, int timeout) {
	struct pollfd fd = {
		.fd = sd_bus_get_fd(backend->bus),
		.events = POLLIN,
	};

	if (poll(&fd, 1, timeout) == -1) {
		if (errno == EAGAIN || errno == EINTR) {
			return 0;
		} else {
			return -1;
		}
	}

	if (fd.revents & (POLLERR | POLLHUP)) {
		errno = ECONNRESET;
		return -1;
	}
	return 0;
}

static int dispatch_and_execute(struct libseat *base, int timeout) {
	struct backend_logind *backend = backend_logind_from_libseat_backend(base);
	if (backend->initial_setup) {
		backend->initial_setup = false;
		if (backend->active) {
			backend->seat_listener->enable_seat(&backend->base,
							    backend->seat_listener_data);
		} else {
			backend->seat_listener->disable_seat(&backend->base,
							     backend->seat_listener_data);
		}
	}

	int total_dispatched = 0;
	int dispatched = 0;
	while ((dispatched = sd_bus_process(backend->bus, NULL)) > 0) {
		total_dispatched += dispatched;
	}
	if (total_dispatched == 0 && timeout != 0) {
		if (poll_connection(backend, timeout) == -1) {
			log_errorf("Could not poll connection: %s", strerror(errno));
			return -1;
		}
		while ((dispatched = sd_bus_process(backend->bus, NULL)) > 0) {
			total_dispatched += dispatched;
		}
	}
	check_pending_events(backend);
	return total_dispatched;
}

static const char *seat_name(struct libseat *base) {
	struct backend_logind *backend = backend_logind_from_libseat_backend(base);
	return backend->seat;
}

static struct backend_logind *backend_logind_from_libseat_backend(struct libseat *base) {
	assert(base->impl == &logind_impl);
	return (struct backend_logind *)base;
}

static int session_activate(struct backend_logind *session) {
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	// Note: the Activate call might not make the session active immediately
	int ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", session->path,
				     "org.freedesktop.login1.Session", "Activate", &error, &msg, "");
	if (ret < 0) {
		log_errorf("Could not activate session: %s", error.message);
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	return ret;
}

static int session_check_active(struct backend_logind *session) {
	sd_bus_error error = SD_BUS_ERROR_NULL;
	int active = 0;
	int ret = sd_bus_get_property_trivial(session->bus, "org.freedesktop.login1", session->path,
					      "org.freedesktop.login1.Session", "Active", &error,
					      'b', &active);
	if (ret < 0) {
		log_errorf("Could not check if session is active: %s", error.message);
	} else {
		session->active = (bool)active;
	}

	sd_bus_error_free(&error);
	return ret;
}

static int take_control(struct backend_logind *session) {
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	int ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", session->path,
				     "org.freedesktop.login1.Session", "TakeControl", &error, &msg,
				     "b", false);
	if (ret < 0) {
		log_errorf("Could not take control of session: %s", error.message);
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	return ret;
}

static void release_control(struct backend_logind *session) {
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	int ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", session->path,
				     "org.freedesktop.login1.Session", "ReleaseControl", &error,
				     &msg, "");
	if (ret < 0) {
		log_errorf("Could not release control of session: %s", error.message);
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
}

static void set_active(struct backend_logind *backend, bool active) {
	if (backend->active == active) {
		return;
	}

	backend->active = active;
	if (active) {
		log_info("Enabling seat");
		backend->seat_listener->enable_seat(&backend->base, backend->seat_listener_data);
	} else {
		log_info("Disabling seat");
		backend->seat_listener->disable_seat(&backend->base, backend->seat_listener_data);
	}
}

static int pause_device(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error) {
	struct backend_logind *session = userdata;

	uint32_t major, minor;
	const char *type;
	int ret = sd_bus_message_read(msg, "uus", &major, &minor, &type);
	if (ret < 0) {
		log_errorf("Could not parse D-Bus response: %s", strerror(-ret));
		return 0;
	}

	if (dev_major_is_drm(major) && strcmp(type, "gone") != 0) {
		log_debugf("DRM device paused: %s", type);
		assert(session->has_drm > 0);
		set_active(session, false);
	}

	if (strcmp(type, "pause") == 0) {
		ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", session->path,
					 "org.freedesktop.login1.Session", "PauseDeviceComplete",
					 ret_error, &msg, "uu", major, minor);
		if (ret < 0) {
			log_errorf("Could not send PauseDeviceComplete signal: %s",
				   ret_error->message);
		}
	}

	return 0;
}

static int resume_device(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error) {
	(void)ret_error;
	struct backend_logind *session = userdata;
	int ret;

	int fd;
	uint32_t major, minor;
	ret = sd_bus_message_read(msg, "uuh", &major, &minor, &fd);
	if (ret < 0) {
		log_errorf("Could not parse D-Bus response: %s", strerror(-ret));
		return 0;
	}

	if (dev_major_is_drm(major)) {
		log_debug("DRM device resumed");
		assert(session->has_drm > 0);
		set_active(session, true);
	}

	return 0;
}

static int properties_changed(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error) {
	(void)ret_error;
	struct backend_logind *session = userdata;
	int ret = 0;

	if (session->has_drm > 0) {
		return 0;
	}

	// PropertiesChanged arg 1: interface
	const char *interface;
	ret = sd_bus_message_read_basic(msg, 's', &interface); // skip path
	if (ret < 0) {
		goto error;
	}

	bool is_session = strcmp(interface, "org.freedesktop.login1.Session") == 0;
	bool is_seat = strcmp(interface, "org.freedesktop.login1.Seat") == 0;
	if (!is_session || !is_seat) {
		// not interesting for us; ignore
		return 0;
	}

	// PropertiesChanged arg 2: changed properties with values
	ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
	if (ret < 0) {
		goto error;
	}

	const char *s;
	while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
		ret = sd_bus_message_read_basic(msg, 's', &s);
		if (ret < 0) {
			goto error;
		}

		if (is_session && strcmp(s, "Active") == 0) {
			int ret;
			ret = sd_bus_message_enter_container(msg, 'v', "b");
			if (ret < 0) {
				goto error;
			}

			bool value;
			ret = sd_bus_message_read_basic(msg, 'b', &value);
			if (ret < 0) {
				goto error;
			}

			log_debugf("%s state changed: %d", s, value);
			set_active(session, value);
			return 0;
		} else {
			sd_bus_message_skip(msg, "{sv}");
		}

		ret = sd_bus_message_exit_container(msg);
		if (ret < 0) {
			goto error;
		}
	}

	if (ret < 0) {
		goto error;
	}

	ret = sd_bus_message_exit_container(msg);
	if (ret < 0) {
		goto error;
	}

	// PropertiesChanged arg 3: changed properties without values
	sd_bus_message_enter_container(msg, 'a', "s");
	while ((ret = sd_bus_message_read_basic(msg, 's', &s)) > 0) {
		if (is_session && strcmp(s, "Active") == 0) {
			sd_bus_error error = SD_BUS_ERROR_NULL;
			const char *obj = "org.freedesktop.login1.Session";
			const char *field = "Active";
			bool value;
			ret = sd_bus_get_property_trivial(session->bus, "org.freedesktop.login1",
							  session->path, obj, field, &error, 'b',
							  &value);
			if (ret < 0) {
				log_errorf("Could not get '%s' property: %s", field, error.message);
				return 0;
			}

			log_debugf("%s state changed: %d", field, value);
			set_active(session, value);
			return 0;
		}
	}

error:
	if (ret < 0) {
		log_errorf("Could not parse D-Bus PropertiesChanged: %s", strerror(-ret));
	}

	return 0;
}

static int add_signal_matches(struct backend_logind *backend) {
	static const char *logind = "org.freedesktop.login1";
	static const char *session_interface = "org.freedesktop.login1.Session";
	static const char *property_interface = "org.freedesktop.DBus.Properties";
	int ret;

	ret = sd_bus_match_signal(backend->bus, NULL, logind, backend->path, session_interface,
				  "PauseDevice", pause_device, backend);
	if (ret < 0) {
		log_errorf("Could not add D-Bus match: %s", strerror(-ret));
		return ret;
	}

	ret = sd_bus_match_signal(backend->bus, NULL, logind, backend->path, session_interface,
				  "ResumeDevice", resume_device, backend);
	if (ret < 0) {
		log_errorf("Could not add D-Bus match: %s", strerror(-ret));
		return ret;
	}

	ret = sd_bus_match_signal(backend->bus, NULL, logind, backend->path, property_interface,
				  "PropertiesChanged", properties_changed, backend);
	if (ret < 0) {
		log_errorf("Could not add D-Bus match: %s", strerror(-ret));
		return ret;
	}

	ret = sd_bus_match_signal(backend->bus, NULL, logind, backend->seat_path, property_interface,
				  "PropertiesChanged", properties_changed, backend);
	if (ret < 0) {
		log_errorf("Could not add D-Bus match: %s", strerror(-ret));
		return ret;
	}

	return 0;
}

static int find_session_path(struct backend_logind *session) {
	int ret;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", "/org/freedesktop/login1",
				 "org.freedesktop.login1.Manager", "GetSession", &error, &msg, "s",
				 session->id);
	if (ret < 0) {
		log_errorf("Could not get session: %s", error.message);
		goto out;
	}

	const char *path;
	ret = sd_bus_message_read(msg, "o", &path);
	if (ret < 0) {
		log_errorf("Could not parse D-Bus response: %s", strerror(-ret));
		goto out;
	}
	session->path = strdup(path);

out:
	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);

	return ret;
}

static int find_seat_path(struct backend_logind *session) {
	int ret;
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	ret = sd_bus_call_method(session->bus, "org.freedesktop.login1", "/org/freedesktop/login1",
				 "org.freedesktop.login1.Manager", "GetSeat", &error, &msg, "s",
				 session->seat);
	if (ret < 0) {
		log_errorf("Could not get seat: %s", error.message);
		goto out;
	}

	const char *path;
	ret = sd_bus_message_read(msg, "o", &path);
	if (ret < 0) {
		log_errorf("Could not parse D-Bus response: %s", strerror(-ret));
		goto out;
	}
	session->seat_path = strdup(path);

out:
	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);

	return ret;
}

static int get_display_session(char **session_id) {
	assert(session_id != NULL);
	char *xdg_session_id = getenv("XDG_SESSION_ID");
	int ret;

	if (xdg_session_id) {
		// This just checks whether the supplied session ID is valid
		ret = sd_session_is_active(xdg_session_id);
		if (ret < 0) {
			log_errorf("Could not check if session was active: %s", strerror(-ret));
			goto error;
		}
		*session_id = strdup(xdg_session_id);
		goto success;
	}

	// If there's a session active for the current process then just use
	// that
	ret = sd_pid_get_session(getpid(), session_id);
	if (ret == 0) {
		goto success;
	}

	// Find any active sessions for the user if the process isn't part of an
	// active session itself
	ret = sd_uid_get_display(getuid(), session_id);
	if (ret < 0) {
		log_errorf("Could not get primary session for user: %s", strerror(-ret));
		goto error;
	}

success:
	assert(*session_id != NULL);
	return 0;

error:
	free(*session_id);
	*session_id = NULL;
	return ret;
}

static int set_type(struct backend_logind *backend, const char *type) {
	sd_bus_message *msg = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;

	int ret = sd_bus_call_method(backend->bus, "org.freedesktop.login1", backend->path,
				     "org.freedesktop.login1.Session", "SetType", &error, &msg, "s",
				     type);
	if (ret < 0) {
		log_errorf("Could not set session type: %s", error.message);
	}

	sd_bus_error_free(&error);
	sd_bus_message_unref(msg);
	return ret;
}

static struct libseat *logind_open_seat(const struct libseat_seat_listener *listener, void *data) {
	struct backend_logind *backend = calloc(1, sizeof(struct backend_logind));
	if (backend == NULL) {
		return NULL;
	}

	int ret;
	ret = get_display_session(&backend->id);
	if (ret < 0) {
		goto error;
	}

	ret = sd_session_get_seat(backend->id, &backend->seat);
	if (ret < 0) {
		goto error;
	}

	ret = sd_bus_default_system(&backend->bus);
	if (ret < 0) {
		goto error;
	}

	ret = find_session_path(backend);
	if (ret < 0) {
		goto error;
	}

	ret = find_seat_path(backend);
	if (ret < 0) {
		goto error;
	}

	ret = add_signal_matches(backend);
	if (ret < 0) {
		goto error;
	}

	ret = session_activate(backend);
	if (ret < 0) {
		goto error;
	}

	ret = session_check_active(backend);
	if (ret < 0) {
		goto error;
	}

	ret = take_control(backend);
	if (ret < 0) {
		goto error;
	}

	const char *env = getenv("XDG_SESSION_TYPE");
	if (env != NULL) {
		set_type(backend, env);
	}

	backend->initial_setup = true;
	backend->seat_listener = listener;
	backend->seat_listener_data = data;
	backend->base.impl = &logind_impl;

	check_pending_events(backend);
	return &backend->base;

error:
	destroy(backend);
	errno = -ret;
	return NULL;
}

const struct seat_impl logind_impl = {
	.open_seat = logind_open_seat,
	.disable_seat = disable_seat,
	.close_seat = close_seat,
	.seat_name = seat_name,
	.open_device = open_device,
	.close_device = close_device,
	.switch_session = switch_session,
	.get_fd = get_fd,
	.dispatch = dispatch_and_execute,
};
