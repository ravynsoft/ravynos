/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDOERS_LDAP_CONF_H
#define SUDOERS_LDAP_CONF_H

/*
 * Configuration data types.
 * When adding a new data type, be sure to update sudo_ldap_parse_keyword()
 * and sudo_ldap_set_options_table().
 */
#define CONF_BOOL		0
#define CONF_INT		1
#define CONF_STR		2
#define CONF_LIST_STR		4
#define CONF_DEREF_VAL		5
#define CONF_REQCERT_VAL	6

#define SUDO_LDAP_CLEAR		0
#define SUDO_LDAP_SSL		1
#define SUDO_LDAP_STARTTLS	2

struct ldap_config_table {
    const char *conf_str;	/* config file string */
    int type;			/* CONF_* value, see above */
    int opt_val;		/* LDAP_OPT_* (or -1 for sudo internal) */
    void *valp;			/* pointer into ldap_conf */
};

struct ldap_config_str {
    STAILQ_ENTRY(ldap_config_str) entries;
    char val[];
};
STAILQ_HEAD(ldap_config_str_list, ldap_config_str);

/* LDAP configuration structure */
struct ldap_config {
    int port;
    int version;
    int debug;
    int ldap_debug;
    int tls_checkpeer;
    int tls_reqcert;
    int timelimit;
    int timeout;
    int bind_timelimit;
    int use_sasl;
    int rootuse_sasl;
    int ssl_mode;
    int timed;
    int deref;
    int netgroup_query;
    char *host;
    struct ldap_config_str_list uri;
    char *binddn;
    char *bindpw;
    char *rootbinddn;
    struct ldap_config_str_list base;
    struct ldap_config_str_list netgroup_base;
    char *search_filter;
    char *netgroup_search_filter;
    char *ssl;
    char *tls_cacertfile;
    char *tls_cacertdir;
    char *tls_random_file;
    char *tls_cipher_suite;
    char *tls_certfile;
    char *tls_keyfile;
    char *tls_keypw;
    char *sasl_mech;
    char *sasl_auth_id;
    char *rootsasl_auth_id;
    char *sasl_secprops;
    char *krb5_ccname;
};

extern struct ldap_config ldap_conf;

struct sudoers_context;
const char *sudo_krb5_ccname_path(const char *old_ccname);
bool sudo_ldap_read_config(const struct sudoers_context *ctx);
int sudo_ldap_set_options_global(void);
int sudo_ldap_set_options_conn(LDAP *ld);

#endif /* SUDOERS_LDAP_CONF_H */
