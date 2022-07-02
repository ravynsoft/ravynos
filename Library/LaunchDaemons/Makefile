SYSLIB= /System/Library/LaunchDaemons
LIBLIB= /Library/LaunchDaemons

# Removed auditd for now - they seem to leak memory
# or something and eventually crash the system
#SYSDAEMONS= 	

SYSDAEMONS=	com.apple.auditd.json \
		com.apple.notifyd.json \
		com.apple.syslogd.json \
		org.freebsd.devd.json \
		org.freebsd.tty1.json \
		org.freebsd.tty2.json \
		com.ravynos.WindowServer.json
DAEMONS=

all: clean build
build:
	mkdir -pv ${BUILDROOT}${SYSLIB} ${BUILDROOT}${LIBLIB}
.for daemon in ${SYSDAEMONS}
	${INSTALL} -m 0644 ${daemon} ${BUILDROOT}${SYSLIB}
.endfor
.for daemon in ${DAEMONS}
	${INSTALL} -m 0644 ${daemon} ${BUILDROOT}${LIBLIB}
.endfor

clean:
	rm -rf ${BUILDROOT}${SYSLIB} ${BUILDROOT}${LIBLIB}

