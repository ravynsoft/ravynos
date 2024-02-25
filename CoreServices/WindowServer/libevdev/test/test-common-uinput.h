// SPDX-License-Identifier: MIT
/*
 * Copyright © 2013 Red Hat, Inc.
 */

#include "config.h"
#include <libevdev/libevdev.h>

#define DEFAULT_IDS NULL

struct uinput_device* uinput_device_new(const char *name);
int uinput_device_new_with_events(struct uinput_device **dev, const char *name, const struct input_id *ids, ...);
int uinput_device_new_with_events_v(struct uinput_device **dev, const char *name, const struct input_id *ids, va_list args);
void uinput_device_free(struct uinput_device *dev);

int uinput_device_create(struct uinput_device* dev);
int uinput_device_set_name(struct uinput_device* dev, const char *name);
int uinput_device_set_ids(struct uinput_device* dev, const struct input_id *ids);
int uinput_device_set_bit(struct uinput_device* dev, unsigned int bit);
int uinput_device_set_prop(struct uinput_device *dev, unsigned int prop);
int uinput_device_set_event_bit(struct uinput_device* dev, unsigned int type, unsigned int code);
int uinput_device_set_event_bits(struct uinput_device* dev, ...);
int uinput_device_set_event_bits_v(struct uinput_device* dev, va_list args);
int uinput_device_set_abs_bit(struct uinput_device* dev, unsigned int code, const struct input_absinfo *absinfo);
int uinput_device_event(const struct uinput_device* dev, unsigned int type, unsigned int code, int value);
int uinput_device_event_multiple(const struct uinput_device* dev, ...);
int uinput_device_event_multiple_v(const struct uinput_device* dev, va_list args);
int uinput_device_get_fd(const struct uinput_device *dev);
const char* uinput_device_get_devnode(const struct uinput_device *dev);

char *uinput_devnode_from_syspath(const char *syspath);
