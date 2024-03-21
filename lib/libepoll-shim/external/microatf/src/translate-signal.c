#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
	if (argc != 2) {
		exit(1);
	}

	int sig = atoi(argv[1]);
	if (sig == 0) {
		exit(1);
	}

	switch (sig) {
#ifdef SIGSEGV
	case SIGSEGV:
		fprintf(stdout, "Segmentation fault\n");
		break;
#endif
#ifdef SIGBUS
#if !defined(SIGSEGV) || SIGBUS != SIGSEGV
	case SIGBUS:
		fprintf(stdout, "Bus error\n");
		break;
#endif
#endif
#ifdef SIGFPE
	case SIGFPE:
		fprintf(stdout, "Floating-point exception\n");
		break;
#endif
#ifdef SIGILL
	case SIGILL:
		fprintf(stdout, "Illegal instruction\n");
		break;
#endif
#ifdef SIGINT
	case SIGINT:
		fprintf(stdout, "User interrupt\n");
		break;
#endif
#ifdef SIGABRT
	case SIGABRT:
		fprintf(stdout, "Subprocess aborted\nChild aborted\n");
		break;
#endif
#ifdef SIGKILL
	case SIGKILL:
		fprintf(stdout, "Subprocess killed\nChild killed\n");
		break;
#endif
#ifdef SIGTERM
	case SIGTERM:
		fprintf(stdout, "Subprocess terminated\nChild terminated\n");
		break;
#endif
#ifdef SIGHUP
	case SIGHUP:
		fprintf(stdout, "SIGHUP\n");
		break;
#endif
#ifdef SIGQUIT
	case SIGQUIT:
		fprintf(stdout, "SIGQUIT\n");
		break;
#endif
#ifdef SIGTRAP
	case SIGTRAP:
		fprintf(stdout, "SIGTRAP\n");
		break;
#endif
#ifdef SIGIOT
#if !defined(SIGABRT) || SIGIOT != SIGABRT
	case SIGIOT:
		fprintf(stdout, "SIGIOT\n");
		break;
#endif
#endif
#ifdef SIGUSR1
	case SIGUSR1:
		fprintf(stdout, "SIGUSR1\n");
		break;
#endif
#ifdef SIGUSR2
	case SIGUSR2:
		fprintf(stdout, "SIGUSR2\n");
		break;
#endif
#ifdef SIGPIPE
	case SIGPIPE:
		fprintf(stdout, "SIGPIPE\n");
		break;
#endif
#ifdef SIGALRM
	case SIGALRM:
		fprintf(stdout, "SIGALRM\n");
		break;
#endif
#ifdef SIGSTKFLT
	case SIGSTKFLT:
		fprintf(stdout, "SIGSTKFLT\n");
		break;
#endif
#ifdef SIGCHLD
	case SIGCHLD:
		fprintf(stdout, "SIGCHLD\n");
		break;
#elif defined(SIGCLD)
	case SIGCLD:
		fprintf(stdout, "SIGCLD\n");
		break;
#endif
#ifdef SIGCONT
	case SIGCONT:
		fprintf(stdout, "SIGCONT\n");
		break;
#endif
#ifdef SIGSTOP
	case SIGSTOP:
		fprintf(stdout, "SIGSTOP\n");
		break;
#endif
#ifdef SIGTSTP
	case SIGTSTP:
		fprintf(stdout, "SIGTSTP\n");
		break;
#endif
#ifdef SIGTTIN
	case SIGTTIN:
		fprintf(stdout, "SIGTTIN\n");
		break;
#endif
#ifdef SIGTTOU
	case SIGTTOU:
		fprintf(stdout, "SIGTTOU\n");
		break;
#endif
#ifdef SIGURG
	case SIGURG:
		fprintf(stdout, "SIGURG\n");
		break;
#endif
#ifdef SIGXCPU
	case SIGXCPU:
		fprintf(stdout, "SIGXCPU\n");
		break;
#endif
#ifdef SIGXFSZ
	case SIGXFSZ:
		fprintf(stdout, "SIGXFSZ\n");
		break;
#endif
#ifdef SIGVTALRM
	case SIGVTALRM:
		fprintf(stdout, "SIGVTALRM\n");
		break;
#endif
#ifdef SIGPROF
	case SIGPROF:
		fprintf(stdout, "SIGPROF\n");
		break;
#endif
#ifdef SIGWINCH
	case SIGWINCH:
		fprintf(stdout, "SIGWINCH\n");
		break;
#endif
#ifdef SIGPOLL
	case SIGPOLL:
		fprintf(stdout, "SIGPOLL\n");
		break;
#endif
#ifdef SIGIO
#if !defined(SIGPOLL) || SIGIO != SIGPOLL
	case SIGIO:
		fprintf(stdout, "SIGIO\n");
		break;
#endif
#endif
#ifdef SIGPWR
	case SIGPWR:
		fprintf(stdout, "SIGPWR\n");
		break;
#endif
#ifdef SIGSYS
	case SIGSYS:
		fprintf(stdout, "SIGSYS\n");
		break;
#endif
#ifdef SIGUNUSED
#if !defined(SIGSYS) || SIGUNUSED != SIGSYS
	case SIGUNUSED:
		fprintf(stdout, "SIGUNUSED\n");
		break;
#endif
#endif
	default:
		fprintf(stdout, "Signal %d\n", sig);
		break;
	}

	fflush(stdout);
}
