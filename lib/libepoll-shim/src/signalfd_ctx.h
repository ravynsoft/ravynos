#ifndef SIGNALFD_CTX_H_
#define SIGNALFD_CTX_H_

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>

#include "kqueue_event.h"

typedef struct {
	sigset_t sigs;
	KQueueEvent kqueue_event;
} SignalFDCtx;

typedef struct {
	uint32_t ssi_signo;
	int32_t ssi_errno;
	int32_t ssi_code;
	uint32_t ssi_pid;
	uint32_t ssi_uid;
	int32_t ssi_fd;
	uint32_t ssi_tid;
	uint32_t ssi_band;
	uint32_t ssi_overrun;
	uint32_t ssi_trapno;
	int32_t ssi_status;
	int32_t ssi_int;
	uint64_t ssi_ptr;
	uint64_t ssi_utime;
	uint64_t ssi_stime;
	uint64_t ssi_addr;
	uint16_t ssi_addr_lsb;
	uint8_t pad[46];
} SignalFDCtxSiginfo;

errno_t signalfd_ctx_init(SignalFDCtx *signalfd, int kq, sigset_t const *sigs);
errno_t signalfd_ctx_terminate(SignalFDCtx *signalfd);

errno_t signalfd_ctx_read(SignalFDCtx *signalfd, int kq,
    SignalFDCtxSiginfo *siginfo);
void signalfd_ctx_poll(SignalFDCtx *signalfd, int kq, uint32_t *revents);

#endif
