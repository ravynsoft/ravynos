add_library(libSystem SHARED)
target_sources(libSystem PRIVATE init.c CompatibilityHacks.c stubs.c)
set_target_properties(libSystem PROPERTIES
    OUTPUT_NAME "System.B" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

# Library versions within libsystem were taken from Monterey System.tbd
# libcache.dylib 83
# libcommonCrypto.dylib 60165
# libcompiler_rt.dylib 101.2
# libcorecrypto.dylib 866.0.10
# libdispatch.dylib 1173.0.3
# libdyld.dylib 732.2
# libkeymgr.dylib 30
# liblaunch.dylib 1738.0.17
# libmacho.dylib 949.0.1
# libquarantine.dylib 110.0.3
# libremovefile.dylib 48
# libsystem_asl.dylib 377.0.1
# libsystem_blocks.dylib 74
# libsystem_c.dylib 1353.0.10
# libsystem_configuration.dylib 1061.0.2
# libsystem_coreservices.dylib 114
# libsystem_dnssd.dylib 1096.0.2
# libsystem_featureflags.dylib 17
# libsystem_kernel.dylib 6153.0.168
# libsystem_m.dylib 3178
# libsystem_malloc.dylib 283
# libsystem_notify.dylib 241
# libsystem_platform.dylib 220
# libsystem_pthread.dylib 416.0.6
# libsystem_sandbox.dylib 1217.0.27
# libsystem_secinit.dylib 62.0.2
# libsystem_trace.dylib 1147.0.3
# libunwind.dylib 35.4
# libxpc.dylib 1738.0.17
# libSystem.B.dylib 1281

add_subdirectory(libsystem_blocks)
add_subdirectory(libsystem_kernel)
add_subdirectory(libsystem_platform)
add_subdirectory(libsystem_malloc)
add_subdirectory(libsystem_pthread)
add_subdirectory(libsystem_notify)
add_subdirectory(libsystem_info)
add_subdirectory(libsystem_c)
add_subdirectory(libsystem_darwin)
add_subdirectory(libsystem_asl)
add_subdirectory(libsystem_m)
add_subdirectory(libsystem_trace)
add_subdirectory(libsystem_dnssd)
#add_subdirectory(CommonCrypto)
add_subdirectory(libxpc)
add_subdirectory(libdispatch)
add_subdirectory(liblaunch)
add_subdirectory(removefile)
add_subdirectory(copyfile)

target_include_directories(libSystem PRIVATE
    libsystem_c/darwin ../../Kernel/xnu/EXTERNAL_HEADERS
    ${RAVYN_SDKROOT}/usr/include/pthread private
    ../../Kernel/xnu/bsd ../../Kernel/xnu/libsyscall/mach
    ../../Kernel/xnu/libsyscall/wrappers libsystem_c/include
    ${ROOT_BINARY_DIR}/Kernel/xnu/bsd
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/${CpuArch}
    ${RAVYN_SDKROOT}/usr/include
    ${RAVYN_SDKROOT}/usr/local/include
)

target_compile_definitions(libSystem PRIVATE __DARWIN_UNIX03)

target_compile_options(libSystem PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    -include${CMAKE_CURRENT_BINARY_DIR}/config.${CpuArch}.normal.h
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int -fno-common
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

add_dependencies(libdyld libsystem_pthread libsystem_c)
add_dependencies(libsystem_c libsystem_malloc libsystem_platform)
add_dependencies(libsystem_pthread libsystem_platform)
add_dependencies(libsystem_malloc libsystem_platform CrashReporterClient)
add_dependencies(libsystem_platform compiler_rt libkernel)
add_dependencies(libplatform libkernel)
add_dependencies(libsystem_darwin libdyld libsystem_c)
add_dependencies(removefile libsystem_darwin)
add_dependencies(copyfile libsystem_darwin)
add_dependencies(libxpc libsystem_darwin)
add_dependencies(dispatch libsystem_darwin)
add_dependencies(liblaunch libxpc)
add_dependencies(libsystem_asl libxpc)
add_dependencies(libsystem_m libsystem_platform)
add_dependencies(libsystem_dnssd libsystem_darwin)
#add_dependencies(libcommonCrypto xnu_headers)
add_dependencies(libSystem 
    libsystem_c libsystem_malloc libsystem_pthread libsystem_platform libdyld
    libxpc dispatch libsystem_blocks liblaunch CrashReporterClient libsystem_m
    libsystem_notify libsystem_info copyfile removefile libsystem_trace
    libsystem_dnssd #libcommonCrypto
)
add_dependencies(xnu libSystem)

add_custom_command(OUTPUT config
   COMMAND ${CMAKE_COMMAND} -E env
        DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
        RUNTIME_SPEC_PATH=${TOOLCHAIN}/Library/Xcode/Specifications
        "PATH=${TOOLS}:$ENV{PATH}"
        ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/linker_arguments.sh
        ${CpuArch} normal ${CMAKE_CURRENT_BINARY_DIR} ${RAVYN_SDKROOT} ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(OUTPUT libunwind.dylib
    DEPENDS libunwind
    COMMAND ${CC} -shared -o ${RAVYN_SDKROOT}/usr/lib/system/libunwind.dylib 
        -Wl,-install_name,/usr/lib/system/libunwind.dylib 
        -Wl,-compatibility_version,1.0.0 -Wl,-current_version,35.4
        -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system
        -mmacosx-version-min=${MACOS_VERSION_MIN} -umbrella System
        -Wl,-all_load ${RAVYN_SDKROOT}/usr/lib/libunwind.a
        -lsystem_c -lsystem_pthread -lsystem_platform -lcompiler_rt
        -lsystem_malloc -ldyld
)

target_sources(libSystem PRIVATE config libunwind.dylib)

target_link_options(libSystem PRIVATE
    -mmacos-version-min=${MACOS_VERSION_MIN} -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/local/lib/system
    -L${RAVYN_SDKROOT}/usr/lib -Wl,-no_warn_inits
    -Wl,-install_name,/usr/lib/libSystem.B.dylib -Wl,-current_version,1254
    -Wl,-compatibility_version,1.0.0 -all_load
    -nodefaultlibs -Wl,-dylib -Wl,-search_paths_first
    -Wl,-reexport-lsystem_c -Wl,-reexport-lsystem_blocks -Wl,-reexport-lsystem_asl
    -Wl,-reexport-lsystem_malloc -Wl,-reexport-lsystem_platform
    -Wl,-reexport-lsystem_darwin -Wl,-reexport-lsystem_pthread
    -Wl,-reexport-lsystem_kernel -Wl,-reexport-lcompiler_rt
    -Wl,-reexport-lunwind -Wl,-reexport-lxpc -Wl,-reexport-ldyld
    -Wl,-reexport-llaunch -Wl,-reexport-ldispatch -Wl,-reexport-lsystem_m
    -Wl,-reexport-lsystem_notify -Wl,-reexport-lsystem_info -Wl,-reexport-lcopyfile
    -Wl,-reexport-lremovefile -Wl,-reexport-lsystem_trace -Wl,-reexport-lsystem_dnssd
    -Wl,-force_load,${RAVYN_SDKROOT}/usr/lib/system/libCrashReporterClient.a
    -Wl,-force_load,${ROOT_BINARY_DIR}/Libraries/Libsystem/libsystem_kernel/libmach.a
    -Wl,-force_load,${ROOT_BINARY_DIR}/Libraries/Libsystem/libsystem_kernel/libsyscalls.a
    -Wl,-force_load,${RAVYN_SDKROOT}/usr/local/lib/system/libc.a
    -L${CMAKE_CURRENT_BINARY_DIR}/../llvm_target/compiler-rt/lib/ravynos
    -lclang_rt.builtins-${CpuArch} -ldyld -lsystem_c -lc++
    -lsystem_platform -lsystem_kernel -lsystem_blocks -lsystem_malloc 
    -lsystem_info -lsystem_notify -lsystem_trace -lsystem_asl -lsystem_blocks
    -lsystem_pthread -ldispatch -lxpc -llaunch -lsystem_m)

add_custom_command(TARGET libSystem POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/create_dylib_symlinks.sh
        ${RAVYN_SDKROOT} install normal 
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/create_dylib_symlinks.sh
        ${SYSROOT_DIR} install normal 
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/create_system_framework.sh
        ${SYSROOT_DIR} - - - normal && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libSystem>
        ${RAVYN_SDKROOT}/usr/lib/ && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libSystem>
        ${SYSROOT_DIR}/usr/lib/ && ${CMAKE_COMMAND} -E copy_directory 
        ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/Resources
        ${SYSROOT_DIR}/System/Library/Frameworks/System.framework/Versions/B/Resources
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
        ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/Resources/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
        ${SYSROOT_DIR}/System/Library/Frameworks/System.framework/Versions/B/Resources/
)
