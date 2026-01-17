file(GLOB mach_defs ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/mach/*.defs)
file(COPY ${mach_defs} 
    DESTINATION ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/mach/)

set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall)
add_custom_command(OUTPUT libc-features.h
	COMMAND env "ARCHS=${CpuArch} "	VARIANT_PLATFORM_NAME=macosx
		DERIVED_FILES_DIR=${CMAKE_CURRENT_BINARY_DIR}
		SRCROOT=${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c
		perl ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/xcodescripts/generate_features.pl
)

set(common_headers 
    ../private ${CMAKE_CURRENT_BINARY_DIR}/${CpuArch}
    
    ${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include
    ${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/local/include
    ${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include
    
    ${CMAKE_CURRENT_SOURCE_DIR}/os
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/spawn

    ${CMAKE_CURRENT_SOURCE_DIR}/../bsd
    ${CMAKE_CURRENT_SOURCE_DIR}/../osfmk
    ${CMAKE_CURRENT_SOURCE_DIR}/../libkern
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/san
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/darwin
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/locale
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/locale/FreeBSD
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/stdtime/FreeBSD
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/private
    ${ROOT_SOURCE_DIR}/Frameworks
    ${RAVYN_SDKROOT}/usr/include/pthread

    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/libproc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mach

    ${SDK_SOURCE_DIR}/usr/include
    ${SDK_SOURCE_DIR}/usr/include/c++/v1
    ${ROOT_BINARY_DIR}/Kernel/xnu/bsd
    ${RAVYN_SDKROOT}/usr/local/include
)

set(common_defs
    LIBSYSCALL_INTERFACE CF_OPEN_SOURCE CF_EXCLUDE_CSTD_HEADERS
    _FORTIFY_SOURCE=0 SYSCALL_PRE1050 __OS_EXPOSE_INTERNALS__
)

set(common_opts
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    -mmacosx-version-min=10.15 -Wno-implicit-int
    -Wno-implicit-function-declaration -Wno-int-conversion
    -fdollars-in-identifiers -no-cpp-precomp -fno-common 
    -fno-stack-protector -fno-stack-check -momit-leaf-frame-pointer
    -DPRIVATE
)
set(libmach_opts
    --sysroot=${RAVYN_SDKROOT} -isysroot${RAVYN_SDKROOT} -w
    -Wno-implicit-function-declaration
    -fdollars-in-identifiers -no-cpp-precomp -fno-common
    -fno-stack-protector -fno-stack-check -momit-leaf-frame-pointer
)

# Generate syscall stubs
add_library(libsyscalls STATIC IMPORTED)
set_target_properties(libsyscalls PROPERTIES 
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libsyscalls.a
)
add_custom_command(
    OUTPUT
        LIBKERNEL_STUBS
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/sys
    COMMAND env
        SDKROOT=${RAVYN_SDKROOT}
        BUILT_PRODUCTS_DIR=${CMAKE_CURRENT_BINARY_DIR}
        DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
        "PATH=${TOOLS}:$ENV{PATH}" ARCHS=${CpuArch}
        SRCROOT=${CMAKE_CURRENT_SOURCE_DIR}
        OBJROOT=${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/create-syscalls.pl
            ${CMAKE_CURRENT_SOURCE_DIR}/../bsd/kern/syscalls.master
            ${CMAKE_CURRENT_SOURCE_DIR}/custom
            ${CMAKE_CURRENT_SOURCE_DIR}/Platforms MacOSX
            ${CMAKE_CURRENT_BINARY_DIR}/sys
    COMMAND env
        SDKROOT=${RAVYN_SDKROOT} SDK_INSTALL_HEADERS_ROOT=.
        BUILT_PRODUCTS_DIR=${CMAKE_CURRENT_BINARY_DIR}
        DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
        "PATH=${TOOLS}:$ENV{PATH}" ARCHS=${CpuArch}
        SRCROOT=${CMAKE_CURRENT_SOURCE_DIR}
        OBJROOT=${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/compile-syscalls.pl
            ${CMAKE_CURRENT_BINARY_DIR}/sys/stubs.list
            ${CMAKE_CURRENT_BINARY_DIR}/libsyscalls.a
)

# Install the mach defs before generating headers
add_custom_command(
    OUTPUT 
        MIG_HEADERS
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/obj
    COMMAND env
        SDKROOT=${RAVYN_SDKROOT} PLATFORM_NAME=MacOSX ARCHS=${CpuArch}
        SRCROOT=${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall
        OBJROOT=${CMAKE_CURRENT_BINARY_DIR}/obj
        BUILT_PRODUCTS_DIR=${CMAKE_CURRENT_BINARY_DIR}
        DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
        "PATH=${TOOLS}:$ENV{PATH}"
        sh ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/mach_install_mig.sh
    USES_TERMINAL
)
set_source_files_properties(MIG_HEADERS PROPERTIES SYMBOLIC "true")

set(libmach_inc
    -I${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include
    -I${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include/mach
    -I${CMAKE_CURRENT_BINARY_DIR}/${CpuArch}
    -I${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include/mach
    -I${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include
    -I${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include/servers
)
set(defs
    -DLIBSYSCALL_INTERFACE -DPRIVATE -DVARIANT_CANCELABLE -D__DARWIN_UNIX03
    -Dsemaphore_consume_ref_t=mach_port_t
    -Dthread_act_consume_ref_t=mach_port_t
)
add_custom_target(libmach
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/obj
    COMMAND
        set -e $<SEMICOLON>
        for f in ${CMAKE_CURRENT_BINARY_DIR}/obj/*User.c$<SEMICOLON>
        do
           ${CC} -Wno-incompatible-sysroot ${defs} ${libmach_opts} 
                ${libmach_inc} -o $$f.o -c $$f $<SEMICOLON>
        done
    COMMAND
        ${TOOLS}/llvm-ar rcs ${CMAKE_CURRENT_BINARY_DIR}/libmach.a
            ${CMAKE_CURRENT_BINARY_DIR}/obj/*.o
)

add_library(libkernel_legacy OBJECT
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/accept.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/bind.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/getattrlist.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/getpeername.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/getsockname.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/lchown.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/listen.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/recvfrom.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/recvmsg.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/sendmsg.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/sendto.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/setattrlist.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/socketpair.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/connect.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/mprotect.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/msync.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/munmap.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/open.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/select-pre1050.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/munmap.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/getaudit.c
)
target_compile_definitions(libkernel_legacy PRIVATE
    VARIANT_LEGACY __DARWIN_UNIX03=0 NO_SYSCALL_LEGACY
)
target_compile_options(libkernel_legacy PRIVATE ${common_opts})
target_include_directories(libkernel_legacy PRIVATE ${common_headers})

add_dependencies(libkernel_legacy xnu_headers)

add_library(libkernel_static STATIC
    MIG_HEADERS
    LIBKERNEL_STUBS
    libc-features.h
    ${CMAKE_CURRENT_SOURCE_DIR}/custom/errno.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/pselect-darwinext-cancel.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/chmod.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/fchmod.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/getrlimit.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/mmap.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/setrlimit.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/port_descriptions.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_bridge_remote_time.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_packet.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/clonefile.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_in_cksum.s
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/reboot.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_continuous_time.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unlinkat.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/clock_sleep.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/renameat.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/error_codes.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/exc_catcher_state_identity.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/exc_catcher_state.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/exc_catcher.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/fprintf_stderr.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/guarded_open_dprotected_np.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/host.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_error_string.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_error.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_init.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_absolute_time.s
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_msg.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_traps.s
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mig_allocate.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mig_deallocate.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/quota_obsolete.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/__commpage_gettimeofday.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mig_reply_setup.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mig_reply_port.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/restartable.defs
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mig_strncpy.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/ms_thread_switch.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/panic.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/port_obj.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/semaphore.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/slot_name.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/string.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/getiopolicy_np.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_legacy.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_right.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/proc.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/libproc/libproc.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_approximate_time.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/gethostuuid.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_boottime.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/utimensat.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/thread_act.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/memory_entry.defs
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/thread_register_state.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/renamex.c
	${CMAKE_CURRENT_SOURCE_DIR}/os/log_data.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/spawn/posix_spawn.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/stackshot.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/setpriority.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/fcntl.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_get_times.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/terminate_with_reason.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/fcntl-cancel.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/sfi.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/ioctl.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_channel.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/posix_sem_obsolete.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/kill.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/kill.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/pid_shutdown_networking.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/coalition.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/csr.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/fs_snapshot.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_timebase_info.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/kdebug_trace.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_approximate_time.s
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/select-cancel.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/sigsuspend.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_nexus.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/sigsuspend-cancel.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_voucher.defs
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unlink.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/rmdir.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/work_interval.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/rename.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_copy_in_cksum.s
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/remove-counter.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/__get_cpu_capabilities.s
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_copy_in_cksum_gen.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/init_cpu_capabilities.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_in_cksum_gen.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/open_dprotected_np.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/libproc/proc_listpidspath.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_channel_event.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/varargs_wrappers.s
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_port.c
	${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_vm.c
	${CMAKE_CURRENT_SOURCE_DIR}/os/alloc_once.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/persona.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/guarded_open_np.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/carbon_delete.c
)

add_library(libkernel SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/_libc_funcptr.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strlen.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/_libkernel_init.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/index.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strcmp.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/memset.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/memcpy.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strsep.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strcpy.c
	${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strlcpy.c
)

target_include_directories(libkernel_static PRIVATE ${common_headers})
target_include_directories(libkernel PRIVATE ${common_headers})

target_compile_definitions(libkernel_static PRIVATE ${common_defs} 
    VARIANT_CANCELABLE __DARWIN_UNIX03)
target_compile_definitions(libkernel PRIVATE ${common_defs}
    VARIANT_CANCELABLE __DARWIN_UNIX03)

target_compile_options(libkernel_legacy PRIVATE ${common_opts}
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)
target_compile_options(libkernel_static PRIVATE ${common_opts}
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)
target_compile_options(libkernel PRIVATE ${common_opts}
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

set_target_properties(libkernel_static PROPERTIES 
    OUTPUT_NAME "kernel_static" PREFIX "lib")

set_target_properties(libmach PROPERTIES OUTPUT_NAME "mach" PREFIX "lib")

set_target_properties(libkernel PROPERTIES 
    OUTPUT_NAME "system_kernel" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

target_link_libraries(libkernel_static libkernel_legacy)

add_dependencies(libkernel_static libkernel_legacy)
add_dependencies(libkernel xnu_headers compiler_rt libkernel_static libmach)

target_link_options(libkernel PRIVATE 
    -v -umbrella System -all_load -mmacosx-version-min=10.15
    -nostdlib -Wl,-install_name,/usr/lib/system/libsystem_kernel.dylib
)
target_link_libraries(libkernel PRIVATE libkernel_static
    libsyscalls ${CMAKE_CURRENT_BINARY_DIR}/libmach.a)

add_custom_command(TARGET libkernel POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libkernel> 
		${RAVYN_SDKROOT}/usr/lib/system/libsystem_kernel.dylib
)