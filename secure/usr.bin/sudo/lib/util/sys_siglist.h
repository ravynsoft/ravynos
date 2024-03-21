/* public domain */

#include <config.h>
#include <sys/types.h>
#include <signal.h>
#include <sudo_compat.h>

int sudo_end_of_headers;
static char *sudo_sys_siglist[NSIG];

#ifdef SIGHUP
    if (sudo_sys_siglist[SIGHUP] == NULL)
	sudo_sys_siglist[SIGHUP] = "Hangup";
#endif
#ifdef SIGINT
    if (sudo_sys_siglist[SIGINT] == NULL)
	sudo_sys_siglist[SIGINT] = "Interrupt";
#endif
#ifdef SIGQUIT
    if (sudo_sys_siglist[SIGQUIT] == NULL)
	sudo_sys_siglist[SIGQUIT] = "Quit";
#endif
#ifdef SIGILL
    if (sudo_sys_siglist[SIGILL] == NULL)
	sudo_sys_siglist[SIGILL] = "Illegal instruction";
#endif
#ifdef SIGTRAP
    if (sudo_sys_siglist[SIGTRAP] == NULL)
	sudo_sys_siglist[SIGTRAP] = "Trace trap";
#endif
#ifdef SIGABRT
    if (sudo_sys_siglist[SIGABRT] == NULL)
	sudo_sys_siglist[SIGABRT] = "Abort";
#endif
#ifdef SIGIOT
    if (sudo_sys_siglist[SIGIOT] == NULL)
	sudo_sys_siglist[SIGIOT] = "IOT instruction";
#endif
#ifdef SIGEMT
    if (sudo_sys_siglist[SIGEMT] == NULL)
	sudo_sys_siglist[SIGEMT] = "EMT trap";
#endif
#ifdef SIGFPE
    if (sudo_sys_siglist[SIGFPE] == NULL)
	sudo_sys_siglist[SIGFPE] = "Floating point exception";
#endif
#ifdef SIGKILL
    if (sudo_sys_siglist[SIGKILL] == NULL)
	sudo_sys_siglist[SIGKILL] = "Killed";
#endif
#ifdef SIGBUS
    if (sudo_sys_siglist[SIGBUS] == NULL)
	sudo_sys_siglist[SIGBUS] = "Bus error";
#endif
#ifdef SIGSEGV
    if (sudo_sys_siglist[SIGSEGV] == NULL)
	sudo_sys_siglist[SIGSEGV] = "Memory fault";
#endif
#ifdef SIGSYS
    if (sudo_sys_siglist[SIGSYS] == NULL)
	sudo_sys_siglist[SIGSYS] = "Bad system call";
#endif
#ifdef SIGUNUSED
    if (sudo_sys_siglist[SIGUNUSED] == NULL)
	sudo_sys_siglist[SIGUNUSED] = "Unused";
#endif
#ifdef SIGPIPE
    if (sudo_sys_siglist[SIGPIPE] == NULL)
	sudo_sys_siglist[SIGPIPE] = "Broken pipe";
#endif
#ifdef SIGALRM
    if (sudo_sys_siglist[SIGALRM] == NULL)
	sudo_sys_siglist[SIGALRM] = "Alarm clock";
#endif
#ifdef SIGTERM
    if (sudo_sys_siglist[SIGTERM] == NULL)
	sudo_sys_siglist[SIGTERM] = "Terminated";
#endif
#ifdef SIGSTKFLT
    if (sudo_sys_siglist[SIGSTKFLT] == NULL)
	sudo_sys_siglist[SIGSTKFLT] = "Stack fault";
#endif
#ifdef SIGIO
    if (sudo_sys_siglist[SIGIO] == NULL)
	sudo_sys_siglist[SIGIO] = "I/O possible";
#endif
#ifdef SIGXCPU
    if (sudo_sys_siglist[SIGXCPU] == NULL)
	sudo_sys_siglist[SIGXCPU] = "CPU time limit exceeded";
#endif
#ifdef SIGXFSZ
    if (sudo_sys_siglist[SIGXFSZ] == NULL)
	sudo_sys_siglist[SIGXFSZ] = "File size limit exceeded";
#endif
#ifdef SIGVTALRM
    if (sudo_sys_siglist[SIGVTALRM] == NULL)
	sudo_sys_siglist[SIGVTALRM] = "Virtual timer expired";
#endif
#ifdef SIGPROF
    if (sudo_sys_siglist[SIGPROF] == NULL)
	sudo_sys_siglist[SIGPROF] = "Profiling timer expired";
#endif
#ifdef SIGWINCH
    if (sudo_sys_siglist[SIGWINCH] == NULL)
	sudo_sys_siglist[SIGWINCH] = "Window size change";
#endif
#ifdef SIGLOST
    if (sudo_sys_siglist[SIGLOST] == NULL)
	sudo_sys_siglist[SIGLOST] = "File lock lost";
#endif
#ifdef SIGUSR1
    if (sudo_sys_siglist[SIGUSR1] == NULL)
	sudo_sys_siglist[SIGUSR1] = "User defined signal 1";
#endif
#ifdef SIGUSR2
    if (sudo_sys_siglist[SIGUSR2] == NULL)
	sudo_sys_siglist[SIGUSR2] = "User defined signal 2";
#endif
#ifdef SIGPWR
    if (sudo_sys_siglist[SIGPWR] == NULL)
	sudo_sys_siglist[SIGPWR] = "Power-fail/Restart";
#endif
#ifdef SIGPOLL
    if (sudo_sys_siglist[SIGPOLL] == NULL)
	sudo_sys_siglist[SIGPOLL] = "Pollable event occurred";
#endif
#ifdef SIGSTOP
    if (sudo_sys_siglist[SIGSTOP] == NULL)
	sudo_sys_siglist[SIGSTOP] = "Stopped (signal)";
#endif
#ifdef SIGTSTP
    if (sudo_sys_siglist[SIGTSTP] == NULL)
	sudo_sys_siglist[SIGTSTP] = "Stopped";
#endif
#ifdef SIGCONT
    if (sudo_sys_siglist[SIGCONT] == NULL)
	sudo_sys_siglist[SIGCONT] = "Continued";
#endif
#ifdef SIGCHLD
    if (sudo_sys_siglist[SIGCHLD] == NULL)
	sudo_sys_siglist[SIGCHLD] = "Child exited";
#endif
#ifdef SIGCLD
    if (sudo_sys_siglist[SIGCLD] == NULL)
	sudo_sys_siglist[SIGCLD] = "Child exited";
#endif
#ifdef SIGTTIN
    if (sudo_sys_siglist[SIGTTIN] == NULL)
	sudo_sys_siglist[SIGTTIN] = "Stopped (tty input)";
#endif
#ifdef SIGTTOU
    if (sudo_sys_siglist[SIGTTOU] == NULL)
	sudo_sys_siglist[SIGTTOU] = "Stopped (tty output)";
#endif
#ifdef SIGINFO
    if (sudo_sys_siglist[SIGINFO] == NULL)
	sudo_sys_siglist[SIGINFO] = "Information request";
#endif
#ifdef SIGURG
    if (sudo_sys_siglist[SIGURG] == NULL)
	sudo_sys_siglist[SIGURG] = "Urgent I/O condition";
#endif
#ifdef SIGWAITING
    if (sudo_sys_siglist[SIGWAITING] == NULL)
	sudo_sys_siglist[SIGWAITING] = "No runnable LWPs";
#endif
#ifdef SIGLWP
    if (sudo_sys_siglist[SIGLWP] == NULL)
	sudo_sys_siglist[SIGLWP] = "Inter-LWP signal";
#endif
#ifdef SIGFREEZE
    if (sudo_sys_siglist[SIGFREEZE] == NULL)
	sudo_sys_siglist[SIGFREEZE] = "Checkpoint freeze";
#endif
#ifdef SIGTHAW
    if (sudo_sys_siglist[SIGTHAW] == NULL)
	sudo_sys_siglist[SIGTHAW] = "Checkpoint thaw";
#endif
#ifdef SIGCANCEL
    if (sudo_sys_siglist[SIGCANCEL] == NULL)
	sudo_sys_siglist[SIGCANCEL] = "Thread cancellation";
#endif
