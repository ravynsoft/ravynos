//fake headers
#ifdef __APPLE__
#include_next <Availability.h>

/*
 * If anyone wants to run the linker *on* iOS devices then
 * it must be a jailbroken device anyway and thus I don't
 * care about prohibited APIs.
 */
#undef  __IOS_PROHIBITED
#define __IOS_PROHIBITED

#undef  __TVOS_PROHIBITED
#define __TVOS_PROHIBITED

#undef  __WATCHOS_PROHIBITED
#define __WATCHOS_PROHIBITED

#endif /* __APPLE__ */
