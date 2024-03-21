/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2007-2016, 2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_AUTH_H
#define SUDO_AUTH_H

/* Private auth function return values (rowhammer resistant).  */
#define AUTH_INTR		0x69d61fc8	/* 1101001110101100001111111001000 */
#define AUTH_NONINTERACTIVE	0x1629e037	/* 0010110001010011110000000110111 */

struct sudoers_context;
typedef struct sudo_auth {
    unsigned int flags;		/* various flags, see below */
    int status;			/* status from verify routine */
    const char *name;		/* name of the method as a string */
    void *data;			/* method-specific data pointer */
    int (*init)(const struct sudoers_context *ctx, struct passwd *pw, struct sudo_auth *auth);
    int (*setup)(const struct sudoers_context *ctx, struct passwd *pw, char **prompt, struct sudo_auth *auth);
    int (*verify)(const struct sudoers_context *ctx, struct passwd *pw, const char *p, struct sudo_auth *auth, struct sudo_conv_callback *callback);
    int (*approval)(const struct sudoers_context *ctx, struct passwd *pw, struct sudo_auth *auth, bool exempt);
    int (*cleanup)(const struct sudoers_context *ctx, struct passwd *pw, struct sudo_auth *auth, bool force);
    int (*begin_session)(const struct sudoers_context *ctx, struct passwd *pw, char **user_env[], struct sudo_auth *auth);
    int (*end_session)(struct sudo_auth *auth);
} sudo_auth;

/* Values for sudo_auth.flags.  */
#define FLAG_DISABLED		0x02U	/* method disabled */
#define FLAG_STANDALONE		0x04U	/* standalone auth method */
#define FLAG_ONEANDONLY		0x08U	/* one and only auth method */
#define FLAG_NONINTERACTIVE	0x10U	/* no user input allowed */

/* Shortcuts for using the flags above. */
#define IS_DISABLED(x)		((x)->flags & FLAG_DISABLED)
#define IS_STANDALONE(x)	((x)->flags & FLAG_STANDALONE)
#define IS_ONEANDONLY(x)	((x)->flags & FLAG_ONEANDONLY)
#define IS_NONINTERACTIVE(x)	((x)->flags & FLAG_NONINTERACTIVE)

/* Like tgetpass() but uses conversation function */
char *auth_getpass(const char *prompt, int type, struct sudo_conv_callback *callback);

/* Pointer to conversation function to use with auth_getpass(). */
extern sudo_conv_t sudo_conv;

/* Prototypes for standalone methods */
int bsdauth_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int bsdauth_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *prompt, sudo_auth *auth, struct sudo_conv_callback *callback);
int bsdauth_approval(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool exempt);
int bsdauth_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);
void bsdauth_set_style(const char *style);
int sudo_aix_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_aix_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_aix_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);
int sudo_fwtk_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_fwtk_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *prompt, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_fwtk_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);
int sudo_pam_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_pam_init_quiet(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_pam_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *prompt, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_pam_approval(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool exempt);
int sudo_pam_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);
int sudo_pam_begin_session(const struct sudoers_context *ctx, struct passwd *pw, char **user_env[], sudo_auth *auth);
int sudo_pam_end_session(sudo_auth *auth);
int sudo_securid_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_securid_setup(const struct sudoers_context *ctx, struct passwd *pw, char **prompt, sudo_auth *auth);
int sudo_securid_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_sia_setup(const struct sudoers_context *ctx, struct passwd *pw, char **prompt, sudo_auth *auth);
int sudo_sia_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *prompt, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_sia_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);
int sudo_sia_begin_session(const struct sudoers_context *ctx, struct passwd *pw, char **user_env[], sudo_auth *auth);

/* Prototypes for normal methods */
int sudo_afs_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_dce_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_krb5_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_krb5_setup(const struct sudoers_context *ctx, struct passwd *pw, char **prompt, sudo_auth *auth);
int sudo_krb5_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_krb5_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);
int sudo_passwd_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_passwd_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_passwd_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);
int sudo_rfc1938_setup(const struct sudoers_context *ctx, struct passwd *pw, char **prompt, sudo_auth *auth);
int sudo_rfc1938_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_secureware_init(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth);
int sudo_secureware_verify(const struct sudoers_context *ctx, struct passwd *pw, const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback);
int sudo_secureware_cleanup(const struct sudoers_context *ctx, struct passwd *pw, sudo_auth *auth, bool force);

/* Fields: name, flags, init, setup, verify, approval, cleanup, begin_sess, end_sess */
#define AUTH_ENTRY(n, f, i, s, v, a, c, b, e) \
	{ (f), AUTH_FAILURE, (n), NULL, (i), (s), (v), (a), (c) , (b), (e) },

#endif /* SUDO_AUTH_H */
