/*
 * Copyright (c) 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * Test program to exercise seccomp(2) and ptrace(2) intercept code.
 *
 * Usage: test_ptrace [-d 1-3] [command]
 */

/* Ignore architecture restrictions and define this unilaterally. */
#define HAVE_PTRACE_INTERCEPT
#include "exec_ptrace.c"

static sig_atomic_t got_sigchld;
static int debug;
int sudo_debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;

sudo_dso_public int main(int argc, char *argv[]);

static void
handler(int signo)
{
    if (signo == SIGCHLD)
	got_sigchld = 1;
}

void
intercept_closure_reset(struct intercept_closure *closure)
{
    memset(closure, 0, sizeof(*closure));
}

bool
intercept_check_policy(const char *command, int argc, char **argv, int envc,
    char **envp, const char *runcwd, int *oldcwd, void *v)
{
    struct intercept_closure *closure = v;
    struct stat sb1, sb2;
    bool is_denied;
    debug_decl(intercept_check_policy, SUDO_DEBUG_EXEC);

    /* Fake policy decisions. */
    is_denied = stat(command, &sb1) == 0 && stat("/usr/bin/who", &sb2) == 0 &&
	sb1.st_ino == sb2.st_ino && sb1.st_dev == sb2.st_dev;
    if (is_denied) {
	sudo_debug_printf(SUDO_DEBUG_DIAG, "denied %s", command);
	closure->state = POLICY_REJECT;
    } else {
	sudo_debug_printf(SUDO_DEBUG_DIAG, "allowed %s", command);
	closure->state = POLICY_TEST;
    }
    *oldcwd = -1;

    debug_return_bool(true);
}

static void
init_debug_files(struct sudo_conf_debug_file_list *file_list,
    struct sudo_debug_file *file)
{
    debug_decl(init_debug_files, SUDO_DEBUG_EXEC);

    TAILQ_INIT(file_list);
    switch (debug) {
    case 0:
	debug_return;
    case 1:
	file->debug_flags = (char *)"exec@diag";
	break;
    case 2:
	file->debug_flags = (char *)"exec@info";
	break;
    default:
	file->debug_flags = (char *)"exec@debug";
	break;
    }
    file->debug_file = (char *)"/dev/stderr";
    TAILQ_INSERT_HEAD(file_list, file, entries);

    debug_return;
}

int
sudo_sigaction(int signo, struct sigaction *sa, struct sigaction *osa)
{
    return sigaction(signo, sa, osa);
}

int
main(int argc, char *argv[])
{
    struct sudo_conf_debug_file_list debug_files;
    struct sudo_debug_file debug_file;
    const char *base, *shell = _PATH_SUDO_BSHELL;
    struct intercept_closure closure = { 0 };
    const char *errstr;
    sigset_t blocked, empty;
    struct sigaction sa;
    pid_t child, my_pid, pid, my_pgrp;
    int ch, status;
    debug_decl_vars(main, SUDO_DEBUG_MAIN);

    initprogname(argc > 0 ? argv[0] : "test_ptrace");

    if (!have_seccomp_action("trap"))
	sudo_fatalx("SECCOMP_MODE_FILTER not available in this kernel");

    while ((ch = getopt(argc, argv, "d:")) != -1) {
	switch (ch) {
	case 'd':
	    debug = sudo_strtonum(optarg, 1, INT_MAX, &errstr);
	    if (errstr != NULL)
		sudo_fatalx(U_("%s: %s"), optarg, U_(errstr));
	    break;
	default:
	    fprintf(stderr, "usage: %s [-d 1-3] [command]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    if (argc > 0)
	shell = argv[0];
    base = strrchr(shell, '/');
    base = base ? base + 1 : shell;

    /* Set debug level based on the debug flag. */
    init_debug_files(&debug_files, &debug_file);
    sudo_debug_instance = sudo_debug_register(getprogname(),
	NULL, NULL, &debug_files, -1);
    if (sudo_debug_instance == SUDO_DEBUG_INSTANCE_ERROR)
	return EXIT_FAILURE;

    /* Block SIGCHLD and SIGUSR during critical section. */
    sigemptyset(&empty);
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGCHLD);
    sigaddset(&blocked, SIGUSR1);
    sigprocmask(SIG_BLOCK, &blocked, NULL);

    /* Signal handler sets a flag for SIGCHLD, nothing for SIGUSR1. */
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    /* Fork a shell. */
    my_pid = getpid();
    my_pgrp = getpgrp();
    child = fork();
    switch (child) {
    case -1:
	sudo_fatal("fork");
    case 0:
	/* child */
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
	    sudo_fatal("%s", "unable to set no_new_privs bit");
	if (!set_exec_filter())
	    _exit(EXIT_FAILURE);

	/* Suspend child until tracer seizes control and sends SIGUSR1. */
	sigsuspend(&empty);
	execl(shell, base, NULL);
	sudo_fatal("execl");
    default:
	/* Parent attaches to child and allows it to continue. */
	if (exec_ptrace_seize(child) == -1)
	    return EXIT_FAILURE;
	break;
    }

    /* Wait for SIGCHLD. */
    for (;;) {
	sigsuspend(&empty);
	if (!got_sigchld)
	    continue;
	got_sigchld = 0;

	for (;;) {
	    do {
		pid = waitpid(-1, &status, __WALL|WNOHANG);
	    } while (pid == -1 && errno == EINTR);
	    if (pid <= 0) {
		if (pid == -1 && errno != ECHILD)
		    sudo_fatal("waitpid");
		/* No child to wait for. */
		break;
	    }

	    if (WIFEXITED(status)) {
		sudo_debug_printf(SUDO_DEBUG_DIAG, "%d: exited %d",
		    (int)pid, WEXITSTATUS(status));
		if (pid == child)
		    return WEXITSTATUS(status);
	    } else if (WIFSIGNALED(status)) {
		sudo_debug_printf(SUDO_DEBUG_DIAG, "%d: killed by signal %d",
		    (int)pid, WTERMSIG(status));
		if (pid == child)
		    return WTERMSIG(status) | 128;
	    } else if (WIFSTOPPED(status)) {
		if (exec_ptrace_stopped(pid, status, &closure)) {
		    if (pid == child) {
			sudo_suspend_parent(WSTOPSIG(status), my_pid,
			    my_pgrp, child, NULL, NULL);
			if (kill(child, SIGCONT) != 0)
			    sudo_warn("kill(%d, SIGCONT)", (int)child);
		    }
		}
	    } else {
		sudo_fatalx("%d: unknown status 0x%x", (int)pid, status);
	    }
	}
    }
}
