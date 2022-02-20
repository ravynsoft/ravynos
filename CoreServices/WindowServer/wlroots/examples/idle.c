#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "idle-client-protocol.h"

static struct org_kde_kwin_idle *idle_manager = NULL;
static struct wl_seat *seat = NULL;
static uint32_t timeout = 0, simulate_activity_timeout = 0, close_timeout = 0;
static int run = 1;

struct thread_args {
	struct wl_display *display;
	struct org_kde_kwin_idle_timeout *timer;
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	fprintf(stdout, "interfaces found: %s\n", interface);
	if (strcmp(interface, "org_kde_kwin_idle") == 0) {
		idle_manager = wl_registry_bind(registry, name, &org_kde_kwin_idle_interface, 1);
	}
	else if (strcmp(interface, "wl_seat") == 0) {
		seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
	//TODO
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

static void handle_idle(void* data, struct org_kde_kwin_idle_timeout *timer) {
	fprintf(stdout, "idle state\n");
}

static void handle_resume(void* data, struct org_kde_kwin_idle_timeout *timer) {
	fprintf(stdout, "active state\n");
}

static const struct org_kde_kwin_idle_timeout_listener idle_timer_listener = {
	.idle = handle_idle,
	.resumed = handle_resume,
};

static int parse_args(int argc, char *argv[]) {
	int c;
	while ((c = getopt(argc, argv, "c:hs:t:")) != -1) {
		switch(c)
		{
			case 'c':
				close_timeout = strtoul(optarg, NULL, 10);
				break;
			case 's':
				simulate_activity_timeout = strtoul(optarg, NULL, 10);
				break;
			case 't':
				timeout = strtoul(optarg, NULL, 10);
				break;
			case 'h':
			case '?':
				printf("Usage: %s [OPTIONS]\n", argv[0]);
				printf("  -t seconds\t\t\tidle timeout in seconds\n");
				printf("  -s seconds optional\t\tsimulate user activity after x seconds\n");
				printf("  -c seconds optional\t\tclose program after x seconds\n");
				printf("  -h\t\t\t\tthis help menu\n");
				return 1;
			default:
				return 1;
		}
	}
	return 0;
}

static void *simulate_activity(void *data) {
	sleep(simulate_activity_timeout);
	fprintf(stdout, "simulate user activity\n");
	struct thread_args *arg = data;
	org_kde_kwin_idle_timeout_simulate_user_activity(arg->timer);
	wl_display_roundtrip(arg->display);
	return NULL;
}

static void *close_program(void *data) {
	sleep(close_timeout);
	struct thread_args *arg = data;
	org_kde_kwin_idle_timeout_release(arg->timer);
	wl_display_roundtrip(arg->display);
	fprintf(stdout, "close program\n");
	run = 0;
	return NULL;
}

static void *main_loop(void *data) {
	struct wl_display *display = data;
	while (wl_display_dispatch(display) != -1) {
		;
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (parse_args(argc, argv) != 0) {
		return -1;
	}
	if (timeout == 0) {
		printf("idle timeout 0 is invalid\n");
		return -1;
	}

	struct wl_display *display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "failed to create display\n");
		return -1;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);
	wl_registry_destroy(registry);

	if (idle_manager == NULL) {
		fprintf(stderr, "display doesn't support idle protocol\n");
		return -1;
	}
	if (seat== NULL) {
		fprintf(stderr, "seat error\n");
		return -1;
	}
	struct org_kde_kwin_idle_timeout *timer =
		org_kde_kwin_idle_get_idle_timeout(idle_manager, seat, timeout * 1000);

	if (timer == NULL) {
		fprintf(stderr, "Could not create idle_timeout\n");
		return -1;
	}

	pthread_t t1, t2, t3;
	struct thread_args arg = {
		.timer = timer,
		.display = display,
	};

	bool create_t1 = (simulate_activity_timeout != 0) &&
		(simulate_activity_timeout < close_timeout);

	if (create_t1) {
		if (pthread_create(&t1, NULL, &simulate_activity, (void *)&arg) != 0) {
			return -1;
		}
	}

	bool create_t2 = (close_timeout != 0);

	if (create_t2) {
		if (pthread_create(&t2, NULL, &close_program, (void *)&arg) != 0) {
			if (create_t1) {
				pthread_cancel(t1);
			}
			return -1;
		}
	}

	org_kde_kwin_idle_timeout_add_listener(timer, &idle_timer_listener, timer);
	fprintf(stdout, "waiting\n");

	if (pthread_create(&t3, NULL, &main_loop, (void *)display) != 0) {
		if (create_t1) {
			pthread_cancel(t1);
		}
		if (create_t2) {
			pthread_cancel(t2);
		}
		return -1;
	}

	if (create_t1) {
		pthread_join(t1, NULL);
	}
	if (create_t2) {
		pthread_join(t2, NULL);
	}
	pthread_cancel(t3);

	wl_display_disconnect(display);
	return 0;
}
