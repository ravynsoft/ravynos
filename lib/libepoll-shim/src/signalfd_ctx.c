#include "signalfd_ctx.h"

#include <sys/types.h>

#include <sys/event.h>

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

static errno_t
signalfd_has_pending(SignalFDCtx const *signalfd, bool *has_pending,
    sigset_t *pending)
{
	sigset_t pending_sigs;

	if (sigpending(&pending_sigs) < 0 ||
	    sigandset(&pending_sigs, &pending_sigs, &signalfd->sigs) < 0) {
		return errno;
	}

	*has_pending = !sigisemptyset(&pending_sigs);
	if (pending) {
		*pending = pending_sigs;
	}
	return 0;
}

static errno_t
signalfd_ctx_trigger_manually(SignalFDCtx *signalfd, int kq)
{
	return kqueue_event_trigger(&signalfd->kqueue_event, kq);
}

static void
signalfd_signal_handler(int signo)
{
	(void)signo;
}

errno_t
signalfd_ctx_init(SignalFDCtx *signalfd, int kq, sigset_t const *sigs)
{
	errno_t ec;

	assert(sigs != NULL);

	*signalfd = (SignalFDCtx) { .sigs = *sigs };

#ifndef _SIG_MAXSIG
#define _SIG_MAXSIG (8 * sizeof(sigset_t))
#endif

	struct kevent kevs[_SIG_MAXSIG + 2];
	int n = 0;

	if ((ec = kqueue_event_init(&signalfd->kqueue_event, kevs, &n,
		 false)) != 0) {
		goto out2;
	}

	for (int i = 1; i <= _SIG_MAXSIG; ++i) {
		if (sigismember(&signalfd->sigs, i)) {
			EV_SET(&kevs[n++], (unsigned int)i, EVFILT_SIGNAL,
			    EV_ADD, 0, 0, 0);

			/*
			 * On Linux, signals with disposition SIG_DFL and a
			 * default action of "ignored" are returned from
			 * sigwait.
			 * We can emulate this by registering an empty signal
			 * handler.
			 */
			if (i == SIGCHLD || /**/
			    i == SIGURG ||  /**/
			    i == SIGCONT || /**/
#ifdef SIGIO
			    i == SIGIO || /**/
#endif
#ifdef SIGWINCH
			    i == SIGWINCH || /**/
#endif
#ifdef SIGINFO
			    i == SIGINFO || /**/
#endif
#ifdef SIGPWR
			    i == SIGPWR || /**/
#endif
#ifdef SIGTHR
			    i == SIGTHR || /**/
#endif
			    false) {
				struct sigaction sa;

				if (sigaction(i, NULL, &sa) == 0 &&
				    !(sa.sa_flags & SA_SIGINFO) &&
				    sa.sa_handler == SIG_DFL) {
					sa.sa_flags |= SA_RESTART;
					sa.sa_handler = signalfd_signal_handler;
					(void)sigaction(i, &sa, NULL);
				}
			}
		}
	}

	n = kevent(kq, kevs, n, NULL, 0, NULL);
	if (n < 0) {
		ec = errno;
		goto out;
	}

	bool has_pending;
	if ((ec = signalfd_has_pending(signalfd, &has_pending, NULL)) != 0) {
		goto out;
	}
	if (has_pending) {
		if ((ec = signalfd_ctx_trigger_manually(signalfd, kq)) != 0) {
			goto out;
		}
	}

	return 0;

out:
	(void)kqueue_event_terminate(&signalfd->kqueue_event);
out2:
	return ec;
}

errno_t
signalfd_ctx_terminate(SignalFDCtx *signalfd)
{
	errno_t ec = 0;
	errno_t ec_local;

	ec_local = kqueue_event_terminate(&signalfd->kqueue_event);
	ec = ec != 0 ? ec : ec_local;

	return ec;
}

static errno_t
signalfd_ctx_read_impl(SignalFDCtx *signalfd,
    SignalFDCtxSiginfo *signalfd_siginfo)
{
	errno_t ec;

	_Static_assert(sizeof(*signalfd_siginfo) == 128, "");

	/*
	 * EVFILT_SIGNAL is an "observer". It does not hook into the
	 * signal disposition mechanism. On the other hand, `signalfd` does.
	 * Therefore, to properly emulate `signalfd`, `sigtimedwait` must be
	 * called.
	 */

	siginfo_t siginfo;
	memset(&siginfo, 0, sizeof(siginfo));

#if defined(__OpenBSD__) || defined(__APPLE__)
	for (;;) {
		bool has_pending;
		sigset_t pending_sigs;
		if ((ec = signalfd_has_pending(signalfd, &has_pending,
			 &pending_sigs)) != 0) {
			return ec;
		}
		if (!has_pending) {
			return EAGAIN;
		}

#if defined(__OpenBSD__)
		/*
		 * sigwait does not behave nicely when multiple signals
		 * are pending (as of OpenBSD 6.8). So, only try to
		 * grab one.
		 */
		int signum = __builtin_ffsll((long long)pending_sigs);
		sigset_t mask = sigmask(signum);

		extern int __thrsigdivert(sigset_t set, siginfo_t * info,
		    struct timespec const *timeout);

		/* `&(struct timespec){0, 0}` returns EAGAIN but spams
		 * the dmesg log. Let's do it with an invalid timespec
		 * and EINVAL. */
		int s = __thrsigdivert(mask, NULL,
		    &(struct timespec) { 0, -1 });
		ec = s < 0 ? errno : 0;
		if (ec == EINVAL || ec == EAGAIN) {
			/* We must retry because we only checked for
			 * one signal. There may be others pending. */
			continue;
		}
#elif defined(__APPLE__)
		int s;
		ec = sigwait(&signalfd->sigs, &s);
#else
#error ""
#endif
		if (ec != 0) {
			break;
		}

		siginfo.si_signo = s;

		break;
	}
#else
	{
		int s = sigtimedwait(&signalfd->sigs, &siginfo,
		    &(struct timespec) { 0, 0 });
		ec = s < 0 ? errno : 0;
		if (ec == 0) {
			assert(siginfo.si_signo == s);
		}
	}
#endif
	if (ec != 0) {
		return ec;
	}

	/*
	 * First, fill the POSIX compatible fields, then anything else OS
	 * specific we have.
	 */

	signalfd_siginfo->ssi_signo = (uint32_t)siginfo.si_signo;
	signalfd_siginfo->ssi_code = siginfo.si_code;
	signalfd_siginfo->ssi_errno = siginfo.si_errno;

	signalfd_siginfo->ssi_pid = (uint32_t)siginfo.si_pid;
	signalfd_siginfo->ssi_uid = siginfo.si_uid;

	signalfd_siginfo->ssi_addr = (uint64_t)(uintptr_t)siginfo.si_addr;

	signalfd_siginfo->ssi_status = siginfo.si_status;

#ifndef __OpenBSD__
	signalfd_siginfo->ssi_band = (uint32_t)siginfo.si_band;
#endif

	signalfd_siginfo->ssi_int = siginfo.si_value.sival_int;
	signalfd_siginfo->ssi_ptr = (uint64_t)(uintptr_t)
					siginfo.si_value.sival_ptr;

	/*
	 * Sane defaults for Linux specific fields, may be amended in the
	 * future.
	 */
	signalfd_siginfo->ssi_fd = -1;
	signalfd_siginfo->ssi_tid = (uint32_t)-1;

#ifdef __FreeBSD__
	signalfd_siginfo->ssi_trapno = (uint32_t)siginfo.si_trapno;

	signalfd_siginfo->ssi_tid = (uint32_t)siginfo.si_timerid;
	signalfd_siginfo->ssi_overrun = (uint32_t)siginfo.si_overrun;

	if (siginfo.si_code == SI_MESGQ) {
		/* Re-use this field for si_mqd. */
		signalfd_siginfo->ssi_fd = siginfo.si_mqd;
	}
#elif __NetBSD__
	signalfd_siginfo->ssi_utime = siginfo.si_utime;
	signalfd_siginfo->ssi_stime = siginfo.si_stime;

	signalfd_siginfo->ssi_trapno = siginfo.si_trap;
	/* No space for trap2/trap3 */

	signalfd_siginfo->ssi_fd = siginfo.si_fd;

	/* No space for syscall/ptrace_state */
#endif
	return 0;
}

static bool
signalfd_ctx_clear_signal(SignalFDCtx *signalfd, int kq, bool was_triggered)
{
	if (was_triggered) {
		/*
		 * When there are other signals pending we can keep the kq
		 * readable and therefore don't need to clear it.
		 */
		bool has_pending;
		if (signalfd_has_pending(signalfd, &has_pending, NULL) != 0 ||
		    has_pending) {
			return true;
		}
	}

	/*
	 * Clear the kq. Signals can arrive here, leading to a race.
	 */

	kqueue_event_clear(&signalfd->kqueue_event, kq);

	/*
	 * Because of the race, we must recheck and manually trigger if
	 * necessary.
	 */
	bool has_pending;
	if (signalfd_has_pending(signalfd, &has_pending, NULL) != 0 ||
	    has_pending) {
		(void)signalfd_ctx_trigger_manually(signalfd, kq);
		return true;
	}
	return false;
}

errno_t
signalfd_ctx_read(SignalFDCtx *signalfd, int kq, SignalFDCtxSiginfo *siginfo)
{
	errno_t ec;

	ec = signalfd_ctx_read_impl(signalfd, siginfo);
	if (ec == 0 || ec == EAGAIN || ec == EWOULDBLOCK) {
		(void)signalfd_ctx_clear_signal(signalfd, kq, false);
	}

	return ec;
}

void
signalfd_ctx_poll(SignalFDCtx *signalfd, int kq, uint32_t *revents)
{
	bool pending = signalfd_ctx_clear_signal(signalfd, kq, revents != NULL);
	if (revents) {
		*revents = pending ? POLLIN : 0;
	}
}
