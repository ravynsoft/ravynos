//
// Copyright (c) 2019-2019 Apple Inc. All rights reserved.
//
// lf_cs_logging.c - Implemenents routines for logging info, erros, warnings
//                   and debug for livefiles Apple_CoreStorage plugin.
//

#include <stdio.h>
#include <stdarg.h>

#include "lf_cs_logging.h"

#if !LF_CS_USE_OSLOG

#define VPRINTF(fmt, val)       vfprintf(stderr, fmt, val)

void
log_debug(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	VPRINTF(fmt, va);
	va_end(va);
}

void
log_info(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	VPRINTF(fmt, va);
	va_end(va);
}

void
log_warn(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	VPRINTF(fmt, va);
	va_end(va);
}

void
log_err(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	VPRINTF(fmt, va);
	va_end(va);
}

#endif /* !LF_CS_USE_OSLOG */
