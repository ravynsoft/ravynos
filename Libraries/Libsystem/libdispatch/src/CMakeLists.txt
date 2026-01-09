
include(SwiftSupport)
include(DTrace)

add_library(dispatch
              allocator.c
              apply.c
              benchmark.c
              data.c
              init.c
              introspection.c
              io.c
              mach.c
              object.c
              once.c
              queue.c
              semaphore.c
              source.c
              time.c
              transform.c
              voucher.c
              shims.c
              protocol.defs
              provider.d
              allocator_internal.h
              data_internal.h
              inline_internal.h
              internal.h
              introspection_internal.h
              io_internal.h
              mach_internal.h
              object_internal.h
              queue_internal.h
              semaphore_internal.h
              shims.h
              source_internal.h
              trace.h
              voucher_internal.h
              event/event.c
              event/event_config.h
              event/event_epoll.c
              event/event_internal.h
              event/event_kevent.c
              event/event_windows.c
              firehose/firehose_internal.h
              shims/android_stubs.h
              shims/atomic.h
              shims/atomic_sfb.h
              shims/getprogname.h
              shims/hw_config.h
              shims/lock.c
              shims/lock.h
              shims/perfmon.h
              shims/time.h
              shims/tsd.h
              shims/yield.h)
if(WIN32)
  target_sources(dispatch
                 PRIVATE
                   shims/generic_sys_queue.h
                   shims/generic_win_stubs.c
                   shims/generic_win_stubs.h
                   shims/getprogname.c)
endif()
if(DISPATCH_USE_INTERNAL_WORKQUEUE)
  target_sources(dispatch
                 PRIVATE
                   event/workqueue.c
                   event/workqueue_internal.h)
endif()
target_sources(dispatch
               PRIVATE
                 block.cpp)
if(HAVE_OBJC)
  target_sources(dispatch
                 PRIVATE
                   data.m
                   object.m)
endif()
if(ENABLE_SWIFT)
  set(swift_optimization_flags)
  if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    set(swift_optimization_flags -O)
  endif()
  add_swift_library(swiftDispatch
                    MODULE_NAME
                      Dispatch
                    MODULE_LINK_NAME
                      dispatch
                    MODULE_PATH
                      ${CMAKE_CURRENT_BINARY_DIR}/swift/Dispatch.swiftmodule
                    OUTPUT
                      ${CMAKE_CURRENT_BINARY_DIR}/swiftDispatch.o
                    SOURCES
                      swift/Block.swift
                      swift/Data.swift
                      swift/Dispatch.swift
                      swift/IO.swift
                      swift/Private.swift
                      swift/Queue.swift
                      swift/Source.swift
                      swift/Time.swift
                      swift/Wrapper.swift
                    TARGET
                      ${CMAKE_C_COMPILER_TARGET}
                    CFLAGS
                      -fblocks
                      -fmodule-map-file=${PROJECT_SOURCE_DIR}/dispatch/module.modulemap
                    SWIFT_FLAGS
                      -I ${PROJECT_SOURCE_DIR}
                      -I/usr/include
                      ${swift_optimization_flags}
                    DEPENDS
                      ${PROJECT_SOURCE_DIR}/dispatch/module.modulemap)
  target_sources(dispatch
                 PRIVATE
                   swift/DispatchStubs.cc
                   ${CMAKE_CURRENT_BINARY_DIR}/swiftDispatch.o)
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(dispatch
                          PRIVATE
                            swiftSwiftOnoneSupport)
  endif()
endif()
if(ENABLE_DTRACE)
  dtrace_usdt_probe(${CMAKE_CURRENT_SOURCE_DIR}/provider.d
                    OUTPUT_SOURCES
                      dispatch_dtrace_provider_headers)
  target_sources(dispatch
                 PRIVATE
                   ${dispatch_dtrace_provider_headers})
endif()
target_include_directories(dispatch
                           PRIVATE
                             ${PROJECT_BINARY_DIR}
                             ${PROJECT_SOURCE_DIR}
                             ${CMAKE_CURRENT_SOURCE_DIR}
                             ${CMAKE_CURRENT_BINARY_DIR}
                             ${PROJECT_SOURCE_DIR}/private)
target_include_directories(dispatch
                           SYSTEM BEFORE PRIVATE
                             "${BlocksRuntime_INCLUDE_DIR}")
if(WIN32)
  target_compile_definitions(dispatch
                             PRIVATE
                               _CRT_NONSTDC_NO_WARNINGS)
endif()
if(NOT "${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
  target_compile_options(dispatch PRIVATE -fno-exceptions)
endif()
if(DISPATCH_ENABLE_ASSERTS)
  target_compile_definitions(dispatch
                             PRIVATE
                               -DDISPATCH_DEBUG=1)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  target_compile_definitions(dispatch
                             PRIVATE
                               -D_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_SYSTEM_NAME STREQUAL Android)
  target_compile_options(dispatch
                         PRIVATE
                           -U_GNU_SOURCE)
endif()
if(BSD_OVERLAY_FOUND)
  target_compile_options(dispatch
                         PRIVATE
                           ${BSD_OVERLAY_CFLAGS})
endif()
if("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
  target_compile_options(dispatch
                         PRIVATE
                           /W3)
else()
  target_compile_options(dispatch
                         PRIVATE
                           -Wall)
endif()
# FIXME(compnerd) add check for -fblocks?
if("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
  target_compile_options(dispatch
                         PRIVATE
                           -Xclang -fblocks)
else()
  # FIXME(compnerd) add check for -momit-leaf-frame-pointer?
  target_compile_options(dispatch
                         PRIVATE
                           -fblocks
                           -momit-leaf-frame-pointer)
endif()
if(BSD_OVERLAY_FOUND)
  target_link_libraries(dispatch PRIVATE ${BSD_OVERLAY_LDFLAGS})
endif()
if(LibRT_FOUND)
  target_link_libraries(dispatch PRIVATE RT::rt)
endif()
target_link_libraries(dispatch
                      PRIVATE
                        Threads::Threads
                        BlocksRuntime::BlocksRuntime)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  target_link_libraries(dispatch
                        PRIVATE
                          WS2_32
                          WinMM
                          synchronization)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  set_property(TARGET dispatch
               APPEND_STRING
               PROPERTY LINK_FLAGS
                 "-Xlinker -compatibility_version -Xlinker 1"
                 "-Xlinker -current_version -Xlinker ${VERSION}"
                 "-Xlinker -dead_strip"
                 "-Xlinker -alias_list -Xlinker ${PROJECT_SOURCE_DIR}/xcodeconfig/libdispatch.aliases")
endif()
dispatch_set_linker(dispatch)

# Temporary staging; the various swift projects that depend on libdispatch
# all expect libdispatch.so to be in src/.libs/libdispatch.so
# So for now, make a copy so we don't have to do a coordinated commit across
# all the swift projects to change this assumption.
add_custom_command(TARGET dispatch POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory .libs
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dispatch> .libs
                   COMMENT "Copying libdispatch to .libs")

get_swift_host_arch(SWIFT_HOST_ARCH)

install(TARGETS
          dispatch
        DESTINATION
          "${INSTALL_TARGET_DIR}")
if(ENABLE_SWIFT)
  install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/swift/Dispatch.swiftmodule
            ${CMAKE_CURRENT_BINARY_DIR}/swift/Dispatch.swiftdoc
          DESTINATION
            "${INSTALL_TARGET_DIR}/${SWIFT_HOST_ARCH}")
endif()

