#! /bin/bash -e -x

if [[ "${PLATFORM_NAME}" =~ "simulator" ]] ; then
    PLIST="${SRCROOT}"/syslogd.tproj/com.apple.syslogd_sim.plist
    ASL_CONF="${SRCROOT}"/syslogd.tproj/asl.conf.ios_sim
elif [[ "${PLATFORM_NAME}" == "macosx" ]] ; then
    PLIST="${SRCROOT}"/syslogd.tproj/com.apple.syslogd.plist
    ASL_CONF="${SRCROOT}"/syslogd.tproj/asl.conf.osx
    SYSLOG_CONF="${SRCROOT}"/syslogd.tproj/syslog.conf
elif [[ "${PLATFORM_NAME}" == "bridgeos" ]] ; then
    PLIST="${SRCROOT}"/syslogd.tproj/com.apple.syslogd.disabled.plist
    ASL_CONF="${SRCROOT}"/syslogd.tproj/asl.conf.ios
    SYSLOG_CONF="${SRCROOT}"/syslogd.tproj/syslog.conf
    SYSTEM_LOG_CONF="${SRCROOT}"/syslogd.tproj/com.apple.system.log.bridgeos
else
    PLIST="${SRCROOT}"/syslogd.tproj/com.apple.syslogd.disabled.plist
    ASL_CONF="${SRCROOT}"/syslogd.tproj/asl.conf.ios
    SYSLOG_CONF="${SRCROOT}"/syslogd.tproj/syslog.conf
    SYSTEM_LOG_CONF="${SRCROOT}"/syslogd.tproj/com.apple.system.log
fi

DESTDIR="${DSTROOT}"/private/etc
install -d -m 0755 -o root -g wheel "${DESTDIR}"
install -m 0644 -o root -g wheel "${ASL_CONF}" "${DESTDIR}"/asl.conf
if [[ -n "${SYSLOG_CONF}" ]] ; then
    install -m 0644 -o root -g wheel "${SYSLOG_CONF}" "${DESTDIR}"
fi

DESTDIR="${DSTROOT}"/System/Library/LaunchDaemons
install -d -m 0755 -o root -g wheel "${DESTDIR}"
install -m 0644 -o root -g wheel "${PLIST}" "${DESTDIR}"/com.apple.syslogd.plist
plutil -convert binary1 "${DESTDIR}"/com.apple.syslogd.plist

if [[ "${PLATFORM_NAME}" =~ "simulator" ]] ; then
    exit 0
fi

install -d -m 0755 -o root -g wheel "$DSTROOT"/private/var/log/asl

if [[ "${PLATFORM_NAME}" != "macosx" ]]; then
    install -d -m 0755 -o root -g wheel "$DSTROOT"/usr/share/sandbox
    install -m 0644 -o root -g wheel "$SRCROOT"/syslogd.tproj/syslogd.sb "$DSTROOT"/usr/share/sandbox
fi

if ! [[ "${PLATFORM_NAME}" =~ "simulator" || "${PLATFORM_NAME}" == "macosx" ]]; then
	DESTDIR="${DSTROOT}"/usr/local/etc/asl
	install -d -m 0755 -o root -g wheel "${DESTDIR}"
	install -m 0644 -o root -g wheel "${SYSTEM_LOG_CONF}" "${DESTDIR}"/com.apple.system.log
fi
