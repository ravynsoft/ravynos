#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "input-method-unstable-v2-client-protocol.h"
#include "text-input-unstable-v3-client-protocol.h"
#include "xdg-shell-client-protocol.h"

const char usage[] = "Usage: input-method [seconds]\n\
\n\
Creates an input method using the input-method protocol.\n\
\n\
Whenever a text input is activated, this program sends a few sequences of\n\
commands and checks the validity of the responses, relying on returned\n\
surrounding text.\n\
\n\
The \"seconds\" argument is optional and defines the maximum delay between\n\
stages.";

struct input_method_state {
	enum zwp_text_input_v3_change_cause change_cause;
	struct {
		enum zwp_text_input_v3_content_hint hint;
		enum zwp_text_input_v3_content_purpose purpose;
	} content_type;
	struct {
		char *text;
		uint32_t cursor;
		uint32_t anchor;
	} surrounding;
};

static int sleeptime = 0;

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_seat *seat = NULL;
static struct zwp_input_method_manager_v2 *input_method_manager = NULL;
static struct zwp_input_method_v2 *input_method = NULL;

struct input_method_state pending;
struct input_method_state current;

static uint32_t serial = 0;
bool active = false;
bool pending_active = false;
bool unavailable = false;
bool running = false;

uint32_t update_stage = 0;

int timer_fd = 0;

static void print_state_diff(struct input_method_state previous,
		struct input_method_state future) {
	if (previous.content_type.hint != future.content_type.hint) {
		char *strs[] = { "COMPLETION", "SPELLCHECK", "AUTO_CAPITALIZATION",
			"LOWERCASE", "UPPERCASE", "TITLECASE", "HIDDEN_TEXT",
			"SENSITIVE_DATA", "LATIN", "MULTILINE"};
		printf("content_type.hint:");
		uint32_t hint = future.content_type.hint;
		if (!hint) {
			printf(" NONE");
		}
		for (unsigned i = 0; i < sizeof(strs) / sizeof(*strs); i++) {
			if (hint & 1 << i) {
				printf(" %s", strs[i]);
			}
		}
		printf("\n");
	}
	if (previous.content_type.purpose != future.content_type.purpose) {
		char *strs[] = { "NORMAL", "ALPHA", "DIGITS", "NUMBER", "PHONE", "URL",
			"EMAIL", "NAME", "PASSWORD", "PIN", "DATE", "TIME", "DATETIME",
			"TERMINAL" };
		printf("content_type.purpose: %s\n", strs[future.content_type.purpose]);
	}
	if (!!previous.surrounding.text != !!future.surrounding.text
			|| (previous.surrounding.text && future.surrounding.text
				&& strcmp(previous.surrounding.text, future.surrounding.text) != 0)
			|| previous.surrounding.anchor != future.surrounding.anchor
			|| previous.surrounding.cursor != future.surrounding.cursor) {
		char *text = future.surrounding.text;
		if (!text) {
			printf("Removed surrounding text\n");
		} else {
			printf("Surrounding text: %s\n", text);
			uint32_t anchor = future.surrounding.anchor;
			uint32_t cursor = future.surrounding.cursor;
			if (cursor == anchor) {
				char *temp = strndup(text, cursor);
				printf("Cursor after %d: %s\n", cursor, temp);
				free(temp);
			} else {
				if (cursor > anchor) {
					uint32_t tmp = anchor;
					anchor = cursor;
					cursor = tmp;
				}
				char *temp = strndup(&text[cursor], anchor - cursor);
				printf("Selection: %s\n", temp);
				free(temp);
			}
		}
	}
	if (previous.change_cause != future.change_cause) {
		char *strs[] = { "INPUT_METHOD", "OTHER" };
		printf("Change cause: %s\n", strs[future.change_cause]);
	}
}

static void handle_content_type(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2,
		uint32_t hint, uint32_t purpose) {
	pending.content_type.hint = hint;
	pending.content_type.purpose = purpose;
}

static void handle_surrounding_text(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2,
		const char *text, uint32_t cursor, uint32_t anchor) {
	free(pending.surrounding.text);
	pending.surrounding.text = strdup(text);
	pending.surrounding.cursor = cursor;
	pending.surrounding.anchor = anchor;
}

static void handle_text_change_cause(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2,
		uint32_t cause) {
	pending.change_cause = cause;
}

static void handle_activate(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	pending_active = true;
}

static void handle_deactivate(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	pending_active = false;
}

static void handle_unavailable(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	printf("IM disappeared\n");
	zwp_input_method_v2_destroy(zwp_input_method_v2);
	input_method = NULL;
	running = false;
}

static void im_activate(void *data,
		struct zwp_input_method_v2 *id) {
	update_stage = 0;
}

static void timer_arm(unsigned seconds) {
	printf("Timer armed\n");
	struct itimerspec spec = {
		.it_interval = {0},
		.it_value = {
			.tv_sec = seconds,
			.tv_nsec = 0
		}
	};
	if (timerfd_settime(timer_fd, 0, &spec, NULL)) {
		fprintf(stderr, "Failed to arm timer: %s\n", strerror(errno));
	}
}

static void do_updates(void) {
	printf("Update %d\n", update_stage);
	switch (update_stage) {
	case 0:
		// TODO: remember initial surrounding text
		zwp_input_method_v2_set_preedit_string(input_method, "Preedit", 2, 4);
		zwp_input_method_v2_commit(input_method, serial);
		// don't expect an answer, preedit doesn't change anything visible
		timer_arm(sleeptime);
		update_stage++;
		return;
	case 1:
		zwp_input_method_v2_set_preedit_string(input_method, "Præedit2", strlen("Pr"), strlen("Præed"));
		zwp_input_method_v2_commit_string(input_method, "_Commit_");
		zwp_input_method_v2_commit(input_method, serial);
		update_stage++;
		break;
	case 2:
		if (strcmp(current.surrounding.text, "_Commit_") != 0) {
			return;
		}
		zwp_input_method_v2_commit_string(input_method, "_CommitNoPreed_");
		zwp_input_method_v2_commit(input_method, serial);
		timer_arm(sleeptime);
		update_stage++;
		break;
	case 3:
		if (strcmp(current.surrounding.text, "_Commit__CommitNoPreed_") != 0) {
			return;
		}
		zwp_input_method_v2_commit_string(input_method, "_WaitNo_");
		zwp_input_method_v2_delete_surrounding_text(input_method, strlen("_CommitNoPreed_"), 0);
		zwp_input_method_v2_commit(input_method, serial);
		update_stage++;
		break;
	case 4:
		if (strcmp(current.surrounding.text, "_Commit__WaitNo_") != 0) {
			return;
		}
		zwp_input_method_v2_set_preedit_string(input_method, "PreedWithDel", strlen("Preed"), strlen("Preed"));
		zwp_input_method_v2_delete_surrounding_text(input_method, strlen("_WaitNo_"), 0);
		zwp_input_method_v2_commit(input_method, serial);
		update_stage++;
		break;
	case 5:
		if (strcmp(current.surrounding.text, "_Commit_") != 0) {
			return;
		}
		zwp_input_method_v2_delete_surrounding_text(input_method, strlen("mit_"), 0);
		zwp_input_method_v2_commit(input_method, serial);
		update_stage++;
		break;
	case 6:
		if (strcmp(current.surrounding.text, "_Com") != 0) {
			printf("Failed\n");
		}
		update_stage++;
		break;
	default:
		printf("Submitted everything\n");
		return;
	};
}

static void handle_timer(void) {
	printf("Timer dispatched at %d\n", update_stage);
	do_updates();
}

static void im_deactivate(void *data,
		struct zwp_input_method_v2 *context) {
	// No special action needed
}

static void handle_done(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	bool prev_active = active;
	serial++;
	printf("Handle serial %d\n", serial);
	if (active != pending_active) {
		printf("Now %s\n", pending_active ? "active" : "inactive");
	}
	if (pending_active) {
		print_state_diff(current, pending);
	}
	active = pending_active;
	free(current.surrounding.text);
	struct input_method_state default_state = {0};
	current = pending;
	pending = default_state;
	if (active && !prev_active) {
		im_activate(data, zwp_input_method_v2);
	} else if (!active && prev_active) {
		im_deactivate(data, zwp_input_method_v2);
	}

	do_updates();
}

static const struct zwp_input_method_v2_listener im_listener = {
	.activate = handle_activate,
	.deactivate = handle_deactivate,
	.surrounding_text = handle_surrounding_text,
	.text_change_cause = handle_text_change_cause,
	.content_type = handle_content_type,
	.done = handle_done,
	.unavailable = handle_unavailable,
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, "wl_compositor") == 0) {
		compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, 1);
	} else if (strcmp(interface, zwp_input_method_manager_v2_interface.name) == 0) {
		input_method_manager = wl_registry_bind(registry, name,
			&zwp_input_method_manager_v2_interface, 1);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// who cares
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

int main(int argc, char **argv) {
	if (argc > 1) {
		if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
			printf(usage);
			return 0;
		}
		sleeptime = atoi(argv[1]);
	}
	display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "Failed to create display\n");
		return EXIT_FAILURE;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (compositor == NULL) {
		fprintf(stderr, "wl-compositor not available\n");
		return EXIT_FAILURE;
	}
	if (input_method_manager == NULL) {
		fprintf(stderr, "input-method not available\n");
		return EXIT_FAILURE;
	}
	if (seat == NULL) {
		fprintf(stderr, "seat not available\n");
		return EXIT_FAILURE;
	}

	input_method = zwp_input_method_manager_v2_get_input_method(
		input_method_manager, seat);
	running = true;
	zwp_input_method_v2_add_listener(input_method, &im_listener, NULL);

	int display_fd = wl_display_get_fd(display);
	timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
	if (timer_fd < 0) {
		fprintf(stderr, "Failed to start timer\n");
		return EXIT_FAILURE;
	}
	int epoll = epoll_create1(EPOLL_CLOEXEC);
	if (epoll < 0) {
		fprintf(stderr, "Failed to start epoll\n");
		return EXIT_FAILURE;
	}

	struct epoll_event epoll_display = {
		.events = EPOLLIN | EPOLLOUT,
		.data = {.fd = display_fd},
	};
	if (epoll_ctl(epoll, EPOLL_CTL_ADD, display_fd, &epoll_display)) {
		fprintf(stderr, "Failed to epoll display\n");
		return EXIT_FAILURE;
	}

	wl_display_roundtrip(display); // timer may be armed here

	struct epoll_event epoll_timer = {
		.events = EPOLLIN,
		.data = {.fd = timer_fd},
	};
	if (epoll_ctl(epoll, EPOLL_CTL_ADD, timer_fd, &epoll_timer)) {
		fprintf(stderr, "Failed to epoll timer\n");
		return EXIT_FAILURE;
	}

	timer_arm(2);

	struct epoll_event caught;
	while (epoll_wait(epoll, &caught, 1, -1)) {
		if (!running) {
			printf("Exiting\n");
			return EXIT_SUCCESS;
		}
		if (caught.data.fd == display_fd) {
			if (wl_display_dispatch(display) == -1) {
				break;
			}
		} else if (caught.data.fd == timer_fd) {
			uint64_t expirations;
			ssize_t n = read(timer_fd, &expirations, sizeof(expirations));
			assert(n >= 0);
			handle_timer();
		} else {
			printf("Unknown source\n");
		}
	}
	return EXIT_SUCCESS;
}
