// SPDX-License-Identifier: MIT
/*
 * Copyright © 2013 Red Hat, Inc.
 */

#include "config.h"
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libevdev-int.h"
#include "libevdev-util.h"
#include "libevdev.h"

#include "event-names.h"

#define MAXEVENTS 64

enum event_filter_status {
	EVENT_FILTER_NONE,	/**< Event untouched by filters */
	EVENT_FILTER_MODIFIED,	/**< Event was modified */
	EVENT_FILTER_DISCARD,	/**< Discard current event */
};

/* Keeps a record of touches during SYN_DROPPED  */
enum touch_state {
	TOUCH_OFF,
	TOUCH_STARTED, /* Started during SYN_DROPPED */
	TOUCH_STOPPED, /* Stopped during SYN_DROPPED */
	TOUCH_ONGOING, /* Existed before, still have same tracking ID */
	TOUCH_CHANGED, /* Existed before but have new tracking ID now, so
			  stopped + started in that slot */
};

struct slot_change_state {
	enum touch_state state;
	unsigned long axes[NLONGS(ABS_CNT)]; /* bitmask for updated axes */
};

static int sync_mt_state(struct libevdev *dev,
			 struct slot_change_state *changes_out);
static int
update_key_state(struct libevdev *dev, const struct input_event *e);

static inline int*
slot_value(const struct libevdev *dev, int slot, int axis)
{
	if (unlikely(slot > dev->num_slots)) {
		log_bug(dev, "Slot %d exceeds number of slots (%d)\n", slot, dev->num_slots);
		slot = 0;
	}
	if (unlikely(axis < ABS_MT_MIN || axis > ABS_MT_MAX)) {
		log_bug(dev, "MT axis %d is outside the valid range [%d,%d]\n",
			axis, ABS_MT_MIN, ABS_MT_MAX);
		axis = ABS_MT_MIN;
	}
	return &dev->mt_slot_vals[slot * ABS_MT_CNT + axis - ABS_MT_MIN];
}

static int
init_event_queue(struct libevdev *dev)
{
	const int MIN_QUEUE_SIZE = 256;
	int nevents = 1; /* terminating SYN_REPORT */
	int nslots;
	unsigned int type, code;

	/* count the number of axes, keys, etc. to get a better idea at how
	   many events per EV_SYN we could possibly get. That's the max we
	   may get during SYN_DROPPED too. Use double that, just so we have
	   room for events while syncing a device.
	 */
	for (type = EV_KEY; type < EV_MAX; type++) {
		int max = libevdev_event_type_get_max(type);
		for (code = 0; max > 0 && code < (unsigned int) max; code++) {
			if (libevdev_has_event_code(dev, type, code))
				nevents++;
		}
	}

	nslots = libevdev_get_num_slots(dev);
	if (nslots > 1) {
		int num_mt_axes = 0;

		for (code = ABS_MT_SLOT; code <= ABS_MAX; code++) {
			if (libevdev_has_event_code(dev, EV_ABS, code))
				num_mt_axes++;
		}

		/* We already counted the first slot in the initial count */
		nevents += num_mt_axes * (nslots - 1);
	}

	return queue_alloc(dev, max(MIN_QUEUE_SIZE, nevents * 2));
}

static void
libevdev_dflt_log_func(enum libevdev_log_priority priority,
		       void *data,
		       const char *file, int line, const char *func,
		       const char *format, va_list args)
{
	const char *prefix;
	switch(priority) {
		case LIBEVDEV_LOG_ERROR: prefix = "libevdev error"; break;
		case LIBEVDEV_LOG_INFO: prefix = "libevdev info"; break;
		case LIBEVDEV_LOG_DEBUG:
					prefix = "libevdev debug";
					break;
		default:
					prefix = "libevdev INVALID LOG PRIORITY";
					break;
	}
	/* default logging format:
	   libevev error in libevdev_some_func: blah blah
	   libevev info in libevdev_some_func: blah blah
	   libevev debug in file.c:123:libevdev_some_func: blah blah
	 */

	fprintf(stderr, "%s in ", prefix);
	if (priority == LIBEVDEV_LOG_DEBUG)
		fprintf(stderr, "%s:%d:", file, line);
	fprintf(stderr, "%s: ", func);
	vfprintf(stderr, format, args);
}

static void
fix_invalid_absinfo(const struct libevdev *dev,
		    int axis,
		    struct input_absinfo* abs_info)
{
	/*
	 * The reported absinfo for ABS_MT_TRACKING_ID is sometimes
	 * uninitialized for certain mtk-soc, due to init code mangling
	 * in the vendor kernel.
	 */
	if (axis == ABS_MT_TRACKING_ID &&
	    abs_info->maximum == abs_info->minimum) {
		abs_info->minimum = -1;
		abs_info->maximum = 0xFFFF;
		log_bug(dev,
			"Device \"%s\" has invalid ABS_MT_TRACKING_ID range",
			dev->name);
	}
}

/*
 * Global logging settings.
 */
static struct logdata log_data = {
	.priority = LIBEVDEV_LOG_INFO,
	.global_handler = libevdev_dflt_log_func,
	.userdata = NULL,
};

void
_libevdev_log_msg(const struct libevdev *dev,
		  enum libevdev_log_priority priority,
		  const char *file, int line, const char *func,
		  const char *format, ...)
{
	va_list args;

	if (dev && dev->log.device_handler) {
		/**
		 * if both global handler and device handler are set
		 * we've set up the handlers wrong.  And that means we'll
		 * likely get the printf args wrong and cause all sorts of
		 * mayhem. Seppuku is called for.
		 */
		if (unlikely(dev->log.global_handler))
			abort();

		if (priority > dev->log.priority)
			return;
	} else if (!log_data.global_handler || priority > log_data.priority) {
		return;
	} else if (unlikely(log_data.device_handler)) {
		abort(); /* Seppuku, see above */
	}

	va_start(args, format);
	if (dev && dev->log.device_handler)
		dev->log.device_handler(dev, priority, dev->log.userdata, file, line, func, format, args);
	else
		log_data.global_handler(priority, log_data.userdata, file, line, func, format, args);
	va_end(args);
}

static void
libevdev_reset(struct libevdev *dev)
{
	enum libevdev_log_priority pri = dev->log.priority;
	libevdev_device_log_func_t handler = dev->log.device_handler;

	free(dev->name);
	free(dev->phys);
	free(dev->uniq);
	free(dev->mt_slot_vals);
	memset(dev, 0, sizeof(*dev));
	dev->fd = -1;
	dev->initialized = false;
	dev->num_slots = -1;
	dev->current_slot = -1;
	dev->grabbed = LIBEVDEV_UNGRAB;
	dev->sync_state = SYNC_NONE;
	dev->log.priority = pri;
	dev->log.device_handler = handler;
	libevdev_enable_event_type(dev, EV_SYN);
}

LIBEVDEV_EXPORT struct libevdev*
libevdev_new(void)
{
	struct libevdev *dev;

	dev = calloc(1, sizeof(*dev));
	if (!dev)
		return NULL;

	libevdev_reset(dev);

	return dev;
}

LIBEVDEV_EXPORT int
libevdev_new_from_fd(int fd, struct libevdev **dev)
{
	struct libevdev *d;
	int rc;

	d = libevdev_new();
	if (!d)
		return -ENOMEM;

	rc = libevdev_set_fd(d, fd);
	if (rc < 0)
		libevdev_free(d);
	else
		*dev = d;
	return rc;
}

LIBEVDEV_EXPORT void
libevdev_free(struct libevdev *dev)
{
	if (!dev)
		return;

	queue_free(dev);
	libevdev_reset(dev);
	free(dev);
}

LIBEVDEV_EXPORT void
libevdev_set_log_function(libevdev_log_func_t logfunc, void *data)
{
	log_data.global_handler = logfunc;
	log_data.userdata = data;
}

LIBEVDEV_EXPORT void
libevdev_set_log_priority(enum libevdev_log_priority priority)
{
	if (priority > LIBEVDEV_LOG_DEBUG)
		priority = LIBEVDEV_LOG_DEBUG;
	log_data.priority = priority;
}

LIBEVDEV_EXPORT enum libevdev_log_priority
libevdev_get_log_priority(void)
{
	return log_data.priority;
}

LIBEVDEV_EXPORT void
libevdev_set_device_log_function(struct libevdev *dev,
				 libevdev_device_log_func_t logfunc,
				 enum libevdev_log_priority priority,
				 void *data)
{
	if (!dev) {
		log_bug(NULL, "device must not be NULL\n");
		return;
	}

	dev->log.priority = priority;
	dev->log.device_handler = logfunc;
	dev->log.userdata = data;
}

enum libevdev_log_priority
_libevdev_log_priority(const struct libevdev *dev)
{
	if (dev && dev->log.device_handler)
		return dev->log.priority;
	return libevdev_get_log_priority();
}

LIBEVDEV_EXPORT int
libevdev_change_fd(struct libevdev *dev, int fd)
{
	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -1;
	}
	dev->fd = fd;
	dev->grabbed = LIBEVDEV_UNGRAB;
	return 0;
}

static void
reset_tracking_ids(struct libevdev *dev)
{
	if (dev->num_slots == -1 ||
	    !libevdev_has_event_code(dev, EV_ABS, ABS_MT_TRACKING_ID))
		return;

	for (int slot = 0; slot < dev->num_slots; slot++)
		libevdev_set_slot_value(dev, slot, ABS_MT_TRACKING_ID, -1);
}

static inline void
free_slots(struct libevdev *dev)
{
	dev->num_slots = -1;
	free(dev->mt_slot_vals);
	dev->mt_slot_vals = NULL;
}

static int
init_slots(struct libevdev *dev)
{
	const struct input_absinfo *abs_info;
	int rc = 0;

	free(dev->mt_slot_vals);
	dev->mt_slot_vals = NULL;

	/* devices with ABS_RESERVED aren't MT devices,
	   see the documentation for multitouch-related
	   functions for more details */
	if (libevdev_has_event_code(dev, EV_ABS, ABS_RESERVED) ||
	    !libevdev_has_event_code(dev, EV_ABS, ABS_MT_SLOT)) {
		if (dev->num_slots != -1) {
			free_slots(dev);
		}
		return rc;
	}

	abs_info = libevdev_get_abs_info(dev, ABS_MT_SLOT);

	free_slots(dev);
	dev->num_slots = abs_info->maximum + 1;
	dev->mt_slot_vals = calloc(dev->num_slots * ABS_MT_CNT, sizeof(int));
	if (!dev->mt_slot_vals) {
		rc = -ENOMEM;
		goto out;
	}
	dev->current_slot = abs_info->value;

	reset_tracking_ids(dev);
out:
	return rc;
}

LIBEVDEV_EXPORT int
libevdev_set_fd(struct libevdev* dev, int fd)
{
	int rc;
	int i;
	char buf[256];

	if (dev->initialized) {
		log_bug(dev, "device already initialized.\n");
		return -EBADF;
	}

	if (fd < 0) {
		return -EBADF;
	}

	libevdev_reset(dev);

	rc = ioctl(fd, EVIOCGBIT(0, sizeof(dev->bits)), dev->bits);
	if (rc < 0)
		goto out;

	memset(buf, 0, sizeof(buf));
	rc = ioctl(fd, EVIOCGNAME(sizeof(buf) - 1), buf);
	if (rc < 0)
		goto out;

	free(dev->name);
	dev->name = strdup(buf);
	if (!dev->name) {
		errno = ENOMEM;
		goto out;
	}

	free(dev->phys);
	dev->phys = NULL;
	memset(buf, 0, sizeof(buf));
	rc = ioctl(fd, EVIOCGPHYS(sizeof(buf) - 1), buf);
	if (rc < 0) {
		/* uinput has no phys */
		if (errno != ENOENT)
			goto out;
	} else {
		dev->phys = strdup(buf);
		if (!dev->phys) {
			errno = ENOMEM;
			goto out;
		}
	}

	free(dev->uniq);
	dev->uniq = NULL;
	memset(buf, 0, sizeof(buf));
	rc = ioctl(fd, EVIOCGUNIQ(sizeof(buf) - 1), buf);
	if (rc < 0) {
		if (errno != ENOENT)
			goto out;
	} else  {
		dev->uniq = strdup(buf);
		if (!dev->uniq) {
			errno = ENOMEM;
			goto out;
		}
	}

	rc = ioctl(fd, EVIOCGID, &dev->ids);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGVERSION, &dev->driver_version);
	if (rc < 0)
		goto out;

	/* Built on a kernel with props, running against a kernel without property
	   support. This should not be a fatal case, we'll be missing properties but other
	   than that everything is as expected.
	 */
	rc = ioctl(fd, EVIOCGPROP(sizeof(dev->props)), dev->props);
	if (rc < 0 && errno != EINVAL)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_REL, sizeof(dev->rel_bits)), dev->rel_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(dev->abs_bits)), dev->abs_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_LED, sizeof(dev->led_bits)), dev->led_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(dev->key_bits)), dev->key_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_SW, sizeof(dev->sw_bits)), dev->sw_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_MSC, sizeof(dev->msc_bits)), dev->msc_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_FF, sizeof(dev->ff_bits)), dev->ff_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGBIT(EV_SND, sizeof(dev->snd_bits)), dev->snd_bits);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGKEY(sizeof(dev->key_values)), dev->key_values);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGLED(sizeof(dev->led_values)), dev->led_values);
	if (rc < 0)
		goto out;

	rc = ioctl(fd, EVIOCGSW(sizeof(dev->sw_values)), dev->sw_values);
	if (rc < 0)
		goto out;

	/* rep is a special case, always set it to 1 for both values if EV_REP is set */
	if (bit_is_set(dev->bits, EV_REP)) {
		for (i = 0; i < REP_CNT; i++)
			set_bit(dev->rep_bits, i);
		rc = ioctl(fd, EVIOCGREP, dev->rep_values);
		if (rc < 0)
			goto out;
	}

	for (i = ABS_X; i <= ABS_MAX; i++) {
		if (bit_is_set(dev->abs_bits, i)) {
			struct input_absinfo abs_info;
			rc = ioctl(fd, EVIOCGABS(i), &abs_info);
			if (rc < 0)
				goto out;

			fix_invalid_absinfo(dev, i, &abs_info);

			dev->abs_info[i] = abs_info;
		}
	}

	dev->fd = fd;

	rc = init_slots(dev);
	if (rc != 0)
		goto out;

	if (dev->num_slots != -1) {
		struct slot_change_state unused[dev->num_slots];
		sync_mt_state(dev, unused);
	}

	rc = init_event_queue(dev);
	if (rc < 0) {
		dev->fd = -1;
		return -rc;
	}

	/* not copying key state because we won't know when we'll start to
	 * use this fd and key's are likely to change state by then.
	 * Same with the valuators, really, but they may not change.
	 */

	dev->initialized = true;
out:
	if (rc)
		libevdev_reset(dev);
	return rc ? -errno : 0;
}

LIBEVDEV_EXPORT int
libevdev_get_fd(const struct libevdev* dev)
{
	return dev->fd;
}

static int
sync_key_state(struct libevdev *dev)
{
	int rc;
	int i;
	unsigned long keystate[NLONGS(KEY_CNT)] = {0};

	rc = ioctl(dev->fd, EVIOCGKEY(sizeof(keystate)), keystate);
	if (rc < 0)
		goto out;

	for (i = 0; i < KEY_CNT; i++) {
		int old, new;
		old = bit_is_set(dev->key_values, i);
		new = bit_is_set(keystate, i);
		if (old ^ new)
			queue_push_event(dev, EV_KEY, i, new ? 1 : 0);
	}

	memcpy(dev->key_values, keystate, rc);

	rc = 0;
out:
	return rc ? -errno : 0;
}

static int
sync_sw_state(struct libevdev *dev)
{
	int rc;
	int i;
	unsigned long swstate[NLONGS(SW_CNT)] = {0};

	rc = ioctl(dev->fd, EVIOCGSW(sizeof(swstate)), swstate);
	if (rc < 0)
		goto out;

	for (i = 0; i < SW_CNT; i++) {
		int old, new;
		old = bit_is_set(dev->sw_values, i);
		new = bit_is_set(swstate, i);
		if (old ^ new)
			queue_push_event(dev, EV_SW, i, new ? 1 : 0);
	}

	memcpy(dev->sw_values, swstate, rc);

	rc = 0;
out:
	return rc ? -errno : 0;
}

static int
sync_led_state(struct libevdev *dev)
{
	int rc;
	int i;
	unsigned long ledstate[NLONGS(LED_CNT)] = {0};

	rc = ioctl(dev->fd, EVIOCGLED(sizeof(ledstate)), ledstate);
	if (rc < 0)
		goto out;

	for (i = 0; i < LED_CNT; i++) {
		int old, new;
		old = bit_is_set(dev->led_values, i);
		new = bit_is_set(ledstate, i);
		if (old ^ new) {
			queue_push_event(dev, EV_LED, i, new ? 1 : 0);
		}
	}

	memcpy(dev->led_values, ledstate, rc);

	rc = 0;
out:
	return rc ? -errno : 0;
}
static int
sync_abs_state(struct libevdev *dev)
{
	int rc;
	int i;

	for (i = ABS_X; i < ABS_CNT; i++) {
		struct input_absinfo abs_info;

		if (i >= ABS_MT_MIN && i <= ABS_MT_MAX)
			continue;

		if (!bit_is_set(dev->abs_bits, i))
			continue;

		rc = ioctl(dev->fd, EVIOCGABS(i), &abs_info);
		if (rc < 0)
			goto out;

		if (dev->abs_info[i].value != abs_info.value) {
			queue_push_event(dev, EV_ABS, i, abs_info.value);
			dev->abs_info[i].value = abs_info.value;
		}
	}

	rc = 0;
out:
	return rc ? -errno : 0;
}

static int
sync_mt_state(struct libevdev *dev,
	      struct slot_change_state changes_out[dev->num_slots])
{
#define MAX_SLOTS 256
	int rc = 0;
	struct slot_change_state changes[MAX_SLOTS] = {0};
	unsigned int nslots = min(MAX_SLOTS, dev->num_slots);

	for (int axis = ABS_MT_MIN; axis <= ABS_MT_MAX; axis++) {
		/* EVIOCGMTSLOTS required format */
		struct mt_sync_state {
			uint32_t code;
			int32_t val[MAX_SLOTS];
		} mt_state;

		if (axis == ABS_MT_SLOT ||
		    !libevdev_has_event_code(dev, EV_ABS, axis))
			continue;

		mt_state.code = axis;
		rc = ioctl(dev->fd, EVIOCGMTSLOTS(sizeof(mt_state)), &mt_state);
		if (rc < 0)
			goto out;

		for (unsigned int slot = 0; slot < nslots; slot++) {
			int val_before = *slot_value(dev, slot, axis),
			    val_after = mt_state.val[slot];

			if (axis == ABS_MT_TRACKING_ID) {
				if (val_before == -1 && val_after != -1) {
					changes[slot].state = TOUCH_STARTED;
				} else if (val_before != -1 && val_after == -1) {
					changes[slot].state = TOUCH_STOPPED;
				} else if (val_before != -1 && val_after != -1 &&
					   val_before == val_after) {
					changes[slot].state = TOUCH_ONGOING;
				} else if (val_before != -1 && val_after != -1 &&
					   val_before != val_after) {
					changes[slot].state = TOUCH_CHANGED;
				} else {
					changes[slot].state = TOUCH_OFF;
				}
			}

			if (val_before == val_after)
				continue;

			*slot_value(dev, slot, axis) = val_after;

			set_bit(changes[slot].axes, axis);
			/* note that this slot has updates */
			set_bit(changes[slot].axes, ABS_MT_SLOT);
		}
	}

	if (dev->num_slots > MAX_SLOTS)
		memset(changes_out, 0, sizeof(*changes) * dev->num_slots);

	memcpy(changes_out, changes, sizeof(*changes) * nslots);
out:
	return rc;
}

static void
terminate_slots(struct libevdev *dev,
		const struct slot_change_state changes[dev->num_slots],
		int *last_reported_slot)
{
	const unsigned int map[] = {BTN_TOOL_FINGER, BTN_TOOL_DOUBLETAP,
				    BTN_TOOL_TRIPLETAP, BTN_TOOL_QUADTAP,
				    BTN_TOOL_QUINTTAP};
	bool touches_stopped = false;
	int ntouches_before = 0, ntouches_after = 0;

	/* For BTN_TOOL_* emulation, we need to know how many touches we had
	 * before and how many we have left once we terminate all the ones
	 * that changed and all the ones that stopped.
	 */
	for (int slot = 0; slot < dev->num_slots;  slot++) {
		switch(changes[slot].state) {
		case TOUCH_OFF:
			break;
		case TOUCH_CHANGED:
		case TOUCH_STOPPED:
			queue_push_event(dev, EV_ABS, ABS_MT_SLOT, slot);
			queue_push_event(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);

			*last_reported_slot = slot;
			touches_stopped = true;
			ntouches_before++;
			break;
		case TOUCH_ONGOING:
			ntouches_before++;
			ntouches_after++;
			break;
		case TOUCH_STARTED:
			break;
		}
	}

	/* If any of the touches stopped, we need to split the sync state
	   into two frames - one with all the stopped touches, one with the
	   new touches starting (if any) */
	if (touches_stopped) {
		/* Send through the required BTN_TOOL_ 0 and 1 events for
		 * the previous and current number of fingers. And update
		 * our own key state accordingly, so that during the second
		 * sync event frame sync_key_state() sets everything correctly
		 * for the *real* number of touches.
		 */
		if (ntouches_before > 0 && ntouches_before <= 5) {
			struct input_event ev = {
				.type = EV_KEY,
				.code = map[ntouches_before - 1],
				.value = 0,
			};
			queue_push_event(dev, ev.type, ev.code, ev.value);
			update_key_state(dev, &ev);
		}

		if (ntouches_after > 0 && ntouches_after <= 5) {
			struct input_event ev = {
				.type = EV_KEY,
				.code = map[ntouches_after - 1],
				.value = 1,
			};
			queue_push_event(dev, ev.type, ev.code, ev.value);
			update_key_state(dev, &ev);
		}

		queue_push_event(dev, EV_SYN, SYN_REPORT, 0);
	}
}

static int
push_mt_sync_events(struct libevdev *dev,
		    const struct slot_change_state changes[dev->num_slots],
		    int last_reported_slot)
{
	struct input_absinfo abs_info;
	int rc;

	for (int slot = 0; slot < dev->num_slots;  slot++) {
		/* stopped touches were already terminated in
		 * terminate_slots */
		if (changes[slot].state == TOUCH_STOPPED ||
		    !bit_is_set(changes[slot].axes, ABS_MT_SLOT))
			continue;

		queue_push_event(dev, EV_ABS, ABS_MT_SLOT, slot);
		last_reported_slot = slot;

		for (int axis = ABS_MT_MIN; axis <= ABS_MT_MAX; axis++) {
			if (axis == ABS_MT_SLOT ||
			    !libevdev_has_event_code(dev, EV_ABS, axis))
				continue;

			if (bit_is_set(changes[slot].axes, axis))
				queue_push_event(dev, EV_ABS, axis,
						 *slot_value(dev, slot, axis));
		}
	}

	/* add one last slot event to make sure the client is on the same
	   slot as the kernel */

	rc = ioctl(dev->fd, EVIOCGABS(ABS_MT_SLOT), &abs_info);
	if (rc < 0)
		goto out;

	dev->current_slot = abs_info.value;

	if (dev->current_slot != last_reported_slot)
		queue_push_event(dev, EV_ABS, ABS_MT_SLOT, dev->current_slot);

	rc = 0;
out:
	return rc ? -errno : 0;
}

static int
read_more_events(struct libevdev *dev)
{
	int free_elem;
	int len;
	struct input_event *next;

	free_elem = queue_num_free_elements(dev);
	if (free_elem <= 0)
		return 0;

	next = queue_next_element(dev);
	len = read(dev->fd, next, free_elem * sizeof(struct input_event));
	if (len < 0)
		return -errno;

	if (len > 0 && len % sizeof(struct input_event) != 0)
		return -EINVAL;

	if (len > 0) {
		int nev = len/sizeof(struct input_event);
		queue_set_num_elements(dev, queue_num_elements(dev) + nev);
	}

	return 0;
}

static inline void
drain_events(struct libevdev *dev)
{
	int rc;
	size_t nelem;
	int iterations = 0;
	const int max_iterations = 8; /* EVDEV_BUF_PACKETS in
					 kernel/drivers/input/evedev.c */

	queue_shift_multiple(dev, queue_num_elements(dev), NULL);

	do {
		rc = read_more_events(dev);
		if (rc == -EAGAIN)
			return;

		if (rc < 0) {
			log_error(dev, "Failed to drain events before sync.\n");
			return;
		}

		nelem = queue_num_elements(dev);
		queue_shift_multiple(dev, nelem, NULL);
	} while (iterations++ < max_iterations && nelem >= queue_size(dev));

	/* Our buffer should be roughly the same or bigger than the kernel
	   buffer in most cases, so we usually don't expect to recurse. If
	   we do, make sure we stop after max_iterations and proceed with
	   what we have.  This could happen if events queue up faster than
	   we can drain them.
	 */
	if (iterations >= max_iterations)
		log_info(dev, "Unable to drain events, buffer size mismatch.\n");
}

static int
sync_state(struct libevdev *dev)
{
	int rc = 0;
	bool want_mt_sync = false;
	int last_reported_slot = 0;
	struct slot_change_state changes[dev->num_slots > 0 ? dev->num_slots : 1];
		memset(changes, 0, sizeof(changes));

	 /* see section "Discarding events before synchronizing" in
	  * libevdev/libevdev.h */
	drain_events(dev);

	/* We generate one or two event frames during sync.
	 * The first one (if it exists) terminates all slots that have
	 * either terminated during SYN_DROPPED or changed their tracking
	 * ID.
	 *
	 * The second frame syncs everything up to the current state of the
	 * device - including re-starting those slots that have a changed
	 * tracking id.
	 */
	if (dev->num_slots > -1 &&
	    libevdev_has_event_code(dev, EV_ABS, ABS_MT_SLOT)) {
		want_mt_sync = true;
		rc = sync_mt_state(dev, changes);
		if (rc == 0)
			terminate_slots(dev, changes, &last_reported_slot);
		else
			want_mt_sync = false;
	}

	if (libevdev_has_event_type(dev, EV_KEY))
		rc = sync_key_state(dev);
	if (libevdev_has_event_type(dev, EV_LED))
		rc = sync_led_state(dev);
	if (libevdev_has_event_type(dev, EV_SW))
		rc = sync_sw_state(dev);
	if (rc == 0 && libevdev_has_event_type(dev, EV_ABS))
		rc = sync_abs_state(dev);
	if (rc == 0 && want_mt_sync)
		push_mt_sync_events(dev, changes, last_reported_slot);

	dev->queue_nsync = queue_num_elements(dev);

	if (dev->queue_nsync > 0) {
		queue_push_event(dev, EV_SYN, SYN_REPORT, 0);
		dev->queue_nsync++;
	}

	return rc;
}

static int
update_key_state(struct libevdev *dev, const struct input_event *e)
{
	if (!libevdev_has_event_type(dev, EV_KEY))
		return 1;

	if (e->code > KEY_MAX)
		return 1;

	set_bit_state(dev->key_values, e->code, e->value != 0);

	return 0;
}

static int
update_mt_state(struct libevdev *dev, const struct input_event *e)
{
	if (e->code == ABS_MT_SLOT && dev->num_slots > -1) {
		int i;
		dev->current_slot = e->value;
		/* sync abs_info with the current slot values */
		for (i = ABS_MT_SLOT + 1; i <= ABS_MT_MAX; i++) {
			if (libevdev_has_event_code(dev, EV_ABS, i))
				dev->abs_info[i].value = *slot_value(dev, dev->current_slot, i);
		}

		return 0;
	}

	if (dev->current_slot == -1)
		return 1;

	*slot_value(dev, dev->current_slot, e->code) = e->value;

	return 0;
}

static int
update_abs_state(struct libevdev *dev, const struct input_event *e)
{
	if (!libevdev_has_event_type(dev, EV_ABS))
		return 1;

	if (e->code > ABS_MAX)
		return 1;

	if (e->code >= ABS_MT_MIN && e->code <= ABS_MT_MAX)
		update_mt_state(dev, e);

	dev->abs_info[e->code].value = e->value;

	return 0;
}

static int
update_led_state(struct libevdev *dev, const struct input_event *e)
{
	if (!libevdev_has_event_type(dev, EV_LED))
		return 1;

	if (e->code > LED_MAX)
		return 1;

	set_bit_state(dev->led_values, e->code, e->value != 0);

	return 0;
}

static int
update_sw_state(struct libevdev *dev, const struct input_event *e)
{
	if (!libevdev_has_event_type(dev, EV_SW))
		return 1;

	if (e->code > SW_MAX)
		return 1;

	set_bit_state(dev->sw_values, e->code, e->value != 0);

	return 0;
}

static int
update_state(struct libevdev *dev, const struct input_event *e)
{
	int rc = 0;

	switch(e->type) {
		case EV_SYN:
		case EV_REL:
			break;
		case EV_KEY:
			rc = update_key_state(dev, e);
			break;
		case EV_ABS:
			rc = update_abs_state(dev, e);
			break;
		case EV_LED:
			rc = update_led_state(dev, e);
			break;
		case EV_SW:
			rc = update_sw_state(dev, e);
			break;
	}

	dev->last_event_time.tv_sec = e->input_event_sec;
	dev->last_event_time.tv_usec = e->input_event_usec;

	return rc;
}

/**
 * Sanitize/modify events where needed.
 */
static inline enum event_filter_status
sanitize_event(const struct libevdev *dev,
	       struct input_event *ev,
	       enum SyncState sync_state)
{
	if (!libevdev_has_event_code(dev, ev->type, ev->code))
		return EVENT_FILTER_DISCARD;

	if (unlikely(dev->num_slots > -1 &&
		     libevdev_event_is_code(ev, EV_ABS, ABS_MT_SLOT) &&
		     (ev->value < 0 || ev->value >= dev->num_slots))) {
		log_bug(dev, "Device \"%s\" received an invalid slot index %d."
				"Capping to announced max slot number %d.\n",
				dev->name, ev->value, dev->num_slots - 1);
		ev->value = dev->num_slots - 1;
		return EVENT_FILTER_MODIFIED;

	/* Drop any invalid tracking IDs, they are only supposed to go from
	   N to -1 or from -1 to N. Never from -1 to -1, or N to M. Very
	   unlikely to ever happen from a real device.
	   */
	}

	if (unlikely(sync_state == SYNC_NONE &&
			    dev->num_slots > -1 &&
			    libevdev_event_is_code(ev, EV_ABS, ABS_MT_TRACKING_ID) &&
			    ((ev->value == -1 &&
			     *slot_value(dev, dev->current_slot, ABS_MT_TRACKING_ID) == -1) ||
			     (ev->value != -1 &&
			     *slot_value(dev, dev->current_slot, ABS_MT_TRACKING_ID) != -1)))) {
		log_bug(dev, "Device \"%s\" received a double tracking ID %d in slot %d.\n",
			dev->name, ev->value, dev->current_slot);
		return EVENT_FILTER_DISCARD;
	}

	return EVENT_FILTER_NONE;
}

LIBEVDEV_EXPORT int
libevdev_next_event(struct libevdev *dev, unsigned int flags, struct input_event *ev)
{
	int rc = LIBEVDEV_READ_STATUS_SUCCESS;
	enum event_filter_status filter_status;
	const unsigned int valid_flags = LIBEVDEV_READ_FLAG_NORMAL |
					 LIBEVDEV_READ_FLAG_SYNC |
					 LIBEVDEV_READ_FLAG_FORCE_SYNC |
					 LIBEVDEV_READ_FLAG_BLOCKING;

	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	}

	if (dev->fd < 0)
		return -EBADF;

	if ((flags & valid_flags) == 0) {
		log_bug(dev, "invalid flags %#x.\n", flags);
		return -EINVAL;
	}

	if (flags & LIBEVDEV_READ_FLAG_SYNC) {
		if (dev->sync_state == SYNC_NEEDED) {
			rc = sync_state(dev);
			if (rc != 0)
				return rc;
			dev->sync_state = SYNC_IN_PROGRESS;
		}

		if (dev->queue_nsync == 0) {
			dev->sync_state = SYNC_NONE;
			return -EAGAIN;
		}

	} else if (dev->sync_state != SYNC_NONE) {
		struct input_event e;

		/* call update_state for all events here, otherwise the library has the wrong view
		   of the device too */
		while (queue_shift(dev, &e) == 0) {
			dev->queue_nsync--;
			if (sanitize_event(dev, &e, dev->sync_state) != EVENT_FILTER_DISCARD)
				update_state(dev, &e);
		}

		dev->sync_state = SYNC_NONE;
	}

	/* Always read in some more events. Best case this smoothes over a potential SYN_DROPPED,
	   worst case we don't read fast enough and end up with SYN_DROPPED anyway.

	   Except if the fd is in blocking mode and we still have events from the last read, don't
	   read in any more.
	 */
	do {
		if (queue_num_elements(dev) == 0) {
			rc = read_more_events(dev);
			if (rc < 0 && rc != -EAGAIN)
				goto out;
		}

		if (flags & LIBEVDEV_READ_FLAG_FORCE_SYNC) {
			dev->sync_state = SYNC_NEEDED;
			rc = LIBEVDEV_READ_STATUS_SYNC;
			goto out;
		}

		if (queue_shift(dev, ev) != 0)
			return -EAGAIN;

		filter_status = sanitize_event(dev, ev, dev->sync_state);
		if (filter_status != EVENT_FILTER_DISCARD)
			update_state(dev, ev);

	/* if we disabled a code, get the next event instead */
	} while(filter_status == EVENT_FILTER_DISCARD ||
		!libevdev_has_event_code(dev, ev->type, ev->code));

	rc = LIBEVDEV_READ_STATUS_SUCCESS;
	if (ev->type == EV_SYN && ev->code == SYN_DROPPED) {
		dev->sync_state = SYNC_NEEDED;
		rc = LIBEVDEV_READ_STATUS_SYNC;
	}

	if (flags & LIBEVDEV_READ_FLAG_SYNC && dev->queue_nsync > 0) {
		dev->queue_nsync--;
		rc = LIBEVDEV_READ_STATUS_SYNC;
		if (dev->queue_nsync == 0)
			dev->sync_state = SYNC_NONE;
	}

out:
	return rc;
}

LIBEVDEV_EXPORT int
libevdev_has_event_pending(struct libevdev *dev)
{
	struct pollfd fds = { dev->fd, POLLIN, 0 };
	int rc;

	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	}

	if (dev->fd < 0)
		return -EBADF;

	if (queue_num_elements(dev) != 0)
		return 1;

	rc = poll(&fds, 1, 0);
	return (rc >= 0) ? rc : -errno;
}

LIBEVDEV_EXPORT const char *
libevdev_get_name(const struct libevdev *dev)
{
	return dev->name ? dev->name : "";
}

LIBEVDEV_EXPORT const char *
libevdev_get_phys(const struct libevdev *dev)
{
	return dev->phys;
}

LIBEVDEV_EXPORT const char *
libevdev_get_uniq(const struct libevdev *dev)
{
	return dev->uniq;
}

#define STRING_SETTER(field) \
LIBEVDEV_EXPORT void libevdev_set_##field(struct libevdev *dev, const char *field) \
{ \
	if (field == NULL) \
		return; \
	free(dev->field); \
	dev->field = strdup(field); \
}

STRING_SETTER(name)
STRING_SETTER(phys)
STRING_SETTER(uniq)

#define PRODUCT_GETTER(name) \
LIBEVDEV_EXPORT int libevdev_get_id_##name(const struct libevdev *dev) \
{ \
	return dev->ids.name; \
}

PRODUCT_GETTER(product)
PRODUCT_GETTER(vendor)
PRODUCT_GETTER(bustype)
PRODUCT_GETTER(version)

#define PRODUCT_SETTER(field) \
LIBEVDEV_EXPORT void libevdev_set_id_##field(struct libevdev *dev, int field) \
{ \
	dev->ids.field = field;\
}

PRODUCT_SETTER(product)
PRODUCT_SETTER(vendor)
PRODUCT_SETTER(bustype)
PRODUCT_SETTER(version)

LIBEVDEV_EXPORT int
libevdev_get_driver_version(const struct libevdev *dev)
{
	return dev->driver_version;
}

LIBEVDEV_EXPORT int
libevdev_has_property(const struct libevdev *dev, unsigned int prop)
{
	return (prop <= INPUT_PROP_MAX) && bit_is_set(dev->props, prop);
}

LIBEVDEV_EXPORT int
libevdev_enable_property(struct libevdev *dev, unsigned int prop)
{
	if (prop > INPUT_PROP_MAX)
		return -1;

	set_bit(dev->props, prop);
	return 0;
}

LIBEVDEV_EXPORT int
libevdev_disable_property(struct libevdev *dev, unsigned int prop)
{
	if (prop > INPUT_PROP_MAX)
		return -1;

	clear_bit(dev->props, prop);
	return 0;
}

LIBEVDEV_EXPORT int
libevdev_has_event_type(const struct libevdev *dev, unsigned int type)
{
	return type == EV_SYN ||(type <= EV_MAX && bit_is_set(dev->bits, type));
}

LIBEVDEV_EXPORT int
libevdev_has_event_code(const struct libevdev *dev, unsigned int type, unsigned int code)
{
	const unsigned long *mask = NULL;
	int max;

	if (!libevdev_has_event_type(dev, type))
		return 0;

	if (type == EV_SYN)
		return 1;

	max = type_to_mask_const(dev, type, &mask);

	if (max == -1 || code > (unsigned int)max)
		return 0;

	return bit_is_set(mask, code);
}

LIBEVDEV_EXPORT int
libevdev_get_event_value(const struct libevdev *dev, unsigned int type, unsigned int code)
{
	int value = 0;

	if (!libevdev_has_event_type(dev, type) || !libevdev_has_event_code(dev, type, code))
		return 0;

	switch (type) {
		case EV_ABS: value = dev->abs_info[code].value; break;
		case EV_KEY: value = bit_is_set(dev->key_values, code); break;
		case EV_LED: value = bit_is_set(dev->led_values, code); break;
		case EV_SW: value = bit_is_set(dev->sw_values, code); break;
		case EV_REP:
			    switch(code) {
				    case REP_DELAY:
					    libevdev_get_repeat(dev, &value, NULL);
					    break;
				    case REP_PERIOD:
					    libevdev_get_repeat(dev, NULL, &value);
					    break;
				    default:
					    value = 0;
					    break;
			    }
			    break;
		default:
			value = 0;
			break;
	}

	return value;
}

LIBEVDEV_EXPORT int
libevdev_set_event_value(struct libevdev *dev, unsigned int type, unsigned int code, int value)
{
	int rc = 0;
	struct input_event e;

	if (!libevdev_has_event_type(dev, type) || !libevdev_has_event_code(dev, type, code))
		return -1;

	e.type = type;
	e.code = code;
	e.value = value;

	if (sanitize_event(dev, &e, SYNC_NONE) != EVENT_FILTER_NONE)
		return -1;

	switch(type) {
		case EV_ABS: rc = update_abs_state(dev, &e); break;
		case EV_KEY: rc = update_key_state(dev, &e); break;
		case EV_LED: rc = update_led_state(dev, &e); break;
		case EV_SW: rc = update_sw_state(dev, &e); break;
		default:
			     rc = -1;
			     break;
	}

	return rc;
}

LIBEVDEV_EXPORT int
libevdev_fetch_event_value(const struct libevdev *dev, unsigned int type, unsigned int code, int *value)
{
	if (libevdev_has_event_type(dev, type) &&
	    libevdev_has_event_code(dev, type, code)) {
		*value = libevdev_get_event_value(dev, type, code);
		return 1;
	}

	return 0;
}

LIBEVDEV_EXPORT int
libevdev_get_slot_value(const struct libevdev *dev, unsigned int slot, unsigned int code)
{
	if (!libevdev_has_event_type(dev, EV_ABS) || !libevdev_has_event_code(dev, EV_ABS, code))
		return 0;

	if (dev->num_slots < 0 || slot >= (unsigned int)dev->num_slots)
		return 0;

	if (code > ABS_MT_MAX || code < ABS_MT_MIN)
		return 0;

	return *slot_value(dev, slot, code);
}

LIBEVDEV_EXPORT int
libevdev_set_slot_value(struct libevdev *dev, unsigned int slot, unsigned int code, int value)
{
	if (!libevdev_has_event_type(dev, EV_ABS) || !libevdev_has_event_code(dev, EV_ABS, code))
		return -1;

	if (dev->num_slots == -1 || slot >= (unsigned int)dev->num_slots)
		return -1;

	if (code > ABS_MT_MAX || code < ABS_MT_MIN)
		return -1;

	if (code == ABS_MT_SLOT) {
		if (value < 0 || value >= libevdev_get_num_slots(dev))
			return -1;
		dev->current_slot = value;
	}

	*slot_value(dev, slot, code) = value;

	return 0;
}

LIBEVDEV_EXPORT int
libevdev_fetch_slot_value(const struct libevdev *dev, unsigned int slot, unsigned int code, int *value)
{
	if (libevdev_has_event_type(dev, EV_ABS) &&
	    libevdev_has_event_code(dev, EV_ABS, code) &&
	    dev->num_slots >= 0 &&
	    slot < (unsigned int)dev->num_slots) {
		*value = libevdev_get_slot_value(dev, slot, code);
		return 1;
	}

	return 0;
}

LIBEVDEV_EXPORT int
libevdev_get_num_slots(const struct libevdev *dev)
{
	return dev->num_slots;
}

LIBEVDEV_EXPORT int
libevdev_get_current_slot(const struct libevdev *dev)
{
	return dev->current_slot;
}

LIBEVDEV_EXPORT const struct input_absinfo*
libevdev_get_abs_info(const struct libevdev *dev, unsigned int code)
{
	if (!libevdev_has_event_type(dev, EV_ABS) ||
	    !libevdev_has_event_code(dev, EV_ABS, code))
		return NULL;

	return &dev->abs_info[code];
}

#define ABS_GETTER(name) \
LIBEVDEV_EXPORT int libevdev_get_abs_##name(const struct libevdev *dev, unsigned int code) \
{ \
	const struct input_absinfo *absinfo = libevdev_get_abs_info(dev, code); \
	return absinfo ? absinfo->name : 0; \
}

ABS_GETTER(maximum)
ABS_GETTER(minimum)
ABS_GETTER(fuzz)
ABS_GETTER(flat)
ABS_GETTER(resolution)

#define ABS_SETTER(field) \
LIBEVDEV_EXPORT void libevdev_set_abs_##field(struct libevdev *dev, unsigned int code, int val) \
{ \
	if (!libevdev_has_event_code(dev, EV_ABS, code)) \
		return; \
	dev->abs_info[code].field = val; \
}

ABS_SETTER(maximum)
ABS_SETTER(minimum)
ABS_SETTER(fuzz)
ABS_SETTER(flat)
ABS_SETTER(resolution)

LIBEVDEV_EXPORT void
libevdev_set_abs_info(struct libevdev *dev, unsigned int code, const struct input_absinfo *abs)
{
	if (!libevdev_has_event_code(dev, EV_ABS, code))
		return;

	dev->abs_info[code] = *abs;
}

LIBEVDEV_EXPORT int
libevdev_enable_event_type(struct libevdev *dev, unsigned int type)
{
	int max;

	if (type > EV_MAX)
		return -1;

	if (libevdev_has_event_type(dev, type))
		return 0;

	max = libevdev_event_type_get_max(type);
	if (max == -1)
		return -1;

	set_bit(dev->bits, type);

	if (type == EV_REP) {
		int delay = 0, period = 0;
		libevdev_enable_event_code(dev, EV_REP, REP_DELAY, &delay);
		libevdev_enable_event_code(dev, EV_REP, REP_PERIOD, &period);
	}
	return 0;
}

LIBEVDEV_EXPORT int
libevdev_disable_event_type(struct libevdev *dev, unsigned int type)
{
	int max;

	if (type > EV_MAX || type == EV_SYN)
		return -1;

	max = libevdev_event_type_get_max(type);
	if (max == -1)
		return -1;

	clear_bit(dev->bits, type);

	return 0;
}

LIBEVDEV_EXPORT int
libevdev_enable_event_code(struct libevdev *dev, unsigned int type,
			   unsigned int code, const void *data)
{
	unsigned int max;
	unsigned long *mask = NULL;

	if (libevdev_enable_event_type(dev, type))
		return -1;

	switch(type) {
		case EV_SYN:
			return 0;
		case EV_ABS:
		case EV_REP:
			if (data == NULL)
				return -1;
			break;
		default:
			if (data != NULL)
				return -1;
			break;
	}

	max = type_to_mask(dev, type, &mask);

	if (code > max || (int)max == -1)
		return -1;

	set_bit(mask, code);

	if (type == EV_ABS) {
		const struct input_absinfo *abs = data;
		dev->abs_info[code] = *abs;
		if (code == ABS_MT_SLOT) {
			if (init_slots(dev) != 0)
				return -1;
		} else if (code == ABS_MT_TRACKING_ID) {
			reset_tracking_ids(dev);
		}
	} else if (type == EV_REP) {
		const int *value = data;
		dev->rep_values[code] = *value;
	}

	return 0;
}

LIBEVDEV_EXPORT int
libevdev_disable_event_code(struct libevdev *dev, unsigned int type, unsigned int code)
{
	unsigned int max;
	unsigned long *mask = NULL;

	if (type > EV_MAX || type == EV_SYN)
		return -1;

	max = type_to_mask(dev, type, &mask);

	if (code > max || (int)max == -1)
		return -1;

	clear_bit(mask, code);

	if (type == EV_ABS) {
		if (code == ABS_MT_SLOT) {
			if (init_slots(dev) != 0)
				return -1;
		} else if (code == ABS_MT_TRACKING_ID) {
			reset_tracking_ids(dev);
		}
	}

	return 0;
}

LIBEVDEV_EXPORT int
libevdev_kernel_set_abs_info(struct libevdev *dev, unsigned int code, const struct input_absinfo *abs)
{
	int rc;

	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	}

	if (dev->fd < 0)
		return -EBADF;

	if (code > ABS_MAX)
		return -EINVAL;

	rc = ioctl(dev->fd, EVIOCSABS(code), abs);
	if (rc < 0)
		rc = -errno;
	else
		rc = libevdev_enable_event_code(dev, EV_ABS, code, abs);

	return rc;
}

LIBEVDEV_EXPORT int
libevdev_grab(struct libevdev *dev, enum libevdev_grab_mode grab)
{
	int rc = 0;

	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	}

	if (dev->fd < 0)
		return -EBADF;

	if (grab != LIBEVDEV_GRAB && grab != LIBEVDEV_UNGRAB) {
		log_bug(dev, "invalid grab parameter %#x\n", grab);
		return -EINVAL;
	}

	if (grab == dev->grabbed)
		return 0;

	if (grab == LIBEVDEV_GRAB)
		rc = ioctl(dev->fd, EVIOCGRAB, (void *)1);
	else if (grab == LIBEVDEV_UNGRAB)
		rc = ioctl(dev->fd, EVIOCGRAB, (void *)0);

	if (rc == 0)
		dev->grabbed = grab;

	return rc < 0 ? -errno : 0;
}

LIBEVDEV_EXPORT int
libevdev_event_is_type(const struct input_event *ev, unsigned int type)
{
	return type < EV_CNT && ev->type == type;
}

LIBEVDEV_EXPORT int
libevdev_event_is_code(const struct input_event *ev, unsigned int type, unsigned int code)
{
	int max;

	if (!libevdev_event_is_type(ev, type))
		return 0;

	max = libevdev_event_type_get_max(type);
	return (max > -1 && code <= (unsigned int)max && ev->code == code);
}

LIBEVDEV_EXPORT const char*
libevdev_event_type_get_name(unsigned int type)
{
	if (type > EV_MAX)
		return NULL;

	return ev_map[type];
}

LIBEVDEV_EXPORT const char*
libevdev_event_code_get_name(unsigned int type, unsigned int code)
{
	int max = libevdev_event_type_get_max(type);

	if (max == -1 || code > (unsigned int)max)
		return NULL;

	return event_type_map[type][code];
}

LIBEVDEV_EXPORT const char *
libevdev_event_value_get_name(unsigned int type,
			      unsigned int code,
			      int value)
{
	/* This is a simplified version because nothing else
	   is an enum like ABS_MT_TOOL_TYPE so we don't need
	   a generic lookup */
	if (type != EV_ABS || code != ABS_MT_TOOL_TYPE)
		return NULL;

	if (value < 0 || value > MT_TOOL_MAX)
		return NULL;

	return mt_tool_map[value];
}

LIBEVDEV_EXPORT const char*
libevdev_property_get_name(unsigned int prop)
{
	if (prop > INPUT_PROP_MAX)
		return NULL;

	return input_prop_map[prop];
}

LIBEVDEV_EXPORT int
libevdev_event_type_get_max(unsigned int type)
{
	if (type > EV_MAX)
		return -1;

	return ev_max[type];
}

LIBEVDEV_EXPORT int
libevdev_get_repeat(const struct libevdev *dev, int *delay, int *period)
{
	if (!libevdev_has_event_type(dev, EV_REP))
		return -1;

	if (delay != NULL)
		*delay = dev->rep_values[REP_DELAY];
	if (period != NULL)
		*period = dev->rep_values[REP_PERIOD];

	return 0;
}

LIBEVDEV_EXPORT int
libevdev_kernel_set_led_value(struct libevdev *dev, unsigned int code, enum libevdev_led_value value)
{
	return libevdev_kernel_set_led_values(dev, code, value, -1);
}

LIBEVDEV_EXPORT int
libevdev_kernel_set_led_values(struct libevdev *dev, ...)
{
	struct input_event ev[LED_MAX + 1];
	enum libevdev_led_value val;
	va_list args;
	int code;
	int rc = 0;
	size_t nleds = 0;

	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	}

	if (dev->fd < 0)
		return -EBADF;

	memset(ev, 0, sizeof(ev));

	va_start(args, dev);
	code = va_arg(args, unsigned int);
	while (code != -1) {
		if (code > LED_MAX) {
			rc = -EINVAL;
			break;
		}
		val = va_arg(args, enum libevdev_led_value);
		if (val != LIBEVDEV_LED_ON && val != LIBEVDEV_LED_OFF) {
			rc = -EINVAL;
			break;
		}

		if (libevdev_has_event_code(dev, EV_LED, code)) {
			struct input_event *e = ev;

			while (e->type > 0 && e->code != code)
				e++;

			if (e->type == 0)
				nleds++;
			e->type = EV_LED;
			e->code = code;
			e->value = (val == LIBEVDEV_LED_ON);
		}
		code = va_arg(args, unsigned int);
	}
	va_end(args);

	if (rc == 0 && nleds > 0) {
		ev[nleds].type = EV_SYN;
		ev[nleds++].code = SYN_REPORT;

		rc = write(libevdev_get_fd(dev), ev, nleds * sizeof(ev[0]));
		if (rc > 0) {
			nleds--; /* last is EV_SYN */
			while (nleds--)
				update_led_state(dev, &ev[nleds]);
		}
		rc = (rc != -1) ? 0 : -errno;
	}

	return rc;
}

LIBEVDEV_EXPORT int
libevdev_set_clock_id(struct libevdev *dev, int clockid)
{
	if (!dev->initialized) {
		log_bug(dev, "device not initialized. call libevdev_set_fd() first\n");
		return -EBADF;
	}

	if (dev->fd < 0)
		return -EBADF;

	return ioctl(dev->fd, EVIOCSCLOCKID, &clockid) ? -errno : 0;
}
