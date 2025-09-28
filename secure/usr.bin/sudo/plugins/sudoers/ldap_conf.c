/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2003-2023 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * This code is derived from software contributed by Aaron Spangler.
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

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef HAVE_LBER_H
# include <lber.h>
#endif
#include <ldap.h>
#if defined(HAVE_LDAP_SSL_H)
# include <ldap_ssl.h>
#elif defined(HAVE_MPS_LDAP_SSL_H)
# include <mps/ldap_ssl.h>
#endif

#include <sudoers.h>
#include <sudo_lbuf.h>
#include <sudo_ldap.h>
#include <sudo_ldap_conf.h>

/* Older Netscape LDAP SDKs don't prototype ldapssl_set_strength() */
#if defined(HAVE_LDAPSSL_SET_STRENGTH) && !defined(HAVE_LDAP_SSL_H) && !defined(HAVE_MPS_LDAP_SSL_H)
extern int ldapssl_set_strength(LDAP *ldap, int strength);
#endif

#if !defined(LDAP_OPT_NETWORK_TIMEOUT) && defined(LDAP_OPT_CONNECT_TIMEOUT)
# define LDAP_OPT_NETWORK_TIMEOUT LDAP_OPT_CONNECT_TIMEOUT
#endif

#ifndef LDAPS_PORT
# define LDAPS_PORT 636
#endif

/* Default search filter. */
#define DEFAULT_SEARCH_FILTER	"(objectClass=sudoRole)"

/* Default netgroup search filter. */
#define DEFAULT_NETGROUP_SEARCH_FILTER	"(objectClass=nisNetgroup)"

/* LDAP configuration structure */
struct ldap_config ldap_conf;

static struct ldap_config_table ldap_conf_global[] = {
    { "sudoers_debug", CONF_INT, -1, &ldap_conf.debug },
    { "host", CONF_STR, -1, &ldap_conf.host },
    { "port", CONF_INT, -1, &ldap_conf.port },
    { "ssl", CONF_STR, -1, &ldap_conf.ssl },
    { "sslpath", CONF_STR, -1, &ldap_conf.tls_certfile },
    { "uri", CONF_LIST_STR, -1, &ldap_conf.uri },
#ifdef LDAP_OPT_DEBUG_LEVEL
    { "debug", CONF_INT, LDAP_OPT_DEBUG_LEVEL, &ldap_conf.ldap_debug },
#endif
#ifdef LDAP_OPT_X_TLS_REQUIRE_CERT
    { "tls_checkpeer", CONF_BOOL, LDAP_OPT_X_TLS_REQUIRE_CERT,
	&ldap_conf.tls_checkpeer },
    { "tls_reqcert", CONF_REQCERT_VAL, LDAP_OPT_X_TLS_REQUIRE_CERT,
	&ldap_conf.tls_reqcert },
#else
    { "tls_checkpeer", CONF_BOOL, -1, &ldap_conf.tls_checkpeer },
#endif
#ifdef LDAP_OPT_X_TLS_CACERTFILE
    { "tls_cacertfile", CONF_STR, LDAP_OPT_X_TLS_CACERTFILE,
	&ldap_conf.tls_cacertfile },
    { "tls_cacert", CONF_STR, LDAP_OPT_X_TLS_CACERTFILE,
	&ldap_conf.tls_cacertfile },
#endif
#ifdef LDAP_OPT_X_TLS_CACERTDIR
    { "tls_cacertdir", CONF_STR, LDAP_OPT_X_TLS_CACERTDIR,
	&ldap_conf.tls_cacertdir },
#endif
#ifdef LDAP_OPT_X_TLS_RANDOM_FILE
    { "tls_randfile", CONF_STR, LDAP_OPT_X_TLS_RANDOM_FILE,
	&ldap_conf.tls_random_file },
#endif
#ifdef LDAP_OPT_X_TLS_CIPHER_SUITE
    { "tls_ciphers", CONF_STR, LDAP_OPT_X_TLS_CIPHER_SUITE,
	&ldap_conf.tls_cipher_suite },
#elif defined(LDAP_OPT_SSL_CIPHER)
    { "tls_ciphers", CONF_STR, LDAP_OPT_SSL_CIPHER,
	&ldap_conf.tls_cipher_suite },
#endif
#ifdef LDAP_OPT_X_TLS_CERTFILE
    { "tls_cert", CONF_STR, LDAP_OPT_X_TLS_CERTFILE,
	&ldap_conf.tls_certfile },
#else
    { "tls_cert", CONF_STR, -1, &ldap_conf.tls_certfile },
#endif
#ifdef LDAP_OPT_X_TLS_KEYFILE
    { "tls_key", CONF_STR, LDAP_OPT_X_TLS_KEYFILE,
	&ldap_conf.tls_keyfile },
#else
    { "tls_key", CONF_STR, -1, &ldap_conf.tls_keyfile },
#endif
#ifdef HAVE_LDAP_SSL_CLIENT_INIT
    { "tls_keypw", CONF_STR, -1, &ldap_conf.tls_keypw },
#endif
    { "binddn", CONF_STR, -1, &ldap_conf.binddn },
    { "bindpw", CONF_STR, -1, &ldap_conf.bindpw },
    { "rootbinddn", CONF_STR, -1, &ldap_conf.rootbinddn },
    { "sudoers_base", CONF_LIST_STR, -1, &ldap_conf.base },
    { "sudoers_timed", CONF_BOOL, -1, &ldap_conf.timed },
    { "sudoers_search_filter", CONF_STR, -1, &ldap_conf.search_filter },
    { "netgroup_base", CONF_LIST_STR, -1, &ldap_conf.netgroup_base },
    { "netgroup_search_filter", CONF_STR, -1, &ldap_conf.netgroup_search_filter },
    { "netgroup_query", CONF_BOOL, -1, &ldap_conf.netgroup_query },
#ifdef HAVE_LDAP_SASL_INTERACTIVE_BIND_S
    { "use_sasl", CONF_BOOL, -1, &ldap_conf.use_sasl },
    { "sasl_mech", CONF_STR, -1, &ldap_conf.sasl_mech },
    { "sasl_auth_id", CONF_STR, -1, &ldap_conf.sasl_auth_id },
    { "rootuse_sasl", CONF_BOOL, -1, &ldap_conf.rootuse_sasl },
    { "rootsasl_auth_id", CONF_STR, -1, &ldap_conf.rootsasl_auth_id },
    { "krb5_ccname", CONF_STR, -1, &ldap_conf.krb5_ccname },
#endif /* HAVE_LDAP_SASL_INTERACTIVE_BIND_S */
    { NULL }
};

static struct ldap_config_table ldap_conf_conn[] = {
#ifdef LDAP_OPT_PROTOCOL_VERSION
    { "ldap_version", CONF_INT, LDAP_OPT_PROTOCOL_VERSION,
	&ldap_conf.version },
#endif
#ifdef LDAP_OPT_NETWORK_TIMEOUT
    { "bind_timelimit", CONF_INT, -1 /* needs timeval, set manually */,
	&ldap_conf.bind_timelimit },
    { "network_timeout", CONF_INT, -1 /* needs timeval, set manually */,
	&ldap_conf.bind_timelimit },
#elif defined(LDAP_X_OPT_CONNECT_TIMEOUT)
    { "bind_timelimit", CONF_INT, LDAP_X_OPT_CONNECT_TIMEOUT,
	&ldap_conf.bind_timelimit },
    { "network_timeout", CONF_INT, LDAP_X_OPT_CONNECT_TIMEOUT,
	&ldap_conf.bind_timelimit },
#endif
    { "timelimit", CONF_INT, LDAP_OPT_TIMELIMIT, &ldap_conf.timelimit },
#ifdef LDAP_OPT_TIMEOUT
    { "timeout", CONF_INT, -1 /* needs timeval, set manually */,
	&ldap_conf.timeout },
#endif
#ifdef LDAP_OPT_DEREF
    { "deref", CONF_DEREF_VAL, LDAP_OPT_DEREF, &ldap_conf.deref },
#endif
#ifdef LDAP_OPT_X_SASL_SECPROPS
    { "sasl_secprops", CONF_STR, LDAP_OPT_X_SASL_SECPROPS,
	&ldap_conf.sasl_secprops },
#endif
    { NULL }
};

#ifdef HAVE_LDAP_CREATE
/*
 * Rebuild the hosts list and include a specific port for each host.
 * ldap_create() does not take a default port parameter so we must
 * append one if we want something other than LDAP_PORT.
 */
static bool
sudo_ldap_conf_add_ports(void)
{
    char *host, *last, *port, defport[13];
    char hostbuf[LINE_MAX * 2];
    int len;
    debug_decl(sudo_ldap_conf_add_ports, SUDOERS_DEBUG_LDAP);

    hostbuf[0] = '\0';
    len = snprintf(defport, sizeof(defport), ":%d", ldap_conf.port);
    if (len < 0 || len >= ssizeof(defport)) {
	sudo_warnx(U_("%s: port too large"), __func__);
	debug_return_bool(false);
    }

    for ((host = strtok_r(ldap_conf.host, " \t", &last)); host; (host = strtok_r(NULL, " \t", &last))) {
	if (hostbuf[0] != '\0')
	    CHECK_STRLCAT(hostbuf, " ", sizeof(hostbuf));
	CHECK_STRLCAT(hostbuf, host, sizeof(hostbuf));

	/* Append port if there is not one already. */
	if ((port = strrchr(host, ':')) == NULL ||
	    !isdigit((unsigned char)port[1])) {
	    CHECK_STRLCAT(hostbuf, defport, sizeof(hostbuf));
	}
    }

    free(ldap_conf.host);
    if ((ldap_conf.host = strdup(hostbuf)) == NULL)
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_bool(ldap_conf.host != NULL);

overflow:
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    debug_return_bool(false);
}
#endif

#ifndef HAVE_LDAP_INITIALIZE
/*
 * For each uri, convert to host:port pairs.  For ldaps:// enable SSL
 * Accepts: uris of the form ldap:/// or ldap://hostname:portnum/
 * where the trailing slash is optional.
 * Returns LDAP_SUCCESS on success, else non-zero.
 */
static int
sudo_ldap_parse_uri(const struct ldap_config_str_list *uri_list)
{
    const struct ldap_config_str *entry;
    char *buf, hostbuf[LINE_MAX];
    int nldap = 0, nldaps = 0;
    int ret = -1;
    debug_decl(sudo_ldap_parse_uri, SUDOERS_DEBUG_LDAP);

    hostbuf[0] = '\0';
    STAILQ_FOREACH(entry, uri_list, entries) {
	char *cp, *last, *uri;
	const char *host, *port;

	buf = strdup(entry->val);
	if (buf == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	for ((uri = strtok_r(buf, " \t", &last)); uri != NULL; (uri = strtok_r(NULL, " \t", &last))) {
	    if (strncasecmp(uri, "ldap://", 7) == 0) {
		nldap++;
		host = uri + 7;
	    } else if (strncasecmp(uri, "ldaps://", 8) == 0) {
		nldaps++;
		host = uri + 8;
	    } else {
		sudo_warnx(U_("unsupported LDAP uri type: %s"), uri);
		goto done;
	    }

	    /* trim optional trailing slash */
	    if ((cp = strrchr(host, '/')) != NULL && cp[1] == '\0') {
		*cp = '\0';
	    }

	    if (hostbuf[0] != '\0')
		CHECK_STRLCAT(hostbuf, " ", sizeof(hostbuf));

	    if (*host == '\0')
		host = "localhost";		/* no host specified, use localhost */

	    CHECK_STRLCAT(hostbuf, host, sizeof(hostbuf));

	    /* If using SSL and no port specified, add port 636 */
	    if (nldaps) {
		if ((port = strrchr(host, ':')) == NULL ||
		    !isdigit((unsigned char)port[1]))
		    CHECK_STRLCAT(hostbuf, ":636", sizeof(hostbuf));
	    }
	}

	if (nldaps != 0) {
	    if (nldap != 0) {
		sudo_warnx("%s", U_("unable to mix ldap and ldaps URIs"));
		goto done;
	    }
	    if (ldap_conf.ssl_mode == SUDO_LDAP_STARTTLS)
		sudo_warnx("%s", U_("starttls not supported when using ldaps"));
	    ldap_conf.ssl_mode = SUDO_LDAP_SSL;
	}
	free(buf);
    }
    buf = NULL;

    /* Store parsed URI(s) in host for ldap_create() or ldap_init(). */
    free(ldap_conf.host);
    if ((ldap_conf.host = strdup(hostbuf)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }

    ret = LDAP_SUCCESS;

done:
    free(buf);
    debug_return_int(ret);

overflow:
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    free(buf);
    debug_return_int(-1);
}
#endif /* HAVE_LDAP_INITIALIZE */

/*
 * Decode a secret if it is base64 encoded, else return NULL.
 */
static char *
sudo_ldap_decode_secret(const char *secret)
{
    unsigned char *result = NULL;
    size_t len, reslen;
    debug_decl(sudo_ldap_decode_secret, SUDOERS_DEBUG_LDAP);

    if (strncasecmp(secret, "base64:", sizeof("base64:") - 1) == 0) {
	/*
	 * Decode a base64 secret.  The decoded length is 3/4 the encoded
	 * length but padding may be missing so round up to a multiple of 4.
	 */
	secret += sizeof("base64:") - 1;
	reslen = ((strlen(secret) + 3) / 4 * 3);
	result = malloc(reslen + 1);
	if (result == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	} else {
	    len = base64_decode(secret, result, reslen);
	    if (len == (size_t)-1) {
		free(result);
		result = NULL;
	    } else {
		result[len] = '\0';
	    }
	}
    }
    debug_return_str((char *)result);
}

static void
sudo_ldap_read_secret(const char *path)
{
    FILE *fp;
    char *line = NULL;
    size_t linesize = 0;
    ssize_t len;
    debug_decl(sudo_ldap_read_secret, SUDOERS_DEBUG_LDAP);

    if ((fp = fopen(path, "r")) != NULL) {
	len = getdelim(&line, &linesize, '\n', fp);
	if (len != -1) {
	    /* trim newline */
	    while (len > 0 && line[len - 1] == '\n')
		line[--len] = '\0';
	    /* copy to bindpw and binddn */
	    free(ldap_conf.bindpw);
	    ldap_conf.bindpw = sudo_ldap_decode_secret(line);
	    if (ldap_conf.bindpw == NULL) {
		/* not base64 encoded, use directly */
		ldap_conf.bindpw = line;
		line = NULL;
	    }
	    free(ldap_conf.binddn);
	    ldap_conf.binddn = ldap_conf.rootbinddn;
	    ldap_conf.rootbinddn = NULL;
	}
	fclose(fp);
	free(line);
    }
    debug_return;
}

/*
 * Look up keyword in config tables.
 * Returns true if found, else false.
 */
static bool
sudo_ldap_parse_keyword(const struct sudoers_context *ctx, const char *keyword,
    const char *value, struct ldap_config_table *table)
{
    struct ldap_config_table *cur;
    const char *errstr;
    debug_decl(sudo_ldap_parse_keyword, SUDOERS_DEBUG_LDAP);

    /* Look up keyword in config tables */
    for (cur = table; cur->conf_str != NULL; cur++) {
	if (strcasecmp(keyword, cur->conf_str) == 0) {
	    switch (cur->type) {
	    case CONF_DEREF_VAL:
#ifdef LDAP_OPT_DEREF
		if (strcasecmp(value, "searching") == 0)
		    *(int *)(cur->valp) = LDAP_DEREF_SEARCHING;
		else if (strcasecmp(value, "finding") == 0)
		    *(int *)(cur->valp) = LDAP_DEREF_FINDING;
		else if (strcasecmp(value, "always") == 0)
		    *(int *)(cur->valp) = LDAP_DEREF_ALWAYS;
		else
		    *(int *)(cur->valp) = LDAP_DEREF_NEVER;
#endif /* LDAP_OPT_DEREF */
		break;
	    case CONF_REQCERT_VAL:
#ifdef LDAP_OPT_X_TLS_REQUIRE_CERT
		if (strcasecmp(value, "never") == 0)
		    *(int *)(cur->valp) = LDAP_OPT_X_TLS_NEVER;
		else if (strcasecmp(value, "allow") == 0)
		    *(int *)(cur->valp) = LDAP_OPT_X_TLS_ALLOW;
		else if (strcasecmp(value, "try") == 0)
		    *(int *)(cur->valp) = LDAP_OPT_X_TLS_TRY;
		else if (strcasecmp(value, "hard") == 0)
		    *(int *)(cur->valp) = LDAP_OPT_X_TLS_HARD;
		else if (strcasecmp(value, "demand") == 0)
		    *(int *)(cur->valp) = LDAP_OPT_X_TLS_DEMAND;
#endif /* LDAP_OPT_X_TLS_REQUIRE_CERT */
		break;
	    case CONF_BOOL:
		*(int *)(cur->valp) = sudo_strtobool(value) == true;
		break;
	    case CONF_INT:
		*(int *)(cur->valp) = (int)sudo_strtonum(value, INT_MIN, INT_MAX,
		    &errstr);
		if (errstr != NULL) {
		    sudo_warnx(U_("%s: %s: %s: %s"), ctx->settings.ldap_conf,
			keyword, value, U_(errstr));
		}
		break;
	    case CONF_STR:
		{
		    char *cp = NULL;

		    free(*(char **)(cur->valp));
		    if (*value && (cp = strdup(value)) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
			debug_return_bool(false);
		    }
		    *(char **)(cur->valp) = cp;
		    break;
		}
	    case CONF_LIST_STR:
		{
		    struct ldap_config_str_list *head;
		    struct ldap_config_str *str;
		    size_t len = strlen(value);

		    if (len > 0) {
			head = (struct ldap_config_str_list *)cur->valp;
			if ((str = malloc(sizeof(*str) + len + 1)) == NULL) {
			    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
			    debug_return_bool(false);
			}
			memcpy(str->val, value, len + 1);
			STAILQ_INSERT_TAIL(head, str, entries);
		    }
		}
		break;
	    default:
		sudo_warnx(
		    "internal error: unhandled CONF_ value %d for option %s",
		    cur->type, cur->conf_str);
		sudo_warnx(
		    "update %s to add missing support for CONF_ value %d",
		    __func__, cur->type);
		break;
	    }
	    debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

#ifdef HAVE_LDAP_SASL_INTERACTIVE_BIND_S
const char *
sudo_krb5_ccname_path(const char *old_ccname)
{
    const char *ccname = old_ccname;
    debug_decl(sudo_krb5_ccname_path, SUDOERS_DEBUG_LDAP);

    if (ccname == NULL)
	debug_return_const_str(NULL);

    /* Strip off leading FILE: or WRFILE: prefix. */
    switch (ccname[0]) {
	case 'F':
	case 'f':
	    if (strncasecmp(ccname, "FILE:", 5) == 0)
		ccname += 5;
	    break;
	case 'W':
	case 'w':
	    if (strncasecmp(ccname, "WRFILE:", 7) == 0)
		ccname += 7;
	    break;
    }
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"ccache %s -> %s", old_ccname, ccname);

    /* Credential cache must be a fully-qualified path name. */
    debug_return_const_str(*ccname == '/' ? ccname : NULL);
}

static bool
sudo_check_krb5_ccname(const char *ccname)
{
    int fd;
    const char *ccname_path;
    debug_decl(sudo_check_krb5_ccname, SUDOERS_DEBUG_LDAP);

    /* Strip off prefix to get path name. */
    ccname_path = sudo_krb5_ccname_path(ccname);
    if (ccname_path == NULL) {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "unsupported krb5 credential cache path: %s", ccname);
	debug_return_bool(false);
    }
    /* Make sure credential cache is fully-qualified and exists. */
    fd = open(ccname_path, O_RDONLY|O_NONBLOCK, 0);
    if (fd == -1) {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "unable to open krb5 credential cache: %s", ccname_path);
	debug_return_bool(false);
    }
    close(fd);
    sudo_debug_printf(SUDO_DEBUG_INFO,
	"using krb5 credential cache: %s", ccname_path);
    debug_return_bool(true);
}
#endif /* HAVE_LDAP_SASL_INTERACTIVE_BIND_S */

bool
sudo_ldap_read_config(const struct sudoers_context *ctx)
{
    char *cp, *keyword, *value, *line = NULL;
    struct ldap_config_str *conf_str;
    size_t linesize = 0;
    FILE *fp;
    debug_decl(sudo_ldap_read_config, SUDOERS_DEBUG_LDAP);

    /* defaults */
    ldap_conf.version = 3;
    ldap_conf.port = -1;
    ldap_conf.tls_checkpeer = -1;
    ldap_conf.tls_reqcert = -1;
    ldap_conf.timelimit = -1;
    ldap_conf.timeout = -1;
    ldap_conf.bind_timelimit = -1;
    ldap_conf.use_sasl = -1;
    ldap_conf.rootuse_sasl = -1;
    ldap_conf.deref = -1;
    ldap_conf.search_filter = strdup(DEFAULT_SEARCH_FILTER);
    ldap_conf.netgroup_search_filter = strdup(DEFAULT_NETGROUP_SEARCH_FILTER);
    ldap_conf.netgroup_query = true;
    STAILQ_INIT(&ldap_conf.uri);
    STAILQ_INIT(&ldap_conf.base);
    STAILQ_INIT(&ldap_conf.netgroup_base);

    if (ldap_conf.search_filter == NULL || ldap_conf.netgroup_search_filter == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }

    if ((fp = fopen(ctx->settings.ldap_conf, "r")) == NULL)
	debug_return_bool(false);

    while (sudo_parseln(&line, &linesize, NULL, fp, PARSELN_COMM_BOL|PARSELN_CONT_IGN) != -1) {
	if (*line == '\0')
	    continue;		/* skip empty line */

	/* split into keyword and value */
	keyword = cp = line;
	while (*cp && !isblank((unsigned char) *cp))
	    cp++;
	if (*cp)
	    *cp++ = '\0';	/* terminate keyword */

	/* skip whitespace before value */
	while (isblank((unsigned char) *cp))
	    cp++;
	value = cp;

	/* Look up keyword in config tables */
	if (!sudo_ldap_parse_keyword(ctx, keyword, value, ldap_conf_global))
	    sudo_ldap_parse_keyword(ctx, keyword, value, ldap_conf_conn);
    }
    free(line);
    fclose(fp);

    if (!ldap_conf.host) {
	ldap_conf.host = strdup("localhost");
	if (ldap_conf.host == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_bool(false);
	}
    }
    if (STAILQ_EMPTY(&ldap_conf.netgroup_base)) {
	/* netgroup_query is only valid in conjunction with netgroup_base */
	ldap_conf.netgroup_query = false;
    }

    DPRINTF1("LDAP Config Summary");
    DPRINTF1("===================");
    if (!STAILQ_EMPTY(&ldap_conf.uri)) {
	STAILQ_FOREACH(conf_str, &ldap_conf.uri, entries) {
	    DPRINTF1("uri              %s", conf_str->val);
	}
    } else {
	DPRINTF1("host             %s",
	    ldap_conf.host ? ldap_conf.host : "(NONE)");
	DPRINTF1("port             %d", ldap_conf.port);
    }
    DPRINTF1("ldap_version     %d", ldap_conf.version);

    if (!STAILQ_EMPTY(&ldap_conf.base)) {
	STAILQ_FOREACH(conf_str, &ldap_conf.base, entries) {
	    DPRINTF1("sudoers_base     %s", conf_str->val);
	}
    } else {
	DPRINTF1("sudoers_base     %s", "(NONE: LDAP disabled)");
    }
    if (ldap_conf.search_filter) {
	DPRINTF1("search_filter    %s", ldap_conf.search_filter);
    }
    if (!STAILQ_EMPTY(&ldap_conf.netgroup_base)) {
	STAILQ_FOREACH(conf_str, &ldap_conf.netgroup_base, entries) {
	    DPRINTF1("netgroup_base    %s", conf_str->val);
	}
	DPRINTF1("netgroup_query   %s",
	    ldap_conf.netgroup_query ? "(yes)" : "(no)");
    } else {
	DPRINTF1("netgroup_base %s", "(NONE: will use nsswitch)");
    }
    if (ldap_conf.netgroup_search_filter) {
        DPRINTF1("netgroup_search_filter %s", ldap_conf.netgroup_search_filter);
    }
    DPRINTF1("binddn           %s",
	ldap_conf.binddn ? ldap_conf.binddn : "(anonymous)");
    DPRINTF1("bindpw           %s",
	ldap_conf.bindpw ? ldap_conf.bindpw : "(anonymous)");
    if (ldap_conf.bind_timelimit > 0) {
	DPRINTF1("bind_timelimit   %d", ldap_conf.bind_timelimit);
    }
    if (ldap_conf.timelimit > 0) {
	DPRINTF1("timelimit        %d", ldap_conf.timelimit);
    }
    if (ldap_conf.deref != -1) {
	DPRINTF1("deref            %d", ldap_conf.deref);
    }
    DPRINTF1("ssl              %s", ldap_conf.ssl ? ldap_conf.ssl : "(no)");
    if (ldap_conf.tls_checkpeer != -1) {
	DPRINTF1("tls_checkpeer    %s",
	    ldap_conf.tls_checkpeer ? "(yes)" : "(no)");
    }
#ifdef LDAP_OPT_X_TLS_REQUIRE_CERT
    if (ldap_conf.tls_reqcert != -1) {
	DPRINTF1("tls_reqcert    %s",
	    ldap_conf.tls_reqcert == LDAP_OPT_X_TLS_NEVER ? "hard" :
	    ldap_conf.tls_reqcert == LDAP_OPT_X_TLS_ALLOW ? "allow" :
	    ldap_conf.tls_reqcert == LDAP_OPT_X_TLS_TRY ? "try" :
	    ldap_conf.tls_reqcert == LDAP_OPT_X_TLS_HARD ? "hard" :
	    ldap_conf.tls_reqcert == LDAP_OPT_X_TLS_DEMAND ? "demand" :
	    "unknown");
    }
#endif /* LDAP_OPT_X_TLS_REQUIRE_CERT */
    if (ldap_conf.tls_cacertfile != NULL) {
	DPRINTF1("tls_cacertfile   %s", ldap_conf.tls_cacertfile);
    }
    if (ldap_conf.tls_cacertdir != NULL) {
	DPRINTF1("tls_cacertdir    %s", ldap_conf.tls_cacertdir);
    }
    if (ldap_conf.tls_random_file != NULL) {
	DPRINTF1("tls_random_file  %s", ldap_conf.tls_random_file);
    }
    if (ldap_conf.tls_cipher_suite != NULL) {
	DPRINTF1("tls_cipher_suite %s", ldap_conf.tls_cipher_suite);
    }
    if (ldap_conf.tls_certfile != NULL) {
	DPRINTF1("tls_certfile     %s", ldap_conf.tls_certfile);
    }
    if (ldap_conf.tls_keyfile != NULL) {
	DPRINTF1("tls_keyfile      %s", ldap_conf.tls_keyfile);
    }
#ifdef HAVE_LDAP_SASL_INTERACTIVE_BIND_S
    if (ldap_conf.use_sasl != -1) {
	if (ldap_conf.sasl_mech == NULL) {
	    /* Default mechanism is GSSAPI. */
	    ldap_conf.sasl_mech = strdup("GSSAPI");
	    if (ldap_conf.sasl_mech == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		debug_return_bool(false);
	    }
	}
	DPRINTF1("use_sasl         %s", ldap_conf.use_sasl ? "yes" : "no");
	DPRINTF1("sasl_mech        %s", ldap_conf.sasl_mech);
	DPRINTF1("sasl_auth_id     %s",
	    ldap_conf.sasl_auth_id ? ldap_conf.sasl_auth_id : "(NONE)");
	DPRINTF1("rootuse_sasl     %d",
	    ldap_conf.rootuse_sasl);
	DPRINTF1("rootsasl_auth_id %s",
	    ldap_conf.rootsasl_auth_id ? ldap_conf.rootsasl_auth_id : "(NONE)");
	DPRINTF1("sasl_secprops    %s",
	    ldap_conf.sasl_secprops ? ldap_conf.sasl_secprops : "(NONE)");
	DPRINTF1("krb5_ccname      %s",
	    ldap_conf.krb5_ccname ? ldap_conf.krb5_ccname : "(NONE)");
    }
#endif
    DPRINTF1("===================");

    if (STAILQ_EMPTY(&ldap_conf.base))
	debug_return_bool(false);	/* if no base is defined, ignore LDAP */

    if (ldap_conf.bind_timelimit > 0)
	ldap_conf.bind_timelimit *= 1000;	/* convert to ms */

    /*
     * Interpret SSL option
     */
    if (ldap_conf.ssl != NULL) {
	if (strcasecmp(ldap_conf.ssl, "start_tls") == 0)
	    ldap_conf.ssl_mode = SUDO_LDAP_STARTTLS;
	else if (sudo_strtobool(ldap_conf.ssl) == true)
	    ldap_conf.ssl_mode = SUDO_LDAP_SSL;
    }

#if defined(HAVE_LDAPSSL_SET_STRENGTH) && !defined(LDAP_OPT_X_TLS_REQUIRE_CERT)
    if (ldap_conf.tls_checkpeer != -1) {
	ldapssl_set_strength(NULL,
	    ldap_conf.tls_checkpeer ? LDAPSSL_AUTH_CERT : LDAPSSL_AUTH_WEAK);
    }
#endif

#ifndef HAVE_LDAP_INITIALIZE
    /* Convert uri list to host list if no ldap_initialize(). */
    if (!STAILQ_EMPTY(&ldap_conf.uri)) {
	struct ldap_config_str *uri;

	if (sudo_ldap_parse_uri(&ldap_conf.uri) != LDAP_SUCCESS)
	    debug_return_bool(false);
	while ((uri = STAILQ_FIRST(&ldap_conf.uri)) != NULL) {
	    STAILQ_REMOVE_HEAD(&ldap_conf.uri, entries);
	    free(uri);
	}
	ldap_conf.port = LDAP_PORT;
    }
#endif

    if (STAILQ_EMPTY(&ldap_conf.uri)) {
	/* Use port 389 for plaintext LDAP and port 636 for SSL LDAP */
	if (ldap_conf.port < 0)
	    ldap_conf.port =
		ldap_conf.ssl_mode == SUDO_LDAP_SSL ? LDAPS_PORT : LDAP_PORT;

#ifdef HAVE_LDAP_CREATE
	/*
	 * Cannot specify port directly to ldap_create(), each host must
	 * include :port to override the default.
	 */
	if (ldap_conf.port != LDAP_PORT) {
	    if (!sudo_ldap_conf_add_ports())
		debug_return_bool(false);
	}
#endif
    }

    /* If search filter is not parenthesized, make it so. */
    if (ldap_conf.search_filter && ldap_conf.search_filter[0] != '(') {
	size_t len = strlen(ldap_conf.search_filter);
	cp = ldap_conf.search_filter;
	ldap_conf.search_filter = malloc(len + 3);
	if (ldap_conf.search_filter == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_bool(false);
	}
	ldap_conf.search_filter[0] = '(';
	memcpy(ldap_conf.search_filter + 1, cp, len);
	ldap_conf.search_filter[len + 1] = ')';
	ldap_conf.search_filter[len + 2] = '\0';
	free(cp);
    }


    /* If rootbinddn set, read in /etc/ldap.secret if it exists. */
    if (ldap_conf.rootbinddn) {
	sudo_ldap_read_secret(ctx->settings.ldap_secret);
    } else if (ldap_conf.bindpw) {
	cp = sudo_ldap_decode_secret(ldap_conf.bindpw);
	if (cp != NULL) {
	    free(ldap_conf.bindpw);
	    ldap_conf.bindpw = cp;
	}
    }

    if (ldap_conf.tls_keypw) {
	cp = sudo_ldap_decode_secret(ldap_conf.tls_keypw);
	if (cp != NULL) {
	    free(ldap_conf.tls_keypw);
	    ldap_conf.tls_keypw = cp;
	}
    }

#ifdef HAVE_LDAP_SASL_INTERACTIVE_BIND_S
    /*
     * Make sure we can open the file specified by krb5_ccname.
     */
    if (ldap_conf.krb5_ccname != NULL) {
	if (!sudo_check_krb5_ccname(ldap_conf.krb5_ccname))
	    ldap_conf.krb5_ccname = NULL;
    }
#endif

    debug_return_bool(true);
}

/*
 * Set LDAP options from the specified options table
 * Returns LDAP_SUCCESS on success, else non-zero.
 */
static int
sudo_ldap_set_options_table(LDAP *ld, struct ldap_config_table *table)
{
    struct ldap_config_table *cur;
    int ival, rc, errors = 0;
    char *sval;
    debug_decl(sudo_ldap_set_options_table, SUDOERS_DEBUG_LDAP);

    for (cur = table; cur->conf_str != NULL; cur++) {
	if (cur->opt_val == -1)
	    continue;

	switch (cur->type) {
	case CONF_DEREF_VAL:
	case CONF_REQCERT_VAL:
	case CONF_BOOL:
	case CONF_INT:
	    ival = *(int *)(cur->valp);
	    if (ival >= 0) {
		DPRINTF1("ldap_set_option: %s -> %d", cur->conf_str, ival);
		rc = ldap_set_option(ld, cur->opt_val, &ival);
		if (rc != LDAP_OPT_SUCCESS) {
		    sudo_warnx("ldap_set_option: %s -> %d: %s",
			cur->conf_str, ival, ldap_err2string(rc));
		    errors++;
		}
	    }
	    break;
	case CONF_STR:
	    sval = *(char **)(cur->valp);
	    if (sval != NULL) {
		DPRINTF1("ldap_set_option: %s -> %s", cur->conf_str, sval);
		rc = ldap_set_option(ld, cur->opt_val, sval);
		if (rc != LDAP_OPT_SUCCESS) {
		    sudo_warnx("ldap_set_option: %s -> %s: %s",
			cur->conf_str, sval, ldap_err2string(rc));
		    errors++;
		}
	    }
	    break;
	case CONF_LIST_STR:
	    /* Lists are iterated over and don't set LDAP options directly. */
	    break;
	default:
	    sudo_warnx("internal error: unhandled CONF_ value %d for option %s",
		cur->type, cur->conf_str);
	    sudo_warnx("update %s to add missing support for CONF_ value %d",
		__func__, cur->type);
	}
    }
    debug_return_int(errors ? -1 : LDAP_SUCCESS);
}

/*
 * Set LDAP options based on the global config table.
 * Returns LDAP_SUCCESS on success, else non-zero.
 */
int
sudo_ldap_set_options_global(void)
{
    int ret;
    debug_decl(sudo_ldap_set_options_global, SUDOERS_DEBUG_LDAP);

    /* Set ber options */
#ifdef LBER_OPT_DEBUG_LEVEL
    if (ldap_conf.ldap_debug)
	ber_set_option(NULL, LBER_OPT_DEBUG_LEVEL, &ldap_conf.ldap_debug);
#endif

    /* Parse global LDAP options table. */
    ret = sudo_ldap_set_options_table(NULL, ldap_conf_global);
    debug_return_int(ret);
}

/*
 * Set LDAP options based on the per-connection config table.
 * Returns LDAP_SUCCESS on success, else non-zero.
 */
int
sudo_ldap_set_options_conn(LDAP *ld)
{
    int rc;
    debug_decl(sudo_ldap_set_options_conn, SUDOERS_DEBUG_LDAP);

    /* Parse per-connection LDAP options table. */
    rc = sudo_ldap_set_options_table(ld, ldap_conf_conn);
    if (rc == -1)
	debug_return_int(-1);

#ifdef LDAP_OPT_TIMEOUT
    /* Convert timeout to a timeval */
    if (ldap_conf.timeout > 0) {
	struct timeval tv;
	tv.tv_sec = ldap_conf.timeout;
	tv.tv_usec = 0;
	DPRINTF1("ldap_set_option(LDAP_OPT_TIMEOUT, %d)", ldap_conf.timeout);
	rc = ldap_set_option(ld, LDAP_OPT_TIMEOUT, &tv);
	if (rc != LDAP_OPT_SUCCESS) {
	    sudo_warnx("ldap_set_option(TIMEOUT, %d): %s",
		ldap_conf.timeout, ldap_err2string(rc));
	}
    }
#endif
#ifdef LDAP_OPT_NETWORK_TIMEOUT
    /* Convert bind_timelimit to a timeval */
    if (ldap_conf.bind_timelimit > 0) {
	struct timeval tv;
	tv.tv_sec = ldap_conf.bind_timelimit / 1000;
	tv.tv_usec = 0;
	DPRINTF1("ldap_set_option(LDAP_OPT_NETWORK_TIMEOUT, %d)",
	    ldap_conf.bind_timelimit / 1000);
	rc = ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &tv);
# if !defined(LDAP_OPT_CONNECT_TIMEOUT) || LDAP_VENDOR_VERSION != 510
	/* Tivoli Directory Server 6.3 libs always return a (bogus) error. */
	if (rc != LDAP_OPT_SUCCESS) {
	    sudo_warnx("ldap_set_option(NETWORK_TIMEOUT, %d): %s",
		ldap_conf.bind_timelimit / 1000, ldap_err2string(rc));
	}
# endif
    }
#endif

#if defined(LDAP_OPT_X_TLS) && !defined(HAVE_LDAPSSL_INIT)
    if (ldap_conf.ssl_mode == SUDO_LDAP_SSL) {
	int val = LDAP_OPT_X_TLS_HARD;
	DPRINTF1("ldap_set_option(LDAP_OPT_X_TLS, LDAP_OPT_X_TLS_HARD)");
	rc = ldap_set_option(ld, LDAP_OPT_X_TLS, &val);
	if (rc != LDAP_SUCCESS) {
	    sudo_warnx("ldap_set_option(LDAP_OPT_X_TLS, LDAP_OPT_X_TLS_HARD): %s",
		ldap_err2string(rc));
	    debug_return_int(-1);
	}
    }
#endif
    debug_return_int(LDAP_SUCCESS);
}
