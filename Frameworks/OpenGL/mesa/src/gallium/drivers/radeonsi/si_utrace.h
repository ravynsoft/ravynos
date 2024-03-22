/*
 * Copyright 2023 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SI_UTRACE_H
#define SI_UTRACE_H

#include <stdint.h>
#include "si_pipe.h"

void si_utrace_init(struct si_context *sctx);
void si_utrace_fini(struct si_context *sctx);

void si_utrace_flush(struct si_context *sctx, uint64_t submission_id);

#endif
