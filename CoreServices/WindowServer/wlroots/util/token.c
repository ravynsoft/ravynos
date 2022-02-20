#define _POSIX_C_SOURCE 200809L
#include "util/token.h"
#include "wlr/util/log.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool generate_token(char out[static TOKEN_STRLEN]) {
	static FILE *urandom = NULL;
	uint64_t data[2];

	if (!urandom) {
		int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
		if (fd < 0) {
			wlr_log_errno(WLR_ERROR, "Failed to open random device");
			return false;
		}
		if (!(urandom = fdopen(fd, "r"))) {
			wlr_log_errno(WLR_ERROR, "fdopen failed");
			close(fd);
			return false;
		}
	}
	if (fread(data, sizeof(data), 1, urandom) != 1) {
		wlr_log_errno(WLR_ERROR, "Failed to read from random device");
		return false;
	}
	if (snprintf(out, TOKEN_STRLEN, "%016" PRIx64 "%016" PRIx64, data[0], data[1]) != TOKEN_STRLEN - 1) {
		wlr_log_errno(WLR_ERROR, "Failed to format hex string token");
		return false;
	}
	return true;
}

