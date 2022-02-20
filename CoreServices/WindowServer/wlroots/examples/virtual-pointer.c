/*
 * Copyright © 2019 Josef Gajdusek
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

#define _POSIX_C_SOURCE 200112L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

static struct wl_seat *seat = NULL;
static struct zwlr_virtual_pointer_manager_v1 *pointer_manager = NULL;

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name,
			&wl_seat_interface, version);
	} else if (strcmp(interface,
			zwlr_virtual_pointer_manager_v1_interface.name) == 0) {
		pointer_manager = wl_registry_bind(registry, name,
			&zwlr_virtual_pointer_manager_v1_interface, 1);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// Who cares?
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: ./virtual-pointer <subcommand>\n");
		return EXIT_FAILURE;
	}
	struct wl_display * display = wl_display_connect(NULL);
	if (display == NULL) {
		perror("failed to create display");
		return EXIT_FAILURE;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (pointer_manager == NULL) {
		fprintf(stderr, "compositor does not support wlr-virtual-pointer-unstable-v1\n");
		return EXIT_FAILURE;
	}

	struct zwlr_virtual_pointer_v1 *pointer =
		zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
			pointer_manager, seat);

	const char *cmd = argv[1];
	if (strcmp(cmd, "motion") == 0) {
		if (argc < 4) {
			fprintf(stderr, "Usage: ./virtual-pointer motion <dx> <dy>\n");
			return EXIT_FAILURE;
		}
		wl_fixed_t dx = wl_fixed_from_double(atof(argv[2]));
		wl_fixed_t dy = wl_fixed_from_double(atof(argv[3]));
		zwlr_virtual_pointer_v1_motion(pointer, 0, dx, dy);
	} else if (strcmp(cmd, "absolute") == 0) {
		if (argc < 6) {
			fprintf(stderr, "Usage: ./virtual-pointer absolute <x> <y> <x_extent> <y_extent>\n");
			return EXIT_FAILURE;
		}
		uint32_t x = atoi(argv[2]);
		uint32_t y = atoi(argv[3]);
		uint32_t x_extent = atoi(argv[4]);
		uint32_t y_extent = atoi(argv[5]);
		zwlr_virtual_pointer_v1_motion_absolute(pointer, 0, x, y, x_extent, y_extent);
	} else if (strcmp(cmd, "button") == 0) {
		if (argc < 4) {
			fprintf(stderr, "Usage: ./virtual-pointer button <button> press|release\n");
			return EXIT_FAILURE;
		}
		uint32_t button = atoi(argv[2]);
		bool press = !!strcmp(argv[3], "release");
		zwlr_virtual_pointer_v1_button(pointer, 0, button, press);
	} else if (strcmp(cmd, "axis") == 0) {
		if (argc < 4) {
			fprintf(stderr, "Usage: ./virtual-pointer axis <axis> <value>\n");
			return EXIT_FAILURE;
		}
		uint32_t axis = atoi(argv[2]);
		wl_fixed_t value = wl_fixed_from_double(atof(argv[3]));
		zwlr_virtual_pointer_v1_axis(pointer, 0, axis, value);
		zwlr_virtual_pointer_v1_axis_stop(pointer, 0, axis);
	} else if (strcmp(cmd, "axis_discrete") == 0) {
		if (argc < 5) {
			fprintf(stderr, "Usage: ./virtual-pointer axis <axis> <value> <value_discrete>\n");
			return EXIT_FAILURE;
		}
		uint32_t axis = atoi(argv[2]);
		wl_fixed_t value = wl_fixed_from_double(atof(argv[3]));
		uint32_t discrete = atoi(argv[4]);
		zwlr_virtual_pointer_v1_axis_discrete(pointer, 0, axis, value, discrete);
		zwlr_virtual_pointer_v1_axis_stop(pointer, 0, axis);
	} else {
		fprintf(stderr, "Invalid subcommand\n");
		return EXIT_FAILURE;
	}

	zwlr_virtual_pointer_v1_frame(pointer);
	zwlr_virtual_pointer_v1_destroy(pointer);

	wl_display_flush(display);

	return EXIT_SUCCESS;
}
