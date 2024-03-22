/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_FENCE_H
#define D3D12_FENCE_H

#include "util/u_inlines.h"

#include "d3d12_common.h"


constexpr uint64_t NsPerMs = 1000000;
constexpr uint64_t MaxTimeoutInNs = (uint64_t)UINT_MAX * NsPerMs;

#ifdef _WIN32
inline void
d3d12_fence_close_event(HANDLE event, int fd)
{
   if (event)
      CloseHandle(event);
}

inline HANDLE
d3d12_fence_create_event(int *fd)
{
   *fd = -1;
   return CreateEvent(NULL, false, false, NULL);
}

inline bool
d3d12_fence_wait_event(HANDLE event, int event_fd, uint64_t timeout_ns)
{
   DWORD timeout_ms = (timeout_ns == OS_TIMEOUT_INFINITE || timeout_ns > MaxTimeoutInNs) ? INFINITE : timeout_ns / NsPerMs;
   return WaitForSingleObject(event, timeout_ms) == WAIT_OBJECT_0;
}
#else
#include <sys/eventfd.h>
#include <poll.h>
#include <util/libsync.h>

inline void
d3d12_fence_close_event(HANDLE event, int fd)
{
   if (fd != -1)
      close(fd);
}

inline HANDLE
d3d12_fence_create_event(int *fd)
{
   *fd = eventfd(0, 0);
   return (HANDLE)(size_t)*fd;
}

inline bool
d3d12_fence_wait_event(HANDLE event, int event_fd, uint64_t timeout_ns)
{
   int timeout_ms = (timeout_ns == OS_TIMEOUT_INFINITE || timeout_ns > MaxTimeoutInNs) ? -1 : timeout_ns / NsPerMs;
   return sync_wait(event_fd, timeout_ms) == 0;
}
#endif

struct pipe_screen;
struct d3d12_screen;

struct d3d12_fence {
   struct pipe_reference reference;
   ID3D12Fence *cmdqueue_fence;
   HANDLE event;
   int event_fd;
   uint64_t value;
   bool signaled;
};

static inline struct d3d12_fence *
d3d12_fence(struct pipe_fence_handle *pfence)
{
   return (struct d3d12_fence *)pfence;
}

struct d3d12_fence *
d3d12_create_fence(struct d3d12_screen *screen);

struct d3d12_fence *
d3d12_open_fence(struct d3d12_screen *screen, HANDLE handle, const void *name);

void
d3d12_fence_reference(struct d3d12_fence **ptr, struct d3d12_fence *fence);

bool
d3d12_fence_finish(struct d3d12_fence *fence, uint64_t timeout_ns);

void
d3d12_screen_fence_init(struct pipe_screen *pscreen);

#endif
