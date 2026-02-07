add_library(libsystem_asl SHARED)
set_target_properties(libsystem_asl PROPERTIES NO_SONAME true PREFIX "lib"
    OUTPUT_NAME "system_asl" SUFFIX ".dylib")

target_sources(libsystem_asl PRIVATE
    asl.c
    asl_client.c
    asl_core.c
    asl_fd.c
    asl_file.c
    asl_legacy1.c
    asl_msg.c
    asl_msg_list.c
    #asl_mt_shim.c
    asl_object.c
    asl_store.c
    asl_string.c
    asl_util.c
    syslog.c)

set(MIG_FLAGS -DPRIVATE -DXPC_DEBUG -DMACH -D__APPLE__ -fblocks
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=10.15)
target_compile_definitions(libsystem_asl PRIVATE XPC_DEBUG MACH __APPLE__
   __DARWIN_C_LEVEL=__DARWIN_C_FULL _DARWIN_C_SOURCE _POSIX_C_SOURCE=200809L)
target_compile_options(libsystem_asl PRIVATE -fblocks --sysroot=${RAVYN_SDKROOT}
    -mmacos-version-min=10.15 -Wno-implicit-function-declaration
)
target_include_directories(libsystem_asl PRIVATE
    .
    ../liblaunch
    ../../MiscLibs
    ../../openbsm
    ../libsystem_info/membership.subproj
    ${ROOT_SOURCE_DIR}/BSD/syslog/aslcommon
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
    ../private ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/asl_ipc.h
        ${CMAKE_CURRENT_BINARY_DIR}/asl_ipcServer.c
        ${CMAKE_CURRENT_BINARY_DIR}/asl_ipcUser.c
    COMMAND
        ${MIG} ${MIG_FLAGS} -header asl_ipc.h ${CMAKE_CURRENT_SOURCE_DIR}/asl_ipc.defs
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/asl_ipc.defs
)
target_sources(libsystem_asl PRIVATE asl_ipcUser.c asl_ipcServer.c)
target_link_options(libsystem_asl PRIVATE
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -umbrella System
    -Wl,-all_load -L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib
    -lCrashReporterClient -lcompiler_rt -ldyld -lsystem_kernel -lsystem_platform
    -lsystem_pthread -lsystem_malloc -lsystem_c -lsystem_blocks
    # -lsystem_trace -lsystem_notify -llaunch
    -lunwind -ldispatch -lxpc -ldispatch  -lxpc -lcompiler_rt
    #-Wl,-upward-lsystem_info
    -Wl,-undefined,warning -Wl,-flat_namespace
    -Wl,-install_name,/usr/lib/system/libsystem_asl.dylib -Wl,-current_version,377.0.1
    -Wl,-compatibility_version,1.0.0 -mmacos-version-min=10.15
)
add_dependencies(libsystem_asl libsystem_darwin dispatch)
add_custom_command(TARGET libsystem_asl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_asl> ${RAVYN_SDKROOT}/usr/lib/system/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_asl> ${SYSROOT_DIR}/usr/lib/system/
)
