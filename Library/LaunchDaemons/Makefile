SYSLIB= /System/Library/LaunchDaemons
LIBLIB= /Library/LaunchDaemons

# Removed auditd and syslogd (asld) for now - they seem to leak memory
# or something and eventually crash the system
#SYSDAEMONS= 	com.apple.auditd.json \
		com.apple.notifyd.json \
		com.apple.syslogd.json \
		org.freebsd.tty.json \
		org.freebsd.devd.json

SYSDAEMONS=	com.apple.notifyd.json \
		org.freebsd.devd.json \
		org.freebsd.tty.json \
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

