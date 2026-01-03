set(CMAKE_INSTALL_PREFIX "")

add_library(libc
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

add_dependencies(libc xnu_headers)
set_property(TARGET libc PROPERTY OUTPUT_NAME "system_c")

target_include_directories(libc PRIVATE
    ../libplatform/private
    ../libmalloc/include ../libmalloc/private
    ../libdispatch/src/BlocksRuntime
    ../dyld/include
    ../../Kernel/xnu/libsyscall/wrappers
    ../../Kernel/xnu/libsyscall/mach
    ${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
    include
    ../libpthread/private ../libpthread
    ../../Kernel/xnu/libsyscall/wrappers/spawn
    ../../Kernel/xnu/libsyscall
    ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
    locale locale/FreeBSD stdtime/FreeBSD gen stdio/FreeBSD
    ../../Developer/cctools/include
    ../../CoreServices/syslog/libsystem_asl.tproj/include
    ../../Developer/cctools/include/foreign
    ../Libinfo/membership.subproj ../Libinfo/lookup.subproj
    ../../Kernel/xnu/libkern
    ../libpthread/pthread ../Libnotify ../libdispatch
)

target_compile_definitions(libc PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03 
    memory_order_seq_cst=__ATOMIC_SEQ_CST
    "__LIBC_PTHREAD_KEY_XLOCALE=__LIBC_PTHREAD_KEY(0)"
    HAVE_STDINT_H
)

target_compile_options(libc PRIVATE
    --sysroot=${RAVYN_SDKROOT} -w
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
    -Wno-implicit-function-declaration
    -Wno-int-conversion
)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "QuothTheRaven_NeverBuild")
add_library(libdarwin
    libdarwin/bsd.c
    libdarwin/ctl.c
    libdarwin/dirstat.c
    libdarwin/dirstat_collection.c
    libdarwin/err.c
    libdarwin/exception.c
    libdarwin/init.c
    libdarwin/mach.c
    libdarwin/stdio.c
    libdarwin/stdlib.c
    libdarwin/string.c
    libdarwin/variant.c
)
add_dependencies(libdarwin libplatform libmalloc libc)
set_property(TARGET libdarwin PROPERTY OUTPUT_NAME "system_darwin")

target_include_directories(libdarwin PRIVATE
        ${ROOT_SOURCE_DIR}/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk
        ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/Headers
        ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders
	. include
	libdarwin/h
	../libmalloc/include ../libmalloc/private
	../libplatform/private
	${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk
	#${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
	${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/libkern
	${ROOT_SOURCE_DIR}/Developer/cctools/include
	#${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
)

target_compile_definitions(libdarwin PRIVATE
    TARGET_OS_BRIDGE=0 __DARWIN_UNIX03 
    __DARWIN_BYTE_ORDER=__BYTE_ORDER__
)

target_compile_options(libdarwin PRIVATE
    --sysroot=${RAVYN_SDKROOT} -w
    -mmacosx-version-min=${CMAKE_MACOSX_MIN_VERSION}
    -Wno-implicit-function-declaration
    -Wno-int-conversion
)
endif()
