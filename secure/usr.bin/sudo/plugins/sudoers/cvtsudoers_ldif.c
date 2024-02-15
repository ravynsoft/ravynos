/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <stdarg.h>

#include <sudoers.h>
#include <sudo_ldap.h>
#include <redblack.h>
#include <cvtsudoers.h>
#include <sudo_lbuf.h>
#include <gram.h>

struct seen_user {
    const char *name;
    unsigned long count;
};

static struct rbtree *seen_users;

static int
seen_user_compare(const void *aa, const void *bb)
{
    const struct seen_user *a = aa;
    const struct seen_user *b = bb;

    return strcasecmp(a->name, b->name);
}

static void
seen_user_free(void *v)
{
    struct seen_user *su = v;

    free((void *)su->name);
    free(su);
}

static bool
safe_string(const char *str)
{
    const unsigned char *ustr = (const unsigned char *)str;
    unsigned char ch = *ustr++;
    debug_decl(safe_string, SUDOERS_DEBUG_UTIL);

    /* Initial char must be <= 127 and not LF, CR, SPACE, ':', '<' */
    switch (ch) {
    case '\0':
	debug_return_bool(true);
    case '\n':
    case '\r':
    case ' ':
    case ':':
    case '<':
	debug_return_bool(false);
    default:
	if (ch > 127)
	    debug_return_bool(false);
    }

    /* Any value <= 127 decimal except NUL, LF, and CR is safe */
    while ((ch = *ustr++) != '\0') {
	if (ch > 127 || ch == '\n' || ch == '\r')
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

static bool
print_attribute_ldif(FILE *fp, const char *name, const char *value)
{
    const unsigned char *uvalue = (unsigned char *)value;
    char *encoded = NULL;
    size_t esize;
    debug_decl(print_attribute_ldif, SUDOERS_DEBUG_UTIL);

    if (!safe_string(value)) {
	const size_t vlen = strlen(value);
	esize = ((vlen + 2) / 3 * 4) + 1;
	if ((encoded = malloc(esize)) == NULL)
	    debug_return_bool(false);
	if (base64_encode(uvalue, vlen, encoded, esize) == (size_t)-1) {
	    free(encoded);
	    debug_return_bool(false);
	}
	fprintf(fp, "%s:: %s\n", name, encoded);
	free(encoded);
    } else if (*value != '\0') {
	fprintf(fp, "%s: %s\n", name, value);
    } else {
	fprintf(fp, "%s:\n", name);
    }

    debug_return_bool(true);
}

/*
 * Print sudoOptions from a defaults_list.
 */
static bool
print_options_ldif(FILE *fp, const struct defaults_list *options)
{
    struct defaults *opt;
    char *attr_val;
    int len;
    debug_decl(print_options_ldif, SUDOERS_DEBUG_UTIL);

    TAILQ_FOREACH(opt, options, entries) {
	if (opt->type != DEFAULTS)
	    continue;		/* don't support bound defaults */

	if (opt->val != NULL) {
	    /* There is no need to double quote values here. */
	    len = asprintf(&attr_val, "%s%s%s", opt->var,
		opt->op == '+' ? "+=" : opt->op == '-' ? "-=" : "=", opt->val);
	} else {
	    /* Boolean flag. */
	    len = asprintf(&attr_val, "%s%s",
		opt->op == false ? "!" : "", opt->var);
	}
	if (len == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, "sudoOption", attr_val);
	free(attr_val);
    }

    debug_return_bool(!ferror(fp));
}

/*
 * Print global Defaults in a single sudoRole object.
 */
static bool
print_global_defaults_ldif(FILE *fp,
    const struct sudoers_parse_tree *parse_tree, const char *base)
{
    unsigned int count = 0;
    struct sudo_lbuf lbuf;
    struct defaults *opt;
    char *dn;
    debug_decl(print_global_defaults_ldif, SUDOERS_DEBUG_UTIL);

    sudo_lbuf_init(&lbuf, NULL, 0, NULL, 80);

    TAILQ_FOREACH(opt, &parse_tree->defaults, entries) {
	/* Skip bound Defaults (unsupported). */
	if (opt->type == DEFAULTS) {
	    count++;
	} else {
	    lbuf.len = 0;
	    sudo_lbuf_append(&lbuf, "# ");
	    sudoers_format_default_line(&lbuf, parse_tree, opt, false, true);
	    fprintf(fp, "# Unable to translate %s:%d:%d:\n%s\n",
		opt->file, opt->line, opt->column, lbuf.buf);
	}
    }
    sudo_lbuf_destroy(&lbuf);

    if (count == 0)
	debug_return_bool(true);

    if (asprintf(&dn, "cn=defaults,%s", base) == -1) {
	sudo_fatalx(U_("%s: %s"), __func__,
	    U_("unable to allocate memory"));
    }
    print_attribute_ldif(fp, "dn", dn);
    free(dn);
    print_attribute_ldif(fp, "objectClass", "top");
    print_attribute_ldif(fp, "objectClass", "sudoRole");
    print_attribute_ldif(fp, "cn", "defaults");
    print_attribute_ldif(fp, "description", "Default sudoOption's go here");

    print_options_ldif(fp, &parse_tree->defaults);
    putc('\n', fp);

    debug_return_bool(!ferror(fp));
}

/*
 * Format a sudo_command as a string.
 * Returns the formatted, dynamically allocated string or dies on error.
 */
static char *
format_cmnd(struct sudo_command *c, bool negated)
{
    struct command_digest *digest;
    char *buf, *cp, *cmnd;
    size_t bufsiz;
    int len;
    debug_decl(format_cmnd, SUDOERS_DEBUG_UTIL);

    cmnd = c->cmnd ? c->cmnd : (char *)"ALL";
    bufsiz = negated + strlen(cmnd) + 1;
    if (c->args != NULL)
	bufsiz += 1 + strlen(c->args);
    TAILQ_FOREACH(digest, &c->digests, entries) {
	bufsiz += strlen(digest_type_to_name(digest->digest_type)) + 1 +
	    strlen(digest->digest_str) + 1;
	if (TAILQ_NEXT(digest, entries) != NULL)
	    bufsiz += 2;
    }

    if ((buf = malloc(bufsiz)) == NULL) {
	sudo_fatalx(U_("%s: %s"), __func__,
	    U_("unable to allocate memory"));
    }

    cp = buf;
    TAILQ_FOREACH(digest, &c->digests, entries) {
	len = snprintf(cp, bufsiz - (size_t)(cp - buf), "%s:%s%s ", 
	    digest_type_to_name(digest->digest_type), digest->digest_str,
	    TAILQ_NEXT(digest, entries) ? "," : "");
	if (len < 0 || len >= (int)bufsiz - (cp - buf))
	    sudo_fatalx(U_("internal error, %s overflow"), __func__);
	cp += len;
    }

    len = snprintf(cp, bufsiz - (size_t)(cp - buf), "%s%s%s%s",
	negated ? "!" : "", cmnd, c->args ? " " : "", c->args ? c->args : "");
    if (len < 0 || len >= (int)bufsiz - (cp - buf))
	sudo_fatalx(U_("internal error, %s overflow"), __func__);

    debug_return_str(buf);
}

/*
 * Print struct member in LDIF format as the specified attribute.
 * See print_member_int() in parse.c.
 */
static void
print_member_ldif(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    char *name, int type, bool negated, short alias_type,
    const char *attr_name)
{
    struct alias *a;
    struct member *m;
    char *attr_val;
    int len;
    debug_decl(print_member_ldif, SUDOERS_DEBUG_UTIL);

    switch (type) {
    case MYSELF:
	/* Only valid for sudoRunasUser */
	print_attribute_ldif(fp, attr_name, "");
	break;
    case ALL:
	if (name == NULL) {
	    print_attribute_ldif(fp, attr_name, negated ? "!ALL" : "ALL");
	    break;
	}
	FALLTHROUGH;
    case COMMAND:
	attr_val = format_cmnd((struct sudo_command *)name, negated);
	print_attribute_ldif(fp, attr_name, attr_val);
	free(attr_val);
	break;
    case ALIAS:
	if ((a = alias_get(parse_tree, name, alias_type)) != NULL) {
	    TAILQ_FOREACH(m, &a->members, entries) {
		print_member_ldif(fp, parse_tree, m->name, m->type,
		    negated ? !m->negated : m->negated, alias_type, attr_name);
	    }
	    alias_put(a);
	    break;
	}
	FALLTHROUGH;
    default:
	len = asprintf(&attr_val, "%s%s", negated ? "!" : "", name);
	if (len == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, attr_name, attr_val);
	free(attr_val);
	break;
    }

    debug_return;
}

/*
 * Print a Cmnd_Spec in LDIF format.
 * A pointer to the next Cmnd_Spec is passed in to make it possible to
 * merge adjacent entries that are identical in all but the command.
 */
static void
print_cmndspec_ldif(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    struct cmndspec *cs, struct cmndspec **nextp, struct defaults_list *options)
{
    char timebuf[sizeof("20120727121554Z")];
    struct cmndspec *next = *nextp;
    struct member *m;
    struct tm gmt;
    char *attr_val;
    bool last_one;
    size_t len;
    debug_decl(print_cmndspec_ldif, SUDOERS_DEBUG_UTIL);

    /* Print runasuserlist as sudoRunAsUser attributes */
    if (cs->runasuserlist != NULL) {
	TAILQ_FOREACH(m, cs->runasuserlist, entries) {
	    print_member_ldif(fp, parse_tree, m->name, m->type, m->negated,
		RUNASALIAS, "sudoRunAsUser");
	}
    }

    /* Print runasgrouplist as sudoRunAsGroup attributes */
    if (cs->runasgrouplist != NULL) {
	TAILQ_FOREACH(m, cs->runasgrouplist, entries) {
	    print_member_ldif(fp, parse_tree, m->name, m->type, m->negated,
		RUNASALIAS, "sudoRunAsGroup");
	}
    }

    /* Print sudoNotBefore and sudoNotAfter attributes */
    if (cs->notbefore != UNSPEC) {
	if (gmtime_r(&cs->notbefore, &gmt) == NULL) {
	    sudo_warn("%s", U_("unable to get GMT time"));
	} else {
	    timebuf[sizeof(timebuf) - 1] = '\0';
	    len = strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%SZ", &gmt);
	    if (len == 0 || timebuf[sizeof(timebuf) - 1] != '\0') {
		sudo_warnx("%s", U_("unable to format timestamp"));
	    } else {
		print_attribute_ldif(fp, "sudoNotBefore", timebuf);
	    }
	}
    }
    if (cs->notafter != UNSPEC) {
	if (gmtime_r(&cs->notafter, &gmt) == NULL) {
	    sudo_warn("%s", U_("unable to get GMT time"));
	} else {
	    timebuf[sizeof(timebuf) - 1] = '\0';
	    len = strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%SZ", &gmt);
	    if (len == 0 || timebuf[sizeof(timebuf) - 1] != '\0') {
		sudo_warnx("%s", U_("unable to format timestamp"));
	    } else {
		print_attribute_ldif(fp, "sudoNotAfter", timebuf);
	    }
	}
    }

    /* Print timeout as a sudoOption. */
    if (cs->timeout > 0) {
	if (asprintf(&attr_val, "command_timeout=%d", cs->timeout) == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, "sudoOption", attr_val);
	free(attr_val);
    }

    /* Print tags as sudoOption attributes */
    if (TAGS_SET(cs->tags)) {
	struct cmndtag tag = cs->tags;

	if (tag.nopasswd != UNSPEC) {
	    print_attribute_ldif(fp, "sudoOption",
		tag.nopasswd ? "!authenticate" : "authenticate");
	}
	if (tag.noexec != UNSPEC) {
	    print_attribute_ldif(fp, "sudoOption",
		tag.noexec ? "noexec" : "!noexec");
	}
	if (tag.intercept != UNSPEC) {
	    print_attribute_ldif(fp, "sudoOption",
		tag.intercept ? "intercept" : "!intercept");
	}
	if (tag.send_mail != UNSPEC) {
	    if (tag.send_mail) {
		print_attribute_ldif(fp, "sudoOption", "mail_all_cmnds");
	    } else {
		print_attribute_ldif(fp, "sudoOption", "!mail_all_cmnds");
		print_attribute_ldif(fp, "sudoOption", "!mail_always");
		print_attribute_ldif(fp, "sudoOption", "!mail_no_perms");
	    }
	}
	if (tag.setenv != UNSPEC && tag.setenv != IMPLIED) {
	    print_attribute_ldif(fp, "sudoOption",
		tag.setenv ? "setenv" : "!setenv");
	}
	if (tag.follow != UNSPEC) {
	    print_attribute_ldif(fp, "sudoOption",
		tag.follow ? "sudoedit_follow" : "!sudoedit_follow");
	}
	if (tag.log_input != UNSPEC) {
	    print_attribute_ldif(fp, "sudoOption",
		tag.log_input ? "log_input" : "!log_input");
	}
	if (tag.log_output != UNSPEC) {
	    print_attribute_ldif(fp, "sudoOption",
		tag.log_output ? "log_output" : "!log_output");
	}
    }
    print_options_ldif(fp, options);

    /* Print runchroot and runcwd. */
    if (cs->runchroot != NULL) {
	if (asprintf(&attr_val, "runchroot=%s", cs->runchroot) == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, "sudoOption", attr_val);
	free(attr_val);
    }
    if (cs->runcwd != NULL) {
	if (asprintf(&attr_val, "runcwd=%s", cs->runcwd) == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, "sudoOption", attr_val);
	free(attr_val);
    }

#ifdef HAVE_SELINUX
    /* Print SELinux role/type */
    if (cs->role != NULL && cs->type != NULL) {
	if (asprintf(&attr_val, "role=%s", cs->role) == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, "sudoOption", attr_val);
	free(attr_val);

	if (asprintf(&attr_val, "type=%s", cs->type) == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, "sudoOption", attr_val);
	free(attr_val);
    }
#endif /* HAVE_SELINUX */

#ifdef HAVE_APPARMOR
    /* Print AppArmor profile */
    if (cs->apparmor_profile != NULL) {
	if (asprintf(&attr_val, "apparmor_profile=%s", cs->apparmor_profile) == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_attribute_ldif(fp, "sudoOption", attr_val);
	free(attr_val);
    }
#endif /* HAVE_APPARMOR */

#ifdef HAVE_PRIV_SET
    /* Print Solaris privs/limitprivs */
    if (cs->privs != NULL || cs->limitprivs != NULL) {
	if (cs->privs != NULL) {
	    if (asprintf(&attr_val, "privs=%s", cs->privs) == -1) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	    print_attribute_ldif(fp, "sudoOption", attr_val);
	    free(attr_val);
	}
	if (cs->limitprivs != NULL) {
	    if (asprintf(&attr_val, "limitprivs=%s", cs->limitprivs) == -1) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	    print_attribute_ldif(fp, "sudoOption", attr_val);
	    free(attr_val);
	}
    }
#endif /* HAVE_PRIV_SET */

    /*
     * Merge adjacent commands with matching tags, runas, SELinux
     * role/type and Solaris priv settings.
     */
    for (;;) {
	/* Does the next entry differ only in the command itself? */
	/* XXX - move into a function that returns bool */
	/* XXX - TAG_SET does not account for implied SETENV */
	last_one = next == NULL ||
	    RUNAS_CHANGED(cs, next) || TAGS_CHANGED(cs->tags, next->tags)
#ifdef HAVE_PRIV_SET
	    || cs->privs != next->privs || cs->limitprivs != next->limitprivs
#endif /* HAVE_PRIV_SET */
#ifdef HAVE_SELINUX
	    || cs->role != next->role || cs->type != next->type
#endif /* HAVE_SELINUX */
	    || cs->runchroot != next->runchroot || cs->runcwd != next->runcwd;

	print_member_ldif(fp, parse_tree, cs->cmnd->name, cs->cmnd->type,
	    cs->cmnd->negated, CMNDALIAS, "sudoCommand");
	if (last_one)
	    break;
	cs = next;
	next = TAILQ_NEXT(cs, entries);
    }

    *nextp = next;

    debug_return;
}

/*
 * Convert user name to cn, avoiding duplicates and quoting as needed.
 * See http://www.faqs.org/rfcs/rfc2253.html
 */
static char *
user_to_cn(const char *user)
{
    struct seen_user key, *su = NULL;
    struct rbnode *node;
    const char *src;
    char *cn, *dst;
    size_t size;
    debug_decl(user_to_cn, SUDOERS_DEBUG_UTIL);

    /* Allocate as much as we could possibly need. */
    size = (2 * strlen(user)) + 64 + 1;
    if ((cn = malloc(size)) == NULL)
	goto bad;

    /*
     * Increment the number of times we have seen this user.
     */
    key.name = user;
    node = rbfind(seen_users, &key);
    if (node != NULL) {
	su = node->data;
    } else {
	if ((su = malloc(sizeof(*su))) == NULL)
	    goto bad;
	su->count = 0;
	if ((su->name = strdup(user)) == NULL)
	    goto bad;
	if (rbinsert(seen_users, su, NULL) != 0)
	    goto bad;
    }

    /* Build cn, quoting special chars as needed (we allocated 2 x len). */
    for (src = user, dst = cn; *src != '\0'; src++) {
	switch (*src) {
	case ',':
	case '+':
	case '"':
	case '\\':
	case '<':
	case '>':
	case '#':
	case ';':
	    *dst++ = '\\';	/* always escape */
	    break;
	case ' ':
	    if (src == user || src[1] == '\0')
		*dst++ = '\\';	/* only escape at beginning or end of string */
	    break;
	default:
	    break;
	}
	*dst++ = *src;
    }
    *dst = '\0';

    /* Append count if there are duplicate users (cn must be unique). */
    if (su->count != 0) {
	size -= (size_t)(dst - cn);
	if ((size_t)snprintf(dst, size, "_%lu", su->count) >= size) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    goto bad;
	}
    }
    su->count++;

    debug_return_str(cn);
bad:
    if (su != NULL && su->count == 0)
	seen_user_free(su);
    free(cn);
    debug_return_str(NULL);
}

/*
 * Print a single User_Spec.
 */
static bool
print_userspec_ldif(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    struct userspec *us, struct cvtsudoers_config *conf)
{
    struct privilege *priv;
    struct member *m;
    struct cmndspec *cs, *next;
    debug_decl(print_userspec_ldif, SUDOERS_DEBUG_UTIL);

    /*
     * Each userspec struct may contain multiple privileges for
     * the user.  We export each privilege as a separate sudoRole
     * object for simplicity's sake.
     */
    TAILQ_FOREACH(priv, &us->privileges, entries) {
	TAILQ_FOREACH_SAFE(cs, &priv->cmndlist, entries, next) {
	    const char *base = conf->sudoers_base;
	    char *cn, *dn;

	    /*
	     * Increment the number of times we have seen this user.
	     * If more than one user is listed, just use the first one.
	     */
	    m = TAILQ_FIRST(&us->users);
	    cn = user_to_cn(m->type == ALL ? "ALL" : m->name);
	    if (cn == NULL || asprintf(&dn, "cn=%s,%s", cn, base) == -1) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }

	    print_attribute_ldif(fp, "dn", dn);
	    print_attribute_ldif(fp, "objectClass", "top");
	    print_attribute_ldif(fp, "objectClass", "sudoRole");
	    print_attribute_ldif(fp, "cn", cn);
	    free(cn);
	    free(dn);

	    TAILQ_FOREACH(m, &us->users, entries) {
		print_member_ldif(fp, parse_tree, m->name, m->type, m->negated,
		    USERALIAS, "sudoUser");
	    }

	    TAILQ_FOREACH(m, &priv->hostlist, entries) {
		print_member_ldif(fp, parse_tree, m->name, m->type, m->negated,
		    HOSTALIAS, "sudoHost");
	    }

	    print_cmndspec_ldif(fp, parse_tree, cs, &next, &priv->defaults);

	    if (conf->sudo_order != 0) {
		char numbuf[STRLEN_MAX_UNSIGNED(conf->sudo_order) + 1];
		if (conf->order_max != 0 && conf->sudo_order > conf->order_max) {
		    sudo_fatalx(U_("too many sudoers entries, maximum %u"),
			conf->order_padding);
		}
		(void)snprintf(numbuf, sizeof(numbuf), "%u", conf->sudo_order);
		print_attribute_ldif(fp, "sudoOrder", numbuf);
		putc('\n', fp);
		conf->sudo_order += conf->order_increment;
	    }
	}
    }

    debug_return_bool(!ferror(fp));
}

/*
 * Print User_Specs.
 */
static bool
print_userspecs_ldif(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    struct cvtsudoers_config *conf)
{
    struct userspec *us;
    debug_decl(print_userspecs_ldif, SUDOERS_DEBUG_UTIL);
 
    TAILQ_FOREACH(us, &parse_tree->userspecs, entries) {
	if (!print_userspec_ldif(fp, parse_tree, us, conf))
	    debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Export the parsed sudoers file in LDIF format.
 */
bool
convert_sudoers_ldif(const struct sudoers_parse_tree *parse_tree,
    const char *output_file, struct cvtsudoers_config *conf)
{
    bool ret = true;
    FILE *output_fp = stdout;
    debug_decl(convert_sudoers_ldif, SUDOERS_DEBUG_UTIL);

    if (conf->sudoers_base == NULL) {
	sudo_fatalx("%s", U_("the SUDOERS_BASE environment variable is not set and the -b option was not specified."));
    }

    if (output_file != NULL && strcmp(output_file, "-") != 0) {
	if ((output_fp = fopen(output_file, "w")) == NULL)
	    sudo_fatal(U_("unable to open %s"), output_file);
    }

    /* Create a dictionary of already-seen users. */
    seen_users = rbcreate(seen_user_compare);

    /* Dump global Defaults in LDIF format. */
    if (!ISSET(conf->suppress, SUPPRESS_DEFAULTS))
	print_global_defaults_ldif(output_fp, parse_tree, conf->sudoers_base);

    /* Dump User_Specs in LDIF format, expanding Aliases. */
    if (!ISSET(conf->suppress, SUPPRESS_PRIVS))
	print_userspecs_ldif(output_fp, parse_tree, conf);

    /* Clean up. */
    rbdestroy(seen_users, seen_user_free);

    (void)fflush(output_fp);
    if (ferror(output_fp))
	ret = false;
    if (output_fp != stdout)
	fclose(output_fp);

    debug_return_bool(ret);
}
