# libc-specific portion of the Mach/Darwin syscall interface

.PATH: ${LIBC_SRCTOP}/compat-darwin

# emit empty assembly stubs for syscalls in dynamic libc
SHARED_CFLAGS+=	-D'_SYSCALL_BODY(name)='

MAPFILE=	${LIBC_SRCTOP}/compat-darwin/Symbol.map
SYM_MAPS+=	${MAPFILE}

SYMBOLS!=	tr '\n' ' ' < ${MAPFILE} | \
		sed -e 's/^ravynOS_.*{\(.*\)}.*FBSD.*$$/\1/' | \
		sed -e 's/_NS.* //g' | \
		tr ';' ' '

.for sym in ${SYMBOLS:S/\;//g}
${sym}.S: ${MAPFILE}
	printf "#include <SYS.h>\n" > ${.TARGET}
	printf ".global ${sym}\n${sym}:\n" >> ${.TARGET}
	printf "\t_SYSCALL(${sym})\n\tret\n" >> ${.TARGET}

SRCS+=	${sym}.S
.endfor

