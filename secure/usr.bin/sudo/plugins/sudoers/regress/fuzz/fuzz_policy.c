/*
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

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#ifndef HAVE_GETADDRINFO
# include <compat/getaddrinfo.h>
#endif

#include <sudoers.h>
#include <sudo_iolog.h>
#include <interfaces.h>
#include <timestamp.h>
#include "auth/sudo_auth.h"

extern char **environ;
extern sudo_dso_public struct policy_plugin sudoers_policy;

char *audit_msg;

static int pass;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static FILE *
open_data(const uint8_t *data, size_t size)
{
#ifdef HAVE_FMEMOPEN
    /* Operate in-memory. */
    return fmemopen((void *)data, size, "r");
#else
    char tempfile[] = "/tmp/sudoers.XXXXXX";
    size_t nwritten;
    int fd;

    /* Use (unlinked) temporary file. */
    fd = mkstemp(tempfile);
    if (fd == -1)
	return NULL;
    unlink(tempfile);
    nwritten = write(fd, data, size);
    if (nwritten != size) {
	close(fd);
	return NULL;
    }
    lseek(fd, 0, SEEK_SET);
    return fdopen(fd, "r");
#endif
}

/*
 * Array that gets resized as needed.
 */
struct dynamic_array {
    char **entries;
    size_t len;
    size_t size;
};

static void
free_strvec(char **vec)
{
    size_t i;

    for (i = 0; vec[i] != NULL; i++)
	free(vec[i]);
}

static void
free_dynamic_array(struct dynamic_array *arr)
{
    if (arr->entries != NULL) {
	free_strvec(arr->entries);
	free(arr->entries);
    }
    memset(arr, 0, sizeof(*arr));
}

static bool
push(struct dynamic_array *arr, const char *entry)
{
    char *copy = NULL;

    if (entry != NULL) {
	if ((copy = strdup(entry)) == NULL)
	    return false;
    }

    if (arr->len + (entry != NULL) >= arr->size) {
	char **tmp = reallocarray(arr->entries, arr->size + 1024, sizeof(char *));
	if (tmp == NULL) {
	    free(copy);
	    return false;
	}
	arr->entries = tmp;
	arr->size += 1024;
    }
    if (copy != NULL)
	arr->entries[arr->len++] = copy;
    arr->entries[arr->len] = NULL;

    return true;
}

static int
fuzz_conversation(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    int n;

    for (n = 0; n < num_msgs; n++) {
	const struct sudo_conv_message *msg = &msgs[n];

	switch (msg->msg_type & 0xff) {
	    case SUDO_CONV_PROMPT_ECHO_ON:
	    case SUDO_CONV_PROMPT_MASK:
	    case SUDO_CONV_PROMPT_ECHO_OFF:
		/* input not supported */
		return -1;
	    case SUDO_CONV_ERROR_MSG:
	    case SUDO_CONV_INFO_MSG:
		/* no output for fuzzers */
		break;
	    default:
		return -1;
	}
    }
    return 0;
}

static int
fuzz_printf(int msg_type, const char * restrict fmt, ...)
{
    return 0;
}

static int
fuzz_hook_stub(struct sudo_hook *hook)
{
    return 0;
}

/*
 * The fuzzing environment may not have DNS available, this may result
 * in long delays that cause a timeout when fuzzing.
 * This getaddrinfo() resolves every name as "localhost" (127.0.0.1).
 */
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
/* Avoid compilation errors if getaddrinfo() or freeaddrinfo() are macros. */
# undef getaddrinfo
# undef freeaddrinfo

int
# ifdef HAVE_GETADDRINFO
getaddrinfo(
# else
sudo_getaddrinfo(
# endif
    const char *nodename, const char *servname,
    const struct addrinfo *hints, struct addrinfo **res)
{
    struct addrinfo *ai;
    struct in_addr addr;
    unsigned short port = 0;

    /* Stub getaddrinfo(3) to avoid a DNS timeout in CIfuzz. */
    if (servname == NULL) {
	/* Must have either nodename or servname. */
	if (nodename == NULL)
	    return EAI_NONAME;
    } else {
	struct servent *servent;
	const char *errstr;

	/* Parse servname as a port number or IPv4 TCP service name. */
	port = sudo_strtonum(servname, 0, USHRT_MAX, &errstr);
	if (errstr != NULL && errno == ERANGE)
	    return EAI_SERVICE;
	if (hints != NULL && ISSET(hints->ai_flags, AI_NUMERICSERV))
	    return EAI_NONAME;
	servent = getservbyname(servname, "tcp");
	if (servent == NULL)
	    return EAI_NONAME;
	port = htons(servent->s_port);
    }

    /* Hard-code IPv4 localhost for fuzzing. */
    ai = calloc(1, sizeof(*ai) + sizeof(struct sockaddr_in));
    if (ai == NULL)
	return EAI_MEMORY;
    ai->ai_canonname = strdup("localhost");
    if (ai == NULL) {
	free(ai);
	return EAI_MEMORY;
    }
    ai->ai_family = AF_INET;
    ai->ai_protocol = IPPROTO_TCP;
    ai->ai_addrlen = sizeof(struct sockaddr_in);
    ai->ai_addr = (struct sockaddr *)(ai + 1);
    inet_pton(AF_INET, "127.0.0.1", &addr);
    ((struct sockaddr_in *)ai->ai_addr)->sin_family = AF_INET;
    ((struct sockaddr_in *)ai->ai_addr)->sin_addr = addr;
    ((struct sockaddr_in *)ai->ai_addr)->sin_port = htons(port);
    *res = ai;
    return 0;
}

void
# ifdef HAVE_GETADDRINFO
freeaddrinfo(struct addrinfo *ai)
# else
sudo_freeaddrinfo(struct addrinfo *ai)
# endif
{
    struct addrinfo *next;

    while (ai != NULL) {
	next = ai->ai_next;
	free(ai->ai_canonname);
	free(ai);
	ai = next;
    }
}
#endif /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */

enum fuzz_policy_pass {
    PASS_NONE,
    PASS_VERSION,
    PASS_CHECK_LOG_LOCAL,
    PASS_CHECK_LOG_REMOTE,
    PASS_CHECK_NOT_FOUND,
    PASS_CHECK_NOT_FOUND_DOT,
    PASS_LIST,
    PASS_LIST_OTHER,
    PASS_LIST_CHECK,
    PASS_VALIDATE,
    PASS_INVALIDATE
};

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    struct dynamic_array plugin_args = { NULL };
    struct dynamic_array settings = { NULL };
    struct dynamic_array user_info = { NULL };
    struct dynamic_array argv = { NULL };
    struct dynamic_array env_add = { NULL };
    char **command_info = NULL, **argv_out = NULL, **user_env_out = NULL;
    const char *errstr = NULL;
    const int num_passes = 10;
    char *line = NULL;
    size_t linesize = 0;
    ssize_t linelen;
    int res = 1;
    FILE *fp;

    fp = open_data(data, size);
    if (fp == NULL)
        return 0;

    initprogname("fuzz_policy");
    sudoers_debug_register(getprogname(), NULL);
    if (getenv("SUDO_FUZZ_VERBOSE") == NULL)
	sudo_warn_set_conversation(fuzz_conversation);

    /* user_info and settings must be non-NULL (even if empty). */
    push(&user_info, NULL);
    push(&settings, NULL);

    /* Iterate over each line of data. */
    while ((linelen = getdelim(&line, &linesize, '\n', fp)) != -1) {
	if (line[linelen - 1] == '\n')
	    line[linelen - 1] = '\0';

	/* Skip comments and blank lines. */
	if (line[0] == '#' || line[0] == '\0')
	    continue;

	/* plugin args */
	if (strncmp(line, "error_recovery=", sizeof("error_recovery=") - 1) == 0) {
	    push(&plugin_args, line);
	    continue;
	}
	if (strncmp(line, "sudoers_file=", sizeof("sudoers_file=") - 1) == 0) {
	    push(&plugin_args, line);
	    continue;
	}
	if (strncmp(line, "sudoers_mode=", sizeof("sudoers_mode=") - 1) == 0) {
	    push(&plugin_args, line);
	    continue;
	}
	if (strncmp(line, "sudoers_gid=", sizeof("sudoers_gid=") - 1) == 0) {
	    push(&plugin_args, line);
	    continue;
	}
	if (strncmp(line, "sudoers_uid=", sizeof("sudoers_uid=") - 1) == 0) {
	    push(&plugin_args, line);
	    continue;
	}
	if (strncmp(line, "ldap_conf=", sizeof("ldap_conf=") - 1) == 0) {
	    push(&plugin_args, line);
	    continue;
	}
	if (strncmp(line, "ldap_secret=", sizeof("ldap_secret=") - 1) == 0) {
	    push(&plugin_args, line);
	    continue;
	}

	/* user info */
	if (strncmp(line, "user=", sizeof("user=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "uid=", sizeof("uid=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "gid=", sizeof("gid=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "groups=", sizeof("groups=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "cwd=", sizeof("cwd=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "tty=", sizeof("tty=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "host=", sizeof("host=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "lines=", sizeof("lines=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "cols=", sizeof("cols=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "sid=", sizeof("sid=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "umask=", sizeof("umask=") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}
	if (strncmp(line, "rlimit_", sizeof("rlimit_") - 1) == 0) {
	    push(&user_info, line);
	    continue;
	}

	/* First argv entry is the command, the rest are args. */
	if (strncmp(line, "argv=", sizeof("argv=") - 1) == 0) {
	    push(&argv, line);
	    continue;
	}

	/* Additional environment variables to add. */
	if (strncmp(line, "env=", sizeof("env=") - 1) == 0) {
	    const char *cp = line + sizeof("env=") - 1;
	    if (strchr(cp, '=') != NULL)
		push(&env_add, cp);
	    continue;
	}

	/* Treat anything else as a setting. */
	push(&settings, line);
    }
    fclose(fp);
    free(line);
    line = NULL;

    /* Exercise code paths that use KRB5CCNAME and SUDO_PROMPT. */
    putenv((char *)"KRB5CCNAME=/tmp/krb5cc_123456");
    putenv((char *)"SUDO_PROMPT=[sudo] password for %p: ");

    sudoers_policy.register_hooks(SUDO_API_VERSION, fuzz_hook_stub);

    for (pass = 1; res == 1 && pass <= num_passes; pass++) {
	/* Call policy open function */
	res = sudoers_policy.open(SUDO_API_VERSION, fuzz_conversation,
	    fuzz_printf, settings.entries, user_info.entries, environ,
	    plugin_args.entries, &errstr);
	if (res == 1) {
	    if (argv.len == 0) {
		/* Must have a command to check. */
		push(&argv, "/usr/bin/id");
	    }

	    switch (pass) {
	    case PASS_NONE:
		break;
	    case PASS_VERSION:
		/* sudo -V */
		sudoers_policy.show_version(true);
		break;
	    case PASS_CHECK_LOG_LOCAL: {
		/* sudo command w/ local I/O logging (MODE_RUN) */
		sudoers_policy.check_policy((int)argv.len, argv.entries,
		    env_add.entries, &command_info, &argv_out, &user_env_out,
		    &errstr);
		/* call check_policy() again to check for leaks. */
		sudoers_policy.check_policy((int)argv.len, argv.entries,
		    env_add.entries, &command_info, &argv_out, &user_env_out,
		    &errstr);
		/* sudo_auth_begin_session() is stubbed out below. */
		sudoers_policy.init_session(NULL, NULL, NULL);
		break;
	    }
	    case PASS_CHECK_LOG_REMOTE:
		/* sudo command w/ remote I/O logging (MODE_RUN) */
		sudoers_policy.check_policy((int)argv.len, argv.entries,
		    env_add.entries, &command_info, &argv_out, &user_env_out,
		    &errstr);
		/* call check_policy() again to check for leaks. */
		sudoers_policy.check_policy((int)argv.len, argv.entries,
		    env_add.entries, &command_info, &argv_out, &user_env_out,
		    &errstr);
		/* sudo_auth_begin_session() is stubbed out below. */
		sudoers_policy.init_session(NULL, NULL, NULL);
		break;
	    case PASS_CHECK_NOT_FOUND:
		/* sudo command (not found) */
		sudoers_policy.check_policy((int)argv.len, argv.entries,
		    env_add.entries, &command_info, &argv_out, &user_env_out,
		    &errstr);
		/* sudo_auth_begin_session() is stubbed out below. */
		sudoers_policy.init_session(NULL, NULL, NULL);
		break;
	    case PASS_CHECK_NOT_FOUND_DOT:
		/* sudo command (found but in cwd) */
		sudoers_policy.check_policy((int)argv.len, argv.entries,
		    env_add.entries, &command_info, &argv_out, &user_env_out,
		    &errstr);
		/* call check_policy() again to check for leaks. */
		sudoers_policy.check_policy((int)argv.len, argv.entries,
		    env_add.entries, &command_info, &argv_out, &user_env_out,
		    &errstr);
		/* sudo_auth_begin_session() is stubbed out below. */
		sudoers_policy.init_session(NULL, NULL, NULL);
		break;
	    case PASS_LIST:
		/* sudo -l (MODE_LIST) */
		sudoers_policy.list(0, NULL, false, NULL, &errstr);
		/* call list() again to check for leaks. */
		sudoers_policy.list(0, NULL, false, NULL, &errstr);
		break;
	    case PASS_LIST_OTHER:
		/* sudo -l -U root (MODE_LIST) */
		sudoers_policy.list(0, NULL, false, "root", &errstr);
		/* call list() again to check for leaks. */
		sudoers_policy.list(0, NULL, false, "root", &errstr);
		break;
	    case PASS_LIST_CHECK:
		/* sudo -l command (MODE_CHECK) */
		sudoers_policy.list((int)argv.len, argv.entries, false, NULL,
		    &errstr);
		/* call list() again to check for leaks. */
		sudoers_policy.list((int)argv.len, argv.entries, false, NULL,
		    &errstr);
		break;
	    case PASS_VALIDATE:
		/* sudo -v (MODE_VALIDATE) */
		sudoers_policy.validate(&errstr);
		/* call validate() again to check for leaks. */
		sudoers_policy.validate(&errstr);
		break;
	    case PASS_INVALIDATE:
		/* sudo -k */
		sudoers_policy.invalidate(false);
		/* call invalidate() again to check for leaks. */
		sudoers_policy.invalidate(false);
		break;
	    }
	}

	/* Free resources. */
	if (sudoers_policy.close != NULL)
	    sudoers_policy.close(0, 0);
	else
	    sudoers_cleanup();
    }

    sudoers_policy.deregister_hooks(SUDO_API_VERSION, fuzz_hook_stub);

    free_dynamic_array(&plugin_args);
    free_dynamic_array(&settings);
    free_dynamic_array(&user_info);
    free_dynamic_array(&argv);
    free_dynamic_array(&env_add);

    sudoers_debug_deregister();

    fflush(stdout);

    return 0;
}

/* STUB */
bool
user_is_exempt(const struct sudoers_context *ctx)
{
    return false;
}

/* STUB */
bool
set_interfaces(const char *ai)
{
    return true;
}

/* STUB */
void
dump_interfaces(const char *ai)
{
    return;
}

/* STUB */
void
dump_auth_methods(void)
{
    return;
}

/* STUB */
int
sudo_auth_begin_session(const struct sudoers_context *ctx, struct passwd *pw,
    char **user_env[])
{
    return 1;
}

/* STUB */
int
sudo_auth_end_session(void)
{
    return 1;
}

/* STUB */
bool
sudo_auth_needs_end_session(void)
{
    return false;
}

/* STUB */
int
timestamp_remove(const struct sudoers_context *ctx, bool unlink_it)
{
    return true;
}

/* STUB */
int
create_admin_success_flag(const struct sudoers_context *ctx)
{
    return true;
}

/* STUB */
static int
sudo_file_open(struct sudoers_context *ctx, struct sudo_nss *nss)
{
    return 0;
}

/* STUB */
static int
sudo_file_close(struct sudoers_context *ctx, struct sudo_nss *nss)
{
    return 0;
}

/* STUB */
static struct sudoers_parse_tree *
sudo_file_parse(struct sudoers_context *ctx, const struct sudo_nss *nss)
{
    static struct sudoers_parse_tree parse_tree;

    return &parse_tree;
}

/* STUB */
static int
sudo_file_query(struct sudoers_context *ctx, const struct sudo_nss *nss,
    struct passwd *pw)
{
    return 0;
}

/* STUB */
static int
sudo_file_getdefs(struct sudoers_context *ctx, const struct sudo_nss *nss)
{
    /* Set some Defaults */
    set_default(ctx, "log_input", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "log_output", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "env_file", "/dev/null", true, "sudoers", 1, 1, false);
    set_default(ctx, "restricted_env_file", "/dev/null", true, "sudoers", 1, 1, false);
    set_default(ctx, "exempt_group", "sudo", true, "sudoers", 1, 1, false);
    set_default(ctx, "runchroot", "/", true, "sudoers", 1, 1, false);
    set_default(ctx, "runcwd", "~", true, "sudoers", 1, 1, false);
    set_default(ctx, "fqdn", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "runas_default", "root", true, "sudoers", 1, 1, false);
    set_default(ctx, "tty_tickets", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "umask", "022", true, "sudoers", 1, 1, false);
    set_default(ctx, "logfile", "/var/log/sudo", true, "sudoers", 1, 1, false);
    set_default(ctx, "syslog", "auth", true, "sudoers", 1, 1, false);
    set_default(ctx, "syslog_goodpri", "notice", true, "sudoers", 1, 1, false);
    set_default(ctx, "syslog_badpri", "alert", true, "sudoers", 1, 1, false);
    set_default(ctx, "syslog_maxlen", "2048", true, "sudoers", 1, 1, false);
    set_default(ctx, "loglinelen", "0", true, "sudoers", 1, 1, false);
    set_default(ctx, "log_year", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "log_host", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "mailerpath", NULL, false, "sudoers", 1, 1, false);
    set_default(ctx, "mailerflags", "-t", true, "sudoers", 1, 1, false);
    set_default(ctx, "mailto", "root@localhost", true, "sudoers", 1, 1, false);
    set_default(ctx, "mailfrom", "sudo@sudo.ws", true, "sudoers", 1, 1, false);
    set_default(ctx, "mailsub", "Someone has been naughty on %h", true, "sudoers", 1, 1, false);
    set_default(ctx, "timestampowner", "#0", true, "sudoers", 1, 1, false);
    set_default(ctx, "compress_io", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "iolog_flush", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "iolog_flush", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "maxseq", "2176782336", true, "sudoers", 1, 1, false);
    set_default(ctx, "sudoedit_checkdir", NULL, false, "sudoers", 1, 1, false);
    set_default(ctx, "sudoedit_follow", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "ignore_iolog_errors", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "ignore_iolog_errors", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "noexec", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "exec_background", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "use_pty", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "utmp_runas", NULL, true, "sudoers", 1, 1, false);
    set_default(ctx, "iolog_mode", "0640", true, "sudoers", 1, 1, false);
    set_default(ctx, "iolog_user", NULL, false, "sudoers", 1, 1, false);
    set_default(ctx, "iolog_group", NULL, false, "sudoers", 1, 1, false);
    if (pass != PASS_CHECK_LOG_LOCAL) {
	set_default(ctx, "log_servers", "localhost", true, "sudoers", 1, 1, false);
	set_default(ctx, "log_server_timeout", "30", true, "sudoers", 1, 1, false);
	set_default(ctx, "log_server_cabundle", "/etc/ssl/cacert.pem", true, "sudoers", 1, 1, false);
	set_default(ctx, "log_server_peer_cert", "/etc/ssl/localhost.crt", true, "sudoers", 1, 1, false);
	set_default(ctx, "log_server_peer_key", "/etc/ssl/private/localhost.key", true, "sudoers", 1, 1, false);
    }

    return 0;
}

static struct sudo_nss sudo_nss_file = {
    { NULL, NULL },
    "sudoers",
    sudo_file_open,
    sudo_file_close,
    sudo_file_parse,
    sudo_file_query,
    sudo_file_getdefs
};

struct sudo_nss_list *
sudo_read_nss(void)
{
    static struct sudo_nss_list snl = TAILQ_HEAD_INITIALIZER(snl);

    if (TAILQ_EMPTY(&snl))
	TAILQ_INSERT_TAIL(&snl, &sudo_nss_file, entries);

    return &snl;
}

/* STUB */
int
check_user(struct sudoers_context *ctx, unsigned int validated,
    unsigned int mode)
{
    return AUTH_SUCCESS;
}

/* STUB */
int
check_user_runchroot(const char *runchroot)
{
    return true;
}

/* STUB */
int
check_user_runcwd(const char *runcwd)
{
    return true;
}

/* STUB */
void
group_plugin_unload(void)
{
    return;
}

/* STUB */
bool
log_warning(const struct sudoers_context *ctx, unsigned int flags,
    const char * restrict fmt, ...)
{
    return true;
}

/* STUB */
bool
log_warningx(const struct sudoers_context *ctx, unsigned int flags,
    const char * restrict fmt, ...)
{
    return true;
}

/* STUB */
bool
gai_log_warning(const struct sudoers_context *ctx, unsigned int flags,
    int errnum, const char * restrict fmt, ...)
{
    return true;
}

/* STUB */
bool
log_denial(const struct sudoers_context *ctx, unsigned int status,
    bool inform_user)
{
    return true;
}

/* STUB */
bool
log_failure(const  struct sudoers_context *ctx,unsigned int status, int flags)
{
    return true;
}

/* STUB */
bool
log_exit_status(const struct sudoers_context *ctx, int exit_status)
{
    return true;
}

/* STUB */
bool
mail_parse_errors(const struct sudoers_context *ctx)
{
    return true;
}

/* STUB */
bool
log_parse_error(const struct sudoers_context *ctx, const char *file,
    int line, int column, const char * restrict fmt, va_list args)
{
    return true;
}

/* STUB */
int
audit_failure(const struct sudoers_context *ctx, char *const argv[],
    char const * restrict const fmt, ...)
{
    return 0;
}

/* STUB */
unsigned int
sudoers_lookup(struct sudo_nss_list *snl, struct sudoers_context *ctx,
    time_t now, sudoers_lookup_callback_fn_t callback, void *cb_data,
    int *cmnd_status, int pwflag)
{
    return VALIDATE_SUCCESS;
}

/* STUB */
int
display_cmnd(struct sudoers_context *ctx, const struct sudo_nss_list *snl,
    struct passwd *pw, int verbose)
{
    return true;
}

/* STUB */
int
display_privs(struct sudoers_context *ctx, const struct sudo_nss_list *snl,
    struct passwd *pw, int verbose)
{
    return true;
}

/* STUB */
int
find_path(const char *infile, char **outfile, struct stat *sbp,
    const char *path, bool ignore_dot, char * const *allowlist)
{
    switch (pass) {
    case PASS_CHECK_NOT_FOUND:
	return NOT_FOUND;
    case PASS_CHECK_NOT_FOUND_DOT:
	return NOT_FOUND_DOT;
    default:
	if (infile[0] == '/') {
	    *outfile = strdup(infile);
	} else {
	    if (asprintf(outfile, "/usr/bin/%s", infile) == -1)
		*outfile = NULL;
	}
	if (*outfile == NULL)
	    return NOT_FOUND_ERROR;
	return FOUND;
    }
}

/* STUB */
int
resolve_cmnd(struct sudoers_context *ctx, const char *infile, char **outfile,
    const char *path)
{
    return find_path(infile, outfile, NULL, path, false, NULL);
}

/* STUB */
bool
expand_iolog_path(const char *inpath, char *path, size_t pathlen,
    const struct iolog_path_escape *escapes, void *closure)
{
    return strlcpy(path, inpath, pathlen) < pathlen;
}

/* STUB */
bool
iolog_nextid(const char *iolog_dir, char sessid[7])
{
    strlcpy(sessid, "000001", 7);
    return true;
}

/* STUB */
bool
cb_maxseq(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    return true;
}

/* STUB */
bool
cb_iolog_user(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    return true;
}

/* STUB */
bool
cb_iolog_group(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    return true;
}

/* STUB */
bool
cb_iolog_mode(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    return true;
}

/* STUB */
bool
cb_group_plugin(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    return true;
}

/* STUB */
bool
cb_timestampowner(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    return true;
}

/* STUB */
void
bsdauth_set_style(const char *style)
{
    return;
}
