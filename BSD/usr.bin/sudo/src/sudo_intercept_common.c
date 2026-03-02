/*
 * SPDX-License-Identifier: ISC
 *
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#ifdef HAVE_CRT_EXTERNS_H
# include <crt_externs.h>
#endif

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_exec.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_util.h>
#include <intercept.pb-c.h>

#ifdef HAVE__NSGETENVIRON
# define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif

static union sudo_token_un intercept_token;
static in_port_t intercept_port;
static bool log_only;

/* Send entire request to sudo (blocking). */
static bool
send_req(int sock, const void *buf, size_t len)
{
    const uint8_t *cp = buf;
    ssize_t nwritten;
    debug_decl(send_req, SUDO_DEBUG_EXEC);

    do {
	nwritten = send(sock, cp, len, 0);
	if (nwritten == -1) {
	    if (errno == EINTR)
		continue;
	    debug_return_bool(false);
	}
	len -= (size_t)nwritten;
	cp += nwritten;
    } while (len > 0);

    debug_return_bool(true);
}

static bool
send_client_hello(int sock)
{
    InterceptRequest msg = INTERCEPT_REQUEST__INIT;
    InterceptHello hello = INTERCEPT_HELLO__INIT;
    uint8_t *buf = NULL;
    uint32_t msg_len;
    size_t len;
    bool ret = false;
    debug_decl(send_client_hello, SUDO_DEBUG_EXEC);

    /* Setup client hello. */
    hello.pid = getpid();
    msg.type_case = INTERCEPT_REQUEST__TYPE_HELLO;
    msg.u.hello = &hello;

    len = intercept_request__get_packed_size(&msg);
    if (len > MESSAGE_SIZE_MAX) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "InterceptRequest too large: %zu", len);
	goto done;
    }
    /* Wire message size is used for length encoding, precedes message. */
    msg_len = len & 0xffffffff;
    len += sizeof(msg_len);

    if ((buf = sudo_mmap_alloc(len)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    memcpy(buf, &msg_len, sizeof(msg_len));
    intercept_request__pack(&msg, buf + sizeof(msg_len));

    ret = send_req(sock, buf, len);

done:
    sudo_mmap_free(buf);
    debug_return_bool(ret);
}

/*
 * Receive InterceptResponse from sudo over fd.
 */
static InterceptResponse *
recv_intercept_response(int fd)
{
    InterceptResponse *res = NULL;
    ssize_t nread;
    uint32_t rem, res_len;
    uint8_t *cp, *buf = NULL;
    debug_decl(recv_intercept_response, SUDO_DEBUG_EXEC);

    /* Read message size (uint32_t in host byte order). */
    for (;;) {
	nread = recv(fd, &res_len, sizeof(res_len), 0);
	if (nread == ssizeof(res_len))
	    break;
	switch (nread) {
	case 0:
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unexpected EOF reading response size");
	    break;
	case -1:
	    if (errno == EINTR)
		continue;
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"error reading response size");
	    break;
	default:
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"error reading response size: short read");
	    break;
	}
	goto done;
    }
    if (res_len > MESSAGE_SIZE_MAX) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "InterceptResponse too large: %u", res_len);
        goto done;
    }

    /* Read response from sudo (blocking). */
    if ((buf = sudo_mmap_alloc(res_len)) == NULL) {
	goto done;
    }
    cp = buf;
    rem = res_len;
    do {
	nread = recv(fd, cp, rem, 0);
	switch (nread) {
	case 0:
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unexpected EOF reading response");
	    goto done;
	case -1:
	    if (errno == EINTR)
		continue;
	    sudo_debug_printf(
		SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"error reading response");
	    goto done;
	default:
	    rem -= (uint32_t)nread;
	    cp += nread;
	    break;
	}
    } while (rem > 0);
    res = intercept_response__unpack(NULL, res_len, buf);
    if (res == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to unpack %s size %u", "InterceptResponse", res_len);
        goto done;
    }

done:
    sudo_mmap_free(buf);
    debug_return_ptr(res);
}

/*
 * Look up SUDO_INTERCEPT_FD in the environment.
 * This function is run when the shared library is loaded.
 */
__attribute__((constructor)) static void
sudo_interposer_init(void)
{
    InterceptResponse *res = NULL;
    static bool initialized;
    int flags, fd = -1;
    char **p;
    debug_decl(sudo_interposer_init, SUDO_DEBUG_EXEC);

    if (initialized)
	debug_return;
    initialized = true;

    /* Read debug and path section of sudo.conf and init debugging. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG|SUDO_CONF_PATHS) != -1) {
	sudo_debug_register("sudo_intercept.so", NULL, NULL,
	    sudo_conf_debug_files("sudo_intercept.so"), INTERCEPT_FD_MIN);
    }
    sudo_debug_enter(__func__, __FILE__, __LINE__, sudo_debug_subsys);

    /*
     * Missing SUDO_INTERCEPT_FD will result in execve() failure.
     * Note that we cannot use getenv(3) here on Linux at least.
     */
    for (p = environ; *p != NULL; p++) {
	if (strncmp(*p, "SUDO_INTERCEPT_FD=", sizeof("SUDO_INTERCEPT_FD=") -1) == 0) {
	    const char *fdstr = *p + sizeof("SUDO_INTERCEPT_FD=") - 1;
	    const char *errstr;

	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO, "%s", *p);

	    fd = (int)sudo_strtonum(fdstr, 0, INT_MAX, &errstr);
	    if (errstr != NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "invalid SUDO_INTERCEPT_FD: %s: %s", fdstr, errstr);
		goto done;
	    }
	}
    }
    if (fd == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "SUDO_INTERCEPT_FD not found in environment");
	goto done;
    }

    /*
     * We don't want to use non-blocking I/O.
     */
    flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1)
        (void)fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    /*
     * Send InterceptHello message to over the fd.
     */
    if (!send_client_hello(fd))
	goto done;

    res = recv_intercept_response(fd);
    if (res != NULL) {
	if (res->type_case == INTERCEPT_RESPONSE__TYPE_HELLO_RESP) {
	    intercept_token.u64[0] = res->u.hello_resp->token_lo;
	    intercept_token.u64[1] = res->u.hello_resp->token_hi;
	    intercept_port = (in_port_t)res->u.hello_resp->portno;
	    log_only = res->u.hello_resp->log_only;
	} else {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unexpected type_case value %d in %s from %s",
		res->type_case, "InterceptResponse", "sudo");
	}
	intercept_response__free_unpacked(res, NULL);
    }

done:
    if (fd != -1)
	close(fd);

    debug_return;
}

static bool
send_policy_check_req(int sock, const char *cmnd, char * const argv[],
    char * const envp[])
{
    InterceptRequest msg = INTERCEPT_REQUEST__INIT;
    PolicyCheckRequest req = POLICY_CHECK_REQUEST__INIT;
    char cwdbuf[PATH_MAX];
    char *empty[1] = { NULL };
    uint8_t *buf = NULL;
    bool ret = false;
    uint32_t msg_len;
    size_t len;
    debug_decl(fmt_policy_check_req, SUDO_DEBUG_EXEC);

    /* Send token first (out of band) to initiate connection. */
    if (!send_req(sock, &intercept_token, sizeof(intercept_token))) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to send token back to sudo");
	goto done;
    }

    /* Setup policy check request. */
    req.intercept_fd = sock;
    req.command = (char *)cmnd;
    req.argv = argv ? (char **)argv : empty;
    for (req.n_argv = 0; req.argv[req.n_argv] != NULL; req.n_argv++)
	continue;
    req.envp = envp ? (char **)envp : empty;
    for (req.n_envp = 0; req.envp[req.n_envp] != NULL; req.n_envp++)
	continue;
    if (getcwd(cwdbuf, sizeof(cwdbuf)) != NULL) {
	req.cwd = cwdbuf;
    }
    msg.type_case = INTERCEPT_REQUEST__TYPE_POLICY_CHECK_REQ;
    msg.u.policy_check_req = &req;

    len = intercept_request__get_packed_size(&msg);
    if (len > MESSAGE_SIZE_MAX) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "InterceptRequest too large: %zu", len);
	goto done;
    }
    /* Wire message size is used for length encoding, precedes message. */
    msg_len = len & 0xffffffff;
    len += sizeof(msg_len);

    if ((buf = sudo_mmap_alloc(len)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    memcpy(buf, &msg_len, sizeof(msg_len));
    intercept_request__pack(&msg, buf + sizeof(msg_len));

    ret = send_req(sock, buf, len);

done:
    sudo_mmap_free(buf);
    debug_return_bool(ret);
}

/* 
 * Connect back to sudo process at localhost:intercept_port
 */
static int
intercept_connect(void)
{
    int sock = -1;
    int on = 1;
    struct sockaddr_in sin4;
    debug_decl(intercept_connect, SUDO_DEBUG_EXEC);

    if (intercept_port == 0) {
	sudo_warnx("%s", U_("intercept port not set"));
	goto done;
    }

    memset(&sin4, 0, sizeof(sin4));
    sin4.sin_family = AF_INET;
    sin4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin4.sin_port = htons(intercept_port);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
	sudo_warn("socket");
	goto done;
    }

    /* Send data immediately, we need low latency IPC. */
    (void)setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

    if (connect(sock, (struct sockaddr *)&sin4, sizeof(sin4)) == -1) {
	sudo_warn("connect");
	close(sock);
	sock = -1;
	goto done;
    }

done:
    debug_return_int(sock);
}

/* Called from sudo_intercept.c */
bool command_allowed(const char *cmnd, char * const argv[], char * const envp[], char **ncmndp, char ***nargvp, char ***nenvpp);

bool
command_allowed(const char *cmnd, char * const argv[],
    char * const envp[], char **ncmndp, char ***nargvp, char ***nenvpp)
{
    char *ncmnd = NULL, **nargv = NULL, **nenvp = NULL;
    InterceptResponse *res = NULL;
    bool ret = false;
    size_t idx, len = 0;
    int sock;
    debug_decl(command_allowed, SUDO_DEBUG_EXEC);

    if (sudo_debug_needed(SUDO_DEBUG_INFO)) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "req_command: %s", cmnd);
	if (argv != NULL) {
	    for (idx = 0; argv[idx] != NULL; idx++) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "req_argv[%zu]: %s", idx, argv[idx]);
	    }
	}
    }

    sock = intercept_connect();
    if (sock == -1)
	goto done;

    if (!send_policy_check_req(sock, cmnd, argv, envp))
	goto done;

    if (log_only) {
	/* Just logging, no policy check. */
	nenvp = sudo_preload_dso_mmap(envp, sudo_conf_intercept_path(), sock);
	if (nenvp == NULL)
	    goto oom;
	*ncmndp = (char *)cmnd;		/* safe */
	*nargvp = (char **)argv;	/* safe */
	*nenvpp = nenvp;
	ret = true;
	goto done;
    }

    res = recv_intercept_response(sock);
    if (res == NULL)
	goto done;

    switch (res->type_case) {
    case INTERCEPT_RESPONSE__TYPE_ACCEPT_MSG:
	if (sudo_debug_needed(SUDO_DEBUG_INFO)) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"run_command: %s", res->u.accept_msg->run_command);
	    for (idx = 0; idx < res->u.accept_msg->n_run_argv; idx++) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "run_argv[%zu]: %s", idx, res->u.accept_msg->run_argv[idx]);
	    }
	}
	ncmnd = sudo_mmap_strdup(res->u.accept_msg->run_command);
	if (ncmnd == NULL)
	    goto oom;
	nargv = sudo_mmap_allocarray(res->u.accept_msg->n_run_argv + 1,
	    sizeof(char *));
	if (nargv == NULL)
	    goto oom;
	for (len = 0; len < res->u.accept_msg->n_run_argv; len++) {
	    nargv[len] = sudo_mmap_strdup(res->u.accept_msg->run_argv[len]);
	    if (nargv[len] == NULL)
		goto oom;
	}
	nargv[len] = NULL;
	nenvp = sudo_preload_dso_mmap(envp, sudo_conf_intercept_path(), sock);
	if (nenvp == NULL)
	    goto oom;
	*ncmndp = ncmnd;
	*nargvp = nargv;
	*nenvpp = nenvp;
	ret = true;
	goto done;
    case INTERCEPT_RESPONSE__TYPE_REJECT_MSG:
	/* Policy module displayed reject message but we are in raw mode. */
	fputc('\r', stderr);
	goto done;
    case INTERCEPT_RESPONSE__TYPE_ERROR_MSG:
	/* Policy module may display error message but we are in raw mode. */
	fputc('\r', stderr);
	sudo_warnx("%s", res->u.error_msg->error_message);
	goto done;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unexpected type_case value %d in %s from %s",
	    res->type_case, "InterceptResponse", "sudo");
	goto done;
    }

oom:
    sudo_mmap_free(ncmnd);
    while (len > 0)
	sudo_mmap_free(nargv[--len]);
    sudo_mmap_free(nargv);

done:
    /* Keep socket open for ctor when we execute the command. */
    if (!ret && sock != -1)
	close(sock);
    intercept_response__free_unpacked(res, NULL);

    debug_return_bool(ret);
}
