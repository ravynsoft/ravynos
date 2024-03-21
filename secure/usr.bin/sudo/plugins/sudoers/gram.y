%{
/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2013, 2014-2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sudoers.h>
#include <sudo_digest.h>
#include <toke.h>

#ifdef YYBISON
# define YYERROR_VERBOSE
#endif

/* If we last saw a newline the entry is on the preceding line. */
#define this_lineno	(sudoerschar == '\n' ? sudolineno - 1 : sudolineno)

// PVS Studio suppression
// -V::560, 592, 1037, 1042

/*
 * Globals
 */
bool parse_error = false;

static struct sudoers_parser_config parser_conf =
    SUDOERS_PARSER_CONFIG_INITIALIZER;

/* Optional logging function for parse errors. */
sudoers_logger_t sudoers_error_hook;

static int alias_line, alias_column;

#ifdef NO_LEAKS
static struct parser_leak_list parser_leak_list =
    SLIST_HEAD_INITIALIZER(parser_leak_list);
#endif

struct sudoers_parse_tree parsed_policy = {
    { NULL, NULL }, /* entries */
    TAILQ_HEAD_INITIALIZER(parsed_policy.userspecs),
    TAILQ_HEAD_INITIALIZER(parsed_policy.defaults),
    NULL, /* aliases */
    NULL, /* lhost */
    NULL, /* shost */
    NULL, /* nss */
    NULL  /* ctx */
};

/*
 * Local prototypes
 */
static void init_options(struct command_options *opts);
static bool add_defaults(short, struct member *, struct defaults *);
static bool add_userspec(struct member *, struct privilege *);
static struct defaults *new_default(char *, char *, short);
static struct member *new_member(char *, short);
static struct sudo_command *new_command(char *, char *);
static struct command_digest *new_digest(unsigned int, char *);
static void alias_error(const char *name, int errnum);
%}

%union {
    struct cmndspec *cmndspec;
    struct defaults *defaults;
    struct member *member;
    struct runascontainer *runas;
    struct privilege *privilege;
    struct command_digest *digest;
    struct sudo_command command;
    struct command_options options;
    struct cmndtag tag;
    char *string;
    const char *cstring;
    int tok;
}

%start file				/* special start symbol */
%token <command> COMMAND		/* absolute pathname w/ optional args */
%token <string>  ALIAS			/* an UPPERCASE alias name */
%token <string>	 DEFVAR			/* a Defaults variable name */
%token <string>  NTWKADDR		/* ipv4 or ipv6 address */
%token <string>  NETGROUP		/* a netgroup (+NAME) */
%token <string>  USERGROUP		/* a usergroup (%NAME) */
%token <string>  WORD			/* a word */
%token <string>  DIGEST			/* a SHA-2 digest */
%token <tok>	 INCLUDE		/* @include */
%token <tok>	 INCLUDEDIR		/* @includedir */
%token <tok>	 DEFAULTS		/* Defaults entry */
%token <tok>	 DEFAULTS_HOST		/* Host-specific defaults entry */
%token <tok>	 DEFAULTS_USER		/* User-specific defaults entry */
%token <tok>	 DEFAULTS_RUNAS		/* Runas-specific defaults entry */
%token <tok>	 DEFAULTS_CMND		/* Command-specific defaults entry */
%token <tok> 	 NOPASSWD		/* no passwd req for command */
%token <tok> 	 PASSWD			/* passwd req for command (default) */
%token <tok> 	 NOEXEC			/* preload fake execve() for cmnd */
%token <tok> 	 EXEC			/* don't preload fake execve() */
%token <tok>	 SETENV			/* user may set environment for cmnd */
%token <tok>	 NOSETENV		/* user may not set environment */
%token <tok>	 LOG_INPUT		/* log user's cmnd input */
%token <tok>	 NOLOG_INPUT		/* don't log user's cmnd input */
%token <tok>	 LOG_OUTPUT		/* log cmnd output */
%token <tok>	 NOLOG_OUTPUT		/* don't log cmnd output */
%token <tok>	 MAIL			/* mail log message */
%token <tok>	 NOMAIL			/* don't mail log message */
%token <tok>	 FOLLOWLNK		/* follow symbolic links */
%token <tok>	 NOFOLLOWLNK		/* don't follow symbolic links */
%token <tok>	 INTERCEPT		/* intercept children of command */
%token <tok>	 NOINTERCEPT		/* disable intercepting of children */
%token <tok>	 ALL			/* ALL keyword */
%token <tok>	 HOSTALIAS		/* Host_Alias keyword */
%token <tok>	 CMNDALIAS		/* Cmnd_Alias keyword */
%token <tok>	 USERALIAS		/* User_Alias keyword */
%token <tok>	 RUNASALIAS		/* Runas_Alias keyword */
%token <tok>	 ':' '=' ',' '!' '+' '-' /* union member tokens */
%token <tok>	 '(' ')'		/* runas tokens */
%token <tok>	 '\n'			/* newline (with optional comment) */
%token <tok>	 ERROR			/* error from lexer */
%token <tok>	 NOMATCH		/* no match from lexer */
%token <tok>	 CHROOT			/* root directory for command */
%token <tok>	 CWD			/* working directory for command */
%token <tok>	 TYPE			/* SELinux type */
%token <tok>	 ROLE			/* SELinux role */
%token <tok>	 APPARMOR_PROFILE	/* AppArmor profile */
%token <tok>	 PRIVS			/* Solaris privileges */
%token <tok>	 LIMITPRIVS		/* Solaris limit privileges */
%token <tok>	 CMND_TIMEOUT		/* command timeout */
%token <tok>	 NOTBEFORE		/* time restriction */
%token <tok>	 NOTAFTER		/* time restriction */
%token <tok>	 MYSELF			/* run as myself, not another user */
%token <tok>	 SHA224_TOK		/* sha224 token */
%token <tok>	 SHA256_TOK		/* sha256 token */
%token <tok>	 SHA384_TOK		/* sha384 token */
%token <tok>	 SHA512_TOK		/* sha512 token */

%type <cmndspec>  cmndspec
%type <cmndspec>  cmndspeclist
%type <defaults>  defaults_entry
%type <defaults>  defaults_list
%type <member>	  cmnd
%type <member>	  opcmnd
%type <member>	  digcmnd
%type <member>	  cmndlist
%type <member>	  host
%type <member>	  hostlist
%type <member>	  ophost
%type <member>	  opuser
%type <member>	  user
%type <member>	  userlist
%type <member>	  opgroup
%type <member>	  group
%type <member>	  grouplist
%type <runas>	  runasspec
%type <runas>	  runaslist
%type <privilege> privilege
%type <privilege> privileges
%type <tag>	  cmndtag
%type <options>	  options
%type <string>	  chdirspec
%type <string>	  chrootspec
%type <string>	  rolespec
%type <string>	  typespec
%type <string>	  apparmor_profilespec
%type <string>	  privsspec
%type <string>	  limitprivsspec
%type <string>	  timeoutspec
%type <string>	  notbeforespec
%type <string>	  notafterspec
%type <string>	  include
%type <string>	  includedir
%type <digest>	  digestspec
%type <digest>	  digestlist
%type <cstring>	  reserved_word

%%

file		:	{
			    ; /* empty file */
			}
		|	line
		;

line		:	entry
		|	line entry
		;

entry		:	'\n' {
			    ; /* blank line */
			}
                |       error '\n' {
			    yyerrok;
			}
		|	include {
			    const bool success = push_include($1,
				parsed_policy.ctx->user.shost, &parser_conf);
			    parser_leak_remove(LEAK_PTR, $1);
			    free($1);
			    if (!success && !parser_conf.recovery)
				YYERROR;
			}
		|	includedir {
			    const bool success = push_includedir($1,
				parsed_policy.ctx->user.shost, &parser_conf);
			    parser_leak_remove(LEAK_PTR, $1);
			    free($1);
			    if (!success && !parser_conf.recovery)
				YYERROR;
			}
		|	userlist privileges '\n' {
			    if (!add_userspec($1, $2)) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			}
		|	USERALIAS useraliases '\n' {
			    ;
			}
		|	HOSTALIAS hostaliases '\n' {
			    ;
			}
		|	CMNDALIAS cmndaliases '\n' {
			    ;
			}
		|	RUNASALIAS runasaliases '\n' {
			    ;
			}
		|	DEFAULTS defaults_list '\n' {
			    if (!add_defaults(DEFAULTS, NULL, $2))
				YYERROR;
			}
		|	DEFAULTS_USER userlist defaults_list '\n' {
			    if (!add_defaults(DEFAULTS_USER, $2, $3))
				YYERROR;
			}
		|	DEFAULTS_RUNAS userlist defaults_list '\n' {
			    if (!add_defaults(DEFAULTS_RUNAS, $2, $3))
				YYERROR;
			}
		|	DEFAULTS_HOST hostlist defaults_list '\n' {
			    if (!add_defaults(DEFAULTS_HOST, $2, $3))
				YYERROR;
			}
		|	DEFAULTS_CMND cmndlist defaults_list '\n' {
			    if (!add_defaults(DEFAULTS_CMND, $2, $3))
				YYERROR;
			}
		;

include		:	INCLUDE WORD '\n' {
			    $$ = $2;
			}
		|	INCLUDE WORD error '\n' {
			    yyerrok;
			    $$ = $2;
			}
		;

includedir	:	INCLUDEDIR WORD '\n' {
			    $$ = $2;
			}
		|	INCLUDEDIR WORD error '\n' {
			    yyerrok;
			    $$ = $2;
			}
		;

defaults_list	:	defaults_entry
		|	defaults_list ',' defaults_entry {
			    parser_leak_remove(LEAK_DEFAULTS, $3);
			    HLTQ_CONCAT($1, $3, entries);
			    $$ = $1;
			}
		;

defaults_entry	:	DEFVAR {
			    $$ = new_default($1, NULL, true);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_DEFAULTS, $$);
			}
		|	'!' DEFVAR {
			    $$ = new_default($2, NULL, false);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $2);
			    parser_leak_add(LEAK_DEFAULTS, $$);
			}
		|	DEFVAR '=' WORD {
			    $$ = new_default($1, $3, true);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_remove(LEAK_PTR, $3);
			    parser_leak_add(LEAK_DEFAULTS, $$);
			}
		|	DEFVAR '+' WORD {
			    $$ = new_default($1, $3, '+');
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_remove(LEAK_PTR, $3);
			    parser_leak_add(LEAK_DEFAULTS, $$);
			}
		|	DEFVAR '-' WORD {
			    $$ = new_default($1, $3, '-');
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_remove(LEAK_PTR, $3);
			    parser_leak_add(LEAK_DEFAULTS, $$);
			}
		;

privileges	:	privilege
		|	privileges ':' privilege {
			    parser_leak_remove(LEAK_PRIVILEGE, $3);
			    HLTQ_CONCAT($1, $3, entries);
			    $$ = $1;
			}
		|	privileges ':' error {
			    yyerrok;
			    $$ = $1;
			}
		;

privilege	:	hostlist '=' cmndspeclist {
			    struct privilege *p = calloc(1, sizeof(*p));
			    if (p == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_PRIVILEGE, p);
			    TAILQ_INIT(&p->defaults);
			    parser_leak_remove(LEAK_MEMBER, $1);
			    HLTQ_TO_TAILQ(&p->hostlist, $1, entries);
			    parser_leak_remove(LEAK_CMNDSPEC, $3);
			    HLTQ_TO_TAILQ(&p->cmndlist, $3, entries);
			    HLTQ_INIT(p, entries);
			    $$ = p;
			}
		;

ophost		:	host {
			    $$ = $1;
			    $$->negated = false;
			}
		|	'!' host {
			    $$ = $2;
			    $$->negated = true;
			}
		;

host		:	ALIAS {
			    $$ = new_member($1, ALIAS);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	ALL {
			    $$ = new_member(NULL, ALL);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	NETGROUP {
			    $$ = new_member($1, NETGROUP);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	NTWKADDR {
			    $$ = new_member($1, NTWKADDR);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	WORD {
			    $$ = new_member($1, WORD);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		;

cmndspeclist	:	cmndspec
		|	cmndspeclist ',' cmndspec {
			    struct cmndspec *prev;
			    prev = HLTQ_LAST($1, cmndspec, entries);
			    parser_leak_remove(LEAK_CMNDSPEC, $3);
			    HLTQ_CONCAT($1, $3, entries);

			    /* propagate runcwd and runchroot */
			    if ($3->runcwd == NULL)
				$3->runcwd = prev->runcwd;
			    if ($3->runchroot == NULL)
				$3->runchroot = prev->runchroot;
#ifdef HAVE_SELINUX
			    /* propagate role and type */
			    if ($3->role == NULL && $3->type == NULL) {
				$3->role = prev->role;
				$3->type = prev->type;
			    }
#endif /* HAVE_SELINUX */
#ifdef HAVE_PRIV_SET
			    /* propagate privs & limitprivs */
			    if ($3->privs == NULL && $3->limitprivs == NULL) {
			        $3->privs = prev->privs;
			        $3->limitprivs = prev->limitprivs;
			    }
#endif /* HAVE_PRIV_SET */
			    /* propagate command time restrictions */
			    if ($3->notbefore == UNSPEC)
				$3->notbefore = prev->notbefore;
			    if ($3->notafter == UNSPEC)
				$3->notafter = prev->notafter;
			    /* propagate command timeout */
			    if ($3->timeout == UNSPEC)
				$3->timeout = prev->timeout;
			    /* propagate tags and runas list */
			    if ($3->tags.nopasswd == UNSPEC)
				$3->tags.nopasswd = prev->tags.nopasswd;
			    if ($3->tags.noexec == UNSPEC)
				$3->tags.noexec = prev->tags.noexec;
			    if ($3->tags.intercept == UNSPEC)
				$3->tags.intercept = prev->tags.intercept;
			    if ($3->tags.setenv == UNSPEC &&
				prev->tags.setenv != IMPLIED)
				$3->tags.setenv = prev->tags.setenv;
			    if ($3->tags.log_input == UNSPEC)
				$3->tags.log_input = prev->tags.log_input;
			    if ($3->tags.log_output == UNSPEC)
				$3->tags.log_output = prev->tags.log_output;
			    if ($3->tags.send_mail == UNSPEC)
				$3->tags.send_mail = prev->tags.send_mail;
			    if ($3->tags.follow == UNSPEC)
				$3->tags.follow = prev->tags.follow;
			    if (($3->runasuserlist == NULL &&
				 $3->runasgrouplist == NULL) &&
				(prev->runasuserlist != NULL ||
				 prev->runasgrouplist != NULL)) {
				$3->runasuserlist = prev->runasuserlist;
				$3->runasgrouplist = prev->runasgrouplist;
			    }
			    $$ = $1;
			}
		;

cmndspec	:	runasspec options cmndtag digcmnd {
			    struct cmndspec *cs = calloc(1, sizeof(*cs));
			    if (cs == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_CMNDSPEC, cs);
			    if ($1 != NULL) {
				if ($1->runasusers != NULL) {
				    cs->runasuserlist =
					malloc(sizeof(*cs->runasuserlist));
				    if (cs->runasuserlist == NULL) {
					free(cs);
					sudoerserror(N_("unable to allocate memory"));
					YYERROR;
				    }
				    /* g/c done via runas container */
				    HLTQ_TO_TAILQ(cs->runasuserlist,
					$1->runasusers, entries);
				}
				if ($1->runasgroups != NULL) {
				    cs->runasgrouplist =
					malloc(sizeof(*cs->runasgrouplist));
				    if (cs->runasgrouplist == NULL) {
					free(cs);
					sudoerserror(N_("unable to allocate memory"));
					YYERROR;
				    }
				    /* g/c done via runas container */
				    HLTQ_TO_TAILQ(cs->runasgrouplist,
					$1->runasgroups, entries);
				}
				parser_leak_remove(LEAK_RUNAS, $1);
				free($1);
			    }
#ifdef HAVE_SELINUX
			    cs->role = $2.role;
			    parser_leak_remove(LEAK_PTR, $2.role);
			    cs->type = $2.type;
			    parser_leak_remove(LEAK_PTR, $2.type);
#endif
#ifdef HAVE_APPARMOR
			    cs->apparmor_profile = $2.apparmor_profile;
			    parser_leak_remove(LEAK_PTR, $2.apparmor_profile);
#endif
#ifdef HAVE_PRIV_SET
			    cs->privs = $2.privs;
			    parser_leak_remove(LEAK_PTR, $2.privs);
			    cs->limitprivs = $2.limitprivs;
			    parser_leak_remove(LEAK_PTR, $2.limitprivs);
#endif
			    cs->notbefore = $2.notbefore;
			    cs->notafter = $2.notafter;
			    cs->timeout = $2.timeout;
			    cs->runcwd = $2.runcwd;
			    parser_leak_remove(LEAK_PTR, $2.runcwd);
			    cs->runchroot = $2.runchroot;
			    parser_leak_remove(LEAK_PTR, $2.runchroot);
			    cs->tags = $3;
			    cs->cmnd = $4;
			    parser_leak_remove(LEAK_MEMBER, $4);
			    HLTQ_INIT(cs, entries);
			    /* sudo "ALL" implies the SETENV tag */
			    if (cs->cmnd->type == ALL && !cs->cmnd->negated &&
				cs->tags.setenv == UNSPEC)
				cs->tags.setenv = IMPLIED;
			    $$ = cs;
			}
		;

digestspec	:	SHA224_TOK ':' DIGEST {
			    $$ = new_digest(SUDO_DIGEST_SHA224, $3);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $3);
			    parser_leak_add(LEAK_DIGEST, $$);
			}
		|	SHA256_TOK ':' DIGEST {
			    $$ = new_digest(SUDO_DIGEST_SHA256, $3);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $3);
			    parser_leak_add(LEAK_DIGEST, $$);
			}
		|	SHA384_TOK ':' DIGEST {
			    $$ = new_digest(SUDO_DIGEST_SHA384, $3);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $3);
			    parser_leak_add(LEAK_DIGEST, $$);
			}
		|	SHA512_TOK ':' DIGEST {
			    $$ = new_digest(SUDO_DIGEST_SHA512, $3);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $3);
			    parser_leak_add(LEAK_DIGEST, $$);
			}
		;

digestlist	:	digestspec
		|	digestlist ',' digestspec {
			    parser_leak_remove(LEAK_DIGEST, $3);
			    HLTQ_CONCAT($1, $3, entries);
			    $$ = $1;
			}
		;

digcmnd		:	opcmnd {
			    $$ = $1;
			}
		|	digestlist opcmnd {
			    struct sudo_command *c =
				(struct sudo_command *) $2->name;

			    if ($2->type != COMMAND && $2->type != ALL) {
				sudoerserror(N_("a digest requires a path name"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_DIGEST, $1);
			    HLTQ_TO_TAILQ(&c->digests, $1, entries);
			    $$ = $2;
			}
		;

opcmnd		:	cmnd {
			    $$ = $1;
			    $$->negated = false;
			}
		|	'!' cmnd {
			    $$ = $2;
			    $$->negated = true;
			}
		;

chdirspec	:	CWD '=' WORD {
			    if ($3[0] != '/' && $3[0] != '~') {
				if (strcmp($3, "*") != 0) {
				    sudoerserror(N_("values for \"CWD\" must"
					" start with a '/', '~', or '*'"));
				    YYERROR;
				}
			    }
			    if (strlen($3) >= PATH_MAX) {
				sudoerserror(N_("\"CWD\" path too long"));
				YYERROR;
			    }
			    $$ = $3;
			}
		;

chrootspec	:	CHROOT '=' WORD {
			    if ($3[0] != '/' && $3[0] != '~') {
				if (strcmp($3, "*") != 0) {
				    sudoerserror(N_("values for \"CHROOT\" must"
					" start with a '/', '~', or '*'"));
				    YYERROR;
				}
			    }
			    if (strlen($3) >= PATH_MAX) {
				sudoerserror(N_("\"CHROOT\" path too long"));
				YYERROR;
			    }
			    $$ = $3;
			}
		;

timeoutspec	:	CMND_TIMEOUT '=' WORD {
			    $$ = $3;
			}
		;

notbeforespec	:	NOTBEFORE '=' WORD {
			    $$ = $3;
			}

notafterspec	:	NOTAFTER '=' WORD {
			    $$ = $3;
			}
		;

rolespec	:	ROLE '=' WORD {
			    $$ = $3;
			}
		;

typespec	:	TYPE '=' WORD {
			    $$ = $3;
			}
		;

apparmor_profilespec	:	APPARMOR_PROFILE '=' WORD {
				$$ = $3;
			}
		;

privsspec	:	PRIVS '=' WORD {
			    $$ = $3;
			}
		;
limitprivsspec	:	LIMITPRIVS '=' WORD {
			    $$ = $3;
			}
		;

runasspec	:	/* empty */ {
			    $$ = NULL;
			}
		|	'(' runaslist ')' {
			    $$ = $2;
			}
		;

runaslist	:	/* empty */ {
			    /* User may run command as themselves. */
			    $$ = calloc(1, sizeof(struct runascontainer));
			    if ($$ != NULL) {
				$$->runasusers = new_member(NULL, MYSELF);
				/* $$->runasgroups = NULL; */
				if ($$->runasusers == NULL) {
				    free($$);
				    $$ = NULL;
				}
			    }
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_RUNAS, $$);
			}
		|	userlist {
			    /* User may run command as a user in userlist. */
			    $$ = calloc(1, sizeof(struct runascontainer));
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_RUNAS, $$);
			    parser_leak_remove(LEAK_MEMBER, $1);
			    $$->runasusers = $1;
			    /* $$->runasgroups = NULL; */
			}
		|	userlist ':' grouplist {
			    /*
			     * User may run command as a user in userlist
			     * and optionally as a group in grouplist.
			     */
			    $$ = calloc(1, sizeof(struct runascontainer));
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_RUNAS, $$);
			    parser_leak_remove(LEAK_MEMBER, $1);
			    parser_leak_remove(LEAK_MEMBER, $3);
			    $$->runasusers = $1;
			    $$->runasgroups = $3;
			}
		|	':' grouplist {
			    /* User may run command as a group in grouplist. */
			    $$ = calloc(1, sizeof(struct runascontainer));
			    if ($$ != NULL) {
				$$->runasusers = new_member(NULL, MYSELF);
				if ($$->runasusers == NULL) {
				    free($$);
				    $$ = NULL;
				}
			    }
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_RUNAS, $$);
			    parser_leak_remove(LEAK_MEMBER, $2);
			    $$->runasgroups = $2;
			}
		|	':' {
			    /* User may run command as themselves. */
			    $$ = calloc(1, sizeof(struct runascontainer));
			    if ($$ != NULL) {
				$$->runasusers = new_member(NULL, MYSELF);
				/* $$->runasgroups = NULL; */
				if ($$->runasusers == NULL) {
				    free($$);
				    $$ = NULL;
				}
			    }
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_RUNAS, $$);
			}
		;

reserved_word	:	ALL		{ $$ = "ALL"; }
		|	CHROOT		{ $$ = "CHROOT"; }
		|	CWD		{ $$ = "CWD"; }
		|	CMND_TIMEOUT	{ $$ = "CMND_TIMEOUT"; }
		|	NOTBEFORE	{ $$ = "NOTBEFORE"; }
		|	NOTAFTER	{ $$ = "NOTAFTER"; }
		|	ROLE		{ $$ = "ROLE"; }
		|	TYPE		{ $$ = "TYPE"; }
		|	PRIVS		{ $$ = "PRIVS"; }
		|	LIMITPRIVS	{ $$ = "LIMITPRIVS"; }
		|	APPARMOR_PROFILE { $$ = "APPARMOR_PROFILE"; }
		;

reserved_alias	:	reserved_word {
			    sudoerserrorf(U_("syntax error, reserved word %s used as an alias name"), $1);
			    YYERROR;
			}
		;

options		:	/* empty */ {
			    init_options(&$$);
			}
		|	options chdirspec {
			    parser_leak_remove(LEAK_PTR, $$.runcwd);
			    free($$.runcwd);
			    $$.runcwd = $2;
			}
		|	options chrootspec {
			    parser_leak_remove(LEAK_PTR, $$.runchroot);
			    free($$.runchroot);
			    $$.runchroot = $2;
			}
		|	options notbeforespec {
			    $$.notbefore = parse_gentime($2);
			    parser_leak_remove(LEAK_PTR, $2);
			    free($2);
			    if ($$.notbefore == -1) {
				sudoerserror(N_("invalid notbefore value"));
				YYERROR;
			    }
			}
		|	options notafterspec {
			    $$.notafter = parse_gentime($2);
			    parser_leak_remove(LEAK_PTR, $2);
			    free($2);
			    if ($$.notafter == -1) {
				sudoerserror(N_("invalid notafter value"));
				YYERROR;
			    }
			}
		|	options timeoutspec {
			    $$.timeout = parse_timeout($2);
			    parser_leak_remove(LEAK_PTR, $2);
			    free($2);
			    if ($$.timeout == -1) {
				if (errno == ERANGE)
				    sudoerserror(N_("timeout value too large"));
				else
				    sudoerserror(N_("invalid timeout value"));
				YYERROR;
			    }
			}
		|	options rolespec {
#ifdef HAVE_SELINUX
			    parser_leak_remove(LEAK_PTR, $$.role);
			    free($$.role);
			    $$.role = $2;
#endif
			}
		|	options typespec {
#ifdef HAVE_SELINUX
			    parser_leak_remove(LEAK_PTR, $$.type);
			    free($$.type);
			    $$.type = $2;
#endif
			}
		|	options apparmor_profilespec {
#ifdef HAVE_APPARMOR
			    parser_leak_remove(LEAK_PTR, $$.apparmor_profile);
			    free($$.apparmor_profile);
			    $$.apparmor_profile = $2;
#endif
			}
		|	options privsspec {
#ifdef HAVE_PRIV_SET
			    parser_leak_remove(LEAK_PTR, $$.privs);
			    free($$.privs);
			    $$.privs = $2;
#endif
			}
		|	options limitprivsspec {
#ifdef HAVE_PRIV_SET
			    parser_leak_remove(LEAK_PTR, $$.limitprivs);
			    free($$.limitprivs);
			    $$.limitprivs = $2;
#endif
			}
		;

cmndtag		:	/* empty */ {
			    TAGS_INIT(&$$);
			}
		|	cmndtag NOPASSWD {
			    $$.nopasswd = true;
			}
		|	cmndtag PASSWD {
			    $$.nopasswd = false;
			}
		|	cmndtag NOEXEC {
			    $$.noexec = true;
			}
		|	cmndtag EXEC {
			    $$.noexec = false;
			}
		|	cmndtag INTERCEPT {
			    $$.intercept = true;
			}
		|	cmndtag NOINTERCEPT {
			    $$.intercept = false;
			}
		|	cmndtag SETENV {
			    $$.setenv = true;
			}
		|	cmndtag NOSETENV {
			    $$.setenv = false;
			}
		|	cmndtag LOG_INPUT {
			    $$.log_input = true;
			}
		|	cmndtag NOLOG_INPUT {
			    $$.log_input = false;
			}
		|	cmndtag LOG_OUTPUT {
			    $$.log_output = true;
			}
		|	cmndtag NOLOG_OUTPUT {
			    $$.log_output = false;
			}
		|	cmndtag FOLLOWLNK {
			    $$.follow = true;
			}
		|	cmndtag NOFOLLOWLNK {
			    $$.follow = false;
			}
		|	cmndtag MAIL {
			    $$.send_mail = true;
			}
		|	cmndtag NOMAIL {
			    $$.send_mail = false;
			}
		;

cmnd		:	ALL {
			    struct sudo_command *c;

			    if ((c = new_command(NULL, NULL)) == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    $$ = new_member((char *)c, ALL);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	ALIAS {
			    $$ = new_member($1, ALIAS);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	COMMAND {
			    struct sudo_command *c;

			    if (strlen($1.cmnd) >= PATH_MAX) {
				sudoerserror(N_("command too long"));
				YYERROR;
			    }
			    if ((c = new_command($1.cmnd, $1.args)) == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    $$ = new_member((char *)c, COMMAND);
			    if ($$ == NULL) {
				free(c);
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1.cmnd);
			    parser_leak_remove(LEAK_PTR, $1.args);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	WORD {
			    if (strcmp($1, "list") == 0) {
				struct sudo_command *c;

				if ((c = new_command($1, NULL)) == NULL) {
				    sudoerserror(N_("unable to allocate memory"));
				    YYERROR;
				}
				$$ = new_member((char *)c, COMMAND);
				if ($$ == NULL) {
				    free(c);
				    sudoerserror(N_("unable to allocate memory"));
				    YYERROR;
				}
				parser_leak_remove(LEAK_PTR, $1);
				parser_leak_add(LEAK_MEMBER, $$);
			    } else {
				sudoerserror(N_("expected a fully-qualified path name"));
				YYERROR;
			    }
			}
		;

hostaliases	:	hostalias
		|	hostaliases ':' hostalias
		;

hostalias	:	ALIAS {
			    alias_line = this_lineno;
			    alias_column = (int)sudolinebuf.toke_start + 1;
			} '=' hostlist {
			    if (!alias_add(&parsed_policy, $1, HOSTALIAS,
				sudoers, alias_line, alias_column, $4)) {
				alias_error($1, errno);
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_remove(LEAK_MEMBER, $4);
			}
		|	reserved_alias '=' hostlist
		;

hostlist	:	ophost
		|	hostlist ',' ophost {
			    parser_leak_remove(LEAK_MEMBER, $3);
			    HLTQ_CONCAT($1, $3, entries);
			    $$ = $1;
			}
		;

cmndaliases	:	cmndalias
		|	cmndaliases ':' cmndalias
		;

cmndalias	:	ALIAS {
			    alias_line = this_lineno;
			    alias_column = (int)sudolinebuf.toke_start + 1;
			} '=' cmndlist {
			    if (!alias_add(&parsed_policy, $1, CMNDALIAS,
				sudoers, alias_line, alias_column, $4)) {
				alias_error($1, errno);
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_remove(LEAK_MEMBER, $4);
			}
		|	reserved_alias '=' cmndlist
		;

cmndlist	:	digcmnd
		|	cmndlist ',' digcmnd {
			    parser_leak_remove(LEAK_MEMBER, $3);
			    HLTQ_CONCAT($1, $3, entries);
			    $$ = $1;
			}
		;

runasaliases	:	runasalias
		|	runasaliases ':' runasalias
		;

runasalias	:	ALIAS {
			    alias_line = this_lineno;
			    alias_column = (int)sudolinebuf.toke_start + 1;
			} '=' userlist {
			    if (!alias_add(&parsed_policy, $1, RUNASALIAS,
				sudoers, alias_line, alias_column, $4)) {
				alias_error($1, errno);
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_remove(LEAK_MEMBER, $4);
			}
		|	reserved_alias '=' userlist
		;

useraliases	:	useralias
		|	useraliases ':' useralias
		;

useralias	:	ALIAS {
			    alias_line = this_lineno;
			    alias_column = (int)sudolinebuf.toke_start + 1;
			} '=' userlist {
			    if (!alias_add(&parsed_policy, $1, USERALIAS,
				sudoers, alias_line, alias_column, $4)) {
				alias_error($1, errno);
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_remove(LEAK_MEMBER, $4);
			}
		|	reserved_alias '=' userlist
		;

userlist	:	opuser
		|	userlist ',' opuser {
			    parser_leak_remove(LEAK_MEMBER, $3);
			    HLTQ_CONCAT($1, $3, entries);
			    $$ = $1;
			}
		;

opuser		:	user {
			    $$ = $1;
			    $$->negated = false;
			}
		|	'!' user {
			    $$ = $2;
			    $$->negated = true;
			}
		;

user		:	ALIAS {
			    $$ = new_member($1, ALIAS);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	ALL {
			    $$ = new_member(NULL, ALL);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	NETGROUP {
			    $$ = new_member($1, NETGROUP);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	USERGROUP {
			    $$ = new_member($1, USERGROUP);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	WORD {
			    $$ = new_member($1, WORD);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		;

grouplist	:	opgroup
		|	grouplist ',' opgroup {
			    parser_leak_remove(LEAK_MEMBER, $3);
			    HLTQ_CONCAT($1, $3, entries);
			    $$ = $1;
			}
		;

opgroup		:	group {
			    $$ = $1;
			    $$->negated = false;
			}
		|	'!' group {
			    $$ = $2;
			    $$->negated = true;
			}
		;

group		:	ALIAS {
			    $$ = new_member($1, ALIAS);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	ALL {
			    $$ = new_member(NULL, ALL);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		|	WORD {
			    $$ = new_member($1, WORD);
			    if ($$ == NULL) {
				sudoerserror(N_("unable to allocate memory"));
				YYERROR;
			    }
			    parser_leak_remove(LEAK_PTR, $1);
			    parser_leak_add(LEAK_MEMBER, $$);
			}
		;
%%
/* Like yyerror() but takes a printf-style format string. */
void
sudoerserrorf(const char * restrict fmt, ...)
{
    const int column = (int)(sudolinebuf.toke_start + 1);
    va_list ap;
    debug_decl(sudoerserrorf, SUDOERS_DEBUG_PARSER);

    if (sudoers_error_hook != NULL) {
	va_start(ap, fmt);
	sudoers_error_hook(parsed_policy.ctx, sudoers, this_lineno, column,
	    fmt, ap);
	va_end(ap);
    }
    if (parser_conf.verbose > 0 && fmt != NULL) {
	LEXTRACE("<*> ");
#ifndef TRACELEXER
	if (trace_print == NULL || trace_print == sudoers_trace_print) {
	    char *tofree = NULL;
	    const char *s;
	    int oldlocale;

	    /* Warnings are displayed in the user's locale. */
	    sudoers_setlocale(SUDOERS_LOCALE_USER, &oldlocale);

	    va_start(ap, fmt);
	    if (strcmp(fmt, "%s") == 0) {
		/* Optimize common case, a single string. */
		s = _(va_arg(ap, char *));
	    } else {
		if (vasprintf(&tofree, _(fmt), ap) != -1) {
		    s = tofree;
		} else {
		    s = _("syntax error");
		    tofree = NULL;
		}
	    }
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("%s:%d:%zu: %s\n"), sudoers,
		this_lineno, sudolinebuf.toke_start + 1, s);
	    free(tofree);
	    va_end(ap);
	    sudoers_setlocale(oldlocale, NULL);

	    /* Display the offending line and token if possible. */
	    if (sudolinebuf.len != 0) {
		char tildes[128];
		size_t tlen = 0;

		sudo_printf(SUDO_CONV_ERROR_MSG, "%s%s", sudolinebuf.buf,
		    sudolinebuf.buf[sudolinebuf.len - 1] == '\n' ? "" : "\n");
		if (sudolinebuf.toke_end > sudolinebuf.toke_start) {
		    tlen = sudolinebuf.toke_end - sudolinebuf.toke_start - 1;
		    if (tlen >= sizeof(tildes))
			tlen = sizeof(tildes) - 1;
		    memset(tildes, '~', tlen);
		}
		tildes[tlen] = '\0';
		sudo_printf(SUDO_CONV_ERROR_MSG, "%*s^%s\n",
		    (int)sudolinebuf.toke_start, "", tildes);
	    }
	}
#endif
    }
    parse_error = true;
    debug_return;
}

void
sudoerserror(const char *s)
{
    if (sudoerschar == ERROR) {
	/* Use error string from lexer. */
	s = sudoers_errstr;
	sudoers_errstr = NULL;
    }

#pragma pvs(push)
#pragma pvs(disable: 575, 618)

    if (s == NULL)
	sudoerserrorf(NULL);
    else
	sudoerserrorf("%s", s);

#pragma pvs(pop)
}

static void
alias_error(const char *name, int errnum)
{
    if (errnum == EEXIST)
	sudoerserrorf(U_("Alias \"%s\" already defined"), name);
    else
	sudoerserror(N_("unable to allocate memory"));
}

static struct defaults *
new_default(char *var, char *val, short op)
{
    struct defaults *d;
    debug_decl(new_default, SUDOERS_DEBUG_PARSER);

    if ((d = calloc(1, sizeof(struct defaults))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }

    d->var = var;
    d->val = val;
    /* d->type = 0; */
    d->op = op;
    /* d->binding = NULL; */
    d->line = this_lineno;
    d->column = (int)(sudolinebuf.toke_start + 1);
    d->file = sudo_rcstr_addref(sudoers);
    HLTQ_INIT(d, entries);

    debug_return_ptr(d);
}

static struct member *
new_member(char *name, short type)
{
    struct member *m;
    debug_decl(new_member, SUDOERS_DEBUG_PARSER);

    if ((m = calloc(1, sizeof(struct member))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }

    m->name = name;
    m->type = type;
    HLTQ_INIT(m, entries);

    debug_return_ptr(m);
}

static struct sudo_command *
new_command(char *cmnd, char *args)
{
    struct sudo_command *c;
    debug_decl(new_command, SUDOERS_DEBUG_PARSER);

    if ((c = calloc(1, sizeof(*c))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }
    /* garbage collected as part of struct member */

    c->cmnd = cmnd;
    c->args = args;
    TAILQ_INIT(&c->digests);

    debug_return_ptr(c);
}

static struct command_digest *
new_digest(unsigned int digest_type, char *digest_str)
{
    struct command_digest *digest;
    debug_decl(new_digest, SUDOERS_DEBUG_PARSER);

    if ((digest = malloc(sizeof(*digest))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }

    HLTQ_INIT(digest, entries);
    digest->digest_type = digest_type;
    digest->digest_str = digest_str;
    if (digest->digest_str == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	free(digest);
	digest = NULL;
    }

    debug_return_ptr(digest);
}

static void
free_defaults_binding(struct defaults_binding *binding)
{
    debug_decl(free_defaults_binding, SUDOERS_DEBUG_PARSER);

    /* Bindings may be shared among multiple Defaults entries. */
    if (binding != NULL) {
	if (--binding->refcnt == 0) {
	    free_members(&binding->members);
	    free(binding);
	}
    }

    debug_return;
}

/*
 * Add a list of defaults structures to the defaults list.
 * The bmem argument, if non-NULL, specifies a list of hosts, users,
 * or runas users the entries apply to (determined by the type).
 */
static bool
add_defaults(short type, struct member *bmem, struct defaults *defs)
{
    struct defaults *d, *next;
    struct defaults_binding *binding;
    bool ret = true;
    debug_decl(add_defaults, SUDOERS_DEBUG_PARSER);

    if (defs == NULL)
	debug_return_bool(false);

    /*
     * We use a single binding for each entry in defs.
     */
    if ((binding = malloc(sizeof(*binding))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	sudoerserror(N_("unable to allocate memory"));
	debug_return_bool(false);
    }
    if (bmem != NULL) {
	parser_leak_remove(LEAK_MEMBER, bmem);
	HLTQ_TO_TAILQ(&binding->members, bmem, entries);
    } else {
	TAILQ_INIT(&binding->members);
    }
    binding->refcnt = 0;

    /*
     * Set type and binding (who it applies to) for new entries.
     * Then add to the global defaults list.
     */
    parser_leak_remove(LEAK_DEFAULTS, defs);
    HLTQ_FOREACH_SAFE(d, defs, entries, next) {
	d->type = type;
	d->binding = binding;
	binding->refcnt++;
	TAILQ_INSERT_TAIL(&parsed_policy.defaults, d, entries);
    }

    debug_return_bool(ret);
}

/*
 * Allocate a new struct userspec, populate it, and insert it at the
 * end of the userspecs list.
 */
static bool
add_userspec(struct member *members, struct privilege *privs)
{
    struct userspec *u;
    debug_decl(add_userspec, SUDOERS_DEBUG_PARSER);

    if ((u = calloc(1, sizeof(*u))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_bool(false);
    }
    /* We already parsed the newline so sudolineno is off by one. */
    u->line = sudolineno - 1;
    u->column = (int)(sudolinebuf.toke_start + 1);
    u->file = sudo_rcstr_addref(sudoers);
    parser_leak_remove(LEAK_MEMBER, members);
    HLTQ_TO_TAILQ(&u->users, members, entries);
    parser_leak_remove(LEAK_PRIVILEGE, privs);
    HLTQ_TO_TAILQ(&u->privileges, privs, entries);
    STAILQ_INIT(&u->comments);
    TAILQ_INSERT_TAIL(&parsed_policy.userspecs, u, entries);

    debug_return_bool(true);
}

/*
 * Free a member struct and its contents.
 */
void
free_member(struct member *m)
{
    debug_decl(free_member, SUDOERS_DEBUG_PARSER);

    if (m->type == COMMAND || (m->type == ALL && m->name != NULL)) {
	struct command_digest *digest;
	struct sudo_command *c = (struct sudo_command *)m->name;
	free(c->cmnd);
	free(c->args);
	while ((digest = TAILQ_FIRST(&c->digests)) != NULL) {
	    TAILQ_REMOVE(&c->digests, digest, entries);
	    free(digest->digest_str);
	    free(digest);
	}
    }
    free(m->name);
    free(m);

    debug_return;
}

/*
 * Free a tailq of members but not the struct member_list container itself.
 */
void
free_members(struct member_list *members)
{
    struct member *m;
    debug_decl(free_members, SUDOERS_DEBUG_PARSER);

    while ((m = TAILQ_FIRST(members)) != NULL) {
	TAILQ_REMOVE(members, m, entries);
	free_member(m);
    }

    debug_return;
}

void
free_defaults(struct defaults_list *defs)
{
    struct defaults *def;
    debug_decl(free_defaults, SUDOERS_DEBUG_PARSER);

    while ((def = TAILQ_FIRST(defs)) != NULL) {
	TAILQ_REMOVE(defs, def, entries);
	free_default(def);
    }

    debug_return;
}

void
free_default(struct defaults *def)
{
    debug_decl(free_default, SUDOERS_DEBUG_PARSER);

    free_defaults_binding(def->binding);
    sudo_rcstr_delref(def->file);
    free(def->var);
    free(def->val);
    free(def);

    debug_return;
}

void
free_cmndspec(struct cmndspec *cs, struct cmndspec_list *csl)
{
    struct cmndspec *prev, *next;
    debug_decl(free_cmndspec, SUDOERS_DEBUG_PARSER);

    prev = TAILQ_PREV(cs, cmndspec_list, entries);
    next = TAILQ_NEXT(cs, entries);
    TAILQ_REMOVE(csl, cs, entries);

    /* Don't free runcwd/runchroot that are in use by other entries. */
    if ((prev == NULL || cs->runcwd != prev->runcwd) &&
	(next == NULL || cs->runcwd != next->runcwd)) {
	free(cs->runcwd);
    }
    if ((prev == NULL || cs->runchroot != prev->runchroot) &&
	(next == NULL || cs->runchroot != next->runchroot)) {
	free(cs->runchroot);
    }
#ifdef HAVE_SELINUX
    /* Don't free root/type that are in use by other entries. */
    if ((prev == NULL || cs->role != prev->role) &&
	(next == NULL || cs->role != next->role)) {
	free(cs->role);
    }
    if ((prev == NULL || cs->type != prev->type) &&
	(next == NULL || cs->type != next->type)) {
	free(cs->type);
    }
#endif /* HAVE_SELINUX */
#ifdef HAVE_PRIV_SET
    /* Don't free privs/limitprivs that are in use by other entries. */
    if ((prev == NULL || cs->privs != prev->privs) &&
	(next == NULL || cs->privs != next->privs)) {
	free(cs->privs);
    }
    if ((prev == NULL || cs->limitprivs != prev->limitprivs) &&
	(next == NULL || cs->limitprivs != next->limitprivs)) {
	free(cs->limitprivs);
    }
#endif /* HAVE_PRIV_SET */
    /* Don't free user/group lists that are in use by other entries. */
    if (cs->runasuserlist != NULL) {
	if ((prev == NULL || cs->runasuserlist != prev->runasuserlist) &&
	    (next == NULL || cs->runasuserlist != next->runasuserlist)) {
	    free_members(cs->runasuserlist);
	    free(cs->runasuserlist);
	}
    }
    if (cs->runasgrouplist != NULL) {
	if ((prev == NULL || cs->runasgrouplist != prev->runasgrouplist) &&
	    (next == NULL || cs->runasgrouplist != next->runasgrouplist)) {
	    free_members(cs->runasgrouplist);
	    free(cs->runasgrouplist);
	}
    }
    free_member(cs->cmnd);
    free(cs);

    debug_return;
}

void
free_cmndspecs(struct cmndspec_list *csl)
{
    struct member_list *runasuserlist = NULL, *runasgrouplist = NULL;
    char *runcwd = NULL, *runchroot = NULL;
#ifdef HAVE_SELINUX
    char *role = NULL, *type = NULL;
#endif /* HAVE_SELINUX */
#ifdef HAVE_PRIV_SET
    char *privs = NULL, *limitprivs = NULL;
#endif /* HAVE_PRIV_SET */
    struct cmndspec *cs;
    debug_decl(free_cmndspecs, SUDOERS_DEBUG_PARSER);

    while ((cs = TAILQ_FIRST(csl)) != NULL) {
	TAILQ_REMOVE(csl, cs, entries);

	/* Only free the first instance of runcwd/runchroot. */
	if (cs->runcwd != runcwd) {
	    runcwd = cs->runcwd;
	    free(cs->runcwd);
	}
	if (cs->runchroot != runchroot) {
	    runchroot = cs->runchroot;
	    free(cs->runchroot);
	}
#ifdef HAVE_SELINUX
	/* Only free the first instance of a role/type. */
	if (cs->role != role) {
	    role = cs->role;
	    free(cs->role);
	}
	if (cs->type != type) {
	    type = cs->type;
	    free(cs->type);
	}
#endif /* HAVE_SELINUX */
#ifdef HAVE_PRIV_SET
	/* Only free the first instance of privs/limitprivs. */
	if (cs->privs != privs) {
	    privs = cs->privs;
	    free(cs->privs);
	}
	if (cs->limitprivs != limitprivs) {
	    limitprivs = cs->limitprivs;
	    free(cs->limitprivs);
	}
#endif /* HAVE_PRIV_SET */
	/* Only free the first instance of runas user/group lists. */
	if (cs->runasuserlist && cs->runasuserlist != runasuserlist) {
	    runasuserlist = cs->runasuserlist;
	    free_members(runasuserlist);
	    free(runasuserlist);
	}
	if (cs->runasgrouplist && cs->runasgrouplist != runasgrouplist) {
	    runasgrouplist = cs->runasgrouplist;
	    free_members(runasgrouplist);
	    free(runasgrouplist);
	}
	free_member(cs->cmnd);
	free(cs);
    }

    debug_return;
}

void
free_privilege(struct privilege *priv)
{
    struct defaults *def;
    debug_decl(free_privilege, SUDOERS_DEBUG_PARSER);

    free(priv->ldap_role);
    free_members(&priv->hostlist);
    free_cmndspecs(&priv->cmndlist);
    while ((def = TAILQ_FIRST(&priv->defaults)) != NULL) {
	TAILQ_REMOVE(&priv->defaults, def, entries);
	free_default(def);
    }
    free(priv);

    debug_return;
}

void
free_userspecs(struct userspec_list *usl)
{
    struct userspec *us;
    debug_decl(free_userspecs, SUDOERS_DEBUG_PARSER);

    while ((us = TAILQ_FIRST(usl)) != NULL) {
	TAILQ_REMOVE(usl, us, entries);
	free_userspec(us);
    }

    debug_return;
}

void
free_userspec(struct userspec *us)
{
    struct privilege *priv;
    struct sudoers_comment *comment;
    debug_decl(free_userspec, SUDOERS_DEBUG_PARSER);

    free_members(&us->users);
    while ((priv = TAILQ_FIRST(&us->privileges)) != NULL) {
	TAILQ_REMOVE(&us->privileges, priv, entries);
	free_privilege(priv);
    }
    while ((comment = STAILQ_FIRST(&us->comments)) != NULL) {
	STAILQ_REMOVE_HEAD(&us->comments, entries);
	free(comment->str);
	free(comment);
    }
    sudo_rcstr_delref(us->file);
    free(us);

    debug_return;
}

/*
 * Initialized a sudoers parse tree.
 * Takes ownership of lhost and shost.
 */
void
init_parse_tree(struct sudoers_parse_tree *parse_tree, char *lhost, char *shost,
    struct sudoers_context *ctx, struct sudo_nss *nss)
{
    TAILQ_INIT(&parse_tree->userspecs);
    TAILQ_INIT(&parse_tree->defaults);
    parse_tree->aliases = NULL;
    parse_tree->shost = shost;
    parse_tree->lhost = lhost;
    parse_tree->ctx = ctx;
    parse_tree->nss = nss;
}

/*
 * Move the contents of parsed_policy to new_tree.
 */
void
reparent_parse_tree(struct sudoers_parse_tree *new_tree)
{
    TAILQ_CONCAT(&new_tree->userspecs, &parsed_policy.userspecs, entries);
    TAILQ_CONCAT(&new_tree->defaults, &parsed_policy.defaults, entries);
    new_tree->aliases = parsed_policy.aliases;
    parsed_policy.aliases = NULL;
}

/*
 * Free the contents of a sudoers parse tree and initialize it.
 */
void
free_parse_tree(struct sudoers_parse_tree *parse_tree)
{
    free_userspecs(&parse_tree->userspecs);
    free_defaults(&parse_tree->defaults);
    free_aliases(parse_tree->aliases);
    parse_tree->aliases = NULL;
    free(parse_tree->lhost);
    if (parse_tree->shost != parse_tree->lhost)
	free(parse_tree->shost);
    parse_tree->lhost = parse_tree->shost = NULL;
    parse_tree->nss = NULL;
    parse_tree->ctx = NULL;
}

/*
 * Free up space used by data structures from a previous parser run and sets
 * the current sudoers file to path.
 */
bool
init_parser(struct sudoers_context *ctx, const char *file)
{
    bool ret = true;
    debug_decl(init_parser, SUDOERS_DEBUG_PARSER);

    free_parse_tree(&parsed_policy);
    parsed_policy.ctx = ctx;
    parser_leak_init();
    init_lexer();
    parse_error = false;

    if (ctx != NULL) {
	parser_conf = ctx->parser_conf;
    } else {
	const struct sudoers_parser_config def_conf =
	    SUDOERS_PARSER_CONFIG_INITIALIZER;
	parser_conf = def_conf;
    }

    sudo_rcstr_delref(sudoers);
    if (file != NULL) {
	if ((sudoers = sudo_rcstr_dup(file)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    ret = false;
	}
    } else {
	sudoers = NULL;
    }

    sudo_rcstr_delref(sudoers_search_path);
    if (parser_conf.sudoers_path != NULL) {
	sudoers_search_path = sudo_rcstr_dup(parser_conf.sudoers_path);
	if (sudoers_search_path == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    ret = false;
	}
    } else {
	sudoers_search_path = NULL;
    }

    debug_return_bool(ret);
}

bool
reset_parser(void)
{
    return init_parser(NULL, NULL);
}

/*
 * Initialize all options in a cmndspec.
 */
static void
init_options(struct command_options *opts)
{
    opts->notbefore = UNSPEC;
    opts->notafter = UNSPEC;
    opts->timeout = UNSPEC;
    opts->runchroot = NULL;
    opts->runcwd = NULL;
#ifdef HAVE_SELINUX
    opts->role = NULL;
    opts->type = NULL;
#endif
#ifdef HAVE_PRIV_SET
    opts->privs = NULL;
    opts->limitprivs = NULL;
#endif
#ifdef HAVE_APPARMOR
    opts->apparmor_profile = NULL;
#endif
}

uid_t
sudoers_file_uid(void)
{
    return parser_conf.sudoers_uid;
}

gid_t
sudoers_file_gid(void)
{
    return parser_conf.sudoers_gid;
}

mode_t
sudoers_file_mode(void)
{
    return parser_conf.sudoers_mode;
}

bool
sudoers_error_recovery(void)
{
    return parser_conf.recovery;
}

bool
sudoers_strict(void)
{
    return parser_conf.strict;
}

bool
parser_leak_add(enum parser_leak_types type, void *v)
{
#ifdef NO_LEAKS
    struct parser_leak_entry *entry;
    debug_decl(parser_leak_add, SUDOERS_DEBUG_PARSER);

    if (v == NULL)
	debug_return_bool(false);

    entry = calloc(1, sizeof(*entry));
    if (entry == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    switch (type) {
    case LEAK_PRIVILEGE:
	entry->u.p = v;
	break;
    case LEAK_CMNDSPEC:
	entry->u.cs = v;
	break;
    case LEAK_DEFAULTS:
	entry->u.d = v;
	break;
    case LEAK_MEMBER:
	entry->u.m = v;
	break;
    case LEAK_DIGEST:
	entry->u.dig = v;
	break;
    case LEAK_RUNAS:
	entry->u.rc = v;
	break;
    case LEAK_PTR:
	entry->u.ptr = v;
	break;
    default:
	free(entry);
	sudo_warnx("unexpected leak type %d", type);
	debug_return_bool(false);
    }
    entry->type = type;
    SLIST_INSERT_HEAD(&parser_leak_list, entry, entries);
    debug_return_bool(true);
#else
    return true;
#endif /* NO_LEAKS */
}

bool
parser_leak_remove(enum parser_leak_types type, void *v)
{
#ifdef NO_LEAKS
    struct parser_leak_entry *entry, *prev = NULL;
    debug_decl(parser_leak_remove, SUDOERS_DEBUG_PARSER);

    if (v == NULL)
	debug_return_bool(false);

    SLIST_FOREACH(entry, &parser_leak_list, entries) {
	switch (entry->type) {
	case LEAK_PRIVILEGE:
	    if (entry->u.p == v)
	    	goto found;
	    break;
	case LEAK_CMNDSPEC:
	    if (entry->u.cs == v)
	    	goto found;
	    break;
	case LEAK_DEFAULTS:
	    if (entry->u.d == v)
	    	goto found;
	    break;
	case LEAK_MEMBER:
	    if (entry->u.m == v)
	    	goto found;
	    break;
	case LEAK_DIGEST:
	    if (entry->u.dig == v)
	    	goto found;
	    break;
	case LEAK_RUNAS:
	    if (entry->u.rc == v)
	    	goto found;
	    break;
	case LEAK_PTR:
	    if (entry->u.ptr == v)
	    	goto found;
	    break;
	default:
	    sudo_warnx("unexpected leak type %d in %p", entry->type, entry);
	}
	prev = entry;
    }
    /* If this happens, there is a bug in the leak tracking code. */
    sudo_warnx("%s: unable to find %p, type %d", __func__, v, type);
    debug_return_bool(false);
found:
    if (prev == NULL)
	SLIST_REMOVE_HEAD(&parser_leak_list, entries);
    else
	SLIST_REMOVE_AFTER(prev, entries);
    free(entry);
    debug_return_bool(true);
#else
    return true;
#endif /* NO_LEAKS */
}

#ifdef NO_LEAKS
static void
parser_leak_free(void)
{
    struct parser_leak_entry *entry;
    void *next;
    debug_decl(parser_leak_run, SUDOERS_DEBUG_PARSER);

    /* Free the leaks. */
    while ((entry = SLIST_FIRST(&parser_leak_list))) {
	SLIST_REMOVE_HEAD(&parser_leak_list, entries);
	switch (entry->type) {
	case LEAK_PRIVILEGE:
	    {
		struct privilege *priv;

		HLTQ_FOREACH_SAFE(priv, entry->u.p, entries, next)
		    free_privilege(priv);
		free(entry);
	    }
	    break;
	case LEAK_CMNDSPEC:
	    {
		struct cmndspec_list specs;

		HLTQ_TO_TAILQ(&specs, entry->u.cs, entries);
		free_cmndspecs(&specs);
		free(entry);
	    }
	    break;
	case LEAK_DEFAULTS:
	    {
		struct defaults_list defs;

		HLTQ_TO_TAILQ(&defs, entry->u.d, entries);
		free_defaults(&defs);
		free(entry);
	    }
	    break;
	case LEAK_MEMBER:
	    {
		struct member *m;

		HLTQ_FOREACH_SAFE(m, entry->u.m, entries, next)
		    free_member(m);
		free(entry);
	    }
	    break;
	case LEAK_DIGEST:
	    {
		struct command_digest *dig;

		HLTQ_FOREACH_SAFE(dig, entry->u.dig, entries, next) {
		    free(dig->digest_str);
		    free(dig);
		}
		free(entry);
	    }
	    break;
	case LEAK_RUNAS:
	    {
		struct member *m;

		if (entry->u.rc->runasusers != NULL) {
		    HLTQ_FOREACH_SAFE(m, entry->u.rc->runasusers, entries, next)
			free_member(m);
		}
		if (entry->u.rc->runasgroups != NULL) {
		    HLTQ_FOREACH_SAFE(m, entry->u.rc->runasgroups, entries, next)
			free_member(m);
		}
		free(entry->u.rc);
		free(entry);
		break;
	    }
	case LEAK_PTR:
	    free(entry->u.ptr);
	    free(entry);
	    break;
	default:
	    sudo_warnx("unexpected garbage type %d", entry->type);
	}
    }

    debug_return;
}
#endif /* NO_LEAKS */

void
parser_leak_init(void)
{
#ifdef NO_LEAKS
    static bool initialized;
    debug_decl(parser_leak_init, SUDOERS_DEBUG_PARSER);

    if (!initialized) {
	atexit(parser_leak_free);
	initialized = true;
	debug_return;
    }

    /* Already initialized, free existing leaks. */
    parser_leak_free();
    debug_return;
#endif /* NO_LEAKS */
}
