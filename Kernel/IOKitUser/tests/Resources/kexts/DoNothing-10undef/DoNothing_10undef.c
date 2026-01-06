/*
 * DoNothing-10undef.kext
 * A bare do nothing kext.  It should have undefined symbols.
 */

#include <mach/mach_types.h>

#define UNUSED __attribute__((unused))

// These will turn into undefined symbols in the kext.  The count
// as well as the names themselves are checked in OSKext-tests.c

extern void do_nothing_1(void);
extern void do_nothing_2(void);
extern void do_nothing_3(void);
extern void do_nothing_4(void);
extern void do_nothing_5(void);
extern void do_nothing_6(void);
extern void do_nothing_7(void);
extern void do_nothing_8(void);
extern void do_nothing_9(void);
extern void do_nothing_10(void);

kern_return_t DoNothing_10undef_start(kmod_info_t * ki, void *d);
kern_return_t DoNothing_10undef_stop(kmod_info_t *ki, void *d);

kern_return_t DoNothing_10undef_start(UNUSED kmod_info_t * ki, UNUSED void *d)
{
    // reference functions to keep symbols from getting stripped.
    do_nothing_1();
    do_nothing_2();
    do_nothing_3();
    do_nothing_4();
    do_nothing_5();
    do_nothing_6();
    do_nothing_7();
    do_nothing_8();
    do_nothing_9();
    do_nothing_10();

    return KERN_SUCCESS;
}

kern_return_t DoNothing_10undef_stop(UNUSED kmod_info_t *ki, UNUSED void *d)
{
    return KERN_SUCCESS;
}
