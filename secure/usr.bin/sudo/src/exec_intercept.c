/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#include <sudo.h>
#include <sudo_exec.h>
#include <sudo_plugin.h>
#include <sudo_plugin_int.h>
#include <sudo_rand.h>
#include <intercept.pb-c.h>
#include <exec_intercept.h>

#ifdef _PATH_SUDO_INTERCEPT
static union sudo_token_un intercept_token;
static in_port_t intercept_listen_port;
static struct intercept_closure *accept_closure;
static void intercept_accept_cb(int fd, int what, void *v);
static void intercept_cb(int fd, int what, void *v);

/*
 * Enable the closure->ev event with the specified events and callback,
 * and set the connection state to new_state if it is valid.
 * Returns true on success, else false.
 */
static bool
intercept_enable_event(int fd, short events, enum intercept_state new_state,
    sudo_ev_callback_t callback, struct intercept_closure *closure)
{
    int rc;
    debug_decl(intercept_enable_event, SUDO_DEBUG_EXEC);

    rc = sudo_ev_set(&closure->ev, fd, events, callback, closure);
    if (rc == -1 || sudo_ev_add(NULL, &closure->ev, NULL, false) == -1) {
	sudo_warn("%s", U_("unable to add event to queue"));
	debug_return_bool(false);
    }
    if (new_state != INVALID_STATE)
	closure->state = new_state;
    debug_return_bool(true);
}

static bool
enable_read_event(int fd, enum intercept_state new_state,
    sudo_ev_callback_t callback, struct intercept_closure *closure)
{
    return intercept_enable_event(fd, SUDO_EV_READ|SUDO_EV_PERSIST,
	new_state, callback, closure);
}

static bool
enable_write_event(int fd, sudo_ev_callback_t callback,
    struct intercept_closure *closure)
{
    return intercept_enable_event(fd, SUDO_EV_WRITE|SUDO_EV_PERSIST,
	INVALID_STATE, callback, closure);
}

/*
 * Create an intercept closure.
 * Returns an opaque pointer to the closure, which is also
 * passed to the event callback when not using ptrace(2).
 */
void *
intercept_setup(int fd, struct sudo_event_base *evbase,
    const struct command_details *details)
{
    struct intercept_closure *closure;
    debug_decl(intercept_setup, SUDO_DEBUG_EXEC);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"intercept fd %d\n", fd);

    closure = calloc(1, sizeof(*closure));
    if (closure == NULL) {
	sudo_warnx("%s", U_("unable to allocate memory"));
	goto bad;
    }
    closure->details = details;
    closure->listen_sock = -1;
    sudo_ev_set_base(&closure->ev, evbase);

    if (ISSET(details->flags, CD_USE_PTRACE)) {
	/*
	 * We can perform a policy check immediately using ptrace(2)
	 * but should ignore the execve(2) of the initial command
	 * (and sesh for SELinux RBAC).
	 */
	closure->state = RECV_POLICY_CHECK;
	closure->initial_command = 1;
	if (ISSET(details->flags, CD_RBAC_ENABLED))
	    closure->initial_command++;
    } else {
	/*
	 * Not using ptrace(2), use LD_PRELOAD (or its equivalent).  If
	 * we've already seen an InterceptHello, expect a policy check first.
	 */
	const int new_state = sudo_token_isset(intercept_token) ?
	    RECV_SECRET : RECV_HELLO_INITIAL;
	if (!enable_read_event(fd, new_state, intercept_cb, closure))
	    goto bad;
    }

    debug_return_ptr(closure);

bad:
    free(closure);
    debug_return_ptr(NULL);
}

/*
 * Reset intercept_closure so it can be re-used.
 */
void
intercept_closure_reset(struct intercept_closure *closure)
{
    size_t n;
    debug_decl(intercept_closure_reset, SUDO_DEBUG_EXEC);

    if (closure->listen_sock != -1) {
	close(closure->listen_sock);
	closure->listen_sock = -1;
    }
    free(closure->buf);
    free(closure->command);
    if (closure->run_argv != NULL) {
	for (n = 0; closure->run_argv[n] != NULL; n++)
	    free(closure->run_argv[n]);
	free(closure->run_argv);
    }
    if (closure->run_envp != NULL) {
	for (n = 0; closure->run_envp[n] != NULL; n++)
	    free(closure->run_envp[n]);
	free(closure->run_envp);
    }
    closure->errstr = NULL;
    closure->command = NULL;
    closure->run_argv = NULL;
    closure->run_envp = NULL;
    closure->buf = NULL;
    closure->len = 0;
    closure->off = 0;
    /* Does not currently reset token. */

    debug_return;
}

/*
 * Close intercept socket and free closure when we are done with
 * the connection.
 */
static void
intercept_connection_close(struct intercept_closure *closure)
{
    const int fd = sudo_ev_get_fd(&closure->ev);
    debug_decl(intercept_connection_close, SUDO_DEBUG_EXEC);

    sudo_ev_del(NULL, &closure->ev);
    close(fd);
    intercept_closure_reset(closure);
    free(closure);

    debug_return;
}

void
intercept_cleanup(struct exec_closure *ec)
{
    debug_decl(intercept_cleanup, SUDO_DEBUG_EXEC);

    if (accept_closure != NULL) {
	/* DSO-based intercept. */
	intercept_connection_close(accept_closure);
	accept_closure = NULL;
    } else if (ec->intercept != NULL) {
	/* ptrace-based intercept. */
	intercept_closure_reset(ec->intercept);
	free(ec->intercept);
	ec->intercept = NULL;
    }

    debug_return;
}

/*
 * Prepare to listen on localhost using an ephemeral port.
 * Sets intercept_token and intercept_listen_port as side effects.
 */
static bool
prepare_listener(struct intercept_closure *closure)
{
    struct sockaddr_in sin4;
    socklen_t sin4_len = sizeof(sin4);
    int sock;
    debug_decl(prepare_listener, SUDO_DEBUG_EXEC);

    /* Generate a random token. */
    do {
	arc4random_buf(&intercept_token, sizeof(intercept_token));
    } while (!sudo_token_isset(intercept_token));

    /* Create localhost listener socket (currently AF_INET only). */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
	sudo_warn("socket");
	goto bad;
    }
    memset(&sin4, 0, sizeof(sin4));
    sin4.sin_family = AF_INET;
    sin4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin4.sin_port = 0;
    if (bind(sock, (struct sockaddr *)&sin4, sizeof(sin4)) == -1) {
	sudo_warn("bind");
	goto bad;
    }
    if (getsockname(sock, (struct sockaddr *)&sin4, &sin4_len) == -1) {
	sudo_warn("getsockname");
	goto bad;
    }
    if (listen(sock, SOMAXCONN) == -1) {
	sudo_warn("listen");
	goto bad;
    }

    closure->listen_sock = sock;
    intercept_listen_port = ntohs(sin4.sin_port);
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"%s: listening on port %hu", __func__, intercept_listen_port);

    debug_return_bool(true);

bad:
    if (sock != -1)
	close(sock);
    debug_return_bool(false);
}

/*
 * Allocate a new command_info[] and update command and runcwd in it.
 * Fills in cmnd_out with a copy of the command if not NULL.
 * Returns the new command_info[] which the caller must free.
 */
static char **
update_command_info(char * const *old_command_info, const char *cmnd,
    const char *runcwd, char **cmnd_out, struct intercept_closure *closure)
{
    char **command_info;
    char * const *oci;
    size_t n;
    debug_decl(update_command_info, SUDO_DEBUG_EXEC);

    /* Rebuild command_info[] with new command and add a runcwd. */
    for (n = 0; old_command_info[n] != NULL; n++)
	continue;
    command_info = reallocarray(NULL, n + 3, sizeof(char *));
    if (command_info == NULL) {
	goto bad;
    }
    for (oci = old_command_info, n = 0; *oci != NULL; oci++) {
	const char *cp = *oci;
	switch (*cp) {
	case 'c':
	    if (strncmp(cp, "command=", sizeof("command=") - 1) == 0) {
		if (cmnd == NULL) {
		    /* No new command specified, use old value. */
		    cmnd = cp + sizeof("command=") - 1;
		}
		/* Filled in at the end. */
		continue;
	    }
	    break;
	case 'r':
	    if (strncmp(cp, "runcwd=", sizeof("runcwd=") - 1) == 0) {
		/* Filled in at the end. */
		continue;
	    }
	    break;
	}
	command_info[n] = strdup(cp);
	if (command_info[n] == NULL) {
	    goto bad;
	}
	n++;
    }

    /* Append new command. */
    if (cmnd == NULL) {
	closure->errstr = N_("command not set by the security policy");
	goto bad;
    }
    command_info[n] = sudo_new_key_val("command", cmnd);
    if (command_info[n] == NULL) {
	goto oom;
    }
    n++;

    /* Append actual runcwd. */
    command_info[n] = sudo_new_key_val("runcwd", runcwd ? runcwd : "unknown");
    if (command_info[n] == NULL) {
	goto oom;
    }
    n++;

    command_info[n] = NULL;

    if (cmnd_out != NULL) {
	*cmnd_out = strdup(cmnd);
	if (*cmnd_out == NULL) {
	    goto oom;
	}
    }
    debug_return_ptr(command_info);

oom:
    closure->errstr = N_("unable to allocate memory");

bad:
    if (command_info != NULL) {
	for (n = 0; command_info[n] != NULL; n++) {
	    free(command_info[n]);
	}
	free(command_info);
    }
    debug_return_ptr(NULL);
}

/*
 * Perform a policy check for the given command.
 * While argv must be NULL-terminated, envp need not be.
 * Sets closure->state to the result of the policy check before returning.
 * Return false on error, else true.
 */
bool
intercept_check_policy(const char *command, int argc, char **argv, int envc,
    char **envp, const char *runcwd, int *oldcwd, void *v)
{
    struct intercept_closure *closure = v;
    char **command_info = NULL;
    char **command_info_copy = NULL;
    char **user_env_out = NULL;
    char **run_argv = NULL;
    int rc, saved_dir = -1;
    size_t i;
    bool ret = true;
    struct stat sb;
    debug_decl(intercept_check_policy, SUDO_DEBUG_EXEC);

    /* Change to runcwd before the policy check if necessary. */
    if (*command != '/') {
	if (runcwd == NULL || (saved_dir = open(".", O_RDONLY)) == -1 ||
		chdir(runcwd) == -1) {
	    if (runcwd == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "relative command path but no runcwd specified");
	    } else if (saved_dir == -1) {
		sudo_debug_printf(
		    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		    "unable to open current directory for reading");
	    } else {
		sudo_debug_printf(
		    SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		    "unable to chdir for %s", runcwd);
	    }
	    if (ISSET(closure->details->flags, CD_INTERCEPT)) {
		/* Inability to change cwd is fatal in intercept mode. */
		if (closure->errstr == NULL)
		    closure->errstr = N_("command rejected by policy");
		audit_reject(policy_plugin.name, SUDO_POLICY_PLUGIN,
		    closure->errstr, closure->details->info);
		closure->state = POLICY_REJECT;
		goto done;
	    }
	}
    }

    /*
     * Short-circuit the policy check if the command doesn't exist.
     * Otherwise, both sudo and the shell will report the error.
     */
    if (stat(command, &sb) == -1) {
	closure->errstr = NULL;
	closure->state = POLICY_ERROR;
	goto done;
    }

    if (ISSET(closure->details->flags, CD_INTERCEPT)) {
	/* We don't currently have a good way to validate the environment. */
	sudo_debug_set_active_instance(policy_plugin.debug_instance);
	rc = policy_plugin.u.policy->check_policy(argc, argv, NULL,
	    &command_info, &run_argv, &user_env_out, &closure->errstr);
	sudo_debug_set_active_instance(sudo_debug_instance);
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "check_policy returns %d", rc);

	switch (rc) {
	case 1:
	    /* Rebuild command_info[] with runcwd and extract command. */
	    command_info_copy = update_command_info(command_info, NULL,
		runcwd, &closure->command, closure);
	    if (command_info_copy == NULL)
		goto oom;
	    command_info = command_info_copy;
	    closure->state = POLICY_ACCEPT;
	    break;
	case 0:
	    if (closure->errstr == NULL)
		closure->errstr = N_("command rejected by policy");
	    audit_reject(policy_plugin.name, SUDO_POLICY_PLUGIN,
		closure->errstr, command_info);
	    closure->state = POLICY_REJECT;
	    goto done;
	default:
	    /* Plugin error? */
	    goto bad;
	}
    } else {
	/* No actual policy check, just logging child processes. */
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "not checking policy, audit only");
	closure->command = strdup(command);
	if (closure->command == NULL)
	    goto oom;

	/* Rebuild command_info[] with new command and runcwd. */
	command_info_copy = update_command_info(closure->details->info,
	    command, runcwd, NULL, closure);
	if (command_info_copy == NULL)
	    goto oom;
	command_info = command_info_copy;
	closure->state = POLICY_ACCEPT;
	run_argv = argv;
    }

    if (sudo_debug_needed(SUDO_DEBUG_INFO)) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "run_command: %s", closure->command);
	for (i = 0; command_info[i] != NULL; i++) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"command_info[%zu]: %s", i, command_info[i]);
	}
	for (i = 0; run_argv[i] != NULL; i++) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"run_argv[%zu]: %s", i, run_argv[i]);
	}
    }

    /* Make a copy of run_argv, it may share contents of argv. */
    for (i = 0; run_argv[i] != NULL; i++)
	continue;
    closure->run_argv = reallocarray(NULL, i + 1, sizeof(char *));
    if (closure->run_argv == NULL)
	goto oom;
    for (i = 0; run_argv[i] != NULL; i++) {
	closure->run_argv[i] = strdup(run_argv[i]);
	if (closure->run_argv[i] == NULL)
	    goto oom;
    }
    closure->run_argv[i] = NULL;

    /* Make a copy of envp, which may not be NULL-terminated. */
    closure->run_envp = reallocarray(NULL, (size_t)envc + 1, sizeof(char *));
    if (closure->run_envp == NULL)
	goto oom;
    for (i = 0; i < (size_t)envc; i++) {
	closure->run_envp[i] = strdup(envp[i]);
	if (closure->run_envp[i] == NULL)
	    goto oom;
    }
    closure->run_envp[i] = NULL;

    if (ISSET(closure->details->flags, CD_INTERCEPT)) {
	audit_accept(policy_plugin.name, SUDO_POLICY_PLUGIN, command_info,
		closure->run_argv, closure->run_envp);

	/* Call approval plugins and audit the result. */
	if (!approval_check(command_info, closure->run_argv, closure->run_envp)) {
	    if (closure->errstr == NULL)
		closure->errstr = N_("approval plugin error");
	    closure->state = POLICY_REJECT;
	    goto done;
	}
    }

    /* Audit the event again for the sudo front-end. */
    audit_accept("sudo", SUDO_FRONT_END, command_info, closure->run_argv,
	closure->run_envp);

    goto done;

oom:
    closure->errstr = N_("unable to allocate memory");

bad:
    if (saved_dir != -1) {
	if (fchdir(saved_dir) == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		"%s: unable to restore saved cwd", __func__);
	}
	close(saved_dir);
	saved_dir = -1;
    }
    if (closure->errstr == NULL)
	closure->errstr = N_("policy plugin error");
    audit_error(policy_plugin.name, SUDO_POLICY_PLUGIN, closure->errstr,
	command_info ? command_info : closure->details->info);
    closure->state = POLICY_ERROR;
    ret = false;

done:
    if (command_info_copy != NULL) {
	for (i = 0; command_info_copy[i] != NULL; i++) {
	    free(command_info_copy[i]);
	}
	free(command_info_copy);
    }
    *oldcwd = saved_dir;

    debug_return_bool(ret);
}

static bool
intercept_check_policy_req(PolicyCheckRequest *req,
    struct intercept_closure *closure)
{
    char **argv = NULL;
    bool ret = false;
    int oldcwd = -1;
    size_t n;
    debug_decl(intercept_check_policy_req, SUDO_DEBUG_EXEC);

    if (req->command == NULL || req->n_argv > INT_MAX || req->n_envp > INT_MAX) {
	closure->errstr = N_("invalid PolicyCheckRequest");
	goto done;
    }

    if (sudo_debug_needed(SUDO_DEBUG_INFO)) {
	sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	    "req_command: %s", req->command);
	for (n = 0; n < req->n_argv; n++) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"req_argv[%zu]: %s", n, req->argv[n]);
	}
    }

    /* If argv is empty, reserve an extra slot for the command. */
    if (req->n_argv == 0)
	req->n_argv = 1;

    /*
     * Rebuild argv from PolicyCheckReq so it is NULL-terminated.
     * The plugin API requires us to pass the pathname to exec in argv[0].
     */
    argv = reallocarray(NULL, req->n_argv + 1, sizeof(char *));
    if (argv == NULL) {
	closure->errstr = N_("unable to allocate memory");
	goto done;
    }
    argv[0] = req->command;
    for (n = 1; n < req->n_argv; n++) {
	argv[n] = req->argv[n];
    }
    argv[n] = NULL;

    ret = intercept_check_policy(req->command, (int)req->n_argv, argv,
	(int)req->n_envp, req->envp, req->cwd, &oldcwd, closure);

done:
    if (oldcwd != -1) {
	if (fchdir(oldcwd) == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		"%s: unable to restore saved cwd", __func__);
	}
	close(oldcwd);
    }

    free(argv);

    debug_return_bool(ret);
}

/*
 * Read token from sudo_intercept.so and verify w/ intercept_token.
 * Returns true on success, false on mismatch and -1 on error.
 */
static int
intercept_verify_token(int fd, struct intercept_closure *closure)
{
    ssize_t nread;
    debug_decl(intercept_verify_token, SUDO_DEBUG_EXEC);

    nread = recv(fd, closure->token.u8 + closure->off,
	sizeof(closure->token) - closure->off, 0);
    switch (nread) {
    case 0:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "EOF reading token");
	debug_return_int(false);
    case -1:
	debug_return_int(-1);
    default:
	if (nread + closure->off == sizeof(closure->token))
	    break;
	/* partial read, update offset and try again */
	closure->off += (uint32_t)nread;
	errno = EAGAIN;
	debug_return_int(-1);
    }

    closure->off = 0;
    if (memcmp(&closure->token, &intercept_token, sizeof(closure->token)) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "token mismatch: got 0x%8x%8x%8x%8x, expected 0x%8x%8x%8x%8x",
	    closure->token.u32[3], closure->token.u32[2],
	    closure->token.u32[1], closure->token.u32[0],
	    intercept_token.u32[3], intercept_token.u32[2],
	    intercept_token.u32[1], intercept_token.u32[0]);
	debug_return_int(false);
    }
    debug_return_int(true);
}

/*
 * Read a message from sudo_intercept.so and act on it.
 */
static bool
intercept_read(int fd, struct intercept_closure *closure)
{
    InterceptRequest *req = NULL;
    bool ret = false;
    ssize_t nread;
    debug_decl(intercept_read, SUDO_DEBUG_EXEC);

    if (closure->state == RECV_SECRET) {
	switch (intercept_verify_token(fd, closure)) {
	case true:
	    closure->state = RECV_POLICY_CHECK;
	    break;
	case false:
	    goto done;
	default:
	    if (errno == EINTR || errno == EAGAIN) {
		sudo_debug_printf(
		    SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		    "reading intercept token");
		debug_return_bool(true);
	    }
	    sudo_warn("recv");
	    goto done;
	}
    }

    if (closure->len == 0) {
	uint32_t req_len;

	/* Read message size (uint32_t in host byte order). */
	nread = recv(fd, &req_len, sizeof(req_len), 0);
	if (nread != sizeof(req_len)) {
	    if (nread == -1) {
		if (errno == EINTR || errno == EAGAIN) {
		    sudo_debug_printf(
			SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
			"reading intercept message size");
		    debug_return_bool(true);
		}
		sudo_warn("recv");
	    }
	    goto done;
	}

	if (req_len == 0) {
	    /* zero-length message is possible */
	    goto unpack;
	}
	if (req_len > MESSAGE_SIZE_MAX) {
	    sudo_warnx(U_("client request too large: %zu"), (size_t)req_len);
	    goto done;
	}
	if ((closure->buf = malloc(req_len)) == NULL) {
	    sudo_warnx("%s", U_("unable to allocate memory"));
	    goto done;
	}
	closure->len = req_len;
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: expecting %u bytes from client",
	    __func__, closure->len);
    }

    nread = recv(fd, closure->buf + closure->off, closure->len - closure->off,
	0);
    switch (nread) {
    case 0:
	/* EOF, other side must have exited. */
	goto done;
    case -1:
	if (errno == EINTR || errno == EAGAIN) {
	    sudo_debug_printf(
		SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		"reading intercept message");
	    debug_return_bool(true);
	}
	sudo_warn("recv");
	goto done;
    default:
	closure->off += (uint32_t)nread;
	break;
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: received %zd bytes from client",
	__func__, nread);

    if (closure->off != closure->len) {
	/* Partial read. */
	debug_return_bool(true);
    }

unpack:
    req = intercept_request__unpack(NULL, closure->len, closure->buf);
    if (req == NULL) {
	sudo_warnx(U_("unable to unpack %s size %zu"), "InterceptRequest",
	    (size_t)closure->len);
	goto done;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: finished receiving %u bytes from client", __func__, closure->len);
    sudo_ev_del(NULL, &closure->ev);
    free(closure->buf);
    closure->buf = NULL;
    closure->len = 0;
    closure->off = 0;

    switch (req->type_case) {
    case INTERCEPT_REQUEST__TYPE_POLICY_CHECK_REQ:
	if (closure->state != RECV_POLICY_CHECK) {
	    /* Only a single policy check request is allowed. */
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"state mismatch, expected RECV_POLICY_CHECK (%d), got %d",
		RECV_POLICY_CHECK, closure->state);
	    goto done;
	}

	ret = intercept_check_policy_req(req->u.policy_check_req, closure);
	if (!ret)
	    goto done;
	if (!ISSET(closure->details->flags, CD_INTERCEPT)) {
	    /* Just logging, re-use event to read next InterceptHello. */
	    ret = enable_read_event(fd, RECV_HELLO, intercept_cb, closure);
	    goto done;
	}
	break;
    case INTERCEPT_REQUEST__TYPE_HELLO:
	switch (closure->state) {
	case RECV_HELLO_INITIAL:
	    if (!prepare_listener(closure))
		goto done;
	    break;
	case RECV_HELLO:
	    break;
	default:
	    /* Only accept hello on a socket with an accepted command. */
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"got InterceptHello without an accepted command");
	    goto done;
	}
	break;
    default:
	sudo_warnx(U_("unexpected type_case value %d in %s from %s"),
	    req->type_case, "InterceptRequest", "sudo_intercept.so");
	goto done;
    }

    /* Switch event to write mode for the reply. */
    if (!enable_write_event(fd, intercept_cb, closure))
	goto done;

    ret = true;

done:
    intercept_request__free_unpacked(req, NULL);
    debug_return_bool(ret);
}

static bool
fmt_intercept_response(InterceptResponse *resp,
    struct intercept_closure *closure)
{
    uint32_t resp_len;
    bool ret = false;
    debug_decl(fmt_intercept_response, SUDO_DEBUG_EXEC);

    closure->len = (uint32_t)intercept_response__get_packed_size(resp);
    if (closure->len > MESSAGE_SIZE_MAX) {
	sudo_warnx(U_("server message too large: %zu"), (size_t)closure->len);
	goto done;
    }

    /* Wire message size is used for length encoding, precedes message. */
    resp_len = closure->len;
    closure->len += sizeof(resp_len);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"size + InterceptResponse %zu bytes", (size_t)closure->len);

    if ((closure->buf = malloc(closure->len)) == NULL) {
	sudo_warnx("%s", U_("unable to allocate memory"));
	goto done;
    }
    memcpy(closure->buf, &resp_len, sizeof(resp_len));
    intercept_response__pack(resp, closure->buf + sizeof(resp_len));

    ret = true;

done:
    debug_return_bool(ret);
}

static bool
fmt_hello_response(struct intercept_closure *closure)
{
    HelloResponse hello_resp = HELLO_RESPONSE__INIT;
    InterceptResponse resp = INTERCEPT_RESPONSE__INIT;
    debug_decl(fmt_hello_response, SUDO_DEBUG_EXEC);

    hello_resp.portno = intercept_listen_port;
    hello_resp.token_lo = intercept_token.u64[0];
    hello_resp.token_hi = intercept_token.u64[1];
    hello_resp.log_only = !ISSET(closure->details->flags, CD_INTERCEPT);

    resp.u.hello_resp = &hello_resp;
    resp.type_case = INTERCEPT_RESPONSE__TYPE_HELLO_RESP;

    debug_return_bool(fmt_intercept_response(&resp, closure));
}

static bool
fmt_accept_message(struct intercept_closure *closure)
{
    PolicyAcceptMessage msg = POLICY_ACCEPT_MESSAGE__INIT;
    InterceptResponse resp = INTERCEPT_RESPONSE__INIT;
    size_t n;
    debug_decl(fmt_accept_message, SUDO_DEBUG_EXEC);

    msg.run_command = closure->command;
    msg.run_argv = closure->run_argv;
    for (n = 0; closure->run_argv[n] != NULL; n++)
	continue;
    msg.n_run_argv = n;
    msg.run_envp = closure->run_envp;
    for (n = 0; closure->run_envp[n] != NULL; n++)
	continue;
    msg.n_run_envp = n;

    resp.u.accept_msg = &msg;
    resp.type_case = INTERCEPT_RESPONSE__TYPE_ACCEPT_MSG;

    debug_return_bool(fmt_intercept_response(&resp, closure));
}

static bool
fmt_reject_message(struct intercept_closure *closure)
{
    PolicyRejectMessage msg = POLICY_REJECT_MESSAGE__INIT;
    InterceptResponse resp = INTERCEPT_RESPONSE__INIT;
    debug_decl(fmt_reject_message, SUDO_DEBUG_EXEC);

    msg.reject_message = (char *)closure->errstr;

    resp.u.reject_msg = &msg;
    resp.type_case = INTERCEPT_RESPONSE__TYPE_REJECT_MSG;

    debug_return_bool(fmt_intercept_response(&resp, closure));
}

static bool
fmt_error_message(struct intercept_closure *closure)
{
    PolicyErrorMessage msg = POLICY_ERROR_MESSAGE__INIT;
    InterceptResponse resp = INTERCEPT_RESPONSE__INIT;
    debug_decl(fmt_error_message, SUDO_DEBUG_EXEC);

    msg.error_message = (char *)closure->errstr;

    resp.u.error_msg = &msg;
    resp.type_case = INTERCEPT_RESPONSE__TYPE_ERROR_MSG;

    debug_return_bool(fmt_intercept_response(&resp, closure));
}

/*
 * Write a response to sudo_intercept.so.
 */
static bool
intercept_write(int fd, struct intercept_closure *closure)
{
    bool ret = false;
    ssize_t nwritten;
    debug_decl(intercept_write, SUDO_DEBUG_EXEC);

    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO, "state %d",
	closure->state);

    if (closure->len == 0) {
	/* Format new message. */
	switch (closure->state) {
	    case RECV_HELLO_INITIAL:
	    case RECV_HELLO:
		if (!fmt_hello_response(closure))
		    goto done;
		break;
	    case POLICY_ACCEPT:
		if (!fmt_accept_message(closure))
		    goto done;
		break;
	    case POLICY_REJECT:
		if (!fmt_reject_message(closure))
		    goto done;
		break;
	    default:
		if (!fmt_error_message(closure))
		    goto done;
		break;
	}
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: sending %u bytes to client",
	__func__, closure->len - closure->off);
    nwritten = send(fd, closure->buf + closure->off,
	closure->len - closure->off, 0);
    if (nwritten == -1) {
	if (errno == EINTR || errno == EAGAIN) {
	    sudo_debug_printf(
		SUDO_DEBUG_WARN|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
		"writing intercept message");
	    debug_return_bool(true);
	}
	sudo_warn("send");
	goto done;
    }
    closure->off += (uint32_t)nwritten;

    if (closure->off != closure->len) {
	/* Partial write. */
	debug_return_bool(true);
    }

    sudo_debug_printf(SUDO_DEBUG_INFO,
	"%s: sent %u bytes to client", __func__, closure->len);
    sudo_ev_del(NULL, &closure->ev);
    free(closure->buf);
    closure->buf = NULL;
    closure->len = 0;
    closure->off = 0;

    switch (closure->state) {
    case RECV_HELLO_INITIAL:
	/* Re-use the listener event. */
	close(fd);
	if (!enable_read_event(closure->listen_sock, RECV_CONNECTION,
		intercept_accept_cb, closure))
	    goto done;
	closure->listen_sock = -1;
	closure->state = RECV_CONNECTION;
	accept_closure = closure;
	break;
    case POLICY_ACCEPT:
	/* Re-use event to read InterceptHello from sudo_intercept.so ctor. */
	if (!enable_read_event(fd, RECV_HELLO, intercept_cb, closure))
	    goto done;
	break;
    default:
	/* Done with this connection. */
	intercept_connection_close(closure);
    }

    ret = true;

done:
    debug_return_bool(ret);
}

static void
intercept_cb(int fd, int what, void *v)
{
    struct intercept_closure *closure = v;
    bool success = false;
    debug_decl(intercept_cb, SUDO_DEBUG_EXEC);

    switch (what) {
    case SUDO_EV_READ:
	success = intercept_read(fd, closure);
	break;
    case SUDO_EV_WRITE:
	success = intercept_write(fd, closure);
	break;
    default:
	sudo_warnx("%s: unexpected event type %d", __func__, what);
	break;
    }

    if (!success)
	intercept_connection_close(closure);

    debug_return;
}

/*
 * Accept a new connection from the client register a new event for it.
 */
static void
intercept_accept_cb(int fd, int what, void *v)
{
    struct intercept_closure *closure = v;
    struct sudo_event_base *evbase = sudo_ev_get_base(&closure->ev);
    struct sockaddr_in sin4;
    socklen_t sin4_len = sizeof(sin4);
    int client_sock, flags, on = 1;
    debug_decl(intercept_accept_cb, SUDO_DEBUG_EXEC);

    if (closure->state != RECV_CONNECTION) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "state mismatch, expected RECV_CONNECTION (%d), got %d",
	    RECV_CONNECTION, closure->state);
	intercept_connection_close(closure);
	accept_closure = NULL;
	debug_return;
    }

    client_sock = accept(fd, (struct sockaddr *)&sin4, &sin4_len);
    if (client_sock == -1) {
	sudo_warn("accept");
	goto bad;
    }
    flags = fcntl(client_sock, F_GETFL, 0);
    if (flags != -1)
	(void)fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);

    /* Send data immediately, we need low latency IPC. */
    (void)setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

    /*
     * Create a new intercept closure and register an event for client_sock.
     */
    if (intercept_setup(client_sock, evbase, closure->details) == NULL) {
	goto bad;
    }

    debug_return;

bad:
    if (client_sock != -1)
	close(client_sock);
    debug_return;
}
#else /* _PATH_SUDO_INTERCEPT */
void *
intercept_setup(int fd, struct sudo_event_base *evbase,
    const struct command_details *details)
{
    debug_decl(intercept_setup, SUDO_DEBUG_EXEC);

    /* Intercept support not compiled in. */

    debug_return_ptr(NULL);
}

void
intercept_cleanup(struct exec_closure *ec)
{
    debug_decl(intercept_cleanup, SUDO_DEBUG_EXEC);

    /* Intercept support not compiled in. */

    debug_return;
}
#endif /* _PATH_SUDO_INTERCEPT */
