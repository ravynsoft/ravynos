
cmake_minimum_required(VERSION 3.4.3)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

project(dispatch
        VERSION 1.3
        LANGUAGES C CXX)
enable_testing()

if("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
  include(ClangClCompileRules)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_VISIBILITY_PRESET hidden)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(GNUInstallDirs)
include(SwiftSupport)
include(DispatchUtilities)

set(SWIFT_LIBDIR "lib" CACHE PATH "Library folder name, defined by swift main buildscript")
set(INSTALL_LIBDIR "${SWIFT_LIBDIR}" CACHE PATH "Path where the libraries should be installed")

include(DispatchAppleOptions)
include(DispatchSanitization)

include(DispatchCompilerWarnings)
dispatch_common_warnings()

option(ENABLE_DISPATCH_INIT_CONSTRUCTOR "enable libdispatch_init as a constructor" ON)
set(USE_LIBDISPATCH_INIT_CONSTRUCTOR ${ENABLE_DISPATCH_INIT_CONSTRUCTOR})

option(ENABLE_SWIFT "enable libdispatch swift overlay" OFF)
if(ENABLE_SWIFT)
  if(NOT CMAKE_SWIFT_COMPILER)
    message(FATAL_ERROR "CMAKE_SWIFT_COMPILER must be defined to enable swift")
  endif()

  get_filename_component(SWIFT_TOOLCHAIN ${CMAKE_SWIFT_COMPILER} DIRECTORY)
  get_filename_component(SWIFT_TOOLCHAIN ${SWIFT_TOOLCHAIN} DIRECTORY)

  string(TOLOWER ${CMAKE_SYSTEM_NAME} SWIFT_OS)
  get_swift_host_arch(SWIFT_HOST_ARCH)

  set(SWIFT_RUNTIME_LIBDIR ${SWIFT_TOOLCHAIN}/${SWIFT_LIBDIR}/swift/${SWIFT_OS}/${SWIFT_HOST_ARCH})

  add_library(swiftCore
              SHARED IMPORTED GLOBAL)
  set_target_properties(swiftCore
                        PROPERTIES
                          IMPORTED_LOCATION
                            ${SWIFT_RUNTIME_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}swiftCore${CMAKE_SHARED_LIBRARY_SUFFIX})

  add_library(swiftSwiftOnoneSupport
              SHARED IMPORTED GLOBAL)
  set_target_properties(swiftSwiftOnoneSupport
                        PROPERTIES
                          IMPORTED_LOCATION
                           ${SWIFT_RUNTIME_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}swiftSwiftOnoneSupport${CMAKE_SHARED_LIBRARY_SUFFIX})

  set(INSTALL_TARGET_DIR "${INSTALL_LIBDIR}/swift/${SWIFT_OS}" CACHE PATH "Path where the libraries will be installed")
  set(INSTALL_DISPATCH_HEADERS_DIR "${INSTALL_LIBDIR}/swift/dispatch" CACHE PATH "Path where the headers will be installed for libdispatch")
  set(INSTALL_BLOCK_HEADERS_DIR "${INSTALL_LIBDIR}/swift/Block" CACHE PATH "Path where the headers will be installed for the blocks runtime")
  set(INSTALL_OS_HEADERS_DIR "${INSTALL_LIBDIR}/swift/os" CACHE PATH "Path where the os/ headers will be installed")
endif()

if(NOT ENABLE_SWIFT)
  set(INSTALL_TARGET_DIR "${INSTALL_LIBDIR}" CACHE PATH "Path where the libraries will be installed")
  set(INSTALL_DISPATCH_HEADERS_DIR "include/dispatch" CACHE PATH "Path where the headers will be installed")
  set(INSTALL_BLOCK_HEADERS_DIR "include" CACHE PATH "Path where the headers will be installed for the blocks runtime")
  set(INSTALL_OS_HEADERS_DIR "include/os" CACHE PATH "Path where the headers will be installed")
endif()

option(ENABLE_DTRACE "enable dtrace support" "")

option(BUILD_SHARED_LIBS "build shared libraries" ON)

option(ENABLE_TESTING "build libdispatch tests" ON)

option(USE_LLD_LINKER "use the lld linker" FALSE)

if(NOT USE_LLD_LINKER AND
   (CMAKE_SYSTEM_NAME STREQUAL Linux OR
    CMAKE_SYSTEM_NAME STREQUAL FreeBSD OR
    CMAKE_SYSTEM_NAME STREQUAL Android))
  set(USE_GOLD_LINKER_DEFAULT TRUE)
else()
  set(USE_GOLD_LINKER_DEFAULT FALSE)
endif()
option(USE_GOLD_LINKER "use the gold linker" ${USE_GOLD_LINKER_DEFAULT})

option(ENABLE_THREAD_LOCAL_STORAGE "enable usage of thread local storage via _Thread_local" ON)
set(DISPATCH_USE_THREAD_LOCAL_STORAGE ${ENABLE_THREAD_LOCAL_STORAGE})

if(CMAKE_SYSTEM_NAME STREQUAL Linux OR
   CMAKE_SYSTEM_NAME STREQUAL Android OR
   CMAKE_SYSTEM_NAME STREQUAL FreeBSD OR
   CMAKE_SYSTEM_NAME STREQUAL Windows)
  set(ENABLE_INTERNAL_PTHREAD_WORKQUEUES_DEFAULT ON)
else()
  set(ENABLE_INTERNAL_PTHREAD_WORKQUEUES_DEFAULT OFF)
endif()
option(ENABLE_INTERNAL_PTHREAD_WORKQUEUES "use libdispatch's own implementation of pthread workqueues" ${ENABLE_INTERNAL_PTHREAD_WORKQUEUES_DEFAULT})
if(ENABLE_INTERNAL_PTHREAD_WORKQUEUES)
  set(DISPATCH_USE_INTERNAL_WORKQUEUE 1)
  set(HAVE_PTHREAD_WORKQUEUES 0)
else()
  check_include_files(pthread/workqueue_private.h HAVE_PTHREAD_WORKQUEUE_PRIVATE_H)
  if(HAVE_PTHREAD_WORKQUEUE_PRIVATE_H)
    set(HAVE_PTHREAD_WORKQUEUES 1)
    set(DISPATCH_USE_INTERNAL_WORKQUEUE 0)
  else()
    set(HAVE_PTHREAD_WORKQUEUES 0)
    set(DISPATCH_USE_INTERNAL_WORKQUEUE 1)
  endif()
endif()

option(INSTALL_PRIVATE_HEADERS "installs private headers in the same location as the public ones" OFF)

find_package(BlocksRuntime QUIET)
if(NOT BlocksRuntime_FOUND)
  set(BlocksRuntime_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/BlocksRuntime)

  add_library(BlocksRuntime
              STATIC
                ${PROJECT_SOURCE_DIR}/src/BlocksRuntime/data.c
                ${PROJECT_SOURCE_DIR}/src/BlocksRuntime/runtime.c)
  set_target_properties(BlocksRuntime
                        PROPERTIES
                          POSITION_INDEPENDENT_CODE TRUE)
  if(HAVE_OBJC AND CMAKE_DL_LIBS)
    set_target_properties(BlocksRuntime
                          PROPERTIES
                            INTERFACE_LINK_LIBRARIES ${CMAKE_DL_LIBS})
  endif()

  add_library(BlocksRuntime::BlocksRuntime ALIAS BlocksRuntime)

  install(FILES
            ${PROJECT_SOURCE_DIR}/src/BlocksRuntime/Block.h
          DESTINATION
            "${INSTALL_BLOCK_HEADERS_DIR}")
  if(INSTALL_PRIVATE_HEADERS)
    install(FILES
              ${PROJECT_SOURCE_DIR}/src/BlocksRuntime/Block_private.h
            DESTINATION
              "${INSTALL_BLOCK_HEADERS_DIR}")
  endif()
endif()

check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)
if(_GNU_SOURCE)
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} -D_GNU_SOURCE)
endif()

check_c_source_compiles("void __attribute__((__noreturn__)) main() { __builtin_trap(); }"
                        __BUILTIN_TRAP)
if(__BUILTIN_TRAP)
  set(HAVE_NORETURN_BUILTIN_TRAP 1)
endif()

find_package(LibRT)

check_function_exists(_pthread_workqueue_init HAVE__PTHREAD_WORKQUEUE_INIT)
check_function_exists(getprogname HAVE_GETPROGNAME)
check_function_exists(mach_absolute_time HAVE_MACH_ABSOLUTE_TIME)
check_function_exists(mach_approximate_time HAVE_MACH_APPROXIMATE_TIME)
check_function_exists(mach_port_construct HAVE_MACH_PORT_CONSTRUCT)
check_function_exists(malloc_create_zone HAVE_MALLOC_CREATE_ZONE)
check_function_exists(pthread_key_init_np HAVE_PTHREAD_KEY_INIT_NP)
check_function_exists(pthread_main_np HAVE_PTHREAD_MAIN_NP)
check_function_exists(strlcpy HAVE_STRLCPY)
check_function_exists(sysconf HAVE_SYSCONF)
check_function_exists(arc4random HAVE_ARC4RANDOM)

if(NOT HAVE_STRLCPY AND NOT HAVE_GETPROGNAME)
  include(FindPkgConfig)
  pkg_check_modules(BSD_OVERLAY libbsd-overlay)
  if(BSD_OVERLAY_FOUND)
    set(HAVE_STRLCPY 1 CACHE INTERNAL "Have function strlcpy" FORCE)
    set(HAVE_GETPROGNAME 1 CACHE INTERNAL "Have function getprogname" FORCE)
  endif()
endif()

find_package(Threads REQUIRED)

check_include_files("TargetConditionals.h" HAVE_TARGETCONDITIONALS_H)
check_include_files("dlfcn.h" HAVE_DLFCN_H)
check_include_files("fcntl.h" HAVE_FCNTL_H)
check_include_files("inttypes.h" HAVE_INTTYPES_H)
check_include_files("libkern/OSAtomic.h" HAVE_LIBKERN_OSATOMIC_H)
check_include_files("libkern/OSCrossEndian.h" HAVE_LIBKERN_OSCROSSENDIAN_H)
check_include_files("libproc_internal.h" HAVE_LIBPROC_INTERNAL_H)
check_include_files("mach/mach.h" HAVE_MACH)
if(HAVE_MACH)
  set(__DARWIN_NON_CANCELABLE 1)
else()
  set(__DARWIN_NON_CANCELABLE 0)
endif()
check_include_files("malloc/malloc.h" HAVE_MALLOC_MALLOC_H)
check_include_files("memory.h" HAVE_MEMORY_H)
check_include_files("pthread/qos.h" HAVE_PTHREAD_QOS_H)
check_include_files("pthread/workqueue_private.h" HAVE_PTHREAD_WORKQUEUE_PRIVATE_H)
check_include_files("pthread_machdep.h" HAVE_PTHREAD_MACHDEP_H)
check_include_files("pthread_np.h" HAVE_PTHREAD_NP_H)
check_include_files("pthread_workqueue.h" HAVE_PTHREAD_WORKQUEUE_H)
check_include_files("stdint.h" HAVE_STDINT_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("string.h" HAVE_STRING_H)
check_include_files("strings.h" HAVE_STRINGS_H)
check_include_files("sys/guarded.h" HAVE_SYS_GUARDED_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("objc/objc-internal.h" HAVE_OBJC)

if(HAVE_MACH)
  set(USE_MACH_SEM 1)
else()
  set(USE_MACH_SEM 0)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  add_definitions(-DUSE_WIN32_SEM)
endif()
check_library_exists(pthread sem_init "" USE_POSIX_SEM)
# NOTE: android has not always provided a libpthread, but uses the pthreads API
if(CMAKE_SYSTEM_NAME STREQUAL Android)
  set(USE_POSIX_SEM 1)
endif()

check_symbol_exists(CLOCK_UPTIME "time.h" HAVE_DECL_CLOCK_UPTIME)
check_symbol_exists(CLOCK_UPTIME_FAST "time.h" HAVE_DECL_CLOCK_UPTIME_FAST)
check_symbol_exists(CLOCK_MONOTONIC "time.h" HAVE_DECL_CLOCK_MONOTONIC)
check_symbol_exists(CLOCK_REALTIME "time.h" HAVE_DECL_CLOCK_REALTIME)
check_symbol_exists(CLOCK_MONOTONIC_COARSE "time.h" HAVE_DECL_CLOCK_MONOTONIC_COARSE)
check_symbol_exists(FD_COPY "sys/select.h" HAVE_DECL_FD_COPY)
check_symbol_exists(NOTE_LOWAT "sys/event.h" HAVE_DECL_NOTE_LOWAT)
check_symbol_exists(NOTE_NONE "sys/event.h" HAVE_DECL_NOTE_NONE)
check_symbol_exists(NOTE_REAP "sys/event.h" HAVE_DECL_NOTE_REAP)
check_symbol_exists(NOTE_REVOKE "sys/event.h" HAVE_DECL_NOTE_REVOKE)
check_symbol_exists(NOTE_SIGNAL "sys/event.h" HAVE_DECL_NOTE_SIGNAL)
check_symbol_exists(POSIX_SPAWN_START_SUSPENDED "sys/spawn.h" HAVE_DECL_POSIX_SPAWN_START_SUSPENDED)
check_symbol_exists(SIGEMT "signal.h" HAVE_DECL_SIGEMT)
check_symbol_exists(VQ_DESIRED_DISK "sys/mount.h" HAVE_DECL_VQ_DESIRED_DISK)
check_symbol_exists(VQ_NEARLOWDISK "sys/mount.h" HAVE_DECL_VQ_NEARLOWDISK)
check_symbol_exists(VQ_QUOTA "sys/mount.h" HAVE_DECL_VQ_QUOTA)
check_symbol_exists(VQ_UPDATE "sys/mount.h" HAVE_DECL_VQ_UPDATE)
check_symbol_exists(VQ_VERYLOWDISK "sys/mount.h" HAVE_DECL_VQ_VERYLOWDISK)
check_symbol_exists(VQ_FREE_SPACE_CHANGE "sys/mount.h" HAVE_DECL_VQ_FREE_SPACE_CHANGE)
check_symbol_exists(strlcpy "string.h" HAVE_STRLCPY)
check_symbol_exists(program_invocation_name "errno.h" HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME)
if (HAVE_DECL_PROGRAM_INVOCATION_SHORT_NAME)
  add_definitions(-D_GNU_SOURCE=1)
endif()
check_symbol_exists(__printflike "bsd/sys/cdefs.h" HAVE_PRINTFLIKE)

if(CMAKE_SYSTEM_NAME STREQUAL Android)
  set(ENABLE_DTRACE_DEFAULT OFF)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
  add_definitions(-D_WITH_DPRINTF)
endif()

if(ENABLE_DTRACE STREQUAL "")
  find_program(dtrace_EXECUTABLE dtrace)
  if(dtrace_EXECUTABLE)
    add_definitions(-DDISPATCH_USE_DTRACE=1)
  else()
    add_definitions(-DDISPATCH_USE_DTRACE=0)
  endif()
elseif(ENABLE_DTRACE)
  find_program(dtrace_EXECUTABLE dtrace)
  if(NOT dtrace_EXECUTABLE)
    message(FATAL_ERROR "dtrace not found but explicitly requested")
  endif()
  add_definitions(-DDISPATCH_USE_DTRACE=1)
else()
  add_definitions(-DDISPATCH_USE_DTRACE=0)
endif()

find_program(leaks_EXECUTABLE leaks)
if(leaks_EXECUTABLE)
  set(HAVE_LEAKS TRUE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  add_custom_command(OUTPUT
                       "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                       "${PROJECT_SOURCE_DIR}/private/module.modulemap"
                     COMMAND
                       ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/dispatch/darwin/module.modulemap" "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                     COMMAND
                       ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/private/darwin/module.modulemap" "${PROJECT_SOURCE_DIR}/private/module.modulemap")
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows)
  add_custom_command(OUTPUT
                       "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                       "${PROJECT_SOURCE_DIR}/private/module.modulemap"
                     COMMAND
                       ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/dispatch/generic/module.modulemap" "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                     COMMAND
                       ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/private/generic/module.modulemap" "${PROJECT_SOURCE_DIR}/private/module.modulemap")
else()
  add_custom_command(OUTPUT
                       "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                       "${PROJECT_SOURCE_DIR}/private/module.modulemap"
                     COMMAND
                       ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/dispatch/generic/module.modulemap" "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                     COMMAND
                       ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/private/generic/module.modulemap" "${PROJECT_SOURCE_DIR}/private/module.modulemap")
endif()
add_custom_target(module-map-symlinks
                  DEPENDS
                     "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                     "${PROJECT_SOURCE_DIR}/private/module.modulemap")
configure_file("${PROJECT_SOURCE_DIR}/cmake/config.h.in"
               "${PROJECT_BINARY_DIR}/config/config_ac.h")
add_definitions(-DHAVE_CONFIG_H)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  include(DispatchWindowsSupport)
  dispatch_windows_arch_spelling(${CMAKE_SYSTEM_PROCESSOR} DISPATCH_MSVC_ARCH)
  dispatch_windows_include_for_arch(${DISPATCH_MSVC_ARCH} DISPATCH_INCLUDES)
  include_directories(BEFORE SYSTEM ${DISPATCH_INCLUDES})
  dispatch_windows_lib_for_arch(${CMAKE_SYSTEM_PROCESSOR} DISPATCH_LIBDIR)
  link_directories(${DISPATCH_LIBDIR})
endif()

add_subdirectory(dispatch)
add_subdirectory(man)
add_subdirectory(os)
add_subdirectory(private)
add_subdirectory(src)
if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()

