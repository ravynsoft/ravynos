add_library(libSystem SHARED)
target_sources(libSystem PRIVATE init.c CompatibilityHacks.c)
set_target_properties(libSystem PROPERTIES
    OUTPUT_NAME "System.B" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)


add_subdirectory(libsystem_kernel)
add_subdirectory(libsystem_platform)
add_subdirectory(libsystem_malloc)
add_subdirectory(libsystem_pthread)
#add_subdirectory(libsystem_notify)
add_subdirectory(libsystem_c)
add_subdirectory(libsystem_darwin)
#add_subdirectory(CommonCrypto)

target_include_directories(libSystem PRIVATE
    libsystem_c/darwin ../../Kernel/xnu/EXTERNAL_HEADERS
    ${RAVYN_SDKROOT}/usr/include/pthread private
    ../../Kernel/xnu/bsd ../../Kernel/xnu/libsyscall/mach
    ../../Kernel/xnu/libsyscall/wrappers
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
)

target_compile_definitions(libSystem PRIVATE
    __DARWIN_UNIX03 DYLD_MACOSX_VERSION_10_11=0x000A0B00
)

target_compile_options(libSystem PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE} -isystem ${RAVYN_SDKROOT}/usr/include
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

add_dependencies(libsystem_c libsystem_malloc libsystem_platform)
add_dependencies(libsystem_pthread libsystem_platform)
add_dependencies(libsystem_malloc libsystem_platform)
#add_dependencies(libsystem_platform compiler_rt)
#add_dependencies(libsystem_notify xnu_headers)
#add_dependencies(libcommonCrypto xnu_headers)
add_dependencies(libSystem 
    libsystem_c libsystem_malloc libsystem_pthread libsystem_platform
    #libsystem_notify libcommonCrypto
)

target_link_options(libSystem PRIVATE
    -mmacos-version-min=10.15 -v
    -rdynamic -nodefaultlibs -Wl,-search_paths_first -Wl,-reexport-lsystem_c
    -Wl,-reexport-lsystem_malloc -Wl,-reexport-lsystem_platform -Wl,-reexport-lsystem_pthread
)
