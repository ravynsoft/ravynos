/*
 * Copyright © 2023 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#ifndef __FREEDRENO_GPU_EVENT_H__
#define __FREEDRENO_GPU_EVENT_H__

#include "adreno_pm4.xml.h"

enum fd_gpu_event : uint32_t {
    FD_WRITE_PRIMITIVE_COUNTS = 0,
    FD_START_PRIMITIVE_CTRS,
    FD_STOP_PRIMITIVE_CTRS,
    FD_START_FRAGMENT_CTRS,
    FD_STOP_FRAGMENT_CTRS,
    FD_START_COMPUTE_CTRS,
    FD_STOP_COMPUTE_CTRS,
    FD_ZPASS_DONE,
    FD_RB_DONE,
    FD_FLUSH_SO_0,
    FD_FLUSH_SO_1,
    FD_FLUSH_SO_2,
    FD_FLUSH_SO_3,
    FD_CACHE_FLUSH,
    FD_CACHE_INVALIDATE,
    FD_CCU_INVALIDATE_DEPTH,
    FD_CCU_INVALIDATE_COLOR,
    FD_CCU_FLUSH_BLIT_CACHE,
    FD_CCU_FLUSH_DEPTH,
    FD_CCU_FLUSH_COLOR,
    FD_LRZ_CLEAR,
    FD_LRZ_FLUSH,
    FD_BLIT,
    FD_LABEL,

    FD_GPU_EVENT_MAX,
};

struct fd_gpu_event_info {
    enum vgt_event_type raw_event;
    bool needs_seqno;
};

template <chip CHIP>
constexpr struct fd_gpu_event_info fd_gpu_events[FD_GPU_EVENT_MAX] = {};

template <>
constexpr inline struct fd_gpu_event_info fd_gpu_events<A6XX>[FD_GPU_EVENT_MAX] = {
    {WRITE_PRIMITIVE_COUNTS, false},  /* FD_WRITE_PRIMITIVE_COUNTS */
    {START_PRIMITIVE_CTRS, false},    /* FD_START_PRIMITIVE_CTRS */
    {STOP_PRIMITIVE_CTRS, false},     /* FD_STOP_PRIMITIVE_CTRS */
    {START_FRAGMENT_CTRS, false},     /* FD_START_FRAGMENT_CTRS */
    {STOP_FRAGMENT_CTRS, false},      /* FD_STOP_FRAGMENT_CTRS */
    {START_COMPUTE_CTRS, false},      /* FD_START_COMPUTE_CTRS */
    {STOP_COMPUTE_CTRS, false},       /* FD_STOP_COMPUTE_CTRS */
    {ZPASS_DONE, false},              /* FD_ZPASS_DONE */
    {RB_DONE_TS, true},               /* FD_RB_DONE */
    {FLUSH_SO_0, false},              /* FD_FLUSH_SO_0 */
    {FLUSH_SO_1, false},              /* FD_FLUSH_SO_1 */
    {FLUSH_SO_2, false},              /* FD_FLUSH_SO_2 */
    {FLUSH_SO_3, false},              /* FD_FLUSH_SO_3 */
    {CACHE_FLUSH_TS, true},           /* FD_CACHE_FLUSH */
    {CACHE_INVALIDATE, false},        /* FD_CACHE_INVALIDATE */
    {PC_CCU_INVALIDATE_DEPTH, false}, /* FD_CCU_INVALIDATE_DEPTH */
    {PC_CCU_INVALIDATE_COLOR, false}, /* FD_CCU_INVALIDATE_COLOR */
    {PC_CCU_RESOLVE_TS, true},        /* FD_CCU_FLUSH_BLIT_CACHE */
    {PC_CCU_FLUSH_DEPTH_TS, true},    /* FD_CCU_FLUSH_DEPTH */
    {PC_CCU_FLUSH_COLOR_TS, true},    /* FD_CCU_FLUSH_COLOR */
    {LRZ_CLEAR, false},               /* FD_LRZ_CLEAR */
    {LRZ_FLUSH, false},               /* FD_LRZ_FLUSH */
    {BLIT, false},                    /* FD_BLIT */
    {LABEL, false},                   /* FD_LABEL */
};

template <>
constexpr inline struct fd_gpu_event_info fd_gpu_events<A7XX>[FD_GPU_EVENT_MAX] = {
    {WRITE_PRIMITIVE_COUNTS, false},  /* FD_WRITE_PRIMITIVE_COUNTS */
    {START_PRIMITIVE_CTRS, false},    /* FD_START_PRIMITIVE_CTRS */
    {STOP_PRIMITIVE_CTRS, false},     /* FD_STOP_PRIMITIVE_CTRS */
    {START_FRAGMENT_CTRS, false},     /* FD_START_FRAGMENT_CTRS */
    {STOP_FRAGMENT_CTRS, false},      /* FD_STOP_FRAGMENT_CTRS */
    {START_COMPUTE_CTRS, false},      /* FD_START_COMPUTE_CTRS */
    {STOP_COMPUTE_CTRS, false},       /* FD_STOP_COMPUTE_CTRS */
    {ZPASS_DONE, false},              /* FD_ZPASS_DONE */
    {RB_DONE_TS, true},               /* FD_RB_DONE */
    {FLUSH_SO_0, false},              /* FD_FLUSH_SO_0 */
    {FLUSH_SO_1, false},              /* FD_FLUSH_SO_1 */
    {FLUSH_SO_2, false},              /* FD_FLUSH_SO_2 */
    {FLUSH_SO_3, false},              /* FD_FLUSH_SO_3 */
    {CACHE_FLUSH7, false},            /* FD_CACHE_FLUSH */
    {CACHE_INVALIDATE7, false},       /* FD_CACHE_INVALIDATE */
    {CCU_INVALIDATE_DEPTH, false},    /* FD_CCU_INVALIDATE_DEPTH */
    {CCU_INVALIDATE_COLOR, false},    /* FD_CCU_INVALIDATE_COLOR */
    {CCU_RESOLVE_CLEAN, false},       /* FD_CCU_FLUSH_BLIT_CACHE */
    {CCU_FLUSH_DEPTH, false},         /* FD_CCU_FLUSH_DEPTH */
    {CCU_FLUSH_COLOR, false},         /* FD_CCU_FLUSH_COLOR */
    {LRZ_CLEAR, false},               /* FD_LRZ_CLEAR */
    {LRZ_FLUSH, false},               /* FD_LRZ_FLUSH */
    {BLIT, false},                    /* FD_BLIT */
    {LABEL, false},                   /* FD_LABEL */
};

#endif