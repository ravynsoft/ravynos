set(CMAKE_INSTALL_PREFIX "")

add_library(libc OBJECT
    compat-43/sigaltstk.c
    compat-43/sigcompat.c
    darwin/atexit_receipt.c
    darwin/kvm.c
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

ExternalProject_Add(compiler_rt
    SOURCE_DIR ${ROOT_SOURCE_DIR}/Developer/llvm-project/compiler-rt
    BINARY_DIR compiler_rt
    CMAKE_ARGS
        ${EXTRA_ARGS} -Wno-dev
        -DROOT_SOURCE_DIR=${ROOT_SOURCE_DIR}
        -DROOT_BINARY_DIR=${ROOT_BINARY_DIR}
        -DCMAKE_INSTALL_PREFIX=${ROOT_BINARY_DIR}/Developer/Toolchains/Default.xctoolchain/usr
        -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=${BuildArch}
        -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_ZLIB=ON
        -DLLVM_DIR=<BINARY_DIR>/../clang
        # -DCOMPILER_RT_BAREMETAL_BUILD=ON
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF
        -DCOMPILER_RT_BUILD_PROFILE=OFF -DCOMPILER_RT_BUILD_SANITIZERS=OFF
        -DCOMPILER_RT_BUILD_XRAY=OFF -DCOMPILER_RT_INCLUDE_TESTS=OFF
        -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
        -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${CpuArch}-corvux-ravynos
        -DCMAKE_SYSROOT=${RAVYN_SDKROOT}
        -DCMAKE_C_COMPILER_WORKS=ON -DCMAKE_CXX_COMPILER_WORKS=ON
    DEPENDS libSystem
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

# add_dependencies(libc )

# add_custom_command(TARGET libfirehose_kernel
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libfirehose_kernel>
#         ${CMAKE_BINARY_DIR}/sysroot/usr/local/lib/kernel
# )
