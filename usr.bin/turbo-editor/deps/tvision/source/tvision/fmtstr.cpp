/*------------------------------------------------------------*/
/* filename -       fmtstr.cpp                                */
/*                                                            */
/* function(s)                                                */
/*            fmtStr                                          */
/*            vfmtStr                                         */
/*------------------------------------------------------------*/

#include <tvision/tv.h>

#include <stdio.h>

#if defined( __BORLANDC__ )
#define va_copy(dst, src) ((void) ((dst) = (src)))
#endif // __BORLANDC__

char *fmtStr(const char _FAR *fmt, ...) noexcept
{
    va_list ap;

    va_start(ap, fmt);
    char *ret = vfmtStr(fmt, ap);
    va_end(ap);

    return ret;
}

char *vfmtStr(const char _FAR *fmt, va_list ap) noexcept
{
    va_list ap2;

    va_copy(ap2, ap);
    int n = vsnprintf(0, 0, fmt, ap2);
    va_end(ap2);

    if (n < 0)
        return 0;

    char *buf = new char[n + 1];
    if (buf)
        vsprintf(buf, fmt, ap);

    return buf;
}
