
.include "${SRCTOP}/lib/clang/llvm.pre.mk"

CFLAGS+=	-I${OBJTOP}/lib/clang/libllvm

.include "${SRCTOP}/lib/clang/llvm.build.mk"

# Special case for the bootstrap-tools phase.
.if (defined(TOOLS_PREFIX) || ${MACHINE} == "host") && \
    (${PROG_CXX} == "clang-tblgen" || ${PROG_CXX} == "lldb-tblgen" || \
     ${PROG_CXX} == "llvm-min-tblgen" || ${PROG_CXX} == "llvm-tblgen")
LIBDEPS+=	llvmminimal
.else
LIBDEPS+=	llvm
LIBADD+=	z
LIBADD+=	zstd
DPADD+=		${OBJTOP}/lib/libmach/libmach.so
LDADD+=		${OBJTOP}/lib/libmach/libmach.so
.endif

.for lib in ${LIBDEPS}
DPADD+=		${OBJTOP}/lib/clang/lib${lib}/lib${lib}.a
LDADD+=		${OBJTOP}/lib/clang/lib${lib}/lib${lib}.a
.endfor

PACKAGE?=	clang

.if ${.MAKE.OS} == "FreeBSD" || !defined(BOOTSTRAPPING)
LIBADD+=	execinfo
LIBADD+=	tinfow
.endif
LIBADD+=	pthread

NO_MACH= yes

.include <bsd.prog.mk>
