set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(tree-macros REQUIRED)
find_package(queue-macros REQUIRED)

add_library(rwlock OBJECT rwlock.c)
set_property(TARGET rwlock PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(rwlock PUBLIC Threads::Threads)
target_include_directories(rwlock
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_library(epoll-shim INTERFACE)
  add_library(epoll-shim::epoll-shim ALIAS epoll-shim)
  add_library(epoll-shim-interpose INTERFACE)
  add_library(epoll-shim::epoll-shim-interpose ALIAS epoll-shim-interpose)
  return()
endif()

add_library(wrap OBJECT wrap.c)
set_property(TARGET wrap PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(wrap PUBLIC Threads::Threads)
target_include_directories(wrap
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

macro(add_compat_target _name _condition)
  add_library(compat_${_name} OBJECT compat_${_name}.c)
  target_link_libraries(compat_${_name} PUBLIC wrap)
  set_property(TARGET compat_${_name} PROPERTY POSITION_INDEPENDENT_CODE ON)
  target_compile_options(
    compat_${_name}
    INTERFACE "SHELL:-include \"${CMAKE_CURRENT_LIST_DIR}/compat_${_name}.h\"")
  add_library(compat_enable_${_name} INTERFACE)
  if(${_condition})
    target_sources(compat_enable_${_name}
                   INTERFACE $<TARGET_OBJECTS:compat_${_name}>)
    target_link_libraries(compat_enable_${_name} INTERFACE compat_${_name})
    string(TOUPPER "${_name}" _upper_name)
    target_compile_definitions(compat_enable_${_name}
                               INTERFACE COMPAT_ENABLE_${_upper_name})
  endif()
endmacro()

include(CheckSymbolExists)

# FreeBSD 13 and NetBSD 10 support native eventfd descriptors. NetBSD 10
# supports native timerfd descriptors. Prefer them if available.
check_symbol_exists(eventfd "sys/eventfd.h" HAVE_EVENTFD)
check_symbol_exists(timerfd_create "sys/timerfd.h" HAVE_TIMERFD)

check_symbol_exists(kqueue1 "sys/event.h;sys/time.h" HAVE_KQUEUE1)
add_compat_target(kqueue1 "NOT;HAVE_KQUEUE1")
check_symbol_exists(sigandset "signal.h" HAVE_SIGANDSET)
check_symbol_exists(sigorset "signal.h" HAVE_SIGORSET)
check_symbol_exists(sigisemptyset "signal.h" HAVE_SIGISEMPTYSET)
add_compat_target(
  sigops "NOT;HAVE_SIGANDSET;AND;NOT;HAVE_SIGORSET;AND;NOT;HAVE_SIGISEMPTYSET")
include(CheckCSourceRuns)
check_c_source_runs(
  [=[
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
int main() {
  struct kevent kev;
  EV_SET(&kev, 0, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, 0, 0);
  int kq;
  return ((kq = kqueue()) >= 0 && kevent(kq, &kev, 1, NULL, 0, NULL) == 0) ? 0 : 1;
}
]=]
  ALLOWS_ONESHOT_TIMERS_WITH_TIMEOUT_ZERO)
add_library(evfilt_timer_quirks INTERFACE)
if(NOT ALLOWS_ONESHOT_TIMERS_WITH_TIMEOUT_ZERO)
  target_compile_definitions(
    evfilt_timer_quirks
    INTERFACE QUIRK_EVFILT_TIMER_DISALLOWS_ONESHOT_TIMEOUT_ZERO)
endif()
add_compat_target(pipe2 "APPLE")
add_compat_target(socket "APPLE")
add_compat_target(socketpair "APPLE")
add_compat_target(itimerspec "APPLE")
add_compat_target(sem "APPLE")
add_compat_target(ppoll "APPLE")

target_link_libraries(rwlock PUBLIC $<BUILD_INTERFACE:compat_enable_sem>)

add_library(
  epoll-shim
  epoll_shim_ctx.c
  epoll.c
  epollfd_ctx.c
  kqueue_event.c
  signalfd.c
  signalfd_ctx.c
  timespec_util.c)
if(NOT HAVE_EVENTFD)
  target_sources(epoll-shim PRIVATE eventfd.c eventfd_ctx.c)
endif()
if(NOT HAVE_TIMERFD)
  target_sources(epoll-shim PRIVATE timerfd.c timerfd_ctx.c)
endif()
include(GenerateExportHeader)
generate_export_header(epoll-shim BASE_NAME epoll_shim)
target_link_libraries(
  epoll-shim
  PRIVATE Threads::Threads #
          $<BUILD_INTERFACE:queue-macros::queue-macros>
          $<BUILD_INTERFACE:tree-macros::tree-macros> #
          $<BUILD_INTERFACE:evfilt_timer_quirks>
          $<BUILD_INTERFACE:compat_enable_kqueue1>
          $<BUILD_INTERFACE:compat_enable_ppoll>
          $<BUILD_INTERFACE:compat_enable_itimerspec>
          $<BUILD_INTERFACE:compat_enable_sigops>
          $<BUILD_INTERFACE:rwlock>
          $<BUILD_INTERFACE:wrap>)
if(HAVE_TIMERFD)
  target_compile_definitions(epoll-shim PRIVATE HAVE_TIMERFD)
endif()
target_compile_definitions(epoll-shim PRIVATE EPOLL_SHIM_DISABLE_WRAPPER_MACROS)
target_include_directories(
  epoll-shim
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/install-include>)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/pollrdhup.c"
  [=[
#define _GNU_SOURCE
#include <poll.h>
int main() {
  printf("0x%x", POLLRDHUP);
  return 0;
}
]=])
try_run(HAVE_POLLRDHUP_RUN_RESULT HAVE_POLLRDHUP
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/pollrdhup.c"
    RUN_OUTPUT_VARIABLE POLLRDHUP_VALUE)
if(NOT HAVE_POLLRDHUP)
  set(POLLRDHUP_VALUE "0x2000")
endif()

set(_headers
    "epoll-shim/detail/common.h" #
    "epoll-shim/detail/poll.h" #
    "epoll-shim/detail/read.h" #
    "epoll-shim/detail/write.h" #
    "sys/epoll.h" #
    "sys/signalfd.h")
if(NOT HAVE_EVENTFD)
  list(APPEND _headers "sys/eventfd.h")
endif()
if(NOT HAVE_TIMERFD)
  list(APPEND _headers "sys/timerfd.h")
endif()
foreach(_header IN LISTS _headers)
  configure_file("${PROJECT_SOURCE_DIR}/include/${_header}"
                 "${PROJECT_BINARY_DIR}/install-include/${_header}")
endforeach()

set_target_properties(epoll-shim PROPERTIES SOVERSION 0)

#

add_library(epoll-shim-interpose epoll_shim_interpose.c)
target_link_libraries(epoll-shim-interpose PUBLIC epoll-shim)
target_compile_definitions(epoll-shim-interpose
                           PUBLIC EPOLL_SHIM_DISABLE_WRAPPER_MACROS)
generate_export_header(epoll-shim-interpose BASE_NAME epoll_shim_interpose)
target_include_directories(
  epoll-shim-interpose PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties(epoll-shim-interpose PROPERTIES SOVERSION 0)
