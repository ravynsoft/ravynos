set(CMAKE_INSTALL_PREFIX "")

add_darwin_shared_library(libSystem SHARED 
    INSTALL_NAME_DIR ${RAVYN_SDKROOT}/usr/lib
)
target_sources(libSystem PRIVATE init.c CompatibilityHacks.c)
# set_property(TARGET libSystem PROPERTY OUTPUT_NAME "System")

target_include_directories(libSystem PRIVATE
    ../../Kernel/xnu/libsyscall/wrappers ../../Kernel/xnu/libsyscall/mach
    ../libpthread ../libpthread/private ../libpthread/pthread
    ../libmalloc/include ../../Kernel/xnu/bsd ../../Kernel/xnu/osfmk
    ../../Kernel/xnu/EXTERNAL_HEADERS
    ../../Kernel/xnu/libsyscall
    ../dyld/include ../libmalloc/private
    ${CMAKE_CURRENT_BINARY_DIR}/../../Kernel/xnu/EXPORT_HDRS/osfmk
    ../libplatform/include ${SYSROOT_DIR}/usr/include
    ../Libc ../../Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk
    ../Libc/include ../../Developer/llvm-project/clang/lib/Headers
    ../libplatform/private ../Libc/darwin
)
target_compile_definitions(libSystem PRIVATE
    __DARWIN_UNIX03 DYLD_MACOSX_VERSION_10_11=0x000A0B00
)

target_compile_options(libSystem PRIVATE
    --sysroot=${RAVYN_SDKROOT} -w
    -F${RAVYN_SDKROOT}/System/Library/Frameworks
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
)

target_link_libraries(libSystem libpthread libplatform libmalloc libc)

add_dependencies(libSystem
    xnu_headers libc libplatform libpthread libmalloc)
