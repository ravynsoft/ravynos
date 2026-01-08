add_library(libsystem_pthread)
set_property(TARGET libsystem_pthread PROPERTY OUTPUT_NAME "system_pthread")
set_property(TARGET libsystem_pthread PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_pthread PROPERTY PREFIX "lib")

file(GLOB_RECURSE srcfiles LIST_DIRECTORIES false *.c *.s)

target_sources(libsystem_pthread PRIVATE ${srcfiles})
target_sources(libsystem_pthread INTERFACE ${interface})
target_sources(libsystem_pthread PRIVATE ${privhdrs})

target_include_directories(libsystem_pthread PRIVATE
    ../private resolver ${SDK_SOURCE_DIR}/usr/include
    ../libsystem_c/darwin ${RAVYN_SDKROOT}/usr/include
)

target_compile_definitions(libsystem_pthread PRIVATE
    DISPATCH_USE_DTRACE=0
    # memory_order_dependency=os_atomic_memory_order_dependency
    # memory_order_acquire=os_atomic_memory_order_acquire
    # memory_order_release=os_atomic_memory_order_release
    # memory_order_relaxed=os_atomic_memory_order_relaxed
    # OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1
    # OS_ATOMIC_CONFIG_MEMORY_ORDER_DEPENDENCY=1 
    # OS_ATOMIC_CONFIG_STARVATION_FREE_ONLY=0
    # __DARWIN_LITTLE_ENDIAN=1234 __DARWIN_BIG_ENDIAN=4321
    # __PTHREAD_EXPOSE_INTERNALS__=1
)

target_compile_options(libsystem_pthread PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
)
