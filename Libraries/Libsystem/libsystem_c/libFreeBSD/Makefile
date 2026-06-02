.PATH: ${.CURDIR:H}
LIB = FreeBSD
SRCS =  compat-43/FreeBSD/gethostid.c \
	compat-43/FreeBSD/getwd.c \
	compat-43/FreeBSD/killpg.c \
	compat-43/FreeBSD/sethostid.c \
	compat-43/FreeBSD/setpgrp.c \
	compat-43/FreeBSD/setrgid.c \
	compat-43/FreeBSD/setruid.c \
	compat-43/sigcompat.c \
 \
	db/btree/FreeBSD/bt_close.c \
	db/btree/FreeBSD/bt_conv.c \
	db/btree/FreeBSD/bt_debug.c \
	db/btree/FreeBSD/bt_delete.c \
	db/btree/FreeBSD/bt_get.c \
	db/btree/FreeBSD/bt_open.c \
	db/btree/FreeBSD/bt_overflow.c \
	db/btree/FreeBSD/bt_page.c \
	db/btree/FreeBSD/bt_put.c \
	db/btree/FreeBSD/bt_search.c \
	db/btree/FreeBSD/bt_seq.c \
	db/btree/FreeBSD/bt_split.c \
	db/btree/FreeBSD/bt_utils.c \
	db/db/FreeBSD/db.c \
	db/hash/FreeBSD/hash.c \
	db/hash/FreeBSD/hash_bigkey.c \
	db/hash/FreeBSD/hash_buf.c \
	db/hash/FreeBSD/hash_func.c \
	db/hash/FreeBSD/hash_log2.c \
	db/hash/FreeBSD/hash_page.c \
	db/hash/FreeBSD/ndbm.c \
	db/mpool/FreeBSD/mpool.c \
	db/recno/FreeBSD/rec_close.c \
	db/recno/FreeBSD/rec_delete.c \
	db/recno/FreeBSD/rec_get.c \
	db/recno/FreeBSD/rec_open.c \
	db/recno/FreeBSD/rec_put.c \
	db/recno/FreeBSD/rec_search.c \
	db/recno/FreeBSD/rec_seq.c \
	db/recno/FreeBSD/rec_utils.c \
 \
	gdtoa/FreeBSD/_ldtoa.c \
	gdtoa/FreeBSD/_hdtoa.c \
	gdtoa/FreeBSD/gdtoa-dmisc.c \
	gdtoa/FreeBSD/gdtoa-dtoa.c \
	gdtoa/FreeBSD/gdtoa-gdtoa.c \
	gdtoa/FreeBSD/gdtoa-gethex.c \
	gdtoa/FreeBSD/gdtoa-gmisc.c \
	gdtoa/FreeBSD/gdtoa-hd_init.c \
	gdtoa/FreeBSD/gdtoa-hexnan.c \
	gdtoa/FreeBSD/gdtoa-misc.c \
	gdtoa/FreeBSD/gdtoa-smisc.c \
	gdtoa/FreeBSD/gdtoa-strtod.c \
	gdtoa/FreeBSD/gdtoa-strtodg.c \
	gdtoa/FreeBSD/gdtoa-strtof.c \
	gdtoa/FreeBSD/gdtoa-strtopdd.c \
	gdtoa/FreeBSD/gdtoa-strtord.c \
	gdtoa/FreeBSD/gdtoa-sum.c \
	gdtoa/FreeBSD/gdtoa-ulp.c \
	gdtoa/FreeBSD/glue.c \
 \
	gen/nanosleep.c \
	gen/FreeBSD/_rand48.c \
	gen/FreeBSD/alarm.c \
	gen/FreeBSD/arc4random.c \
	gen/FreeBSD/assert.c \
	gen/FreeBSD/basename.c \
	gen/FreeBSD/clock.c \
	gen/FreeBSD/closedir.c \
	gen/FreeBSD/ctermid.c \
	gen/FreeBSD/daemon.c \
	gen/FreeBSD/dirname.c \
	gen/FreeBSD/drand48.c \
	gen/FreeBSD/erand48.c \
	gen/FreeBSD/err.c \
	gen/FreeBSD/exec.c \
	gen/FreeBSD/fmtcheck.c \
	gen/FreeBSD/fmtmsg.c \
	gen/FreeBSD/lockf.c \
	gen/FreeBSD/fnmatch.c \
	gen/FreeBSD/ftok.c \
	gen/FreeBSD/getbsize.c \
	gen/FreeBSD/getcap.c \
	gen/FreeBSD/getcwd.c \
	gen/FreeBSD/gethostname.c \
	gen/FreeBSD/getlogin.c \
	gen/FreeBSD/getmntinfo.c \
	gen/FreeBSD/getpagesize.c \
	gen/FreeBSD/getpeereid.c \
	gen/FreeBSD/getprogname.c \
	gen/FreeBSD/glob.c \
	gen/FreeBSD/isatty.c \
	gen/FreeBSD/jrand48.c \
	gen/FreeBSD/lcong48.c \
	gen/FreeBSD/lrand48.c \
	gen/FreeBSD/mrand48.c \
	gen/FreeBSD/nice.c \
	gen/FreeBSD/nrand48.c \
	gen/FreeBSD/opendir.c \
	gen/FreeBSD/popen.c \
	gen/FreeBSD/psignal.c \
	gen/FreeBSD/readdir.c \
	gen/FreeBSD/readpassphrase.c \
	gen/FreeBSD/rewinddir.c \
	gen/FreeBSD/scandir.c \
	gen/FreeBSD/seed48.c \
	gen/FreeBSD/seekdir.c \
	gen/FreeBSD/sethostname.c \
	gen/FreeBSD/setmode.c \
	gen/FreeBSD/setprogname.c \
	gen/FreeBSD/siginterrupt.c \
	gen/FreeBSD/siglist.c \
	gen/FreeBSD/signal.c \
	gen/FreeBSD/sleep.c \
	gen/FreeBSD/srand48.c \
	gen/FreeBSD/stringlist.c \
	gen/FreeBSD/sysconf.c \
	gen/FreeBSD/sysctl.c \
	gen/FreeBSD/sysctlbyname.c \
	gen/FreeBSD/sysctlnametomib.c \
	gen/FreeBSD/termios.c \
	gen/FreeBSD/telldir.c \
	gen/FreeBSD/time.c \
	gen/FreeBSD/times.c \
	gen/FreeBSD/ttyname.c \
	gen/FreeBSD/ttyslot.c \
	gen/FreeBSD/ualarm.c \
	gen/FreeBSD/ulimit.c \
	gen/FreeBSD/usleep.c \
	gen/FreeBSD/unvis.c \
	gen/FreeBSD/utime.c \
	gen/FreeBSD/vis.c \
	gen/FreeBSD/wait3.c \
	gen/FreeBSD/waitpid.c \
	gen/FreeBSD/wordexp.c \
	gen/FreeBSD/timespec_get.c \
	gen/FreeBSD/scandir_b.c \
	gen/FreeBSD/getmntinfo64.c \
 \
	locale/FreeBSD/ascii.c \
	locale/FreeBSD/big5.c \
	locale/FreeBSD/btowc.c \
	locale/FreeBSD/collate.c \
	locale/FreeBSD/collcmp.c \
	locale/FreeBSD/euc.c \
	locale/FreeBSD/fix_grouping.c \
	locale/FreeBSD/gb18030.c \
	locale/FreeBSD/gb2312.c \
	locale/FreeBSD/gbk.c \
	locale/FreeBSD/ldpart.c \
	locale/FreeBSD/lmessages.c \
	locale/FreeBSD/lmonetary.c \
	locale/FreeBSD/lnumeric.c \
	locale/FreeBSD/localeconv.c \
	locale/FreeBSD/mblen.c \
	locale/FreeBSD/mbrlen.c \
	locale/FreeBSD/mbrtowc.c \
	locale/FreeBSD/mbsinit.c \
	locale/FreeBSD/mbsnrtowcs.c \
	locale/FreeBSD/mbsrtowcs.c \
	locale/FreeBSD/mbstowcs.c \
	locale/FreeBSD/mbtowc.c \
	locale/FreeBSD/mskanji.c \
	locale/FreeBSD/nextwctype.c \
	locale/FreeBSD/nl_langinfo.c \
	locale/FreeBSD/nomacros.c \
	locale/FreeBSD/none.c \
	locale/FreeBSD/runetype.c \
	locale/FreeBSD/setlocale.c \
	locale/FreeBSD/setrunelocale.c \
	locale/FreeBSD/table.c \
	locale/FreeBSD/tolower.c \
	locale/FreeBSD/toupper.c \
	locale/FreeBSD/utf8.c \
	locale/FreeBSD/wcrtomb.c \
	locale/FreeBSD/wcsftime.c \
	locale/FreeBSD/wcsnrtombs.c \
	locale/FreeBSD/wcsrtombs.c \
	locale/FreeBSD/wcstod.c \
	locale/FreeBSD/wcstof.c \
	locale/FreeBSD/wcstoimax.c \
	locale/FreeBSD/wcstol.c \
	locale/FreeBSD/wcstold.c \
	locale/FreeBSD/wcstoll.c \
	locale/FreeBSD/wcstombs.c \
	locale/FreeBSD/wcstoul.c \
	locale/FreeBSD/wcstoull.c \
	locale/FreeBSD/wcstoumax.c \
	locale/FreeBSD/wctob.c \
	locale/FreeBSD/wctomb.c \
	locale/FreeBSD/wctrans.c \
	locale/FreeBSD/wctype.c \
	locale/FreeBSD/wcwidth.c \
	locale/FreeBSD/rune.c \
	locale/FreeBSD/utf2.c \
 \
	net/FreeBSD/addr2ascii.c \
	net/FreeBSD/ascii2addr.c \
	net/FreeBSD/inet_addr.c \
	net/FreeBSD/inet_lnaof.c \
	net/FreeBSD/inet_makeaddr.c \
	net/FreeBSD/inet_net_ntop.c \
	net/FreeBSD/inet_net_pton.c \
	net/FreeBSD/inet_neta.c \
	net/FreeBSD/inet_netof.c \
	net/FreeBSD/inet_network.c \
	net/FreeBSD/inet_ntoa.c \
	net/FreeBSD/linkaddr.c \
	net/FreeBSD/nsap_addr.c \
	net/FreeBSD/sockatmark.c \
	net/FreeBSD/sourcefilter.c \
	net/FreeBSD/recv.c \
	net/FreeBSD/send.c \
	net/inet_ntop.c \
	net/inet_pton.c \
 \
	nls/FreeBSD/msgcat.c \
 \
	regex/FreeBSD/regerror.c \
 \
	stdio/FreeBSD/open_memstream.c \
	stdio/FreeBSD/_flock_stub.c \
	stdio/FreeBSD/asprintf.c \
	stdio/FreeBSD/clrerr.c \
	stdio/FreeBSD/dprintf.c \
	stdio/FreeBSD/fclose.c \
	stdio/FreeBSD/fdopen.c \
	stdio/FreeBSD/feof.c \
	stdio/FreeBSD/ferror.c \
	stdio/FreeBSD/fflush.c \
	stdio/FreeBSD/fgetc.c \
	stdio/FreeBSD/fgetln.c \
	stdio/FreeBSD/fgetpos.c \
	stdio/FreeBSD/fgets.c \
	stdio/FreeBSD/fgetwc.c \
	stdio/FreeBSD/fgetwln.c \
	stdio/FreeBSD/fgetws.c \
	stdio/FreeBSD/fileno.c \
	stdio/FreeBSD/findfp.c \
	stdio/FreeBSD/flags.c \
	stdio/FreeBSD/fopen.c \
	stdio/FreeBSD/fprintf.c \
	stdio/FreeBSD/fpurge.c \
	stdio/FreeBSD/fputc.c \
	stdio/FreeBSD/fputs.c \
	stdio/FreeBSD/fputwc.c \
	stdio/FreeBSD/fputws.c \
	stdio/FreeBSD/fread.c \
	stdio/FreeBSD/freopen.c \
	stdio/FreeBSD/fscanf.c \
	stdio/FreeBSD/fseek.c \
	stdio/FreeBSD/fsetpos.c \
	stdio/FreeBSD/ftell.c \
	stdio/FreeBSD/funopen.c \
	stdio/FreeBSD/fvwrite.c \
	stdio/FreeBSD/fwalk.c \
	stdio/FreeBSD/fwide.c \
	stdio/FreeBSD/fwprintf.c \
	stdio/FreeBSD/fwrite.c \
	stdio/FreeBSD/fwscanf.c \
	stdio/FreeBSD/getc.c \
	stdio/FreeBSD/getchar.c \
	stdio/FreeBSD/getdelim.c \
	stdio/FreeBSD/getline.c \
	stdio/FreeBSD/gets.c \
	stdio/FreeBSD/getw.c \
	stdio/FreeBSD/getwc.c \
	stdio/FreeBSD/getwchar.c \
	stdio/FreeBSD/makebuf.c \
	stdio/FreeBSD/mktemp.c \
	stdio/FreeBSD/perror.c \
	stdio/FreeBSD/printf-pos.c \
	stdio/FreeBSD/printf.c \
	stdio/FreeBSD/putc.c \
	stdio/FreeBSD/putchar.c \
	stdio/FreeBSD/puts.c \
	stdio/FreeBSD/putw.c \
	stdio/FreeBSD/putwc.c \
	stdio/FreeBSD/putwchar.c \
	stdio/FreeBSD/refill.c \
	stdio/FreeBSD/remove.c \
	stdio/FreeBSD/rewind.c \
	stdio/FreeBSD/rget.c \
	stdio/FreeBSD/scanf.c \
	stdio/FreeBSD/setbuf.c \
	stdio/FreeBSD/setbuffer.c \
	stdio/FreeBSD/setvbuf.c \
	stdio/FreeBSD/snprintf.c \
	stdio/FreeBSD/sprintf.c \
	stdio/FreeBSD/sscanf.c \
	stdio/FreeBSD/stdio.c \
	stdio/FreeBSD/swprintf.c \
	stdio/FreeBSD/swscanf.c \
	stdio/FreeBSD/tempnam.c \
	stdio/FreeBSD/tmpfile.c \
	stdio/FreeBSD/tmpnam.c \
	stdio/FreeBSD/ungetc.c \
	stdio/FreeBSD/ungetwc.c \
	stdio/FreeBSD/vasprintf.c \
	stdio/FreeBSD/vdprintf.c \
	stdio/FreeBSD/vfprintf.c \
	stdio/FreeBSD/vfscanf.c \
	stdio/FreeBSD/vfwprintf.c \
	stdio/FreeBSD/vfwscanf.c \
	stdio/FreeBSD/vprintf.c \
	stdio/FreeBSD/vscanf.c \
	stdio/FreeBSD/vsnprintf.c \
	stdio/FreeBSD/vsprintf.c \
	stdio/FreeBSD/vsscanf.c \
	stdio/FreeBSD/vswprintf.c \
	stdio/FreeBSD/vswscanf.c \
	stdio/FreeBSD/vwprintf.c \
	stdio/FreeBSD/vwscanf.c \
	stdio/FreeBSD/wbuf.c \
	stdio/FreeBSD/wprintf.c \
	stdio/FreeBSD/wscanf.c \
	stdio/FreeBSD/wsetup.c \
	stdio/FreeBSD/open_wmemstream.c \
	stdio/FreeBSD/fmemopen.c \
	stdio/FreeBSD/xprintf_errno.c \
	stdio/FreeBSD/xprintf_float.c \
	stdio/FreeBSD/xprintf_hexdump.c \
	stdio/FreeBSD/xprintf_int.c \
	stdio/FreeBSD/xprintf_quote.c \
	stdio/FreeBSD/xprintf_str.c \
	stdio/FreeBSD/xprintf_time.c \
	stdio/FreeBSD/xprintf_vis.c \
	stdio/FreeBSD/xprintf.c \
 \
	stdtime/FreeBSD/asctime.c \
	stdtime/FreeBSD/difftime.c \
	stdtime/FreeBSD/ftime.c \
	stdtime/FreeBSD/localtime.c \
	stdtime/FreeBSD/strftime.c \
	stdtime/FreeBSD/strptime.c \
	stdtime/FreeBSD/time32.c \
	stdtime/FreeBSD/timelocal.c \
 \
	string/FreeBSD/rpmatch.c \
	string/FreeBSD/memmem.c \
	string/FreeBSD/rindex.c \
	string/FreeBSD/strcasecmp.c \
	string/FreeBSD/strcasestr.c \
	string/FreeBSD/strcoll.c \
	string/FreeBSD/strcspn.c \
	string/FreeBSD/strdup.c \
	string/FreeBSD/strerror.c \
	string/FreeBSD/strmode.c \
	string/FreeBSD/strndup.c \
	string/FreeBSD/strnstr.c \
	string/FreeBSD/strpbrk.c \
	string/FreeBSD/strrchr.c \
	string/FreeBSD/strsep.c \
	string/FreeBSD/strsignal.c \
	string/FreeBSD/strspn.c \
	string/FreeBSD/strtok.c \
	string/FreeBSD/strxfrm.c \
	string/FreeBSD/swab.c \
	string/FreeBSD/wcpcpy.c \
	string/FreeBSD/wcpncpy.c \
	string/FreeBSD/wcscasecmp.c \
	string/FreeBSD/wcscat.c \
	string/FreeBSD/wcschr.c \
	string/FreeBSD/wcscmp.c \
	string/FreeBSD/wcscoll.c \
	string/FreeBSD/wcscpy.c \
	string/FreeBSD/wcscspn.c \
	string/FreeBSD/wcsdup.c \
	string/FreeBSD/wcslcat.c \
	string/FreeBSD/wcslcpy.c \
	string/FreeBSD/wcslen.c \
	string/FreeBSD/wcsncasecmp.c \
	string/FreeBSD/wcsncat.c \
	string/FreeBSD/wcsncmp.c \
	string/FreeBSD/wcsncpy.c \
	string/FreeBSD/wcsnlen.c \
	string/FreeBSD/wcspbrk.c \
	string/FreeBSD/wcsrchr.c \
	string/FreeBSD/wcsspn.c \
	string/FreeBSD/wcsstr.c \
	string/FreeBSD/wcstok.c \
	string/FreeBSD/wcswidth.c \
	string/FreeBSD/wcsxfrm.c \
	string/FreeBSD/wmemchr.c \
	string/FreeBSD/wmemcmp.c \
	string/FreeBSD/wmemcpy.c \
	string/FreeBSD/wmemmove.c \
	string/FreeBSD/wmemset.c \
	string/FreeBSD/timingsafe_bcmp.c \
 \
	sys/OpenBSD/stack_protector.c \
 \
	stdlib/a64l.c \
	stdlib/FreeBSD/_Exit_.c \
	stdlib/FreeBSD/abort.c \
	stdlib/FreeBSD/abs.c \
	stdlib/FreeBSD/atexit.c \
	stdlib/FreeBSD/atof.c \
	stdlib/FreeBSD/atoi.c \
	stdlib/FreeBSD/atol.c \
	stdlib/FreeBSD/atoll.c \
	stdlib/FreeBSD/bsearch.c \
	stdlib/FreeBSD/div.c \
	stdlib/FreeBSD/exit.c \
	stdlib/FreeBSD/getenv.c \
	stdlib/FreeBSD/getopt.c \
	stdlib/FreeBSD/getopt_long.c \
	stdlib/FreeBSD/getsubopt.c \
	stdlib/FreeBSD/hcreate.c \
	stdlib/FreeBSD/heapsort.c \
	stdlib/FreeBSD/imaxabs.c \
	stdlib/FreeBSD/imaxdiv.c \
	stdlib/FreeBSD/insque.c \
	stdlib/FreeBSD/labs.c \
	stdlib/FreeBSD/ldiv.c \
	stdlib/FreeBSD/llabs.c \
	stdlib/FreeBSD/lldiv.c \
	stdlib/FreeBSD/lsearch.c \
	stdlib/FreeBSD/merge.c \
	stdlib/FreeBSD/putenv.c \
	stdlib/FreeBSD/qsort.c \
	stdlib/FreeBSD/qsort_r.c \
	stdlib/FreeBSD/radixsort.c \
	stdlib/FreeBSD/rand.c \
	stdlib/FreeBSD/random.c \
	stdlib/FreeBSD/reallocf.c \
	stdlib/FreeBSD/realpath.c \
	stdlib/FreeBSD/remque.c \
	stdlib/FreeBSD/setenv.c \
	stdlib/FreeBSD/strhash.c \
	stdlib/FreeBSD/strtoimax.c \
	stdlib/FreeBSD/strtol.c \
	stdlib/FreeBSD/strtoll.c \
	stdlib/FreeBSD/strtoq.c \
	stdlib/FreeBSD/strtoul.c \
	stdlib/FreeBSD/strtoull.c \
	stdlib/FreeBSD/strtoumax.c \
	stdlib/FreeBSD/strtouq.c \
	stdlib/FreeBSD/system.c \
	stdlib/FreeBSD/tdelete.c \
	stdlib/FreeBSD/tfind.c \
	stdlib/FreeBSD/tsearch.c \
	stdlib/FreeBSD/twalk.c \
	stdlib/grantpt.c \
	stdlib/l64a.c \
	stdlib/OpenBSD/ecvt.c \
	stdlib/OpenBSD/gcvt.c \
	stdlib/qsort_b.c \
	stdlib/FreeBSD/heapsort_b.c \
	stdlib/FreeBSD/heapsort_r.c \
	stdlib/FreeBSD/merge_b.c \
	stdlib/FreeBSD/psort.c \
	stdlib/FreeBSD/psort_b.c \
	stdlib/FreeBSD/psort_r.c 

.if "${CpuArch}" == "x86_64"
SRCS += gdtoa/FreeBSD/gdtoa-strtopx.c gdtoa/FreeBSD/machdep_ldisx.c
.else if "${CpuArch}" == "arm64"
SRCS += gdtoa/FreeBSD/machdep_ldisd.c
.endif

SLF = /System/Library/Frameworks
CFLAGS += -DHAVE_STDINT_H -D__isthreaded -DPRIVATE -I${.CURDIR:H}/include \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${.CURDIR:H}/fbsdcompat \
	-I${.CURDIR:H}/gdtoa \
	-I${.CURDIR:H}/gdtoa/FreeBSD \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/../private \
	-I${.CURDIR:H}/../libsystem_info/membership.subproj \
	-I${.CURDIR:H}/../libsystem_info/lookup.subproj \
	-I${.CURDIR:H}/../libsystem_info/gen.subproj \
	-I${.CURDIR:H}/../libsystem_notify \
	-I${.CURDIR:H}/../libsystem_blocks \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
        -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc \
	-I${SDK_SOURCE_DIR}/usr/include \
	-I${SDK_SOURCE_DIR}/usr/include -I${SDK_SOURCE_DIR}/usr/include/arpa \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS \
	-Wimplicit-fallthrough -Wno-unknown-pragmas -Wno-assume \
	-Wformat-nonliteral -Wno-vla-extension -Wundef-prefix=SUPPORT_ \
	-include${.CURDIR}/../fbsdcompat/_fbsd_compat_.h \
	-include${SDK_SOURCE_DIR}/usr/include/os/availability.h \
	${CFLAGS.${.TARGET:T}}

CFLAGS.creat.o = -D__FBSDID=__RCSID
CFLAGS.sigcompat.o = -DLIBC_ALIAS_SIGPAUSE
CFLAGS.bt_close.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_conv.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_debug.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_delete.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_get.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_open.o = -D__DBINTERFACE_PRIVATE -UDEBUG
CFLAGS.bt_overflow.o = -D__DBINTERFACE_PRIVATE -UDEBUG
CFLAGS.bt_page.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_put.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_search.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_seq.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_split.o = -D__DBINTERFACE_PRIVATE
CFLAGS.bt_utils.o = -D__DBINTERFACE_PRIVATE
CFLAGS.db.o = -D__DBINTERFACE_PRIVATE
CFLAGS.hash.o = -D__DBINTERFACE_PRIVATE -UDEBUG
CFLAGS.hash_bigkey.o = -D__DBINTERFACE_PRIVATE -UDEBUG
CFLAGS.hash_buf.o = -D__DBINTERFACE_PRIVATE -UDEBUG
CFLAGS.hash_func.o = -D__DBINTERFACE_PRIVATE
CFLAGS.hash_log2.o = -D__DBINTERFACE_PRIVATE
CFLAGS.hash_page.o = -D__DBINTERFACE_PRIVATE -UDEBUG
CFLAGS.ndbm.o = -D__DBINTERFACE_PRIVATE
CFLAGS.mpool.o = -D__DBINTERFACE_PRIVATE -UDEBUG
CFLAGS.rec_close.o = -D__DBINTERFACE_PRIVATE
CFLAGS.rec_delete.o = -D__DBINTERFACE_PRIVATE
CFLAGS.rec_get.o = -D__DBINTERFACE_PRIVATE
CFLAGS.rec_open.o = -D__DBINTERFACE_PRIVATE -D_DARWIN_UNLIMITED_STREAMS
CFLAGS.rec_put.o = -D__DBINTERFACE_PRIVATE
CFLAGS.rec_search.o = -D__DBINTERFACE_PRIVATE
CFLAGS.rec_seq.o = -D__DBINTERFACE_PRIVATE
CFLAGS.rec_utils.o = -D__DBINTERFACE_PRIVATE
CFLAGS.closedir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.opendir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.readdir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.rewinddir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.scandir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.seekdir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.telldir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.termios.o = -DLIBC_ALIAS_TCDRAIN
CFLAGS.collate.o = -D_DARWIN_UNLIMITED_STREAMS
CFLAGS.setrunelocale.o = -D_DARWIN_UNLIMITED_STREAMS
CFLAGS.stack_protector.o = -fno-stack-protector
CFLAGS.realpath.o = -DLIBC_ALIAS_REALPATH
CFLAGS.psort_b.o = -DI_AM_PSORT_B
CFLAGS.psort_r.o = -DI_AM_PSORT_R
CFLAGS.xprintf_errno.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf_float.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf_hexdump.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf_int.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf_quote.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf_str.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf_time.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf_vis.o = -I${.CURDIR:H}/stdio
CFLAGS.xprintf.o = -I${.CURDIR:H}/stdio
CFLAGS.arc4random.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
CFLAGS.vfprintf.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
CFLAGS.psort.o = -I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
CFLAGS.psort_b.o = -DI_AM_PSORT_B \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders
CFLAGS.psort_r.o = -DI_AM_PSORT_R \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders

.include <bsd.lib.mk>

