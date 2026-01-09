add_library(libsystem_c OBJECT)
set_property(TARGET libsystem_c PROPERTY OUTPUT_NAME "system_c")
set_property(TARGET libsystem_c PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_c PROPERTY PREFIX "lib")

target_sources(libsystem_c PRIVATE
    compat-43/sigaltstk.c
    compat-43/sigcompat.c
    darwin/kvm.c
    darwin/atexit_receipt.c
    emulated/brk.c
    emulated/bsd_signal.c
    emulated/lchflags.c
    emulated/lchmod.c
    emulated/lutimes.c
    emulated/statvfs.c
    emulated/tcgetsid.c
    gen/backtrace.c
    gen/clock_gettime.c
    gen/confstr.c
    gen/crypt.c
    gen/devname.c
    gen/dirfd.c
    gen/disklabel.c
    gen/errlst.c
    gen/filesec.c
    gen/fts.c
    gen/get_compat.c
    gen/getloadavg.c
    gen/getttyent.c
    gen/getusershell.c
    gen/getvfsbyname.c
    gen/nanosleep.c
    gen/nftw.c
    gen/nlist.c
    gen/raise.c
    gen/setlogin.c
    gen/sigsetops.c
    gen/strtofflags.c
    gen/sync_volume_np.c
    gen/thread_stack_pcs.c
    gen/uname.c
    gen/utmpx-darwin.c
    gmon/gmon.c
    locale/frune.c
    locale/isctype.c
    locale/iswctype.c
    locale/mbrune.c
    locale/runedepreciated.c
    locale/setinvalidrune.c
    locale/xlocale.c
    os/assumes.c
    os/debug_private.c
    posix1e/acl.c
    posix1e/acl_entry.c
    posix1e/acl_file.c
    posix1e/acl_flag.c
    posix1e/acl_perm.c
    posix1e/acl_translate.c
    stdio/xprintf_all_in_one.c
    stdio/xprintf_comp.c
    stdio/xprintf_domain.c
    stdio/xprintf_exec.c
    stdtime/getdate.c
    stdtime/timezone_unix03.c
    string/bcopy.c
    string/stpcpy.c
    string/stpncpy.c
    string/strcat.c
    string/strncat.c
    sys/OSMemoryNotification.c
    sys/OSThermalNotification.c
    sys/_libc_fork_child.c
    sys/_libc_init.c
    sys/chmodx_np.c
    sys/crt_externs.c
    sys/fork.c
    sys/gettimeofday.c
    sys/openx_np.c
    sys/posix_spawn.c
    sys/settimeofday.c
    sys/sigaction.c
    sys/statx_np.c
    sys/umaskx_np.c
    util/fparseln.c
    util/login.c
    util/login_tty.c
    util/logout.c
    util/logwtmp.c
    util/mkpath_np.c
    util/opendev.c
    util/pty.c
    uuid/uuidsrc/clear.c
    uuid/uuidsrc/compare.c
    uuid/uuidsrc/copy.c
    uuid/uuidsrc/gen_uuid.c
    uuid/uuidsrc/isnull.c
    uuid/uuidsrc/pack.c
    uuid/uuidsrc/parse.c
    uuid/uuidsrc/unpack.c
    uuid/uuidsrc/unparse.c
)

# add_dependencies(libc xnu_headers)

target_include_directories(libsystem_c PRIVATE
    ../../../Kernel/xnu/bsd
    ../../../Kernel/xnu/osfmk
    ../private locale locale/FreeBSD stdtime/FreeBSD . gen stdio/FreeBSD
    ../../libdispatch/src/BlocksRuntime ../libsystem_info/membership.subproj
    ../libsystem_info/lookup.subproj
    ../../../Kernel/xnu/libsyscall/wrappers/spawn
    ../../dyld/include ./darwin ../../../Kernel/xnu/libsyscall/mach
    ../../../Kernel/xnu/libkern ${SDK_SOURCE_DIR}/usr/include/pthread
    ${SDK_SOURCE_DIR}/usr/include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
)

target_compile_definitions(libsystem_c PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03 
    "__LIBC_PTHREAD_KEY_XLOCALE=__LIBC_PTHREAD_KEY(0)"
    memory_order_seq_cst=__ATOMIC_SEQ_CST
    HAVE_STDINT_H _COMM_PAGE_ATM_DIAGNOSTIC_CONFIG=0x00007fffffe00048ULL
)

target_compile_options(libsystem_c PRIVATE
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    --target=${TARGET_TRIPLE}
    -iframework ${RAVYN_SDKROOT}/System/Library/Frameworks
    -isystem ${RAVYN_SDKROOT}/usr/include
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    -include${SDK_SOURCE_DIR}/usr/include/os/availability.h
)
