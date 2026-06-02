.PATH: ${.CURDIR:H}
MK_WERROR = no
LIB = vInode32
SRCS = gen/fts.c \
	gen/FreeBSD/getmntinfo.c \
	gen/FreeBSD/glob.c \
	gen/nftw.c \
	gen/FreeBSD/opendir.c \
	gen/FreeBSD/readdir.c \
	gen/FreeBSD/rewinddir.c \
	gen/FreeBSD/scandir.c \
	gen/FreeBSD/seekdir.c \
	gen/FreeBSD/telldir.c \
	gen/FreeBSD/scandir_b.c \
	sys/statx_np.c

CFLAGS += -DPRIVATE -U__DARWIN_64_BIT_INO_T -D__DARWIN_64_BIT_INO_T=0 \
	-DBUILDING_VARIANT -DVARIANT_INODE32 -D__isthreaded \
	-I${.CURDIR:H}/include -I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach \
	-I${ROOT_BINARY_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.CURDIR:H}/../private -I${.CURDIR:H}/fbsdcompat \
	-include${.CURDIR:H}/fbsdcompat/_fbsd_compat_.h \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/dirent.h \
	${CFLAGS.${.TARGET:T}}

CFLAGS.glob.o = -DLIBC_ALIAS_GLOB
CFLAGS.opendir.o = -DLIBC_ALIAS___OPENDIR2 -DLIBC_ALIAS_OPENDIR \
	-include${.CURDIR:H}/gen/__dirent.h
CFLAGS.readdir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.rewinddir.o = -DLIBC_ALIAS_REWINDDIR -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.scandir.o = -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.seekdir.o = -DLIBC_ALIAS_SEEKDIR -include${.CURDIR:H}/gen/__dirent.h
CFLAGS.telldir.o = -DLIBC_ALIAS__SEEKDIR -DLIBC_ALIAS_TELLDIR \
	-include${.CURDIR:H}/gen/__dirent.h
CFLAGS.fts.o = -DLIBC_ALIAS_FTS_CHILDREN -DLIBC_ALIAS_FTS_CLOSE \
	-DLIBC_ALIAS_FTS_OPEN -DLIBC_ALIAS_FTS_OPEN_B \
	-DLIBC_ALIAS_FTS_READ -DLIBC_ALIAS_FTS_SET
CFLAGS.nftw.o = -DLIBC_ALIAS_FTW -DLIBC_ALIAS_NFTW
CFLAGS.scandir_b.o = -include${.CURDIR:H}/gen/__dirent.h

.include <bsd.lib.mk>
