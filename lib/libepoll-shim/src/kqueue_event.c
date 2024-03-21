#include "kqueue_event.h"

#include <sys/param.h>

#include <assert.h>

#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "wrap.h"

errno_t
kqueue_event_init(KQueueEvent *kqueue_event, struct kevent *kevs,
    int *kevs_length, bool should_trigger)
{
	*kqueue_event = (KQueueEvent) { .self_pipe_ = { -1, -1 } };

#ifdef EVFILT_USER
	EV_SET(&kevs[(*kevs_length)++], 0, /**/
	    EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, 0);

	if (should_trigger) {
		EV_SET(&kevs[(*kevs_length)++], 0, /**/
		    EVFILT_USER, 0, NOTE_TRIGGER, 0, 0);
		kqueue_event->is_triggered_ = true;
	}

	return 0;
#else
	errno_t ec;

	if (pipe2(kqueue_event->self_pipe_, O_NONBLOCK | O_CLOEXEC) < 0) {
		return errno;
	}

	EV_SET(&kevs[(*kevs_length)++], kqueue_event->self_pipe_[0], /**/
	    EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
	if (should_trigger) {
		if ((ec = kqueue_event_trigger(kqueue_event, -1)) != 0) {
			goto out;
		}
		assert(kqueue_event->is_triggered_);
	}

	return 0;

out:
	(void)kqueue_event_terminate(kqueue_event);
	return ec;
#endif
}

errno_t
kqueue_event_terminate(KQueueEvent *kqueue_event)
{
#ifdef EVFILT_USER
	(void)kqueue_event;
	return 0;
#else
	errno_t ec = 0;
	if (real_close(kqueue_event->self_pipe_[0]) < 0) {
		ec = ec != 0 ? ec : errno;
	}
	if (real_close(kqueue_event->self_pipe_[1]) < 0) {
		ec = ec != 0 ? ec : errno;
	}
	return ec;
#endif
}

bool
kqueue_event_is_triggered(KQueueEvent *kqueue_event)
{
	return kqueue_event->is_triggered_;
}

errno_t
kqueue_event_trigger(KQueueEvent *kqueue_event, int kq)
{
	if (kqueue_event->is_triggered_) {
		return 0;
	}

#ifdef EVFILT_USER
	struct kevent kevs[1];
	EV_SET(&kevs[0], 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, 0);

	if (kevent(kq, kevs, 1, NULL, 0, NULL) < 0) {
		return errno;
	}
#else
	char c = 0;
	if (real_write(kqueue_event->self_pipe_[1], &c, 1) < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			return errno;
		}
	}
#endif

	kqueue_event->is_triggered_ = true;
	return 0;
}

void
kqueue_event_clear(KQueueEvent *kqueue_event, int kq)
{
#ifndef EVFILT_USER
	char c[32];
	while (real_read(kqueue_event->self_pipe_[0], c, sizeof(c)) >= 0) {
	}
#endif

	struct kevent kevs[32];
	int n;

	while ((n = kevent(kq, NULL, 0, kevs, 32,
		    &(struct timespec) { 0, 0 })) > 0) {
	}

	kqueue_event->is_triggered_ = false;
}
