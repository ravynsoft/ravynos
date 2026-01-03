set(CMAKE_INSTALL_PREFIX "")

file(GLOB mach_defs Kernel/xnu/osfmk/mach/*.defs)
file(COPY ${mach_defs} 
    DESTINATION ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/mach/)

add_library(libkernel STATIC
    MIG_HEADERS
    custom/__kdebug_trace_string.s
    custom/__gettimeofday.s
    custom/__thread_selfusage.s
    custom/__pipe.s
    custom/__sigaltstack.s
    custom/__getpid.s
    custom/dummy.c
    custom/__lseek.s
    custom/__vfork.s
    custom/__ptrace.s
    custom/errno.c
    custom/__sigreturn.s
    custom/__thread_selfid.s
    custom/custom.s
    custom/__fork.s
    custom/__syscall.s
    mach/ms_thread_switch.c
    mach/string.c
    mach/mig_reply_setup.c
    mach/mach_vm.c
    mach/port_descriptions.c
    mach/mach_traps.s
    mach/exc_catcher_state_identity.c
    mach/thread_act.c
    mach/semaphore.c
    mach/exc_catcher.c
    mach/port_obj.c
    mach/mach_msg.c
    mach/mach_init.c
    mach/panic.c
    mach/clock_sleep.c
    mach/mach_error.c
    mach/host.c
    mach/mach_right.c
    mach/mig_reply_port.c
    mach/mig_strncpy.c
    mach/mig_allocate.c
    mach/slot_name.c
    mach/mach_error_string.c
    mach/mig_deallocate.c
    mach/mach_legacy.c
    mach/fprintf_stderr.c
    mach/exc_catcher_state.c
    mach/error_codes.c
    mach/mach_port.c
    wrappers/_libc_funcptr.c
    wrappers/renameat.c
    wrappers/rename.c
    wrappers/getiopolicy_np.c
    wrappers/cancelable/select.c
    wrappers/cancelable/fcntl-base.c
    wrappers/cancelable/sigsuspend.c
    wrappers/cancelable/pselect-darwinext-cancel.c
    wrappers/cancelable/sigsuspend-cancel.c
    wrappers/cancelable/fcntl.c
    wrappers/cancelable/fcntl-cancel.c
    wrappers/cancelable/pselect-darwinext.c
    wrappers/cancelable/select-cancel.c
    wrappers/persona.c
    wrappers/gethostuuid.c
    wrappers/utimensat.c
    wrappers/carbon_delete.c
    wrappers/init_cpu_capabilities.c
    wrappers/rmdir.c
    wrappers/thread_register_state.c
    wrappers/reboot.c
    wrappers/setpriority.c
    wrappers/csr.c
    wrappers/mach_continuous_time.c
    wrappers/spawn/posix_spawn.c
    wrappers/guarded_open_np.c
    wrappers/posix_sem_obsolete.c
    wrappers/renamex.c
    wrappers/ioctl.c
    wrappers/guarded_open_dprotected_np.c
    wrappers/sigsuspend-base.c
    wrappers/string/strcpy.c
    wrappers/string/strlen.c
    wrappers/string/strsep.c
    wrappers/string/strcmp.c
    wrappers/string/memcpy.c
    wrappers/string/index.c
    wrappers/string/memset.c
    wrappers/string/strlcpy.c
    wrappers/fs_snapshot.c
    wrappers/kdebug_trace.c
    wrappers/kill.c
    wrappers/mach_timebase_info.c
    wrappers/coalition.c
    wrappers/clonefile.c
    wrappers/libproc/proc_listpidspath.c
    wrappers/libproc/libproc.c
    wrappers/mach_get_times.c
    wrappers/__get_cpu_capabilities.s
    wrappers/mach_absolute_time.s
    wrappers/varargs_wrappers.s
    wrappers/pid_shutdown_networking.c
    wrappers/mach_boottime.c
    wrappers/unix03/mmap.c
    wrappers/unix03/setrlimit.c
    wrappers/unix03/munmap.c
    wrappers/unix03/fchmod.c
    wrappers/unix03/chmod.c
    wrappers/unix03/getrlimit.c
    wrappers/stackshot.c
    wrappers/__commpage_gettimeofday.c
    wrappers/_libkernel_init.c
    wrappers/mach_bridge_remote_time.c
    wrappers/remove-counter.c
    wrappers/legacy/select.c
    wrappers/legacy/mprotect.c
    wrappers/legacy/sigsuspend.c
    wrappers/legacy/lchown.c
    wrappers/legacy/recvfrom.c
    wrappers/legacy/getattrlist.c
    wrappers/legacy/sendto.c
    wrappers/legacy/kill.c
    wrappers/legacy/connect.c
    wrappers/legacy/listen.c
    wrappers/legacy/socketpair.c
    wrappers/legacy/getaudit.c
    wrappers/legacy/accept.c
    wrappers/legacy/recvmsg.c
    wrappers/legacy/getpeername.c
    wrappers/legacy/bind.c
    wrappers/legacy/munmap.c
    wrappers/legacy/select-pre1050.c
    wrappers/legacy/getsockname.c
    wrappers/legacy/msync.c
    wrappers/legacy/sendmsg.c
    wrappers/legacy/setattrlist.c
    wrappers/legacy/open.c
    wrappers/unlink.c
    wrappers/mach_approximate_time.s
    wrappers/quota_obsolete.c
    wrappers/mach_approximate_time.c
    wrappers/terminate_with_reason.c
    wrappers/select-base.c
    wrappers/proc.c
    wrappers/unlinkat.c
    wrappers/work_interval.c
    wrappers/skywalk/os_nexus.c
    wrappers/skywalk/os_channel.c
    wrappers/skywalk/cpu_copy_in_cksum.s
    wrappers/skywalk/os_packet.c
    wrappers/skywalk/cpu_in_cksum.s
    wrappers/skywalk/cpu_copy_in_cksum_gen.c
    wrappers/skywalk/os_channel_event.c
    wrappers/skywalk/cpu_in_cksum_gen.c
    wrappers/sfi.c
    wrappers/open_dprotected_np.c
    os/alloc_once.c
    os/log_data.c
)

target_sources(libkernel INTERFACE
    custom/SYS.h
    mach/externs.h
    mach/errorlib.h
    mach/mach/errorlib.h
    mach/mach/mach_interface.h
    mach/mach/mach_right.h
    mach/mach/mach_error.h
    mach/mach/sync.h
    mach/mach/mach_init.h
    mach/mach/port_descriptions.h
    mach/mach/vm_task.h
    mach/mach/mach_right_private.h
    mach/mach/port_obj.h
    mach/mach/thread_state.h
    mach/mach/vm_page_size.h
    mach/mach/mach.h
    mach/mach/mach_sync_ipc.h
    mach/abort.h
    mach/exc_catcher.h
    mach/servers/ls_defs.h
    mach/servers/netname_defs.h
    mach/servers/nm_defs.h
    mach/servers/key_defs.h
    mach/string.h
    mach/stack_logging_internal.h
    wrappers/spawn/spawn.h
    wrappers/spawn/spawn_private.h
    wrappers/string/strings.h
    wrappers/_errno.h
    wrappers/_libkernel_init.h
    wrappers/libproc/libproc_internal.h
    wrappers/libproc/libproc.h
    wrappers/gethostuuid.h
    wrappers/gethostuuid_private.h
    os/thread_self_restrict.h
    os/proc.h
    os/tsd.h
)

add_dependencies(libkernel xnu_headers libc)
set_property(TARGET libkernel PROPERTY OUTPUT_NAME "system_kernel")

target_include_directories(libkernel PRIVATE
    custom mach os wrappers wrappers/string wrappers/libproc
    wrappers/libproc/spawn
    ${CMAKE_CURRENT_BINARY_DIR}/internal_hdr/include
    ${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/local/include
    ${CMAKE_CURRENT_BINARY_DIR}/mig_hdr/include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
    #${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
)

target_compile_definitions(libkernel PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03 LIBSYSCALL_INTERFACE
    __DARWIN_VERS_1050 NO_SYSCALL_LEGACY SYSCALL_PRE1050
)

target_compile_options(libkernel PRIVATE
    --sysroot=${RAVYN_SDKROOT} -w
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
    -Wno-implicit-function-declaration -Wno-int-conversion
    -fdollars-in-identifiers -no-cpp-precomp -fno-common 
    -fno-stack-protector -fno-stack-check -momit-leaf-frame-pointer
    -UKERNEL -DPRIVATE -F${RAVYN_SDKROOT}/System/Library/Frameworks
    -isystem=${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    -isystem=${RAVYN_SDKROOT}/usr/local/include
    -isystem=${RAVYN_SDKROOT}/usr/include
)

target_link_options(libkernel PRIVATE -umbrella System -all_load)

# Install the mach defs before generating headers
add_custom_command(
    OUTPUT 
        MIG_HEADERS
    COMMAND env SDKROOT=macosx SRCROOT=${CMAKE_CURRENT_SOURCE_DIR}
        ARCHS=${CpuArch} OBJROOT=${CMAKE_CURRENT_BINARY_DIR}/obj 
        BUILT_PRODUCTS_DIR=${CMAKE_CURRENT_BINARY_DIR}
        "PATH=${TOOLS}:$ENV{PATH}" DEVELOPER_DIR=${ROOT_BINARY_DIR}/Developer
        sh ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/mach_install_mig.sh
    USES_TERMINAL
)
set_source_files_properties(MIG_HEADERS PROPERTIES SYMBOLIC "true")
