add_library(libsystem_platform SHARED)
set_target_properties(libsystem_platform PROPERTIES
    OUTPUT_NAME "platform" SUFFIX ".dylib" PREFIX "libsystem_" NO_SONAME true)

add_library(libplatform STATIC)
set_target_properties(libplatform PROPERTIES
    OUTPUT_NAME "platform" PREFIX "libsystem_" SUFFIX ".a")

set(libplatform_sources
    simple/asl.c
    simple/getenv.c
    simple/string_io.c
    string/generic/memmove.c
    string/generic/strcpy.c
    string/generic/strlen.c
    string/generic/memccpy.c
    string/generic/ffsll.c
    string/generic/strcmp.c
    string/generic/memchr.c
    string/generic/strchr.c
    string/generic/bzero.c
    string/generic/flsll.c
    string/generic/memcmp.c
    string/generic/memset_pattern.c
    string/generic/strncpy.c
    string/generic/strnlen.c
    string/generic/strlcat.c
    string/generic/strlcpy.c
    string/generic/strstr.c
    string/generic/strncmp.c
    force_libplatform_to_build.c
    os/alloc_once.c
    os/semaphore.c
    ucontext/x86_64/_ctx_start.s
    ucontext/x86_64/_setcontext.s
    ucontext/x86_64/getcontext.s
    ucontext/generic/setcontext.c
    ucontext/generic/makecontext.c
    ucontext/generic/swapcontext.c
    ucontext/generic/getmcontext.c
    setjmp/x86_64/_setjmp.s
    setjmp/x86_64/setjmp.s
    setjmp/x86_64/_sigtramp.s
    setjmp/generic/setjmperr.c
    setjmp/generic/sigaction.c
    setjmp/generic/sigtramp.c
    init.c
    cachecontrol/x86_64/cache.s
    cachecontrol/generic/cache.c
    atomics/common/MKGetTimeBaseInfo.c
    atomics/x86_64/OSAtomic.s
    atomics/x86_64/pfz.s
    atomics/init.c
    introspection/introspection.c
)

set(libplatform_incs
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ../private internal
    os/resolver
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
)
set(libplatform_defs
    DISPATCH_USE_DTRACE=0 _FORTIFY_SOURCE=0 OS_UNFAIR_LOCK_INLINE=0
    OSATOMIC_USE_INLINED=0 OSATOMIC_DEPRECATED=0 OSSPINLOCK_USE_INLINED=0

)
set(libplatform_opts
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    -Wno-implicit-function-declaration -Wno-int-conversion -Wno-implicit-int
    -fno-stack-protector -fdollars-in-identifiers -fno-common -fverbose-asm
    -momit-leaf-frame-pointer -DPRIVATE
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_sources(libsystem_platform PRIVATE ${libplatform_sources} os/lock.c)
target_include_directories(libsystem_platform PRIVATE ${libplatform_incs})
target_compile_definitions(libsystem_platform PRIVATE ${libplatform_defs} LONG_BIT=64)
target_compile_options(libsystem_platform PRIVATE ${libplatform_opts}
    -isystem${RAVYN_SDKROOT}/usr/include/c++/v1)

set_source_files_properties(_setcontext.s os/lock.c
    TARGET_DIRECTORY libsystem_platform PROPERTIES
    COMPILE_OPTIONS "-include${RAVYN_SDKROOT}/usr/include/stdatomic.h")

set_source_files_properties(_setcontext.s os/lock.c
    TARGET_DIRECTORY libplatform PROPERTIES
    COMPILE_OPTIONS "-include${RAVYN_SDKROOT}/usr/include/stdatomic.h")

target_sources(libplatform PRIVATE ${libplatform_sources} os/lock.c)
target_include_directories(libplatform PRIVATE ${libplatform_incs})
target_compile_definitions(libplatform PRIVATE ${libplatform_defs}
    VARIANT_STATIC VARIANT_NO_RESOLVERS)
target_compile_options(libplatform PRIVATE ${libplatform_opts})

target_link_options(libsystem_platform PRIVATE
    --sysroot=${RAVYN_SDKROOT} -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-install_name,/usr/lib/system/libsystem_platform.dylib
    -Wl,-current_version,220 -Wl,-compatibility_version,1.0.0
    -Wl,-search_paths_first -umbrella System
    -L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib
    -L${ROOT_BINARY_DIR}/Libraries/llvm_target/compiler-rt/lib/ravynos
    -lsystem_kernel -ldyld
    -Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/xcodeconfig/libplatform.aliases
)

add_custom_command(TARGET libplatform POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
                ${RAVYN_SDKROOT}/usr/local/lib/system &&
                ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libplatform>
		${RAVYN_SDKROOT}/usr/local/lib/system/
)

add_custom_command(TARGET libsystem_platform POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_platform>
		${RAVYN_SDKROOT}/usr/lib/system/
)
