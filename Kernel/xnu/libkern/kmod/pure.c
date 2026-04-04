/*
 * Minimal C++ runtime support for XNU kernel modules.
 * Handles pure virtual function calls.
 */

#include <mach/mach_types.h>

extern void panic(const char * fmt, ...);

/*
 * __cxa_pure_virtual is called when a pure virtual function
 * is invoked. In kernel space, this is always a fatal error.
 */
__private_extern__ void
__cxa_pure_virtual(void)
{
    panic("Pure virtual function called in kernel extension");
}

/*
 * Some toolchains may emit this older symbol instead.
 */
__private_extern__ void
__pure_virtual(void)
{
    panic("Pure virtual function called (legacy handler)");
}
