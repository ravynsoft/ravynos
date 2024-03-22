/*
 * Copyright 2023 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 */

#ifndef U_GPUVIS_H
#define U_GPUVIS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_GPUVIS

void util_gpuvis_init(void);

void util_gpuvis_begin(const char *name);

/* ctx needs to be the return value from begin*/
void util_gpuvis_end(void);

#else

static inline void
util_gpuvis_init(void)
{
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* U_GPUVIS_H */