#define _XOPEN_SOURCE 700 // for snprintf
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>
#include "util/time.h"

static bool colored = true;
static enum wlr_log_importance log_importance = WLR_ERROR;
static struct timespec start_time = {-1};

static const char *verbosity_colors[] = {
	[WLR_SILENT] = "",
	[WLR_ERROR] = "\x1B[1;31m",
	[WLR_INFO] = "\x1B[1;34m",
	[WLR_DEBUG] = "\x1B[1;90m",
};

static const char *verbosity_headers[] = {
	[WLR_SILENT] = "",
	[WLR_ERROR] = "[ERROR]",
	[WLR_INFO] = "[INFO]",
	[WLR_DEBUG] = "[DEBUG]",
};

static void init_start_time(void) {
	if (start_time.tv_sec >= 0) {
		return;
	}
	clock_gettime(CLOCK_MONOTONIC, &start_time);
}

static void log_stderr(enum wlr_log_importance verbosity, const char *fmt,
		va_list args) {
	init_start_time();

	if (verbosity > log_importance) {
		return;
	}

	struct timespec ts = {0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	timespec_sub(&ts, &ts, &start_time);

	fprintf(stderr, "%02d:%02d:%02d.%03ld ", (int)(ts.tv_sec / 60 / 60),
		(int)(ts.tv_sec / 60 % 60), (int)(ts.tv_sec % 60),
		ts.tv_nsec / 1000000);

	unsigned c = (verbosity < WLR_LOG_IMPORTANCE_LAST) ? verbosity : WLR_LOG_IMPORTANCE_LAST - 1;

	if (colored && isatty(STDERR_FILENO)) {
		fprintf(stderr, "%s", verbosity_colors[c]);
	} else {
		fprintf(stderr, "%s ", verbosity_headers[c]);
	}

	vfprintf(stderr, fmt, args);

	if (colored && isatty(STDERR_FILENO)) {
		fprintf(stderr, "\x1B[0m");
	}
	fprintf(stderr, "\n");
}

static wlr_log_func_t log_callback = log_stderr;

static void log_wl(const char *fmt, va_list args) {
	static char wlr_fmt[1024];
	int n = snprintf(wlr_fmt, sizeof(wlr_fmt), "[wayland] %s", fmt);
	if (n > 0 && wlr_fmt[n - 1] == '\n') {
		wlr_fmt[n - 1] = '\0';
	}
	_wlr_vlog(WLR_INFO, wlr_fmt, args);
}

void wlr_log_init(enum wlr_log_importance verbosity, wlr_log_func_t callback) {
	init_start_time();

	if (verbosity < WLR_LOG_IMPORTANCE_LAST) {
		log_importance = verbosity;
	}
	if (callback) {
		log_callback = callback;
	}

	wl_log_set_handler_server(log_wl);
}

void _wlr_vlog(enum wlr_log_importance verbosity, const char *fmt, va_list args) {
	log_callback(verbosity, fmt, args);
}

void _wlr_log(enum wlr_log_importance verbosity, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_callback(verbosity, fmt, args);
	va_end(args);
}

enum wlr_log_importance wlr_log_get_verbosity(void) {
	return log_importance;
}
