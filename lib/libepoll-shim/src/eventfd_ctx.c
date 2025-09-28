#include "eventfd_ctx.h"

#include <sys/types.h>

#include <sys/event.h>
#include <sys/param.h>

#include <assert.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

_Static_assert(sizeof(unsigned int) < sizeof(uint64_t), "");

errno_t
eventfd_ctx_init(EventFDCtx *eventfd, int kq, unsigned int counter, int flags)
{
	errno_t ec;

	assert((flags & ~(EVENTFD_CTX_FLAG_SEMAPHORE)) == 0);

	*eventfd = (EventFDCtx) {
		.flags_ = flags,
		.counter_ = counter,
	};

	struct kevent kevs[2];
	int kevs_length = 0;

	if ((ec = kqueue_event_init(&eventfd->kqueue_event_, /**/
		 kevs, &kevs_length, counter > 0)) != 0) {
		goto out2;
	}

	if (kevent(kq, kevs, kevs_length, NULL, 0, NULL) < 0) {
		ec = errno;
		goto out;
	}

	return 0;

out:
	(void)kqueue_event_terminate(&eventfd->kqueue_event_);
out2:
	return ec;
}

errno_t
eventfd_ctx_terminate(EventFDCtx *eventfd)
{
	errno_t ec = 0;
	errno_t ec_local;

	ec_local = kqueue_event_terminate(&eventfd->kqueue_event_);
	ec = ec != 0 ? ec : ec_local;

	return (ec);
}

errno_t
eventfd_ctx_write(EventFDCtx *eventfd, int kq, uint64_t value)
{
	errno_t ec;

	if (value == UINT64_MAX) {
		return EINVAL;
	}

	uint_least64_t current_value = eventfd->counter_;

	uint_least64_t new_value;
	if (__builtin_add_overflow(current_value, value, &new_value) ||
	    new_value > UINT64_MAX - 1) {
		return EAGAIN;
	}

	eventfd->counter_ = new_value;

	ec = kqueue_event_trigger(&eventfd->kqueue_event_, kq);
	if (ec != 0) {
		return ec;
	}

	return 0;
}

errno_t
eventfd_ctx_read(EventFDCtx *eventfd, int kq, uint64_t *value)
{
	uint_least64_t current_value;

	current_value = eventfd->counter_;
	if (current_value == 0) {
		return EAGAIN;
	}

	uint_least64_t new_value =			     /**/
	    (eventfd->flags_ & EVENTFD_CTX_FLAG_SEMAPHORE) ? /**/
	    current_value - 1 :
	    0;

	if (new_value == 0 &&
	    kqueue_event_is_triggered(&eventfd->kqueue_event_)) {
		kqueue_event_clear(&eventfd->kqueue_event_, kq);
	}

	eventfd->counter_ = new_value;

	*value =					     /**/
	    (eventfd->flags_ & EVENTFD_CTX_FLAG_SEMAPHORE) ? /**/
	    1 :
	    current_value;
	return 0;
}
