add_library(libSystem SHARED)
target_sources(libSystem PRIVATE init.c CompatibilityHacks.c)
set_property(TARGET libSystem PROPERTY OUTPUT_NAME "System")
set_property(TARGET libSystem PROPERTY SUFFIX ".B.dylib")

add_subdirectory(libsystem_platform)
add_subdirectory(libsystem_malloc)
add_subdirectory(libsystem_pthread)
add_subdirectory(libsystem_c)

# target_include_directories(libSystem PRIVATE
#     ../../Kernel/xnu/libsyscall/wrappers ../../Kernel/xnu/libsyscall/mach
#     ../libpthread ../libpthread/private ../libpthread/pthread
#     ../libmalloc/include ../../Kernel/xnu/bsd ../../Kernel/xnu/osfmk
#     ../../Kernel/xnu/EXTERNAL_HEADERS
#     ../../Kernel/xnu/libsyscall
#     ../dyld/include ../libmalloc/private
#     ${CMAKE_CURRENT_BINARY_DIR}/../../Kernel/xnu/EXPORT_HDRS/osfmk
#     ../libplatform/include ${SYSROOT_DIR}/usr/include
#     ../Libc ../../Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk
#     ../Libc/include ../../Developer/llvm-project/clang/lib/Headers
#     ../libplatform/private ../Libc/darwin
# )

target_compile_definitions(libSystem PRIVATE
    __DARWIN_UNIX03 DYLD_MACOSX_VERSION_10_11=0x000A0B00
)

target_compile_options(libSystem PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
)

target_link_libraries(libSystem
    libsystem_pthread libsystem_platform libsystem_malloc libsystem_c) #libkernel compiler_rt libdarwin

add_dependencies(libsystem_c xnu_headers)
add_dependencies(libsystem_pthread xnu_headers)
add_dependencies(libsystem_malloc xnu_headers)
add_dependencies(libsystem_platform xnu_headers)
add_dependencies(libSystem 
    libsystem_c libsystem_malloc libsystem_pthread libsystem_platform)

# add_custom_command(TARGET libSystem
#     POST_BUILD
#     COMMAND env PLATFORM_NAME=macosx sh
#         ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/create_system_framework.sh
#             ${SYSROOT_DIR} . none ${BuildArch} normal
#     COMMAND env PLATFORM_NAME=macosx sh
#         ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/create_system_framework.sh
#             ${RAVYN_SDKROOT} . none ${BuildArch} normal
# )
