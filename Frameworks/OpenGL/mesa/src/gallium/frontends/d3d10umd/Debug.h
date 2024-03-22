#pragma once

#include "DriverIncludes.h"
#include "util/u_debug.h"


#ifdef __cplusplus
extern "C" {
#endif


#define ST_DEBUG_OLD_TEX_OPS   (1 <<  0)
#define ST_DEBUG_TGSI          (1 <<  1)


#ifdef DEBUG
extern unsigned st_debug;
#else
#define st_debug 0
#endif


#ifdef DEBUG
void st_debug_parse(void);
#else
#define st_debug_parse() ((void)0)
#endif


void
DebugPrintf(const char *format, ...);


void
CheckHResult(HRESULT hr, const char *function, unsigned line);


#define CHECK_NTSTATUS(status) \
   CheckNTStatus(status, __func__, __LINE__)


#define CHECK_HRESULT(hr) \
   CheckHResult(hr, __func__, __LINE__)


void
AssertFail(const char *expr, const char *file, unsigned line, const char *function);


#ifndef NDEBUG
#define ASSERT(expr) ((expr) ? (void)0 : AssertFail(#expr, __FILE__, __LINE__, __func__))
#else
#define ASSERT(expr) do { } while (0 && (expr))
#endif


#if 0 && !defined(NDEBUG)
#define LOG_ENTRYPOINT() DebugPrintf("%s\n", __func__)
#else
#define LOG_ENTRYPOINT() (void)0
#endif

#define LOG_UNSUPPORTED_ENTRYPOINT() DebugPrintf("%s XXX\n", __func__)

#define LOG_UNSUPPORTED(expr) \
   do { if (expr) DebugPrintf("%s:%d XXX %s\n", __func__, __LINE__, #expr); } while(0)


#ifdef __cplusplus
}
#endif

