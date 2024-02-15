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

#ifndef SUDO_EXEC_INTERCEPT_H
#define SUDO_EXEC_INTERCEPT_H

enum intercept_state {
    INVALID_STATE,
    RECV_HELLO_INITIAL,
    RECV_HELLO,
    RECV_SECRET,
    RECV_POLICY_CHECK,
    RECV_CONNECTION,
    POLICY_ACCEPT,
    POLICY_REJECT,
    POLICY_TEST,
    POLICY_ERROR
};

/* Closure for intercept_cb() */
struct intercept_closure {
    union sudo_token_un token;
    const struct command_details *details;
    struct sudo_event ev;
    const char *errstr;
    char *command;		/* dynamically allocated */
    char **run_argv;		/* owned by plugin */
    char **run_envp;		/* dynamically allocated */
    uint8_t *buf;		/* dynamically allocated */
    uint32_t len;
    uint32_t off;
    int listen_sock;
    enum intercept_state state;
    int initial_command;
};

void intercept_closure_reset(struct intercept_closure *closure);
bool intercept_check_policy(const char *command, int argc, char **argv, int envc, char **envp, const char *runcwd, int *oldcwd, void *closure);

#endif /* SUDO_EXEC_INTERCEPT_H */
