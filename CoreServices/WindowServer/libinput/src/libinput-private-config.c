/*
 * Copyright © 2021 Red Hat, Inc.
 * Copyright © 2021 José Expósito
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

#include "libinput-private-config.h"
#include "libinput-private.h"

int
libinput_device_config_gesture_hold_is_available(struct libinput_device *device)
{
	if (!libinput_device_has_capability(device,
					    LIBINPUT_DEVICE_CAP_GESTURE))
		return 0;

	if (!device->config.gesture->get_hold_default(device))
		return 0;

	return 1;
}

enum libinput_config_status
libinput_device_config_gesture_set_hold_enabled(struct libinput_device *device,
						enum libinput_config_hold_state enable)
{
	if (enable != LIBINPUT_CONFIG_HOLD_ENABLED &&
	    enable != LIBINPUT_CONFIG_HOLD_DISABLED)
		return LIBINPUT_CONFIG_STATUS_INVALID;

	if (!libinput_device_config_gesture_hold_is_available(device)) {
		return enable ? LIBINPUT_CONFIG_STATUS_UNSUPPORTED :
				LIBINPUT_CONFIG_STATUS_SUCCESS;
	}

	return device->config.gesture->set_hold_enabled(device, enable);
}

enum libinput_config_hold_state
libinput_device_config_gesture_get_hold_enabled(struct libinput_device *device)
{
	if (!libinput_device_config_gesture_hold_is_available(device))
		return LIBINPUT_CONFIG_HOLD_DISABLED;

	return device->config.gesture->get_hold_enabled(device);
}

enum libinput_config_hold_state
libinput_device_config_gesture_get_hold_default_enabled(struct libinput_device *device)
{
	if (!libinput_device_config_gesture_hold_is_available(device))
		return LIBINPUT_CONFIG_HOLD_DISABLED;

	return device->config.gesture->get_hold_default(device);
}
