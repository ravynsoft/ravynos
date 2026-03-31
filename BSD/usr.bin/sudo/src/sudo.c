/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef __TANDEM
# include <floss.h>
#endif

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/socket.h>
#ifdef __linux__
# include <sys/prctl.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#ifdef HAVE_SELINUX
# include <selinux/selinux.h>		/* for is_selinux_enabled() */
#endif
#ifdef HAVE_SETAUTHDB
# include <usersec.h>
#endif /* HAVE_SETAUTHDB */
#if defined(HAVE_GETPRPWNAM) && defined(HAVE_SET_AUTH_PARAMETERS)
# ifdef __hpux
#  undef MAXINT
#  include <hpsecurity.h>
# else
#  include <sys/security.h>
# endif /* __hpux */
# include <prot.h>
#endif /* HAVE_GETPRPWNAM && HAVE_SET_AUTH_PARAMETERS */

#include <sudo.h>
#include <sudo_plugin.h>
#include <sudo_plugin_int.h>

/*
 * Local variables
 */
struct plugin_container policy_plugin;
struct plugin_container_list io_plugins = TAILQ_HEAD_INITIALIZER(io_plugins);
struct plugin_container_list audit_plugins = TAILQ_HEAD_INITIALIZER(audit_plugins);
struct plugin_container_list approval_plugins = TAILQ_HEAD_INITIALIZER(approval_plugins);
int sudo_debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;
static struct sudo_event_base *sudo_event_base;

struct sudo_gc_entry {
    SLIST_ENTRY(sudo_gc_entry) entries;
    enum sudo_gc_types type;
    union {
	char **vec;
	void *ptr;
    } u;
};
SLIST_HEAD(sudo_gc_list, sudo_gc_entry);
#ifdef NO_LEAKS
static struct sudo_gc_list sudo_gc_list = SLIST_HEAD_INITIALIZER(sudo_gc_list);
#endif

/*
 * Local functions
 */
static void fix_fds(void);
static void sudo_check_suid(const char *path);
static char **get_user_info(struct user_details *);
static void command_info_to_details(char * const info[],
    struct command_details *details);
static void gc_init(void);

/* Policy plugin convenience functions. */
static void policy_open(void);
static void policy_close(const char *cmnd, int exit_status, int error);
static int policy_show_version(int verbose);
static bool policy_check(int argc, char * const argv[], char *env_add[],
    char **command_info[], char **run_argv[], char **run_envp[]);
static void policy_list(int argc, char * const argv[],
    int verbose, const char *user);
static void policy_validate(char * const argv[]);
static void policy_invalidate(int unlinkit);

/* I/O log plugin convenience functions. */
static bool iolog_open(char * const command_info[], int run_argc,
    char * const run_argv[], char * const run_envp[]);
static void iolog_close(int exit_status, int error);
static void iolog_show_version(int verbose, int argc, char * const argv[],
    char * const envp[]);
static void unlink_plugin(struct plugin_container_list *plugin_list, struct plugin_container *plugin);
static void free_plugin_container(struct plugin_container *plugin, bool ioplugin);

/* Audit plugin convenience functions (some are public). */
static void audit_open(void);
static void audit_close(int exit_status, int error);
static void audit_show_version(int verbose);

/* Approval plugin convenience functions (some are public). */
static void approval_show_version(int verbose);

sudo_dso_public int main(int argc, char *argv[], char *envp[]);

static struct sudo_settings *sudo_settings;
static char * const *user_info, * const *submit_argv, * const *submit_envp;
static int submit_optind;

int
main(int argc, char *argv[], char *envp[])
{
    struct command_details command_details;
    struct user_details user_details;
    unsigned int sudo_mode;
    int nargc, status = 0;
    char **nargv, **env_add;
    char **command_info = NULL, **argv_out = NULL, **run_envp = NULL;
    const char * const allowed_prognames[] = { "sudo", "sudoedit", NULL };
    const char *list_user;
    sigset_t mask;
    debug_decl_vars(main, SUDO_DEBUG_MAIN);

    /* Only allow "sudo" or "sudoedit" as the program name. */
    initprogname2(argc > 0 ? argv[0] : "sudo", allowed_prognames);

    /* Crank resource limits to unlimited. */
    unlimit_sudo();

    /* Make sure fds 0-2 are open and do OS-specific initialization. */
    fix_fds();
    os_init(argc, argv, envp);

    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE_NAME, LOCALEDIR);
    textdomain(PACKAGE_NAME);

    (void) tzset();

    /* Must be done before we do any password lookups */
#if defined(HAVE_GETPRPWNAM) && defined(HAVE_SET_AUTH_PARAMETERS)
    (void) set_auth_parameters(argc, argv);
# ifdef HAVE_INITPRIVS
    initprivs();
# endif
#endif /* HAVE_GETPRPWNAM && HAVE_SET_AUTH_PARAMETERS */

    /* Initialize the debug subsystem. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG) == -1)
	return EXIT_FAILURE;
    sudo_debug_instance = sudo_debug_register(getprogname(),
	NULL, NULL, sudo_conf_debug_files(getprogname()), -1);
    if (sudo_debug_instance == SUDO_DEBUG_INSTANCE_ERROR)
	return EXIT_FAILURE;

    /* Make sure we are setuid root. */
    sudo_check_suid(argc > 0 ? argv[0] : "sudo");

    /* Save original signal state and setup default signal handlers. */
    save_signals();
    init_signals();

    /* Reset signal mask to the default value (unblock). */
    (void) sigemptyset(&mask);
    (void) sigprocmask(SIG_SETMASK, &mask, NULL);

    /* Parse the rest of sudo.conf. */
    sudo_conf_read(NULL, SUDO_CONF_ALL & ~SUDO_CONF_DEBUG);

    /* Fill in user_info with user name, uid, cwd, etc. */
    if ((user_info = get_user_info(&user_details)) == NULL)
	return EXIT_FAILURE; /* get_user_info printed error message */

    /* Disable core dumps if not enabled in sudo.conf. */
    if (sudo_conf_disable_coredump())
	disable_coredump();

    /* Parse command line arguments, preserving the original argv/envp. */
    submit_argv = argv;
    submit_envp = envp;
    sudo_mode = parse_args(argc, argv, user_details.shell, &submit_optind,
	&nargc, &nargv, &sudo_settings, &env_add, &list_user);
    sudo_debug_printf(SUDO_DEBUG_DEBUG, "sudo_mode 0x%x", sudo_mode);

    /* Print sudo version early, in case of plugin init failure. */
    if (ISSET(sudo_mode, MODE_VERSION)) {
	printf(_("Sudo version %s\n"), PACKAGE_VERSION);
	if (user_details.cred.uid == ROOT_UID)
	    (void) printf(_("Configure options: %s\n"), CONFIGURE_ARGS);
    }

    /* Use conversation function for sudo_(warn|fatal)x? for plugins. */
    sudo_warn_set_conversation(sudo_conversation);

    /* Load plugins. */
    if (!sudo_load_plugins())
	sudo_fatalx("%s", U_("fatal error, unable to load plugins"));

    /* Allocate event base so plugin can use it. */
    if ((sudo_event_base = sudo_ev_base_alloc()) == NULL)
	sudo_fatalx("%s", U_("unable to allocate memory"));

    /* Open policy and audit plugins. */
    /* XXX - audit policy_open errors */
    audit_open();
    policy_open();

    switch (sudo_mode & MODE_MASK) {
	case MODE_VERSION:
	    policy_show_version(!user_details.cred.uid);
	    iolog_show_version(!user_details.cred.uid, nargc, nargv,
		submit_envp);
	    approval_show_version(!user_details.cred.uid);
	    audit_show_version(!user_details.cred.uid);
	    break;
	case MODE_VALIDATE:
	case MODE_VALIDATE|MODE_INVALIDATE:
	    policy_validate(nargv);
	    break;
	case MODE_KILL:
	case MODE_INVALIDATE:
	    policy_invalidate(sudo_mode == MODE_KILL);
	    break;
	case MODE_CHECK:
	case MODE_CHECK|MODE_INVALIDATE:
	case MODE_LIST:
	case MODE_LIST|MODE_INVALIDATE:
	    policy_list(nargc, nargv, ISSET(sudo_mode, MODE_LONG_LIST),
		list_user);
	    break;
	case MODE_EDIT:
	case MODE_RUN:
	    if (!policy_check(nargc, nargv, env_add, &command_info, &argv_out,
		    &run_envp))
		goto access_denied;

	    /* Reset nargv/nargc based on argv_out. */
	    /* XXX - leaks old nargv in shell mode */
	    for (nargv = argv_out, nargc = 0; nargv[nargc] != NULL; nargc++)
		continue;
	    if (nargc == 0)
		sudo_fatalx("%s",
		    U_("plugin did not return a command to execute"));

	    /* Approval plugins run after policy plugin accepts the command. */
	    if (!approval_check(command_info, nargv, run_envp))
		goto access_denied;

	    /* Open I/O plugin once policy and approval plugins succeed. */
	    if (!iolog_open(command_info, nargc, nargv, run_envp))
		goto access_denied;

	    /* Audit the accept event on behalf of the sudo front-end. */
	    if (!audit_accept("sudo", SUDO_FRONT_END, command_info,
		    nargv, run_envp))
		goto access_denied;

	    /* Setup command details and run command/edit. */
	    command_info_to_details(command_info, &command_details);
	    if (command_details.utmp_user == NULL)
		command_details.utmp_user = user_details.username;
	    command_details.submitcwd = user_details.cwd;
	    command_details.tty = user_details.tty;
	    command_details.argv = nargv;
	    command_details.argc = nargc;
	    command_details.envp = run_envp;
	    if (ISSET(sudo_mode, MODE_LOGIN_SHELL))
		SET(command_details.flags, CD_LOGIN_SHELL);
	    if (ISSET(sudo_mode, MODE_BACKGROUND))
		SET(command_details.flags, CD_BACKGROUND);
	    if (ISSET(command_details.flags, CD_SUDOEDIT)) {
		status = sudo_edit(&command_details, &user_details);
	    } else {
		status = run_command(&command_details, &user_details);
	    }
	    /* The close method was called by sudo_edit/run_command. */
	    break;
	default:
	    sudo_fatalx(U_("unexpected sudo mode 0x%x"), sudo_mode);
    }

    /*
     * If the command was terminated by a signal, sudo needs to terminated
     * the same way.  Otherwise, the shell may ignore a keyboard-generated
     * signal.  However, we want to avoid having sudo dump core itself.
     */
    if (WIFSIGNALED(status)) {
	struct sigaction sa;

	/* Make sure sudo doesn't dump core itself. */
	disable_coredump();

	/* Re-send the signal to the main sudo process. */
	memset(&sa, 0, sizeof(sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_DFL;
	sigaction(WTERMSIG(status), &sa, NULL);
	sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys,
	    WTERMSIG(status) | 128);
	kill(getpid(), WTERMSIG(status));
    }
    sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys,
	WEXITSTATUS(status));
    return WEXITSTATUS(status);

access_denied:
    /* Policy/approval failure, close policy and audit plugins before exit. */
    if (policy_plugin.u.policy->version >= SUDO_API_MKVERSION(1, 15))
	policy_close(NULL, 0, EACCES);
    audit_close(SUDO_PLUGIN_NO_STATUS, 0);
    sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys,
	EXIT_FAILURE);
    return EXIT_FAILURE;
}

int
os_init_common(int argc, char *argv[], char *envp[])
{
#ifdef STATIC_SUDOERS_PLUGIN
    preload_static_symbols();
#endif
    gc_init();
    return 0;
}

/*
 * Ensure that stdin, stdout and stderr are open; set to /dev/null if not.
 * Some operating systems do this automatically in the kernel or libc.
 */
static void
fix_fds(void)
{
    int miss[3];
    debug_decl(fix_fds, SUDO_DEBUG_UTIL);

    /*
     * stdin, stdout and stderr must be open; set them to /dev/null
     * if they are closed.
     */
    miss[STDIN_FILENO] = fcntl(STDIN_FILENO, F_GETFL, 0) == -1;
    miss[STDOUT_FILENO] = fcntl(STDOUT_FILENO, F_GETFL, 0) == -1;
    miss[STDERR_FILENO] = fcntl(STDERR_FILENO, F_GETFL, 0) == -1;
    if (miss[STDIN_FILENO] || miss[STDOUT_FILENO] || miss[STDERR_FILENO]) {
	int devnull =
	    open(_PATH_DEVNULL, O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (devnull == -1)
	    sudo_fatal(U_("unable to open %s"), _PATH_DEVNULL);
	if (miss[STDIN_FILENO] && dup2(devnull, STDIN_FILENO) == -1)
	    sudo_fatal("dup2");
	if (miss[STDOUT_FILENO] && dup2(devnull, STDOUT_FILENO) == -1)
	    sudo_fatal("dup2");
	if (miss[STDERR_FILENO] && dup2(devnull, STDERR_FILENO) == -1)
	    sudo_fatal("dup2");
	if (devnull > STDERR_FILENO)
	    close(devnull);
    }
    debug_return;
}

/*
 * Allocate space for groups and fill in using sudo_getgrouplist2()
 * for when we cannot (or don't want to) use getgroups().
 * Returns 0 on success and -1 on failure.
 */
static int
fill_group_list(const char *user, struct sudo_cred *cred)
{
    int ret = -1;
    debug_decl(fill_group_list, SUDO_DEBUG_UTIL);

    /*
     * If user specified a max number of groups, use it, otherwise let
     * sudo_getgrouplist2() allocate the group vector.
     */
    cred->ngroups = sudo_conf_max_groups();
    if (cred->ngroups > 0) {
	cred->groups =
	    reallocarray(NULL, (size_t)cred->ngroups, sizeof(GETGROUPS_T));
	if (cred->groups != NULL) {
	    /* Clamp to max_groups if insufficient space for all groups. */
	    if (sudo_getgrouplist2(user, cred->gid, &cred->groups,
		    &cred->ngroups) == -1) {
		cred->ngroups = sudo_conf_max_groups();
	    }
	    ret = 0;
	}
    } else {
	cred->groups = NULL;
	ret = sudo_getgrouplist2(user, cred->gid, &cred->groups,
	    &cred->ngroups);
    }
    if (ret == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: %s: unable to get groups via sudo_getgrouplist2()",
	    __func__, user);
    } else {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: %s: got %d groups via sudo_getgrouplist2()",
	    __func__, user, cred->ngroups);
    }
    debug_return_int(ret);
}

static char *
get_user_groups(const char *user, struct sudo_cred *cred)
{
    char *cp, *gid_list = NULL;
    size_t glsize;
    int i, len, group_source;
    debug_decl(get_user_groups, SUDO_DEBUG_UTIL);

    cred->groups = NULL;
    group_source = sudo_conf_group_source();
    if (group_source != GROUP_SOURCE_DYNAMIC) {
	long maxgroups = sysconf(_SC_NGROUPS_MAX);
	if (maxgroups < 0)
	    maxgroups = NGROUPS_MAX;

	/* Note that macOS may return ngroups > NGROUPS_MAX. */
	cred->ngroups = getgroups(0, NULL); // -V575
	if (cred->ngroups > 0) {
	    /* Use groups from kernel if not at limit or source is static. */
	    if (cred->ngroups != maxgroups || group_source == GROUP_SOURCE_STATIC) {
		cred->groups = reallocarray(NULL, (size_t)cred->ngroups,
		    sizeof(GETGROUPS_T));
		if (cred->groups == NULL)
		    goto done;
		cred->ngroups = getgroups(cred->ngroups, cred->groups);
		if (cred->ngroups < 0) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
			"%s: unable to get %d groups via getgroups()",
			__func__, cred->ngroups);
		    free(cred->groups);
		    cred->groups = NULL;
		} else {
		    sudo_debug_printf(SUDO_DEBUG_INFO,
			"%s: got %d groups via getgroups()",
			__func__, cred->ngroups);
		}
	    }
	}
    }
    if (cred->groups == NULL) {
	/*
	 * Query group database if kernel list is too small or disabled.
	 * Typically, this is because NFS can only support up to 16 groups.
	 */
	if (fill_group_list(user, cred) == -1)
	    goto done;
    }

    /*
     * Format group list as a comma-separated string of gids.
     */
    glsize = sizeof("groups=") - 1 +
	((size_t)cred->ngroups * (STRLEN_MAX_UNSIGNED(gid_t) + 1));
    if ((gid_list = malloc(glsize)) == NULL)
	goto done;
    memcpy(gid_list, "groups=", sizeof("groups=") - 1);
    cp = gid_list + sizeof("groups=") - 1;
    glsize -= (size_t)(cp - gid_list);
    for (i = 0; i < cred->ngroups; i++) {
	len = snprintf(cp, glsize, "%s%u", i ? "," : "",
	    (unsigned int)cred->groups[i]);
	if (len < 0 || (size_t)len >= glsize)
	    sudo_fatalx(U_("internal error, %s overflow"), __func__);
	cp += len;
	glsize -= (size_t)len;
    }
done:
    debug_return_str(gid_list);
}

/*
 * Return user information as an array of name=value pairs.
 * and fill in struct user_details (which shares the same strings).
 */
static char **
get_user_info(struct user_details *ud)
{
    char *cp, **info, path[PATH_MAX];
    size_t info_max = 32 + RLIM_NLIMITS;
    size_t i = 0, n;
    mode_t mask;
    struct passwd *pw;
    int ttyfd;
    debug_decl(get_user_info, SUDO_DEBUG_UTIL);

    /*
     * On BSD systems you can set a hint to keep the password and
     * group databases open instead of having to open and close
     * them all the time.  Since sudo does a lot of password and
     * group lookups, keeping the file open can speed things up.
     */
#ifdef HAVE_SETPASSENT
    setpassent(1);
#endif /* HAVE_SETPASSENT */
#ifdef HAVE_SETGROUPENT
    setgroupent(1);
#endif /* HAVE_SETGROUPENT */

    memset(ud, 0, sizeof(*ud));

    /* XXX - bound check number of entries */
    info = reallocarray(NULL, info_max, sizeof(char *));
    if (info == NULL)
	goto oom;

    ud->pid = getpid();
    ud->ppid = getppid();
    ud->pgid = getpgid(0);
    ttyfd = open(_PATH_TTY, O_RDWR);
    sudo_get_ttysize(ttyfd, &ud->ts_rows, &ud->ts_cols);
    if (ttyfd != -1) {
	if ((ud->tcpgid = tcgetpgrp(ttyfd)) == -1)
	    ud->tcpgid = 0;
	close(ttyfd);
    }
    if ((ud->sid = getsid(0)) == -1)
	ud->sid = 0;

    ud->cred.uid = getuid();
    ud->cred.euid = geteuid();
    ud->cred.gid = getgid();
    ud->cred.egid = getegid();

    /* Store cred for use by sudo_askpass(). */
    sudo_askpass_cred(&ud->cred);

#ifdef HAVE_SETAUTHDB
    aix_setauthdb(IDtouser(ud->cred.uid), NULL);
#endif
    pw = getpwuid(ud->cred.uid);
#ifdef HAVE_SETAUTHDB
    aix_restoreauthdb();
#endif
    if (pw == NULL)
	sudo_fatalx(U_("you do not exist in the %s database"), "passwd");

    info[i] = sudo_new_key_val("user", pw->pw_name);
    if (info[i] == NULL)
	goto oom;
    ud->username = info[i] + sizeof("user=") - 1;

    /* Stash user's shell for use with the -s flag; don't pass to plugin. */
    if ((ud->shell = getenv("SHELL")) == NULL || ud->shell[0] == '\0') {
	ud->shell = pw->pw_shell[0] ? pw->pw_shell : _PATH_SUDO_BSHELL;
    }
    if ((cp = strdup(ud->shell)) == NULL)
	goto oom;
    if (!gc_add(GC_PTR, cp))
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    ud->shell = cp;

    if (asprintf(&info[++i], "pid=%d", (int)ud->pid) == -1)
	goto oom;
    if (asprintf(&info[++i], "ppid=%d", (int)ud->ppid) == -1)
	goto oom;
    if (asprintf(&info[++i], "pgid=%d", (int)ud->pgid) == -1)
	goto oom;
    if (asprintf(&info[++i], "tcpgid=%d", (int)ud->tcpgid) == -1)
	goto oom;
    if (asprintf(&info[++i], "sid=%d", (int)ud->sid) == -1)
	goto oom;
    if (asprintf(&info[++i], "uid=%u", (unsigned int)ud->cred.uid) == -1)
	goto oom;
    if (asprintf(&info[++i], "euid=%u", (unsigned int)ud->cred.euid) == -1)
	goto oom;
    if (asprintf(&info[++i], "gid=%u", (unsigned int)ud->cred.gid) == -1)
	goto oom;
    if (asprintf(&info[++i], "egid=%u", (unsigned int)ud->cred.egid) == -1)
	goto oom;

    if ((cp = get_user_groups(ud->username, &ud->cred)) == NULL)
	goto oom;
    info[++i] = cp;
    if (!gc_add(GC_PTR, ud->cred.groups))
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    mask = umask(0);
    umask(mask);
    if (asprintf(&info[++i], "umask=0%o", (unsigned int)mask) == -1)
	goto oom;

    if (getcwd(path, sizeof(path)) != NULL) {
	info[++i] = sudo_new_key_val("cwd", path);
	if (info[i] == NULL)
	    goto oom;
	ud->cwd = info[i] + sizeof("cwd=") - 1;
    }

    if (get_process_ttyname(path, sizeof(path)) != NULL) {
	info[++i] = sudo_new_key_val("tty", path);
	if (info[i] == NULL)
	    goto oom;
	ud->tty = info[i] + sizeof("tty=") - 1;
    } else {
	/* tty may not always be present */
	if (errno != ENOENT)
	    sudo_warn("%s", U_("unable to determine tty"));
    }

    cp = sudo_gethostname();
    info[++i] = sudo_new_key_val("host", cp ? cp : "localhost");
    free(cp);
    if (info[i] == NULL)
	goto oom;
    ud->host = info[i] + sizeof("host=") - 1;

    if (asprintf(&info[++i], "lines=%d", ud->ts_rows) == -1)
	goto oom;
    if (asprintf(&info[++i], "cols=%d", ud->ts_cols) == -1)
	goto oom;

    n = serialize_rlimits(&info[i + 1], info_max - (i + 1));
    if (n == (size_t)-1)
	goto oom;
    i += n;

    info[++i] = NULL;

    /* Add to list of vectors to be garbage collected at exit. */
    if (!gc_add(GC_VECTOR, info))
	goto bad;

    debug_return_ptr(info);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
bad:
    while (i)
	free(info[--i]);
    free(info);
    debug_return_ptr(NULL);
}

/*
 * Convert a command_info array into a command_details structure.
 */
static void
command_info_to_details(char * const info[], struct command_details *details)
{
    const char *errstr;
    char *cp;
    id_t id;
    size_t i;
    debug_decl(command_info_to_details, SUDO_DEBUG_PCOMM);

    memset(details, 0, sizeof(*details));
    details->info = info;
    details->closefrom = -1;
    details->execfd = -1;
    details->flags = CD_SUDOEDIT_CHECKDIR | CD_SET_GROUPS;
    TAILQ_INIT(&details->preserved_fds);

#define SET_STRING(s, n) \
    if (strncmp(s, info[i], sizeof(s) - 1) == 0 && info[i][sizeof(s) - 1]) { \
	details->n = info[i] + sizeof(s) - 1; \
	break; \
    }
#define SET_FLAG(s, n) \
    if (strncmp(s, info[i], sizeof(s) - 1) == 0) { \
	switch (sudo_strtobool(info[i] + sizeof(s) - 1)) { \
	    case true: \
		SET(details->flags, n); \
		break; \
	    case false: \
		CLR(details->flags, n); \
		break; \
	    default: \
		sudo_debug_printf(SUDO_DEBUG_ERROR, \
		    "invalid boolean value for %s", info[i]); \
		break; \
	} \
	break; \
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "command info from plugin:");
    for (i = 0; info[i] != NULL; i++) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "    %zu: %s", i, info[i]);
	switch (info[i][0]) {
	    case 'a':
		SET_STRING("apparmor_profile=", apparmor_profile);
		break;
	    case 'c':
		SET_STRING("chroot=", chroot)
		SET_STRING("command=", command)
		SET_STRING("cwd=", runcwd)
		SET_FLAG("cwd_optional=", CD_CWD_OPTIONAL)
		if (strncmp("closefrom=", info[i], sizeof("closefrom=") - 1) == 0) {
		    cp = info[i] + sizeof("closefrom=") - 1;
		    details->closefrom =
			(int)sudo_strtonum(cp, 0, INT_MAX, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    break;
		}
		break;
	    case 'e':
		SET_FLAG("exec_background=", CD_EXEC_BG)
		if (strncmp("execfd=", info[i], sizeof("execfd=") - 1) == 0) {
		    cp = info[i] + sizeof("execfd=") - 1;
		    details->execfd =
			(int)sudo_strtonum(cp, 0, INT_MAX, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
#ifdef HAVE_FEXECVE
		    /* Must keep fd open during exec. */
		    add_preserved_fd(&details->preserved_fds, details->execfd);
		    SET(details->flags, CD_FEXECVE);
#else
		    /* Plugin thinks we support fexecve() but we don't. */
		    (void)fcntl(details->execfd, F_SETFD, FD_CLOEXEC);
		    details->execfd = -1;
#endif
		    break;
		}
		break;
	    case 'i':
		SET_FLAG("intercept=", CD_INTERCEPT)
		SET_FLAG("intercept_verify=", CD_INTERCEPT_VERIFY)
		break;
	    case 'l':
		SET_STRING("login_class=", login_class)
		SET_FLAG("log_subcmds=", CD_LOG_SUBCMDS)
		break;
	    case 'n':
		if (strncmp("nice=", info[i], sizeof("nice=") - 1) == 0) {
		    cp = info[i] + sizeof("nice=") - 1;
		    details->priority = (int)sudo_strtonum(cp, INT_MIN, INT_MAX,
			&errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    SET(details->flags, CD_SET_PRIORITY);
		    break;
		}
		SET_FLAG("noexec=", CD_NOEXEC)
		break;
	    case 'p':
		SET_FLAG("preserve_groups=", CD_PRESERVE_GROUPS)
		if (strncmp("preserve_fds=", info[i], sizeof("preserve_fds=") - 1) == 0) {
		    parse_preserved_fds(&details->preserved_fds,
			info[i] + sizeof("preserve_fds=") - 1);
		    break;
		}
		break;
	    case 'r':
		if (strncmp("rlimit_", info[i], sizeof("rlimit_") - 1) == 0) {
		    parse_policy_rlimit(info[i] + sizeof("rlimit_") - 1);
		    break;
		}
		if (strncmp("runas_egid=", info[i], sizeof("runas_egid=") - 1) == 0) {
		    cp = info[i] + sizeof("runas_egid=") - 1;
		    id = sudo_strtoid(cp, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    details->cred.egid = (gid_t)id;
		    SET(details->flags, CD_SET_EGID);
		    break;
		}
		if (strncmp("runas_euid=", info[i], sizeof("runas_euid=") - 1) == 0) {
		    cp = info[i] + sizeof("runas_euid=") - 1;
		    id = sudo_strtoid(cp, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    details->cred.euid = (uid_t)id;
		    SET(details->flags, CD_SET_EUID);
		    break;
		}
		if (strncmp("runas_gid=", info[i], sizeof("runas_gid=") - 1) == 0) {
		    cp = info[i] + sizeof("runas_gid=") - 1;
		    id = sudo_strtoid(cp, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    details->cred.gid = (gid_t)id;
		    SET(details->flags, CD_SET_GID);
		    break;
		}
		if (strncmp("runas_groups=", info[i], sizeof("runas_groups=") - 1) == 0) {
		    cp = info[i] + sizeof("runas_groups=") - 1;
		    details->cred.ngroups = sudo_parse_gids(cp, NULL,
			&details->cred.groups);
		    /* sudo_parse_gids() will print a warning on error. */
		    if (details->cred.ngroups == -1)
			exit(EXIT_FAILURE); /* XXX */
		    if (!gc_add(GC_PTR, details->cred.groups)) {
			sudo_fatalx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
		    }
		    break;
		}
		if (strncmp("runas_uid=", info[i], sizeof("runas_uid=") - 1) == 0) {
		    cp = info[i] + sizeof("runas_uid=") - 1;
		    id = sudo_strtoid(cp, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    details->cred.uid = (uid_t)id;
		    SET(details->flags, CD_SET_UID);
		    break;
		}
#ifdef HAVE_PRIV_SET
		if (strncmp("runas_privs=", info[i], sizeof("runas_privs=") - 1) == 0) {
                    const char *endp;
		    cp = info[i] + sizeof("runas_privs=") - 1;
	            if (*cp != '\0') {
			details->privs = priv_str_to_set(cp, ",", &endp);
			if (details->privs == NULL)
			    sudo_warn("invalid runas_privs %s", endp);
		    }
		    break;
		}
		if (strncmp("runas_limitprivs=", info[i], sizeof("runas_limitprivs=") - 1) == 0) {
                    const char *endp;
		    cp = info[i] + sizeof("runas_limitprivs=") - 1;
	            if (*cp != '\0') {
			details->limitprivs = priv_str_to_set(cp, ",", &endp);
			if (details->limitprivs == NULL)
			    sudo_warn("invalid runas_limitprivs %s", endp);
		    }
		    break;
		}
#endif /* HAVE_PRIV_SET */
		SET_STRING("runas_user=", runas_user)
		break;
	    case 's':
		SET_STRING("selinux_role=", selinux_role)
		SET_STRING("selinux_type=", selinux_type)
		SET_FLAG("set_utmp=", CD_SET_UTMP)
		SET_FLAG("sudoedit=", CD_SUDOEDIT)
		SET_FLAG("sudoedit_checkdir=", CD_SUDOEDIT_CHECKDIR)
		SET_FLAG("sudoedit_follow=", CD_SUDOEDIT_FOLLOW)
		if (strncmp("sudoedit_nfiles=", info[i], sizeof("sudoedit_nfiles=") - 1) == 0) {
		    cp = info[i] + sizeof("sudoedit_nfiles=") - 1;
		    details->nfiles = (int)sudo_strtonum(cp, 1, INT_MAX,
			&errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    break;
		}
		break;
	    case 't':
		if (strncmp("timeout=", info[i], sizeof("timeout=") - 1) == 0) {
		    cp = info[i] + sizeof("timeout=") - 1;
		    details->timeout =
			(unsigned int)sudo_strtonum(cp, 0, UINT_MAX, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    SET(details->flags, CD_SET_TIMEOUT);
		    break;
		}
		break;
	    case 'u':
		if (strncmp("umask=", info[i], sizeof("umask=") - 1) == 0) {
		    cp = info[i] + sizeof("umask=") - 1;
		    details->umask = sudo_strtomode(cp, &errstr);
		    if (errstr != NULL)
			sudo_fatalx(U_("%s: %s"), info[i], U_(errstr));
		    SET(details->flags, CD_SET_UMASK);
		    break;
		}
		SET_FLAG("umask_override=", CD_OVERRIDE_UMASK)
		SET_FLAG("use_ptrace=", CD_USE_PTRACE)
		SET_FLAG("use_pty=", CD_USE_PTY)
		SET_STRING("utmp_user=", utmp_user)
		break;
	}
    }

    /* Only use ptrace(2) for intercept/log_subcmds if supported. */
    exec_ptrace_fix_flags(details);

    if (!ISSET(details->flags, CD_SET_EUID))
	details->cred.euid = details->cred.uid;
    if (!ISSET(details->flags, CD_SET_EGID))
	details->cred.egid = details->cred.gid;
    if (!ISSET(details->flags, CD_SET_UMASK))
	CLR(details->flags, CD_OVERRIDE_UMASK);

#ifdef HAVE_SETAUTHDB
    aix_setauthdb(IDtouser(details->cred.euid), NULL);
#endif
    if (details->runas_user != NULL)
	details->pw = getpwnam(details->runas_user);
    if (details->pw == NULL)
	details->pw = getpwuid(details->cred.euid);
#ifdef HAVE_SETAUTHDB
    aix_restoreauthdb();
#endif
    if (details->pw != NULL && (details->pw = pw_dup(details->pw)) == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (!gc_add(GC_PTR, details->pw))
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

#ifdef HAVE_SELINUX
    if (details->selinux_role != NULL && is_selinux_enabled() > 0) {
	SET(details->flags, CD_RBAC_ENABLED);
	if (selinux_getexeccon(details->selinux_role, details->selinux_type) != 0)
	    exit(EXIT_FAILURE);
    }
#endif

#ifdef HAVE_APPARMOR
    if (details->apparmor_profile != NULL && apparmor_is_enabled()) {
	if (apparmor_prepare(details->apparmor_profile) != 0)
	    exit(EXIT_FAILURE);
    }
#endif

    debug_return;
}

static void
sudo_check_suid(const char *sudo)
{
    char pathbuf[PATH_MAX];
    struct stat sb;
    bool qualified;
    debug_decl(sudo_check_suid, SUDO_DEBUG_PCOMM);

    if (geteuid() != ROOT_UID) {
#if defined(__linux__) && defined(PR_GET_NO_NEW_PRIVS)
	/* The no_new_privs flag disables set-user-ID at execve(2) time. */
	if (prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0) == 1) {
	    sudo_warnx("%s", U_("The \"no new privileges\" flag is set, which "
		"prevents sudo from running as root."));
	    sudo_warnx("%s", U_("If sudo is running in a container, you may need"
		" to adjust the container configuration to disable the flag."));
	    exit(EXIT_FAILURE);
	}
#endif /* __linux__ && PR_GET_NO_NEW_PRIVS */

	/* Search for sudo binary in PATH if not fully qualified. */
	qualified = strchr(sudo, '/') != NULL;
	if (!qualified) {
	    char *path = getenv_unhooked("PATH");
	    if (path != NULL) {
		const char *cp, *ep;
		const char *pathend = path + strlen(path);

		for (cp = sudo_strsplit(path, pathend, ":", &ep); cp != NULL;
		    cp = sudo_strsplit(NULL, pathend, ":", &ep)) {

		    int len = snprintf(pathbuf, sizeof(pathbuf), "%.*s/%s",
			(int)(ep - cp), cp, sudo);
		    if (len < 0 || len >= ssizeof(pathbuf))
			continue;
		    if (access(pathbuf, X_OK) == 0) {
			sudo = pathbuf;
			qualified = true;
			break;
		    }
		}
	    }
	}

	if (qualified && stat(sudo, &sb) == 0) {
	    /* Try to determine why sudo was not running as root. */
	    if (sb.st_uid != ROOT_UID || !ISSET(sb.st_mode, S_ISUID)) {
		sudo_fatalx(
		    U_("%s must be owned by uid %d and have the setuid bit set"),
		    sudo, ROOT_UID);
	    } else {
		sudo_fatalx(U_("effective uid is not %d, is %s on a file system "
		    "with the 'nosuid' option set or an NFS file system without"
		    " root privileges?"), ROOT_UID, sudo);
	    }
	} else {
	    sudo_fatalx(
		U_("effective uid is not %d, is sudo installed setuid root?"),
		ROOT_UID);
	}
    }
    debug_return;
}

bool
set_user_groups(struct command_details *details)
{
    bool ret = false;
    debug_decl(set_user_groups, SUDO_DEBUG_EXEC);

    if (!ISSET(details->flags, CD_PRESERVE_GROUPS)) {
	if (details->cred.ngroups >= 0) {
	    if (sudo_setgroups(details->cred.ngroups, details->cred.groups) < 0) {
		sudo_warn("%s", U_("unable to set supplementary group IDs"));
		goto done;
	    }
	}
    }
#ifdef HAVE_SETEUID
    if (ISSET(details->flags, CD_SET_EGID) && setegid(details->cred.egid)) {
	sudo_warn(U_("unable to set effective gid to runas gid %u"),
	    (unsigned int)details->cred.egid);
	goto done;
    }
#endif
    if (ISSET(details->flags, CD_SET_GID) && setgid(details->cred.gid)) {
	sudo_warn(U_("unable to set gid to runas gid %u"),
	    (unsigned int)details->cred.gid);
	goto done;
    }
    ret = true;

done:
    CLR(details->flags, CD_SET_GROUPS);
    debug_return_bool(ret);
}

/*
 * Run the command and wait for it to complete.
 * Returns wait status suitable for use with the wait(2) macros.
 */
int
run_command(struct command_details *command_details,
    const struct user_details *user_details)
{
    struct command_status cstat;
    int status = W_EXITCODE(1, 0);
    debug_decl(run_command, SUDO_DEBUG_EXEC);

    cstat.type = CMD_INVALID;
    cstat.val = 0;

    if (command_details->command == NULL) {
	sudo_warnx("%s", U_("command not set by the security policy"));
	debug_return_int(status);
    }
    if (command_details->argv == NULL) {
	sudo_warnx("%s", U_("argv not set by the security policy"));
	debug_return_int(status);
    }
    if (command_details->envp == NULL) {
	sudo_warnx("%s", U_("envp not set by the security policy"));
	debug_return_int(status);
    }

    sudo_execute(command_details, user_details, sudo_event_base, &cstat);

    switch (cstat.type) {
    case CMD_ERRNO:
	/* exec_setup() or execve() returned an error. */
	iolog_close(0, cstat.val);
	policy_close(command_details->command, 0, cstat.val);
	audit_close(SUDO_PLUGIN_EXEC_ERROR, cstat.val);
	break;
    case CMD_WSTATUS:
	/* Command ran, exited or was killed. */
	status = cstat.val;
	iolog_close(status, 0);
	policy_close(command_details->command, status, 0);
	audit_close(SUDO_PLUGIN_WAIT_STATUS, cstat.val);
	break;
    default:
	/* TODO: handle front end error conditions. */
	sudo_warnx(U_("unexpected child termination condition: %d"), cstat.type);
	break;
    }
    debug_return_int(status);
}

/*
 * Format struct sudo_settings as name=value pairs for the plugin
 * to consume.  Returns a NULL-terminated plugin-style array of pairs.
 */
static char **
format_plugin_settings(struct plugin_container *plugin)
{
    size_t plugin_settings_size;
    struct sudo_debug_file *debug_file;
    struct sudo_settings *setting;
    char **plugin_settings;
    size_t i = 0;
    debug_decl(format_plugin_settings, SUDO_DEBUG_PCOMM);

    /* We update the ticket entry by default. */
    if (sudo_settings[ARG_IGNORE_TICKET].value == NULL &&
	    sudo_settings[ARG_UPDATE_TICKET].value == NULL) {
	sudo_settings[ARG_UPDATE_TICKET].value = "true";
    }

    /* Determine sudo_settings array size (including plugin_path and NULL) */
    plugin_settings_size = 2;
    for (setting = sudo_settings; setting->name != NULL; setting++)
	plugin_settings_size++;
    if (plugin->debug_files != NULL) {
	TAILQ_FOREACH(debug_file, plugin->debug_files, entries)
	    plugin_settings_size++;
    }

    /* Allocate and fill in. */
    plugin_settings = reallocarray(NULL, plugin_settings_size, sizeof(char *));
    if (plugin_settings == NULL)
	goto bad;
    plugin_settings[i] = sudo_new_key_val("plugin_path", plugin->path);
    if (plugin_settings[i] == NULL)
	goto bad;
    for (setting = sudo_settings; setting->name != NULL; setting++) {
        if (setting->value != NULL) {
            sudo_debug_printf(SUDO_DEBUG_INFO, "settings: %s=%s",
                setting->name, setting->value);
	    plugin_settings[++i] =
		sudo_new_key_val(setting->name, setting->value);
	    if (plugin_settings[i] == NULL)
		goto bad;
        }
    }
    if (plugin->debug_files != NULL) {
	TAILQ_FOREACH(debug_file, plugin->debug_files, entries) {
	    /* XXX - quote filename? */
	    if (asprintf(&plugin_settings[++i], "debug_flags=%s %s",
		debug_file->debug_file, debug_file->debug_flags) == -1)
		goto bad;
	}
    }
    plugin_settings[++i] = NULL;

    /* Add to list of vectors to be garbage collected at exit. */
    if (!gc_add(GC_VECTOR, plugin_settings))
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    debug_return_ptr(plugin_settings);
bad:
    while (i)
	free(plugin_settings[--i]);
    free(plugin_settings);
    debug_return_ptr(NULL);
}

static void
policy_open(void)
{
    char **plugin_settings;
    const char *errstr = NULL;
    int ok;
    debug_decl(policy_open, SUDO_DEBUG_PCOMM);

    /* Convert struct sudo_settings to plugin_settings[] */
    plugin_settings = format_plugin_settings(&policy_plugin);
    if (plugin_settings == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    /*
     * Backward compatibility for older API versions
     */
    sudo_debug_set_active_instance(SUDO_DEBUG_INSTANCE_INITIALIZER);
    switch (policy_plugin.u.generic->version) {
    case SUDO_API_MKVERSION(1, 0):
    case SUDO_API_MKVERSION(1, 1):
	ok = policy_plugin.u.policy_1_0->open(policy_plugin.u.io_1_0->version,
	    sudo_conversation_1_7, sudo_conversation_printf, plugin_settings,
	    user_info, submit_envp);
	break;
    default:
	ok = policy_plugin.u.policy->open(SUDO_API_VERSION, sudo_conversation,
	    sudo_conversation_printf, plugin_settings, user_info, submit_envp,
	    policy_plugin.options, &errstr);
    }

    /* Stash plugin debug instance ID if set in open() function. */
    policy_plugin.debug_instance = sudo_debug_get_active_instance();
    sudo_debug_set_active_instance(sudo_debug_instance);

    if (ok != 1) {
	    if (ok == -2)
		usage();
	    /* XXX - audit */
	    sudo_fatalx("%s", U_("unable to initialize policy plugin"));
    }

    debug_return;
}

static void
policy_close(const char *cmnd, int exit_status, int error_code)
{
    debug_decl(policy_close, SUDO_DEBUG_PCOMM);

    if (error_code != 0) {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s: calling policy close with errno %d",
	    policy_plugin.name, error_code);
    } else {
	sudo_debug_printf(SUDO_DEBUG_DEBUG,
	    "%s: calling policy close with wait status %d",
	    policy_plugin.name, exit_status);
    }
    if (policy_plugin.u.policy->close != NULL) {
	sudo_debug_set_active_instance(policy_plugin.debug_instance);
	policy_plugin.u.policy->close(exit_status, error_code);
	sudo_debug_set_active_instance(sudo_debug_instance);
    } else if (error_code != 0) {
	if (cmnd != NULL) {
	    errno = error_code;
	    sudo_warn(U_("unable to execute %s"), cmnd);
	}
    }

    debug_return;
}

static int
policy_show_version(int verbose)
{
    int ret = true;
    debug_decl(policy_show_version, SUDO_DEBUG_PCOMM);

    sudo_debug_set_active_instance(policy_plugin.debug_instance);
    if (policy_plugin.u.policy->show_version != NULL)
	ret = policy_plugin.u.policy->show_version(verbose);
    if (policy_plugin.u.policy->version >= SUDO_API_MKVERSION(1, 15)) {
	if (policy_plugin.u.policy->close != NULL)
	    policy_plugin.u.policy->close(0, 0);
    }
    sudo_debug_set_active_instance(sudo_debug_instance);

    debug_return_int(ret);
}

static bool
policy_check(int argc, char * const argv[], char *env_add[],
    char **command_info[], char **run_argv[], char **run_envp[])
{
    const char *errstr = NULL;
    int ok;
    debug_decl(policy_check, SUDO_DEBUG_PCOMM);

    if (policy_plugin.u.policy->check_policy == NULL) {
	sudo_fatalx(U_("policy plugin %s is missing the \"check_policy\" method"),
	    policy_plugin.name);
    }
    sudo_debug_set_active_instance(policy_plugin.debug_instance);
    ok = policy_plugin.u.policy->check_policy(argc, argv, env_add,
	command_info, run_argv, run_envp, &errstr);
    sudo_debug_set_active_instance(sudo_debug_instance);
    sudo_debug_printf(SUDO_DEBUG_INFO, "policy plugin returns %d (%s)",
	ok, errstr ? errstr : "");

    /* On success, the close method will be called by sudo_edit/run_command. */
    if (ok != 1) {
	switch (ok) {
	case 0:
	    audit_reject(policy_plugin.name, SUDO_POLICY_PLUGIN,
		errstr ? errstr : _("command rejected by policy"),
		*command_info);
	    break;
	case -1:
	    audit_error(policy_plugin.name, SUDO_POLICY_PLUGIN,
		errstr ? errstr : _("policy plugin error"),
		*command_info);
	    break;
	case -2:
	    usage();
	    /* NOTREACHED */
	}
	debug_return_bool(false);
    }
    debug_return_bool(audit_accept(policy_plugin.name, SUDO_POLICY_PLUGIN,
	*command_info, *run_argv, *run_envp));
}

sudo_noreturn static void
policy_list(int argc, char * const argv[], int verbose, const char *user)
{
    const char *errstr = NULL;
    /* TODO: add list_user */
    const char * const command_info[] = {
	"command=list",
	NULL
    };
    int ok;
    debug_decl(policy_list, SUDO_DEBUG_PCOMM);

    if (policy_plugin.u.policy->list == NULL) {
	sudo_fatalx(U_("policy plugin %s does not support listing privileges"),
	    policy_plugin.name);
    }
    sudo_debug_set_active_instance(policy_plugin.debug_instance);
    ok = policy_plugin.u.policy->list(argc, argv, verbose, user, &errstr);
    sudo_debug_set_active_instance(sudo_debug_instance);

    switch (ok) {
    case 1:
	audit_accept(policy_plugin.name, SUDO_POLICY_PLUGIN,
	    (char **)command_info, argv, submit_envp);
	break;
    case 0:
	audit_reject(policy_plugin.name, SUDO_POLICY_PLUGIN,
	    errstr ? errstr : _("command rejected by policy"),
	    (char **)command_info);
	break;
    default:
	audit_error(policy_plugin.name, SUDO_POLICY_PLUGIN,
	    errstr ? errstr : _("policy plugin error"),
	    (char **)command_info);
	break;
    }

    /* Policy must be closed after auditing to avoid use after free. */
    if (policy_plugin.u.policy->version >= SUDO_API_MKVERSION(1, 15))
	policy_close(NULL, 0, 0);
    audit_close(SUDO_PLUGIN_NO_STATUS, 0);

    exit(ok != 1);
}

sudo_noreturn static void
policy_validate(char * const argv[])
{
    const char *errstr = NULL;
    const char * const command_info[] = {
	"command=validate",
	NULL
    };
    int ok = 0;
    debug_decl(policy_validate, SUDO_DEBUG_PCOMM);

    if (policy_plugin.u.policy->validate == NULL) {
	sudo_fatalx(U_("policy plugin %s does not support the -v option"),
	    policy_plugin.name);
    }
    sudo_debug_set_active_instance(policy_plugin.debug_instance);
    ok = policy_plugin.u.policy->validate(&errstr);
    sudo_debug_set_active_instance(sudo_debug_instance);

    switch (ok) {
    case 1:
	audit_accept(policy_plugin.name, SUDO_POLICY_PLUGIN,
	    (char **)command_info, argv, submit_envp);
	break;
    case 0:
	audit_reject(policy_plugin.name, SUDO_POLICY_PLUGIN,
	    errstr ? errstr : _("command rejected by policy"),
	    (char **)command_info);
	break;
    default:
	audit_error(policy_plugin.name, SUDO_POLICY_PLUGIN,
	    errstr ? errstr : _("policy plugin error"),
	    (char **)command_info);
	break;
    }

    /* Policy must be closed after auditing to avoid use after free. */
    if (policy_plugin.u.policy->version >= SUDO_API_MKVERSION(1, 15))
	policy_close(NULL, 0, 0);
    audit_close(SUDO_PLUGIN_NO_STATUS, 0);

    exit(ok != 1);
}

sudo_noreturn static void
policy_invalidate(int unlinkit)
{
    debug_decl(policy_invalidate, SUDO_DEBUG_PCOMM);

    if (policy_plugin.u.policy->invalidate == NULL) {
	sudo_fatalx(U_("policy plugin %s does not support the -k/-K options"),
	    policy_plugin.name);
    }
    sudo_debug_set_active_instance(policy_plugin.debug_instance);
    policy_plugin.u.policy->invalidate(unlinkit);
    if (policy_plugin.u.policy->version >= SUDO_API_MKVERSION(1, 15)) {
	if (policy_plugin.u.policy->close != NULL)
	    policy_plugin.u.policy->close(0, 0);
    }
    sudo_debug_set_active_instance(sudo_debug_instance);

    audit_close(SUDO_PLUGIN_NO_STATUS, 0);

    exit(EXIT_SUCCESS);
}

int
policy_init_session(struct command_details *details)
{
    const char *errstr = NULL;
    int ret = true;
    debug_decl(policy_init_session, SUDO_DEBUG_PCOMM);

    /*
     * We set groups, including supplementary group vector,
     * as part of the session setup.  This allows for dynamic
     * groups to be set via pam_group(8) in pam_setcred(3).
     */
    if (ISSET(details->flags, CD_SET_GROUPS)) {
	/* set_user_groups() prints error message on failure. */
	if (!set_user_groups(details))
	    goto done;
    }

    /* Session setup may override sudoers umask so set it first. */
    if (ISSET(details->flags, CD_SET_UMASK))
	(void) umask(details->umask);

    if (policy_plugin.u.policy->init_session) {
	/*
	 * Backward compatibility for older API versions
	 */
	sudo_debug_set_active_instance(policy_plugin.debug_instance);
	switch (policy_plugin.u.generic->version) {
	case SUDO_API_MKVERSION(1, 0):
	case SUDO_API_MKVERSION(1, 1):
	    ret = policy_plugin.u.policy_1_0->init_session(details->pw);
	    break;
	default:
	    ret = policy_plugin.u.policy->init_session(details->pw,
		&details->envp, &errstr);
	}
	sudo_debug_set_active_instance(sudo_debug_instance);
	if (ret != 1) {
	    audit_error(policy_plugin.name, SUDO_POLICY_PLUGIN,
		errstr ? errstr : _("policy plugin error"),
		details->info);
	}
    }

done:
    debug_return_int(ret);
}

static int
iolog_open_int(struct plugin_container *plugin, char * const command_info[],
    int argc, char * const argv[], char * const run_envp[], const char **errstr)
{
    char **plugin_settings;
    int ret;
    debug_decl(iolog_open_int, SUDO_DEBUG_PCOMM);

    /* Convert struct sudo_settings to plugin_settings[] */
    plugin_settings = format_plugin_settings(plugin);
    if (plugin_settings == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }

    /*
     * Backward compatibility for older API versions
     */
    sudo_debug_set_active_instance(plugin->debug_instance);
    switch (plugin->u.generic->version) {
    case SUDO_API_MKVERSION(1, 0):
	ret = plugin->u.io_1_0->open(plugin->u.io_1_0->version,
	    sudo_conversation_1_7, sudo_conversation_printf, plugin_settings,
	    user_info, argc, argv, run_envp);
	break;
    case SUDO_API_MKVERSION(1, 1):
	ret = plugin->u.io_1_1->open(plugin->u.io_1_1->version,
	    sudo_conversation_1_7, sudo_conversation_printf, plugin_settings,
	    user_info, command_info, argc, argv, run_envp);
	break;
    default:
	ret = plugin->u.io->open(SUDO_API_VERSION, sudo_conversation,
	    sudo_conversation_printf, plugin_settings, user_info, command_info,
	    argc, argv, run_envp, plugin->options, errstr);
    }

    /* Stash plugin debug instance ID if set in open() function. */
    plugin->debug_instance = sudo_debug_get_active_instance();
    sudo_debug_set_active_instance(sudo_debug_instance);

    debug_return_int(ret);
}

static bool
iolog_open(char * const command_info[], int argc, char * const argv[],
    char * const run_envp[])
{
    struct plugin_container *plugin, *next;
    const char *errstr = NULL;
    debug_decl(iolog_open, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH_SAFE(plugin, &io_plugins, entries, next) {
	int ok = iolog_open_int(plugin, command_info, argc, argv, run_envp,
	    &errstr);
	switch (ok) {
	case 1:
	    break;
	case 0:
	    /* I/O plugin asked to be disabled, remove and free. */
	    unlink_plugin(&io_plugins, plugin);
	    break;
	case -2:
	    usage();
	    /* NOTREACHED */
	default:
	    sudo_warnx(U_("error initializing I/O plugin %s"),
		plugin->name);
	    audit_error(plugin->name, SUDO_IO_PLUGIN,
		errstr ? errstr : _("error initializing I/O plugin"),
		command_info);
	    debug_return_bool(false);
	}
    }

    debug_return_bool(true);
}

static void
iolog_close(int exit_status, int error_code)
{
    struct plugin_container *plugin;
    debug_decl(iolog_close, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	if (plugin->u.io->close != NULL) {
	    if (error_code != 0) {
		sudo_debug_printf(SUDO_DEBUG_DEBUG,
		    "%s: calling I/O close with errno %d",
		    plugin->name, error_code);
	    } else {
		sudo_debug_printf(SUDO_DEBUG_DEBUG,
		    "%s: calling I/O close with wait status %d",
			plugin->name, exit_status);
	    }
	    sudo_debug_set_active_instance(plugin->debug_instance);
	    plugin->u.io->close(exit_status, error_code);
	    sudo_debug_set_active_instance(sudo_debug_instance);
	}
    }

    debug_return;
}

static void
iolog_show_version(int verbose, int argc, char * const argv[],
    char * const envp[])
{
    const char *errstr = NULL;
    struct plugin_container *plugin;
    debug_decl(iolog_show_version, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH(plugin, &io_plugins, entries) {
	int ok = iolog_open_int(plugin, NULL, argc, argv, envp, &errstr);
	if (ok != -1) {
	    sudo_debug_set_active_instance(plugin->debug_instance);
	    if (plugin->u.io->show_version != NULL) {
		/* Return value of show_version currently ignored. */
		plugin->u.io->show_version(verbose);
	    }
	    if (plugin->u.io->version >= SUDO_API_MKVERSION(1, 15)) {
		if (plugin->u.io->close != NULL)
		    plugin->u.io->close(0, 0);
	    }
	    sudo_debug_set_active_instance(sudo_debug_instance);
	}
    }

    debug_return;
}

/*
 * Remove the specified plugin from the plugins list.
 * Deregisters any hooks before unlinking, then frees the container.
 */
static void
unlink_plugin(struct plugin_container_list *plugin_list,
    struct plugin_container *plugin)
{
    void (*deregister_hooks)(int , int (*)(struct sudo_hook *)) = NULL;
    debug_decl(unlink_plugin, SUDO_DEBUG_PCOMM);

    /* Deregister hooks, if any. */
    if (plugin->u.generic->version >= SUDO_API_MKVERSION(1, 2)) {
	switch (plugin->u.generic->type) {
	case SUDO_IO_PLUGIN:
	    deregister_hooks = plugin->u.io->deregister_hooks;
	    break;
	case SUDO_AUDIT_PLUGIN:
	    deregister_hooks = plugin->u.audit->deregister_hooks;
	    break;
	default:
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"%s: unsupported plugin type %d", __func__,
		plugin->u.generic->type);
	    break;
	}
    }
    if (deregister_hooks != NULL) {
	sudo_debug_set_active_instance(plugin->debug_instance);
	deregister_hooks(SUDO_HOOK_VERSION, deregister_hook);
	sudo_debug_set_active_instance(sudo_debug_instance);
    }

    /* Remove from plugin list and free. */
    TAILQ_REMOVE(plugin_list, plugin, entries);
    free_plugin_container(plugin, true);

    debug_return;
}

static int
audit_open_int(struct plugin_container *plugin, const char **errstr)
{
    char **plugin_settings;
    int ret;
    debug_decl(audit_open_int, SUDO_DEBUG_PCOMM);

    /* Convert struct sudo_settings to plugin_settings[] */
    plugin_settings = format_plugin_settings(plugin);
    if (plugin_settings == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }

    sudo_debug_set_active_instance(plugin->debug_instance);
    ret = plugin->u.audit->open(SUDO_API_VERSION, sudo_conversation,
	sudo_conversation_printf, plugin_settings, user_info,
	submit_optind, submit_argv, submit_envp, plugin->options, errstr);

    /* Stash plugin debug instance ID if set in open() function. */
    plugin->debug_instance = sudo_debug_get_active_instance();
    sudo_debug_set_active_instance(sudo_debug_instance);

    debug_return_int(ret);
}

static void
audit_open(void)
{
    struct plugin_container *plugin, *next;
    const char *errstr = NULL;
    debug_decl(audit_open, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH_SAFE(plugin, &audit_plugins, entries, next) {
	int ok = audit_open_int(plugin, &errstr);
	switch (ok) {
	case 1:
	    break;
	case 0:
	    /* Audit plugin asked to be disabled, remove and free. */
	    unlink_plugin(&audit_plugins, plugin);
	    break;
	case -2:
	    usage();
	    /* NOTREACHED */
	default:
	    /* TODO: pass error message to other audit plugins */
	    sudo_fatalx(U_("error initializing audit plugin %s"),
		plugin->name);
	}
    }

    debug_return;
}

static void
audit_close(int status_type, int status)
{
    struct plugin_container *plugin;
    debug_decl(audit_close, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH(plugin, &audit_plugins, entries) {
	if (plugin->u.audit->close != NULL) {
	    sudo_debug_set_active_instance(plugin->debug_instance);
	    plugin->u.audit->close(status_type, status);
	    sudo_debug_set_active_instance(sudo_debug_instance);
	}
    }

    debug_return;
}

static void
audit_show_version(int verbose)
{
    struct plugin_container *plugin;
    debug_decl(audit_show_version, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH(plugin, &audit_plugins, entries) {
	sudo_debug_set_active_instance(plugin->debug_instance);
	if (plugin->u.audit->show_version != NULL) {
	    /* Return value of show_version currently ignored. */
	    plugin->u.audit->show_version(verbose);
	}
	if (plugin->u.audit->close != NULL)
	    plugin->u.audit->close(SUDO_PLUGIN_NO_STATUS, 0);
	sudo_debug_set_active_instance(sudo_debug_instance);
    }

    debug_return;
}

/*
 * Error from plugin or front-end.
 * The error will not be sent to plugin source, if specified.
 */
static bool
audit_error2(struct plugin_container *source, const char *plugin_name,
    unsigned int plugin_type, const char *audit_msg, char * const command_info[])
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    bool ret = true;
    int ok;
    debug_decl(audit_error2, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH(plugin, &audit_plugins, entries) {
	if (plugin->u.audit->error == NULL)
	    continue;

	/* Avoid a loop if the audit plugin itself has an error. */
	if (plugin == source)
	    continue;

	sudo_debug_set_active_instance(plugin->debug_instance);
	ok = plugin->u.audit->error(plugin_name, plugin_type,
	    audit_msg, command_info, &errstr);
	sudo_debug_set_active_instance(sudo_debug_instance);
	if (ok != 1) {
	    /*
	     * Don't propagate the error to other audit plugins.
	     * It is not worth the trouble to avoid potential loops.
	     */
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"%s: plugin %s error failed, ret %d", __func__,
		plugin->name, ok);
	    sudo_warnx(U_("%s: unable to log error event%s%s"),
		plugin->name, errstr ? ": " : "", errstr ? errstr : "");
	    ret = false;
	}
    }

    debug_return_bool(ret);
}

/*
 * Command accepted by policy.
 * See command_info[] for additional info.
 * XXX - actual environment may be updated by policy_init_session().
 */
bool
audit_accept(const char *plugin_name, unsigned int plugin_type,
    char * const command_info[], char * const run_argv[],
    char * const run_envp[])
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    int ok;
    debug_decl(audit_accept, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH(plugin, &audit_plugins, entries) {
	if (plugin->u.audit->accept == NULL)
	    continue;

	sudo_debug_set_active_instance(plugin->debug_instance);
	ok = plugin->u.audit->accept(plugin_name, plugin_type,
	    command_info, run_argv, run_envp, &errstr);
	sudo_debug_set_active_instance(sudo_debug_instance);
	if (ok != 1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"%s: plugin %s accept failed, ret %d", __func__,
		plugin->name, ok);
	    sudo_warnx(U_("%s: unable to log accept event%s%s"),
		plugin->name, errstr ? ": " : "", errstr ? errstr : "");

	    /* Notify other audit plugins and return. */
	    audit_error2(plugin, plugin->name, SUDO_AUDIT_PLUGIN,
		errstr ? errstr : _("audit plugin error"), command_info);
	    debug_return_bool(false);
	}
    }

    debug_return_bool(true);
}

/*
 * Command rejected by policy or I/O plugin.
 */
bool
audit_reject(const char *plugin_name, unsigned int plugin_type,
    const char *audit_msg, char * const command_info[])
{
    struct plugin_container *plugin;
    const char *errstr = NULL;
    bool ret = true;
    int ok;
    debug_decl(audit_reject, SUDO_DEBUG_PCOMM);

    TAILQ_FOREACH(plugin, &audit_plugins, entries) {
	if (plugin->u.audit->reject == NULL)
	    continue;

	sudo_debug_set_active_instance(plugin->debug_instance);
	ok = plugin->u.audit->reject(plugin_name, plugin_type,
	    audit_msg, command_info, &errstr);
	sudo_debug_set_active_instance(sudo_debug_instance);
	if (ok != 1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR,
		"%s: plugin %s reject failed, ret %d", __func__,
		plugin->name, ok);
	    sudo_warnx(U_("%s: unable to log reject event%s%s"),
		plugin->name, errstr ? ": " : "", errstr ? errstr : "");

	    /* Notify other audit plugins. */
	    audit_error2(plugin, plugin->name, SUDO_AUDIT_PLUGIN,
		errstr ? errstr : _("audit plugin error"), command_info);

	    ret = false;
	    break;
	}
    }

    debug_return_bool(ret);
}

/*
 * Error from plugin or front-end.
 */
bool
audit_error(const char *plugin_name, unsigned int plugin_type,
    const char *audit_msg, char * const command_info[])
{
    return audit_error2(NULL, plugin_name, plugin_type, audit_msg,
	command_info);
}

static int
approval_open_int(struct plugin_container *plugin)
{
    char **plugin_settings;
    const char *errstr = NULL;
    int ret;
    debug_decl(approval_open_int, SUDO_DEBUG_PCOMM);

    /* Convert struct sudo_settings to plugin_settings[] */
    plugin_settings = format_plugin_settings(plugin);
    if (plugin_settings == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    sudo_debug_set_active_instance(SUDO_DEBUG_INSTANCE_INITIALIZER);
    ret = plugin->u.approval->open(SUDO_API_VERSION, sudo_conversation,
	sudo_conversation_printf, plugin_settings, user_info, submit_optind,
	submit_argv, submit_envp, plugin->options, &errstr);

    /* Stash plugin debug instance ID if set in open() function. */
    plugin->debug_instance = sudo_debug_get_active_instance();
    sudo_debug_set_active_instance(sudo_debug_instance);

    switch (ret) {
    case 1:
	break;
    case 0:
	/* approval plugin asked to be disabled, remove and free. */
	unlink_plugin(&approval_plugins, plugin);
	break;
    case -2:
	usage();
	/* NOTREACHED */
    default:
	/* XXX - audit */
	sudo_fatalx(U_("error initializing approval plugin %s"),
	    plugin->name);
    }

    debug_return_int(ret);
}

static void
approval_show_version(int verbose)
{
    struct plugin_container *plugin, *next;
    int ok;
    debug_decl(approval_show_version, SUDO_DEBUG_PCOMM);

    /*
     * Approval plugin us only open for the life of the show_version() call.
     */
    TAILQ_FOREACH_SAFE(plugin, &approval_plugins, entries, next) {
	if (plugin->u.approval->show_version == NULL)
	    continue;

	ok = approval_open_int(plugin);
	if (ok == 1) {
	    /* Return value of show_version currently ignored. */
	    sudo_debug_set_active_instance(plugin->debug_instance);
	    plugin->u.approval->show_version(verbose);
	    if (plugin->u.approval->close != NULL)
		plugin->u.approval->close();
	    sudo_debug_set_active_instance(sudo_debug_instance);
	}
    }

    debug_return;
}

/*
 * Run approval checks (there may be more than one).
 * This is a "one-shot" plugin that has no open/close and is only
 * called if the policy plugin accepts the command first.
 */
bool
approval_check(char * const command_info[], char * const run_argv[],
    char * const run_envp[])
{
    struct plugin_container *plugin, *next;
    const char *errstr = NULL;
    int ok;
    debug_decl(approval_check, SUDO_DEBUG_PCOMM);

    /*
     * Approval plugin is only open for the life of the check() call.
     */
    TAILQ_FOREACH_SAFE(plugin, &approval_plugins, entries, next) {
	if (plugin->u.approval->check == NULL)
	    continue;

	ok = approval_open_int(plugin);
	if (ok != 1)
	    continue;

	sudo_debug_set_active_instance(plugin->debug_instance);
	ok = plugin->u.approval->check(command_info, run_argv, run_envp,
	    &errstr);
	sudo_debug_set_active_instance(sudo_debug_instance);
	sudo_debug_printf(SUDO_DEBUG_INFO, "approval plugin %s returns %d (%s)",
	    plugin->name, ok, errstr ? errstr : "");

	switch (ok) {
	case 0:
	    audit_reject(plugin->name, SUDO_APPROVAL_PLUGIN,
		errstr ? errstr : _("command rejected by approver"),
		command_info);
	    break;
	case 1:
	    if (!audit_accept(plugin->name, SUDO_APPROVAL_PLUGIN, command_info,
		    run_argv, run_envp))
		ok = -1;
	    break;
	case -1:
	    audit_error(plugin->name, SUDO_APPROVAL_PLUGIN,
		errstr ? errstr : _("approval plugin error"),
		command_info);
	    break;
	case -2:
	    usage();
	}

	/* Close approval plugin now that errstr has been consumed. */
	if (plugin->u.approval->close != NULL) {
	    sudo_debug_set_active_instance(plugin->debug_instance);
	    plugin->u.approval->close();
	    sudo_debug_set_active_instance(sudo_debug_instance);
	}

	if (ok != 1)
	    debug_return_bool(false);
    }

    debug_return_bool(true);
}

static void
plugin_event_callback(int fd, int what, void *v)
{
    struct sudo_plugin_event_int *ev_int = v;
    int old_instance;
    debug_decl(plugin_event_callback, SUDO_DEBUG_PCOMM);

    /* Run the real callback using the plugin's debug instance. */
    old_instance = sudo_debug_set_active_instance(ev_int->debug_instance);
    ev_int->callback(fd, what, ev_int->closure);
    sudo_debug_set_active_instance(old_instance);

    debug_return;
}

/*
 * Fill in a previously allocated struct sudo_plugin_event.
 */
static int
plugin_event_set(struct sudo_plugin_event *pev, int fd, int events,
    sudo_ev_callback_t callback, void *closure)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_set, SUDO_DEBUG_PCOMM);

    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    if (sudo_ev_set(&ev_int->private, fd, events, plugin_event_callback, ev_int) == -1)
	debug_return_int(-1);

    /* Stash active instance so we can restore it when callback runs. */
    ev_int->debug_instance = sudo_debug_get_active_instance();

    /* Actual user-specified callback and closure. */
    ev_int->callback = callback;
    ev_int->closure = closure;

    /* Plugin can only operate on the main event loop. */
    ev_int->private.base = sudo_event_base;

    debug_return_int(1);
}

/*
 * Add a struct sudo_plugin_event to the main event loop.
 */
static int
plugin_event_add(struct sudo_plugin_event *pev, struct timespec *timo)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_add, SUDO_DEBUG_PCOMM);

    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    if (sudo_ev_add(NULL, &ev_int->private, timo, 0) == -1)
	debug_return_int(-1);
    debug_return_int(1);
}

/*
 * Delete a struct sudo_plugin_event from the main event loop.
 */
static int
plugin_event_del(struct sudo_plugin_event *pev)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_del, SUDO_DEBUG_PCOMM);

    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    if (sudo_ev_del(NULL, &ev_int->private) == -1)
	debug_return_int(-1);
    debug_return_int(1);
}

/*
 * Get the amount of time remaining in a timeout event.
 */
static int
plugin_event_pending(struct sudo_plugin_event *pev, int events,
    struct timespec *ts)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_pending, SUDO_DEBUG_PCOMM);

    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    debug_return_int(sudo_ev_pending(&ev_int->private, events, ts));
}

/*
 * Get the file descriptor associated with an event.
 */
static int
plugin_event_fd(struct sudo_plugin_event *pev)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_fd, SUDO_DEBUG_PCOMM);

    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    debug_return_int(sudo_ev_get_fd(&ev_int->private));
}

/*
 * Break out of the event loop, killing the command if it is running.
 */
static void
plugin_event_loopbreak(struct sudo_plugin_event *pev)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_loopbreak, SUDO_DEBUG_PCOMM);

    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    sudo_ev_loopbreak(ev_int->private.base);
    debug_return;
}

/*
 * Reset the event base of a struct sudo_plugin_event.
 * The event is removed from the old base (if any) first.
 * A NULL base can be used to set the default sudo event base.
 */
static void
plugin_event_setbase(struct sudo_plugin_event *pev, void *base)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_setbase, SUDO_DEBUG_PCOMM);

    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    if (ev_int->private.base != NULL)
	sudo_ev_del(ev_int->private.base, &ev_int->private);
    ev_int->private.base = base ? base : sudo_event_base;
    debug_return;
}

/*
 * Free a struct sudo_plugin_event allocated by plugin_event_alloc().
 */
static void
plugin_event_free(struct sudo_plugin_event *pev)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_free, SUDO_DEBUG_PCOMM);

    /* The private field is first so sudo_ev_free() can free the struct. */
    ev_int = __containerof(pev, struct sudo_plugin_event_int, public);
    sudo_ev_free(&ev_int->private);

    debug_return;
}

/*
 * Allocate a struct sudo_plugin_event and fill in the public fields.
 */
struct sudo_plugin_event *
sudo_plugin_event_alloc(void)
{
    struct sudo_plugin_event_int *ev_int;
    debug_decl(plugin_event_alloc, SUDO_DEBUG_PCOMM);

    if ((ev_int = malloc(sizeof(*ev_int))) == NULL)
	debug_return_ptr(NULL);

    /* Init public fields. */
    ev_int->public.set = plugin_event_set;
    ev_int->public.add = plugin_event_add;
    ev_int->public.del = plugin_event_del;
    ev_int->public.fd = plugin_event_fd;
    ev_int->public.pending = plugin_event_pending;
    ev_int->public.setbase = plugin_event_setbase;
    ev_int->public.loopbreak = plugin_event_loopbreak;
    ev_int->public.free = plugin_event_free;

    /* Debug instance to use with the callback. */
    ev_int->debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;

    /* Clear private portion in case caller tries to use us uninitialized. */
    memset(&ev_int->private, 0, sizeof(ev_int->private));

    debug_return_ptr(&ev_int->public);
}

static void
free_plugin_container(struct plugin_container *plugin, bool ioplugin)
{
    debug_decl(free_plugin_container, SUDO_DEBUG_PLUGIN);

    free(plugin->path);
    free(plugin->name);
    if (plugin->options != NULL) {
	int i = 0;
	while (plugin->options[i] != NULL)
	    free(plugin->options[i++]);
	free(plugin->options);
    }
    if (ioplugin)
	free(plugin);

    debug_return;
}

bool
gc_add(enum sudo_gc_types type, void *v)
{
#ifdef NO_LEAKS
    struct sudo_gc_entry *gc;
    debug_decl(gc_add, SUDO_DEBUG_MAIN);

    if (v == NULL)
	debug_return_bool(false);

    gc = calloc(1, sizeof(*gc));
    if (gc == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    switch (type) {
    case GC_PTR:
	gc->u.ptr = v;
	break;
    case GC_VECTOR:
	gc->u.vec = v;
	break;
    default:
	free(gc);
	sudo_warnx("unexpected garbage type %d", type);
	debug_return_bool(false);
    }
    gc->type = type;
    SLIST_INSERT_HEAD(&sudo_gc_list, gc, entries);
    debug_return_bool(true);
#else
    return true;
#endif /* NO_LEAKS */
}

#ifdef NO_LEAKS
static void
gc_run(void)
{
    struct plugin_container *plugin;
    struct sudo_gc_entry *gc;
    char **cur;
    debug_decl(gc_run, SUDO_DEBUG_MAIN);

    /* Collect garbage. */
    while ((gc = SLIST_FIRST(&sudo_gc_list))) {
	SLIST_REMOVE_HEAD(&sudo_gc_list, entries);
	switch (gc->type) {
	case GC_PTR:
	    free(gc->u.ptr);
	    free(gc);
	    break;
	case GC_VECTOR:
	    for (cur = gc->u.vec; *cur != NULL; cur++)
		free(*cur);
	    free(gc->u.vec);
	    free(gc);
	    break;
	default:
	    sudo_warnx("unexpected garbage type %d", gc->type);
	}
    }

    /* Free plugin structs. */
    free_plugin_container(&policy_plugin, false);
    while ((plugin = TAILQ_FIRST(&io_plugins))) {
	TAILQ_REMOVE(&io_plugins, plugin, entries);
	free_plugin_container(plugin, true);
    }

    debug_return;
}
#endif /* NO_LEAKS */

static void
gc_init(void)
{
#ifdef NO_LEAKS
    atexit(gc_run);
#endif
}
