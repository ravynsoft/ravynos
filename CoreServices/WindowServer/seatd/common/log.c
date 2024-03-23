#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "log.h"

const long NSEC_PER_SEC = 1000000000;

static void log_stderr(enum libseat_log_level level, const char *fmt, va_list args);

static enum libseat_log_level current_log_level = LIBSEAT_LOG_LEVEL_SILENT;
static libseat_log_func current_log_handler = log_stderr;
static struct timespec start_time = {-1, -1};
static bool colored = false;

static const char *verbosity_colors[] = {
	[LIBSEAT_LOG_LEVEL_SILENT] = "",
	[LIBSEAT_LOG_LEVEL_ERROR] = "\x1B[1;31m",
	[LIBSEAT_LOG_LEVEL_INFO] = "\x1B[1;34m",
	[LIBSEAT_LOG_LEVEL_DEBUG] = "\x1B[1;90m",
};

static const char *verbosity_headers[] = {
	[LIBSEAT_LOG_LEVEL_SILENT] = "",
	[LIBSEAT_LOG_LEVEL_ERROR] = "[ERROR]",
	[LIBSEAT_LOG_LEVEL_INFO] = "[INFO]",
	[LIBSEAT_LOG_LEVEL_DEBUG] = "[DEBUG]",
};

static void timespec_sub(struct timespec *r, const struct timespec *a, const struct timespec *b) {
	r->tv_sec = a->tv_sec - b->tv_sec;
	r->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (r->tv_nsec < 0) {
		r->tv_sec--;
		r->tv_nsec += NSEC_PER_SEC;
	}
}

static void log_stderr(enum libseat_log_level level, const char *fmt, va_list args) {
	struct timespec ts = {0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	timespec_sub(&ts, &ts, &start_time);
	unsigned c = (level < LIBSEAT_LOG_LEVEL_LAST) ? level : LIBSEAT_LOG_LEVEL_LAST - 1;

	const char *prefix, *postfix;

	if (colored) {
		prefix = verbosity_colors[c];
		postfix = "\x1B[0m\n";
	} else {
		prefix = verbosity_headers[c];
		postfix = "\n";
	}

	fprintf(stderr, "%02d:%02d:%02d.%03ld %s ", (int)(ts.tv_sec / 60 / 60),
		(int)(ts.tv_sec / 60 % 60), (int)(ts.tv_sec % 60), ts.tv_nsec / 1000000, prefix);

	vfprintf(stderr, fmt, args);

	fprintf(stderr, "%s", postfix);
}

void log_init(void) {
	if (start_time.tv_sec >= 0) {
		return;
	}
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	colored = isatty(STDERR_FILENO);
}

void _logf(enum libseat_log_level level, const char *fmt, ...) {
	int stored_errno = errno;
	va_list args;
	if (level > current_log_level) {
		return;
	}

	va_start(args, fmt);
	current_log_handler(level, fmt, args);
	va_end(args);

	errno = stored_errno;
}

void libseat_set_log_handler(libseat_log_func handler) {
	if (handler == NULL) {
		handler = log_stderr;
	}
	current_log_handler = handler;
}

void libseat_set_log_level(enum libseat_log_level level) {
	current_log_level = level;
}
