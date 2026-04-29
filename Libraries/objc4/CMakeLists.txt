#add_executable(objcdt STATIC
#	./runtime/objc-cache.mm
#	./objcdt/json.mm
#	./objcdt/objcdt.mm
#	./runtime/objc-probes.d
#)

add_library(objc SHARED
	./runtime/hashtable2.mm
	./runtime/maptable.mm
	./runtime/objc-auto.mm
	./runtime/objc-cache.mm
	./runtime/objc-class-old.mm
	./runtime/objc-class.mm
	./runtime/objc-errors.mm
	./runtime/objc-exception.mm
	./runtime/objc-file.mm
	./runtime/objc-initialize.mm
	./runtime/objc-layout.mm
	./runtime/objc-load.mm
	./runtime/objc-loadmethod.mm
	./runtime/objc-lockdebug.mm
	./runtime/objc-runtime-new.mm
	./runtime/objc-runtime-old.mm
	./runtime/objc-runtime.mm
	./runtime/objc-sel-set.mm
	./runtime/objc-sel.mm
	./runtime/objc-sync.mm
	./runtime/objc-typeencoding.mm
	./runtime/Object.mm
	./runtime/Protocol.mm
	./runtime/OldClasses.subproj/List.m
	./runtime/Messengers.subproj/objc-msg-x86_64.s
	./runtime/objc-accessors.mm
	./runtime/objc-references.mm
	./runtime/objc-os.mm
	./runtime/objc-file-old.mm
	./runtime/objc-block-trampolines.mm
	./runtime/objc-sel-table.s
	./runtime/objc-weak.mm
	./runtime/NSObject.mm
	./runtime/objc-opt.mm
	./runtime/objc-zalloc.mm
	./runtime/objc-cache-old.mm
	./runtime/objc-sel-old.mm
	./macho.c

	./runtime/objc-probes.d # FIXME: need to generate .h from this
)
set_target_properties(objc PROPERTIES
    OUTPUT_NAME "objc.A" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)
set_source_files_properties(./runtime/objc-auto.mm TARGET_DIRECTORY objc PROPERTIES COMPILE_OPTIONS "-fexceptions")
set_source_files_properties(./runtime/objc-exception.mm TARGET_DIRECTORY objc PROPERTIES COMPILE_OPTIONS "-fexceptions")
set_source_files_properties(./runtime/objc-lockdebug.mm TARGET_DIRECTORY objc PROPERTIES COMPILE_OPTIONS "-Os")

target_compile_options(objc PRIVATE --sysroot=${RAVYN_SDKROOT}
    $<$<COMPILE_LANGUAGE:OBJC>:-std=gnu11>
    $<$<COMPILE_LANGUAGE:OBJCXX>:-std=gnu++14>
    $<$<COMPILE_LANGUAGE:CXX>:-D_LIBCPP_VISIBLE="">
    -mmacos-version-min=10.15 -momit-leaf-frame-pointer -fdollars-in-identifiers
    -fobjc-legacy-dispatch -fno-objc-convert-messages-to-runtime-calls -fblocks
    -Wno-deprecated-declarations)

target_link_options(objc PRIVATE
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    -Wl,-install_name,/usr/lib/libobjc.dylib -Wl,-current_version,228
    -Wl,-compatibility_version,1 -Wl,-order_file,${CMAKE_CURRENT_SOURCE_DIR}/libobjc.order
    -Wl,-interposable_list,${CMAKE_CURRENT_SOURCE_DIR}/interposable.txt
    -Wl,-sectalign,__DATA,__data,0x1000 -lc++ -lCrashReporterClient
    -Wl,-unexported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/unexported_symbols
    -mmacos-version-min=${MACOS_VERSION_MIN}
    -lobjc-trampolines -lCrashReporterClient -lsystem_c -lsystem_platform 
    -lsystem_pthread -lsystem_malloc -lunwind -lsystem_blocks -lsystem_kernel
    -ldispatch -ldyld
)
target_include_directories(objc PRIVATE runtime
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${CMAKE_CURRENT_BINARY_DIR}/../Libsystem/libsystem_kernel/mig_hdr/include
    ../Libsystem/private ../Libsystem/libsystem_blocks
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${ROOT_BINARY_DIR}/${CpuArch}
)

# SUPPORT_PREOPT and __OBJC2__ add support for the dyld objc shared cache
target_compile_definitions(objc PRIVATE _LIBC_NO_FEATURE_VERIFICATION
    TARGET_OS_IOSMAC=0 TARGET_OS_OSX=1 NDEBUG __OBJC2__ SUPPORT_PREOPT)

add_custom_command(
    DEPENDS libsystem_darwin
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libobjc-trampolines.dylib
	COMMAND ${CC} --target=${CpuArch}-apple-darwin -x assembler-with-cpp
	    --sysroot=${RAVYN_SDKROOT} -fpic -shared -Wl,-dylib
	    -o ${CMAKE_CURRENT_BINARY_DIR}/libobjc-trampolines.dylib
        -Wl,-install_name,/usr/lib/libobjc-trampolines.dylib
	-Wl,-current_version,228 -umbrella System
	-L${RAVYN_SDKROOT}/usr/lib/system -ldyld
	-mmacos-version-min=${MACOS_VERSION_MIN}
	    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/objc-blocktramps-x86_64.s
	    && ${CMAKE_COMMAND} -E make_directory ${SYSROOT_DIR}/usr/lib ${RAVYN_SDKROOT}/usr/lib
	    && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libobjc-trampolines.dylib
	        ${RAVYN_SDKROOT}/usr/lib/
        && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libobjc-trampolines.dylib
            ${SYSROOT_DIR}/usr/lib/
)
target_sources(objc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libobjc-trampolines.dylib)
add_dependencies(objc libsystem_darwin)

add_custom_command(TARGET objc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib ${SYSROOT_DIR}/usr/lib
        && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:objc> ${RAVYN_SDKROOT}/usr/lib/
        && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:objc> ${SYSROOT_DIR}/usr/lib/
        && ${CMAKE_COMMAND} -E create_symlink libobjc.A.dylib ${RAVYN_SDKROOT}/usr/lib/libobjc.dylib
        && ${CMAKE_COMMAND} -E create_symlink libobjc.A.dylib ${SYSROOT_DIR}/usr/lib/libobjc.dylib
)
