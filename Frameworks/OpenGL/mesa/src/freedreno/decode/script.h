/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <stdint.h>

// XXX make script support optional
#define ENABLE_SCRIPTING 1

#ifdef ENABLE_SCRIPTING

/* called at start to load the script: */
int script_load(const char *file);

/* called at start of each cmdstream file: */
void script_start_cmdstream(const char *name);

/* called at each DRAW_INDX, calls script drawidx fxn to process
 * the current state
 */
__attribute__((weak))
void script_draw(const char *primtype, uint32_t nindx);

struct rnn;
struct rnndomain;
__attribute__((weak))
void script_packet(uint32_t *dwords, uint32_t sizedwords,
                   struct rnn *rnn,
                   struct rnndomain *dom);

/* maybe at some point it is interesting to add additional script
 * hooks for CP_EVENT_WRITE, etc?
 */

/* called at end of each cmdstream file: */
void script_end_cmdstream(void);

void script_start_submit(void);
void script_end_submit(void);

/* called after last cmdstream file: */
void script_finish(void);

#else
// TODO no-op stubs..
#endif

#endif /* SCRIPT_H_ */
