/*
 * Copyright (c) 2021-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <regex.h>
#include <time.h>
#include <unistd.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_iolog.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

#include <logsrvd.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

/*
 * Stub version that always succeeds for small inputs and fails for large.
 * We want to fuzz our parser, not libc's regular expression code.
 */
bool
sudo_regex_compile_v1(void *v, const char *pattern, const char **errstr)
{
    regex_t *preg = v;

    if (strlen(pattern) > 32) {
	*errstr = "invalid regular expression";
	return false;
    }

    /* hopefully avoid regfree() crashes */
    memset(preg, 0, sizeof(*preg));
    return true;
}

/*
 * The fuzzing environment may not have DNS available, this may result
 * in long delays that cause a timeout when fuzzing.
 * This getaddrinfo() resolves every name as "localhost" (127.0.0.1).
 */
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
/* Avoid compilation errors if getaddrinfo() or freeaddrinfo() are macros. */
# undef getaddrinfo
# undef freeaddrinfo

int
# ifdef HAVE_GETADDRINFO
getaddrinfo(
# else
sudo_getaddrinfo(
# endif
    const char *nodename, const char *servname,
    const struct addrinfo *hints, struct addrinfo **res)
{
    struct addrinfo *ai;
    struct in_addr addr;
    unsigned short port = 0;

    /* Stub getaddrinfo(3) to avoid a DNS timeout in CIfuzz. */
    if (servname == NULL) {
	/* Must have either nodename or servname. */
	if (nodename == NULL)
	    return EAI_NONAME;
    } else {
	struct servent *servent;
	const char *errstr;

	/* Parse servname as a port number or IPv4 TCP service name. */
	port = sudo_strtonum(servname, 0, USHRT_MAX, &errstr);
	if (errstr != NULL && errno == ERANGE)
	    return EAI_SERVICE;
	if (hints != NULL && ISSET(hints->ai_flags, AI_NUMERICSERV))
	    return EAI_NONAME;
	servent = getservbyname(servname, "tcp");
	if (servent == NULL)
	    return EAI_NONAME;
	port = htons(servent->s_port);
    }

    /* Hard-code IPv4 localhost for fuzzing. */
    ai = calloc(1, sizeof(*ai) + sizeof(struct sockaddr_in));
    if (ai == NULL)
	return EAI_MEMORY;
    ai->ai_canonname = strdup("localhost");
    if (ai == NULL) {
	free(ai);
	return EAI_MEMORY;
    }
    ai->ai_family = AF_INET;
    ai->ai_protocol = IPPROTO_TCP;
    ai->ai_addrlen = sizeof(struct sockaddr_in);
    ai->ai_addr = (struct sockaddr *)(ai + 1);
    inet_pton(AF_INET, "127.0.0.1", &addr);
    ((struct sockaddr_in *)ai->ai_addr)->sin_family = AF_INET;
    ((struct sockaddr_in *)ai->ai_addr)->sin_addr = addr;
    ((struct sockaddr_in *)ai->ai_addr)->sin_port = htons(port);
    *res = ai;
    return 0;
}

void
# ifdef HAVE_GETADDRINFO
freeaddrinfo(struct addrinfo *ai)
# else
sudo_freeaddrinfo(struct addrinfo *ai)
# endif
{
    struct addrinfo *next;

    while (ai != NULL) {
	next = ai->ai_next;
	free(ai->ai_canonname);
	free(ai);
	ai = next;
    }
}
#endif /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */

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
    char tempfile[] = "/tmp/logsrvd_conf.XXXXXX";
    ssize_t nwritten;
    int fd;

    initprogname("fuzz_logsrvd_conf");
    if (getenv("SUDO_FUZZ_VERBOSE") == NULL)
	sudo_warn_set_conversation(fuzz_conversation);

    /* logsrvd_conf_read() uses a conf file path, not an open file. */
    fd = mkstemp(tempfile);
    if (fd == -1)
	return 0;
    nwritten = write(fd, data, size);
    if (nwritten == -1) {
	close(fd);
	return 0;
    }
    close(fd);

    if (logsrvd_conf_read(tempfile)) {
	/* public config getters */
	logsrvd_conf_iolog_dir();
	logsrvd_conf_iolog_file();
	logsrvd_conf_iolog_mode();
	logsrvd_conf_pid_file();
	logsrvd_conf_relay_address();
	logsrvd_conf_relay_connect_timeout();
	logsrvd_conf_relay_tcp_keepalive();
	logsrvd_conf_relay_timeout();
	logsrvd_conf_server_listen_address();
	logsrvd_conf_server_tcp_keepalive();
	logsrvd_conf_server_timeout();

	/* free config */
	logsrvd_conf_cleanup();
    }

    unlink(tempfile);

    fflush(stdout);

    return 0;
}
