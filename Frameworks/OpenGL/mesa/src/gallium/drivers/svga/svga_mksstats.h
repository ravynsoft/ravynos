/**********************************************************
 * Copyright 2016 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef _SVGA_MKSSTATS_H
#define _SVGA_MKSSTATS_H

#include "svga_winsys.h"

#ifdef VMX86_STATS
#define SVGA_STATS_COUNT_INC(_sws, _stat)                    \
   _sws->stats_inc(_sws, _stat);

#define SVGA_STATS_TIME_PUSH(_sws, _stat)                    \
   struct svga_winsys_stats_timeframe timeFrame;             \
   _sws->stats_time_push(_sws, _stat, &timeFrame);

#define SVGA_STATS_TIME_POP(_sws)                            \
   _sws->stats_time_pop(_sws);

#else

#define SVGA_STATS_COUNT_INC(_sws, _stat)
#define SVGA_STATS_TIME_PUSH(_sws, _stat)
#define SVGA_STATS_TIME_POP(_sws)

#endif
#endif /* _SVGA_MKSSTATS_H */
