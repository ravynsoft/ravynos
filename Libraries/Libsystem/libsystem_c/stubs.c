#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef __UINT64_TYPE__ stub_uint64_t;

/* from corecrypto.kext pkrng - placeholder */
#define CCRNG_STATE_COMMON \
    int (*generate)(struct ccrng_state *rng, size_t outlen, void *out);

struct ccrng_state {
    CCRNG_STATE_COMMON
};

int ccrng_uniform(struct ccrng_state *rng, uint64_t bound, uint64_t *rand)
{
        (void)rng;
        (void)bound;
        if ( rand != NULL )
                *rand = 0;
        return 0;
}

struct ccrng_state *ccrng(int *error)
{
        (void)error;
        return NULL;
}

struct section64 *getsectiondata(const struct mach_header_64 *mhp, const char *segname, const char *sectname, uint64_t *size)
{
    (void)mhp;
    (void)segname;
    (void)sectname;
    if ( size != NULL )
        *size = 0;
    return NULL;
}
