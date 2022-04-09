/*+
 * Copyright 2015 iXsystems, Inc.
 * Copyright 2021-2022 Zoe Knox <zoe@pixin.net> 
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <dirent.h>
#include <err.h>
#include <syslog.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <launch.h>
#include <jansson.h>
#include <paths.h>
#include <termios.h>
#include <libutil.h>
#include <pwd.h>
#include <bootstrap.h>
#include "vproc.h"
#include "vproc_priv.h"
#include "vproc_internal.h"

#define N(x)    ((sizeof(x)) / (sizeof(x[0])))

#define STALL_TIMEOUT	30	// Sleep N seconds after problem

static launch_data_t to_launchd(json_t *json);
static launch_data_t to_launchd_sockets(json_t *json);
static launch_data_t create_socket(json_t *json);
static void to_json_dict(const launch_data_t lval, const char *key, void *ctx);
static json_t *to_json(launch_data_t ld);
static json_t *launch_msg_json(json_t *msg);
static int load_job(const char *filename);

static int cmd_start_stop(int argc, char * const argv[]);
static int cmd_bslist(int argc, char * const argv[]);
static int cmd_bootstrap(int argc, char * const argv[]);
static int cmd_load(int argc, char * const argv[]);
static int cmd_remove(int argc, char * const argv[]);
static int cmd_list(int argc, char * const argv[]);
static int cmd_dump(int argc, char * const argv[]);
static int cmd_log(int argc, char * const argv[]);
static int cmd_help(int argc, char * const argv[]);

kern_return_t vproc_mig_get_root_bootstrap(mach_port_t bp, mach_port_t *rootbp);

mach_port_t bootstrap_port;

static const struct {
	const char *name;
	int (*func)(int argc, char * const argv[]);
	const char *desc;
} cmds[] = {
	{ "start",	cmd_start_stop,	"Start specified job" },
	{ "stop",	cmd_start_stop,	"Stop specified job" },
	{ "load",	cmd_load,	"Load a plist" },
	{ "remove",	cmd_remove, 	"Remove specified job" },
	{ "bootstrap",	cmd_bootstrap,	"Bootstrap launchd" },
        { "bslist",     cmd_bslist,     "List registered Mach services" },
	{ "list",	cmd_list,	"List jobs and information about jobs" },
	{ "dump",	cmd_dump,       "Dumps job(s) plist(s)"},
	{ "log",	cmd_log,	"Adjust logging level of launchd"},
	{ "help",	cmd_help,	"This help output" },
};

/* Bootstrap session will run system-wide LaunchDaemons */
static const char *bootstrap_paths[] = {
	"/etc/launchd.d",
	"/System/Library/LaunchDaemons",
	"/Library/LaunchDaemons"
};

/* Per-user sessions will run system-provided, admin-provided
 * and user-provided LaunchAgents
 */
static char user_agent_path[PATH_MAX];
static const char *agent_paths[] = {
	"/System/Library/LaunchAgents",
	"/Library/LaunchAgents",
	user_agent_path
};

static launch_data_t
to_launchd_sockets(json_t *json)
{
	launch_data_t result, arr;
	const char *key;
	size_t idx;
	json_t *val, *val2;

	result = launch_data_alloc(LAUNCH_DATA_DICTIONARY);

	json_object_foreach(json, key, val) {
		arr = launch_data_alloc(LAUNCH_DATA_ARRAY);

		switch (json_typeof(val)) {
			case JSON_OBJECT:
				launch_data_array_set_index(arr,
				    create_socket(val), 0);
				break;

			case JSON_ARRAY:
				json_array_foreach(val, idx, val2) {
					launch_data_array_set_index(arr,
					    create_socket(val2), idx);
				}
				break;

			default:
				errx(EX_OSERR, "Invalid jlist specification");
		}

		launch_data_dict_insert(result, arr, key);
	}

	return (result);
}

static launch_data_t
create_socket(json_t *json)
{
	int st = SOCK_STREAM;
	int sfd;
	int saved_errno;
	bool passive = true;
	json_t *val;

	if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_TYPE))) {
		if (!strcasecmp(json_string_value(val), "stream"))
			st = SOCK_STREAM;
		else if (!strcasecmp(json_string_value(val), "dgram"))
			st = SOCK_DGRAM;
		else if (!strcasecmp(json_string_value(val), "seqpacket"))
			st = SOCK_SEQPACKET;
	}

	if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_PASSIVE)))
		passive = json_is_true(val);

	if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_PATHNAME))) {
		struct sockaddr_un sun;
		mode_t sun_mode = 0;
		mode_t oldmask;
		bool setm = false;

		memset(&sun, 0, sizeof(sun));

		sun.sun_family = AF_UNIX;

		strncpy(sun.sun_path, json_string_value(val), sizeof(sun.sun_path));

		if ((sfd = socket(AF_UNIX, st, 0)) == -1)
			errx(EX_OSERR, "socket(): %s", strerror(errno));

		if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_PATHMODE))) {
			sun_mode = (mode_t)json_integer_value(val);
			setm = true;
		}

		if (passive) {
			if (unlink(sun.sun_path) == -1 && errno != ENOENT) {
				saved_errno = errno;
				close(sfd);
				errx(EX_OSERR, "unlink(): %s", strerror(saved_errno));
			}
			oldmask = umask(S_IRWXG|S_IRWXO);
			if (bind(sfd, (struct sockaddr *)&sun, (socklen_t) sizeof sun) == -1) {
				saved_errno = errno;
				close(sfd);
				umask(oldmask);
				errx(EX_OSERR, "bind(): %s", strerror(saved_errno));
			}
			umask(oldmask);
			if (setm)
				chmod(sun.sun_path, sun_mode);

			if ((st == SOCK_STREAM || st == SOCK_SEQPACKET) && listen(sfd, -1) == -1) {
				saved_errno = errno;
				close(sfd);
				errx(EX_OSERR, "listen(): %s", strerror(saved_errno));
			}
		} else if (connect(sfd, (struct sockaddr *)&sun, (socklen_t) sizeof sun) == -1) {
			saved_errno = errno;
			close(sfd);
			errx(EX_OSERR, "connect(): %s", strerror(saved_errno));
		}

		return launch_data_new_fd(sfd);
	} else {
		const char *node = NULL, *serv = NULL, *mgroup = NULL;
		char servnbuf[50];
		struct addrinfo hints, *res0, *res;
		int gerr, sock_opt = 1;

		memset(&hints, 0, sizeof(hints));

		hints.ai_socktype = st;
		hints.ai_family = AF_INET;

		if (passive)
			hints.ai_flags |= AI_PASSIVE;

		if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_NODENAME)))
			node = json_string_value(val);

		if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_MULTICASTGROUP)))
			mgroup = json_string_value(val);

		if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_SERVICENAME))) {
			if (json_typeof(val) == JSON_INTEGER) {
				sprintf(servnbuf, "%ld", json_integer_value(val));
				serv = servnbuf;
			} else
				serv = json_string_value(val);
		}

		if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_FAMILY))) {
			if (!strcasecmp(json_string_value(val), "IPv4"))
				hints.ai_family = AF_INET;
			else if (!strcasecmp(json_string_value(val), "IPv6"))
				hints.ai_family = AF_INET6;
		}

		if ((val = json_object_get(json, LAUNCH_JOBSOCKETKEY_PROTOCOL))) {
			if (!strcasecmp(json_string_value(val), "TCP"))
				hints.ai_protocol = IPPROTO_TCP;
			else if (!strcasecmp(json_string_value(val), "UDP"))
				hints.ai_protocol = IPPROTO_UDP;
		}

		if ((gerr = getaddrinfo(node, serv, &hints, &res0)) != 0)
			errx(EX_OSERR, "getaddrinfo(): %s", gai_strerror(gerr));

		for (res = res0; res; res = res->ai_next) {
			if ((sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
				errx(EX_OSERR, "socket(): %s", strerror(errno));

			if (hints.ai_flags & AI_PASSIVE) {
				if (AF_INET6 == res->ai_family && -1 == setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY,
				    (void *)&sock_opt, (socklen_t) sizeof sock_opt))
					errx(EX_OSERR, "setsockopt(IPV6_V6ONLY): %m");

				if (mgroup) {
					if (setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, (void *)&sock_opt, (socklen_t) sizeof sock_opt) == -1)
						errx(EX_OSERR, "setsockopt(SO_REUSEPORT): %s", strerror(errno));
				} else {
					if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sock_opt, (socklen_t) sizeof sock_opt) == -1)
						errx(EX_OSERR, "setsockopt(SO_REUSEADDR): %s", strerror(errno));
				}

				if (bind(sfd, res->ai_addr, res->ai_addrlen) == -1)
					errx(EX_OSERR, "bind(): %s", strerror(errno));

				/* The kernel may have dynamically assigned some part of the
				 * address. (The port being a common example.)
				 */
				if (getsockname(sfd, res->ai_addr, &res->ai_addrlen) == -1)
					errx(EX_OSERR, "getsockname(): %s", strerror(errno));

				//if (mgroup) {
				//	do_mgroup_join(sfd, res->ai_family, res->ai_socktype, res->ai_protocol, mgroup);
				//}
				if ((res->ai_socktype == SOCK_STREAM || res->ai_socktype == SOCK_SEQPACKET) && listen(sfd, -1) == -1)
					errx(EX_OSERR, "listen(): %s", strerror(errno));

			} else {
				if (connect(sfd, res->ai_addr, res->ai_addrlen) == -1)
					errx(EX_OSERR, "connect(): %s", strerror(errno));
			}

			return launch_data_new_fd(sfd);
		}
	}

	errx(EX_OSERR, "Invalid socket specification");
	return (NULL);
}

static launch_data_t
to_launchd(json_t *json)
{
	size_t idx;
	launch_data_t arr, dict;
	const char *key;
	json_t *val;

	switch (json_typeof(json)) {
	case JSON_STRING:
		return launch_data_new_string(json_string_value(json));

	case JSON_INTEGER:
		return launch_data_new_integer(json_integer_value(json));

	case JSON_TRUE:
		return launch_data_new_bool(true);

	case JSON_FALSE:
		return launch_data_new_bool(false);

	case JSON_ARRAY:
		arr = launch_data_alloc(LAUNCH_DATA_ARRAY);
		json_array_foreach(json, idx, val) {
			launch_data_array_set_index(arr, to_launchd(val), idx);
		}

		return arr;

	case JSON_OBJECT:
		dict = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
		json_object_foreach(json, key, val) {
			if (!strcmp(key, "Sockets")) {
				launch_data_dict_insert(dict,
				    to_launchd_sockets(val), key);

				continue;
			}

			launch_data_dict_insert(dict, to_launchd(val), key);
		}

		return dict;

	case JSON_REAL:
	case JSON_NULL:
		return NULL;

	}

	return NULL;
}

static void
to_json_dict(const launch_data_t lval, const char *key, void *ctx)
{
	json_t *obj = (json_t *)ctx;
	json_object_set_new(obj, key, to_json(lval));
}

static json_t *
to_json(launch_data_t ld)
{
	char *txt;
	json_t *arr, *obj;
	size_t i;

	switch (launch_data_get_type(ld)) {
	case LAUNCH_DATA_STRING:
		return json_string(launch_data_get_string(ld));

	case LAUNCH_DATA_INTEGER:
		return json_integer(launch_data_get_integer(ld));

	case LAUNCH_DATA_BOOL:
		return json_boolean(launch_data_get_bool(ld));

	case LAUNCH_DATA_ARRAY:
		arr = json_array();
		for (i = 0; i < launch_data_array_get_count(ld); i++) {
			json_array_append_new(arr, to_json(launch_data_array_get_index(ld, i)));
		}

		return arr;

	case LAUNCH_DATA_DICTIONARY:
		obj = json_object();
		launch_data_dict_iterate(ld, to_json_dict, obj);
		return obj;

	case LAUNCH_DATA_FD:
		asprintf(&txt, "<file descriptor %d>", launch_data_get_fd(ld));
		return json_string(txt);

	case LAUNCH_DATA_MACHPORT:
		asprintf(&txt, "<mach port %d>", launch_data_get_machport(ld));
		return json_string(txt);

	case LAUNCH_DATA_ERRNO:
		/* Could only happen in top-level node */
		errno = launch_data_get_errno(ld);
		return (errno ? NULL : json_null());

	default:
		return json_null();
	}
}

static json_t *
launch_msg_json(json_t *input)
{
	launch_data_t result;

	result = launch_msg(to_launchd(input));

	if (result == NULL)
		return (NULL);
	
	return to_json(result);
}

static void
stall(const char *message, ...)
{
	va_list ap;
	va_start(ap, message);

	vsyslog(LOG_ERR, message, ap);
	va_end(ap);
	sleep(STALL_TIMEOUT);
}

static bool
do_single_user_mode2(void)
{
	bool runcom_fsck = true; /* should_fsck(); */
	// Need to do something about these
	bool runcom_safe = false;
	bool runcom_netboot = false;
	int wstatus;
	int fd;
	pid_t p;

	switch ((p = fork())) {
	case -1:
		syslog(LOG_ERR, "can't fork single-user shell, trying again: %m");
		return false;
	case 0:
		break;
	default:
		(void)waitpid(p, &wstatus, 0);
		if (WIFEXITED(wstatus)) {
			if (WEXITSTATUS(wstatus) == EXIT_SUCCESS) {
				return true;
			} else {
				syslog(LOG_NOTICE | LOG_CONSOLE, "single user mode: exit status: %d", WEXITSTATUS(wstatus));
			}
		} else {
			syslog(LOG_NOTICE | LOG_CONSOLE, "single user mode shell: %s", strsignal(WTERMSIG(wstatus)));
		}
		return false;
	}

	revoke(_PATH_CONSOLE);
	if ((fd = open(_PATH_CONSOLE, O_RDWR)) == -1) {
		_exit(EXIT_FAILURE);
	}
	if (login_tty(fd) == -1) {
		_exit(EXIT_FAILURE);
	}

	setenv("TERM", "vt100", 1);
	setenv("SafeBoot", runcom_safe ? "-x" : "", 1);
	setenv("VerboseFlag", "-v", 1); /* single user mode implies verbose mode */
	setenv("FsckSlash", runcom_fsck ? "-F" : "", 1);
	setenv("NetBoot", runcom_netboot ? "-N" : "", 1);

	if (runcom_fsck) {
		fprintf(stdout, "Singleuser boot -- fsck not done\n");
		fprintf(stdout, "Root device is mounted read-only\n");
		fprintf(stdout, "If you want to make modifications to files:\n");
		fprintf(stdout, "\t/sbin/zfs mount -a\n\t/sbin/zfs set readonly=off airyxOS/ROOT/default\n");
		fprintf(stdout, "If you wish to boot the system:\n");
		fprintf(stdout, "\texit\n");
		fflush(stdout);
	}

	execl(_PATH_BSHELL, "-sh", NULL);
	syslog(LOG_CRIT | LOG_CONSOLE, "can't exec %s for single user: %m\n", _PATH_BSHELL);
	_exit(EXIT_FAILURE);
}

static void
do_single_user_mode(bool sflag)
{
	if (sflag) {
		while (!do_single_user_mode2()) {
			sleep(1);
		}
	}
}
/*
 * Start a session and allocate a controlling terminal.
 * Only called by children of init after forking.
 */
static void
setctty(const char *name, int flags)
{
	int fd;

	revoke(name);
	if ((fd = open(name, flags | O_RDWR)) == -1) {
		stall("can't open %s: %m", name);
		_exit(EXIT_FAILURE);
	}
	if (login_tty(fd) == -1) {
		stall("can't get %s for controlling terminal: %m", name);
		_exit(EXIT_FAILURE);
	}
}

static void
runcom(void)
{
#define PATH_RUNCOM	"/etc/rc"
	bool runcom_fsck = true;
	bool runcom_safe = false;
	bool runcom_netboot = false;
	struct termios term;
	int vdisable;
	pid_t runcom_pid;

	if ((runcom_pid = fork()) == -1) {
		syslog(LOG_ERR | LOG_CONSOLE, "can't fork for %s on %s: %m", _PATH_BSHELL, PATH_RUNCOM);
		sleep(STALL_TIMEOUT);
		return;
	} else if (runcom_pid > 0) {
		(void)waitpid(runcom_pid, NULL, 0);
		return;
	} else {
		// Run the rc script
		syslog(LOG_ERR, "setctty()\n");
		setctty(_PATH_CONSOLE, 0);
		
		syslog(LOG_ERR, "fpathconf()\n");
		sleep(1);
		if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) == -1) {
			syslog(LOG_ERR, "fpathconf(\"%s\") %m", _PATH_CONSOLE);
		} else if (tcgetattr(STDIN_FILENO, &term) == -1) {
			syslog(LOG_ERR, "tcgetattr(\"%s\") %m", _PATH_CONSOLE);
		} else {
			term.c_cc[VINTR] = vdisable;
			term.c_cc[VKILL] = vdisable;
			term.c_cc[VQUIT] = vdisable;
			term.c_cc[VSUSP] = vdisable;
			term.c_cc[VSTART] = vdisable;
			term.c_cc[VSTOP] = vdisable;
			term.c_cc[VDSUSP] = vdisable;
			sleep(1);
			syslog(LOG_ERR, "tcsetattr(STDIN_FILENO) ...");
			if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1)
				syslog(LOG_WARNING, "tcsetattr(\"%s\") %m", _PATH_CONSOLE);
			syslog(LOG_ERR, "done\n");
		}
		sleep(1);
		syslog(LOG_ERR, "setenv\n");
		setenv("SafeBoot", runcom_safe ? "-x" : "", 1);
		setenv("FsckSlash", runcom_fsck ? "-F" : "", 1);
		setenv("NetBoot", runcom_netboot ? "-N" : "", 1);
		syslog(LOG_ERR, "execv\n");
		execl(_PATH_BSHELL, "sh", PATH_RUNCOM, NULL);
		syslog(LOG_ERR | LOG_CONSOLE, "execv errno=%m");
		sleep(2);
		stall("can't exec %s for %s: %m", _PATH_BSHELL, PATH_RUNCOM);
		_exit(EXIT_FAILURE);
	}
	return;
}

static void
system_specific_bootstrap(bool sflag)
{
#define PATH_BOOTSTRAP	"/etc/bootstrap"
	
	// Go into single-user mode if requested
	do_single_user_mode(sflag);
	// Apple does a lot in the code, but we'll just call /etc/bootstrap for now
	system(PATH_BOOTSTRAP);
}

static int
load_job(const char *filename)
{
	FILE *input;
	json_error_t error;
	json_t *msg, *plist;

	input = strcmp(filename, "-") ? fopen(filename, "r") : stdin;
	if (input == NULL)
		return (-1);

	plist = json_loadf(input, JSON_DECODE_ANY, &error);
	msg = json_object();
	json_object_set_new(msg, "SubmitJob", plist);

	return (launch_msg_json(msg) == NULL ? -1 : 0); 
}

/*
 * This is used to bootstrap.  The primary case we care
 * about is during system boot, in which case launchd will
 * invoke us with "-S System", and, optionally, "-s" (indicating
 * single-user mode).  In that case, we need to do single user
 * mode if desired, and then run the rc scripts (aka runcom).
 * After that, we want to load the plist/json files and tell
 * launchd about them.
 *
 * The System session runs all LaunchDaemons under the pid 1 launchd.
 * A per-user launchd then runs all LaunchAgents as the user in the
 * Background session.
 *
 */
static int
cmd_bootstrap(int argc, char * const argv[])
{
	char *session = NULL;
	bool sflag = false;
	int ch;
	
	struct dirent **files;
	char *name, *path;
	unsigned long i;
	int n;

	while ((ch = getopt(argc, __DECONST(char **, argv), "sS:")) != -1) {
		switch (ch) {
		case 's':
			sflag = true;
			break;
		case 'S':
			session = optarg;
			break;
		case '?':
		default:
			break;
		}
	}

	optind = 1;
	optreset = 1;

	if (!session) {
		syslog(LOG_ERR | LOG_CONSOLE, "usage: %s bootstrap [-s] -S <session-type>", getprogname());
		return 1;
	}

	if (strcasecmp(session, "System") == 0) {
		system_specific_bootstrap(sflag);
		// Perhaps this should go in system_specific_bootstrap
		for (i = 0; i < N(bootstrap_paths); i++) {
			n = scandir(bootstrap_paths[i], &files, NULL, alphasort);
			if (n < 0)
				continue;
			
			while (n--) {
				name = files[n]->d_name;
				if (name[0] == '.')
					continue;
				
				printf("Loading job: %s: ", name);
				asprintf(&path, "%s/%s", bootstrap_paths[i], name);
				if (load_job(path) == 0)
					printf("ok\n");
				else
					printf("failed: %s\n", strerror(errno));
			}
		}
		// give jobs time to start
		sleep(2);
		// Then run the rc script(s)
		runcom();
	} else if(strcasecmp(session, "Background") == 0) {
		struct passwd *pwent = getpwuid(getuid());
		if (pwent == NULL) {
			printf("Unknown user uid=%u!\n", getuid());
			return (-1);
		}
		snprintf(user_agent_path, PATH_MAX, "%s/Library/LaunchAgents",
			pwent->pw_dir);

		for (i = 0; i < N(agent_paths); i++) {
			n = scandir(agent_paths[i], &files, NULL, alphasort);
			if (n < 0)
				continue;
			
			while (n--) {
				name = files[n]->d_name;
				if (name[0] == '.')
					continue;
				
				printf("Loading job: %s: ", name);
				asprintf(&path, "%s/%s", agent_paths[i], name);
				if (load_job(path) == 0)
					printf("ok\n");
				else
					printf("failed: %s\n", strerror(errno));
			}
		}
	}
	return (0);
}

static int
cmd_bslist(int argc, char * const argv[])
{
    kern_return_t kr;
    name_array_t svc_names;
    name_array_t svr_names;
    bool_array_t svc_active;
    int svc_name_cnt, svr_name_cnt, act_cnt;
    int ch;
    int verbose = 0;

    mach_port_t bp = bootstrap_port;
    if(getuid() == 0) {
        kr = vproc_mig_get_root_bootstrap(bootstrap_port, &bp);
        if(bp == MACH_PORT_NULL)
            bp = bootstrap_port;
    }

    kr = bootstrap_info(bp,
        &svc_names, &svc_name_cnt,
        &svr_names, &svr_name_cnt,
        &svc_active, &act_cnt, 0);
    if(kr != KERN_SUCCESS) {
        printf("%s\n", bootstrap_strerror(kr));
        return -1;
    }

    if(act_cnt <= 0)
        return 0;

    while((ch = getopt(argc, __DECONST(char **, argv), "v")) != -1) {
        switch (ch) {
            case 'v':
                verbose = 1;
                break;
            default:
                break;
        }
    }

    optind = 1;
    optreset = 1;

    for(int i=0; i<act_cnt; ++i) {
        printf("%s  %s%s%s%s\n",
            svc_active[i] ? "A" : "I",  // FIXME: handle D (on demand)
            svc_names[i] ? svc_names[i] : "-",
            verbose ? " (" : "",
            verbose ? (svr_names[i] ? svr_names[i] : "-") : "",
            verbose ? ")" : "");
    }
    printf("\n");
    return 0;
}

static int
cmd_load(int argc, char * const argv[])
{
	int i;

	if (argc < 2)
		errx(EX_USAGE, "Usage: launchctl load <plist> [<plist> ...]");

	for (i = 1; i < argc; i++) {
		if (load_job(argv[i]) != 0)
			err(EX_OSERR, "Cannot load job from %s", argv[i]);
	}

	return (0);
}

static int
cmd_start_stop(int argc, char * const argv[])
{
	json_t *msg;

	msg = json_object();

	if (argc < 2)
		errx(EX_USAGE, "Usage: %s <jobname>", argv[0]);

	if (!strcmp(argv[0], "start"))
		json_object_set(msg, "StartJob", json_string(argv[1]));

	if (!strcmp(argv[0], "stop"))
		json_object_set(msg, "StopJob", json_string(argv[1]));

	launch_msg_json(msg);
	return (0);
}

static int
cmd_remove(int argc, char * const argv[])
{
	json_t *msg;

	msg = json_object();

	if (argc < 2)
		errx(EX_USAGE, "Usage: remove <jobname>");

	json_object_set(msg, "RemoveJob", json_string(argv[1]));
	launch_msg_json(msg);
	return (0);
}

static int
cmd_list(int argc, char * const argv[])
{
	json_t *msg, *result, *job;
	const char *key;
    char *pidstr;

	(void)argc;
	(void)argv;

	msg = json_string("GetJobs");
	result = launch_msg_json(msg);

	if (result == NULL)
		err(EX_OSERR, "Error getting job list");

    printf("%-8s %-8s %s\n", "PID", "Status", "Label");
	json_object_foreach(result, key, job) {
        pid_t pid = json_integer_value(json_object_get(job, "PID"));
        asprintf(&pidstr, "%u", pid);
		printf("%-8s %-8ld %s\n", pid == 0 ? "-" : pidstr,
                json_integer_value(
                json_object_get(job, "LastExitStatus")), key);
        free(pidstr);
	}

	return (0);
}

static int
cmd_dump(int argc, char * const argv[])
{
	json_t *msg, *result;

	if (argc == 1)
		msg = json_string("GetJobs");

	if (argc == 2) {
		msg = json_object();
		json_object_set(msg, "GetJob", json_string(argv[1]));
	}

	result = launch_msg_json(msg);
	if (result == NULL)
		err(EX_OSERR, "Error getting job information");

	json_dumpf(result, stdout, JSON_INDENT(4));
	return (0);
}

static int
cmd_help(int argc, char * const argv[])
{
	size_t i;

	(void)argc;
	(void)argv;

	fprintf(stderr, "Usage: launchctl <subcommand> [arguments...]\n");

	for (i = 0; i < N(cmds); i++) {
		fprintf(stderr, "%s - %s\n", cmds[i].name, cmds[i].desc);
	}

	return (0);
}

static int
cmd_log(int argc, char * const argv[])
{
	int64_t inval, outval;
	bool badargs = false, maskmode = false, onlymode = false, levelmode = false;

	static const struct {
		const char *name;
		int level;
	} logtbl[] = {
		{ "debug",	LOG_DEBUG },
		{ "info",	LOG_INFO },
		{ "notice",	LOG_NOTICE },
		{ "warning",	LOG_WARNING },
		{ "error",	LOG_ERR },
		{ "critical",	LOG_CRIT },
		{ "alert",	LOG_ALERT },
		{ "emergency",	LOG_EMERG },
	};

	size_t i, j, logtblsz = sizeof logtbl / sizeof logtbl[0];
	int m = 0;

	if (argc >= 2) {
		if (!strcmp(argv[1], "mask"))
			maskmode = true;
		else if (!strcmp(argv[1], "only"))
			onlymode = true;
		else if (!strcmp(argv[1], "level"))
			levelmode = true;
		else
			badargs = true;
	}

	if (maskmode)
		m = LOG_UPTO(LOG_DEBUG);

	if (argc > 2 && (maskmode || onlymode)) {
		for (i = 2; i < (size_t)argc; i++) {
			for (j = 0; j < logtblsz; j++) {
				if (!strcmp(argv[i], logtbl[j].name)) {
					if (maskmode)
						m &= ~(LOG_MASK(logtbl[j].level));
					else
						m |= LOG_MASK(logtbl[j].level);
					break;
				}
			}
			if (j == logtblsz) {
				badargs = true;
				break;
			}
		}
	} else if (argc > 2 && levelmode) {
		for (j = 0; j < logtblsz; j++) {
			if (!strcmp(argv[2], logtbl[j].name)) {
				m = LOG_UPTO(logtbl[j].level);
				break;
			}
		}
		if (j == logtblsz)
			badargs = true;
	} else if (argc != 1) {
		badargs = true;
	}

	if (badargs)
		errx(EX_USAGE, "Usage: log [[mask loglevels...] | [only loglevels...] [level loglevel]]");

	inval = m;

	if (vproc_swap_integer(NULL, VPROC_GSK_GLOBAL_LOG_MASK, argc != 1 ? &inval : NULL, &outval) == NULL) {
		if (argc == 1) {
			for (j = 0; j < logtblsz; j++) {
				if (outval & LOG_MASK(logtbl[j].level)) {
					printf("%s ", logtbl[j].name);
				}
			}
			printf("\n");
		}
		return 0;
	} else {
		return 1;
	}
}

int
main(int argc, char * const argv[])
{
	const char *cmd;
	size_t i;
	int c;

	while ((c = getopt(argc, argv, "S:h")) != -1) {
		switch (c) {

		}
	}

	if (optind == argc)
		return cmd_help(0, NULL);

	cmd = argv[optind];

	for (i = 0; i < N(cmds); i++) {
		if (!strcmp(cmd, cmds[i].name))
			return (cmds[i].func(argc - optind, argv + optind));
	}

	errx(EX_USAGE, "Usage: launchctl <subcommand> [arguments...]");
	return (1);
}
