cmake_minimum_required(VERSION 3.10)
project(epoll-shim-tests LANGUAGES C)

#

include(CTest)

#

find_package(epoll-shim REQUIRED)

find_package(microatf REQUIRED)
include(ATFTest)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#

macro(atf_test_impl _testname _suffix)
  add_executable("${_testname}${_suffix}" "${_testname}.c")
  target_link_libraries(
    "${_testname}${_suffix}" PRIVATE epoll-shim::epoll-shim${_suffix}
                                     Threads::Threads microatf::microatf-c)
  if(APPLE)
    target_link_libraries(
      "${_testname}${_suffix}"
      PRIVATE wrap compat_enable_pipe2 compat_enable_ppoll
              compat_enable_socketpair compat_enable_socket
              compat_enable_itimerspec)
  endif()
  atf_discover_tests("${_testname}${_suffix}" ${ARGN})
endmacro()

macro(atf_test _testname)
  atf_test_impl("${_testname}" "" ${ARGN})
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    atf_test_impl("${_testname}" "-interpose" ${ARGN})
  endif()
endmacro()

#

atf_test(epoll-test)
add_library(epoll-shim-rdhup-linux-def INTERFACE)
target_link_libraries(epoll-shim-rdhup-linux-def INTERFACE epoll-shim::epoll-shim)
target_compile_definitions(epoll-shim-rdhup-linux-def INTERFACE -DUSE_EPOLLRDHUP_LINUX_DEFINITION)
add_library(epoll-shim::epoll-shim-rdhup-linux-def ALIAS epoll-shim-rdhup-linux-def)
atf_test_impl(epoll-test "-rdhup-linux-def")
foreach(_target epoll-test epoll-test-interpose epoll-test-rdhup-linux-def)
  if(TARGET ${_target})
    target_sources(${_target} PRIVATE real_close.c)
    target_link_libraries(${_target} PRIVATE ${CMAKE_DL_LIBS})
  endif()
endforeach()
atf_test(timerfd-test)
atf_test(timerfd-root-test)
atf_test(timerfd-mock-test)
atf_test(signalfd-test)
atf_test(perf-many-fds)
atf_test(atf-test)
atf_test(eventfd-ctx-test)
atf_test(pipe-test)
atf_test(socketpair-test)
get_target_property(_target_type epoll-shim::epoll-shim TYPE)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" #
   AND NOT _target_type STREQUAL STATIC_LIBRARY)
  atf_test(malloc-fail-test)
  foreach(_target malloc-fail-test malloc-fail-test-interpose)
    if(TARGET ${_target})
      target_sources(${_target} PRIVATE real_close.c)
      target_link_libraries(${_target} PRIVATE ${CMAKE_DL_LIBS})
    endif()
  endforeach()
endif()
atf_test(tst-epoll)
atf_test(tst-timerfd)

add_executable(rwlock-test rwlock-test.c)
target_link_libraries(rwlock-test PRIVATE rwlock microatf::microatf-c)
atf_discover_tests(rwlock-test)

add_executable(epoll-include-test epoll-include-test.c)
target_link_libraries(epoll-include-test PRIVATE epoll-shim::epoll-shim)
set_target_properties(
  epoll-include-test
  PROPERTIES C_STANDARD 99 #
             C_STANDARD_REQUIRED YES #
             C_EXTENSIONS NO)
add_test(NAME epoll-include-test COMMAND epoll-include-test)

add_executable(epoll-include-c89-test epoll-include-c89-test.c)
target_link_libraries(epoll-include-c89-test PRIVATE epoll-shim::epoll-shim)
set_target_properties(
  epoll-include-c89-test
  PROPERTIES C_STANDARD 90 #
             C_STANDARD_REQUIRED YES #
             C_EXTENSIONS NO)
add_test(NAME epoll-include-c89-test COMMAND epoll-include-c89-test)
target_compile_options(epoll-include-c89-test PRIVATE "-pedantic"
                                                      "-Werror=variadic-macros")

add_executable(fcntl-warning fcntl-warning.c)
target_link_libraries(fcntl-warning PRIVATE epoll-shim::epoll-shim)
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    fcntl-warning PRIVATE "-Werror" "-Wgnu-zero-variadic-macro-arguments")
endif()

add_executable(dlsym-fail epoll-include-test.c)
target_link_libraries(dlsym-fail PRIVATE c epoll-shim::epoll-shim)
add_test(NAME dlsym-fail COMMAND dlsym-fail)
