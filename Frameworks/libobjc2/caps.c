#include "objc/capabilities.h"
#include <stdint.h>

/**
 * Bitmask of all of the capabilities compiled into this version of the
 * runtime.
 */
static const int32_t caps =
	(1<<OBJC_CAP_EXCEPTIONS) |
	(1<<OBJC_CAP_SYNCRONIZE) |
	(1<<OBJC_CAP_PROPERTIES) |
	(1<<OBJC_CAP_PROPERTY_INTROSPECTION) |
	(1<<OBJC_CAP_OPTIONAL_PROTOCOLS) |
	(1<<OBJC_CAP_NONFRAGILE_IVARS) |
	(1<<OBJC_DEVELOPER_MODE) |
	(1<<OBJC_CAP_REGISTERED_COMPATIBILITY_ALIASES) |
	(1<<OBJC_CAP_ARC) |
	(1<<OBJC_CAP_ASSOCIATED_REFERENCES) |
	(1<<OBJC_CAP_PROTOTYPES) |
#ifndef NO_OBJCXX
	(1<<OBJC_UNIFIED_EXCEPTION_MODEL) |
#endif
#ifdef TYPE_DEPENDENT_DISPATCH
	(1<<OBJC_CAP_TYPE_DEPENDENT_DISPATCH) |
#endif
#ifdef __OBJC_LOW_MEMORY__
	(1<<OBJC_CAP_LOW_MEMORY) |
#endif
#ifdef ENABLE_GC
	(1<<OBJC_CAP_GARBAGE_COLLECTION) |
#endif
#if defined(WITH_TRACING) && defined (__x86_64)
	(1<<OBJC_CAP_TRACING) |
#endif
	0;

OBJC_PUBLIC int objc_test_capability(int x)
{
	if (x >= 32) { return 0; }
	if (caps & (1<<x)) { return 1; }
	return 0;
}
