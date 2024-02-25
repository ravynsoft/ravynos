/*
 * Copyright © 2014-2015 Red Hat, Inc.
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

#include <stdint.h>

#include "evdev.h"

#define MIDDLEBUTTON_TIMEOUT ms2us(50)

/*****************************************
 * BEFORE YOU EDIT THIS FILE, look at the state diagram in
 * doc/middle-button-emulation-state-machine.svg (generated with
 * https://www.diagrams.net).
 *
 * Any changes in this file must be represented in the diagram.
 *
 * Note in regards to the state machine: it only handles left, right and
 * emulated middle button clicks, all other button events are passed
 * through. When in the PASSTHROUGH state, all events are passed through
 * as-is.
 */

static inline const char*
middlebutton_state_to_str(enum evdev_middlebutton_state state)
{
	switch (state) {
	CASE_RETURN_STRING(MIDDLEBUTTON_IDLE);
	CASE_RETURN_STRING(MIDDLEBUTTON_LEFT_DOWN);
	CASE_RETURN_STRING(MIDDLEBUTTON_RIGHT_DOWN);
	CASE_RETURN_STRING(MIDDLEBUTTON_MIDDLE);
	CASE_RETURN_STRING(MIDDLEBUTTON_LEFT_UP_PENDING);
	CASE_RETURN_STRING(MIDDLEBUTTON_RIGHT_UP_PENDING);
	CASE_RETURN_STRING(MIDDLEBUTTON_PASSTHROUGH);
	CASE_RETURN_STRING(MIDDLEBUTTON_IGNORE_LR);
	CASE_RETURN_STRING(MIDDLEBUTTON_IGNORE_L);
	CASE_RETURN_STRING(MIDDLEBUTTON_IGNORE_R);
	}

	return NULL;
}

static inline const char*
middlebutton_event_to_str(enum evdev_middlebutton_event event)
{
	switch (event) {
	CASE_RETURN_STRING(MIDDLEBUTTON_EVENT_L_DOWN);
	CASE_RETURN_STRING(MIDDLEBUTTON_EVENT_R_DOWN);
	CASE_RETURN_STRING(MIDDLEBUTTON_EVENT_OTHER);
	CASE_RETURN_STRING(MIDDLEBUTTON_EVENT_L_UP);
	CASE_RETURN_STRING(MIDDLEBUTTON_EVENT_R_UP);
	CASE_RETURN_STRING(MIDDLEBUTTON_EVENT_TIMEOUT);
	CASE_RETURN_STRING(MIDDLEBUTTON_EVENT_ALL_UP);
	}

	return NULL;
}

static void
middlebutton_state_error(struct evdev_device *device,
			 enum evdev_middlebutton_event event)
{
	evdev_log_bug_libinput(device,
			       "Invalid event %s in middle btn state %s\n",
			       middlebutton_event_to_str(event),
			       middlebutton_state_to_str(device->middlebutton.state));
}

static void
middlebutton_timer_set(struct evdev_device *device, uint64_t now)
{
	libinput_timer_set(&device->middlebutton.timer,
			   now + MIDDLEBUTTON_TIMEOUT);
}

static void
middlebutton_timer_cancel(struct evdev_device *device)
{
	libinput_timer_cancel(&device->middlebutton.timer);
}

static inline void
middlebutton_set_state(struct evdev_device *device,
		       enum evdev_middlebutton_state state,
		       uint64_t now)
{
	switch (state) {
	case MIDDLEBUTTON_LEFT_DOWN:
	case MIDDLEBUTTON_RIGHT_DOWN:
		middlebutton_timer_set(device, now);
		device->middlebutton.first_event_time = now;
		break;
	case MIDDLEBUTTON_IDLE:
	case MIDDLEBUTTON_MIDDLE:
	case MIDDLEBUTTON_LEFT_UP_PENDING:
	case MIDDLEBUTTON_RIGHT_UP_PENDING:
	case MIDDLEBUTTON_PASSTHROUGH:
	case MIDDLEBUTTON_IGNORE_LR:
	case MIDDLEBUTTON_IGNORE_L:
	case MIDDLEBUTTON_IGNORE_R:
		middlebutton_timer_cancel(device);
		break;
	}

	device->middlebutton.state = state;
}

static void
middlebutton_post_event(struct evdev_device *device,
			uint64_t now,
			int button,
			enum libinput_button_state state)
{
	evdev_pointer_notify_button(device,
				    now,
				    button,
				    state);
}

static int
evdev_middlebutton_idle_handle_event(struct evdev_device *device,
				     uint64_t time,
				     enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
		middlebutton_set_state(device, MIDDLEBUTTON_LEFT_DOWN, time);
		break;
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_set_state(device, MIDDLEBUTTON_RIGHT_DOWN, time);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
	case MIDDLEBUTTON_EVENT_L_UP:
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		break;
	}

	return 1;
}

static int
evdev_middlebutton_ldown_handle_event(struct evdev_device *device,
				      uint64_t time,
				      enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_post_event(device, time,
					BTN_MIDDLE,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device, MIDDLEBUTTON_MIDDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		middlebutton_post_event(device, time,
					BTN_LEFT,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device,
				       MIDDLEBUTTON_PASSTHROUGH,
				       time);
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_L_UP:
		middlebutton_post_event(device,
					device->middlebutton.first_event_time,
					BTN_LEFT,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_post_event(device, time,
					BTN_LEFT,
					LIBINPUT_BUTTON_STATE_RELEASED);
		middlebutton_set_state(device, MIDDLEBUTTON_IDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_post_event(device,
					device->middlebutton.first_event_time,
					BTN_LEFT,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device,
				       MIDDLEBUTTON_PASSTHROUGH,
				       time);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_state_error(device, event);
		break;
	}

	return 1;
}

static int
evdev_middlebutton_rdown_handle_event(struct evdev_device *device,
				      uint64_t time,
				      enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
		middlebutton_post_event(device, time,
					BTN_MIDDLE,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device, MIDDLEBUTTON_MIDDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		middlebutton_post_event(device,
					device->middlebutton.first_event_time,
					BTN_RIGHT,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device,
				       MIDDLEBUTTON_PASSTHROUGH,
				       time);
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
		middlebutton_post_event(device,
					device->middlebutton.first_event_time,
					BTN_RIGHT,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_post_event(device, time,
					BTN_RIGHT,
					LIBINPUT_BUTTON_STATE_RELEASED);
		middlebutton_set_state(device, MIDDLEBUTTON_IDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_L_UP:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_post_event(device,
					device->middlebutton.first_event_time,
					BTN_RIGHT,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device,
				       MIDDLEBUTTON_PASSTHROUGH,
				       time);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_state_error(device, event);
		break;
	}

	return 1;
}

static int
evdev_middlebutton_middle_handle_event(struct evdev_device *device,
				       uint64_t time,
				       enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		middlebutton_post_event(device, time,
					BTN_MIDDLE,
					LIBINPUT_BUTTON_STATE_RELEASED);
		middlebutton_set_state(device, MIDDLEBUTTON_IGNORE_LR, time);
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
		middlebutton_post_event(device, time,
					BTN_MIDDLE,
					LIBINPUT_BUTTON_STATE_RELEASED);
		middlebutton_set_state(device,
				       MIDDLEBUTTON_LEFT_UP_PENDING,
				       time);
		break;
	case MIDDLEBUTTON_EVENT_L_UP:
		middlebutton_post_event(device, time,
					BTN_MIDDLE,
					LIBINPUT_BUTTON_STATE_RELEASED);
		middlebutton_set_state(device,
				       MIDDLEBUTTON_RIGHT_UP_PENDING,
				       time);
		break;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_state_error(device, event);
		break;
	}

	return 1;
}

static int
evdev_middlebutton_lup_pending_handle_event(struct evdev_device *device,
					    uint64_t time,
					    enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_post_event(device, time,
					BTN_MIDDLE,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device, MIDDLEBUTTON_MIDDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		middlebutton_set_state(device, MIDDLEBUTTON_IGNORE_L, time);
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_L_UP:
		middlebutton_set_state(device, MIDDLEBUTTON_IDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_state_error(device, event);
		break;
	}

	return 1;
}

static int
evdev_middlebutton_rup_pending_handle_event(struct evdev_device *device,
					    uint64_t time,
					    enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
		middlebutton_post_event(device, time,
					BTN_MIDDLE,
					LIBINPUT_BUTTON_STATE_PRESSED);
		middlebutton_set_state(device, MIDDLEBUTTON_MIDDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		middlebutton_set_state(device, MIDDLEBUTTON_IGNORE_R, time);
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
		middlebutton_set_state(device, MIDDLEBUTTON_IDLE, time);
		break;
	case MIDDLEBUTTON_EVENT_L_UP:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_state_error(device, event);
		break;
	}

	return 1;
}

static int
evdev_middlebutton_passthrough_handle_event(struct evdev_device *device,
					    uint64_t time,
					    enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
	case MIDDLEBUTTON_EVENT_R_DOWN:
	case MIDDLEBUTTON_EVENT_OTHER:
	case MIDDLEBUTTON_EVENT_R_UP:
	case MIDDLEBUTTON_EVENT_L_UP:
		return 0;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_set_state(device, MIDDLEBUTTON_IDLE, time);
		break;
	}

	return 1;
}

static int
evdev_middlebutton_ignore_lr_handle_event(struct evdev_device *device,
					  uint64_t time,
					  enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
		middlebutton_set_state(device, MIDDLEBUTTON_IGNORE_L, time);
		break;
	case MIDDLEBUTTON_EVENT_L_UP:
		middlebutton_set_state(device, MIDDLEBUTTON_IGNORE_R, time);
		break;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_state_error(device, event);
		break;
	}

	return 1;
}

static int
evdev_middlebutton_ignore_l_handle_event(struct evdev_device *device,
					 uint64_t time,
					 enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_R_DOWN:
		return 0;
	case MIDDLEBUTTON_EVENT_OTHER:
	case MIDDLEBUTTON_EVENT_R_UP:
		return 0;
	case MIDDLEBUTTON_EVENT_L_UP:
		middlebutton_set_state(device,
				       MIDDLEBUTTON_PASSTHROUGH,
				       time);
		break;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
	case MIDDLEBUTTON_EVENT_ALL_UP:
		middlebutton_state_error(device, event);
		break;
	}

	return 1;
}
static int
evdev_middlebutton_ignore_r_handle_event(struct evdev_device *device,
					 uint64_t time,
					 enum evdev_middlebutton_event event)
{
	switch (event) {
	case MIDDLEBUTTON_EVENT_L_DOWN:
		return 0;
	case MIDDLEBUTTON_EVENT_R_DOWN:
		middlebutton_state_error(device, event);
		break;
	case MIDDLEBUTTON_EVENT_OTHER:
		return 0;
	case MIDDLEBUTTON_EVENT_R_UP:
		middlebutton_set_state(device,
				       MIDDLEBUTTON_PASSTHROUGH,
				       time);
		break;
	case MIDDLEBUTTON_EVENT_L_UP:
		return 0;
	case MIDDLEBUTTON_EVENT_TIMEOUT:
	case MIDDLEBUTTON_EVENT_ALL_UP:
		break;
	}

	return 1;
}

static int
evdev_middlebutton_handle_event(struct evdev_device *device,
				uint64_t time,
				enum evdev_middlebutton_event event)
{
	int rc = 0;
	enum evdev_middlebutton_state current;

	current = device->middlebutton.state;

	switch (current) {
	case MIDDLEBUTTON_IDLE:
		rc = evdev_middlebutton_idle_handle_event(device, time, event);
		break;
	case MIDDLEBUTTON_LEFT_DOWN:
		rc = evdev_middlebutton_ldown_handle_event(device, time, event);
		break;
	case MIDDLEBUTTON_RIGHT_DOWN:
		rc = evdev_middlebutton_rdown_handle_event(device, time, event);
		break;
	case MIDDLEBUTTON_MIDDLE:
		rc = evdev_middlebutton_middle_handle_event(device, time, event);
		break;
	case MIDDLEBUTTON_LEFT_UP_PENDING:
		rc = evdev_middlebutton_lup_pending_handle_event(device,
								 time,
								 event);
		break;
	case MIDDLEBUTTON_RIGHT_UP_PENDING:
		rc = evdev_middlebutton_rup_pending_handle_event(device,
								 time,
								 event);
		break;
	case MIDDLEBUTTON_PASSTHROUGH:
		rc = evdev_middlebutton_passthrough_handle_event(device,
								 time,
								 event);
		break;
	case MIDDLEBUTTON_IGNORE_LR:
		rc = evdev_middlebutton_ignore_lr_handle_event(device,
							       time,
							       event);
		break;
	case MIDDLEBUTTON_IGNORE_L:
		rc = evdev_middlebutton_ignore_l_handle_event(device,
							      time,
							      event);
		break;
	case MIDDLEBUTTON_IGNORE_R:
		rc = evdev_middlebutton_ignore_r_handle_event(device,
							      time,
							      event);
		break;
	default:
		evdev_log_bug_libinput(device,
				       "Invalid middle button state %d\n",
				       current);
		break;
	}

	evdev_log_debug(device,
			"middlebutton state: %s → %s → %s, rc %d\n",
			middlebutton_state_to_str(current),
			middlebutton_event_to_str(event),
			middlebutton_state_to_str(device->middlebutton.state),
			rc);

	return rc;
}

static inline void
evdev_middlebutton_apply_config(struct evdev_device *device)
{
	if (device->middlebutton.want_enabled ==
	    device->middlebutton.enabled)
		return;

	if (device->middlebutton.button_mask != 0)
		return;

	device->middlebutton.enabled = device->middlebutton.want_enabled;
}

bool
evdev_middlebutton_filter_button(struct evdev_device *device,
				 uint64_t time,
				 int button,
				 enum libinput_button_state state)
{
	enum evdev_middlebutton_event event;
	bool is_press = state == LIBINPUT_BUTTON_STATE_PRESSED;
	int rc;
	unsigned int btnbit = (button - BTN_LEFT);
	uint32_t old_mask = 0;

	if (!device->middlebutton.enabled)
		return false;

	switch (button) {
	case BTN_LEFT:
		if (is_press)
			event = MIDDLEBUTTON_EVENT_L_DOWN;
		else
			event = MIDDLEBUTTON_EVENT_L_UP;
		break;
	case BTN_RIGHT:
		if (is_press)
			event = MIDDLEBUTTON_EVENT_R_DOWN;
		else
			event = MIDDLEBUTTON_EVENT_R_UP;
		break;

	/* BTN_MIDDLE counts as "other" and resets middle button
	 * emulation */
	case BTN_MIDDLE:
	default:
		event = MIDDLEBUTTON_EVENT_OTHER;
		break;
	}

	if (button < BTN_LEFT ||
	    btnbit >= sizeof(device->middlebutton.button_mask) * 8) {
		evdev_log_bug_libinput(device,
				       "Button mask too small for %s\n",
				       libevdev_event_code_get_name(EV_KEY,
								    button));
		return true;
	}

	rc = evdev_middlebutton_handle_event(device, time, event);

	old_mask = device->middlebutton.button_mask;
	if (is_press)
		device->middlebutton.button_mask |= bit(btnbit);
	else
		device->middlebutton.button_mask &= ~bit(btnbit);

	if (old_mask != device->middlebutton.button_mask &&
	    device->middlebutton.button_mask == 0) {
		evdev_middlebutton_handle_event(device,
						time,
						MIDDLEBUTTON_EVENT_ALL_UP);
		evdev_middlebutton_apply_config(device);
	}

	return rc;
}

static void
evdev_middlebutton_handle_timeout(uint64_t now, void *data)
{
	struct evdev_device *device = evdev_device(data);

	evdev_middlebutton_handle_event(device, now, MIDDLEBUTTON_EVENT_TIMEOUT);
}

int
evdev_middlebutton_is_available(struct libinput_device *device)
{
	return 1;
}

static enum libinput_config_status
evdev_middlebutton_set(struct libinput_device *device,
		       enum libinput_config_middle_emulation_state enable)
{
	struct evdev_device *evdev = evdev_device(device);

	switch (enable) {
	case LIBINPUT_CONFIG_MIDDLE_EMULATION_ENABLED:
		evdev->middlebutton.want_enabled = true;
		break;
	case LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED:
		evdev->middlebutton.want_enabled = false;
		break;
	default:
		return LIBINPUT_CONFIG_STATUS_INVALID;
	}

	evdev_middlebutton_apply_config(evdev);

	return LIBINPUT_CONFIG_STATUS_SUCCESS;
}

enum libinput_config_middle_emulation_state
evdev_middlebutton_get(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);

	return evdev->middlebutton.want_enabled ?
			LIBINPUT_CONFIG_MIDDLE_EMULATION_ENABLED :
			LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED;
}

enum libinput_config_middle_emulation_state
evdev_middlebutton_get_default(struct libinput_device *device)
{
	struct evdev_device *evdev = evdev_device(device);

	return evdev->middlebutton.enabled_default ?
			LIBINPUT_CONFIG_MIDDLE_EMULATION_ENABLED :
			LIBINPUT_CONFIG_MIDDLE_EMULATION_DISABLED;
}

void
evdev_init_middlebutton(struct evdev_device *device,
			bool enable,
			bool want_config)
{
	char timer_name[64];

	snprintf(timer_name,
		 sizeof(timer_name),
		 "%s middlebutton",
		 evdev_device_get_sysname(device));
	libinput_timer_init(&device->middlebutton.timer,
			    evdev_libinput_context(device),
			    timer_name,
			    evdev_middlebutton_handle_timeout,
			    device);
	device->middlebutton.enabled_default = enable;
	device->middlebutton.want_enabled = enable;
	device->middlebutton.enabled = enable;

	if (!want_config)
		return;

	device->middlebutton.config.available = evdev_middlebutton_is_available;
	device->middlebutton.config.set = evdev_middlebutton_set;
	device->middlebutton.config.get = evdev_middlebutton_get;
	device->middlebutton.config.get_default = evdev_middlebutton_get_default;
	device->base.config.middle_emulation = &device->middlebutton.config;
}
