add_library(libxpc SHARED)
set_target_properties(libxpc PROPERTIES NO_SONAME true PREFIX "lib"
    OUTPUT_NAME "xpc" SUFFIX ".dylib")

target_sources(libxpc PRIVATE
    mpack.c
    xpc_array.c
    xpc_connection.c
    xpc_dictionary.c
    xpc_misc.c
    xpc_pipe.c
    xpc_type.c
    xpc_runtime.c
    transports/mach.c
)
target_sources(libxpc INTERFACE
    activity.h
    base.h
    connection.h
    debug.h
    endpoint.h
    launchd.h
    xpc.h
)

target_compile_definitions(libxpc PRIVATE XPC_DEBUG MACH __APPLE__)
target_compile_options(libxpc PRIVATE -g -O0 -fblocks --sysroot=${RAVYN_SDKROOT}
    -isystem${RAVYN_SDKROOT}/usr/include/c++/v1
    -include${RAVYN_SDKROOT}/usr/include/stdatomic.h
)
target_include_directories(libxpc PRIVATE
    .
    ../liblaunch
    ../../MiscLibs
    ../../openbsm
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
)
target_link_options(libxpc PRIVATE
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -L${CMAKE_CURRENT_BINARY_DIR}/../../MiscLibs -lsbuf -lnv
    -L${RAVYN_SDKROOT}/usr/lib/system -ldispatch -lobjc
    -L${RAVYN_SDKROOT}/usr/lib -Wl,-undefined,warning -Wl,-flat_namespace
    -Wl,-install_name,/usr/lib/system/libxpc.dylib -Wl,-current_version,1738.0.17
    -Wl,-compatibility_version,1.0.0 -umbrella System -mmacos-version-min=10.15
)
add_dependencies(libxpc libsystem_darwin dispatch)
add_custom_command(TARGET libxpc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libxpc> ${RAVYN_SDKROOT}/usr/lib/system/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libxpc> ${SYSROOT_DIR}/usr/lib/system/
)
