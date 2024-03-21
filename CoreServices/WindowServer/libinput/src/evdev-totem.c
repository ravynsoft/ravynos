/*
 * Copyright © 2018 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"
#include "evdev.h"

enum totem_slot_state {
	SLOT_STATE_NONE,
	SLOT_STATE_BEGIN,
	SLOT_STATE_UPDATE,
	SLOT_STATE_END,
};

struct totem_slot {
	bool dirty;
	unsigned int index;
	enum totem_slot_state state;
	struct libinput_tablet_tool *tool;
	struct tablet_axes axes;
	unsigned char changed_axes[NCHARS(LIBINPUT_TABLET_TOOL_AXIS_MAX + 1)];

	struct device_coords last_point;
};

struct totem_dispatch {
	struct evdev_dispatch base;
	struct evdev_device *device;

	int slot; /* current slot */
	struct totem_slot *slots;
	size_t nslots;

	struct evdev_device *touch_device;

	/* We only have one button */
	bool button_state_now;
	bool button_state_previous;

	enum evdev_arbitration_state arbitration_state;
};

static inline struct totem_dispatch*
totem_dispatch(struct evdev_dispatch *totem)
{
	evdev_verify_dispatch_type(totem, DISPATCH_TOTEM);

	return container_of(totem, struct totem_dispatch, base);
}

static inline struct libinput *
totem_libinput_context(const struct totem_dispatch *totem)
{
	return evdev_libinput_context(totem->device);
}

static struct libinput_tablet_tool *
totem_new_tool(struct totem_dispatch *totem)
{
	struct libinput *libinput = totem_libinput_context(totem);
	struct libinput_tablet_tool *tool;

	tool = zalloc(sizeof *tool);

	*tool = (struct libinput_tablet_tool) {
		.type = LIBINPUT_TABLET_TOOL_TYPE_TOTEM,
		.serial = 0,
		.tool_id = 0,
		.refcount = 1,
	};

	tool->pressure.offset = 0;
	tool->pressure.has_offset = false;
	tool->pressure.threshold.lower = 0;
	tool->pressure.threshold.upper = 1;

	set_bit(tool->axis_caps, LIBINPUT_TABLET_TOOL_AXIS_X);
	set_bit(tool->axis_caps, LIBINPUT_TABLET_TOOL_AXIS_Y);
	set_bit(tool->axis_caps, LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
	set_bit(tool->axis_caps, LIBINPUT_TABLET_TOOL_AXIS_SIZE_MAJOR);
	set_bit(tool->axis_caps, LIBINPUT_TABLET_TOOL_AXIS_SIZE_MINOR);
	set_bit(tool->buttons, BTN_0);

	list_insert(&libinput->tool_list, &tool->link);

	return tool;
}

static inline void
totem_set_touch_device_enabled(struct totem_dispatch *totem,
			       bool enable_touch_device,
			       uint64_t time)
{
	struct evdev_device *touch_device = totem->touch_device;
	struct evdev_dispatch *dispatch;
	struct phys_rect r, *rect = NULL;
	enum evdev_arbitration_state state = ARBITRATION_NOT_ACTIVE;

	if (touch_device == NULL)
		return;

	/* We just pick the coordinates of the first touch we find. The
	 * totem only does one tool right now despite being nominally an MT
	 * device, so let's not go too hard on ourselves*/
	for (size_t i = 0; !enable_touch_device && i < totem->nslots; i++) {
		struct totem_slot *slot = &totem->slots[i];
		struct phys_coords mm;

		if (slot->state == SLOT_STATE_NONE)
			continue;

		/* Totem size is ~70mm. We could calculate the real size but
		   until we need that, hardcoding it is enough */
		mm = evdev_device_units_to_mm(totem->device, &slot->axes.point);
		r.x = mm.x - 30;
		r.y = mm.y - 30;
		r.w = 100;
		r.h = 100;

		rect = &r;

		state = ARBITRATION_IGNORE_RECT;
		break;
	}

	dispatch = touch_device->dispatch;

	if (enable_touch_device) {
	    if (dispatch->interface->touch_arbitration_toggle)
		dispatch->interface->touch_arbitration_toggle(dispatch,
							      touch_device,
							      state,
							      rect,
							      time);
	} else {
		switch (totem->arbitration_state) {
		case ARBITRATION_IGNORE_ALL:
			abort();
		case ARBITRATION_NOT_ACTIVE:
			if (dispatch->interface->touch_arbitration_toggle)
				dispatch->interface->touch_arbitration_toggle(dispatch,
									      touch_device,
									      state,
									      rect,
									      time);
			break;
		case ARBITRATION_IGNORE_RECT:
			if (dispatch->interface->touch_arbitration_update_rect)
				dispatch->interface->touch_arbitration_update_rect(dispatch,
										   touch_device,
										   rect,
										   time);
			break;
		}
	}
	totem->arbitration_state = state;
}

static void
totem_process_key(struct totem_dispatch *totem,
		  struct evdev_device *device,
		  struct input_event *e,
		  uint64_t time)
{
	/* ignore kernel key repeat */
	if (e->value == 2)
		return;

	switch(e->code) {
	case BTN_0:
		totem->button_state_now = !!e->value;
		break;
	default:
		evdev_log_info(device,
			       "Unhandled KEY event code %#x\n",
			       e->code);
		break;
	}
}

static void
totem_process_abs(struct totem_dispatch *totem,
		  struct evdev_device *device,
		  struct input_event *e,
		  uint64_t time)
{
	struct totem_slot *slot = &totem->slots[totem->slot];

	switch(e->code) {
	case ABS_MT_SLOT:
		if ((size_t)e->value >= totem->nslots) {
			evdev_log_bug_libinput(device,
					       "exceeded slot count (%d of max %zd)\n",
					       e->value,
					       totem->nslots);
			e->value = totem->nslots - 1;
		}
		totem->slot = e->value;
		return;
	case ABS_MT_TRACKING_ID:
		/* If the totem is already down on init, we currently
		   ignore it */
		if (e->value >= 0)
			slot->state = SLOT_STATE_BEGIN;
		else if (slot->state != SLOT_STATE_NONE)
			slot->state = SLOT_STATE_END;
		break;
	case ABS_MT_POSITION_X:
		set_bit(slot->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_X);
		break;
	case ABS_MT_POSITION_Y:
		set_bit(slot->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_Y);
		break;
	case ABS_MT_TOUCH_MAJOR:
		set_bit(slot->changed_axes,
			LIBINPUT_TABLET_TOOL_AXIS_SIZE_MAJOR);
		break;
	case ABS_MT_TOUCH_MINOR:
		set_bit(slot->changed_axes,
			LIBINPUT_TABLET_TOOL_AXIS_SIZE_MINOR);
		break;
	case ABS_MT_ORIENTATION:
		set_bit(slot->changed_axes,
			LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z);
		break;
	case ABS_MT_TOOL_TYPE:
		if (e->value != MT_TOOL_DIAL) {
			evdev_log_info(device,
				       "Unexpected tool type %#x, changing to dial\n",
				       e->code);
		}
		break;
	default:
		evdev_log_info(device,
			       "Unhandled ABS event code %#x\n",
			       e->code);
		break;
	}
}

static bool
totem_slot_fetch_axes(struct totem_dispatch *totem,
		      struct totem_slot *slot,
		      struct libinput_tablet_tool *tool,
		      struct tablet_axes *axes_out,
		      uint64_t time)
{
	struct evdev_device *device = totem->device;
	const char tmp[sizeof(slot->changed_axes)] = {0};
	struct tablet_axes axes = {0};
	struct device_float_coords delta;
	bool rc = false;

	if (memcmp(tmp, slot->changed_axes, sizeof(tmp)) == 0) {
		axes = slot->axes;
		goto out;
	}

	if (bit_is_set(slot->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_X) ||
	    bit_is_set(slot->changed_axes, LIBINPUT_TABLET_TOOL_AXIS_Y)) {
		slot->axes.point.x = libevdev_get_slot_value(device->evdev,
							     slot->index,
							     ABS_MT_POSITION_X);
		slot->axes.point.y = libevdev_get_slot_value(device->evdev,
							     slot->index,
							     ABS_MT_POSITION_Y);
	}

	if (bit_is_set(slot->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_ROTATION_Z)) {
		int angle = libevdev_get_slot_value(device->evdev,
						    slot->index,
						    ABS_MT_ORIENTATION);
		/* The kernel gives us ±90 degrees off neutral */
		slot->axes.rotation = (360 - angle) % 360;
	}

	if (bit_is_set(slot->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_SIZE_MAJOR) ||
	    bit_is_set(slot->changed_axes,
		       LIBINPUT_TABLET_TOOL_AXIS_SIZE_MINOR)) {
		int major, minor;
		unsigned int rmajor, rminor;

		major = libevdev_get_slot_value(device->evdev,
						slot->index,
						ABS_MT_TOUCH_MAJOR);
		minor = libevdev_get_slot_value(device->evdev,
						slot->index,
						ABS_MT_TOUCH_MINOR);
		rmajor = libevdev_get_abs_resolution(device->evdev, ABS_MT_TOUCH_MAJOR);
		rminor = libevdev_get_abs_resolution(device->evdev, ABS_MT_TOUCH_MINOR);
		slot->axes.size.major = (double)major/rmajor;
		slot->axes.size.minor = (double)minor/rminor;
	}

	axes.point = slot->axes.point;
	axes.rotation = slot->axes.rotation;
	axes.size = slot->axes.size;

	delta.x = slot->axes.point.x - slot->last_point.x;
	delta.y = slot->axes.point.y - slot->last_point.y;
	axes.delta = filter_dispatch(device->pointer.filter, &delta, tool, time);

	rc = true;
out:
	*axes_out = axes;
	return rc;

}

static void
totem_slot_mark_all_axes_changed(struct totem_dispatch *totem,
			    struct totem_slot *slot,
			    struct libinput_tablet_tool *tool)
{
	static_assert(sizeof(slot->changed_axes) ==
			      sizeof(tool->axis_caps),
		      "Mismatching array sizes");

	memcpy(slot->changed_axes,
	       tool->axis_caps,
	       sizeof(slot->changed_axes));
}

static inline void
totem_slot_reset_changed_axes(struct totem_dispatch *totem,
			      struct totem_slot *slot)
{
	memset(slot->changed_axes, 0, sizeof(slot->changed_axes));
}

static inline void
slot_axes_initialize(struct totem_dispatch *totem,
		     struct totem_slot *slot)
{
	struct evdev_device *device = totem->device;

	slot->axes.point.x = libevdev_get_slot_value(device->evdev,
						     slot->index,
						     ABS_MT_POSITION_X);
	slot->axes.point.y = libevdev_get_slot_value(device->evdev,
						     slot->index,
						     ABS_MT_POSITION_Y);
	slot->last_point.x = slot->axes.point.x;
	slot->last_point.y = slot->axes.point.y;
}

static enum totem_slot_state
totem_handle_slot_state(struct totem_dispatch *totem,
			struct totem_slot *slot,
			uint64_t time)
{
	struct evdev_device *device = totem->device;
	struct tablet_axes axes;
	enum libinput_tablet_tool_tip_state tip_state;
	bool updated;

	switch (slot->state) {
	case SLOT_STATE_BEGIN:
		if (!slot->tool)
			slot->tool = totem_new_tool(totem);
		slot_axes_initialize(totem, slot);
		totem_slot_mark_all_axes_changed(totem, slot, slot->tool);
		break;
	case SLOT_STATE_UPDATE:
	case SLOT_STATE_END:
		assert(slot->tool);
		break;
	case SLOT_STATE_NONE:
		return SLOT_STATE_NONE;
	}

	tip_state = LIBINPUT_TABLET_TOOL_TIP_UP;
	updated = totem_slot_fetch_axes(totem, slot, slot->tool, &axes, time);

	switch (slot->state) {
	case SLOT_STATE_BEGIN:
		tip_state = LIBINPUT_TABLET_TOOL_TIP_DOWN;
		tablet_notify_proximity(&device->base,
					time,
					slot->tool,
					LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN,
					slot->changed_axes,
					&axes);
		totem_slot_reset_changed_axes(totem, slot);
		tablet_notify_tip(&device->base,
				  time,
				  slot->tool,
				  tip_state,
				  slot->changed_axes,
				  &axes);
		slot->state = SLOT_STATE_UPDATE;
		break;
	case SLOT_STATE_UPDATE:
		tip_state = LIBINPUT_TABLET_TOOL_TIP_DOWN;
		if (updated) {
			tablet_notify_axis(&device->base,
					   time,
					   slot->tool,
					   tip_state,
					   slot->changed_axes,
					   &axes);
		}
		break;
	case SLOT_STATE_END:
		/* prox out is handled after button events */
		break;
	case SLOT_STATE_NONE:
		abort();
		break;
	}

	/* We only have one button but possibly multiple totems. It's not
	 * clear how the firmware will work, so for now we just handle the
	 * button state in the first slot.
	 *
	 * Due to the design of the totem we're also less fancy about
	 * button handling than the tablet code. Worst case, you might get
	 * tip up before button up but meh.
	 */
	if (totem->button_state_now != totem->button_state_previous) {
		enum libinput_button_state btn_state;

		if (totem->button_state_now)
			btn_state = LIBINPUT_BUTTON_STATE_PRESSED;
		else
			btn_state = LIBINPUT_BUTTON_STATE_RELEASED;

		tablet_notify_button(&device->base,
				     time,
				     slot->tool,
				     tip_state,
				     &axes,
				     BTN_0,
				     btn_state);

		totem->button_state_previous = totem->button_state_now;
	}

	switch(slot->state) {
	case SLOT_STATE_BEGIN:
	case SLOT_STATE_UPDATE:
		break;
	case SLOT_STATE_END:
		tip_state = LIBINPUT_TABLET_TOOL_TIP_UP;
		tablet_notify_tip(&device->base,
				  time,
				  slot->tool,
				  tip_state,
				  slot->changed_axes,
				  &axes);
		totem_slot_reset_changed_axes(totem, slot);
		tablet_notify_proximity(&device->base,
					time,
					slot->tool,
					LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT,
					slot->changed_axes,
					&axes);
		slot->state = SLOT_STATE_NONE;
		break;
	case SLOT_STATE_NONE:
		abort();
		break;
	}

	slot->last_point = slot->axes.point;
	totem_slot_reset_changed_axes(totem, slot);

	return slot->state;
}

static enum totem_slot_state
totem_handle_state(struct totem_dispatch *totem,
		   uint64_t time)
{
	enum totem_slot_state global_state = SLOT_STATE_NONE;

	for (size_t i = 0; i < totem->nslots; i++) {
		enum totem_slot_state s;

		s = totem_handle_slot_state(totem,
					    &totem->slots[i],
					    time);

		/* If one slot is active, the totem is active */
		if (s != SLOT_STATE_NONE)
			global_state = SLOT_STATE_UPDATE;
	}

	return global_state;
}

static void
totem_interface_process(struct evdev_dispatch *dispatch,
			struct evdev_device *device,
			struct input_event *e,
			uint64_t time)
{
	struct totem_dispatch *totem = totem_dispatch(dispatch);
	enum totem_slot_state global_state;
	bool enable_touch;

	switch(e->type) {
	case EV_ABS:
		totem_process_abs(totem, device, e, time);
		break;
	case EV_KEY:
		totem_process_key(totem, device, e, time);
		break;
	case EV_MSC:
		/* timestamp, ignore */
		break;
	case EV_SYN:
		global_state = totem_handle_state(totem, time);
		enable_touch = (global_state == SLOT_STATE_NONE);
		totem_set_touch_device_enabled(totem,
					       enable_touch,
					       time);
		break;
	default:
		evdev_log_error(device,
				"Unexpected event type %s (%#x)\n",
				libevdev_event_type_get_name(e->type),
				e->type);
		break;
	}
}

static void
totem_interface_suspend(struct evdev_dispatch *dispatch,
			struct evdev_device *device)
{
	struct totem_dispatch *totem = totem_dispatch(dispatch);
	uint64_t now = libinput_now(evdev_libinput_context(device));

	for (size_t i = 0; i < totem->nslots; i++) {
		struct totem_slot *slot = &totem->slots[i];
		struct tablet_axes axes;
		enum libinput_tablet_tool_tip_state tip_state;

		/* If we never initialized a tool, we can skip everything */
		if (!slot->tool)
			continue;

		totem_slot_fetch_axes(totem, slot, slot->tool, &axes, now);
		totem_slot_reset_changed_axes(totem, slot);

		if (slot->state == SLOT_STATE_NONE)
			tip_state = LIBINPUT_TABLET_TOOL_TIP_UP;
		else
			tip_state = LIBINPUT_TABLET_TOOL_TIP_DOWN;

		if (totem->button_state_now) {
			tablet_notify_button(&device->base,
					     now,
					     slot->tool,
					     tip_state,
					     &axes,
					     BTN_0,
					     LIBINPUT_BUTTON_STATE_RELEASED);

			totem->button_state_now = false;
			totem->button_state_previous = false;
		}

		if (slot->state != SLOT_STATE_NONE) {
			tablet_notify_tip(&device->base,
					  now,
					  slot->tool,
					  LIBINPUT_TABLET_TOOL_TIP_UP,
					  slot->changed_axes,
					  &axes);
		}
		tablet_notify_proximity(&device->base,
					now,
					slot->tool,
					LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT,
					slot->changed_axes,
					&axes);
	}
	totem_set_touch_device_enabled(totem, true, now);
}

static void
totem_interface_destroy(struct evdev_dispatch *dispatch)
{
	struct totem_dispatch *totem = totem_dispatch(dispatch);

	free(totem->slots);
	free(totem);
}

static void
totem_interface_device_added(struct evdev_device *device,
			     struct evdev_device *added_device)
{
	struct totem_dispatch *totem = totem_dispatch(device->dispatch);
	struct libinput_device_group *g1, *g2;

	if ((evdev_device_get_id_vendor(added_device) !=
	    evdev_device_get_id_vendor(device)) ||
	    (evdev_device_get_id_product(added_device) !=
	     evdev_device_get_id_product(device)))
	    return;

	/* virtual devices don't have device groups, so check for that
	   libinput replay */
	g1 = libinput_device_get_device_group(&device->base);
	g2 = libinput_device_get_device_group(&added_device->base);
	if (g1 && g2 && g1->identifier != g2->identifier)
		return;

	if (totem->touch_device != NULL) {
		evdev_log_bug_libinput(device,
				       "already has a paired touch device, ignoring (%s)\n",
				       added_device->devname);
		return;
	}

	totem->touch_device = added_device;
	evdev_log_info(device, "%s: is the totem touch device\n", added_device->devname);
}

static void
totem_interface_device_removed(struct evdev_device *device,
			       struct evdev_device *removed_device)
{
	struct totem_dispatch *totem = totem_dispatch(device->dispatch);

	if (totem->touch_device != removed_device)
		return;

	totem_set_touch_device_enabled(totem, true,
				       libinput_now(evdev_libinput_context(device)));
	totem->touch_device = NULL;
}

static void
totem_interface_initial_proximity(struct evdev_device *device,
				  struct evdev_dispatch *dispatch)
{
	struct totem_dispatch *totem = totem_dispatch(dispatch);
	uint64_t now = libinput_now(evdev_libinput_context(device));
	bool enable_touch = true;

	for (size_t i = 0; i < totem->nslots; i++) {
		struct totem_slot *slot = &totem->slots[i];
		struct tablet_axes axes;
		int tracking_id;

		tracking_id = libevdev_get_slot_value(device->evdev,
						      i,
						      ABS_MT_TRACKING_ID);
		if (tracking_id == -1)
			continue;

		slot->tool = totem_new_tool(totem);
		slot_axes_initialize(totem, slot);
		totem_slot_mark_all_axes_changed(totem, slot, slot->tool);
		totem_slot_fetch_axes(totem, slot, slot->tool, &axes, now);
		tablet_notify_proximity(&device->base,
					now,
					slot->tool,
					LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN,
					slot->changed_axes,
					&axes);
		totem_slot_reset_changed_axes(totem, slot);
		tablet_notify_tip(&device->base,
				  now,
				  slot->tool,
				  LIBINPUT_TABLET_TOOL_TIP_DOWN,
				  slot->changed_axes,
				  &axes);
		slot->state = SLOT_STATE_UPDATE;
		enable_touch = false;
	}

	totem_set_touch_device_enabled(totem, enable_touch, now);
}

struct evdev_dispatch_interface totem_interface = {
	.process = totem_interface_process,
	.suspend = totem_interface_suspend,
	.remove = NULL,
	.destroy = totem_interface_destroy,
	.device_added = totem_interface_device_added,
	.device_removed = totem_interface_device_removed,
	.device_suspended = totem_interface_device_removed, /* treat as remove */
	.device_resumed = totem_interface_device_added, /* treat as add */
	.post_added = totem_interface_initial_proximity,
	.touch_arbitration_toggle = NULL,
	.touch_arbitration_update_rect = NULL,
	.get_switch_state = NULL,
};

static bool
totem_reject_device(struct evdev_device *device)
{
	struct libevdev *evdev = device->evdev;
	bool has_xy, has_slot, has_tool_dial, has_size, has_touch_size;
	double w, h;

	has_xy = libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_X) &&
	         libevdev_has_event_code(evdev, EV_ABS, ABS_MT_POSITION_Y);
	has_slot = libevdev_has_event_code(evdev, EV_ABS, ABS_MT_SLOT);
	has_tool_dial = libevdev_has_event_code(evdev, EV_ABS, ABS_MT_TOOL_TYPE) &&
			libevdev_get_abs_maximum(evdev, ABS_MT_TOOL_TYPE) >= MT_TOOL_DIAL;
	has_size = evdev_device_get_size(device, &w, &h) == 0;
	has_touch_size =
		libevdev_get_abs_resolution(device->evdev, ABS_MT_TOUCH_MAJOR) > 0 ||
		libevdev_get_abs_resolution(device->evdev, ABS_MT_TOUCH_MINOR) > 0;

	if (has_xy && has_slot && has_tool_dial && has_size && has_touch_size)
		return false;

	evdev_log_bug_libinput(device,
			       "missing totem capabilities:%s%s%s%s%s. "
			       "Ignoring this device.\n",
			       has_xy ? "" : " xy",
			       has_slot ? "" : " slot",
			       has_tool_dial ? "" : " dial",
			       has_size ? "" : " resolutions",
			       has_touch_size ? "" : " touch-size");
	return true;
}

static uint32_t
totem_accel_config_get_profiles(struct libinput_device *libinput_device)
{
	return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;
}

static enum libinput_config_status
totem_accel_config_set_profile(struct libinput_device *libinput_device,
			    enum libinput_config_accel_profile profile)
{
	return LIBINPUT_CONFIG_STATUS_UNSUPPORTED;
}

static enum libinput_config_accel_profile
totem_accel_config_get_profile(struct libinput_device *libinput_device)
{
	return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;
}

static enum libinput_config_accel_profile
totem_accel_config_get_default_profile(struct libinput_device *libinput_device)
{
	return LIBINPUT_CONFIG_ACCEL_PROFILE_NONE;
}

static int
totem_init_accel(struct totem_dispatch *totem, struct evdev_device *device)
{
	const struct input_absinfo *x, *y;
	struct motion_filter *filter;

	x = device->abs.absinfo_x;
	y = device->abs.absinfo_y;

	/* same filter as the tablet */
	filter = create_pointer_accelerator_filter_tablet(x->resolution,
							  y->resolution);
	if (!filter)
		return -1;

	evdev_device_init_pointer_acceleration(device, filter);

	/* we override the profile hooks for accel configuration with hooks
	 * that don't allow selection of profiles */
	device->pointer.config.get_profiles = totem_accel_config_get_profiles;
	device->pointer.config.set_profile = totem_accel_config_set_profile;
	device->pointer.config.get_profile = totem_accel_config_get_profile;
	device->pointer.config.get_default_profile = totem_accel_config_get_default_profile;

	return 0;
}

struct evdev_dispatch *
evdev_totem_create(struct evdev_device *device)
{
	struct totem_dispatch *totem;
	struct totem_slot *slots;
	int num_slots;

	if (totem_reject_device(device))
		return NULL;

	totem = zalloc(sizeof *totem);
	totem->device = device;
	totem->base.dispatch_type = DISPATCH_TOTEM;
	totem->base.interface = &totem_interface;

	num_slots = libevdev_get_num_slots(device->evdev);
	if (num_slots <= 0)
		goto error;

	totem->slot = libevdev_get_current_slot(device->evdev);
	slots = zalloc(num_slots * sizeof(*totem->slots));

	for (int slot = 0; slot < num_slots; ++slot) {
		slots[slot].index = slot;
	}

	totem->slots = slots;
	totem->nslots = num_slots;

	evdev_init_sendevents(device, &totem->base);
	totem_init_accel(totem, device);

	return &totem->base;
error:
	totem_interface_destroy(&totem->base);
	return NULL;
}
