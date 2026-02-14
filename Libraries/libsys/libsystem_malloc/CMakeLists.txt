add_library(libsystem_malloc SHARED)
set_target_properties(libsystem_malloc PROPERTIES
    OUTPUT_NAME "system_malloc" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

set(libmalloc_sources
    bitarray.c empty.s frozen_malloc.c legacy_malloc.c
    magazine_large.c magazine_malloc.c magazine_medium.c
    magazine_rack.c magazine_small.c magazine_tiny.c
    malloc_common.c malloc_printf.c malloc.c nano_malloc.c
    msl_lite_support.c nano_malloc_common.c nanov2_malloc.c
    purgeable_malloc.c vm.c resolver/resolver.c
)
set(libmalloc_incs
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${CMAKE_CURRENT_BINARY_DIR}/../libsystem_kernel/mig_hdr/include
    ../private internal resolver
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${ROOT_BINARY_DIR}/${CpuArch}
)
set(libmalloc_defs
    DISPATCH_USE_DTRACE=0 OSATOMIC_DEPRECATED=0 OSATOMIC_USE_INLINED=0
    OS_UNFAIR_LOCK_INLINE=0 _FORTIFY_SOURCE=0 NDEBUG
)
set(libmalloc_opts
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    -Wno-implicit-function-declaration -momit-leaf-frame-pointer
    -Wno-int-conversion -Wno-implicit-int -DPRIVATE
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_sources(libsystem_malloc PRIVATE ${libmalloc_sources})
target_include_directories(libsystem_malloc PRIVATE ${libmalloc_incs})
target_compile_definitions(libsystem_malloc PRIVATE ${libmalloc_defs})
target_compile_options(libsystem_malloc PRIVATE ${libmalloc_opts})

target_link_options(libsystem_malloc PRIVATE
    --sysroot=${RAVYN_SDKROOT}
    -mmacos-version-min=10.15 -flat_namespace -undefined warning
    -Wl,-install_name,/usr/lib/system/libsystem_malloc.dylib
    -Wl,-current_version,283 -Wl,-compatibility_version,1.0.0
    -rdynamic -Wl,-search_paths_first -all_load
    -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    -lCrashReporterClient -lsystem_platform -Wl,-upward-lsystem_c
    -Wl,-interposable_list,${CMAKE_CURRENT_SOURCE_DIR}/xcodeconfig/interposable.list
)

add_custom_command(TARGET libsystem_malloc POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_malloc>
		${RAVYN_SDKROOT}/usr/lib/system/
)
