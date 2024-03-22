/*
 * Copyright 2023 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 */

#include "u_gpuvis.h"

#include <threads.h>

#define GPUVIS_TRACE_IMPLEMENTATION
#include "gpuvis_trace_utils.h"

/* Random base value to prevent collisions. As contexts are considered thread
 * global by gpuvis, collisions are quite likely if we start at 0 and there
 * are independent libraries tacing
 */
static unsigned int gpuvis_base_ctx;

static _Thread_local unsigned int gpuvis_current_ctx;

static once_flag gpuvis_once_flag = ONCE_FLAG_INIT;

static void
util_gpuvis_init_once()
{
   gpuvis_trace_init();

   /* Initialize it by address to avoid collisions between libraries using
    * this code (e.g. GL & vulkan) */
   gpuvis_base_ctx = (uintptr_t) util_gpuvis_init_once >> 12;
}

void
util_gpuvis_init(void)
{
   call_once(&gpuvis_once_flag, util_gpuvis_init_once);
}

void
util_gpuvis_begin(const char *name)
{
   unsigned int ctx = gpuvis_base_ctx + ++gpuvis_current_ctx;
   gpuvis_trace_begin_ctx_printf(ctx, "mesa:%s", name);
}

void
util_gpuvis_end(void)
{
   unsigned int ctx = gpuvis_base_ctx + gpuvis_current_ctx--;

   /* Use an empty string to avoid warnings about an empty format string. */
   gpuvis_trace_end_ctx_printf(ctx, "%s", "");
}