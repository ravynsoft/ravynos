set(CMAKE_INSTALL_PREFIX "")

add_library(libmalloc)
set_property(TARGET libmalloc PROPERTY OUTPUT_NAME "system_malloc")

target_sources(libmalloc PRIVATE
    src/bitarray.c src/empty.s src/frozen_malloc.c src/legacy_malloc.c
    src/magazine_large.c src/magazine_malloc.c src/magazine_medium.c
    src/magazine_rack.c src/magazine_small.c src/magazine_tiny.c
    src/malloc_common.c src/malloc_printf.c src/malloc.c src/nano_malloc.c
    src/msl_lite_support.c src/nano_malloc_common.c src/nanov2_malloc.c
    src/purgeable_malloc.c src/vm.c resolver/resolver.c
)
target_sources(libmalloc INTERFACE
    include/malloc/_malloc.h include/malloc/malloc.h
)
target_sources(libmalloc PRIVATE
    private/make_tapi_happy.h private/malloc_implementation.h
    private/malloc_private.h
)

target_include_directories(libmalloc PRIVATE
    src
    ../libplatform/include ../libplatform/private
    ../../Kernel/xnu/libsyscall/wrappers
    include ../dyld/include
    ../../Kernel/xnu/libsyscall/mach
    ../../Kernel/xnu/libsyscall
    private
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
    ${RAVYN_SDKROOT}/usr/include
    resolver
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
)

target_compile_definitions(libmalloc PRIVATE
    TARGET_OS_BRIDGE=0
)

target_compile_options(libmalloc PRIVATE
    --sysroot=${RAVYN_SDKROOT} -w
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
    -Wno-implicit-function-declaration
)

add_dependencies(libmalloc xnu_headers)

# add_custom_command(TARGET libfirehose_kernel
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libfirehose_kernel>
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
# )