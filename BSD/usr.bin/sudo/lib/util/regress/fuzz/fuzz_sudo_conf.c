/*
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static int
fuzz_conversation(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    int n;

    for (n = 0; n < num_msgs; n++) {
	const struct sudo_conv_message *msg = &msgs[n];

	switch (msg->msg_type & 0xff) {
	    case SUDO_CONV_PROMPT_ECHO_ON:
	    case SUDO_CONV_PROMPT_MASK:
	    case SUDO_CONV_PROMPT_ECHO_OFF:
		/* input not supported */
		return -1;
	    case SUDO_CONV_ERROR_MSG:
	    case SUDO_CONV_INFO_MSG:
		/* no output for fuzzers */
		break;
	    default:
		return -1;
	}
    }
    return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    struct plugin_info_list *plugins = sudo_conf_plugins();
    struct sudo_conf_debug_list *debug_list = sudo_conf_debugging();
    struct sudo_conf_debug_file_list *debug_files;
    char tempfile[] = "/tmp/sudo_conf.XXXXXX";
    struct sudo_conf_debug *debug_spec;
    struct sudo_debug_file *debug_file;
    struct plugin_info *info;
    ssize_t nwritten;
    int fd;

    initprogname("fuzz_sudo_conf");
    if (getenv("SUDO_FUZZ_VERBOSE") == NULL)
	sudo_warn_set_conversation(fuzz_conversation);

    /* sudo_conf_read() uses a conf file path, not an open file. */
    fd = mkstemp(tempfile);
    if (fd == -1)
	return 0;
    nwritten = write(fd, data, size);
    if (nwritten == -1) {
	close(fd);
	return 0;
    }
    close(fd);

    /* sudo_conf_read() will re-init and free old data each time it runs. */
    sudo_conf_clear_paths();
    sudo_conf_read(tempfile, SUDO_CONF_ALL);

    /* Path settings. */
    if (sudo_conf_askpass_path() != NULL)
	sudo_warnx("Path askpass %s", sudo_conf_askpass_path());
    if (sudo_conf_sesh_path() != NULL)
	sudo_warnx("Path sesh %s", sudo_conf_sesh_path());
    if (sudo_conf_intercept_path() != NULL)
	sudo_warnx("Path intercept %s", sudo_conf_intercept_path());
    if (sudo_conf_noexec_path() != NULL)
	sudo_warnx("Path noexec %s", sudo_conf_noexec_path());
    if (sudo_conf_plugin_dir_path() != NULL)
	sudo_warnx("Path plugin_dir %s", sudo_conf_plugin_dir_path());

    /* Other settings. */
    sudo_warnx("Set disable_coredump %s",
	sudo_conf_disable_coredump() ? "true" : "false");
    sudo_warnx("Set group_source %s",
	sudo_conf_group_source() == GROUP_SOURCE_ADAPTIVE ? "adaptive" :
	sudo_conf_group_source() == GROUP_SOURCE_STATIC ? "static" : "dynamic");
    sudo_warnx("Set max_groups %d", sudo_conf_max_groups());
    sudo_warnx("Set probe_interfaces %s",
	sudo_conf_probe_interfaces() ? "true" : "false");

    /* Plugins. */
    plugins = sudo_conf_plugins();
    TAILQ_FOREACH(info, plugins, entries) {
	/* We don't bother with the plugin options. */
	sudo_warnx("Plugin %s %s", info->symbol_name, info->path);
    }

    /* Debug settings. */
    debug_list = sudo_conf_debugging();
    TAILQ_FOREACH(debug_spec, debug_list, entries) {
	TAILQ_FOREACH(debug_file, &debug_spec->debug_files, entries) {
	    sudo_warnx("Debug %s %s %s", debug_spec->progname,
		debug_file->debug_file, debug_file->debug_flags);
	}
    }

    debug_files = sudo_conf_debug_files(getprogname());
    if (debug_files != NULL) {
	TAILQ_FOREACH(debug_file, debug_files, entries) {
	    sudo_warnx("Debug %s %s %s", getprogname(),
		debug_file->debug_file, debug_file->debug_flags);
	}
    }

    unlink(tempfile);

    fflush(stdout);

    return 0;
}
