/*
 * DoNothing.kext
 * A bare do nothing kext.  It should have zero undefined symbols.
 */

#include <mach/mach_types.h>

#define UNUSED __attribute__((unused))

kern_return_t DoNothing_start(kmod_info_t * ki, void *d);
kern_return_t DoNothing_stop(kmod_info_t *ki, void *d);

kern_return_t DoNothing_start(UNUSED kmod_info_t * ki, UNUSED void *d)
{
    return KERN_SUCCESS;
}

kern_return_t DoNothing_stop(UNUSED kmod_info_t *ki, UNUSED void *d)
{
    return KERN_SUCCESS;
}
