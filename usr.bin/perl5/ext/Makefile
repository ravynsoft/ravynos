.include <src.opts.mk>

MODULES= \
	Errno \
	ExtUtils-Miniperl \
	File-Find \
	File-Glob \
	FileCache \
	IPC-Open3 \
	Pod-Functions \
	Pod-Html \
	Tie-Hash-NamedCapture \
	Tie-Memoize \
	File-Glob \
	Fcntl \
	POSIX \
	re

all: build
build: 
.for module in ${MODULES}
	if [ -d ${.CURDIR}/${module}/lib ]; then \
		cp -Rv ${.CURDIR}/${module}/lib/* ${PERL5LIB}/; \
		if [ -f ${.CURDIR}/${module}/*.pm ]; then \
			cp -fv ${.CURDIR}/${module}/*.pm ${PERL5LIB}/; \
		fi; \
	else \
		mkdir -pv ${PERL5LIB}/${module:S/-/\//g}; \
		cp -Rv ${.CURDIR}/${module}/* ${PERL5LIB}/${module:S/-/\//g}/..; \
	fi
.endfor

install: .PHONY
includes: .PHONY
