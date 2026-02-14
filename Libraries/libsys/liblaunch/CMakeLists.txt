add_library(liblaunch SHARED libbootstrap.c liblaunch.c libvproc.c)

set_target_properties(liblaunch PROPERTIES PREFIX "lib" OUTPUT_NAME "launch"
    SUFFIX ".dylib" NO_SONAME true)

set(CFLAGS --sysroot=${RAVYN_SDKROOT} -g -DPRIVATE -mmacos-version-min=${MACOS_VERSION_MIN})
set(MIG_FLAGS ${CFLAGS} -D__MigTypeCheck)
target_compile_options(liblaunch PRIVATE ${CFLAGS}
    -include${RAVYN_SDKROOT}/usr/include/sys/_types/_mach_port_t.h
    -include${RAVYN_SDKROOT}/usr/include/stdatomic.h
)
target_include_directories(liblaunch PRIVATE
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    .
    ../../openbsm
    ../libdispatch/config
    ../libsystem_pthread
    ../libsystem_c/fbsdcompat
    ${ROOT_SOURCE_DIR}/BSD/sbin/launchd
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc
    ../
    ${CMAKE_CURRENT_BINARY_DIR}
    ../private ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/san
)
add_custom_target(helperMIG
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/helper.defs
    COMMAND ${MIG} ${MIG_FLAGS} -header helper.h -sheader helperServer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/helper.defs
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/helper.h
        ${CMAKE_CURRENT_BINARY_DIR}/helperServer.h
        ${CMAKE_CURRENT_BINARY_DIR}/launchd_helperUser.c
        ${CMAKE_CURRENT_BINARY_DIR}/launchd_helperServer.c
)
add_dependencies(helperMIG libxpc)
add_custom_target(jobMIG
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/job.defs
    COMMAND ${MIG} ${MIG_FLAGS} -sheader jobServer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/job.defs
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/jobServer.h
        ${CMAKE_CURRENT_BINARY_DIR}/jobUser.c
        ${CMAKE_CURRENT_BINARY_DIR}/job.h
)
add_dependencies(jobMIG libxpc)

target_sources(liblaunch PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/job.h
    ${CMAKE_CURRENT_BINARY_DIR}/jobServer.h
    ${CMAKE_CURRENT_BINARY_DIR}/jobUser.c
    ${CMAKE_CURRENT_BINARY_DIR}/helper.h
    ${CMAKE_CURRENT_BINARY_DIR}/helperServer.h
    ${CMAKE_CURRENT_BINARY_DIR}/launchd_helperUser.c
    ${CMAKE_CURRENT_BINARY_DIR}/launchd_helperServer.c)
add_dependencies(liblaunch jobMIG helperMIG)

target_link_options(liblaunch PRIVATE
    -Wl,-current_version,1738.0.17 -Wl,-compatibility_version,1.0.0
    -Wl,-install_name,/usr/lib/system/liblaunch.dylib -umbrella System
    -L${RAVYN_SDKROOT}/usr/lib/system -lcompiler_rt -ldispatch -lsystem_c
    -lsystem_asl -mmacos-version-min=10.15)

add_dependencies(liblaunch libsystem_darwin libsystem_asl libxpc)
add_custom_command(TARGET liblaunch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:liblaunch> ${RAVYN_SDKROOT}/usr/lib/system/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:liblaunch> ${SYSROOT_DIR}/usr/lib/system/
)
