#
# Copyright 1991-1998 by Open Software Foundation, Inc. 
#              All Rights Reserved 
#  
# Permission to use, copy, modify, and distribute this software and 
# its documentation for any purpose and without fee is hereby granted, 
# provided that the above copyright notice appears in all copies and 
# that both the copyright notice and this permission notice appear in 
# supporting documentation. 
#  
# OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE. 
#  
# IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
# LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
# NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
# 
# 
# cmk1.1

.include <src.opts.mk>
PROG= migcom
SCRIPTS=mig.sh
SRCS= ident.c error.c global.c header.c \
	lexxer.l mig.c parser.y routine.c server.c \
	statement.c string.c type.c user.c utils.c


CFLAGS+= -I${.CURDIR} -I${.CURDIR}/shims -I${.CURDIR}/../../include -Iinclude
CFLAGS+= -I${.CURDIR}/../../sys 
CFLAGS+= -DYY_NO_UNPUT

.if ${TARGET} == "amd64" || ${TARGET} == "i386"
include/machine: ${.CURDIR}/Makefile
	mkdir -p include
	ln -sf ${.CURDIR}/../../sys/x86/include include/machine
.endif

.if ${TARGET} == "aarch64" || ${TARGET} == "arm64"
include/machine: ${.CURDIR}/Makefile
	mkdir -p include
	ln -sf ${.CURDIR}/../../sys/arm64/include include/machine
.endif

.if ${TARGET} == "amd64" || ${TARGET} == "i386" || ${TARGET} == "aarch64" || ${TARGET} == "arm64"
ident.c: include/machine
.else
ident.c: 
.endif
	echo "#include \"alloc.h\" " >${.TARGET}
	echo "const char *MigGenerationDate = \""`date`"\";" >>${.TARGET}
	echo "const char *MigMoreData = \""`whoami`"@"`hostname`"\";" >>${.TARGET}


CLEANFILES+= ident.c *.o *~ *.gz cscope.* 
.if ${TARGET} == "amd64" || ${TARGET} == "i386" || ${TARGET} == "aarch64" || ${TARGET} == "arm64"
CLEANFILES+= include/machine
.endif

.include <bsd.prog.mk>
