#define _GNU_SOURCE

#include <atf-c.h>

#include <sys/types.h>

#include <sys/param.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <errno.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/signalfd.h>

#include "atf-c-leakcheck.h"

ATF_TC_WITHOUT_HEAD(signalfd__simple_signalfd);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__simple_signalfd, tcptr)
{
	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	kill(getpid(), SIGINT);

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		int n;
		ATF_REQUIRE_MSG((n = poll(&pfd, 1, -1)) == 1, "%d", n);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}

	s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	ATF_REQUIRE(s == (ssize_t)sizeof(struct signalfd_siginfo));

	ATF_REQUIRE(fdsi.ssi_signo == SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}

	ATF_REQUIRE(close(sfd) == 0);
}

static void *
sleep_then_kill(void *arg)
{
	(void)arg;
	usleep(300000);
	kill(getpid(), SIGINT);
	return NULL;
}

ATF_TC_WITHOUT_HEAD(signalfd__blocking_read);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__blocking_read, tcptr)
{
	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	pthread_t writer_thread;
	ATF_REQUIRE(
	    pthread_create(&writer_thread, NULL, sleep_then_kill, NULL) == 0);

	s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	ATF_REQUIRE(s == (ssize_t)sizeof(struct signalfd_siginfo));

	ATF_REQUIRE(fdsi.ssi_signo == SIGINT);

	ATF_REQUIRE(pthread_join(writer_thread, NULL) == 0);

	ATF_REQUIRE(close(sfd) == 0);
}

ATF_TC_WITHOUT_HEAD(signalfd__nonblocking_read);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__nonblocking_read, tcptr)
{
	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi;
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	sfd = signalfd(-1, &mask, SFD_NONBLOCK);
	ATF_REQUIRE(sfd >= 0);

	ATF_REQUIRE_ERRNO(EAGAIN,
	    read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) < 0);

	pthread_t writer_thread;
	ATF_REQUIRE(
	    pthread_create(&writer_thread, NULL, sleep_then_kill, NULL) == 0);

	int read_counter = 0;
	do {
		++read_counter;
		s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	} while (s < 0 && errno == EAGAIN);

	ATF_REQUIRE(s == (ssize_t)sizeof(struct signalfd_siginfo));
	ATF_REQUIRE_MSG(read_counter > 10, "%d", read_counter);

	ATF_REQUIRE(fdsi.ssi_signo == SIGINT);

	ATF_REQUIRE(pthread_join(writer_thread, NULL) == 0);

	ATF_REQUIRE(close(sfd) == 0);
}

ATF_TC_WITHOUT_HEAD(signalfd__multiple_signals);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__multiple_signals, tcptr)
{
	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi[16];
	ssize_t s;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	kill(getpid(), SIGINT);
	kill(getpid(), SIGUSR1);
	kill(getpid(), SIGUSR2);

	s = read(sfd, &fdsi, sizeof(fdsi));
	ATF_REQUIRE(s == 3 * (ssize_t)sizeof(struct signalfd_siginfo));

	printf("%d %d %d\n", /**/
	    fdsi[0].ssi_signo, fdsi[1].ssi_signo, fdsi[2].ssi_signo);

	ATF_REQUIRE(			   /**/
	    fdsi[0].ssi_signo == SIGINT || /**/
	    fdsi[1].ssi_signo == SIGINT || /**/
	    fdsi[2].ssi_signo == SIGINT);
	ATF_REQUIRE(			    /**/
	    fdsi[0].ssi_signo == SIGUSR1 || /**/
	    fdsi[1].ssi_signo == SIGUSR1 || /**/
	    fdsi[2].ssi_signo == SIGUSR1);
	ATF_REQUIRE(			    /**/
	    fdsi[0].ssi_signo == SIGUSR2 || /**/
	    fdsi[1].ssi_signo == SIGUSR2 || /**/
	    fdsi[2].ssi_signo == SIGUSR2);

	ATF_REQUIRE(close(sfd) == 0);
}

ATF_TC_WITHOUT_HEAD(signalfd__modify_signalmask);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__modify_signalmask, tcptr)
{
	sigset_t mask;
	int sfd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	sigaddset(&mask, SIGUSR1);

#ifndef __linux__
	atf_tc_expect_fail("modifying an existing signalfd descriptor is "
			   "not currently supported");
#endif

	ATF_REQUIRE(sfd == signalfd(sfd, &mask, 0));

	ATF_REQUIRE(close(sfd) == 0);
}

ATF_TC_WITHOUT_HEAD(signalfd__argument_checks);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__argument_checks, tcptr)
{
	sigset_t mask;
	int sfd;

	int const invalid_fd = 0xbeef;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);
	ATF_REQUIRE(close(sfd) == 0);

	int fds[2];
	ATF_REQUIRE(pipe2(fds, O_CLOEXEC) == 0);

	ATF_REQUIRE_ERRNO(EBADF, signalfd(invalid_fd, &mask, 0));
	ATF_REQUIRE(signalfd(invalid_fd, NULL, 0));
	/* Linux 5.10 returns EFAULT. */
	ATF_REQUIRE(errno == EINVAL || errno == EFAULT);
	ATF_REQUIRE(signalfd(-1, NULL, 0));
	/* Linux 5.10 returns EFAULT. */
	ATF_REQUIRE(errno == EINVAL || errno == EFAULT);

	ATF_REQUIRE_ERRNO(EINVAL, signalfd(fds[0], &mask, 0));
	ATF_REQUIRE(signalfd(fds[0], NULL, 0));
	/* Linux 5.10 returns EFAULT. */
	ATF_REQUIRE(errno == EINVAL || errno == EFAULT);

	ATF_REQUIRE_ERRNO(EINVAL, signalfd(invalid_fd, &mask, 42));

	ATF_REQUIRE_ERRNO(EBADF, signalfd(-2, &mask, 0));

	sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	int dummy;
	ssize_t s = read(sfd, &dummy, sizeof(int));
	ATF_REQUIRE_ERRNO(EINVAL, s < 0);

	ATF_REQUIRE(close(sfd) == 0);

	struct signalfd_siginfo siginfo;
	s = read(sfd, &siginfo, sizeof(struct signalfd_siginfo));
	ATF_REQUIRE_ERRNO(EBADF, s < 0);

	ATF_REQUIRE(close(fds[0]) == 0);
	ATF_REQUIRE(close(fds[1]) == 0);
}

static sig_atomic_t volatile got_sigint = 0;
static void
sigint_handler(int signo)
{
	(void)signo;
	got_sigint = 1;
}

ATF_TC_WITHOUT_HEAD(signalfd__signal_disposition);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__signal_disposition, tcptr)
{
	/*
	 * Check that signalfd's don't work when signals are handled by a
	 * different mechanism.
	 */

	sigset_t mask;
	int sfd;
	struct signalfd_siginfo fdsi;

	ATF_REQUIRE(signal(SIGINT, sigint_handler) != SIG_ERR);

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	sfd = signalfd(-1, &mask, SFD_NONBLOCK);
	ATF_REQUIRE(sfd >= 0);

	ATF_REQUIRE_ERRNO(EAGAIN, /**/
	    read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) < 0);

	ATF_REQUIRE(kill(getpid(), SIGINT) == 0);
	ATF_REQUIRE(got_sigint == 1);

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}

	ATF_REQUIRE_ERRNO(EAGAIN, /**/
	    read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) < 0);

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}

	ATF_REQUIRE(close(sfd) == 0);
}

ATF_TC_WITHOUT_HEAD(signalfd__sigwaitinfo);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__sigwaitinfo, tcptr)
{
	sigset_t mask;

	sigemptyset(&mask);

	int sfd2 = signalfd(-1, &mask, SFD_NONBLOCK);
	ATF_REQUIRE(sfd2 >= 0);

	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGINT);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	kill(getpid(), SIGINT);
	kill(getpid(), SIGUSR1);
	kill(getpid(), SIGUSR2);

	int sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	int sfd3 = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd3 >= 0);

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}
	{
		struct pollfd pfd = { .fd = sfd3, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}

	{
#if defined(__OpenBSD__) || defined(__APPLE__)
		sigset_t mask2;
		sigemptyset(&mask2);
		sigaddset(&mask2, SIGINT);
		int s;
		ATF_REQUIRE(sigwait(&mask2, &s) == 0);
		ATF_REQUIRE(s == SIGINT);
#else
		siginfo_t siginfo;
		int s = sigwaitinfo(&mask, &siginfo);
		ATF_REQUIRE(s == SIGINT);
		ATF_REQUIRE(siginfo.si_signo == SIGINT);
#endif
	}

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}
	{
		struct pollfd pfd = { .fd = sfd3, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}

	{
#if defined(__OpenBSD__) || defined(__APPLE__)
		sigset_t mask2;
		sigemptyset(&mask2);
		sigaddset(&mask2, SIGUSR1);
		int s;
		ATF_REQUIRE(sigwait(&mask2, &s) == 0);
		ATF_REQUIRE(s == SIGUSR1);
#else
		siginfo_t siginfo;
		int s = sigtimedwait(&mask, &siginfo, /**/
		    &(struct timespec) { 0, 0 });
		ATF_REQUIRE(s == SIGUSR1);
		ATF_REQUIRE(siginfo.si_signo == SIGUSR1);
#endif
	}

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}
	{
		struct pollfd pfd = { .fd = sfd3, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, -1) == 1);
		ATF_REQUIRE(pfd.revents == POLLIN);
	}

	{
		struct signalfd_siginfo fdsi;
		ATF_REQUIRE(
		    read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) >= 0);
		ATF_REQUIRE(fdsi.ssi_signo == SIGUSR2);
	}

	ATF_REQUIRE(sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);

	{
		struct pollfd pfd = { .fd = sfd, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}
	{
		struct pollfd pfd = { .fd = sfd3, .events = POLLIN };
		ATF_REQUIRE(poll(&pfd, 1, 0) == 0);
	}

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
	kill(getpid(), SIGUSR1);
	struct signalfd_siginfo fdsi;
	ATF_REQUIRE_ERRNO(EAGAIN, /**/
	    read(sfd2, &fdsi, sizeof(struct signalfd_siginfo)) < 0);
	ATF_REQUIRE(read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) >= 0);
	ATF_REQUIRE(sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);

	ATF_REQUIRE(close(sfd) == 0);
	ATF_REQUIRE(close(sfd2) == 0);
	ATF_REQUIRE(close(sfd3) == 0);
}

ATF_TC_WITHOUT_HEAD(signalfd__sigwait_openbsd);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__sigwait_openbsd, tcptr)
{
#ifndef __OpenBSD__
	atf_tc_skip("test only valid for OpenBSD");
#else
	extern int __thrsigdivert(sigset_t set, siginfo_t * info,
	    struct timespec const *timeout);

	sigset_t mask;

	ATF_REQUIRE(sigemptyset(&mask) == 0);
	ATF_REQUIRE(sigaddset(&mask, SIGINT) == 0);
	ATF_REQUIRE(sigaddset(&mask, SIGUSR1) == 0);
	ATF_REQUIRE(sigaddset(&mask, SIGUSR2) == 0);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	{
		int s = __thrsigdivert(mask, NULL, &(struct timespec) { 0, 0 });
		/*
		 * EAGAIN is correct, but the following shouldn't appear in
		 * dmesg:
		 *
		 * sigwait: trying to sleep zero nanoseconds
		 *
		 */
		ATF_REQUIRE_ERRNO(EAGAIN, s < 0);
	}
	{
		int s = __thrsigdivert(mask, NULL,
		    &(struct timespec) { 0, -1 });
		/*
		 * This should probably be EINVAL, see sys/kern/kern_sig.c:
		 *
		 *      if (timeinvalid)
		 *      	error = EINVAL;
		 *
		 *      if (SCARG(uap, timeout) != NULL && nsecs == INFSLP)
		 *      	error = EAGAIN;
		 *
		 * ...should be:
		 *
		 *      if (timeinvalid)
		 *      	error = EINVAL;
		 *      else if (SCARG(uap, timeout) != NULL && nsecs == 0)
		 *      	error = EAGAIN;
		 *
		 */
#if OpenBSD >= 202204
		/* OpenBSD 7.1 fixed this bug. */
		ATF_REQUIRE_ERRNO(EINVAL, s < 0);
#else
		ATF_REQUIRE_ERRNO(EAGAIN, s < 0);
#endif
	}
#endif
}

ATF_TC_WITHOUT_HEAD(signalfd__sigchld);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__sigchld, tcptr)
{
	sigset_t mask;

	ATF_REQUIRE(sigemptyset(&mask) == 0);
	ATF_REQUIRE(sigaddset(&mask, SIGCHLD) == 0);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	int sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	pid_t pid = fork();
	ATF_REQUIRE(pid >= 0);
	if (pid == 0) {
		_exit(10);
	}

	struct signalfd_siginfo fdsi;
	ssize_t s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	ATF_REQUIRE(s == (ssize_t)sizeof(struct signalfd_siginfo));

	int status;
	ATF_REQUIRE(waitpid(pid, &status, 0) == pid);
	ATF_REQUIRE(WIFEXITED(status));
	ATF_REQUIRE(WEXITSTATUS(status) == 10);

	ATF_REQUIRE(close(sfd) == 0);

	ATF_REQUIRE(fdsi.ssi_signo == SIGCHLD);
#if defined(__OpenBSD__) || defined(__DragonFly__) || defined(__APPLE__)
	ATF_REQUIRE(fdsi.ssi_pid == 0);
	ATF_REQUIRE(fdsi.ssi_code == 0);
	atf_tc_skip("OpenBSD/DragonFlyBSD/macOS do not fill si_pid on SIGCHLD");
#endif
	ATF_REQUIRE_MSG(fdsi.ssi_pid == (uint32_t)pid, "%d %d",
	    (int)fdsi.ssi_pid, (int)pid);
	ATF_REQUIRE(fdsi.ssi_code == CLD_EXITED);
	ATF_REQUIRE(fdsi.ssi_status == 10);
}

ATF_TC_WITHOUT_HEAD(signalfd__sigwinch);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__sigwinch, tcptr)
{
	sigset_t mask;

	ATF_REQUIRE(sigemptyset(&mask) == 0);
	ATF_REQUIRE(sigaddset(&mask, SIGWINCH) == 0);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	int sfd = signalfd(-1, &mask, 0);
	ATF_REQUIRE(sfd >= 0);

	kill(getpid(), SIGWINCH);

	struct signalfd_siginfo fdsi;
	ssize_t s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
	ATF_REQUIRE(s == (ssize_t)sizeof(struct signalfd_siginfo));

	ATF_REQUIRE(fdsi.ssi_signo == SIGWINCH);

	ATF_REQUIRE(close(sfd) == 0);
}

#if !defined(__APPLE__)
static atomic_int signalfd__multiple_readers_count;
static void *
signalfd__multiple_readers_thread(void *arg)
{
	pthread_barrier_t *barrier = arg;

	sigset_t mask;
	ATF_REQUIRE(sigemptyset(&mask) == 0);
	ATF_REQUIRE(sigaddset(&mask, SIGUSR1) == 0);
	int sfd = signalfd(-1, &mask, SFD_NONBLOCK);
	ATF_REQUIRE_MSG(sfd >= 0, "errno: %d", errno);

	{
		int ec = pthread_barrier_wait(barrier);
		ATF_REQUIRE(ec == 0 || ec == PTHREAD_BARRIER_SERIAL_THREAD);
	}

	while (signalfd__multiple_readers_count == 0) {
		struct signalfd_siginfo fdsi;
		ssize_t s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
		ATF_REQUIRE(s == (ssize_t)sizeof(struct signalfd_siginfo) ||
		    (s < 0 && errno == EAGAIN));
		if (s == (ssize_t)sizeof(struct signalfd_siginfo)) {
			++signalfd__multiple_readers_count;
		}
	}

	ATF_REQUIRE(close(sfd) == 0);

	return NULL;
}
#endif
ATF_TC_WITHOUT_HEAD(signalfd__multiple_readers);
ATF_TC_BODY_FD_LEAKCHECK(signalfd__multiple_readers, tcptr)
{
#if defined(__APPLE__)
	atf_tc_skip("Apple does not have pthread_barrier_t");
#else
	sigset_t mask;

	struct rlimit lim = { 512, 512 };
	ATF_REQUIRE(setrlimit(RLIMIT_NOFILE, &lim) == 0);

	ATF_REQUIRE(sigemptyset(&mask) == 0);
	ATF_REQUIRE(sigaddset(&mask, SIGUSR1) == 0);

	ATF_REQUIRE(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);

	pthread_t threads[64];
	size_t nr_threads = sizeof(threads) / sizeof(threads[0]);

	pthread_barrier_t barrier;
	ATF_REQUIRE(pthread_barrier_init(&barrier, NULL,
			(unsigned int)(nr_threads + 1)) == 0);

	for (size_t i = 0; i < nr_threads; ++i) {
		ATF_REQUIRE(
		    pthread_create(&threads[i], NULL,
			signalfd__multiple_readers_thread, &barrier) == 0);
	}

	{
		int ec = pthread_barrier_wait(&barrier);
		ATF_REQUIRE(ec == 0 || ec == PTHREAD_BARRIER_SERIAL_THREAD);
	}

	kill(getpid(), SIGUSR1);

	alarm(30);

	for (size_t i = 0; i < nr_threads; ++i) {
		ATF_REQUIRE(pthread_join(threads[i], NULL) == 0);
	}

	ATF_REQUIRE(signalfd__multiple_readers_count == 1);
#endif
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, signalfd__simple_signalfd);
	ATF_TP_ADD_TC(tp, signalfd__blocking_read);
	ATF_TP_ADD_TC(tp, signalfd__nonblocking_read);
	ATF_TP_ADD_TC(tp, signalfd__multiple_signals);
	ATF_TP_ADD_TC(tp, signalfd__modify_signalmask);
	ATF_TP_ADD_TC(tp, signalfd__argument_checks);
	ATF_TP_ADD_TC(tp, signalfd__signal_disposition);
	ATF_TP_ADD_TC(tp, signalfd__sigwaitinfo);
	ATF_TP_ADD_TC(tp, signalfd__sigwait_openbsd);
	ATF_TP_ADD_TC(tp, signalfd__sigchld);
	ATF_TP_ADD_TC(tp, signalfd__sigwinch);
	ATF_TP_ADD_TC(tp, signalfd__multiple_readers);

	return atf_no_error();
}
