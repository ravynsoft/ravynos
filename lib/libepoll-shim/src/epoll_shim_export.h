
#ifndef EPOLL_SHIM_EXPORT_H
#define EPOLL_SHIM_EXPORT_H

#ifdef EPOLL_SHIM_STATIC_DEFINE
#  define EPOLL_SHIM_EXPORT
#  define EPOLL_SHIM_NO_EXPORT
#else
#  ifndef EPOLL_SHIM_EXPORT
#    ifdef epoll_shim_EXPORTS
        /* We are building this library */
#      define EPOLL_SHIM_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define EPOLL_SHIM_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef EPOLL_SHIM_NO_EXPORT
#    define EPOLL_SHIM_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef EPOLL_SHIM_DEPRECATED
#  define EPOLL_SHIM_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef EPOLL_SHIM_DEPRECATED_EXPORT
#  define EPOLL_SHIM_DEPRECATED_EXPORT EPOLL_SHIM_EXPORT EPOLL_SHIM_DEPRECATED
#endif

#ifndef EPOLL_SHIM_DEPRECATED_NO_EXPORT
#  define EPOLL_SHIM_DEPRECATED_NO_EXPORT EPOLL_SHIM_NO_EXPORT EPOLL_SHIM_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef EPOLL_SHIM_NO_DEPRECATED
#    define EPOLL_SHIM_NO_DEPRECATED
#  endif
#endif

#endif /* EPOLL_SHIM_EXPORT_H */
