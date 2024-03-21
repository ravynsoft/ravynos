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

#ifndef LIBINPUT_PRIVATE_CONFIG_H
#define LIBINPUT_PRIVATE_CONFIG_H

#include "config.h"

#include "libinput.h"

enum libinput_config_hold_state {
	/** Hold gestures are to be disabled, or are currently disabled */
	LIBINPUT_CONFIG_HOLD_DISABLED,
	/** Hold gestures are to be enabled, or are currently disabled */
	LIBINPUT_CONFIG_HOLD_ENABLED,
};

/**
 * @ingroup config
 *
 * Check whether a device can perform hold gestures.
 *
 * @param device The device to configure
 * @return Non-zero if a device can perform hold gestures, zero otherwise.
 *
 * @see libinput_device_config_gesture_set_hold_enabled
 * @see libinput_device_config_gesture_get_hold_enabled
 * @see libinput_device_config_gesture_get_hold_default_enabled
 */
int
libinput_device_config_gesture_hold_is_available(struct libinput_device *device);

/**
 * @ingroup config
 *
 * Enable or disable hold gestures on this device.
 *
 * @param device The device to configure
 * @param enable @ref LIBINPUT_CONFIG_HOLD_ENABLED to enable hold gestures or
 * @ref LIBINPUT_CONFIG_HOLD_DISABLED to disable them
 *
 * @return A config status code. Disabling hold gestures on a device that does
 * not support them always succeeds.
 *
 * @see libinput_device_config_gesture_hold_is_available
 * @see libinput_device_config_gesture_get_hold_enabled
 * @see libinput_device_config_gesture_get_hold_default_enabled
 */
enum libinput_config_status
libinput_device_config_gesture_set_hold_enabled(struct libinput_device *device,
						enum libinput_config_hold_state enable);

/**
 * @ingroup config
 *
 * Check if hold gestures are enabled on this device. If the device does not
 * support hold gestures, this function always returns @ref
 * LIBINPUT_CONFIG_HOLD_DISABLED.
 *
 * @param device The device to configure
 *
 * @retval LIBINPUT_CONFIG_HOLD_ENABLED If hold gestures are currently enabled
 * @retval LIBINPUT_CONFIG_HOLD_DISABLED If hold gestures are currently disabled
 *
 * @see libinput_device_config_gesture_hold_is_available
 * @see libinput_device_config_gesture_set_hold_enabled
 * @see libinput_device_config_gesture_get_hold_default_enabled
 */
enum libinput_config_hold_state
libinput_device_config_gesture_get_hold_enabled(struct libinput_device *device);

/**
 * @ingroup config
 *
 * Return the default setting for whether hold gestures are enabled on this
 * device.
 *
 * @param device The device to configure
 * @retval LIBINPUT_CONFIG_HOLD_ENABLED If hold gestures are enabled by default
 * @retval LIBINPUT_CONFIG_HOLD_DISABLED If hold gestures are disabled by
 * default
 *
 * @see libinput_device_config_gesture_hold_is_available
 * @see libinput_device_config_gesture_set_hold_enabled
 * @see libinput_device_config_gesture_get_hold_enabled
 */
enum libinput_config_hold_state
libinput_device_config_gesture_get_hold_default_enabled(struct libinput_device *device);

#endif /* LIBINPUT_PRIVATE_CONFIG_H */
