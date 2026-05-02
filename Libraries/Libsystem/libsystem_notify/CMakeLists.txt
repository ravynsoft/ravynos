add_library(libsystem_notify SHARED)
set_target_properties(libsystem_notify PROPERTIES
    OUTPUT_NAME "notify" SUFFIX ".dylib" PREFIX "libsystem_" NO_SONAME true)

# FIXME: zoe 2/10/26 Change notify_probes.h to .d and generate header with dtrace
target_sources(libsystem_notify PRIVATE libnotify.c notify_client.c table.c notify_probes.h)

set(CFLAGS -DPRIVATE -D__APPLE__ -fblocks -DDISPATCH_USE_DTRACE=0
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}
    -I${CMAKE_CURRENT_SOURCE_DIR}/../liblaunch
    -I${CMAKE_CURRENT_SOURCE_DIR}/../private
    -I${CMAKE_CURRENT_SOURCE_DIR}/../libsystem_c
    -I${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/private 
    -I${CMAKE_CURRENT_BINARY_DIR}
    -I${ROOT_BINARY_DIR}/${CpuArch}
    -I${RAVYN_SDKROOT}/usr/local/include/kernel
    -I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    -DNOTIFY_POST=notify_post -DNOTIFY_CHECK=notify_check
    -DNOTIFY_DELIVER_END=deliver_end -DNOTIFY_DELIVER_START=deliver_start
    -DNOTIFY_REGISTER_MACH_PORT=notify_register_mach_port)

set(MIG_FLAGS ${CFLAGS} -D__MigTypeCheck)

target_compile_options(libsystem_notify PRIVATE
    ${CFLAGS} -Wno-int-conversion -Wno-implicit-int
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
    "-include${RAVYN_SDKROOT}/usr/include/xpc/xpc.h"
    "-include${RAVYN_SDKROOT}/usr/include/xpc/launchd.h")

target_link_options(libsystem_notify PRIVATE
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -umbrella System
    -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system
    -Wl,-current_version,241 -Wl,-compatibility_version,1
    -Wl,-install_name,/usr/lib/system/libsystem_notify.dylib
    -mmacos-version-min=${MACOS_VERSION_MIN} -lxpc -lsystem_blocks
    -lsystem_c -lsystem_kernel -lsystem_platform -lsystem_asl -lsystem_malloc
    -ldispatch -llaunch -ldyld)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/notify_ipc.h
        ${CMAKE_CURRENT_BINARY_DIR}/notify_ipcUser.c
	${CMAKE_CURRENT_BINARY_DIR}/notify_ipcServer.c
    COMMAND
        ${MIG} ${MIG_FLAGS} -novouchers -header notify_ipc.h ${CMAKE_CURRENT_SOURCE_DIR}/notify_ipc.defs
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/notify_ipc.defs
)
target_sources(libsystem_notify PRIVATE notify_ipcUser.c)

add_dependencies(libsystem_notify libsystem_darwin dispatch)
add_custom_command(TARGET libsystem_notify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_notify> ${RAVYN_SDKROOT}/usr/lib/system/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_notify> ${SYSROOT_DIR}/usr/lib/system/
)

