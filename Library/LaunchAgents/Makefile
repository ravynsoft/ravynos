SYSLIB= /System/Library/LaunchAgents
LIBLIB= /Library/LaunchAgents

SYSAGENTS=	com.ravynos.Dock.json \
		com.ravynos.Filer.json
AGENTS=

all: clean build
build:
	mkdir -pv ${BUILDROOT}${SYSLIB} ${BUILDROOT}${LIBLIB}
.for agent in ${SYSAGENTS}
	${INSTALL} -m 0644 ${agent} ${BUILDROOT}${SYSLIB}
.endfor
.for agent in ${AGENTS}
	${INSTALL} -m 0644 ${agent} ${BUILDROOT}${LIBLIB}
.endfor

clean:
	rm -rf ${BUILDROOT}${SYSLIB} ${BUILDROOT}${LIBLIB}

