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
	./dyld3/libdyldEntryVector.cpp)
set_source_files_properties(./dyld3/Loading.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fvisibility=hidden")
set_source_files_properties(./dyld3/ClosureFileSystemPhysical.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/Tracing.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./src/glue.c TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-builtin")
set_source_files_properties(./src/ImageLoaderMachOCompressed.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-O3")
set_source_files_properties(./dyld3/Loading.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/Diagnostics.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/MachOLoaded.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/MachOFile.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/MachOAnalyzer.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/Closure.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/ClosureWriter.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/ClosureBuilder.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")
set_source_files_properties(./dyld3/PathOverrides.cpp TARGET_DIRECTORY libdyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")

set_target_properties(libdyld PROPERTIES
	PREFIX "lib" OUTPUT_NAME "dyld" SUFFIX ".dylib" NO_SONAME true CXX_STANDARD 20)

target_compile_definitions(libdyld PRIVATE
	BUILDING_LIBDYLD WITHOUT_SANDBOX BROKEN_VECTOR_BASE __USING_SJLJ_EXCEPTIONS__=0
	_USE_EXTENDED_LOCALES_ __DARWIN_C_LEVEL=__DARWIN_C_FULL TARGET_OS_BRIDGE=0)

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
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_include_directories(libdyld PRIVATE
    ${SDK_SOURCE_DIR}/usr/include/c++/v1 src dyld3/shared-cache
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd dyld3
    ../Libsystem/libsystem_c/include ../Libsystem/libsystem_c/locale
    ../Libsystem/libsystem_c/locale/FreeBSD ../Libsystem/libsystem_c/stdtime
    ../Libsystem/libsystem_c/stdtime/FreeBSD ../Libsystem/private
    ../Libsystem/liblaunch ../openbsm
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS)

target_link_options(libdyld PRIVATE
    --target=${CpuArch}-apple-darwin
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    -Wl,-no_inits -Wl,-no_warn_unused_dylibs
    "-Wl,-unexported_symbol,__ZNSt3*"
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-install_name,/usr/lib/system/libdyld.dylib
    -Wl,-flat_namespace -Wl,-undefined,suppress
    -Wl,-current_version,732.2
    -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    -L${CMAKE_CURRENT_BINARY_DIR}/../llvm_target/libcxx/lib
    -lsystem_malloc -lsystem_pthread -lsystem_platform -lsystem_kernel
    -Wl,-upward-lsystem_platform -Wl,-upward-lsystem_malloc
    -Wl,-upward-lsystem_c -Wl,-upward-lsystem_pthread
    -Wl,-upward-lxpc
    -Wl,-upward-lsystem_blocks -Wl,-upward-lsystem_kernel
    -Wl,-upward-ldispatch
    -Wl,-upward-lcompiler_rt -lunwind -lc++ -lc++abi)

add_custom_command(TARGET libdyld POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libdyld>
		${RAVYN_SDKROOT}/usr/lib/system)

add_executable(dyld
	src/dyldStartup.s
 	src/dyldInitialization.cpp
 	src/dyld2.cpp
 	dyld3/ClosureFileSystemPhysical.cpp
 	src/dyldAPIs.cpp
 	src/dyldExceptions.c
 	src/dyld_debugger.cpp
 	dyld3/Tracing.cpp
 	src/glue.c
 	src/ImageLoaderMegaDylib.cpp
 	src/ImageLoader.cpp
 	src/ImageLoaderMachO.cpp
 	src/stub_binding_helper.s
 	src/dyldNew.cpp
 	src/ImageLoaderMachOClassic.cpp
 	src/ImageLoaderMachOCompressed.cpp
 	dyld3/Loading.cpp
 	dyld3/Diagnostics.cpp
 	dyld3/SharedCacheRuntime.cpp
 	dyld3/shared-cache/DyldSharedCache.cpp
 	dyld3/shared-cache/FileUtils.cpp
 	dyld3/MachOLoaded.cpp
 	dyld3/MachOFile.cpp
 	dyld3/MachOAnalyzer.cpp
 	dyld3/Closure.cpp
 	dyld3/ClosureWriter.cpp
 	dyld3/BootArgs.cpp
 	dyld3/ClosureBuilder.cpp
 	dyld3/PathOverrides.cpp
        src/mh_dylib_header_alias.s
 	src/missing_symbol_stubs.c)
set_target_properties(dyld PROPERTIES CXX_STANDARD 17)

set_source_files_properties(src/ImageLoaderMachOCompressed.cpp
    TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-O3")
set_source_files_properties(dyld3/Diagnostics.cpp
                            src/dyldInitialization.cpp
                            dyld3/ClosureFileSystemPhysical.cpp
                            src/dyld_debugger.cpp
                            dyld3/Tracing.cpp
                            src/dyldNew.cpp
                            dyld3/Loading.cpp
                            dyld3/SharedCacheRuntime.cpp
                            dyld3/shared-cache/DyldSharedCache.cpp
                            dyld3/shared-cache/FileUtils.cpp
                            dyld3/MachOLoaded.cpp
                            dyld3/MachOFile.cpp
                            dyld3/MachOAnalyzer.cpp
                            dyld3/Closure.cpp
                            dyld3/ClosureWriter.cpp
                            dyld3/BootArgs.cpp
                            dyld3/ClosureBuilder.cpp
                            dyld3/PathOverrides.cpp
    TARGET_DIRECTORY dyld PROPERTIES COMPILE_OPTIONS "-fno-exceptions")

target_compile_definitions(dyld PRIVATE
    BUILDING_DYLD DYLD_VERSION=732.1 WITHOUT_SANDBOX __USING_SJLJ_EXCEPTIONS__=0
    __DARWIN_C_LEVEL=__DARWIN_C_FULL TARGET_OS_BRIDGE=0 BROKEN_VECTOR_BASE
    "VIS_HIDDEN=__attribute__((visibility($<QUOTE>hidden$<QUOTE>)))"
    "_LIBCPP_BEGIN_NAMESPACE_STD=" "_LIBCPP_END_NAMESPACE_STD="
)

target_compile_options(dyld PRIVATE
	-isystem${RAVYN_SDKROOT}/usr/include/c++/v1
	-isystem${TOOLS}/../lib/clang/17)

target_compile_options(dyld PRIVATE
    --target=${CpuArch}-apple-darwin -DPRIVATE
    --sysroot=${RAVYN_SDKROOT}
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    -fno-stack-check
    -Wno-nullability-completeness -Wno-incompatible-sysroot -Wno-availability
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>

    -I${CMAKE_CURRENT_BINARY_DIR}
    -I${ROOT_BINARY_DIR}/${CpuArch}
    -I${ROOT_SOURCE_DIR}/Libraries/Libsystem/private
    -I${CMAKE_CURRENT_SOURCE_DIR}/include
    -I${CMAKE_CURRENT_SOURCE_DIR}/dyld3
    -I${CMAKE_CURRENT_SOURCE_DIR}/dyld3/shared-cache
    -I${CMAKE_CURRENT_SOURCE_DIR}/src
    -I${CMAKE_CURRENT_SOURCE_DIR}
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc
    -I${ROOT_SOURCE_DIR}/Kernel/Extensions/corecrypto/include
    -I${RAVYN_SDKROOT}/usr/local/include/kernel

    -isystem${RAVYN_SDKROOT}/usr/include
    -iframework${RAVYN_SDKROOT}/System/Library/Frameworks
)

add_custom_command(TARGET dyld POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib ${ROOT_BINARY_DIR}/sysroot/usr/lib
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dyld> ${RAVYN_SDKROOT}/usr/lib/
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dyld> ${ROOT_BINARY_DIR}/sysroot/usr/lib/)

set(libdyld_files
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/dyld_process_info.cpp.o
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/dyld_process_info_notify.cpp.o
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/dyld_stub_binder.s.o
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/dyldLock.cpp.o
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/glue.c.o
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/start_glue.s.o
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/threadLocalHelpers.s.o
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/libdyld.dir/src/threadLocalVariables.c.o)

add_custom_command(OUTPUT libdyld.a
    COMMAND ${CMAKE_AR} rcs libdyld.a ${libdyld_files}
    DEPENDS ${libdyld_files})
add_custom_target(libdyld_static DEPENDS libdyld.a)
add_dependencies(dyld libdyld_static libpthread_dyld)

target_link_options(dyld PRIVATE
    --target=${CpuArch}-apple-darwin
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT}
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -Wl,-current_version,732.2
    -Wl,-static -Wl,-dylinker -Wl,-fixup_chains -Wl,-data_const
    -Wl,-pagezero_size,0 -Wl,-image_base,0 -nodefaultlibs
    -Wl,-dylinker_install_name,/usr/lib/dyld -Wl,-dead_strip
    -Wl,-exported_symbol,__dyld_start  -e __dyld_start
    -Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/src/dyld.exp
    -Wl,-section_order,__DATA,__all_image_info:__nl_symbol_ptr:__got:__auth_ptr:__const:__crash_info:__data:__bss:__common
    ${CMAKE_CURRENT_BINARY_DIR}/libdyld.a
    -L${RAVYN_SDKROOT}/usr/local/lib/dyld -lc -lpthread -lplatform
    -L${ROOT_BINARY_DIR}/Libraries/Libsystem/libsystem_c -lFortifySource -lNetBSD -lBase -lPlatform
    -L${RAVYN_SDKROOT}/usr/lib/system -lCrashReporterClient
    -L${CMAKE_CURRENT_BINARY_DIR}/../llvm_target/libcxx/lib
    -L${RAVYN_SDKROOT}/usr/lib -lunwind -lc++ -lc++abi
    -L${RAVYN_SDKROOT}/usr/local/lib/kernel -lkernel 
)

set_target_properties(dyld PROPERTIES
      SKIP_BUILD_RPATH TRUE
      BUILD_WITH_INSTALL_RPATH FALSE)
