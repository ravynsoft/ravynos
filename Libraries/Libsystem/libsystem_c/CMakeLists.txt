#-------------------------------------------------------------------------
#                         RAVYNOS LIBSYSTEM_C BUILD
#
# The C library (libsystem_c or libc - part of libSystem) has a very
# complex build process. The same sources are built multiple times with
# varying compiler options to create several static archives and several
# Variants. These are then linked together to create the final library,
# which is itself later linked into libSystem. This make script has been
# puzzled out from the Xcode project files.
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# Settings common to all targets
#-------------------------------------------------------------------------

add_compile_definitions(
    __LIBC__ _FORTIFY_SOURCE=0
    _COMM_PAGE_ATM_DIAGNOSTIC_CONFIG=0x00007fffffe00048ULL
	PR_13085474_CHECK=0 # Remove this once dyld is built!
)
add_compile_options(
    --target=${TARGET_TRIPLE} -std=gnu11
    --sysroot=${RAVYN_SDKROOT} -isysroot=${RAVYN_SDKROOT} -w
    -iframework${RAVYN_SDKROOT}/System/Library/Frameworks
    -isystem${RAVYN_SDKROOT}/usr/include/c++/v1
    -isystem${ROOT_BINARY_DIR}/${CpuArch}/System.framework/PrivateHeaders
	-isystem${ROOT_BINARY_DIR}/${CpuArch}
	-isystem${ROOT_BINARY_DIR}/Kernel/xnu/bsd
    -isystem${RAVYN_SDKROOT}/usr/local/include
    -isystem${RAVYN_SDKROOT}/usr/include
    -Wno-implicit-function-declaration
    -Wno-int-conversion -Wno-implicit-int
    -Wno-error=shorten-64-to-32
    -Wno-error=incompatible-pointer-types-discards-qualifiers
    -Wno-nullability-completeness
    -fdollars-in-identifiers -fno-common -fverbose-asm
)
include_directories(gen locale locale/FreeBSD stdtime/FreeBSD darwin)


add_custom_target(features
	COMMAND env "ARCHS=${CpuArch} "	VARIANT_PLATFORM_NAME=macosx
		DERIVED_FILES_DIR=${ROOT_BINARY_DIR}
		SRCROOT=${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c
		perl ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/xcodescripts/generate_features.pl
	COMMAND perl
		${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/xcodescripts/patch_headers_variants.pl
		${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
		${ROOT_BINARY_DIR}/${CpuArch}/System.framework/Versions/B
	BYPRODUCTS ${ROOT_BINARY_DIR}/${CpuArch}/libc-features.h
)


#-------------------------------------------------------------------------
#                           ARCHIVE TARGETS
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# libBase.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(Base STATIC
	emulated/lchflags.c
	emulated/lchmod.c
	emulated/lutimes.c
	emulated/statvfs.c
	gen/backtrace.c
	gen/confstr.c
	gen/crypt.c
	gen/devname.c
	gen/disklabel.c
	gen/errlst.c
	gen/filesec.c
	gen/fts.c
	gen/get_compat.c
	gen/getloadavg.c
	gen/getttyent.c
	gen/getusershell.c
	gen/getvfsbyname.c
	darwin/atexit_receipt.c
	gen/nanosleep.c
	gen/nftw.c
	gen/setlogin.c
	gen/sigsetops.c
	gen/strtofflags.c
	gen/thread_stack_pcs.c
	gen/uname.c
	gen/utmpx-darwin.c
	os/debug_private.c
	gmon/gmon.c
	locale/isctype.c
	string/bcopy.c
	locale/iswctype.c
	locale/xlocale.c
	posix1e/acl.c
	posix1e/acl_entry.c
	posix1e/acl_file.c
	posix1e/acl_flag.c
	posix1e/acl_perm.c
	posix1e/acl_translate.c
	stdtime/getdate.c
	stdtime/timezone_unix03.c
	stdio/xprintf_all_in_one.c
	stdio/xprintf_comp.c
	stdio/xprintf_domain.c
	stdio/xprintf_exec.c
	util/fparseln.c
	util/login_tty.c
	util/logwtmp.c
	util/mkpath_np.c
	gen/raise.c
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
	emulated/tcgetsid.c
	sys/_libc_init.c
	sys/_libc_fork_child.c
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
	locale/frune.c
	locale/mbrune.c
	locale/runedepreciated.c
	locale/setinvalidrune.c
	util/login.c
	util/logout.c
	gen/nlist.c
	sys/OSThermalNotification.c
	string/stpcpy.c
	string/stpncpy.c
	string/strcat.c
	string/strncat.c
	gen/sync_volume_np.c
	gen/dirfd.c
	os/assumes.c
)
set_property(TARGET Base PROPERTY OUTPUT_NAME _Base)
add_dependencies(Base features)
target_compile_options(Base PRIVATE
    "-include${RAVYN_SDKROOT}/usr/include/os/availability.h"
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/_fbsd_compat_.h"
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h"
    -DPRIVATE
    -Wimplicit-fallthrough -Wno-unknown-pragmas -Wno-assume
    -Wformat-nonliteral -Wno-vla-extension -Wundef-prefix=SUPPORT_
)
target_compile_definitions(Base PRIVATE
    VARIANT_STATIC VARIANT_CANCELABLE VARIANT_DARWINEXTSN
    __DARWIN_NON_CANCELABLE=0 __DARWIN_UNIX03 __DARWIN_64_BIT_INO_T
	__DARWIN_VERS_1050 HAVE_STDINT_H
)
target_include_directories(Base PRIVATE
    include ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
	
	${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach fbsdcompat gdtoa
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ../private ../libsystem_info/membership.subproj
    ../libsystem_info/lookup.subproj ../libsystem_notify
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/private
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${SDK_SOURCE_DIR}/usr/include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
)
set_source_files_properties(./string/bcopy.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-momit-leaf-frame-pointer")
set_source_files_properties(./stdio/xprintf_all_in_one.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio/FreeBSD")
set_source_files_properties(./stdio/xprintf_comp.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio/FreeBSD")
set_source_files_properties(./stdio/xprintf_domain.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio/FreeBSD")
set_source_files_properties(./stdio/xprintf_exec.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio/FreeBSD")
set_source_files_properties(./util/fparseln.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-D__FBSDID=__RCSID")
set_source_files_properties(./uuid/uuidsrc/clear.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/compare.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/copy.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/gen_uuid.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/isnull.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/pack.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/parse.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/unpack.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./uuid/uuidsrc/unparse.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/uuid/uuid-config.h")
set_source_files_properties(./locale/frune.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-D__FBSDID=__RCSID")
set_source_files_properties(./locale/mbrune.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-D__FBSDID=__RCSID")
set_source_files_properties(./locale/runedepreciated.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-D__FBSDID=__RCSID")
set_source_files_properties(./locale/setinvalidrune.c TARGET_DIRECTORY Base PROPERTIES COMPILE_OPTIONS "-D__FBSDID=__RCSID")

#-------------------------------------------------------------------------
# libc_dyld.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(libc_dyld STATIC
    gen/FreeBSD/arc4random.c
    gen/FreeBSD/closedir.c
    gen/dirfd.c
    gen/FreeBSD/getcwd.c
    gen/FreeBSD/getpagesize.c
    gen/nanosleep.c
    gen/FreeBSD/opendir.c
    gen/FreeBSD/readdir.c
    gen/FreeBSD/scandir.c
    gen/FreeBSD/sysctl.c
    gen/FreeBSD/sysctlbyname.c
    gen/FreeBSD/telldir.c
    gen/FreeBSD/usleep.c
    stdlib/FreeBSD/atexit.c
    stdlib/FreeBSD/exit.c
    stdlib/FreeBSD/heapsort.c
    stdlib/FreeBSD/merge.c
    stdlib/FreeBSD/qsort.c
    stdlib/FreeBSD/reallocf.c
    stdlib/FreeBSD/realpath.c
    string/bcopy.c
    string/strcat.c
    string/FreeBSD/strdup.c
    string/FreeBSD/strrchr.c
    sys/gettimeofday.c
    sys/_libc_init.c
)
set_property(TARGET libc_dyld PROPERTY OUTPUT_NAME c_dyld)
add_dependencies(libc_dyld features)
target_compile_options(libc_dyld PRIVATE
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/_fbsd_compat_.h"
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h"
    -DPRIVATE
)
target_compile_definitions(libc_dyld PRIVATE
    VARIANT_STATIC VARIANT_CANCELABLE VARIANT_DARWINEXTSN __isthreaded
	__DARWIN_UNIX03 __DARWIN_64_BIT_INO_T __DARWIN_VERS_1050
	__DARWIN_NON_CANCELABLE=0
)
target_include_directories(libc_dyld PRIVATE
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach fbsdcompat gdtoa
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ../private ../libsystem_info/membership.subproj
    ../libsystem_info/lookup.subproj ../libsystem_blocks
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)
set_source_files_properties(./gen/FreeBSD/arc4random.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-include${RAVYN_SDKROOT}/usr/include/os/availability.h;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/_fbsd_compat_.h")
set_source_files_properties(./gen/FreeBSD/closedir.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CLOSEDIR;-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./string/bcopy.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-momit-leaf-frame-pointer")
set_source_files_properties(./gen/FreeBSD/opendir.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS___OPENDIR2;-DLIBC_ALIAS_OPENDIR;-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/readdir.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/scandir.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/telldir.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS__SEEKDIR;-DLIBC_ALIAS_TELLDIR;-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/usleep.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_USLEEP")
set_source_files_properties(./gen/nanosleep.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_NANOSLEEP")
set_source_files_properties(./stdlib/FreeBSD/realpath.c TARGET_DIRECTORY libc_dyld PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REALPATH")

#-------------------------------------------------------------------------
# libc_static.a (libc.a)
#-------------------------------------------------------------------------

add_library(libc_static STATIC
	./secure/chk_fail.c
	./secure/memcpy_chk.c
	./secure/memmove_chk.c
	./secure/memset_chk.c
	./secure/snprintf_chk.c
	./secure/sprintf_chk.c
	./secure/stpcpy_chk.c
	./secure/stpncpy_chk.c
	./secure/strcat_chk.c
	./secure/strcpy_chk.c
	./secure/strlcat_chk.c
	./secure/strlcpy_chk.c
	./secure/strncat_chk.c
	./secure/strncpy_chk.c
	./secure/vsnprintf_chk.c
	./secure/vsprintf_chk.c
	./secure/memccpy_chk.c
    ./x86_64/gen/mcount.s
)
set_property(TARGET libc_static PROPERTY OUTPUT_NAME c_static)
add_dependencies(libc_static features)
target_compile_definitions(libc_static PRIVATE
    VARIANT_STATIC VARIANT_CANCELABLE VARIANT_DARWINEXTSN
	__DARWIN_UNIX03 __DARWIN_64_BIT_INO_T __DARWIN_VERS_1050 __isthreaded
	__DARWIN_NON_CANCELABLE=0
)
target_include_directories(libc_static PRIVATE
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach fbsdcompat gdtoa
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ../private ../libsystem_info/membership.subproj
    ../libsystem_info/lookup.subproj
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)

#-------------------------------------------------------------------------
# libPlatform.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(Platform STATIC darwin/forceLibcToBuild.c darwin/kvm.c)
add_dependencies(Platform features)
if("${CpuArch}" STREQUAL "x86_64")
    target_sources(Platform PRIVATE x86_64/gen/mcount.s)
endif()
target_compile_definitions(Platform PRIVATE
	__DARWIN_UNIX03 __DARWIN_64_BIT_INO_T __DARWIN_NON_CANCELABLE
	__DARWIN_VERS_1050
)

#-------------------------------------------------------------------------
# libFreeBSD.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(FreeBSD STATIC
	compat-43/FreeBSD/creat.c
	compat-43/FreeBSD/gethostid.c
	compat-43/FreeBSD/getwd.c
	compat-43/FreeBSD/killpg.c
	compat-43/FreeBSD/sethostid.c
	compat-43/FreeBSD/setpgrp.c
	compat-43/FreeBSD/setrgid.c
	compat-43/FreeBSD/setruid.c
	db/btree/FreeBSD/bt_close.c
	db/btree/FreeBSD/bt_conv.c
	db/btree/FreeBSD/bt_debug.c
	db/btree/FreeBSD/bt_delete.c
	db/btree/FreeBSD/bt_get.c
	db/btree/FreeBSD/bt_open.c
	db/btree/FreeBSD/bt_overflow.c
	db/btree/FreeBSD/bt_page.c
	db/btree/FreeBSD/bt_put.c
	db/btree/FreeBSD/bt_search.c
	db/btree/FreeBSD/bt_seq.c
	db/btree/FreeBSD/bt_split.c
	db/btree/FreeBSD/bt_utils.c
	db/db/FreeBSD/db.c
	db/hash/FreeBSD/hash.c
	db/hash/FreeBSD/hash_bigkey.c
	db/hash/FreeBSD/hash_buf.c
	db/hash/FreeBSD/hash_func.c
	db/hash/FreeBSD/hash_log2.c
	db/hash/FreeBSD/hash_page.c
	db/hash/FreeBSD/ndbm.c
	db/mpool/FreeBSD/mpool.c
	db/recno/FreeBSD/rec_close.c
	db/recno/FreeBSD/rec_delete.c
	db/recno/FreeBSD/rec_get.c
	db/recno/FreeBSD/rec_open.c
	db/recno/FreeBSD/rec_put.c
	db/recno/FreeBSD/rec_search.c
	db/recno/FreeBSD/rec_seq.c
	db/recno/FreeBSD/rec_utils.c
	string/FreeBSD/timingsafe_bcmp.c
	gdtoa/FreeBSD/_hdtoa.c
	gdtoa/FreeBSD/gdtoa-dmisc.c
	gdtoa/FreeBSD/gdtoa-dtoa.c
	gdtoa/FreeBSD/gdtoa-gdtoa.c
	gdtoa/FreeBSD/gdtoa-gethex.c
	gdtoa/FreeBSD/gdtoa-gmisc.c
	gdtoa/FreeBSD/gdtoa-hd_init.c
	gdtoa/FreeBSD/gdtoa-hexnan.c
	gdtoa/FreeBSD/gdtoa-misc.c
	gdtoa/FreeBSD/gdtoa-smisc.c
	gdtoa/FreeBSD/gdtoa-strtod.c
	gdtoa/FreeBSD/gdtoa-strtodg.c
	gdtoa/FreeBSD/gdtoa-strtof.c
	gdtoa/FreeBSD/gdtoa-strtopdd.c
	gdtoa/FreeBSD/gdtoa-strtord.c
	gdtoa/FreeBSD/gdtoa-sum.c
	gdtoa/FreeBSD/gdtoa-ulp.c
	gdtoa/FreeBSD/glue.c
	gen/FreeBSD/_rand48.c
	gen/FreeBSD/alarm.c
	gen/FreeBSD/arc4random.c
	gen/FreeBSD/assert.c
	gen/FreeBSD/basename.c
	gen/FreeBSD/clock.c
	gen/FreeBSD/closedir.c
	gen/FreeBSD/ctermid.c
	gen/FreeBSD/daemon.c
	gen/FreeBSD/dirname.c
	gen/FreeBSD/drand48.c
	gen/FreeBSD/erand48.c
	gen/FreeBSD/err.c
	gen/FreeBSD/exec.c
	gen/FreeBSD/fmtcheck.c
	gen/FreeBSD/fmtmsg.c
	gen/FreeBSD/fnmatch.c
	gen/FreeBSD/ftok.c
	gen/FreeBSD/getbsize.c
	gen/FreeBSD/getcap.c
	gen/FreeBSD/getcwd.c
	gen/FreeBSD/gethostname.c
	gen/FreeBSD/getlogin.c
	gen/FreeBSD/getmntinfo.c
	gen/FreeBSD/getpagesize.c
	gen/FreeBSD/getpeereid.c
	gen/FreeBSD/getprogname.c
	gen/FreeBSD/glob.c
	gen/FreeBSD/isatty.c
	gen/FreeBSD/jrand48.c
	gen/FreeBSD/lcong48.c
	gen/FreeBSD/lockf.c
	gen/FreeBSD/lrand48.c
	gen/FreeBSD/mrand48.c
	gen/FreeBSD/nice.c
	gen/FreeBSD/nrand48.c
	gen/FreeBSD/opendir.c
	gen/FreeBSD/pause.c
	gen/FreeBSD/popen.c
	gen/FreeBSD/psignal.c
	gen/FreeBSD/readdir.c
	gen/FreeBSD/readpassphrase.c
	gen/FreeBSD/rewinddir.c
	gen/FreeBSD/scandir.c
	gen/FreeBSD/seed48.c
	gen/FreeBSD/seekdir.c
	gen/FreeBSD/sethostname.c
	gen/FreeBSD/setmode.c
	gen/FreeBSD/setprogname.c
	gen/FreeBSD/siginterrupt.c
	gen/FreeBSD/siglist.c
	gen/FreeBSD/signal.c
	gen/FreeBSD/sleep.c
	gen/FreeBSD/srand48.c
	gen/FreeBSD/stringlist.c
	gen/FreeBSD/sysconf.c
	gen/FreeBSD/sysctl.c
	gen/FreeBSD/sysctlbyname.c
	gen/FreeBSD/sysctlnametomib.c
	gen/FreeBSD/telldir.c
	gen/FreeBSD/termios.c
	gen/FreeBSD/time.c
	gen/FreeBSD/times.c
	gen/FreeBSD/ttyname.c
	gen/FreeBSD/ttyslot.c
	gen/FreeBSD/ualarm.c
	gen/FreeBSD/ulimit.c
	gen/FreeBSD/unvis.c
	gen/FreeBSD/usleep.c
	gen/FreeBSD/utime.c
	gen/FreeBSD/vis.c
	gen/FreeBSD/wait.c
	gen/FreeBSD/wait3.c
	gen/FreeBSD/waitpid.c
	locale/FreeBSD/ascii.c
	locale/FreeBSD/big5.c
	locale/FreeBSD/btowc.c
	locale/FreeBSD/collate.c
	locale/FreeBSD/collcmp.c
	locale/FreeBSD/euc.c
	locale/FreeBSD/fix_grouping.c
	locale/FreeBSD/gb18030.c
	locale/FreeBSD/gb2312.c
	locale/FreeBSD/gbk.c
	locale/FreeBSD/ldpart.c
	locale/FreeBSD/lmessages.c
	locale/FreeBSD/lmonetary.c
	locale/FreeBSD/lnumeric.c
	locale/FreeBSD/localeconv.c
	locale/FreeBSD/mblen.c
	locale/FreeBSD/mbrlen.c
	locale/FreeBSD/mbrtowc.c
	locale/FreeBSD/mbsinit.c
	locale/FreeBSD/mbsnrtowcs.c
	locale/FreeBSD/mbsrtowcs.c
	locale/FreeBSD/mbstowcs.c
	locale/FreeBSD/mbtowc.c
	locale/FreeBSD/mskanji.c
	locale/FreeBSD/nextwctype.c
	locale/FreeBSD/nl_langinfo.c
	locale/FreeBSD/nomacros.c
	locale/FreeBSD/none.c
	locale/FreeBSD/runetype.c
	locale/FreeBSD/setlocale.c
	locale/FreeBSD/setrunelocale.c
	locale/FreeBSD/table.c
	locale/FreeBSD/tolower.c
	locale/FreeBSD/toupper.c
	locale/FreeBSD/utf8.c
	locale/FreeBSD/wcrtomb.c
	locale/FreeBSD/wcsftime.c
	locale/FreeBSD/wcsnrtombs.c
	locale/FreeBSD/wcsrtombs.c
	locale/FreeBSD/wcstod.c
	locale/FreeBSD/wcstof.c
	locale/FreeBSD/wcstoimax.c
	locale/FreeBSD/wcstol.c
	locale/FreeBSD/wcstold.c
	locale/FreeBSD/wcstoll.c
	locale/FreeBSD/wcstombs.c
	locale/FreeBSD/wcstoul.c
	locale/FreeBSD/wcstoull.c
	locale/FreeBSD/wcstoumax.c
	locale/FreeBSD/wctob.c
	locale/FreeBSD/wctomb.c
	locale/FreeBSD/wctrans.c
	locale/FreeBSD/wctype.c
	locale/FreeBSD/wcwidth.c
	net/FreeBSD/addr2ascii.c
	net/FreeBSD/ascii2addr.c
	net/FreeBSD/inet_addr.c
	stdio/FreeBSD/open_memstream.c
	net/FreeBSD/inet_lnaof.c
	net/FreeBSD/inet_makeaddr.c
	net/FreeBSD/inet_net_ntop.c
	net/FreeBSD/inet_net_pton.c
	net/FreeBSD/inet_neta.c
	net/FreeBSD/inet_netof.c
	net/FreeBSD/inet_network.c
	net/FreeBSD/inet_ntoa.c
	net/FreeBSD/linkaddr.c
	net/FreeBSD/nsap_addr.c
	net/FreeBSD/recv.c
	net/FreeBSD/send.c
	net/FreeBSD/sockatmark.c
	net/FreeBSD/sourcefilter.c
	nls/FreeBSD/msgcat.c
	stdio/FreeBSD/_flock_stub.c
	stdio/FreeBSD/asprintf.c
	stdio/FreeBSD/clrerr.c
	stdio/FreeBSD/dprintf.c
	stdio/FreeBSD/fclose.c
	stdio/FreeBSD/fdopen.c
	stdio/FreeBSD/feof.c
	stdio/FreeBSD/ferror.c
	stdio/FreeBSD/fflush.c
	stdio/FreeBSD/fgetc.c
	stdio/FreeBSD/fgetln.c
	stdio/FreeBSD/fgetpos.c
	stdio/FreeBSD/fgets.c
	stdio/FreeBSD/fgetwc.c
	stdio/FreeBSD/fgetwln.c
	stdio/FreeBSD/fgetws.c
	stdio/FreeBSD/fileno.c
	stdio/FreeBSD/findfp.c
	stdio/FreeBSD/flags.c
	stdio/FreeBSD/fopen.c
	stdio/FreeBSD/fprintf.c
	stdio/FreeBSD/fpurge.c
	stdio/FreeBSD/fputc.c
	stdio/FreeBSD/fputs.c
	stdio/FreeBSD/fputwc.c
	stdio/FreeBSD/fputws.c
	stdio/FreeBSD/fread.c
	stdio/FreeBSD/freopen.c
	stdio/FreeBSD/fscanf.c
	stdio/FreeBSD/fseek.c
	stdio/FreeBSD/fsetpos.c
	stdio/FreeBSD/ftell.c
	stdio/FreeBSD/funopen.c
	stdio/FreeBSD/fvwrite.c
	stdio/FreeBSD/fwalk.c
	stdio/FreeBSD/fwide.c
	stdio/FreeBSD/fwprintf.c
	stdio/FreeBSD/fwrite.c
	stdio/FreeBSD/fwscanf.c
	stdio/FreeBSD/getc.c
	stdio/FreeBSD/getchar.c
	stdio/FreeBSD/getdelim.c
	stdio/FreeBSD/getline.c
	stdio/FreeBSD/gets.c
	stdio/FreeBSD/getw.c
	stdio/FreeBSD/getwc.c
	stdio/FreeBSD/getwchar.c
	stdio/FreeBSD/makebuf.c
	stdio/FreeBSD/mktemp.c
	stdio/FreeBSD/perror.c
	string/FreeBSD/rpmatch.c
	stdio/FreeBSD/printf-pos.c
	stdio/FreeBSD/printf.c
	stdio/FreeBSD/putc.c
	stdio/FreeBSD/putchar.c
	stdio/FreeBSD/puts.c
	stdio/FreeBSD/putw.c
	stdio/FreeBSD/putwc.c
	stdio/FreeBSD/putwchar.c
	stdio/FreeBSD/refill.c
	stdio/FreeBSD/remove.c
	stdio/FreeBSD/rewind.c
	stdio/FreeBSD/rget.c
	stdio/FreeBSD/scanf.c
	stdio/FreeBSD/setbuf.c
	stdio/FreeBSD/setbuffer.c
	stdio/FreeBSD/setvbuf.c
	stdio/FreeBSD/snprintf.c
	stdio/FreeBSD/sprintf.c
	stdio/FreeBSD/sscanf.c
	stdio/FreeBSD/stdio.c
	stdio/FreeBSD/swprintf.c
	stdio/FreeBSD/swscanf.c
	stdio/FreeBSD/tempnam.c
	stdio/FreeBSD/tmpfile.c
	stdio/FreeBSD/tmpnam.c
	stdio/FreeBSD/ungetc.c
	stdio/FreeBSD/ungetwc.c
	stdio/FreeBSD/vasprintf.c
	stdio/FreeBSD/vdprintf.c
	stdio/FreeBSD/vfprintf.c
	stdio/FreeBSD/vfscanf.c
	stdio/FreeBSD/vfwprintf.c
	stdio/FreeBSD/vfwscanf.c
	stdio/FreeBSD/vprintf.c
	stdio/FreeBSD/vscanf.c
	stdio/FreeBSD/vsnprintf.c
	stdio/FreeBSD/vsprintf.c
	stdio/FreeBSD/vsscanf.c
	stdio/FreeBSD/vswprintf.c
	stdio/FreeBSD/vswscanf.c
	stdio/FreeBSD/vwprintf.c
	stdio/FreeBSD/vwscanf.c
	stdio/FreeBSD/wbuf.c
	stdio/FreeBSD/wprintf.c
	stdio/FreeBSD/wscanf.c
	stdio/FreeBSD/wsetup.c
	stdio/FreeBSD/open_wmemstream.c
	stdtime/FreeBSD/asctime.c
	stdtime/FreeBSD/difftime.c
	stdtime/FreeBSD/ftime.c
	stdtime/FreeBSD/localtime.c
	stdtime/FreeBSD/strftime.c
	stdtime/FreeBSD/strptime.c
	stdtime/FreeBSD/time32.c
	stdtime/FreeBSD/timelocal.c
	string/FreeBSD/memmem.c
	string/FreeBSD/rindex.c
	string/FreeBSD/strcasecmp.c
	string/FreeBSD/strcasestr.c
	string/FreeBSD/strcoll.c
	string/FreeBSD/strcspn.c
	string/FreeBSD/strdup.c
	string/FreeBSD/strerror.c
	string/FreeBSD/strmode.c
	string/FreeBSD/strndup.c
	string/FreeBSD/strnstr.c
	string/FreeBSD/strpbrk.c
	string/FreeBSD/strrchr.c
	string/FreeBSD/strsep.c
	string/FreeBSD/strsignal.c
	string/FreeBSD/strspn.c
	string/FreeBSD/strtok.c
	string/FreeBSD/strxfrm.c
	string/FreeBSD/swab.c
	string/FreeBSD/wcpcpy.c
	string/FreeBSD/wcpncpy.c
	string/FreeBSD/wcscasecmp.c
	string/FreeBSD/wcscat.c
	string/FreeBSD/wcschr.c
	string/FreeBSD/wcscmp.c
	string/FreeBSD/wcscoll.c
	string/FreeBSD/wcscpy.c
	gen/FreeBSD/wordexp.c
	string/FreeBSD/wcscspn.c
	string/FreeBSD/wcsdup.c
	string/FreeBSD/wcslcat.c
	string/FreeBSD/wcslcpy.c
	string/FreeBSD/wcslen.c
	string/FreeBSD/wcsncasecmp.c
	string/FreeBSD/wcsncat.c
	string/FreeBSD/wcsncmp.c
	string/FreeBSD/wcsncpy.c
	string/FreeBSD/wcsnlen.c
	string/FreeBSD/wcspbrk.c
	string/FreeBSD/wcsrchr.c
	string/FreeBSD/wcsspn.c
	stdio/FreeBSD/fmemopen.c
	string/FreeBSD/wcsstr.c
	string/FreeBSD/wcstok.c
	string/FreeBSD/wcswidth.c
	string/FreeBSD/wcsxfrm.c
	string/FreeBSD/wmemchr.c
	string/FreeBSD/wmemcmp.c
	string/FreeBSD/wmemcpy.c
	string/FreeBSD/wmemmove.c
	string/FreeBSD/wmemset.c
	regex/FreeBSD/regerror.c
	stdlib/FreeBSD/abort.c
	locale/FreeBSD/utf2.c
	sys/OpenBSD/stack_protector.c
	stdlib/a64l.c
	stdlib/FreeBSD/_Exit_.c
	stdlib/FreeBSD/abs.c
	stdlib/FreeBSD/atexit.c
	stdlib/FreeBSD/atof.c
	stdlib/FreeBSD/atoi.c
	stdlib/FreeBSD/atol.c
	stdlib/FreeBSD/atoll.c
	stdlib/FreeBSD/bsearch.c
	stdlib/FreeBSD/div.c
	stdlib/FreeBSD/exit.c
	stdlib/FreeBSD/getenv.c
	stdlib/FreeBSD/getopt.c
	stdlib/FreeBSD/getopt_long.c
	stdlib/FreeBSD/getsubopt.c
	stdlib/FreeBSD/hcreate.c
	stdlib/FreeBSD/heapsort.c
	stdlib/FreeBSD/imaxabs.c
	stdlib/FreeBSD/imaxdiv.c
	stdlib/FreeBSD/insque.c
	stdlib/FreeBSD/labs.c
	stdlib/FreeBSD/ldiv.c
	stdlib/FreeBSD/llabs.c
	stdlib/FreeBSD/lldiv.c
	stdlib/FreeBSD/lsearch.c
	stdlib/FreeBSD/merge.c
	stdlib/FreeBSD/putenv.c
	stdlib/FreeBSD/qsort.c
	stdlib/FreeBSD/qsort_r.c
	stdlib/FreeBSD/radixsort.c
	stdlib/FreeBSD/rand.c
	stdlib/FreeBSD/random.c
	stdlib/FreeBSD/reallocf.c
	stdlib/FreeBSD/realpath.c
	stdlib/FreeBSD/remque.c
	stdlib/FreeBSD/setenv.c
	stdlib/FreeBSD/strhash.c
	stdlib/FreeBSD/strtoimax.c
	stdlib/FreeBSD/strtol.c
	stdlib/FreeBSD/strtoll.c
	stdlib/FreeBSD/strtoq.c
	stdlib/FreeBSD/strtoul.c
	stdlib/FreeBSD/strtoull.c
	stdlib/FreeBSD/strtoumax.c
	stdlib/FreeBSD/strtouq.c
	stdlib/FreeBSD/system.c
	stdlib/FreeBSD/tdelete.c
	stdlib/FreeBSD/tfind.c
	stdlib/FreeBSD/tsearch.c
	stdlib/FreeBSD/twalk.c
	stdlib/grantpt.c
	stdlib/l64a.c
	stdlib/OpenBSD/ecvt.c
	stdlib/OpenBSD/gcvt.c
	stdlib/qsort_b.c
	stdlib/FreeBSD/heapsort_b.c
	stdlib/FreeBSD/heapsort_r.c
	gdtoa/FreeBSD/_ldtoa.c
	gen/FreeBSD/timespec_get.c
	locale/FreeBSD/rune.c
	stdlib/FreeBSD/merge_b.c
	stdlib/FreeBSD/psort.c
	stdlib/FreeBSD/psort_b.c
	stdlib/FreeBSD/psort_r.c
	gen/FreeBSD/scandir_b.c
	gen/FreeBSD/getmntinfo64.c
	net/inet_ntop.c
	net/inet_pton.c
	stdio/FreeBSD/xprintf_errno.c
	stdio/FreeBSD/xprintf_float.c
	stdio/FreeBSD/xprintf_hexdump.c
	stdio/FreeBSD/xprintf_int.c
	stdio/FreeBSD/xprintf_quote.c
	stdio/FreeBSD/xprintf_str.c
	stdio/FreeBSD/xprintf_time.c
	stdio/FreeBSD/xprintf_vis.c
	stdio/FreeBSD/xprintf.c
)
add_dependencies(FreeBSD features)
if("${CpuArch}" STREQUAL "x86_64")
    target_sources(FreeBSD PRIVATE
        gdtoa/FreeBSD/gdtoa-strtopx.c gdtoa/FreeBSD/machdep_ldisx.c)
elseif("${CpuArch}" STREQUAL "arm64")
    target_sources(FreeBSD PRIVATE gdtoa/FreeBSD/machdep_ldisd.c)
endif()

target_compile_definitions(FreeBSD PRIVATE
    VARIANT_STATIC VARIANT_CANCELABLE VARIANT_DARWINEXTSN HAVE_STDINT_H
    __DARWIN_NON_CANCELABLE=0 __DARWIN_VERS_1050 __DARWIN_64_BIT_INO_T
    __DARWIN_UNIX03 __isthreaded
    FP_NAN=0 FP_INFINITE=1 FP_ZERO=2 FP_SUBNORMAL=3 FP_NORMAL=4 FP_SUPERNORMAL=5
)
target_compile_options(FreeBSD PRIVATE
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h"
    "-include${SDK_SOURCE_DIR}/usr/include/os/availability.h"
    -DPRIVATE -D__ct_rune_t=ct_rune_t -D__va_list=va_list
)
target_include_directories(FreeBSD PRIVATE
include
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach fbsdcompat gdtoa
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    ../private ../libsystem_info/membership.subproj
    ../libsystem_info/lookup.subproj ../libsystem_notify
    ../libsystem_info/gen.subproj gdtoa gdtoa/FreeBSD ../libsystem_blocks
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/private
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    ${SDK_SOURCE_DIR}/usr/include ${SDK_SOURCE_DIR}/usr/include/arpa
    ${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
)
set_source_files_properties(./compat-43/FreeBSD/creat.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__FBSDID=__RCSID")
set_source_files_properties(./db/btree/FreeBSD/bt_close.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_conv.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_debug.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_delete.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_get.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_open.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-UDEBUG")
set_source_files_properties(./db/btree/FreeBSD/bt_overflow.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-UDEBUG")
set_source_files_properties(./db/btree/FreeBSD/bt_page.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_put.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_search.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_seq.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_split.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/btree/FreeBSD/bt_utils.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/db/FreeBSD/db.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/hash/FreeBSD/hash.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-UDEBUG")
set_source_files_properties(./db/hash/FreeBSD/hash_bigkey.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-UDEBUG")
set_source_files_properties(./db/hash/FreeBSD/hash_buf.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-UDEBUG")
set_source_files_properties(./db/hash/FreeBSD/hash_func.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/hash/FreeBSD/hash_log2.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/hash/FreeBSD/hash_page.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-UDEBUG")
set_source_files_properties(./db/hash/FreeBSD/ndbm.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/mpool/FreeBSD/mpool.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-UDEBUG")
set_source_files_properties(./db/recno/FreeBSD/rec_close.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/recno/FreeBSD/rec_delete.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/recno/FreeBSD/rec_get.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/recno/FreeBSD/rec_open.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE;-D_DARWIN_UNLIMITED_STREAMS")
set_source_files_properties(./db/recno/FreeBSD/rec_put.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/recno/FreeBSD/rec_search.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/recno/FreeBSD/rec_seq.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./db/recno/FreeBSD/rec_utils.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D__DBINTERFACE_PRIVATE")
set_source_files_properties(./gen/FreeBSD/closedir.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/opendir.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/readdir.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/rewinddir.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/scandir.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/seekdir.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/telldir.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./locale/FreeBSD/collate.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D_DARWIN_UNLIMITED_STREAMS")
set_source_files_properties(./locale/FreeBSD/setrunelocale.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-D_DARWIN_UNLIMITED_STREAMS")
set_source_files_properties(./sys/OpenBSD/stack_protector.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-fno-stack-protector")
set_source_files_properties(./stdlib/FreeBSD/realpath.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REALPATH")
set_source_files_properties(./stdlib/FreeBSD/psort_b.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-DI_AM_PSORT_B")
set_source_files_properties(./stdlib/FreeBSD/psort_r.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-DI_AM_PSORT_R")
set_source_files_properties(./stdio/FreeBSD/xprintf_errno.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf_float.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf_hexdump.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf_int.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf_quote.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf_str.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf_time.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf_vis.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")
set_source_files_properties(./stdio/FreeBSD/xprintf.c TARGET_DIRECTORY FreeBSD PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/stdio")

#-------------------------------------------------------------------------
# libNetBSD.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(NetBSD STATIC
    gen/NetBSD/utmpx.c
    stdlib/NetBSD/strfmon.c
    string/NetBSD/memset_s.c
    gen/NetBSD/rb.c
)
add_dependencies(NetBSD features)
target_compile_definitions(NetBSD PRIVATE
    VARIANT_STATIC VARIANT_CANCELABLE VARIANT_DARWINEXTSN
    HAVE_STDINT_H __DARWIN_64_BIT_INO_T __DARWIN_NON_CANCELABLE=0
	__DARWIN_VERS_1050 __DARWIN_UNIX03 rsize_t=__darwin_size_t
)
target_compile_options(NetBSD PRIVATE
    -include${CMAKE_CURRENT_SOURCE_DIR}/nbsdcompat/_nbsd_compat_.h
)
target_include_directories(NetBSD PRIVATE
    include ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    nbsdcompat ../private
    ${SDK_SOURCE_DIR}/usr/include ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)
#-------------------------------------------------------------------------
# libTRE.a
#-------------------------------------------------------------------------
add_library(TRE STATIC
	regex/TRE/lib/regcomp.c
	regex/TRE/lib/regexec.c
	regex/TRE/lib/tre-ast.c
    regex/TRE/lib/tre-compile.c
	regex/TRE/lib/tre-match-backtrack.c
	regex/TRE/lib/tre-match-parallel.c
	regex/TRE/lib/tre-mem.c
	regex/TRE/lib/tre-parse.c
	regex/TRE/lib/tre-stack.c
)
add_dependencies(TRE features)
target_compile_definitions(TRE PRIVATE
    VARIANT_STATIC VARIANT_CANCELABLE VARIANT_DARWINEXTSN
    __DARWIN_NON_CANCELABLE=0 HAVE_STDINT_H HAVE_CONFIG_H
    __DARWIN_64_BIT_INO_T __DARWIN_VERS_1050 __DARWIN_UNIX03
)
target_include_directories(TRE PRIVATE
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    regex/TRE regex/FreeBSD locale/FreeBSD stdtime/FreeBSD ../private
    ${SDK_SOURCE_DIR}/usr/include ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)

#-------------------------------------------------------------------------
# libFortifySource.a
#-------------------------------------------------------------------------
add_library(FortifySource STATIC
    secure/strlcat_chk.c
    secure/chk_fail.c
    secure/memset_chk.c
    secure/strlcpy_chk.c
    secure/strncat_chk.c
    secure/strncpy_chk.c
    secure/memmove_chk.c
    secure/strcpy_chk.c
    secure/memccpy_chk.c
    secure/snprintf_chk.c
    secure/strcat_chk.c
    secure/vsprintf_chk.c
    secure/stpcpy_chk.c
    secure/sprintf_chk.c
    secure/memcpy_chk.c
    secure/stpncpy_chk.c
    secure/vsnprintf_chk.c
)
add_dependencies(FortifySource features)
target_compile_definitions(FortifySource PRIVATE
    VARIANT_STATIC VARIANT_CANCELABLE VARIANT_DARWINEXTSN
    __DARWIN_NON_CANCELABLE=0 HAVE_STDINT_H HAVE_CONFIG_H _FORTIFY_SOURCE=1
    __DARWIN_64_BIT_INO_T __DARWIN_VERS_1050 __DARWIN_UNIX03
)
target_include_directories(FortifySource PRIVATE
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd ../private
)


#-------------------------------------------------------------------------
#                             VARIANT TARGETS
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# libvCancelable.a
#-------------------------------------------------------------------------

set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(vCancelable STATIC
    compat-43/FreeBSD/creat.c
    compat-43/sigcompat.c
    gen/FreeBSD/lockf.c
    gen/nanosleep.c
    gen/FreeBSD/pause.c
    gen/FreeBSD/sleep.c
    gen/FreeBSD/termios.c
    gen/FreeBSD/usleep.c
    gen/FreeBSD/wait.c
    gen/FreeBSD/waitpid.c
    net/FreeBSD/recv.c
    net/FreeBSD/send.c
    stdlib/FreeBSD/system.c
)
add_dependencies(vCancelable features)
target_compile_definitions(vCancelable PRIVATE
    BUILDING_VARIANT VARIANT_CANCELABLE __DARWIN_VERS_1050
	__DARWIN_UNIX03 __DARWIN_64_BIT_INO_T __DARWIN_NON_CANCELABLE=0
)
target_include_directories(vCancelable PRIVATE
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd fbsdcompat
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    regex/TRE regex/FreeBSD locale/FreeBSD stdtime/FreeBSD ../private
    ${SDK_SOURCE_DIR}/usr/include ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)
target_compile_options(vCancelable PRIVATE
    -include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h
)
set_source_files_properties(./compat-43/FreeBSD/creat.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CREAT")
set_source_files_properties(./compat-43/sigcompat.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SIGPAUSE")
set_source_files_properties(./gen/FreeBSD/lockf.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_LOCKF")
set_source_files_properties(./gen/FreeBSD/pause.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_PAUSE;-U__DARWIN_NON_CANCELABLE;-D__DARWIN_NON_CANCELABLE=0")
set_source_files_properties(./gen/FreeBSD/sleep.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SLEEP;-U__DARWIN_NON_CANCELABLE;-D__DARWIN_NON_CANCELABLE=0")
set_source_files_properties(./gen/FreeBSD/termios.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_TCDRAIN;-U__DARWIN_NON_CANCELABLE;-D__DARWIN_NON_CANCELABLE=0")
set_source_files_properties(./gen/FreeBSD/usleep.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_USLEEP;-U__DARWIN_NON_CANCELABLE;-D__DARWIN_NON_CANCELABLE=0")
set_source_files_properties(./gen/FreeBSD/wait.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAIT;-U__DARWIN_NON_CANCELABLE;-D__DARWIN_NON_CANCELABLE=0")
set_source_files_properties(./gen/FreeBSD/waitpid.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAITPID;-U__DARWIN_NON_CANCELABLE;-D__DARWIN_NON_CANCELABLE=0")
set_source_files_properties(./gen/nanosleep.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_NANOSLEEP")
set_source_files_properties(./net/FreeBSD/recv.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_RECV")
set_source_files_properties(./net/FreeBSD/send.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SEND")
set_source_files_properties(./stdlib/FreeBSD/system.c TARGET_DIRECTORY vCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SYSTEM")

#-------------------------------------------------------------------------
# libvDarwinExtsn.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(vDarwinExtsn STATIC
    compat-43/FreeBSD/creat.c
    compat-43/sigcompat.c
    gen/FreeBSD/lockf.c
    gen/FreeBSD/pause.c
    gen/FreeBSD/popen.c
    gen/FreeBSD/sleep.c
    gen/FreeBSD/termios.c
    gen/FreeBSD/usleep.c
    gen/FreeBSD/wait.c
    gen/FreeBSD/waitpid.c
    gen/nanosleep.c
    net/FreeBSD/recv.c
    net/FreeBSD/send.c
    stdio/FreeBSD/fdopen.c
    stdio/FreeBSD/fopen.c
    stdlib/FreeBSD/realpath.c
    stdlib/FreeBSD/system.c
    sys/getgroups.c
)
add_dependencies(vDarwinExtsn features)
target_compile_options(vDarwinExtsn PRIVATE 
    -include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/_fbsd_compat_.h
    -include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h

)
target_compile_definitions(vDarwinExtsn PRIVATE
    BUILDING_VARIANT VARIANT_DARWINEXTSN __DARWIN_UNIX03 __DARWIN_64_BIT_INO_T
	__DARWIN_NON_CANCELABLE __DARWIN_VERS_1050
)
target_include_directories(vDarwinExtsn PRIVATE
    ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd fbsdcompat
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
    regex/TRE regex/FreeBSD locale/FreeBSD stdtime/FreeBSD ../private
    ${SDK_SOURCE_DIR}/usr/include ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)
set_source_files_properties(./compat-43/FreeBSD/creat.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CREAT")
set_source_files_properties(./compat-43/sigcompat.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SIGPAUSE")
set_source_files_properties(./gen/FreeBSD/lockf.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_LOCKF")
set_source_files_properties(./gen/FreeBSD/pause.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_PAUSE")
set_source_files_properties(./gen/FreeBSD/popen.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_POPEN")
set_source_files_properties(./gen/FreeBSD/sleep.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SLEEP")
set_source_files_properties(./gen/FreeBSD/termios.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_TCDRAIN")
set_source_files_properties(./gen/FreeBSD/usleep.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_USLEEP")
set_source_files_properties(./gen/FreeBSD/wait.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAIT")
set_source_files_properties(./gen/FreeBSD/waitpid.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAITPID")
set_source_files_properties(./gen/nanosleep.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_NANOSLEEP")
set_source_files_properties(./net/FreeBSD/recv.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_RECV")
set_source_files_properties(./net/FreeBSD/send.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SEND")
set_source_files_properties(./stdio/FreeBSD/fdopen.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FDOPEN")
set_source_files_properties(./stdio/FreeBSD/fopen.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FOPEN")
set_source_files_properties(./stdlib/FreeBSD/realpath.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REALPATH")
set_source_files_properties(./stdlib/FreeBSD/system.c TARGET_DIRECTORY vDarwinExtsn PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SYSTEM")


#-------------------------------------------------------------------------
# libvDarwinExtsnCancelable.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(vDarwinExtsnCancelable STATIC
    compat-43/FreeBSD/creat.c
    compat-43/sigcompat.c
    gen/FreeBSD/lockf.c
    gen/nanosleep.c
    gen/FreeBSD/pause.c
    gen/FreeBSD/sleep.c
    gen/FreeBSD/termios.c
    gen/FreeBSD/usleep.c
    gen/FreeBSD/wait.c
    gen/FreeBSD/waitpid.c
    net/FreeBSD/recv.c
    net/FreeBSD/send.c
    stdlib/FreeBSD/system.c
    gen/FreeBSD/popen.c
    stdio/FreeBSD/fdopen.c
    stdio/FreeBSD/fopen.c
    stdlib/FreeBSD/realpath.c
    sys/getgroups.c
)
add_dependencies(vDarwinExtsnCancelable features)
target_compile_definitions(vDarwinExtsnCancelable PRIVATE
    BUILDING_VARIANT VARIANT_CANCELABLE VARIANT_DARWINEXTSN __DARWIN_VERS_1050
    __DARWIN_UNIX03 __DARWIN_64_BIT_INO_T __DARWIN_NON_CANCELABLE=0
)
target_include_directories(vDarwinExtsnCancelable PRIVATE
    include ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    fbsdcompat ../private
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
)
target_compile_options(vDarwinExtsnCancelable PRIVATE
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/_fbsd_compat_.h"
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h"
    -DPRIVATE
)
set_source_files_properties(./compat-43/FreeBSD/creat.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CREAT")
set_source_files_properties(./compat-43/sigcompat.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SIGPAUSE")
set_source_files_properties(./gen/FreeBSD/lockf.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_LOCKF")
set_source_files_properties(./gen/FreeBSD/pause.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_PAUSE")
set_source_files_properties(./gen/FreeBSD/popen.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_POPEN")
set_source_files_properties(./gen/FreeBSD/sleep.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SLEEP")
set_source_files_properties(./gen/FreeBSD/termios.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_TCDRAIN")
set_source_files_properties(./gen/FreeBSD/usleep.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_USLEEP")
set_source_files_properties(./gen/FreeBSD/wait.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAIT")
set_source_files_properties(./gen/FreeBSD/waitpid.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAITPID")
set_source_files_properties(./gen/nanosleep.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_NANOSLEEP")
set_source_files_properties(./net/FreeBSD/recv.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_RECV")
set_source_files_properties(./net/FreeBSD/send.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SEND")
set_source_files_properties(./stdio/FreeBSD/fdopen.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FDOPEN")
set_source_files_properties(./stdio/FreeBSD/fopen.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FOPEN")
set_source_files_properties(./stdlib/FreeBSD/realpath.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REALPATH")
set_source_files_properties(./stdlib/FreeBSD/system.c TARGET_DIRECTORY vDarwinExtsnCancelable PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SYSTEM")

#-------------------------------------------------------------------------
# libvPre1050.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(vPre1050 STATIC
    compat-43/FreeBSD/creat.c
    compat-43/sigcompat.c
    gen/FreeBSD/daemon.c
    gen/FreeBSD/lockf.c
    gen/FreeBSD/pause.c
    gen/FreeBSD/popen.c
    gen/FreeBSD/sleep.c
    gen/FreeBSD/termios.c
    gen/FreeBSD/usleep.c
    gen/FreeBSD/wait.c
    gen/FreeBSD/waitpid.c
    gen/nanosleep.c
    net/FreeBSD/recv.c
    net/FreeBSD/send.c
    stdio/FreeBSD/fdopen.c
    stdio/FreeBSD/fopen.c
    stdlib/FreeBSD/realpath.c
    stdlib/FreeBSD/system.c
)
add_dependencies(vPre1050 features)
target_compile_definitions(vPre1050 PRIVATE
    BUILDING_VARIANT VARIANT_PRE1050 __DARWIN_VERS_1050=0 __DARWIN_UNIX03=1
	__DARWIN_64_BIT_INO_T DARWIN_NON_CANCELABLE
)
target_include_directories(vPre1050 PRIVATE
    include ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
    fbsdcompat ../private
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
    ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn
    ${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
)
target_compile_options(vPre1050 PRIVATE
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/_fbsd_compat_.h"
    "-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h"
    -DPRIVATE
)
set_source_files_properties(./compat-43/FreeBSD/creat.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CREAT")
set_source_files_properties(./compat-43/sigcompat.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SIGPAUSE")
set_source_files_properties(./gen/FreeBSD/lockf.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_LOCKF")
set_source_files_properties(./gen/FreeBSD/pause.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_PAUSE")
set_source_files_properties(./gen/FreeBSD/popen.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_POPEN")
set_source_files_properties(./gen/FreeBSD/sleep.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SLEEP")
set_source_files_properties(./gen/FreeBSD/termios.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_TCDRAIN")
set_source_files_properties(./gen/FreeBSD/usleep.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_USLEEP")
set_source_files_properties(./gen/FreeBSD/wait.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAIT")
set_source_files_properties(./gen/FreeBSD/waitpid.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAITPID")
set_source_files_properties(./gen/nanosleep.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_NANOSLEEP")
set_source_files_properties(./net/FreeBSD/recv.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_RECV")
set_source_files_properties(./net/FreeBSD/send.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SEND")
set_source_files_properties(./stdio/FreeBSD/fdopen.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FDOPEN")
set_source_files_properties(./stdio/FreeBSD/fopen.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FOPEN")
set_source_files_properties(./stdlib/FreeBSD/realpath.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REALPATH")
set_source_files_properties(./stdlib/FreeBSD/system.c TARGET_DIRECTORY vPre1050 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SYSTEM")

#-------------------------------------------------------------------------
# libvLegacy.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(vLegacy STATIC
    compat-43/FreeBSD/creat.c
    compat-43/setregid.c
    compat-43/setreuid.c
    compat-43/sigcompat.c
    compat-43/FreeBSD/killpg.c
    gdtoa/FreeBSD/gdtoa-strtof.c
    gdtoa/FreeBSD/gdtoa-strtod.c
    gdtoa/FreeBSD/gdtoa-strtodg.c
    gen/FreeBSD/clock.c
    gen/confstr.c
    gen/crypt.c
    gen/FreeBSD/fnmatch.c
    gen/FreeBSD/lockf.c
    gen/nanosleep.c
    gen/nftw.c
    gen/FreeBSD/nice.c
    gen/FreeBSD/pause.c
    gen/FreeBSD/popen.c
    gen/FreeBSD/setmode.c
    gen/FreeBSD/sleep.c
    gen/FreeBSD/termios.c
    gen/FreeBSD/timezone.c
    gen/FreeBSD/ttyname.c
    gen/FreeBSD/usleep.c
    gen/FreeBSD/wait.c
    gen/FreeBSD/waitpid.c
    locale/FreeBSD/wcsftime.c
    net/FreeBSD/recv.c
    net/FreeBSD/send.c
    regex/TRE/lib/regcomp.c
    stdio/FreeBSD/fdopen.c
    stdio/FreeBSD/fopen.c
    stdio/FreeBSD/fputs.c
    stdio/FreeBSD/freopen.c
    stdio/FreeBSD/fwrite.c 
    stdio/FreeBSD/tempnam.c
    stdlib/FreeBSD/getopt.c
    stdlib/FreeBSD/putenv.c
    stdlib/FreeBSD/realpath.c
    stdlib/FreeBSD/setenv.c
    stdlib/FreeBSD/system.c
    stdtime/FreeBSD/localtime.c
    stdtime/FreeBSD/strftime.c
    stdtime/FreeBSD/strptime.c
    sys/msgctl.c
    sys/semctl.c
    sys/shmctl.c
)
add_dependencies(vLegacy features)
target_compile_definitions(vLegacy PRIVATE
    BUILDING_VARIANT VARIANT_LEGACY __DARWIN_UNIX03=0 __DARWIN_64_BIT_INO_T=0
    __DARWIN_NON_CANCELABLE __DARWIN_VERS_1050 __isthreaded
)
target_include_directories(vLegacy PRIVATE
	BEFORE include ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd
	AFTER ../private fbsdcompat ${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
	${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk gdtoa
	../private/pthread regex/TRE ${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
)
target_compile_options(vLegacy PRIVATE
    "-include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/cdefs.h"
	#"-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h"
)
set_source_files_properties(./compat-43/FreeBSD/creat.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CREAT;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h")
set_source_files_properties(./compat-43/FreeBSD/killpg.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_KILLPG;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./compat-43/setregid.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SETREGID;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./compat-43/setreuid.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SETREUID;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./compat-43/sigcompat.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SIGPAUSE;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./gen/confstr.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CONFSTR;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./gen/crypt.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_ENCRYPT;-DLIBC_ALIAS_SETKEY")
set_source_files_properties(./gen/FreeBSD/clock.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_CLOCK;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./gen/FreeBSD/fnmatch.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FNMATCH;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./gen/FreeBSD/lockf.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_LOCKF;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./gen/FreeBSD/nice.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_NICE")
set_source_files_properties(./gen/FreeBSD/pause.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_PAUSE;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h")
set_source_files_properties(./gen/FreeBSD/popen.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_POPEN;-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn")
set_source_files_properties(./gen/FreeBSD/setmode.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SETMODE")
set_source_files_properties(./gen/FreeBSD/sleep.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SLEEP;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h")
set_source_files_properties(./gen/FreeBSD/termios.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_TCDRAIN;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h")
set_source_files_properties(./gen/FreeBSD/ttyname.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_TTYNAME_R")
set_source_files_properties(./gen/FreeBSD/usleep.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_USLEEP")
set_source_files_properties(./gen/FreeBSD/wait.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAIT;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h")
set_source_files_properties(./gen/FreeBSD/waitpid.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WAITPID;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h")
set_source_files_properties(./gen/nanosleep.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_NANOSLEEP;-include${CMAKE_CURRENT_SOURCE_DIR}/include/sys/cdefs.h")
set_source_files_properties(./gen/nftw.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FTW;-DLIBC_ALIAS_NFTW")
set_source_files_properties(./locale/FreeBSD/wcsftime.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_WCSFTIME;-DLIBC_ALIAS_WCSFTIME_L")
set_source_files_properties(./net/FreeBSD/recv.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_RECV")
set_source_files_properties(./net/FreeBSD/send.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SEND")
set_source_files_properties(./stdio/FreeBSD/fdopen.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FDOPEN")
set_source_files_properties(./stdio/FreeBSD/fopen.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FOPEN")
set_source_files_properties(./stdio/FreeBSD/fputs.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FPUTS")
set_source_files_properties(./stdio/FreeBSD/freopen.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FREOPEN")
set_source_files_properties(./stdio/FreeBSD/fwrite.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FWRITE")
set_source_files_properties(./stdio/FreeBSD/tempnam.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_TEMPNAM;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h")
set_source_files_properties(./stdlib/FreeBSD/getopt.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_GETOPT")
set_source_files_properties(./stdlib/FreeBSD/putenv.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_PUTENV")
set_source_files_properties(./stdlib/FreeBSD/realpath.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REALPATH")
set_source_files_properties(./stdlib/FreeBSD/setenv.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REALPATH")
set_source_files_properties(./stdlib/FreeBSD/system.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SYSTEM;-include${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat/sys/cdefs.h;-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn")
set_source_files_properties(./stdtime/FreeBSD/localtime.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_MKTIME")
set_source_files_properties(./stdtime/FreeBSD/strptime.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_STRPTIME;-DLIBC_ALIAS_STRPTIME_L")
set_source_files_properties(./sys/msgctl.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_MSGCTL;-DKERNEL;-D_LIBC_NO_FEATURE_VERIFICATION")
set_source_files_properties(./sys/semctl.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SEMCTL;-DKERNEL;-D_LIBC_NO_FEATURE_VERIFICATION")
set_source_files_properties(./sys/shmctl.c TARGET_DIRECTORY vLegacy PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SHMCTL;-DKERNEL;-D_LIBC_NO_FEATURE_VERIFICATION")


#-------------------------------------------------------------------------
# libvInode32.a
#-------------------------------------------------------------------------
set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(vInode32 STATIC
    gen/fts.c
    gen/FreeBSD/getmntinfo.c
    gen/FreeBSD/glob.c
    gen/nftw.c
    gen/FreeBSD/opendir.c
    gen/FreeBSD/readdir.c
    gen/FreeBSD/rewinddir.c
    gen/FreeBSD/scandir.c
    gen/FreeBSD/seekdir.c
    gen/FreeBSD/telldir.c
    gen/FreeBSD/scandir_b.c
    sys/statx_np.c
)
add_dependencies(vInode32 features)
target_compile_definitions(vInode32 PRIVATE
    BUILDING_VARIANT VARIANT_INODE32 __DARWIN_64_BIT_INO_T=0 __isthreaded
	__DARWIN_UNIX03 __DARWIN_NON_CANCELABLE __DARWIN_VERS_1050
)
target_include_directories(vInode32 PRIVATE
	include ${ROOT_SOURCE_DIR}/Kernel/xnu/bsd 
	${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach
	${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk
	../private fbsdcompat
)
target_compile_options(vInode32 PRIVATE
	-DPRIVATE -include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/dirent.h
)
set_source_files_properties(./gen/FreeBSD/glob.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_GLOB")
set_source_files_properties(./gen/FreeBSD/opendir.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS___OPENDIR2;-DLIBC_ALIAS_OPENDIR;-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/readdir.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/rewinddir.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_REWINDDIR;-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/scandir.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/seekdir.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_SEEKDIR;-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/FreeBSD/telldir.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS__SEEKDIR;-DLIBC_ALIAS_TELLDIR;-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")
set_source_files_properties(./gen/fts.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FTS_CHILDREN;-DLIBC_ALIAS_FTS_CLOSE;-DLIBC_ALIAS_FTS_OPEN;-DLIBC_ALIAS_FTS_OPEN_B;-DLIBC_ALIAS_FTS_READ;-DLIBC_ALIAS_FTS_SET")
set_source_files_properties(./gen/nftw.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-DLIBC_ALIAS_FTW;-DLIBC_ALIAS_NFTW")
set_source_files_properties(./gen/FreeBSD/scandir_b.c TARGET_DIRECTORY vInode32 PROPERTIES COMPILE_OPTIONS "-include${CMAKE_CURRENT_SOURCE_DIR}/gen/__dirent.h")


add_dependencies(Base NetBSD FortifySource)
add_custom_command(OUTPUT libBase.a
    COMMAND ${TOOLS}/llvm-libtool-darwin -static -arch_only ${CpuArch}
		-o libBase.a $<TARGET_FILE:Base> $<TARGET_FILE:NetBSD>
		$<TARGET_FILE:FortifySource>
)
add_dependencies(libc_static Base FreeBSD TRE vCancelable vDarwinExtsn)
add_dependencies(libc_dyld Base FreeBSD TRE vCancelable)

# FIXME: we need to put these in the proper locations (/usr/local/system/lib,
# /usr/local/loaderd, etc)

#-------------------------------------------------------------------------
#                             LIBSYSTEM_C
#-------------------------------------------------------------------------

set(CMAKE_CURRENT_SOURCE_DIR ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c)
add_library(libsystem_c SHARED
	libBase.a
    darwin/forceLibcToBuild.c
    darwin/compatibility_hacks.c
)
set_property(TARGET libsystem_c PROPERTY OUTPUT_NAME "system_c")
set_property(TARGET libsystem_c PROPERTY SUFFIX ".dylib")
set_property(TARGET libsystem_c PROPERTY PREFIX "lib")
set_property(TARGET libsystem_c PROPERTY NO_SONAME true)

add_custom_target(generateLinklists
	COMMAND env 
		NM=${NM}
		BUILD_ARCHIVES="Base FreeBSD TRE vCancelable vDarwinExtsn vDarwinExtsnCancelable vPre1050 vLegacy vInode32"
		BUILT_PRODUCTS_DIR=${CMAKE_CURRENT_BINARY_DIR} 
		DERIVED_FILES_DIR=${CMAKE_CURRENT_BINARY_DIR}
		sh ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/build_linklists.sh
)

add_dependencies(libsystem_c Platform
    vCancelable vDarwinExtsn vDarwinExtsnCancelable vPre1050 vLegacy vInode32
    libc_dyld libc_static generateLinklists libsystem_malloc libsystem_platform
	libsystem_pthread
)

target_link_options(libsystem_c PRIVATE
    -mmacos-version-min=10.15 -v -Wl,-install_name,/usr/lib/system/libsystem_c.dylib
    -rdynamic -nodefaultlibs -Wl,-search_paths_first
    -all_load -nostdlib -L${RAVYN_SDKROOT}/usr/lib/system -umbrella System
    #-lCrashReporterClient -lcompiler-rt -ldyld -lkernel -lm
    -lsystem_malloc -lsystem_platform -lsystem_pthread
    #-Wl,-upward-ldispatch -Wl,-upward-lmacho -Wl,-upward-lsystem_asl -Wl,-upward-lsystem_blocks
    #-Wl,-upward-lsystem_info -Wl,-upward-lsystem_notify -Wl,-upward-lxpc -Wl,-upward-lcorecrypto
    #-Wl,-upward-lsystem_trace
    -Wl,-interposable_list,${CMAKE_CURRENT_BINARY_DIR}/interposable.list
    -Wl,-unexported_symbols_list,${CMAKE_CURRENT_BINARY_DIR}/unexport.list
    -Wl,-alias_list,xcodescripts/alias.list
    -Wl,-order_file,xcodescripts/Libc.order
    @${CMAKE_CURRENT_BINARY_DIR}/normal.linklist
)

# FIXME: should also build `debug` variant