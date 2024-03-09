#if defined( __BORLANDC__ )

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#if defined( __FLAT__ )
#define _NEAR
#define _PASCAL
#define _SS
#else
#define _NEAR near
#define _PASCAL pascal
#define _SS _ss
#endif

// The text formatter from Borland's RTL.
extern "C" int _PASCAL _NEAR __vprinter( size_t _PASCAL _NEAR (_NEAR *)(const void *, size_t, void *),
                                         void *, const char *, void _SS * );

// In 16-bit mode, __vprinter is a near function, so it must be invoked
// from the same code segment in which it is located. The same goes for
// the callback argument.

#if !defined( __FLAT__ )
#pragma codeseg _TEXT "CODE" // The RTL code segment.
#endif

struct TStrBuf
{
    char *cur;
    char *end;
};

static size_t _PASCAL _NEAR strnputn(const void *s, size_t n, void *outP)
{
    TStrBuf &buf = *(TStrBuf *) outP;
    size_t bufLen = buf.end - buf.cur;
    if (bufLen > 0)
    {
        size_t copyLen = min(n, bufLen - 1);
        memcpy(buf.cur, s, copyLen);
        buf.cur[copyLen] = '\0';
        buf.cur += copyLen;
    }
    return n;
}

int vsnprintf( char _FAR *buffer, size_t size, const char _FAR *fmt,
               void _FAR *arglist )
{
    if (size > 0)
        *buffer = '\0';
    TStrBuf out = {buffer, buffer + size};
    return __vprinter(strnputn, &out, fmt, (void _SS *) arglist);
}

int snprintf(char _FAR *buffer, size_t size, const char _FAR *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    int ret = vsnprintf(buffer, size, fmt, ap);
    va_end(ap);
    return ret;
}

#endif // __BORLANDC__
