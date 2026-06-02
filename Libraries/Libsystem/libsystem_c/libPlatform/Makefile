.PATH: ${.CURDIR:H}
LIB = Platform
SRCS = darwin/forceLibcToBuild.c darwin/kvm.c

.if "${CpuArch}" == "x86_64"
SRCS += x86_64/gen/mcount.s
.endif

.include <bsd.lib.mk>
