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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <stdarg.h>

#include <sudoers.h>
#include <cvtsudoers.h>
#include <gram.h>

static void print_member_list_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree, struct member_list *members, bool negated, short alias_type, bool expand_aliases);

/*
 * Print sudoOptions from a defaults_list.
 */
static bool
print_options_csv(FILE *fp, struct defaults_list *options, bool need_comma)
{
    struct defaults *opt;
    debug_decl(print_options_csv, SUDOERS_DEBUG_UTIL);

    TAILQ_FOREACH(opt, options, entries) {
	if (opt->val != NULL) {
	    /* There is no need to double quote values here. */
	    fprintf(fp, "%s%s%s%s", need_comma ? "," : "", opt->var,
		opt->op == '+' ? "+=" : opt->op == '-' ? "-=" : "=", opt->val);
	} else {
	    /* Boolean flag. */
	    fprintf(fp, "%s%s%s%s", need_comma ? "," : "", opt->var,
		opt->op == false ? "!" : "", opt->var);
	}
	need_comma = true;
    }

    debug_return_bool(!ferror(fp));
}

/*
 * Map a Defaults type to string.
 */
static const char *
defaults_type_to_string(int defaults_type)
{
    switch (defaults_type) {
    case DEFAULTS:
        return "defaults";
    case DEFAULTS_CMND:
        return "defaults_command";
    case DEFAULTS_HOST:
        return "defaults_host";
    case DEFAULTS_RUNAS:
        return "defaults_runas";
    case DEFAULTS_USER:
        return "defaults_user";
    default:
        sudo_fatalx_nodebug("unexpected defaults type %d", defaults_type);
    }
}

/*
 * Map a Defaults type to an alias type.
 */
static short
defaults_to_alias_type(int defaults_type)
{
    switch (defaults_type) {
    case DEFAULTS_CMND:
        return CMNDALIAS;
    case DEFAULTS_HOST:
        return HOSTALIAS;
    case DEFAULTS_RUNAS:
        return RUNASALIAS;
    case DEFAULTS_USER:
        return USERALIAS;
    default:
        sudo_fatalx_nodebug("unexpected defaults type %d", defaults_type);
    }
}

/*
 * Print a string, performing quoting as needed.
 * If a field includes a comma it must be double-quoted.
 * Double quotes are replaced by a pair of double-quotes.
 * XXX - rewrite this
 */
static bool
print_csv_string(FILE * restrict fp, const char * restrict str, bool quoted)
{
    const char *src = str;
    char *dst, *newstr;
    size_t len, newsize;
    bool quote_it = false;
    bool ret = true;
    debug_decl(print_csv_string, SUDOERS_DEBUG_UTIL);

    len = strcspn(str, quoted ? "\"" : "\",");
    if (str[len] == '\0') {
	/* nothing to escape */
	debug_return_bool(fputs(str, fp) != EOF);
    }

    if (!quoted && strchr(str + len, ',') != NULL)
	quote_it = true;

    /* String includes characters we need to escape. */
    newsize = len + 2 + (strlen(len + str) * 2) + 1;
    if ((newstr = malloc(newsize)) == NULL)
	debug_return_bool(false);
    dst = newstr;

    if (quote_it)
	*dst++ = '"';
    while (*src != '\0') {
	if (*src == '"')
	    *dst++ = '"';
	*dst++ = *src++;
    }
    if (quote_it)
	*dst++ = '"';
    *dst = '\0';

    if (fputs(newstr, fp) == EOF)
	ret = false;
    free(newstr);

    debug_return_bool(ret);
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
 * Print struct member in CSV format as the specified attribute.
 * See print_member_int() in parse.c.
 */
static void
print_member_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    char *name, int type, bool negated, bool quoted, short alias_type,
    bool expand_aliases)
{
    struct alias *a;
    char *str;
    int len;
    debug_decl(print_member_csv, SUDOERS_DEBUG_UTIL);

    switch (type) {
    case MYSELF:
	/* Only valid for sudoRunasUser */
	break;
    case ALL:
	if (name == NULL) {
	    fputs(negated ? "!ALL" : "ALL", fp);
	    break;
	}
	FALLTHROUGH;
    case COMMAND:
	str = format_cmnd((struct sudo_command *)name, negated);
	print_csv_string(fp, str, quoted);
	free(str);
	break;
    case ALIAS:
	if (expand_aliases) {
	    if ((a = alias_get(parse_tree, name, alias_type)) != NULL) {
		print_member_list_csv(fp, parse_tree, &a->members, negated,
		    alias_type, expand_aliases);
		alias_put(a);
		break;
	    }
	}
	FALLTHROUGH;
    default:
	len = asprintf(&str, "%s%s", negated ? "!" : "", name);
	if (len == -1) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	print_csv_string(fp, str, quoted);
	free(str);
	break;
    }

    debug_return;
}

/*
 * Print list of struct member in CSV format as the specified attribute.
 * See print_member_int() in parse.c.
 */
static void
print_member_list_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    struct member_list *members, bool negated, short alias_type,
    bool expand_aliases)
{
    struct member *m, *next;
    debug_decl(print_member_list_csv, SUDOERS_DEBUG_UTIL);

    if (TAILQ_EMPTY(members))
        debug_return;

    if (TAILQ_FIRST(members) != TAILQ_LAST(members, member_list))
	putc('"', fp);
    TAILQ_FOREACH_SAFE(m, members, entries, next) {
        print_member_csv(fp, parse_tree, m->name, m->type,
	    negated ? !m->negated : m->negated, true, alias_type,
	    expand_aliases);
	if (next != NULL)
	    putc(',', fp);
    }
    if (TAILQ_FIRST(members) != TAILQ_LAST(members, member_list))
	putc('"', fp);

    debug_return;
}

/*
 * Print the binding for a Defaults entry of the specified type.
 */
static void
print_defaults_binding_csv(FILE *fp,
    const struct sudoers_parse_tree *parse_tree,
     struct defaults_binding *binding, int type, bool expand_aliases)
{
    short alias_type;
    debug_decl(print_defaults_binding_csv, SUDOERS_DEBUG_UTIL);

    if (type != DEFAULTS) {
	/* Print each member object in binding. */
	alias_type = defaults_to_alias_type(type);
	print_member_list_csv(fp, parse_tree, &binding->members, false,
	    alias_type, expand_aliases);
    }

    debug_return;
}

/*
 * Print all Defaults in CSV format:
 *
 * defaults,binding,name,operator,value
 *
 * where "operator" is one of +=, -=, or =
 * and boolean flags use true/false for the value.
 */
static bool
print_defaults_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    bool expand_aliases)
{
    struct defaults *def;
    debug_decl(print_defaults_csv, SUDOERS_DEBUG_UTIL);

    if (TAILQ_EMPTY(&parse_tree->defaults))
	debug_return_bool(true);

    /* Heading line. */
    fputs("defaults_type,binding,name,operator,value\n", fp);

    TAILQ_FOREACH(def, &parse_tree->defaults, entries) {
	const char *operator;

	/* Print operator */
	switch (def->op) {
	case '+':
	    operator = "+=";
	    break;
	case '-':
	    operator = "-=";
	    break;
	case true:
	case false:
	    operator = "=";
	    break;
	default:
	    sudo_warnx("internal error: unexpected defaults op %d", def->op);
	    continue;
	}

	/*
	 * For CSV we use a separate entry for each Defaults setting,
	 * even if they were on the same line in sudoers.
	 */
	fprintf(fp, "%s,", defaults_type_to_string(def->type));

	/* Print binding (if any), which could be a list. */
	print_defaults_binding_csv(fp, parse_tree, def->binding, def->type,
	    expand_aliases);

	/* Print Defaults name + operator. */
	fprintf(fp, ",%s,%s,", def->var, operator);

	/* Print defaults value. */
	/* XXX - differentiate between lists and single values? */
	if (def->val == NULL) {
	    fputs(def->op == true ? "true" : "false", fp);
	} else {
	    /* Does not handle lists specially. */
	    print_csv_string(fp, def->val, false);
	}
	putc('\n', fp);
    }
    putc('\n', fp);
    fflush(fp);

    debug_return_bool(!ferror(fp));
}

/*
 * Callback for alias_apply() to print an alias entry.
 */
static int
print_alias_csv(struct sudoers_parse_tree *parse_tree, struct alias *a, void *v)
{
    FILE *fp = v;
    const char *title;
    debug_decl(print_alias_csv, SUDOERS_DEBUG_UTIL);

    title = alias_type_to_string(a->type);
    if (title == NULL) {
        sudo_warnx("unexpected alias type %d", a->type);
	debug_return_int(0);
    }

    fprintf(fp, "%s,%s,", title, a->name);
    print_member_list_csv(fp, parse_tree, &a->members, false, a->type, false);
    putc('\n', fp);
    debug_return_int(0);
}

/*
 * Print all aliases in CSV format:
 */
static bool
print_aliases_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree)
{
    debug_decl(print_aliases_csv, SUDOERS_DEBUG_UTIL);

    if (TAILQ_EMPTY(&parse_tree->defaults))
	debug_return_bool(true);

    /* Heading line. */
    fputs("alias_type,alias_name,members\n", fp);

    /* print_alias_csv() does not modify parse_tree. */
    alias_apply((struct sudoers_parse_tree *)parse_tree, print_alias_csv, fp);
    putc('\n', fp);

    debug_return_bool(true);
}

/*
 * Print a Cmnd_Spec in CSV format.
 */
static void
print_cmndspec_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    struct cmndspec *cs, struct cmndspec **nextp,
    struct defaults_list *options, bool expand_aliases)
{
    char timebuf[sizeof("20120727121554Z")];
    struct cmndspec *next = *nextp;
    bool need_comma = false;
    struct member *m;
    struct tm gmt;
    bool last_one, quoted = false;
    size_t len;
    debug_decl(print_cmndspec_csv, SUDOERS_DEBUG_UTIL);

    if (cs->runasuserlist != NULL) {
	print_member_list_csv(fp, parse_tree, cs->runasuserlist, false,
	    RUNASALIAS, expand_aliases);
    }
    putc(',', fp);

    if (cs->runasgrouplist != NULL) {
	print_member_list_csv(fp, parse_tree, cs->runasgrouplist, false,
	    RUNASALIAS, expand_aliases);
    }
    putc(',', fp);

    /* We don't know how many options there will be so always quote it. */
    putc('"', fp);
    if (cs->notbefore != UNSPEC) {
	if (gmtime_r(&cs->notbefore, &gmt) == NULL) {
	    sudo_warn("%s", U_("unable to get GMT time"));
	} else {
	    timebuf[sizeof(timebuf) - 1] = '\0';
	    len = strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%SZ", &gmt);
	    if (len == 0 || timebuf[sizeof(timebuf) - 1] != '\0') {
		sudo_warnx("%s", U_("unable to format timestamp"));
	    } else {
		fprintf(fp, "%snotbefore=%s", need_comma ? "," : "", timebuf); // -V547
		need_comma = true;
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
		fprintf(fp, "%snotafter=%s", need_comma ? "," : "", timebuf);
		need_comma = true;
	    }
	}
    }

    if (cs->timeout > 0) {
	fprintf(fp, "%scommand_timeout=%d", need_comma ? "," : "", cs->timeout);
	need_comma = true;
    }

    /* Print tags as options */
    if (TAGS_SET(cs->tags)) {
	struct cmndtag tag = cs->tags;

	if (tag.nopasswd != UNSPEC) {
	    fprintf(fp, "%s%s", need_comma ? "," : "",
		tag.nopasswd ? "!authenticate" : "authenticate");
	    need_comma = true;
	}
	if (tag.noexec != UNSPEC) {
	    fprintf(fp, "%s%s", need_comma ? "," : "",
		tag.noexec ? "noexec" : "!noexec");
	    need_comma = true;
	}
	if (tag.intercept != UNSPEC) {
	    fprintf(fp, "%s%s", need_comma ? "," : "",
		tag.intercept ? "intercept" : "!intercept");
	    need_comma = true;
	}
	if (tag.send_mail != UNSPEC) {
	    if (tag.send_mail) {
		fprintf(fp, "%smail_all_cmnds", need_comma ? "," : "");
	    } else {
		fprintf(fp, "%s!mail_all_cmnds,!mail_always,!mail_no_perms",
		    need_comma ? "," : "");
	    }
	    need_comma = true;
	}
	if (tag.setenv != UNSPEC && tag.setenv != IMPLIED) {
	    fprintf(fp, "%s%s", need_comma ? "," : "",
		tag.setenv ? "setenv" : "!setenv");
	    need_comma = true;
	}
	if (tag.follow != UNSPEC) {
	    fprintf(fp, "%s%s", need_comma ? "," : "",
		tag.follow ? "sudoedit_follow" : "!sudoedit_follow");
	    need_comma = true;
	}
	if (tag.log_input != UNSPEC) {
	    fprintf(fp, "%s%s", need_comma ? "," : "",
		tag.follow ? "log_input" : "!log_input");
	    need_comma = true;
	}
	if (tag.log_output != UNSPEC) {
	    fprintf(fp, "%s%s", need_comma ? "," : "",
		tag.follow ? "log_output" : "!log_output");
	    need_comma = true;
	}
    }
    print_options_csv(fp, options, need_comma);
    if (!TAILQ_EMPTY(options))
	need_comma = true;

    /* Print runchroot and runcwd. */
    if (cs->runchroot != NULL) {
	fprintf(fp, "%srunchroot=%s", need_comma ? "," : "", cs->runchroot);
	need_comma = true;
    }
    if (cs->runcwd != NULL) {
	fprintf(fp, "%sruncwd=%s", need_comma ? "," : "", cs->runcwd);
	need_comma = true;
    }

#ifdef HAVE_SELINUX
    /* Print SELinux role/type */
    if (cs->role != NULL && cs->type != NULL) {
	fprintf(fp, "%srole=%s,type=%s", need_comma ? "," : "",
	    cs->role, cs->type);
	need_comma = true;
    }
#endif /* HAVE_SELINUX */

#ifdef HAVE_APPARMOR
    if (cs->apparmor_profile != NULL) {
	fprintf(fp, "%sapparmor_profile=%s,", need_comma ? "," : "",
	    cs->apparmor_profile);
	need_comma = true;
    }
#endif /* HAVE_APPARMOR */

#ifdef HAVE_PRIV_SET
    /* Print Solaris privs/limitprivs */
    if (cs->privs != NULL || cs->limitprivs != NULL) {
	if (cs->privs != NULL) {
	    fprintf(fp, "%sprivs=%s", need_comma ? "," : "", cs->privs);
	    need_comma = true;
	}
	if (cs->limitprivs != NULL) {
	    fprintf(fp, "%slimitprivs=%s", need_comma ? "," : "", cs->limitprivs);
	    need_comma = true;
	}
    }
#endif /* HAVE_PRIV_SET */
#ifdef __clang_analyzer__
    (void)&need_comma;
#endif
    putc('"', fp);
    putc(',', fp);

    /*
     * Merge adjacent commands with matching tags, runas, SELinux
     * role/type, AppArmor profiles and Solaris priv settings.
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
#ifdef HAVE_APPARMOR
	    || cs->apparmor_profile != next->apparmor_profile
#endif /* HAVE_APPARMOR  */
	    || cs->runchroot != next->runchroot || cs->runcwd != next->runcwd;

	if (!quoted && !last_one) {
	    quoted = true;
	    putc('"', fp);
	}
	m = cs->cmnd;
	print_member_csv(fp, parse_tree, m->name, m->type, m->negated, quoted,
	    CMNDALIAS, expand_aliases);
	if (last_one)
	    break;
	putc(',', fp);
	cs = next;
	next = TAILQ_NEXT(cs, entries);
    }
    if (quoted)
	putc('"', fp);

    *nextp = next;

    debug_return;
}

/*
 * Print a single User_Spec.
 */
static bool
print_userspec_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    struct userspec *us, bool expand_aliases)
{
    struct privilege *priv;
    struct cmndspec *cs, *next;
    debug_decl(print_userspec_csv, SUDOERS_DEBUG_UTIL);

    /*
     * Each userspec struct may contain multiple privileges for the user.
     */
    TAILQ_FOREACH(priv, &us->privileges, entries) {
	TAILQ_FOREACH_SAFE(cs, &priv->cmndlist, entries, next) {
	    fputs("rule,", fp);
	    print_member_list_csv(fp, parse_tree, &us->users, false,
		USERALIAS, expand_aliases);
	    putc(',', fp);

	    print_member_list_csv(fp, parse_tree, &priv->hostlist, false,
		HOSTALIAS, expand_aliases);
	    putc(',', fp);

	    print_cmndspec_csv(fp, parse_tree, cs, &next, &priv->defaults,
		expand_aliases);
	    putc('\n', fp);
	}
    }

    debug_return_bool(!ferror(fp));
}

/*
 * Print User_Specs.
 */
static bool
print_userspecs_csv(FILE *fp, const struct sudoers_parse_tree *parse_tree,
    bool expand_aliases)
{
    struct userspec *us;
    debug_decl(print_userspecs_csv, SUDOERS_DEBUG_UTIL);

    if (TAILQ_EMPTY(&parse_tree->userspecs))
	debug_return_bool(true);

    /* Heading line. */
    fputs("rule,user,host,runusers,rungroups,options,command\n", fp);
 
    TAILQ_FOREACH(us, &parse_tree->userspecs, entries) {
	if (!print_userspec_csv(fp, parse_tree, us, expand_aliases))
	    debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Export the parsed sudoers file in CSV format.
 */
bool
convert_sudoers_csv(const struct sudoers_parse_tree *parse_tree,
    const char *output_file, struct cvtsudoers_config *conf)
{
    bool ret = true;
    FILE *output_fp = stdout;
    debug_decl(convert_sudoers_csv, SUDOERS_DEBUG_UTIL);

    if (output_file != NULL && strcmp(output_file, "-") != 0) {
	if ((output_fp = fopen(output_file, "w")) == NULL)
	    sudo_fatal(U_("unable to open %s"), output_file);
    }

    /* Dump Defaults in CSV format. */
    if (!ISSET(conf->suppress, SUPPRESS_DEFAULTS))
	print_defaults_csv(output_fp, parse_tree, conf->expand_aliases);

    /* Dump Aliases in CSV format. */
    if (!conf->expand_aliases && !ISSET(conf->suppress, SUPPRESS_ALIASES)) {
	print_aliases_csv(output_fp, parse_tree);
    }

    /* Dump User_Specs in CSV format. */
    if (!ISSET(conf->suppress, SUPPRESS_PRIVS))
	print_userspecs_csv(output_fp, parse_tree, conf->expand_aliases);

    (void)fflush(output_fp);
    if (ferror(output_fp))
	ret = false;
    if (output_fp != stdout)
	fclose(output_fp);

    debug_return_bool(ret);
}
