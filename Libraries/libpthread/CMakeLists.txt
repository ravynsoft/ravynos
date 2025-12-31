set(CMAKE_INSTALL_PREFIX "")

add_library(libpthread)
set_property(TARGET libpthread PROPERTY OUTPUT_NAME "system_pthread")

file(GLOB_RECURSE srcfiles LIST_DIRECTORIES false src/*.c src/*.s)
file(GLOB_RECURSE interface LIST_DIRECTORIES false include/*.h)
file(GLOB_RECURSE privhdrs LIST_DIRECTORIES false private/*.h internal/*.h)

target_sources(libpthread PRIVATE ${srcfiles})
target_sources(libpthread INTERFACE ${interface})
target_sources(libpthread PRIVATE ${privhdrs})

file(COPY private/pthread/qos_private.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/private/)
# file(COPY pthread/pthread.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/)

target_include_directories(libpthread PRIVATE
    src/resolver
    ../libplatform/include ../libplatform/private
    ../libmalloc/include ../libmalloc/private
    ../../Kernel/xnu/libsyscall/mach
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers
    private/pthread
    ../../Kernel/xnu/libsyscall
    private .
    ../../Kernel/xnu/libsyscall/wrappers
    ../dyld/include
    ../../Kernel/xnu/bsd
    ../../Kernel/xnu/libsyscall/wrappers/spawn
    pthread
)

target_compile_definitions(libpthread PRIVATE
    DISPATCH_USE_DTRACE=0
    memory_order_dependency=os_atomic_memory_order_dependency
    memory_order_acquire=os_atomic_memory_order_acquire
    memory_order_release=os_atomic_memory_order_release
    memory_order_relaxed=os_atomic_memory_order_relaxed
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
    OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 
    OS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0
    __DARWIN_LITTLE_ENDIAN=1234 __DARWIN_BIG_ENDIAN=4321
    __PTHREAD_EXPOSE_INTERNALS__=1
)

target_compile_options(libpthread PRIVATE
    --sysroot=${RAVYN_SDKROOT} -w
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
    -Wno-implicit-function-declaration
    -Wno-int-conversion
)

add_dependencies(libpthread libplatform)

add_custom_command(TARGET libpthread
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND env DSTROOT=${SYSROOT_DIR} sh
        ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/install-sys-headers.sh
    COMMAND env DSTROOT=${SYSROOT_DIR} sh
        ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/install-symlinks.sh
)