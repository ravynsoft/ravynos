
#ifdef STRICT_MACOS_X
#  define OBJC_NONPORTABLE __attribute__((error("Function not supported by the Apple runtime")))
#else
#  define OBJC_NONPORTABLE
#endif

#if !defined(OBJC_DEPRECATED)
#  if !defined(__DEPRECATE_DIRECT_ACCESS) || defined(__OBJC_LEGACY_GNU_MODE__) || defined(__OBJC_RUNTIME_INTERNAL__)
#    define OBJC_DEPRECATED
#  else
#    if ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR >= 1))
#      define OBJC_DEPRECATED __attribute__((deprecated))
#    else
#      define OBJC_DEPRECATED
#    endif
#  endif
#endif

