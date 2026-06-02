#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <mach/kern_return.h>
#include <mach-o/loader.h>
#include <uuid/uuid.h>
#include <pthread/pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

void start(void)
{
}

/* FIXME: remove once we have CommonCrypto working */
struct ccdigest_info;

void cc_clear(size_t len, void* dst)
{
    if ( dst == NULL )
        return;
    volatile unsigned char* p = (volatile unsigned char*)dst;
    while ( len-- )
        *p++ = 0;
}

void ccdigest_init(const struct ccdigest_info* di, void* ctx)
{
    (void)di;
    (void)ctx;
}

void ccdigest_update(const struct ccdigest_info* di, void* ctx, size_t len, const void* data)
{
    (void)di;
    (void)ctx;
    (void)len;
    (void)data;
}

const struct ccdigest_info* ccsha1_di(void)
{
    return NULL;
}

const struct ccdigest_info* ccsha256_di(void)
{
    return NULL;
}

const struct ccdigest_info* ccsha384_di(void)
{
    return NULL;
}

