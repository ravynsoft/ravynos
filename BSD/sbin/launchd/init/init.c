/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */
/*-
 * Copyright (c) 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Donn Seeley at Berkeley Software Design, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>
#include <Security/AuthSession.h>
#endif

#include <sys/types.h>
#include <sys/event.h>
#include <sys/queue.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <ttyent.h>
#include <unistd.h>
#include <paths.h>
#include <util.h>
#include <libgen.h>
#include <paths.h>
#include <termios.h>

#include "launchd.h"

#define _PATH_RUNCOM            "/etc/rc"

/*
 * Sleep times; used to prevent thrashing.
 */
#define	GETTY_SPACING		 5	/* N secs minimum getty spacing */
#define	GETTY_SLEEP		30	/* sleep N secs after spacing problem */
#define	STALL_TIMEOUT		30	/* wait N secs after warning */
#define	DEATH_WATCH		10	/* wait N secs for procs to die */
#define FAILED_HW_PASS		 5	/* wait N secs before croaking user */

static void stall(const char *, ...);

static void single_user_callback(void *, struct kevent *);
static kq_callback kqsingle_user_callback = single_user_callback;
static void runcom_callback(void *, struct kevent *);
static kq_callback kqruncom_callback = runcom_callback;

static void single_user(void);
static void runcom(void);

static bool runcom_safe = false;
static bool runcom_netboot = false;
static bool single_user_mode = false;
static bool run_runcom = true;
static pid_t single_user_pid = 0;
static pid_t runcom_pid = 0;

static void setctty(const char *, int);

// gvdl@next.com 14 Aug 1995
//   - from ~apps/loginwindow_proj/loginwindow/common.h
#define REALLY_EXIT_TO_CONSOLE                  229

// From old init.c
// These flags are used in the se_flags field of the init_session structure
#define	SE_SHUTDOWN	0x1		/* session won't be restarted */

// The flags below control what sort of getty is launched.
#define SE_GETTY_LAUNCH	0x30	/* What type of getty to launch */ 
#define SE_COMMON	0x00	/* Usual command that is run - getty */
#define SE_ONERROR	0x10	/* Command to run if error condition occurs.
				 * This will almost always be the windowserver
				 * and loginwindow.  This is so if the w.s.
				 * ever dies, that the naive user (stan)
				 * doesn't ever see the console window. */
#define SE_ONOPTION 	0x20	/* Command to run when loginwindow exits with
				 * special error code (229).  This signifies
				 * that the user typed "console" at l.w. and
				 * l.w. wants to exit and have init run getty
				 * which will then put up a console window. */

typedef struct _se_command {
	char	*path;		/* what to run on that port */
	char	**argv;		/* pre-parsed argument array */
} se_cmd_t;

typedef struct init_session {
	kq_callback se_callback;	/* run loop callback */
	int	se_index;		/* index of entry in ttys file */
	pid_t	se_process;		/* controlling process */
	time_t	se_started;		/* used to avoid thrashing */
	int	se_flags;		/* status of session */
	char	*se_device;		/* filename of port */
	se_cmd_t se_getty;		/* what to run on that port */
	se_cmd_t se_onerror;		/* See SE_ONERROR above */
	se_cmd_t se_onoption;		/* See SE_ONOPTION above */
	TAILQ_ENTRY(init_session) tqe;
} *session_t;

static TAILQ_HEAD(sessionshead, init_session) sessions = TAILQ_HEAD_INITIALIZER(sessions);

static void session_new(int, struct ttyent *);
static void session_free(session_t);
static void session_launch(session_t);
static void session_reap(session_t);
static void session_callback(void *, struct kevent *);

static char **construct_argv(char *);
static void setsecuritylevel(int);
static int getsecuritylevel(void);
static int setupargv(session_t, struct ttyent *);
static bool should_fsck(void);

void
init_boot(bool sflag __unused)
{
#if 0	
	int nbmib[2] = { CTL_KERN, KERN_NETBOOT };
	int sbmib[2] = { CTL_KERN, KERN_SAFEBOOT };
	uint32_t v = 0;
	size_t vsz = sizeof(v);

	if (sflag) {
		single_user_mode = true;
		run_runcom = false;
	}

	if (launchd_assumes(sysctl(nbmib, 2, &v, &vsz, NULL, 0) != -1)) {
		if (v != 0)
			runcom_netboot = true;
	}
	if (launchd_assumes(sysctl(sbmib, 2, &v, &vsz, NULL, 0) != -1)) {
		if (v != 0)
			runcom_safe = true;
	}
#endif
}

void
init_pre_kevent(bool sflag)
{
	session_t s;

	if (sflag) {
		single_user_mode = 1;
		run_runcom = 0;
	}
	syslog(LOG_EMERG, "starting init_pre_kevent() single_user_pid=%d runcom_pid=%d\n",
		   single_user_pid, runcom_pid);
	syslog(LOG_EMERG, "... single_user_mode=%d run_runcom=%d\n", single_user_mode, run_runcom);
	if (single_user_pid || runcom_pid) {
		syslog(LOG_ERR, "skipping()\n");
		return;
	}
	if (single_user_mode) {
		syslog(LOG_ERR, "single_user()\n");
		return single_user();
	}
	if (run_runcom) {
		syslog(LOG_EMERG, "runcom()\n");
		return runcom();
	}
	/*
	 * If the administrator has not set the security level to -1
	 * to indicate that the kernel should not run multiuser in secure
	 * mode, and the run script has not set a higher level of security 
	 * than level 1, then put the kernel into secure mode.
	 */
	if (getsecuritylevel() == 0) {
		syslog(LOG_ERR, "setsecuritylevel()");
		setsecuritylevel(1);
	}
	TAILQ_FOREACH(s, &sessions, tqe) {
		if (s->se_process == 0) {
			syslog(LOG_ERR, "session_launch()");
			session_launch(s);
		}
	}
	syslog(LOG_ERR, "done init_pre_kevent()\n");
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

static int
getsecuritylevel(void)
{
	int name[2], curlevel;
	size_t len;

	name[0] = CTL_KERN;
	name[1] = KERN_SECURELVL;
	len = sizeof (curlevel);
	if (sysctl(name, 2, &curlevel, &len, NULL, 0) == -1) {
		syslog(LOG_ALERT, "cannot get kernel security level: %m");
		return -1;
	}
	return curlevel;
}

static void
setsecuritylevel(int newlevel)
{
	int name[2], curlevel;

	curlevel = getsecuritylevel();
	if (newlevel == curlevel)
		return;
	name[0] = CTL_KERN;
	name[1] = KERN_SECURELVL;
	if (sysctl(name, 2, NULL, NULL, &newlevel, sizeof newlevel) == -1) {
		syslog(LOG_ALERT, "cannot change kernel security level from %d to %d: %m",
				curlevel, newlevel);
		return;
	}
	syslog(LOG_INFO, "kernel security level changed from %d to %d",
	    curlevel, newlevel);
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
		launchd_exit(EXIT_FAILURE);
	}
	if (login_tty(fd) == -1) {
		stall("can't get %s for controlling terminal: %m", name);
		launchd_exit(EXIT_FAILURE);
	}
}

static void
single_user(void)
{
	bool runcom_fsck = should_fsck();
	const char *argv[2];

	if (getsecuritylevel() > 0)
		setsecuritylevel(0);
	
	if ((single_user_pid = launchd_fork()) == -1) {
		syslog(LOG_ERR, "can't fork single-user shell, trying again: %m");
		return;
	} else if (single_user_pid == 0) {
#if 0
		setctty(_PATH_CONSOLE, O_POPUP);
#endif
                setenv("TERM", "vt100", 1);
		setenv("SafeBoot", runcom_safe ? "-x" : "", 1);
		setenv("VerboseFlag", "-v", 1); /* single user mode implies verbose mode */
		setenv("FsckSlash", runcom_fsck ? "-F" : "", 1);
		setenv("NetBoot", runcom_netboot ? "-N" : "", 1);

		if (runcom_fsck) {
			fprintf(stdout, "Singleuser boot -- fsck not done\n");
			fprintf(stdout, "Root device is mounted read-only\n\n");
			fprintf(stdout, "If you want to make modifications to files:\n");
			fprintf(stdout, "\t/sbin/fsck -fy\n\t/sbin/mount -uw /\n\n");
			fprintf(stdout, "If you wish to boot the system:\n");
			fprintf(stdout, "\texit\n\n");
			fflush(stdout);
		}

		argv[0] = "-sh";
		argv[1] = NULL;
		execv(_PATH_BSHELL, __DECONST(char *const *, argv));
		syslog(LOG_ERR, "can't exec %s for single user: %m", _PATH_BSHELL);
		sleep(STALL_TIMEOUT);
		launchd_exit(EXIT_FAILURE);
	} else {
		if (kevent_mod(single_user_pid, EVFILT_PROC, EV_ADD, 
					NOTE_EXIT, 0, &kqsingle_user_callback) == -1)
			single_user_callback(NULL, NULL);
	}
}

static void
single_user_callback(void *obj __attribute__((unused)), struct kevent *kev __attribute__((unused)))
{
	int status;

	if (!launchd_assumes(waitpid(single_user_pid, &status, 0) == single_user_pid))
		return;

	if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
		syslog(LOG_INFO, "single user shell terminated, restarting");
		run_runcom = true;
		single_user_mode = false;
	} else {
		syslog(LOG_INFO, "single user shell terminated.");
		run_runcom = false;
		if (WTERMSIG(status) != SIGKILL)
			single_user_mode = true;
	}

	single_user_pid = 0;
}

static struct timeval runcom_start_tv = { 0, 0 };
/*
 * Run the system startup script.
 */
static void
runcom(void)
{
	bool runcom_fsck = should_fsck();
	char *argv[4];
	struct termios term;
	int vdisable;

	gettimeofday(&runcom_start_tv, NULL);
	syslog(LOG_ERR, "launchd_fork()\n");
	if ((runcom_pid = launchd_fork()) == -1) {
		syslog(LOG_ERR, "can't fork for %s on %s: %m", _PATH_BSHELL, _PATH_RUNCOM);
		sleep(STALL_TIMEOUT);
		runcom_pid = 0;
		single_user_mode = true;
		return;
	} else if (runcom_pid > 0) {
		run_runcom = false;
		if (kevent_mod(runcom_pid, EVFILT_PROC, EV_ADD, 
					   NOTE_EXIT, 0, &kqruncom_callback) == -1) {
			syslog(LOG_ERR, "runcom_callback() ... ");
			runcom_callback(NULL, NULL);
			syslog(LOG_ERR, "done\n");
		}
		return;
	}
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
	{
		char _sh[] = "sh";
		int err;

		argv[0] = _sh;
		argv[1] = __DECONST(char *, _PATH_RUNCOM);
		argv[2] = 0;
		syslog(LOG_ERR, "execv(%s, %p)\n", _PATH_BSHELL, argv);	
		err = execv(_PATH_BSHELL, argv);
		syslog(LOG_ERR, "execv err=%d errno=%d", err, errno);
		sleep(2);
	}
	stall("can't exec %s for %s: %m", _PATH_BSHELL, _PATH_RUNCOM);
	launchd_exit(EXIT_FAILURE);
}

static void
runcom_callback(void *obj __attribute__((unused)), struct kevent *kev __attribute__((unused)))
{
	int status;
	struct timeval runcom_end_tv, runcom_total_tv;
	double sec;

	gettimeofday(&runcom_end_tv, NULL);
	timersub(&runcom_end_tv, &runcom_start_tv, &runcom_total_tv);
	sec = runcom_total_tv.tv_sec;
	sec += (double)runcom_total_tv.tv_usec / (double)1000000;
	syslog(LOG_INFO, "%s finished in: %.3f seconds", _PATH_RUNCOM, sec);

	if (launchd_assumes(waitpid(runcom_pid, &status, 0) == runcom_pid)) {
		runcom_pid = 0;
	} else {
		syslog(LOG_ERR, "going to single user mode");
		single_user_mode = true;
		return;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
#ifdef notyet
		logwtmp("~", "reboot", "");
#endif
		return;
	} else if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGTERM || WTERMSIG(status) == SIGKILL)) {
		return;
	}

	syslog(LOG_ERR, "%s on %s terminated abnormally, going to single user mode",
			_PATH_BSHELL, _PATH_RUNCOM);
	single_user_mode = true;
}

/*
 * Construct an argument vector from a command line.
 */
char **
construct_argv(command)
	char *command;
{
	int argc = 0;
	char **argv = (char **) malloc(((strlen(command) + 1) / 2 + 1)
						* sizeof (char *));
	static const char separators[] = " \t";

	if ((argv[argc++] = strtok(command, separators)) == 0)
		return 0;
	while ((argv[argc++] = strtok(NULL, separators)))
		continue;
	return argv;
}

/*
 * Deallocate a session descriptor.
 */

static void free_command(se_cmd_t *se_cmd)
{
    if (se_cmd->path) {
	free(se_cmd->path);
	free(se_cmd->argv);
    }
}

void
session_free(session_t s)
{
	TAILQ_REMOVE(&sessions, s, tqe);
	if (s->se_process) {
#ifdef notyet		
		if (kevent_mod(s->se_process, EVFILT_PROC, EV_ADD, 
					NOTE_EXIT, 0, &kqsimple_zombie_reaper) == -1)
			session_reap(s);
		else
			kill(s->se_process, SIGHUP);
#endif		
	}
	free(s->se_device);
	free_command(&s->se_getty);
	free_command(&s->se_onerror);
	free_command(&s->se_onoption);
	free(s);
}

static int setup_command(se_cmd_t *se_cmd, char *command, char *arg )
{
	char *commandWithArg;

	asprintf(&commandWithArg, "%s %s", command, arg);

	free_command(se_cmd);

	se_cmd->path = commandWithArg;
	se_cmd->argv = construct_argv(commandWithArg);
	if (se_cmd->argv == NULL) {
		free(se_cmd->path);
		se_cmd->path = NULL;
		return 0;
	}
	return 1;
}

/*
 * Calculate getty and if useful window argv vectors.
 */
static int
setupargv(sp, typ)
	session_t sp;
	struct ttyent *typ;
{
    const char *type;

    if ( !setup_command(&sp->se_getty, typ->ty_getty, typ->ty_name) )
    {
	type = "getty";
	goto bad_args;
    }
#if 0
    if (typ->ty_onerror
    && !setup_command(&sp->se_onerror, typ->ty_onerror, typ->ty_name) )
    {
	type = "onerror";
	goto bad_args;
    }

    if (typ->ty_onoption
    && !setup_command(&sp->se_onoption, typ->ty_onoption, typ->ty_name) )
    {
	type = "onoption";
	goto bad_args;
    }
#endif
    return 1;

bad_args:
    syslog(LOG_WARNING, "can't parse %s for port %s", type, sp->se_device);
    return 0;
}


/*
 * Allocate a new session descriptor.
 */
void
session_new(session_index, typ)
	int session_index;
	struct ttyent *typ;
{
	session_t s;

	if ((typ->ty_status & TTY_ON) == 0 ||
	    typ->ty_name == 0 ||
	    typ->ty_getty == 0)
		return;

	s = calloc(1, sizeof(struct init_session));

	s->se_callback = session_callback;
	s->se_index = session_index;

	TAILQ_INSERT_TAIL(&sessions, s, tqe);

	asprintf(&s->se_device, "%s%s", _PATH_DEV, typ->ty_name);

	if (setupargv(s, typ) == 0)
		session_free(s);
}

static void
session_launch(session_t s)
{
	pid_t pid;
	sigset_t mask;
	se_cmd_t *se_cmd;
	const char *session_type = NULL;
	time_t current_time      = time(NULL);
	bool is_loginwindow = false;

	// Setup the default values;
	switch (s->se_flags & SE_GETTY_LAUNCH) {
	case SE_ONOPTION:
		if (s->se_onoption.path) {
			se_cmd       = &s->se_onoption;
			session_type = "onoption";
			break;
		}
		/* No break */
	case SE_ONERROR:
		if (s->se_onerror.path) {
			se_cmd       = &s->se_onerror;
			session_type = "onerror";
			break;
		}
		/* No break */
	case SE_COMMON:
	default:
		se_cmd       = &s->se_getty;
		session_type = "getty";
		break;
	}

	if (strcmp(se_cmd->argv[0], "/System/Library/CoreServices/loginwindow.app/Contents/MacOS/loginwindow") == 0)
		is_loginwindow = true;

	pid = launchd_fork();

	if (pid == -1) {
		syslog(LOG_ERR, "can't fork for %s on port %s: %m",
				session_type, s->se_device);
		return;
	}

	if (pid) {
		s->se_process = pid;
		s->se_started = time(NULL);
		s->se_flags  &= ~SE_GETTY_LAUNCH; // clear down getty launch type
		if (kevent_mod(pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, &s->se_callback) == -1)
			session_reap(s);
		return;
	}

	if (current_time > s->se_started &&
	    current_time - s->se_started < GETTY_SPACING) {
		syslog(LOG_WARNING, "%s repeating too quickly on port %s, sleeping",
		        session_type, s->se_device);
		sleep(GETTY_SLEEP);
	}

	sigemptyset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);


	if (!is_loginwindow)
		launchd_SessionCreate();

	execv(se_cmd->argv[0], se_cmd->argv);
	stall("can't exec %s '%s' for port %s: %m", session_type,
		se_cmd->argv[0], s->se_device);
	launchd_exit(EXIT_FAILURE);
}

static void
session_callback(void *obj, struct kevent *kev __attribute__((unused)))
{
	session_t s = obj;

	session_reap(s);
	if (s->se_flags & SE_SHUTDOWN) {
		session_free(s);
	} else {
		session_launch(s);
	}
}

static void
session_reap(session_t s)
{
	char *line;
	int status;

	if (!launchd_assumes(waitpid(s->se_process, &status, 0) == s->se_process))
		return;

	if (WIFSIGNALED(status)) {
		syslog(LOG_WARNING, "%s port %s exited abnormally: %s",
				s->se_getty.path, s->se_device, strsignal(WTERMSIG(status)));
		s->se_flags |= SE_ONERROR; 
	} else if (WEXITSTATUS(status) == REALLY_EXIT_TO_CONSOLE) {
		/* WIFEXITED(status) assumed */
		s->se_flags |= SE_ONOPTION;
	} else {
		s->se_flags |= SE_ONERROR;
	}

	s->se_process = 0;
	line = s->se_device + sizeof(_PATH_DEV) - 1;

#ifdef notyet	
	if (logout(line))
		logwtmp(line, "", "");
#endif	
}

/*
 * This is an n-squared algorithm.  We hope it isn't run often...
 */
void
update_ttys(void)
{
	session_t sp;
	struct ttyent *typ;
	int session_index = 0;
	int devlen;

	devlen = sizeof(_PATH_DEV) - 1;
	while ((typ = getttyent())) {
		++session_index;

		TAILQ_FOREACH(sp, &sessions, tqe) {
			if (strcmp(typ->ty_name, sp->se_device + devlen) == 0)
				break;
		}

		if (sp == NULL) {
			session_new(session_index, typ);
			continue;
		}

		if (sp->se_index != session_index) {
			syslog(LOG_INFO, "port %s changed utmp index from %d to %d",
			       sp->se_device, sp->se_index,
			       session_index);
			sp->se_index = session_index;
		}

		if ((typ->ty_status & TTY_ON) == 0 ||
		    typ->ty_getty == 0) {
			session_free(sp);
			continue;
		}

		sp->se_flags &= ~SE_SHUTDOWN;

		if (setupargv(sp, typ) == 0) {
			syslog(LOG_WARNING, "can't parse getty for port %s",
				sp->se_device);
			session_free(sp);
		}
	}

	endttyent();
}

/*
 * Block further logins.
 */
void
catatonia(void)
{
	session_t s;

	TAILQ_FOREACH(s, &sessions, tqe)
		s->se_flags |= SE_SHUTDOWN;
}

bool init_check_pid(pid_t p)
{
	session_t s;

	TAILQ_FOREACH(s, &sessions, tqe) {
		if (s->se_process == p)
			return true;
	}

	if (single_user_pid == p)
		return true;

	if (runcom_pid == p)
		return true;

	return false;
}

bool
should_fsck(void)
{
	struct statfs sfs;
	bool r = true;

	if (launchd_assumes(statfs("/", &sfs) != -1)) {
		if (!(sfs.f_flags & MNT_RDONLY)) {
			r = false;
		}
	}
	
	return r;
}
