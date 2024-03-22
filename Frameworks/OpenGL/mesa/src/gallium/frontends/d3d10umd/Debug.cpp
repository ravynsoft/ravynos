#include "Debug.h"

#include <stdarg.h>
#include <stdio.h>


#ifdef DEBUG

unsigned st_debug = 0;

static const
struct debug_named_value st_debug_flags[] = {
   {"oldtexops", ST_DEBUG_OLD_TEX_OPS, "oldtexops"},
   {"tgsi", ST_DEBUG_TGSI, "tgsi"},
   DEBUG_NAMED_VALUE_END
};
void
st_debug_parse(void)
{
   st_debug = debug_get_flags_option("ST_DEBUG", st_debug_flags, st_debug);
}

#endif


void
DebugPrintf(const char *format, ...)
{
    char buf[4096];

    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof buf, format, ap);
    va_end(ap);

    OutputDebugStringA(buf);
}


/**
 * Produce a human readable message from HRESULT.
 *
 * @sa http://msdn.microsoft.com/en-us/library/ms679351(VS.85).aspx
 */
void
CheckHResult(HRESULT hr, const char *function, unsigned line)
{
   if (FAILED(hr)) {
      LPSTR lpMessageBuffer = NULL;

      FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,
                     hr,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPSTR)&lpMessageBuffer,
                     0,
                     NULL);

      DebugPrintf("%s: %u: 0x%08lX: %s", function, line, hr, lpMessageBuffer);

      LocalFree(lpMessageBuffer);
   }
}


void
AssertFail(const char *expr,
           const char *file,
           unsigned line,
           const char *function)
{
   DebugPrintf("%s:%u:%s: Assertion `%s' failed.\n", file, line, function, expr);
#if defined(__GNUC__)
   __asm("int3");
#elif defined(_MSC_VER)
   __debugbreak();
#else
   DebugBreak();
#endif
}
