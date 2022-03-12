#ifndef INTERFACES_INPUT_DEVICE_H
#define INTERFACES_INPUT_DEVICE_H

#include <wlr/types/wlr_input_device.h>

/**
 * Initializes a given wlr_input_device. Allocates memory for the name and sets
 * its vendor and product id to 0.
 * wlr_device must be non-NULL.
 */
void wlr_input_device_init(struct wlr_input_device *wlr_device,
	enum wlr_input_device_type type, const char *name);

/**
 * Cleans up all the memory owned by a given wlr_input_device and signals
 * the destroy event.
 */
void wlr_input_device_finish(struct wlr_input_device *wlr_device);

#endif
