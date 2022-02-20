#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "wlr-output-power-management-unstable-v1-client-protocol.h"

struct output {
	struct wl_output *wl_output;
	struct zwlr_output_power_v1 *output_power;
	struct wl_list link;
};

static struct wl_list outputs;
static struct zwlr_output_power_manager_v1 *output_power_manager = NULL;

static void output_power_handle_mode(void *data,
		struct zwlr_output_power_v1 *output_power,
		enum zwlr_output_power_v1_mode mode) {
	struct output *output = data;

	switch (mode) {
	case ZWLR_OUTPUT_POWER_V1_MODE_OFF:
		printf("Output %p disabled\n", output);
		break;
	case ZWLR_OUTPUT_POWER_V1_MODE_ON:
		printf("Output %p enabled\n", output);
		break;
	}
}

static void output_power_handle_failed(void *data,
		struct zwlr_output_power_v1 *output_power) {
	fprintf(stderr, "failed to set output power mode\n");
	exit(EXIT_FAILURE);
}

static const struct zwlr_output_power_v1_listener output_power_listener = {
	.mode = output_power_handle_mode,
	.failed = output_power_handle_failed,
};

static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_output_interface.name) == 0) {
		struct output *output = calloc(1, sizeof(struct output));
		output->wl_output = wl_registry_bind(registry, name,
			&wl_output_interface, 1);
		wl_list_insert(&outputs, &output->link);
	} else if (strcmp(interface,
			zwlr_output_power_manager_v1_interface.name) == 0) {
		output_power_manager = wl_registry_bind(registry, name,
			&zwlr_output_power_manager_v1_interface, 1);
	}
}

static void registry_handle_global_remove(void *data,
		struct wl_registry *registry, uint32_t name) {
	// Who cares?
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static const char usage[] = "usage: output-power-management [options...]\n"
	"  -h: show this help message\n"
	"  -e: turn outputs on\n"
	"  -d: turn outputs off\n"
	"  -w: continuously watch for power mode changes\n";

int main(int argc, char *argv[]) {
	wl_list_init(&outputs);

	int opt;
	enum zwlr_output_power_v1_mode mode =
		ZWLR_OUTPUT_POWER_V1_MODE_ON;
	bool watch_mode = false;
	while ((opt = getopt(argc, argv, "edhw")) != -1) {
		switch (opt) {
		case 'e':
			mode = ZWLR_OUTPUT_POWER_V1_MODE_ON;
			break;
		case 'd':
			mode = ZWLR_OUTPUT_POWER_V1_MODE_OFF;
			break;
		case 'w':
			watch_mode = true;
			break;
		case 'h':
		default:
			fprintf(stderr, usage);
			return opt == 'h' ? EXIT_SUCCESS : EXIT_FAILURE;
		}
	}

	struct wl_display *display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "failed to create display\n");
		return -1;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (output_power_manager == NULL) {
		fprintf(stderr,
			"compositor doesn't support wlr-output-power-management-unstable-v1\n");
		return EXIT_FAILURE;
	}

	struct output *output;
	wl_list_for_each(output, &outputs, link) {
		output->output_power = zwlr_output_power_manager_v1_get_output_power(
			output_power_manager, output->wl_output);
		zwlr_output_power_v1_add_listener(output->output_power,
			&output_power_listener, output);
	}
	wl_display_roundtrip(display);

	wl_list_for_each(output, &outputs, link) {
		zwlr_output_power_v1_set_mode(output->output_power,
			mode);
	}

	if (!watch_mode) {
		wl_display_roundtrip(display);
		return EXIT_SUCCESS;
	}

	while (wl_display_dispatch(display) != -1) {
		// nothing to see here, please move along
	}

	return EXIT_SUCCESS;
}
