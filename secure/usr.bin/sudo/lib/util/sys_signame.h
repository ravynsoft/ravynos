/* public domain */

#include <config.h>
#include <sys/types.h>
#include <signal.h>
#include <sudo_compat.h>

int sudo_end_of_headers;
static char *sudo_sys_signame[NSIG];

#ifdef SIGHUP
    if (sudo_sys_signame[SIGHUP] == NULL)
	sudo_sys_signame[SIGHUP] = "HUP";
#endif
#ifdef SIGINT
    if (sudo_sys_signame[SIGINT] == NULL)
	sudo_sys_signame[SIGINT] = "INT";
#endif
#ifdef SIGQUIT
    if (sudo_sys_signame[SIGQUIT] == NULL)
	sudo_sys_signame[SIGQUIT] = "QUIT";
#endif
#ifdef SIGILL
    if (sudo_sys_signame[SIGILL] == NULL)
	sudo_sys_signame[SIGILL] = "ILL";
#endif
#ifdef SIGTRAP
    if (sudo_sys_signame[SIGTRAP] == NULL)
	sudo_sys_signame[SIGTRAP] = "TRAP";
#endif
#ifdef SIGABRT
    if (sudo_sys_signame[SIGABRT] == NULL)
	sudo_sys_signame[SIGABRT] = "ABRT";
#endif
#ifdef SIGIOT
    if (sudo_sys_signame[SIGIOT] == NULL)
	sudo_sys_signame[SIGIOT] = "IOT";
#endif
#ifdef SIGEMT
    if (sudo_sys_signame[SIGEMT] == NULL)
	sudo_sys_signame[SIGEMT] = "EMT";
#endif
#ifdef SIGFPE
    if (sudo_sys_signame[SIGFPE] == NULL)
	sudo_sys_signame[SIGFPE] = "FPE";
#endif
#ifdef SIGKILL
    if (sudo_sys_signame[SIGKILL] == NULL)
	sudo_sys_signame[SIGKILL] = "KILL";
#endif
#ifdef SIGBUS
    if (sudo_sys_signame[SIGBUS] == NULL)
	sudo_sys_signame[SIGBUS] = "BUS";
#endif
#ifdef SIGSEGV
    if (sudo_sys_signame[SIGSEGV] == NULL)
	sudo_sys_signame[SIGSEGV] = "SEGV";
#endif
#ifdef SIGSYS
    if (sudo_sys_signame[SIGSYS] == NULL)
	sudo_sys_signame[SIGSYS] = "SYS";
#endif
#ifdef SIGUNUSED
    if (sudo_sys_signame[SIGUNUSED] == NULL)
	sudo_sys_signame[SIGUNUSED] = "UNUSED";
#endif
#ifdef SIGPIPE
    if (sudo_sys_signame[SIGPIPE] == NULL)
	sudo_sys_signame[SIGPIPE] = "PIPE";
#endif
#ifdef SIGALRM
    if (sudo_sys_signame[SIGALRM] == NULL)
	sudo_sys_signame[SIGALRM] = "ALRM";
#endif
#ifdef SIGTERM
    if (sudo_sys_signame[SIGTERM] == NULL)
	sudo_sys_signame[SIGTERM] = "TERM";
#endif
#ifdef SIGSTKFLT
    if (sudo_sys_signame[SIGSTKFLT] == NULL)
	sudo_sys_signame[SIGSTKFLT] = "STKFLT";
#endif
#ifdef SIGIO
    if (sudo_sys_signame[SIGIO] == NULL)
	sudo_sys_signame[SIGIO] = "IO";
#endif
#ifdef SIGXCPU
    if (sudo_sys_signame[SIGXCPU] == NULL)
	sudo_sys_signame[SIGXCPU] = "XCPU";
#endif
#ifdef SIGXFSZ
    if (sudo_sys_signame[SIGXFSZ] == NULL)
	sudo_sys_signame[SIGXFSZ] = "XFSZ";
#endif
#ifdef SIGVTALRM
    if (sudo_sys_signame[SIGVTALRM] == NULL)
	sudo_sys_signame[SIGVTALRM] = "VTALRM";
#endif
#ifdef SIGPROF
    if (sudo_sys_signame[SIGPROF] == NULL)
	sudo_sys_signame[SIGPROF] = "PROF";
#endif
#ifdef SIGWINCH
    if (sudo_sys_signame[SIGWINCH] == NULL)
	sudo_sys_signame[SIGWINCH] = "WINCH";
#endif
#ifdef SIGLOST
    if (sudo_sys_signame[SIGLOST] == NULL)
	sudo_sys_signame[SIGLOST] = "LOST";
#endif
#ifdef SIGUSR1
    if (sudo_sys_signame[SIGUSR1] == NULL)
	sudo_sys_signame[SIGUSR1] = "USR1";
#endif
#ifdef SIGUSR2
    if (sudo_sys_signame[SIGUSR2] == NULL)
	sudo_sys_signame[SIGUSR2] = "USR2";
#endif
#ifdef SIGPWR
    if (sudo_sys_signame[SIGPWR] == NULL)
	sudo_sys_signame[SIGPWR] = "PWR";
#endif
#ifdef SIGPOLL
    if (sudo_sys_signame[SIGPOLL] == NULL)
	sudo_sys_signame[SIGPOLL] = "POLL";
#endif
#ifdef SIGSTOP
    if (sudo_sys_signame[SIGSTOP] == NULL)
	sudo_sys_signame[SIGSTOP] = "STOP";
#endif
#ifdef SIGTSTP
    if (sudo_sys_signame[SIGTSTP] == NULL)
	sudo_sys_signame[SIGTSTP] = "TSTP";
#endif
#ifdef SIGCONT
    if (sudo_sys_signame[SIGCONT] == NULL)
	sudo_sys_signame[SIGCONT] = "CONT";
#endif
#ifdef SIGCHLD
    if (sudo_sys_signame[SIGCHLD] == NULL)
	sudo_sys_signame[SIGCHLD] = "CHLD";
#endif
#ifdef SIGCLD
    if (sudo_sys_signame[SIGCLD] == NULL)
	sudo_sys_signame[SIGCLD] = "CLD";
#endif
#ifdef SIGTTIN
    if (sudo_sys_signame[SIGTTIN] == NULL)
	sudo_sys_signame[SIGTTIN] = "TTIN";
#endif
#ifdef SIGTTOU
    if (sudo_sys_signame[SIGTTOU] == NULL)
	sudo_sys_signame[SIGTTOU] = "TTOU";
#endif
#ifdef SIGINFO
    if (sudo_sys_signame[SIGINFO] == NULL)
	sudo_sys_signame[SIGINFO] = "INFO";
#endif
#ifdef SIGURG
    if (sudo_sys_signame[SIGURG] == NULL)
	sudo_sys_signame[SIGURG] = "URG";
#endif
#ifdef SIGWAITING
    if (sudo_sys_signame[SIGWAITING] == NULL)
	sudo_sys_signame[SIGWAITING] = "WAITING";
#endif
#ifdef SIGLWP
    if (sudo_sys_signame[SIGLWP] == NULL)
	sudo_sys_signame[SIGLWP] = "LWP";
#endif
#ifdef SIGFREEZE
    if (sudo_sys_signame[SIGFREEZE] == NULL)
	sudo_sys_signame[SIGFREEZE] = "FREEZE";
#endif
#ifdef SIGTHAW
    if (sudo_sys_signame[SIGTHAW] == NULL)
	sudo_sys_signame[SIGTHAW] = "THAW";
#endif
#ifdef SIGCANCEL
    if (sudo_sys_signame[SIGCANCEL] == NULL)
	sudo_sys_signame[SIGCANCEL] = "CANCEL";
#endif
