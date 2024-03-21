/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2017, 2020-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_EXEC_H
#define SUDO_EXEC_H

/*
 * Older systems may not support MSG_WAITALL but it shouldn't really be needed.
 */
#ifndef MSG_WAITALL
# define MSG_WAITALL 0
#endif

/*
 * Linux-specific wait flag used with ptrace(2).
 */
#ifndef __WALL
# define __WALL 0
#endif

/*
 * Some older systems support siginfo but predate SI_USER.
 */
#ifdef SI_USER
# define USER_SIGNALED(_info) ((_info) != NULL && (_info)->si_code == SI_USER)
#else
# define USER_SIGNALED(_info) ((_info) != NULL && (_info)->si_code <= 0)
#endif

struct user_details;
struct command_details;
struct command_status;
struct sudo_event_base;
struct stat;

/*
 * Closure passed to I/O event callbacks.
 */
struct exec_closure {
    struct command_details *details;
    struct sudo_event_base *evbase;
    struct sudo_event *backchannel_event;
    struct sudo_event *fwdchannel_event;
    struct sudo_event *sigint_event;
    struct sudo_event *sigquit_event;
    struct sudo_event *sigtstp_event;
    struct sudo_event *sigterm_event;
    struct sudo_event *sighup_event;
    struct sudo_event *sigalrm_event;
    struct sudo_event *sigpipe_event;
    struct sudo_event *sigusr1_event;
    struct sudo_event *sigusr2_event;
    struct sudo_event *sigchld_event;
    struct sudo_event *sigcont_event;
    struct sudo_event *siginfo_event;
    struct sudo_event *sigwinch_event;
    struct command_status *cstat;
    char *ptyname;
    void *intercept;
    pid_t sudo_pid;
    pid_t monitor_pid;
    pid_t cmnd_pid;
    pid_t ppgrp;
    int rows;
    int cols;
    bool foreground;
    bool term_raw;
};

/*
 * I/O buffer with associated read/write events and a logging action.
 * Used to, e.g. pass data from the pty to the user's terminal
 * and any I/O logging plugins.
 */
struct io_buffer;
typedef bool (*sudo_io_action_t)(const char *, unsigned int, struct io_buffer *);
struct io_buffer {
    SLIST_ENTRY(io_buffer) entries;
    struct exec_closure *ec;
    struct sudo_event *revent;
    struct sudo_event *wevent;
    sudo_io_action_t action;
    unsigned int len; /* buffer length (how much produced) */
    unsigned int off; /* write position (how much already consumed) */
    char buf[64 * 1024];
};
SLIST_HEAD(io_buffer_list, io_buffer);

/*
 * Indices into io_fds[] when logging I/O.
 */
#define SFD_STDIN	0
#define SFD_STDOUT	1
#define SFD_STDERR	2
#define SFD_LEADER	3
#define SFD_FOLLOWER	4
#define SFD_USERTTY	5

/* Evaluates to true if the event has /dev/tty as its fd. */
#define USERTTY_EVENT(_ev)	(sudo_ev_get_fd((_ev)) == io_fds[SFD_USERTTY])

/*
 * Special values to indicate whether continuing in foreground or background.
 */
#define SIGCONT_FG	-2
#define SIGCONT_BG	-3

/*
 * Positions in saved_signals[]
 */
#define SAVED_SIGALRM	 0
#define SAVED_SIGCHLD	 1
#define SAVED_SIGCONT	 2
#define SAVED_SIGHUP	 3
#define SAVED_SIGINT	 4
#define SAVED_SIGPIPE	 5
#define SAVED_SIGQUIT	 6
#define SAVED_SIGTERM	 7
#define SAVED_SIGTSTP	 8
#define SAVED_SIGTTIN	 9
#define SAVED_SIGTTOU	10
#define SAVED_SIGUSR1	11
#define SAVED_SIGUSR2	12

/*
 * Error codes for sesh
 */
#define SESH_SUCCESS	    0		/* successful operation */
#define SESH_ERR_FAILURE    1		/* unspecified error */
#define SESH_ERR_KILLED     2		/* killed by a signal */
#define SESH_ERR_INVALID    30		/* invalid -e arg value */
#define SESH_ERR_BAD_PATHS  31		/* odd number of paths */
#define SESH_ERR_NO_FILES   32		/* copy error, no files copied */
#define SESH_ERR_SOME_FILES 33		/* copy error, some files copied */

#define INTERCEPT_FD_MIN    64		/* minimum fd so shell won't close it */
#define MESSAGE_SIZE_MAX    2097152	/* 2Mib max intercept message size */

union sudo_token_un {
    unsigned char u8[16];
    unsigned int u32[4];
    unsigned long long u64[2];
};

#define sudo_token_isset(_t) ((_t).u64[0] || (_t).u64[1])

/*
 * Use ptrace-based intercept (using seccomp) on Linux if possible.
 * On MIPS we can't change the syscall return and only support log_subcmds.
 */
#if defined(_PATH_SUDO_INTERCEPT) && defined(__linux__)
# if defined(HAVE_DECL_SECCOMP_MODE_FILTER) && HAVE_DECL_SECCOMP_MODE_FILTER
#  if defined(__x86_64__) || defined(__i386__) || defined(__aarch64__) || defined(__arm__) || defined(__mips__) || defined(__powerpc__) || (defined(__riscv) && __riscv_xlen == 64) || defined(__s390__)
#   ifndef HAVE_PTRACE_INTERCEPT
#    define HAVE_PTRACE_INTERCEPT 1
#   endif /* HAVE_PTRACE_INTERCEPT */
#  endif /* __amd64__ || __i386__ || __aarch64__ || __riscv || __s390__ */
# endif /* HAVE_DECL_SECCOMP_MODE_FILTER */
#endif /* _PATH_SUDO_INTERCEPT && __linux__ */

/* exec.c */
struct stat;
void exec_cmnd(struct command_details *details, sigset_t *mask, int intercept_fd, int errfd);
void terminate_command(pid_t pid, bool use_pgrp);
bool sudo_terminated(struct command_status *cstat);
void free_exec_closure(struct exec_closure *ec);
bool fd_matches_tty(int fd, struct stat *tty_sb, struct stat *fd_sb);

/* exec_common.c */
int sudo_execve(int fd, const char *path, char *const argv[], char *envp[], int intercept_fd, unsigned int flags);
char **disable_execute(char *envp[], const char *dso);
char **enable_monitor(char *envp[], const char *dso);

/* exec_intercept.c */
void *intercept_setup(int fd, struct sudo_event_base *evbase, const struct command_details *details);
void intercept_cleanup(struct exec_closure *ec);

/* exec_iolog.c */
bool log_ttyin(const char *buf, unsigned int n, struct io_buffer *iob);
bool log_stdin(const char *buf, unsigned int n, struct io_buffer *iob);
bool log_ttyout(const char *buf, unsigned int n, struct io_buffer *iob);
bool log_stdout(const char *buf, unsigned int n, struct io_buffer *iob);
bool log_stderr(const char *buf, unsigned int n, struct io_buffer *iob);
void log_suspend(void *v, int signo);
void log_winchange(struct exec_closure *ec, unsigned int rows, unsigned int cols);
void io_buf_new(int rfd, int wfd, bool (*action)(const char *, unsigned int, struct io_buffer *), void (*read_cb)(int fd, int what, void *v), void (*write_cb)(int fd, int what, void *v), struct exec_closure *ec);
int safe_close(int fd);
void ev_free_by_fd(struct sudo_event_base *evbase, int fd);
void free_io_bufs(void);
void add_io_events(struct exec_closure *ec);
void del_io_events(bool nonblocking);
void init_ttyblock(void);

/* exec_nopty.c */
void exec_nopty(struct command_details *details, const struct user_details *user_details, struct sudo_event_base *evbase, struct command_status *cstat);

/* exec_pty.c */
bool exec_pty(struct command_details *details, const struct user_details *user_details, struct sudo_event_base *evbase, struct command_status *cstat);
extern int io_fds[6];

/* exec_monitor.c */
int exec_monitor(struct command_details *details, sigset_t *omask, bool foreground, int backchannel, int intercept_fd);

/* utmp.c */
bool utmp_login(const char *from_line, const char *to_line, int ttyfd,
    const char *user);
bool utmp_logout(const char *line, int status);

/* exec_preload.c */
char **sudo_preload_dso(char *const envp[], const char *dso_file, int intercept_fd);
char **sudo_preload_dso_mmap(char *const envp[], const char *dso_file, int intercept_fd);

/* exec_ptrace.c */
bool exec_ptrace_stopped(pid_t pid, int status, void *intercept);
bool set_exec_filter(void);
int exec_ptrace_seize(pid_t child);

/* suspend_parent.c */
void sudo_suspend_parent(int signo, pid_t my_pid, pid_t my_pgrp, pid_t cmnd_pid, void *closure, void (*callback)(void *, int));

#endif /* SUDO_EXEC_H */
