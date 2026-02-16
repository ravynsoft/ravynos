file(GLOB mach_defs ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/mach/*.defs)
file(COPY ${mach_defs}
    DESTINATION ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/mach/)

set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall)

set(common_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../bsd
    ${CMAKE_CURRENT_SOURCE_DIR}/os
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/spawn
    ${ROOT_SOURCE_DIR}/Frameworks

    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/libproc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mach

    ${ROOT_BINARY_DIR}/${CpuArch}
    ${RAVYN_SDKROOT}/usr/local/include
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    ${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include
)

set(common_defs
    -D__DARWIN_VERS_1050 -DLIBSYSCALL_INTERFACE -DSYSCALL_PRE1050 -DNO_SYSCALL_LEGACY
    -DCF_OPEN_SOURCE -DCF_EXCLUDE_CSTD_HEADERS -DDEBUG -D_FORTIFY_SOURCE=0
    -DDARWIN_C_SOURCE=DARWIN_C_FULL
)

set(common_opts
    -isysroot${RAVYN_SDKROOT} -mmacosx-version-min=${MACOS_VERSION_MIN}
    -Wno-implicit-int -Wno-implicit-function-declaration -Wno-int-conversion
    -Wno-incompatible-sysroot -Wno-nullability-completeness -no-cpp-precomp -nostdlib
    -fdollars-in-identifiers -fno-common -fno-stack-protector -fno-stack-check
    -momit-leaf-frame-pointer -DPRIVATE -DTARGET_OS_BRIDGE=0
)

# Generate syscall stubs - 24614EF311E7C98600E78584 libsyscalls.a
add_custom_command(
    OUTPUT libsyscalls.a
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

set(libmach_inc
    -I${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include
    -I${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include/mach
    -I${CMAKE_CURRENT_BINARY_DIR}/${CpuArch}
    -I${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include/mach
    -I${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include
    -I${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include/servers
)
set(defs -Wno-incompatible-sysroot -DLIBSYSCALL_INTERFACE -DPRIVATE)
add_custom_command(
    OUTPUT libmach.a
    COMMAND
        mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/obj && env
        SDKROOT=${RAVYN_SDKROOT} PLATFORM_NAME=MacOSX ARCHS=${CpuArch}
        SRCROOT=${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall
        OBJROOT=${CMAKE_CURRENT_BINARY_DIR}/obj
        BUILT_PRODUCTS_DIR=${CMAKE_CURRENT_BINARY_DIR}
        DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
        "PATH=${TOOLS}:$ENV{PATH}" sh
        ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/mach_install_mig.sh $<SEMICOLON>
        set -e $<SEMICOLON>
        for f in ${CMAKE_CURRENT_BINARY_DIR}/obj/*User.c$<SEMICOLON>
        do
           ${CC} ${common_defs} ${common_opts}
                ${libmach_inc} -o $$f.o -c $$f $<SEMICOLON>
        done &&
        ${TOOLS}/llvm-ar rcs ${CMAKE_CURRENT_BINARY_DIR}/libmach.a
            ${CMAKE_CURRENT_BINARY_DIR}/obj/*.o
)

add_library(libkernel STATIC
    libmach.a
    libsyscalls.a

    # legacy syscalls
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
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/open.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/select-pre1050.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/select.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/legacy/getaudit.c

    # unix03 syscalls
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/chmod.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/fchmod.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/getrlimit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/mmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/munmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unix03/setrlimit.c

    # cancelable syscalls
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/pselect-darwinext-cancel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/pselect-darwinext.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/fcntl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/fcntl-cancel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/select.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/select-cancel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/sigsuspend.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/cancelable/sigsuspend-cancel.c

    # custom syscall wrappers
    ${CMAKE_CURRENT_SOURCE_DIR}/custom/errno.c

    # These Skywalk sources are all empty in the public xnu
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_packet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_in_cksum.s
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_channel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_nexus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/os_channel_event.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_copy_in_cksum.s
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_copy_in_cksum_gen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/skywalk/cpu_in_cksum_gen.c

    # mach traps
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/port_descriptions.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/clock_sleep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/error_codes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/exc_catcher_state_identity.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/exc_catcher_state.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/exc_catcher.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/fprintf_stderr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_error_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_msg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_traps.s
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mig_allocate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mig_deallocate.c
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
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_legacy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_right.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/thread_act.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mach/mach_vm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_approximate_time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_continuous_time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_absolute_time.s
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_timebase_info.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_bridge_remote_time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_get_times.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/mach_boottime.c

    ${CMAKE_CURRENT_SOURCE_DIR}/os/alloc_once.c
    ${CMAKE_CURRENT_SOURCE_DIR}/os/log_data.c

    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/libproc/libproc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/libproc/proc_listpidspath.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/proc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/spawn/posix_spawn.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/terminate_with_reason.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/kill.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/ioctl.c

    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/rename.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/clonefile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unlinkat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/renameat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/renamex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/unlink.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/rmdir.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/carbon_delete.c

    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/utimensat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/setpriority.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/guarded_open_dprotected_np.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/quota_obsolete.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/getiopolicy_np.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/gethostuuid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/thread_register_state.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/stackshot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/sfi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/posix_sem_obsolete.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/pid_shutdown_networking.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/coalition.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/fs_snapshot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/work_interval.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/remove-counter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/open_dprotected_np.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/varargs_wrappers.s
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/persona.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/guarded_open_np.c

    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/kdebug_trace.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/reboot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/csr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/init_cpu_capabilities.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/__commpage_gettimeofday.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/__get_cpu_capabilities.s

    # 249C61281194815000ED73F3 - sources of D2AAC0630554660B00DB518D Libsyscall_static -> libsystem_kernel.a
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/_libc_funcptr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/_libkernel_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strlen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/index.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strcmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/memset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/memcpy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strsep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strcpy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string/strlcpy.c
)
set_source_files_properties(wrappers/skywalk/os_packet.c PROPERTIES COMPILE_OPTIONS "-fno-builtin")
set_source_files_properties(wrappers/skywalk/os_channel.c PROPERTIES COMPILE_OPTIONS "-fno-builtin")
set_source_files_properties(wrappers/skywalk/os_nexus.c PROPERTIES COMPILE_OPTIONS "-fno-builtin")
set_source_files_properties(wrappers/skywalk/cpu_copy_in_cksum_gen.c PROPERTIES COMPILE_OPTIONS "-fno-builtin") # seems to be a version at Kernel/xnu/bsd/netinet/cpu_in_cksum_gen.c
set_source_files_properties(wrappers/skywalk/cpu_in_cksum_gen.c PROPERTIES COMPILE_OPTIONS "-fno-builtin")
set_source_files_properties(./Kernel/xnu/libsyscall/wrappers/skywalk/os_channel_event.c PROPERTIES COMPILE_OPTIONS "-fno-builtin")


set_target_properties(libkernel PROPERTIES  OUTPUT_NAME "kernel" PREFIX "lib")
add_dependencies(libkernel features xnu_headers compiler_rt)

target_include_directories(libkernel PRIVATE ${common_headers})
target_compile_definitions(libkernel PRIVATE ${common_defs})
target_compile_options(libkernel PRIVATE ${common_opts} $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

add_custom_command(TARGET libkernel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/local/lib/kernel
        && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libkernel> ${RAVYN_SDKROOT}/usr/local/lib/kernel/
)

add_library(libsystem_kernel SHARED stub.c)
set_target_properties(libsystem_kernel PROPERTIES
    OUTPUT_NAME "kernel" SUFFIX ".dylib" PREFIX "libsystem_" NO_SONAME true)
add_dependencies(libsystem_kernel libkernel)

target_link_options(libsystem_kernel PRIVATE
    -Wl,-umbrella,System -Wl,-all_load
    -mmacosx-version-min=${MACOS_VERSION_MIN} -L${RAVYN_SDKROOT}/usr/lib/system
    -Wl,-install_name,/usr/lib/system/libsystem_kernel.dylib
    -Wl,-current_version,6153.0.168 -Wl,-compatibility_version,1.0.0
    $<TARGET_FILE:libkernel>
    ${CMAKE_CURRENT_BINARY_DIR}/libsyscalls.a
    ${CMAKE_CURRENT_BINARY_DIR}/libmach.a
)

add_custom_command(TARGET libsystem_kernel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_kernel>
        ${RAVYN_SDKROOT}/usr/lib/system/libsystem_kernel.dylib
)
