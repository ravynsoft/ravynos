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

void
__keymgr_initializer(void)
{
}

void
_libSC_info_fork_child(void)
{
}

void
_libSC_info_fork_parent(void)
{
}

void
_libSC_info_fork_prepare(void)
{
}

void
_libtrace_fork_child(void)
{
}

void
_libtrace_init(void)
{
}

void
_libxpc_initializer(void)
{
}

void
cc_atfork_child(void)
{
}

void
cc_atfork_parent(void)
{
}

void
cc_atfork_prepare(void)
{
}

void
xpc_atfork_child(void)
{
}

void
xpc_atfork_parent(void)
{
}

void
xpc_atfork_prepare(void)
{
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