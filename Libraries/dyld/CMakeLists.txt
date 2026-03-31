add_library(libdyld SHARED
	./src/start_glue.s
	./src/glue.c
	./dyld3/Tracing.cpp
	./dyld3/Closure.cpp
	./src/dyldLock.cpp
	./src/dyld_stub_binder.s
	./dyld3/ClosureFileSystemPhysical.cpp
	./src/dyldLibSystemGlue.c
	./src/dyldAPIsInLibSystem.cpp
	./src/threadLocalVariables.c
	./src/threadLocalHelpers.s
	./src/dyld_process_info.cpp
	./src/dyld_process_info_notify.cpp
	./dyld3/Diagnostics.cpp
	./dyld3/shared-cache/DyldSharedCache.cpp
	./dyld3/AllImages.cpp
	./dyld3/APIs.cpp
	./dyld3/APIs_macOS.cpp
	./dyld3/Logging.cpp
	./dyld3/Loading.cpp
	./dyld3/MachOFile.cpp
	./dyld3/MachOLoaded.cpp
	./dyld3/MachOAnalyzer.cpp
	./dyld3/PathOverrides.cpp
	./dyld3/ClosureBuilder.cpp
	./dyld3/ClosureWriter.cpp
	./dyld3/libdyldEntryVector.cpp
)
set_source_files_properties(./dyld3/Loading.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fvisibility=hidden")

set_target_properties(libdyld PROPERTIES
	PREFIX "lib" OUTPUT_NAME "dyld" SUFFIX ".dylib" NO_SONAME true CXX_STANDARD 20)

target_compile_definitions(libdyld PRIVATE
	BUILDING_LIBDYLD WITHOUT_SANDBOX BROKEN_VECTOR_BASE
	_USE_EXTENDED_LOCALES_ __DARWIN_C_LEVEL=__DARWIN_C_FULL TARGET_OS_BRIDGE=0
)

target_compile_options(libdyld PRIVATE
    --target=${CpuArch}-apple-darwin -DPRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    $<$<COMPILE_LANGUAGE:C,CXX>:-include${SDK_SOURCE_DIR}/usr/include/AvailabilityInternal.h>
    $<$<COMPILE_LANGUAGE:C,CXX>:-include${SDK_SOURCE_DIR}/usr/include/AvailabilityVersions.h>
    $<$<COMPILE_LANGUAGE:C,CXX>:-include${SDK_SOURCE_DIR}/usr/include/AvailabilityMacros.h>
    $<$<COMPILE_LANGUAGE:C,CXX>:-include${SDK_SOURCE_DIR}/usr/include/Availability.h>
    $<$<COMPILE_LANGUAGE:C,CXX>:-include${SDK_SOURCE_DIR}/usr/include/mach.h>
    $<$<COMPILE_LANGUAGE:C,CXX>:-include${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/include/xlocale/_stdlib.h>
    -Wno-nullability-completeness -Wno-incompatible-sysroot -Wno-availability
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_include_directories(libdyld PRIVATE
    ${SDK_SOURCE_DIR}/usr/include/c++/v1
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ../Libsystem/libsystem_c/include ../Libsystem/libsystem_c/locale
    ../Libsystem/libsystem_c/locale/FreeBSD ../Libsystem/libsystem_c/stdtime
    ../Libsystem/libsystem_c/stdtime/FreeBSD ../Libsystem/private
    ../Libsystem/liblaunch ../openbsm
    ../Libsystem/private include dyld3 dyld3/shared-cache launch-cache src
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc
    ${ROOT_BINARY_DIR}/${CpuArch}
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
)

target_link_options(libdyld PRIVATE
    --target=${CpuArch}-apple-darwin
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    -flat_namespace -undefined warning
    -Wl,-no_inits -Wl,-no_warn_unused_dylibs
    "-Wl,-unexported_symbol,__ZNSt3*"
    -mmacosx-version-min=10.15 -Wl,-install_name,/usr/lib/system/libdyld.dylib
    -Wl,-current_version,732.2
    -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    -lsystem_malloc -lsystem_pthread -lsystem_platform -lsystem_kernel
    -lsystem_c -lCrashReporterClient
    -Wl,-upward-lsystem_platform -Wl,-upward-lsystem_malloc
    -Wl,-upward-lsystem_c -Wl,-upward-lsystem_pthread
    # -Wl,-upward-lxpc
    -Wl,-upward-lsystem_blocks -Wl,-upward-lsystem_kernel
    # -Wl,-upward-ldispatch
    -Wl,-upward-lcompiler_rt -lunwind -lc++ -lc++abi
)

add_custom_command(TARGET libdyld POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libdyld>
		${RAVYN_SDKROOT}/usr/lib/system)

# add_executable(dyld
# 	./src/dyldStartup.s
# 	./src/dyldInitialization.cpp
# 	./src/dyld2.cpp
# 	./dyld3/ClosureFileSystemPhysical.cpp
# 	./src/dyldAPIs.cpp
# 	./src/dyldExceptions.c
# 	./src/dyld_debugger.cpp
# 	./dyld3/Tracing.cpp
# 	./src/glue.c
# 	./src/ImageLoaderMegaDylib.cpp
# 	./src/ImageLoader.cpp
# 	./src/ImageLoaderMachO.cpp
# 	./src/stub_binding_helper.s
# 	./src/dyldNew.cpp
# 	./src/ImageLoaderMachOClassic.cpp
# 	./src/ImageLoaderMachOCompressed.cpp
# 	./dyld3/Loading.cpp
# 	./dyld3/Diagnostics.cpp
# 	./dyld3/SharedCacheRuntime.cpp
# 	./dyld3/shared-cache/DyldSharedCache.cpp
# 	./dyld3/shared-cache/FileUtils.cpp
# 	./dyld3/MachOLoaded.cpp
# 	./dyld3/MachOFile.cpp
# 	./dyld3/MachOAnalyzer.cpp
# 	./dyld3/Closure.cpp
# 	./dyld3/ClosureWriter.cpp
# 	./dyld3/BootArgs.cpp
# 	./dyld3/ClosureBuilder.cpp
# 	./dyld3/PathOverrides.cpp
# )
# set_source_files_properties(./dyld3/ClosureFileSystemPhysical.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/Tracing.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./src/glue.c TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-builtin")
# set_source_files_properties(./src/ImageLoaderMachOCompressed.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-O3")
# set_source_files_properties(./dyld3/Loading.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/Diagnostics.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/MachOLoaded.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/MachOFile.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/MachOAnalyzer.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/Closure.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/ClosureWriter.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/ClosureBuilder.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
# set_source_files_properties(./dyld3/PathOverrides.cpp TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
