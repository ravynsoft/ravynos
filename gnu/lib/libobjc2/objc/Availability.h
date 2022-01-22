#if defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
#pragma clang system_header
#endif

#include "objc-visibility.h"

#ifdef STRICT_MACOS_X
#	define OBJC_NONPORTABLE __attribute__((error("Function not supported by the Apple runtime")))
#else
#	define OBJC_NONPORTABLE
#endif

#if !defined(__DEPRECATE_DIRECT_ACCESS) || defined(__OBJC_LEGACY_GNU_MODE__) || defined(__OBJC_RUNTIME_INTERNAL__)
#	define OBJC_DEPRECATED
#else
#	define OBJC_DEPRECATED __attribute__((deprecated))
#endif

#ifdef ERROR_UNSUPPORTED_RUNTIME_FUNCTIONS
#	define OBJC_GNUSTEP_RUNTIME_UNSUPPORTED(x) \
		__attribute__((error(x " not supported by this runtime")))
#else
#	define OBJC_GNUSTEP_RUNTIME_UNSUPPORTED(x)
#endif

