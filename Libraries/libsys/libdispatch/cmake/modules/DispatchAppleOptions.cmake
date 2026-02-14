
set(WITH_APPLE_PTHREAD_SOURCE "" CACHE PATH "Path to Apple's libpthread")
set(WITH_APPLE_LIBPLATFORM_SOURCE "" CACHE PATH "Path to Apple's libplatform")
set(WITH_APPLE_LIBCLOSURE_SOURCE "" CACHE PATH "Path to Apple's libclosure")
set(WITH_APPLE_XNU_SOURCE "" CACHE PATH "Path to Apple's XNU")
set(WITH_APPLE_OBJC4_SOURCE "" CACHE PATH "Path to Apple's ObjC4")

if(WITH_APPLE_PTHREAD_SOURCE)
  include_directories(SYSTEM "${WITH_APPLE_PTHREAD_SOURCE}")
endif()
if(WITH_APPLE_LIBPLATFORM_SOURCE)
  include_directories(SYSTEM "${WITH_APPLE_LIBPLATFORM_SOURCE}/include")
endif()
if(WITH_APPLE_LIBCLOSURE_SOURCE)
  include_directories(SYSTEM "${WITH_APPLE_LIBCLOSURE_SOURCE}")
endif()
if(WITH_APPLE_XNU_SOURCE)
  # FIXME(compnerd) this should use -idirafter
  include_directories("${WITH_APPLE_XNU_SOURCE}/libkern")
  include_directories(SYSTEM
                        "${WITH_APPLE_XNU_SOURCE}/bsd"
                        "${WITH_APPLE_XNU_SOURCE}/libsyscall"
                        "${WITH_APPLE_XNU_SOURCE}/libsyscall/wrappers/libproc")

  # hack for xnu/bsd/sys/event.h EVFILT_SOCK declaration
  add_definitions(-DPRIVATE=1)
endif()

if(IS_DIRECTORY "/System/Library/Frameworks/System.framework/PrivateHeaders")
  include_directories(SYSTEM
                        "/System/Library/Frameworks/System.framework/PrivateHeaders")
endif()

option(ENABLE_APPLE_TSD_OPTIMIZATIONS "use non-portable pthread TSD optimizations" OFF)
if(ENABLE_APPLE_TSD_OPTIMIZATIONS)
  set(USE_APPLE_TSD_OPTIMIZATIONS 1)
else()
  set(USE_APPLE_TSD_OPTIMIZATIONS 0)
endif()

# TODO(compnerd) link in libpthread headers


