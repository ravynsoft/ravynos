file(GLOB mach_defs ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/mach/*.defs)
file(COPY ${mach_defs} 
    DESTINATION ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/mach/)

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_C_USING_LINKER_APPLE ${LD})
set(CMAKE_CXX_USING_LINKER_APPLE ${LD})
set(CMAKE_LINK_DEPENDS_USE_LINKER FALSE)

add_library(libkernel SHARED
    MIG_HEADERS
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__kdebug_trace_string.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__gettimeofday.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__thread_selfusage.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__pipe.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__sigaltstack.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__getpid.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/dummy.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__lseek.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__vfork.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__ptrace.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/errno.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__sigreturn.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__thread_selfid.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/custom.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__fork.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/__syscall.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/ms_thread_switch.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/string.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mig_reply_setup.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_vm.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/port_descriptions.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_traps.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/exc_catcher_state_identity.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/thread_act.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/semaphore.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/exc_catcher.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/port_obj.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_msg.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_init.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/panic.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/clock_sleep.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_error.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/host.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_right.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mig_reply_port.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mig_strncpy.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mig_allocate.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/slot_name.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_error_string.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mig_deallocate.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_legacy.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/fprintf_stderr.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/exc_catcher_state.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/error_codes.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach_port.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/_libc_funcptr.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/renameat.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/rename.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/getiopolicy_np.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/select.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/fcntl-base.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/sigsuspend.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/pselect-darwinext-cancel.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/sigsuspend-cancel.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/fcntl.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/fcntl-cancel.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/pselect-darwinext.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/cancelable/select-cancel.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/persona.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/gethostuuid.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/utimensat.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/carbon_delete.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/init_cpu_capabilities.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/rmdir.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/thread_register_state.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/reboot.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/setpriority.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/csr.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_continuous_time.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn/posix_spawn.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/guarded_open_np.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/posix_sem_obsolete.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/renamex.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/ioctl.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/guarded_open_dprotected_np.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/sigsuspend-base.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/strcpy.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/strlen.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/strsep.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/strcmp.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/memcpy.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/index.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/memset.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/strlcpy.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/fs_snapshot.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/kdebug_trace.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/kill.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_timebase_info.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/coalition.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/clonefile.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc/proc_listpidspath.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc/libproc.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_get_times.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/__get_cpu_capabilities.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_absolute_time.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/varargs_wrappers.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/pid_shutdown_networking.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_boottime.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unix03/mmap.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unix03/setrlimit.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unix03/munmap.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unix03/fchmod.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unix03/chmod.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unix03/getrlimit.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/stackshot.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/__commpage_gettimeofday.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/_libkernel_init.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_bridge_remote_time.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/remove-counter.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/select.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/mprotect.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/sigsuspend.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/lchown.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/recvfrom.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/getattrlist.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/sendto.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/kill.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/connect.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/listen.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/socketpair.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/getaudit.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/accept.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/recvmsg.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/getpeername.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/bind.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/munmap.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/select-pre1050.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/getsockname.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/msync.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/sendmsg.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/setattrlist.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/legacy/open.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unlink.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_approximate_time.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/quota_obsolete.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/mach_approximate_time.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/terminate_with_reason.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/select-base.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/proc.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/unlinkat.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/work_interval.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/os_nexus.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/os_channel.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/cpu_copy_in_cksum.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/os_packet.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/cpu_in_cksum.s
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/cpu_copy_in_cksum_gen.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/os_channel_event.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/skywalk/cpu_in_cksum_gen.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/sfi.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/open_dprotected_np.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/os/alloc_once.c
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/os/log_data.c
)

target_sources(libkernel INTERFACE
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/custom/SYS.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/externs.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/errorlib.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/errorlib.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach_interface.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach_right.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach_error.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/sync.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach_init.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/port_descriptions.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/vm_task.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach_right_private.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/port_obj.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/thread_state.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/vm_page_size.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach_sync_ipc.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/abort.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/exc_catcher.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/servers/ls_defs.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/servers/netname_defs.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/servers/nm_defs.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/servers/key_defs.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/string.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/stack_logging_internal.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn/spawn.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn/spawn_private.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/string/strings.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/_errno.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/_libkernel_init.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc/libproc_internal.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc/libproc.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/gethostuuid.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/gethostuuid_private.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/os/thread_self_restrict.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/os/proc.h
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/os/tsd.h
)

add_dependencies(libkernel xnu_headers compiler_rt)

set_target_properties(libkernel PROPERTIES 
    OUTPUT_NAME "system_kernel" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

target_include_directories(libkernel PRIVATE
    os mach wrappers custom
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/darwin
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/locale
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/locale/FreeBSD
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/stdtime/FreeBSD
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/private
    ${ROOT_SOURCE_DIR}/Frameworks
    ${RAVYN_SDKROOT}/usr/include/pthread
    ${DEVEL}/Default.xctoolchain/llvm/libcxx/include
)

target_compile_definitions(libkernel PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03 LIBSYSCALL_INTERFACE
    __DARWIN_VERS_1050 NO_SYSCALL_LEGACY SYSCALL_PRE1050
    __DARWIN_BYTE_ORDER=__BYTE_ORDER__ OS_ALWAYS_INLINE=
    _DARWIN_C_SOURCE _DARWIN_C_LEVEL=_DARWIN_C_FULL
)

target_compile_options(libkernel PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    -mmacosx-version-min=10.15 -isystem ${RAVYN_SDKROOT}/usr/include
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks
    -Wno-implicit-function-declaration -Wno-int-conversion
    -fdollars-in-identifiers -no-cpp-precomp -fno-common 
    -fno-stack-protector -fno-stack-check -momit-leaf-frame-pointer
    -UKERNEL -D_FORTIFY_SOURCE=0 -DPRIVATE
)

target_link_options(libkernel PRIVATE -umbrella System -all_load)

# Install the mach defs before generating headers
add_custom_command(
    OUTPUT 
        MIG_HEADERS
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/obj
    COMMAND env
        SDKROOT=${RAVYN_SDKROOT}
        ARCHS=${CpuArch}
        SRCROOT=${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall
        OBJROOT=${CMAKE_CURRENT_BINARY_DIR}/obj
        BUILT_PRODUCTS_DIR=${CMAKE_CURRENT_BINARY_DIR}
        DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
        "PATH=${TOOLS}:$ENV{PATH}"
        sh ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/xcodescripts/mach_install_mig.sh
    USES_TERMINAL
)
set_source_files_properties(MIG_HEADERS PROPERTIES SYMBOLIC "true")
