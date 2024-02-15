%{
/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2023
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <sudoers.h>
#include <toke.h>
#include <gram.h>
#include <sudo_digest.h>
#include <sudo_lbuf.h>

#if defined(HAVE_STRUCT_DIRENT_D_NAMLEN) && HAVE_STRUCT_DIRENT_D_NAMLEN
# define NAMLEN(dirent)	(dirent)->d_namlen
#else
# define NAMLEN(dirent)	strlen((dirent)->d_name)
#endif

// PVS Studio suppression
// -V::519, 547, 1004, 1037, 1048

int sudolineno;			/* current sudoers line number. */
char *sudoers;			/* sudoers file being parsed. */
char *sudoers_search_path;	/* colon-separated path of sudoers files. */
const char *sudoers_errstr;	/* description of last error from lexer. */
struct sudolinebuf sudolinebuf;	/* sudoers line being parsed. */

static bool continued, sawspace;
static int prev_state;
static unsigned int digest_type = SUDO_DIGEST_INVALID;

static bool pop_include(void);
static int sudoers_input(char *buf, yy_size_t max_size);

#ifndef TRACELEXER
static struct sudo_lbuf trace_lbuf;
#endif

int (*trace_print)(const char *msg) = sudoers_trace_print;

#define ECHO	ignore_result(fwrite(sudoerstext, (size_t)sudoersleng, 1, sudoersout))

#define YY_INPUT(buf, result, max_size)	(result) = sudoers_input(buf, (yy_size_t)(max_size))

#define YY_USER_ACTION do {					\
	sudolinebuf.toke_start = sudolinebuf.toke_end;		\
	sudolinebuf.toke_end += (size_t)sudoersleng;		\
} while (0);

#define sudoersless(n) do {					\
	sudolinebuf.toke_end = sudolinebuf.toke_start + (size_t)(n);	\
	yyless((int)n);						\
} while (0);

%}

HEX16			[0-9A-Fa-f]{1,4}
OCTET			(1?[0-9]{1,2})|(2[0-4][0-9])|(25[0-5])
IPV4ADDR		{OCTET}(\.{OCTET}){3}
IPV6ADDR		({HEX16}?:){2,7}{HEX16}?|({HEX16}?:){2,6}:{IPV4ADDR}

HOSTNAME		[[:alnum:]_-]+
WORD			([^#>!=:,\(\) \t\r\n\\\"]|\\[^\t\n])+
ID			#-?[0-9]+
PATH			\/(\\[\,:= \t#]|[^\,:=\\ \t\r\n#])+
REGEX			\^([^#\r\n\$]|\\[#\$])*\$
ENVAR			([^#!=, \t\r\n\\\"]|\\[^\r\n])([^#=, \t\r\n\\\"]|\\[^\r\n])*
DEFVAR			[a-z_]+

%option noinput
%option nounput
%option noyywrap
%option prefix="sudoers"

%s	GOTDEFS
%x	GOTCMND
%x	GOTREGEX
%x	STARTDEFS
%x	INDEFS
%x	INSTR
%s	WANTDIGEST
%x	GOTINC
%s	EXPECTPATH

%%
<GOTDEFS>[[:blank:]]*,[[:blank:]]* {
			    LEXTRACE(", ");
			    return ',';
			}			/* return ',' */

<GOTDEFS>[[:blank:]]+	BEGIN STARTDEFS;

<STARTDEFS>{DEFVAR}	{
			    BEGIN INDEFS;
			    LEXTRACE("DEFVAR ");
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    return DEFVAR;
			}

<INDEFS>{
    ,			{
			    BEGIN STARTDEFS;
			    LEXTRACE(", ");
			    return ',';
			}			/* return ',' */

    =			{
			    LEXTRACE("= ");
			    return '=';
			}			/* return '=' */

    \+=			{
			    LEXTRACE("+= ");
			    return '+';
			}			/* return '+' */

    -=			{
			    LEXTRACE("-= ");
			    return '-';
			}			/* return '-' */

    \"			{
			    LEXTRACE("BEGINSTR ");
			    sudoerslval.string = NULL;
			    prev_state = YY_START;
			    BEGIN INSTR;
			}

    {ENVAR}		{
			    LEXTRACE("WORD(2) ");
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    return WORD;
			}
}

<INSTR>{
    \\[[:blank:]]*\r?\n[[:blank:]]*	{
			    /* Line continuation char followed by newline. */
			    sudolineno++;
			    continued = true;
			}

    \"			{
			    LEXTRACE("ENDSTR ");
			    BEGIN prev_state;

			    if (sudoerslval.string == NULL) {
				sudoers_errstr = N_("empty string");
				LEXTRACE("ERROR ");
				return ERROR;
			    }
			    if (prev_state == INITIAL || prev_state == GOTDEFS) {
				switch (sudoerslval.string[0]) {
				case '%':
				    if (sudoerslval.string[1] == '\0' ||
					(sudoerslval.string[1] == ':' &&
					sudoerslval.string[2] == '\0')) {
					parser_leak_remove(LEAK_PTR, sudoerslval.string);
					free(sudoerslval.string);
					sudoers_errstr = N_("empty group");
					LEXTRACE("ERROR ");
					return ERROR;
				    }
				    LEXTRACE("USERGROUP ");
				    return USERGROUP;
				case '+':
				    if (sudoerslval.string[1] == '\0') {
					parser_leak_remove(LEAK_PTR, sudoerslval.string);
					free(sudoerslval.string);
					sudoers_errstr = N_("empty netgroup");
					LEXTRACE("ERROR ");
					return ERROR;
				    }
				    LEXTRACE("NETGROUP ");
				    return NETGROUP;
				}
			    }
			    LEXTRACE("WORD(4) ");
			    return WORD;
			}

    \\			{
			    LEXTRACE("BACKSLASH ");
			    if (!append(sudoerstext, sudoersleng))
				yyterminate();
			}

    ([^\"\r\n\\]|\\\")+	{
			    LEXTRACE("STRBODY ");
			    if (!append(sudoerstext, sudoersleng))
				yyterminate();
			}
}

<GOTCMND>{
    \\[\*\?\[\]\!^]	{
			    /* quoted fnmatch glob char, pass verbatim */
			    LEXTRACE("QUOTEDCHAR ");
			    if (!fill_args(sudoerstext, 2, sawspace))
				yyterminate();
			    sawspace = false;
			}

    \\[:\\,= \t#]	{
			    /* quoted sudoers special char, strip backslash */
			    LEXTRACE("QUOTEDCHAR ");
			    if (!fill_args(sudoerstext + 1, 1, sawspace))
				yyterminate();
			    sawspace = false;
			}

    [#:\,=\r\n]		{
			    BEGIN INITIAL;
			    sudoersless(0);
			    yy_set_bol(0);
			    return COMMAND;
			}			/* end of command line args */

    [^#\\:, \t\r\n]+ 	{
			    if (sudoerslval.command.args == NULL && sudoerstext[0] == '^') {
				LEXTRACE("ARG REGEX ");
				BEGIN GOTREGEX;
				sudoersless(0);
				yy_set_bol(0);
			    } else {
				LEXTRACE("ARG ");
				if (!fill_args(sudoerstext, sudoersleng, sawspace))
				    yyterminate();
				sawspace = false;
			    }
			}			/* a command line arg */
}

<GOTREGEX>{
    \\[^\r\n]		{
			    /* quoted character, pass verbatim */
			    LEXTRACE("QUOTEDCHAR ");
			    if (!fill_args(sudoerstext, 2, false))
				yyterminate();
			}

    [#\r\n]		{
			    /* Let the parser attempt to recover. */
			    sudoersless(0);
			    yy_set_bol(0);
			    BEGIN INITIAL;

			    sudoers_errstr = N_("unterminated regular expression");
			    LEXTRACE("ERROR ");
			    return ERROR;
			}			/* illegal inside regex */

    \$			{
			    if (!fill_args("$", 1, false))
				yyterminate();
			    BEGIN INITIAL;
			    continued = false;
			    if (sudoers_strict()) {
				if (!sudo_regex_compile(NULL, sudoerstext, &sudoers_errstr)) {
				    LEXTRACE("ERROR ");
				    return ERROR;
				}
			    }
			    return COMMAND;
			}

    [^#\\\r\n$]+ 	{
			    if (continued) {
				/* remove whitespace after line continuation */
				while (isblank((unsigned char)*sudoerstext)) {
				    sudoerstext++;
				    sudoersleng--;
				}
				continued = false;
			    }
			    if (sudoersleng != 0) {
				if (!fill_args(sudoerstext, sudoersleng, false))
				    yyterminate();
			    }
			}
}

<WANTDIGEST>[[:xdigit:]]+ {
			    /* Only return DIGEST if the length is correct. */
			    size_t digest_len =
				sudo_digest_getlen(digest_type);
			    if ((size_t)sudoersleng == digest_len * 2) {
				if (!fill(sudoerstext, sudoersleng))
				    yyterminate();
				BEGIN INITIAL;
				LEXTRACE("DIGEST ");
				return DIGEST;
			    }
			    BEGIN INITIAL;
			    sudoersless(sudoersleng);
			} /* hex digest */

<WANTDIGEST>[A-Za-z0-9\+/=]+ {
			    /* Only return DIGEST if the length is correct. */
			    size_t len, digest_len =
				sudo_digest_getlen(digest_type);
			    if (sudoerstext[sudoersleng - 1] == '=') {
				/* use padding */
				len = 4 * ((digest_len + 2) / 3);
			    } else {
				/* no padding */
				len = (4 * digest_len + 2) / 3;
			    }
			    if ((size_t)sudoersleng == len) {
				if (!fill(sudoerstext, sudoersleng))
				    yyterminate();
				BEGIN INITIAL;
				LEXTRACE("DIGEST ");
				return DIGEST;
			    }
			    BEGIN INITIAL;
			    sudoersless(sudoersleng);
			} /* base64 digest */

<INITIAL>@include	{
			    if (continued) {
				sudoers_errstr = N_("invalid line continuation");
				LEXTRACE("ERROR ");
				return ERROR;
			    }

			    BEGIN GOTINC;
			    LEXTRACE("INCLUDE ");
			    return INCLUDE;
			}

<INITIAL>@includedir	{
			    if (continued) {
				sudoers_errstr = N_("invalid line continuation");
				LEXTRACE("ERROR ");
				return ERROR;
			    }

			    BEGIN GOTINC;
			    LEXTRACE("INCLUDEDIR ");
			    return INCLUDEDIR;
			}

<INITIAL>^#include[[:blank:]]+.*(\r\n|\n)? {
			    if (continued) {
				sudoers_errstr = N_("invalid line continuation");
				LEXTRACE("ERROR ");
				return ERROR;
			    }

			    /* only consume #include */
			    sudoersless(sizeof("#include") - 1);
			    yy_set_bol(0);

			    BEGIN GOTINC;
			    LEXTRACE("INCLUDE ");
			    return INCLUDE;
			}

<INITIAL>^#includedir[[:blank:]]+.*(\r\n|\n)? {
			    if (continued) {
				sudoers_errstr = N_("invalid line continuation");
				LEXTRACE("ERROR ");
				return ERROR;
			    }

			    /* only consume #includedir */
			    sudoersless(sizeof("#includedir") - 1);
			    yy_set_bol(0);

			    BEGIN GOTINC;
			    LEXTRACE("INCLUDEDIR ");
			    return INCLUDEDIR;
			}

<INITIAL>^[[:blank:]]*Defaults([:@>\!][[:blank:]]*\!*\"?({ID}|{WORD}))? {
			    char deftype;
			    size_t n;

			    if (continued) {
				sudoers_errstr = N_("invalid line continuation");
				LEXTRACE("ERROR ");
				return ERROR;
			    }

			    for (n = 0; isblank((unsigned char)sudoerstext[n]); n++)
				continue;
			    n += sizeof("Defaults") - 1;
			    if ((deftype = sudoerstext[n++]) != '\0') {
				while (isblank((unsigned char)sudoerstext[n]))
				    n++;
			    }
			    BEGIN GOTDEFS;
			    switch (deftype) {
				case ':':
				    sudoersless(n);
				    LEXTRACE("DEFAULTS_USER ");
				    return DEFAULTS_USER;
				case '>':
				    sudoersless(n);
				    LEXTRACE("DEFAULTS_RUNAS ");
				    return DEFAULTS_RUNAS;
				case '@':
				    sudoersless(n);
				    LEXTRACE("DEFAULTS_HOST ");
				    return DEFAULTS_HOST;
				case '!':
				    sudoersless(n);
				    LEXTRACE("DEFAULTS_CMND ");
				    return DEFAULTS_CMND;
				default:
				    LEXTRACE("DEFAULTS ");
				    return DEFAULTS;
			    }
			}

<INITIAL>^[[:blank:]]*(Host|Cmnd|Cmd|User|Runas)_Alias	{
			    size_t n;

			    if (continued) {
				sudoers_errstr = N_("invalid line continuation");
				LEXTRACE("ERROR ");
				return ERROR;
			    }

			    for (n = 0; isblank((unsigned char)sudoerstext[n]); n++)
				continue;
			    switch (sudoerstext[n]) {
				case 'H':
				    LEXTRACE("HOSTALIAS ");
				    return HOSTALIAS;
				case 'C':
				    LEXTRACE("CMNDALIAS ");
				    return CMNDALIAS;
				case 'U':
				    LEXTRACE("USERALIAS ");
				    return USERALIAS;
				case 'R':
				    LEXTRACE("RUNASALIAS ");
				    return RUNASALIAS;
			    }
			}

NOPASSWD[[:blank:]]*:	{
				/* cmnd does not require passwd for this user */
			    	LEXTRACE("NOPASSWD ");
			    	return NOPASSWD;
			}

PASSWD[[:blank:]]*:	{
				/* cmnd requires passwd for this user */
			    	LEXTRACE("PASSWD ");
			    	return PASSWD;
			}

NOEXEC[[:blank:]]*:	{
			    	LEXTRACE("NOEXEC ");
			    	return NOEXEC;
			}

EXEC[[:blank:]]*:	{
			    	LEXTRACE("EXEC ");
			    	return EXEC;
			}

INTERCEPT[[:blank:]]*:	{
			    	LEXTRACE("INTERCEPT ");
			    	return INTERCEPT;
			}

NOINTERCEPT[[:blank:]]*: {
			    	LEXTRACE("NOINTERCEPT ");
			    	return NOINTERCEPT;
			}

SETENV[[:blank:]]*:	{
			    	LEXTRACE("SETENV ");
			    	return SETENV;
			}

NOSETENV[[:blank:]]*:	{
			    	LEXTRACE("NOSETENV ");
			    	return NOSETENV;
			}

LOG_OUTPUT[[:blank:]]*:	{
			    	LEXTRACE("LOG_OUTPUT ");
			    	return LOG_OUTPUT;
			}

NOLOG_OUTPUT[[:blank:]]*:	{
			    	LEXTRACE("NOLOG_OUTPUT ");
			    	return NOLOG_OUTPUT;
			}

LOG_INPUT[[:blank:]]*:	{
			    	LEXTRACE("LOG_INPUT ");
			    	return LOG_INPUT;
			}

NOLOG_INPUT[[:blank:]]*:	{
			    	LEXTRACE("NOLOG_INPUT ");
			    	return NOLOG_INPUT;
			}

MAIL[[:blank:]]*:	{
			    	LEXTRACE("MAIL ");
			    	return MAIL;
			}

NOMAIL[[:blank:]]*:	{
			    	LEXTRACE("NOMAIL ");
			    	return NOMAIL;
			}

FOLLOW[[:blank:]]*:	{
			    	LEXTRACE("FOLLOW ");
			    	return FOLLOWLNK;
			}

NOFOLLOW[[:blank:]]*:	{
			    	LEXTRACE("NOFOLLOW ");
			    	return NOFOLLOWLNK;
			}

<INITIAL,GOTDEFS>(\+|\%|\%:) {
			    if (sudoerstext[0] == '+')
				sudoers_errstr = N_("empty netgroup");
			    else
				sudoers_errstr = N_("empty group");
			    LEXTRACE("ERROR ");
			    return ERROR;
			}

\+{WORD}		{
			    /* netgroup */
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("NETGROUP ");
			    return NETGROUP;
			}

\%:?({WORD}|{ID})	{
			    /* group */
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("USERGROUP ");
			    return USERGROUP;
			}

{IPV4ADDR}(\/{IPV4ADDR})? {
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("NTWKADDR ");
			    return NTWKADDR;
			}

{IPV4ADDR}\/([12]?[0-9]|3[0-2]) {
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("NTWKADDR ");
			    return NTWKADDR;
			}

{IPV6ADDR}(\/{IPV6ADDR})? {
			    if (!ipv6_valid(sudoerstext)) {
				sudoers_errstr = N_("invalid IPv6 address");
				LEXTRACE("ERROR ");
				return ERROR;
			    }
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("NTWKADDR ");
			    return NTWKADDR;
			}

{IPV6ADDR}\/([0-9]|[1-9][0-9]|1[01][0-9]|12[0-8]) {
			    if (!ipv6_valid(sudoerstext)) {
				sudoers_errstr = N_("invalid IPv6 address");
				LEXTRACE("ERROR ");
				return ERROR;
			    }
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("NTWKADDR ");
			    return NTWKADDR;
			}

ALL {
			    LEXTRACE("ALL ");
			    return ALL;

			}

<INITIAL>TIMEOUT {
			    LEXTRACE("CMND_TIMEOUT ");
			    return CMND_TIMEOUT;
			}

<INITIAL>NOTBEFORE {
			    LEXTRACE("NOTBEFORE ");
			    return NOTBEFORE;
			}

<INITIAL>NOTAFTER {
			    LEXTRACE("NOTAFTER ");
			    return NOTAFTER;
			}

<INITIAL>CWD {
			    LEXTRACE("CWD ");
			    prev_state = YY_START;
			    BEGIN EXPECTPATH;
			    return CWD;
			}

<INITIAL>CHROOT {
			    LEXTRACE("CHROOT ");
			    prev_state = YY_START;
			    BEGIN EXPECTPATH;
			    return CHROOT;
			}

<INITIAL>ROLE {
#ifdef HAVE_SELINUX
			    LEXTRACE("ROLE ");
			    return ROLE;
#else
			    goto got_alias;
#endif
			}

<INITIAL>TYPE {
#ifdef HAVE_SELINUX
			    LEXTRACE("TYPE ");
			    return TYPE;
#else
			    goto got_alias;
#endif
			}
<INITIAL>APPARMOR_PROFILE {
#ifdef HAVE_APPARMOR
			    LEXTRACE("APPARMOR_PROFILE ");
			    return APPARMOR_PROFILE;
#else
			    goto got_alias;
#endif
			}
<INITIAL>PRIVS {
#ifdef HAVE_PRIV_SET
			    LEXTRACE("PRIVS ");
			    return PRIVS;
#else
			    goto got_alias;
#endif
			}

<INITIAL>LIMITPRIVS {
#ifdef HAVE_PRIV_SET
			    LEXTRACE("LIMITPRIVS ");
			    return LIMITPRIVS;
#else
			    goto got_alias;
#endif
			}

[[:upper:]][[:upper:][:digit:]_]* {
			got_alias:
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("ALIAS ");
			    return ALIAS;
			}

<GOTDEFS>({PATH}|{REGEX}|sudoedit) {
			    /* XXX - no way to specify digest for command */
			    /* no command args allowed for Defaults!/path */
			    if (!fill_cmnd(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("COMMAND ");
			    return COMMAND;
			}

sha224			{
			    digest_type = SUDO_DIGEST_SHA224;
			    BEGIN WANTDIGEST;
			    LEXTRACE("SHA224_TOK ");
			    return SHA224_TOK;
			}

sha256			{
			    digest_type = SUDO_DIGEST_SHA256;
			    BEGIN WANTDIGEST;
			    LEXTRACE("SHA256_TOK ");
			    return SHA256_TOK;
			}

sha384			{
			    digest_type = SUDO_DIGEST_SHA384;
			    BEGIN WANTDIGEST;
			    LEXTRACE("SHA384_TOK ");
			    return SHA384_TOK;
			}

sha512			{
			    digest_type = SUDO_DIGEST_SHA512;
			    BEGIN WANTDIGEST;
			    LEXTRACE("SHA512_TOK ");
			    return SHA512_TOK;
			}

sudoedit		{
			    BEGIN GOTCMND;
			    LEXTRACE("COMMAND ");
			    if (!fill_cmnd(sudoerstext, sudoersleng))
				yyterminate();
			}			/* sudo -e */

<EXPECTPATH>({PATH}|{WORD}) {
				BEGIN prev_state;
				if (!fill(sudoerstext, sudoersleng))
				    yyterminate();
				LEXTRACE("WORD(5) ");
				return WORD;
			}

{PATH}			{
			    /* directories can't have args... */
			    if (sudoerstext[sudoersleng - 1] == '/') {
				LEXTRACE("COMMAND ");
				if (!fill_cmnd(sudoerstext, sudoersleng))
				    yyterminate();
				return COMMAND;
			    }
			    BEGIN GOTCMND;
			    LEXTRACE("COMMAND ");
			    if (!fill_cmnd(sudoerstext, sudoersleng))
				yyterminate();
			}			/* a pathname */

{REGEX}			{
			    if (sudoers_strict()) {
				if (!sudo_regex_compile(NULL, sudoerstext, &sudoers_errstr)) {
				    LEXTRACE("ERROR ");
				    return ERROR;
				}
			    }
			    BEGIN GOTCMND;
			    LEXTRACE("COMMAND ");
			    if (!fill_cmnd(sudoerstext, sudoersleng))
				yyterminate();
			}			/* a regex */

<INITIAL,GOTDEFS>\"	{
			    LEXTRACE("BEGINSTR ");
			    sudoerslval.string = NULL;
			    prev_state = YY_START;
			    BEGIN INSTR;
			}

<INITIAL,GOTDEFS>({ID}|{WORD}) {
			    /* a word */
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    LEXTRACE("WORD(6) ");
			    return WORD;
			}

<GOTINC>{
    [^\"[:space:]]([^[:space:]]|\\[[:blank:]])*	{
			    /* include file/directory */
			    if (!fill(sudoerstext, sudoersleng))
				yyterminate();
			    BEGIN INITIAL;
			    LEXTRACE("WORD(7) ");
			    return WORD;
			}

    \"			{
			    LEXTRACE("BEGINSTR ");
			    sudoerslval.string = NULL;
			    prev_state = INITIAL;
			    BEGIN INSTR;
			}
}

\(			{
			    LEXTRACE("( ");
			    return '(';
			}

\)			{
			    LEXTRACE(") ");
			    return ')';
			}

,			{
			    LEXTRACE(", ");
			    return ',';
			}			/* return ',' */

=			{
			    LEXTRACE("= ");
			    return '=';
			}			/* return '=' */

:			{
			    LEXTRACE(": ");
			    return ':';
			}			/* return ':' */

<*>!+			{
			    if (sudoersleng & 1) {
				LEXTRACE("!");
				return '!';	/* return '!' */
			    }
			}

<*>\r?\n		{
			    if (YY_START == INSTR) {
				/* throw away old string */
				parser_leak_remove(LEAK_PTR, sudoerslval.string);
				free(sudoerslval.string);
				/* re-scan after changing state */
				BEGIN INITIAL;
				sudoersless(0);
				sudoers_errstr = N_("unexpected line break in string");
				LEXTRACE("ERROR ");
				return ERROR;
			    }
			    BEGIN INITIAL;
			    sudolineno++;
			    continued = false;
			    LEXTRACE("\n");
			    return '\n';
			}			/* return newline */

<*>[[:blank:]]+		{			/* throw away space/tabs */
			    sawspace = true;	/* but remember for fill_args */
			}

<*>\\[[:blank:]]*\r?\n	{
			    sawspace = true;	/* remember for fill_args */
			    sudolineno++;
			    continued = true;
			}			/* throw away EOL after \ */

<INITIAL,STARTDEFS,INDEFS>#(-[^\r\n0-9].*|[^\r\n0-9-].*)?(\r\n|\n)?	{
			    if (sudoerstext[sudoersleng - 1] == '\n') {
				/* comment ending in a newline */
				BEGIN INITIAL;
				sudolineno++;
				continued = false;
			    } else if (!feof(sudoersin)) {
				sudoers_errstr = strerror(errno);
				LEXTRACE("ERROR ");
				return ERROR;
			    }
			    LEXTRACE("#\n");
			    return '\n';
			}			/* comment, not uid/gid */

<*>.			{
			    LEXTRACE("NOMATCH ");
			    return NOMATCH;
			}	/* parse error, no matching token */

<*><<EOF>>		{
			    if (!pop_include())
				yyterminate();
			}

%%
struct path_list {
    SLIST_ENTRY(path_list) entries;
    char *path;
};

SLIST_HEAD(path_list_head, path_list);

struct include_stack {
    struct sudolinebuf line;
    YY_BUFFER_STATE bs;
    char *path; /* search path */
    char *file;
    struct path_list_head more; /* more files in case of includedir */
    int lineno;
    bool keepopen;
};

/*
 * Compare two struct path_list structs in reverse order.
 */
static int
pl_compare(const void *v1, const void *v2)
{
    const struct path_list * const *p1 = v1;
    const struct path_list * const *p2 = v2;

    return strcmp((*p2)->path, (*p1)->path);
}

/*
 * Open dirpath and fill in pathsp with an array of regular files
 * that do not end in '~' or contain a '.'.
 * Returns the number of files or SIZE_MAX (-1) on error.
 * If zero files are found, NULL is stored in pathsp.
 */
static size_t
read_dir_files(const char *dirpath, struct path_list ***pathsp, int verbose)
{
    DIR *dir;
    size_t i, count = 0;
    size_t max_paths = 32;
    struct dirent *dent;
    struct path_list **paths = NULL;
    const size_t dirlen = strlen(dirpath);
    debug_decl(read_dir_files, SUDOERS_DEBUG_PARSER);

    /* XXX - fdopendir */
    dir = opendir(dirpath);
    if (dir == NULL) {
	if (errno == ENOENT)
	    goto done;
	sudo_warn("%s", dirpath);
	goto bad;
    }
    paths = reallocarray(NULL, max_paths, sizeof(*paths));
    if (paths == NULL)
	goto oom;
    while ((dent = readdir(dir)) != NULL) {
	const size_t namelen = NAMLEN(dent);
	const char *name = dent->d_name;
	struct path_list *pl;
	struct stat sb;
	size_t len;
	char *path;

	/* Ignore files that end in '~' or have a '.' in them. */
	if (namelen == 0 || name[namelen - 1] == '~' || strchr(name, '.') != NULL) {
	    /* Warn about ignored files not starting with '.' if verbose. */
	    if (namelen > 0 && name[0] != '.' && verbose > 1) {
		if (name[namelen - 1] == '~' ||
			(namelen > 4 && strcmp(&name[namelen - 4], ".bak") == 0)) {
		    fprintf(stderr, U_("%s/%s: %s"), dirpath, name,
			U_("ignoring editor backup file"));
		} else {
		    fprintf(stderr, U_("%s/%s: %s"), dirpath, name,
			U_("ignoring file name containing '.'"));
		}
		fputc('\n', stderr);
	    }
	    continue;
	}
	len = dirlen + 1 + namelen;
	if ((path = sudo_rcstr_alloc(len)) == NULL)
	    goto oom;
	if ((size_t)snprintf(path, len + 1, "%s/%s", dirpath, name) != len) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    sudo_rcstr_delref(path);
	    goto bad;
	}
	if (stat(path, &sb) != 0 || !S_ISREG(sb.st_mode)) {
	    sudo_rcstr_delref(path);
	    continue;
	}
	pl = malloc(sizeof(*pl));
	if (pl == NULL) {
	    sudo_rcstr_delref(path);
	    goto oom;
	}
	pl->path = path;
	if (count >= max_paths) {
	    struct path_list **tmp;
	    max_paths <<= 1;
	    tmp = reallocarray(paths, max_paths, sizeof(*paths));
	    if (tmp == NULL) {
		sudo_rcstr_delref(path);
		free(pl);
		goto oom;
	    }
	    paths = tmp;
	}
	paths[count++] = pl;
    }
    closedir(dir);
    if (count == 0) {
	free(paths);
	paths = NULL;
    }
done:
    *pathsp = paths;
    debug_return_size_t(count);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
bad:
    sudoerserror(NULL);
    if (dir != NULL)
	closedir(dir);
    for (i = 0; i < count; i++) {
	sudo_rcstr_delref(paths[i]->path);
	free(paths[i]);
    }
    free(paths);
    debug_return_size_t(SIZE_MAX);
}

/*
 * Push a list of all files in dirpath onto stack.
 * Returns the number of files or -1 on error.
 */
static size_t
switch_dir(struct include_stack *stack, char *dirpath, int verbose)
{
    struct path_list **paths = NULL;
    size_t count, i;
    debug_decl(switch_dir, SUDOERS_DEBUG_PARSER);

    count = read_dir_files(dirpath, &paths, verbose);
    if (count > 0) {
	/* Sort the list as an array in reverse order. */
	qsort(paths, count, sizeof(*paths), pl_compare);

	/* Build up the list in sorted order. */
	for (i = 0; i < count; i++) {
	    SLIST_INSERT_HEAD(&stack->more, paths[i], entries);
	}
	free(paths);
    }

    debug_return_size_t(count);
}

#define MAX_SUDOERS_DEPTH	128
#define SUDOERS_STACK_INCREMENT	16

static size_t istacksize, idepth;
static struct include_stack *istack;
static bool keepopen;

void
init_lexer(void)
{
    struct path_list *pl;
    debug_decl(init_lexer, SUDOERS_DEBUG_PARSER);

#ifndef TRACELEXER
    free(trace_lbuf.buf);
    sudo_lbuf_init(&trace_lbuf, NULL, 0, NULL, 0);
#endif

    while (idepth) {
	idepth--;
	while ((pl = SLIST_FIRST(&istack[idepth].more)) != NULL) {
	    SLIST_REMOVE_HEAD(&istack[idepth].more, entries);
	    sudo_rcstr_delref(pl->path);
	    free(pl);
	}
	sudo_rcstr_delref(istack[idepth].path);
	if (idepth && !istack[idepth].keepopen)
	    fclose(istack[idepth].bs->yy_input_file);
	sudoers_delete_buffer(istack[idepth].bs);
	free(istack[idepth].line.buf);
    }
    free(istack);
    istack = NULL;
    istacksize = idepth = 0;
    free(sudolinebuf.buf);
    memset(&sudolinebuf, 0, sizeof(sudolinebuf));
    sudolineno = 1;
    keepopen = false;
    sawspace = false;
    continued = false;
    digest_type = SUDO_DIGEST_INVALID;
    prev_state = INITIAL;
    BEGIN INITIAL;

    debug_return;
}

/*
 * Like strlcpy() but expand %h escapes.
 */
static size_t
strlcpy_expand_host(char * restrict dst, const char * restrict src,
    const char * restrict host, size_t size)
{
    size_t len = 0;
    char ch;
    debug_decl(strlcpy_expand_host, SUDOERS_DEBUG_PARSER);

    while ((ch = *src++) != '\0') {
	if (ch == '%' && *src == 'h') {
	    size_t n = strlcpy(dst, host, size);
	    len += n;
	    if (n >= size) {
		/* truncated */
		n = size ? size - 1 : 0;
	    }
	    dst += n;
	    size -= n;
	    src++;
	    continue;
	}
	if (size > 1) {
	    *dst++ = ch;
	    size--;
	    len++;
	}
    }
    if (size > 0)
	*dst = '\0';

    debug_return_size_t(len);
}

/*
 * Expand any embedded %h (host) escapes in the given path and makes
 * a relative path fully-qualified based on the current sudoers file.
 * Returns a reference-counted string on success or NULL on failure.
 */
static char *
expand_include(const char *src, const char *host)
{
    const char *path = sudoers_search_path ? sudoers_search_path : sudoers;
    const char *path_end = path + strlen(path);
    char *dst, *dst0 = NULL, *dynamic_host = NULL;
    const char *cp, *ep;
    size_t dst_size, src_len;
    size_t nhost = 0;
    debug_decl(expand_include, SUDOERS_DEBUG_PARSER);

    /* Strip double quotes if present. */
    src_len = strlen(src);
    if (src_len > 1 && src[0] == '"' && src[src_len - 1] == '"') {
	src++;
	src_len -= 2;
    }
    if (src_len == 0)
	debug_return_ptr(NULL);

    /* Check for %h escapes in src. */
    cp = src;
    ep = src + src_len;
    while (cp < ep) {
	if (cp[0] == '%' && cp[1] == 'h') {
	    nhost++;
	    cp += 2;
	    continue;
	}
	cp++;
    }

    /* Check for a path separator in the host name, replace with '_'. */
    if (nhost != 0 && strchr(host, '/') != NULL) {
	dynamic_host = malloc(strlen(host) + 1);
	if (dynamic_host == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
	for (dst = dynamic_host; *host != '\0'; host++) {
	    if (*host == '/') {
		*dst++ = '_';
		continue;
	    }
	    *dst++ = *host;
	}
	*dst = '\0';
	host = dynamic_host;
    }

    if (*src == '/') {
	/* Fully-qualified path, make a copy and expand %h escapes. */
	dst_size = src_len + (nhost * strlen(host)) - (nhost * 2) + 1;
	dst0 = sudo_rcstr_alloc(dst_size - 1);
	if (dst0 == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto bad;
	}
	if (strlcpy_expand_host(dst0, src, host, dst_size) >= dst_size)
	    goto oflow;
	goto done;
    }

    /*
     * Relative paths are located in the same dir as the sudoers file.
     * If the current sudoers file was opened via a colon-separated path,
     * use the same path when opening src.
     */
    dst_size = 0;
    for (cp = sudo_strsplit(path, path_end, ":", &ep); cp != NULL;
	    cp = sudo_strsplit(NULL, path_end, ":", &ep)) {
	char *dirend = memrchr(cp, '/', (size_t)(ep - cp));
	if (dirend != NULL) {
	    dst_size += (size_t)(dirend - cp) + 1;
	}
	/* Includes space for ':' separator and NUL terminator. */
	dst_size += src_len + (nhost * strlen(host)) - (nhost * 2) + 1;
    }

    /* Make a copy of the fully-qualified path and return it. */
    dst = dst0 = sudo_rcstr_alloc(dst_size - 1);
    if (dst0 == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    for (cp = sudo_strsplit(path, path_end, ":", &ep); cp != NULL;
	    cp = sudo_strsplit(NULL, path_end, ":", &ep)) {
	size_t len;
	char *dirend;

	if (cp != path) {
	    if (dst_size < 2)
		goto oflow;
	    *dst++ = ':';
	    dst_size--;
	}

	dirend = memrchr(cp, '/', (size_t)(ep - cp));
	if (dirend != NULL) {
	    len = (size_t)(dirend - cp) + 1;
	    if (len >= dst_size)
		goto oflow;
	    memcpy(dst, cp, len);
	    dst += len;
	    dst_size -= len;
	}

	len = strlcpy_expand_host(dst, src, host, dst_size);
	if (len >= dst_size)
	    goto oflow;
	dst += len;
	dst_size -= len;
    }
    *dst = '\0';

done:
    free(dynamic_host);
    debug_return_str(dst0);
oflow:
    sudo_warnx(U_("internal error, %s overflow"), __func__);
bad:
    sudoerserror(NULL);
    free(dynamic_host);
    free(dst0);
    debug_return_str(NULL);
}

/*
 * Open an include file (or file from a directory), push the old
 * sudoers file buffer and switch to the new one.
 * A missing or insecure include dir is simply ignored.
 * Returns false on error, else true.
 */
static bool
push_include_int(const char *opath, const char *host, bool isdir,
     struct sudoers_parser_config *conf)
{
    struct path_list *pl;
    char *file = NULL, *path;
    FILE *fp;
    debug_decl(push_include, SUDOERS_DEBUG_PARSER);

    if ((path = expand_include(opath, host)) == NULL)
	debug_return_bool(false);

    /* push current state onto stack */
    if (idepth >= istacksize) {
	struct include_stack *new_istack;

	if (idepth > MAX_SUDOERS_DEPTH) {
	    if (conf->verbose > 0) {
		fprintf(stderr, U_("%s: %s"), path,
		    U_("too many levels of includes"));
		fputc('\n', stderr);
	    }
	    sudoerserror(NULL);
	    sudo_rcstr_delref(path);
	    debug_return_bool(false);
	}
	istacksize += SUDOERS_STACK_INCREMENT;
	new_istack = reallocarray(istack, istacksize, sizeof(*istack));
	if (new_istack == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    sudoerserror(NULL);
	    sudo_rcstr_delref(path);
	    debug_return_bool(false);
	}
	istack = new_istack;
    }
    SLIST_INIT(&istack[idepth].more);
    if (isdir) {
	struct stat sb;
	char dname[PATH_MAX];
	int fd, status;
	size_t count;

	fd = sudo_open_conf_path(path, dname, sizeof(dname), NULL);
	if (conf->ignore_perms) {
	    /* Skip sudoers security checks when ignore_perms is set. */
	    if (fd == -1 || fstat(fd, &sb) == -1)
		status = SUDO_PATH_MISSING;
	    else
		status = SUDO_PATH_SECURE;
	} else {
	    status = sudo_secure_fd(fd, S_IFDIR, sudoers_file_uid(),
		sudoers_file_gid(), &sb);
	}
	if (fd != -1)
	    close(fd); /* XXX use in read_dir_files? */
	if (status != SUDO_PATH_SECURE) {
	    if (conf->verbose > 0) {
		switch (status) {
		case SUDO_PATH_BAD_TYPE:
		    errno = ENOTDIR;
		    sudo_warn("%s", path);
		    break;
		case SUDO_PATH_WRONG_OWNER:
		    sudo_warnx(U_("%s is owned by uid %u, should be %u"),   
			path, (unsigned int) sb.st_uid,
			(unsigned int) sudoers_file_uid());
		    break;
		case SUDO_PATH_WORLD_WRITABLE:
		    sudo_warnx(U_("%s is world writable"), path);
		    break;
		case SUDO_PATH_GROUP_WRITABLE:
		    sudo_warnx(U_("%s is owned by gid %u, should be %u"),
			path, (unsigned int) sb.st_gid,
			(unsigned int) sudoers_file_gid());
		    break;
		default:
		    break;
		}
	    }
	    /* A missing or insecure include dir is not a fatal error. */
	    sudo_rcstr_delref(path);
	    debug_return_bool(true);
	}
	count = switch_dir(&istack[idepth], dname, conf->verbose);
	switch (count) {
	case SIZE_MAX:
	case 0:
	    /* switch_dir() called sudoerserror() for us */
	    sudo_rcstr_delref(path);
	    debug_return_bool(count ? false : true);
	}

	/* Parse the first dir entry we can open, leave the rest for later. */
	do {
	    sudo_rcstr_delref(file);
	    sudo_rcstr_delref(path);
	    if ((pl = SLIST_FIRST(&istack[idepth].more)) == NULL) {
		/* Unable to open any files in include dir, not an error. */
		debug_return_bool(true);
	    }
	    SLIST_REMOVE_HEAD(&istack[idepth].more, entries);
	    path = pl->path;
	    free(pl);
	    /* The file and path and the same for sudoers.d files. */
	    file = path;
	    sudo_rcstr_addref(file);
	} while ((fp = open_sudoers(file, NULL, false, &keepopen)) == NULL);
    } else {
	if ((fp = open_sudoers(path, &file, true, &keepopen)) == NULL) {
	    /* The error was already printed by open_sudoers() */
	    sudoerserror(NULL);
	    sudo_rcstr_delref(path);
	    debug_return_bool(false);
	}
    }
    /*
     * Push the old (current) file and open the new one.
     * We use the existing refs of sudoers and sudoers_search_path.
     */
    istack[idepth].file = sudoers;
    istack[idepth].path = sudoers_search_path;
    istack[idepth].line = sudolinebuf;
    istack[idepth].bs = YY_CURRENT_BUFFER;
    istack[idepth].lineno = sudolineno;
    istack[idepth].keepopen = keepopen;
    idepth++;
    sudolineno = 1;
    sudoers = file;
    sudoers_search_path = path;
    sudoers_switch_to_buffer(sudoers_create_buffer(fp, YY_BUF_SIZE));
    memset(&sudolinebuf, 0, sizeof(sudolinebuf));

    debug_return_bool(true);
}

bool
push_include(const char *opath, const char *host,
     struct sudoers_parser_config *conf)
{
    return push_include_int(opath, host, false, conf);
}

bool
push_includedir(const char *opath, const char *host,
     struct sudoers_parser_config *conf)
{
    return push_include_int(opath, host, true, conf);
}

/*
 * Restore the previous sudoers file and buffer, or, in the case
 * of an includedir, switch to the next file in the dir.
 * Returns false if there is nothing to pop, else true.
 */
static bool
pop_include(void)
{
    struct path_list *pl;
    FILE *fp;
    debug_decl(pop_include, SUDOERS_DEBUG_PARSER);

    if (idepth == 0 || YY_CURRENT_BUFFER == NULL)
	debug_return_bool(false);

    if (!keepopen)
	fclose(YY_CURRENT_BUFFER->yy_input_file);
    sudoers_delete_buffer(YY_CURRENT_BUFFER);
    /* If we are in an include dir, move to the next file. */
    while ((pl = SLIST_FIRST(&istack[idepth - 1].more)) != NULL) {
	SLIST_REMOVE_HEAD(&istack[idepth - 1].more, entries);
	fp = open_sudoers(pl->path, NULL, false, &keepopen);
	if (fp != NULL) {
	    sudolinebuf.len = sudolinebuf.off = 0;
	    sudolinebuf.toke_start = sudolinebuf.toke_end = 0;
	    sudo_rcstr_delref(sudoers);
	    sudo_rcstr_delref(sudoers_search_path);
	    sudoers_search_path = pl->path;
	    sudoers = sudoers_search_path;
	    sudo_rcstr_addref(sudoers);
	    sudolineno = 1;
	    sudoers_switch_to_buffer(sudoers_create_buffer(fp, YY_BUF_SIZE));
	    free(pl);
	    break;
	}
	/* Unable to open path in include dir, go to next one. */
	sudo_rcstr_delref(pl->path);
	free(pl);
    }
    /* If no path list, just pop the last dir on the stack. */
    if (pl == NULL) {
	idepth--;
	sudoers_switch_to_buffer(istack[idepth].bs);
	free(sudolinebuf.buf);
	sudolinebuf = istack[idepth].line;
	sudo_rcstr_delref(sudoers);
	sudoers = istack[idepth].file;
	sudo_rcstr_delref(sudoers_search_path);
	sudoers_search_path = istack[idepth].path;
	sudolineno = istack[idepth].lineno;
	keepopen = istack[idepth].keepopen;
    }
    debug_return_bool(true);
}

#ifdef TRACELEXER
int
sudoers_trace_print(const char *msg)
{
    return fputs(msg, stderr);
}
#else
int
sudoers_trace_print(const char *msg)
{
    debug_decl_vars(sudoers_trace_print, SUDOERS_DEBUG_PARSER);

    if (sudo_debug_needed(SUDO_DEBUG_DEBUG)) {
	sudo_lbuf_append(&trace_lbuf, "%s", msg);
	if (strchr(msg, '\n') != NULL)
	{
	    /* We already parsed the newline so sudolineno is off by one. */
	    sudo_debug_printf2(NULL, NULL, 0,
		sudo_debug_subsys|SUDO_DEBUG_DEBUG, "sudoerslex: %s:%d: %s",
		sudoers, sudolineno - 1, trace_lbuf.buf);
	    trace_lbuf.len = 0;
	}
    }
    return 0;
}
#endif /* TRACELEXER */

/*
 * Custom input function that uses getdelim(3) and stores the buffer
 * where the error functions can access it for better reporting.
 * On success, buf is guaranteed to end in a newline and not contain
 * embedded NULs.  Calls YY_FATAL_ERROR on error.
 */
static int
sudoers_input(char *buf, yy_size_t max_size)
{
    char *cp;
    size_t avail = sudolinebuf.len - sudolinebuf.off;
    debug_decl(sudoers_input, SUDOERS_DEBUG_PARSER);

    /* Refill line buffer if needed. */
    if (avail == 0) {
	/*
	 * Some getdelim(3) implementations write NUL to buf on EOF.
	 * We peek ahead one char to detect EOF and skip the getdelim() call.
	 * This will preserve the original value of the last line read.
	 */
	int ch = getc(sudoersin);
	if (ch == EOF)
	    goto sudoers_eof;
	ungetc(ch, sudoersin);
	avail = (size_t)getdelim(&sudolinebuf.buf, &sudolinebuf.size, '\n', sudoersin);
	if (avail == (size_t)-1) {
sudoers_eof:
	    /* EOF or error. */
	    if (feof(sudoersin))
		debug_return_int(0);
	    YY_FATAL_ERROR("input in flex scanner failed");
	}

	/* getdelim() can return embedded NULs, truncate if we find one. */
	cp = memchr(sudolinebuf.buf, '\0', avail);
	if (cp != NULL) {
	    *cp++ = '\n';
	    *cp = '\0';
	    avail = (size_t)(cp - sudolinebuf.buf);
	}

	/* Add trailing newline if it is missing. */
	if (sudolinebuf.buf[avail - 1] != '\n') {
	    if (avail + 2 >= sudolinebuf.size) {
		cp = realloc(sudolinebuf.buf, avail + 2);
		if (cp == NULL) {
		    YY_FATAL_ERROR("unable to allocate memory");
		    debug_return_int(0);
		}
		sudolinebuf.buf = cp;
		sudolinebuf.size = avail + 2;
	    }
	    sudolinebuf.buf[avail++] = '\n';
	    sudolinebuf.buf[avail] = '\0';
	}

	sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s:%d: %.*s", sudoers, sudolineno,
	    (int)(avail -1), sudolinebuf.buf);

	sudolinebuf.len = avail;
	sudolinebuf.off = 0;
	sudolinebuf.toke_start = sudolinebuf.toke_end = 0;
    }

    if (avail > max_size)
	avail = max_size;
    memcpy(buf, sudolinebuf.buf + sudolinebuf.off, avail);
    sudolinebuf.off += avail;

    debug_return_int((int)avail);
}
