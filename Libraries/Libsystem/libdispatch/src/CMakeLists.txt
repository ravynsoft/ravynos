set(CMAKE_C_VISIBILITY_PRESET hidden)
set(HAVE_LEAKS FALSE)

add_library(dispatch SHARED
    allocator.c
    apply.c
    benchmark.c
    block.cpp
    data.c
    data.m
    init.c
    introspection.c
    io.c
    mach.c
    object.c
    object.m
    once.c
    queue.c
    semaphore.c
    source.c
    time.c
    transform.c
    voucher.c
#    provider.d
    event/event.c
    event/event_kevent.c
    event/workqueue.c
    shims.c
    shims/lock.c
    shims/yield.c
    firehose/firehose_buffer.c
#    firehose/firehose_server.c
#    firehose/firehose_server_object.m
)

set_target_properties(dispatch PROPERTIES
    NO_SONAME true PREFIX "lib" SUFFIX ".dylib" OUTPUT_NAME "dispatch")

target_include_directories(dispatch PRIVATE
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    .. . ../private/dispatch ../../libsystem_blocks
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
    ../../libsystem_pthread ../../libsystem_platform/include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc
    ${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers
    ../../private
)

set(MIG_FLAGS --sysroot=${RAVYN_SDKROOT} -g -DPRIVATE -mmacos-version-min=10.15 -D__MigTypeCheck)
add_custom_command(
    DEPENDS protocol.defs
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/protocolServer.h
        ${CMAKE_CURRENT_BINARY_DIR}/protocol.h
        ${CMAKE_CURRENT_BINARY_DIR}/libdispatch_internal_protocolServer.c
        ${CMAKE_CURRENT_BINARY_DIR}/libdispatch_internal_protocolUser.c
    COMMAND ${MIG} ${MIG_FLAGS} -header protocol.h -sheader protocolServer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/protocol.defs
)
add_custom_command(
    DEPENDS firehose/firehose.defs
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/firehoseServer.h
        ${CMAKE_CURRENT_BINARY_DIR}/firehose.h
        ${CMAKE_CURRENT_BINARY_DIR}/firehoseServer.c
        ${CMAKE_CURRENT_BINARY_DIR}/firehoseUser.c
    COMMAND ${MIG} ${MIG_FLAGS} -header firehose.h -sheader firehoseServer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/firehose/firehose.defs
)
add_custom_command(
    DEPENDS firehose/firehose_reply.defs
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/firehose_replyServer.h
        ${CMAKE_CURRENT_BINARY_DIR}/firehose_reply.h
        ${CMAKE_CURRENT_BINARY_DIR}/firehoseReplyServer.c
        ${CMAKE_CURRENT_BINARY_DIR}/firehoseReplyUser.c
    COMMAND ${MIG} ${MIG_FLAGS} -header firehose_reply.h -sheader firehose_replyServer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/firehose/firehose_reply.defs
)
target_sources(dispatch PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/libdispatch_internal_protocolServer.c
    ${CMAKE_CURRENT_BINARY_DIR}/libdispatch_internal_protocolUser.c
    ${CMAKE_CURRENT_BINARY_DIR}/firehoseServer.c
    ${CMAKE_CURRENT_BINARY_DIR}/firehoseUser.c
    ${CMAKE_CURRENT_BINARY_DIR}/firehoseReplyServer.c
    ${CMAKE_CURRENT_BINARY_DIR}/firehoseReplyUser.c
)

target_compile_definitions(dispatch PRIVATE
    HAVE_CONFIG_H HAVE_MACH USE_MACH_SEM USE_OBJC
    OS_FIREHOSE_SPI DISPATCH_DEBUG)

target_compile_options(dispatch PRIVATE
   --sysroot=${RAVYN_SDKROOT} -DPRIVATE
   -fno-exceptions -fblocks -fdollars-in-identifiers
   -momit-leaf-frame-pointer -mmacos-version-min=10.15)

target_link_libraries(dispatch PRIVATE
    system_pthread system_blocks objc)

target_link_options(dispatch PRIVATE
    -L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib
    -Wl,-compatibility_version,1 -Wl,-flat_namespace -Wl,-undefined,warning
    -Wl,-current_version,1173.0.3
    -Wl,-install_name,/usr/lib/system/libdispatch.dylib
    -Wl,-dead_strip
    -Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/../xcodeconfig/libdispatch.aliases
    -Wl,-interposable_list,${CMAKE_CURRENT_SOURCE_DIR}/../xcodeconfig/libdispatch.interposable
    -Wl,-order_file,${CMAKE_CURRENT_SOURCE_DIR}/../xcodeconfig/libdispatch.order
    -mmacos-version-min=10.15 -umbrella System)

add_custom_command(TARGET dispatch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dispatch> ${RAVYN_SDKROOT}/usr/lib/system/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dispatch> ${SYSROOT_DIR}/usr/lib/system/
)

add_custom_command(OUTPUT
        "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
        "${PROJECT_SOURCE_DIR}/private/module.modulemap"
    COMMAND
       ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/dispatch/darwin/module.modulemap" "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
     COMMAND
       ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/private/darwin/module.modulemap" "${PROJECT_SOURCE_DIR}/private/module.modulemap")
add_custom_target(module-map-symlinks
                  DEPENDS
                     "${PROJECT_SOURCE_DIR}/dispatch/module.modulemap"
                     "${PROJECT_SOURCE_DIR}/private/module.modulemap")

if(FALSE)
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

  dtrace_usdt_probe(${CMAKE_CURRENT_SOURCE_DIR}/provider.d
                    OUTPUT_SOURCES
                      dispatch_dtrace_provider_headers)
  target_sources(dispatch
                 PRIVATE
                   ${dispatch_dtrace_provider_headers})
endif()
