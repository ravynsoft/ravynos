.include <src.opts.mk>
.include <src.tools.mk>
.include <bsd.compat.pre.mk>

RAVYNOS_VERSION != sed -e '1q;d' ${.CURDIR}/../version.txt
RAVYNOS_CODENAME != sed -e '2q;d' ${.CURDIR}/../version.txt
FILES+= ${.CURDIR}/bootstrap
FILESDIR= /etc
FILESMODE= 0755


FILESGROUPS=	FILES
NLS_ALIASES=	POSIX C \
		en_US.US_ASCII C

# No need as it is empty and just causes rebuilds since this file does so much.
UPDATE_DEPENDFILE=	no

SUBDIR+=termcap
.if ${MK_SENDMAIL} != "no"
SUBDIR+=sendmail
.endif

# NB: keep these sorted by MK_* knobs

ETCMAIL=aliases
.if ${MK_SENDMAIL} != "no"
ETCMAIL_SENDMAIL+=Makefile README access.sample virtusertable.sample \
	mailertable.sample
.endif

# Special top level files for FreeBSD
FREEBSD=COPYRIGHT

# Sanitize DESTDIR
DESTDIR:=	${DESTDIR:C://*:/:g}

afterinstall:
.if ${MK_MAN} != "no"
	${_+_}cd ${SRCTOP}/share/man; ${MAKE} makedb
.endif

distribute:
	# Avoid installing tests here; "make distribution" will do this and
	# correctly place them in the right location.
	${_+_}cd ${.CURDIR} ; ${MAKE} MK_TESTS=no install \
	    DISTBASE=/${DISTRIBUTION} DESTDIR=${DISTDIR}/${DISTRIBUTION}
	${_+_}cd ${.CURDIR} ; ${MAKE} distribution \
	    DISTBASE=/${DISTRIBUTION} DESTDIR=${DISTDIR}/${DISTRIBUTION}

.include <bsd.endian.mk>

.if defined(NO_ROOT)
METALOG.add?=	cat -l >> ${METALOG}
.endif

distribution:
.if !defined(DESTDIR)
	@echo "set DESTDIR before running \"make ${.TARGET}\""
	@false
.endif
.if ${MK_MITKRB5} != "no"
	${_+_}cd ${.CURDIR}/gss-krb5; ${MAKE} install
.else
	${_+_}cd ${.CURDIR}/gss; ${MAKE} install
.endif
	${_+_}cd ${.CURDIR}/mtree; ${MAKE} install
	${_+_}cd ${SRCTOP}/share/termcap; ${MAKE} etc-termcap
	${_+_}cd ${SRCTOP}/usr.sbin/rmt; ${MAKE} etc-rmt
	${INSTALL_SYMLINK} -T "package=runtime" ../var/run/os-release \
		${DESTDIR}/etc/os-release
	${INSTALL_SYMLINK} -T "package=runtime" ../var/db/machine-id \
		${DESTDIR}/etc/machine-id
.for f in adduser.conf launchd_user.env motd.template pci.ids shells zprofile
	${INSTALL} -m 0444 -o root -g wheel ${.CURDIR}/$f ${DESTDIR}/etc/
.endfor
	sed -i_ -e "s/__VERSION__/${RAVYNOS_VERSION}/" \
		-e "s/__CODENAME__/${RAVYNOS_CODENAME}/" ${DESTDIR}/etc/motd.template
	rm -f ${DESTDIR}/etc/motd.template_
	${INSTALL} -m 0555 -o root -g wheel ${.CURDIR}/bootstrap ${DESTDIR}/etc/
.if ${MK_UNBOUND} != "no"
	if [ ! -e ${DESTDIR}/etc/unbound ]; then \
		${INSTALL_SYMLINK} -T "package=unbound" \
		../var/unbound ${DESTDIR}/etc/unbound; \
	fi
.endif
.if ${MK_SENDMAIL} != "no"
	${_+_}cd ${.CURDIR}/sendmail; ${MAKE} distribution
.endif
.if ${MK_KERBEROS} != "no"
	cd ${.CURDIR}/root; \
	    ${INSTALL} -o ${BINOWN} -g ${BINGRP} -m 644 \
		-T "package=runtime,config" \
		dot.k5login ${DESTDIR}/root/.k5login;
.endif

.if ${MK_MAIL} != "no"
	cd ${.CURDIR}/mail; ${INSTALL} -o ${BINOWN} -g ${BINGRP} -m 644 \
	    -T "package=runtime,config" ${ETCMAIL} ${DESTDIR}/etc/mail
	if [ -d ${DESTDIR}/etc/mail -a -f ${DESTDIR}/etc/mail/aliases -a \
	      ! -f ${DESTDIR}/etc/aliases ]; then \
		${INSTALL_SYMLINK} -T "package=runtime" \
		mail/aliases ${DESTDIR}/etc/aliases; \
	fi
.endif
.if ${MK_SENDMAIL} != "no"
	cd ${.CURDIR}/mail; ${INSTALL} -o ${BINOWN} -g ${BINGRP} -m 644 \
		-T "package=sendmail" ${ETCMAIL_SENDMAIL} ${DESTDIR}/etc/mail
.endif
	cd ${.CURDIR}/..; ${INSTALL} -o ${BINOWN} -g ${BINGRP} -m 444 \
		-T "package=runtime" ${FREEBSD} ${DESTDIR}/
.if ${MK_BOOT} != "no"
.if exists(${SRCTOP}/sys/${MACHINE}/conf/GENERIC.hints)
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m 444 \
	    -T "package=bootloader,config" \
	    ${SRCTOP}/sys/${MACHINE}/conf/GENERIC.hints \
	    ${DESTDIR}/boot/device.hints
.endif
.endif

MTREES=		mtree/BSD.root.dist		/		\
		mtree/BSD.var.dist		/var		\
		mtree/BSD.usr.dist		/usr		\
		mtree/BSD.include.dist		/usr/include	\
		mtree/BSD.debug.dist		/usr/lib
.for LIBCOMPAT libcompat in ${_ALL_LIBCOMPATS_libcompats}
.if ${MK_LIB${LIBCOMPAT}} != "no"
MTREES+=	mtree/BSD.lib${libcompat}.dist	/usr
MTREES+=	mtree/BSD.lib${libcompat}.dist	/usr/lib/debug/usr
.endif
.endfor
.if ${MK_TESTS} != "no"
MTREES+=	mtree/BSD.tests.dist		${TESTSBASE}
MTREES+=	mtree/BSD.tests.dist		/usr/lib/debug${TESTSBASE}
.endif
.if ${MK_SENDMAIL} != "no"
MTREES+=	mtree/BSD.sendmail.dist		/
.endif
.for mtree in ${LOCAL_MTREE}
MTREES+=	../${mtree}			/
.endfor

# Clean up files that have changed into directories, as mtree cannot handle this
# scenario.
DISTRIB_CLEANUP_FILES+=	${INCLUDEDIR}/c++/v1/__string
DISTRIB_CLEANUP_FILES+=	${INCLUDEDIR}/c++/v1/__tuple
DISTRIB_CLEANUP_FILES+=	${LIBEXECDIR}/kgdb
distrib-cleanup: .PHONY
	for file in ${DISTRIB_CLEANUP_FILES}; do \
		if [ -f ${DESTDIR}/$${file} ]; then \
			rm -f ${DESTDIR}/$${file}; \
		fi; \
	done

distrib-dirs: ${MTREES:N/*} distrib-cleanup .PHONY
.for _m _d in ${MTREES}
	@m=${.CURDIR}/${_m}; \
	d=${DESTDIR}${_d}; \
	test -d $$d || mkdir -p $$d; \
	${ECHO} ${MTREE_CMD} -deU ${MTREE_FSCHG} \
	    ${MTREE_FOLLOWS_SYMLINKS} -f $$m -p $$d; \
	${MTREE_FILTER} $$m | \
	${MTREE_CMD} -deU ${MTREE_FSCHG} ${MTREE_FOLLOWS_SYMLINKS} \
	    -p $$d
.endfor
.if defined(NO_ROOT)
.for _m _d in ${MTREES}
	@m=${.CURDIR}/${_m}; \
	d=${_d}; \
	test "$$d" = "/" && d=""; \
	d=${DISTBASE}$$d; \
	${ECHO} "${MTREE_CMD:N-W} -C -f $$m -K all | " \
	    "sed s#^\.#.$$d# | ${METALOG.add}" ; \
	${MTREE_FILTER} $$m | \
	${MTREE_CMD:N-W} -C -K all | sed s#^\.#.$$d# | \
	    ${METALOG.add}
.endfor
.endif
.if ${MK_NLS} != "no"
.for alias nls in ${NLS_ALIASES}
	${INSTALL_SYMLINK} -T "package=utilities" \
	    "${nls}" "${DESTDIR}${SHAREDIR}/nls/${alias}"
.endfor
.endif

.include <bsd.prog.mk>

.if ${MK_INSTALL_AS_USER} == "yes" && ${_uid} != 0
MTREE_FILTER= sed -e 's,\([gu]\)name=,\1id=,g' \
	-e 's,\(uid=\)[^ ]* ,\1${_uid} ,' \
	-e 's,\(gid=\)[^ ]* ,\1${_gid} ,' \
	-e 's,\(uid=\)[^ ]*$$,\1${_uid},' \
	-e 's,\(gid=\)[^ ]*$$,\1${_gid},'
.else
MTREE_FILTER= cat
.if !defined(NO_FSCHG)
MTREE_FSCHG=	-i
.endif
.endif
