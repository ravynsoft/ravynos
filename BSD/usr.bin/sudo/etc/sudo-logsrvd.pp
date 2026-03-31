%set
	name="sudo-logsrvd"
	summary="Sudo event and I/O log server"
	description="The sudo_logsrvd daemon collects event and I/O logs \
from sudo clients.
This makes it possible to have all sudo I/O logs on a central server."
	vendor="Todd C. Miller"
	copyright="(c) 2019-2021 Todd C. Miller"

%if [aix]
	# Convert to 4 part version for AIX, including patch level
	pp_aix_version=`echo $version|sed -e 's/^\([0-9]*\.[0-9]*\.[0-9]*\)p\([0-9]*\)$/\1.\2/' -e 's/^\([0-9]*\.[0-9]*\.[0-9]*\)[^0-9\.].*$/\1/' -e 's/^\([0-9]*\.[0-9]*\.[0-9]*\)$/\1.0/'`

	# Don't use sudo to list the package.
	pp_aix_sudo=
%endif

%if [sd]
	pp_sd_vendor_tag="TCM"
%endif

%if [solaris]
	pp_solaris_name="TCM${name}"
	pp_solaris_pstamp=`/usr/bin/date "+%B %d, %Y"`
%endif

%if [macos]
	# System Integrity Protection on macOS won't allow us to write
	# directly to /etc or /var.  We must install in /private instead.
	case "$sysconfdir" in
	/etc|/etc/*)
	    mkdir -p ${pp_destdir}/private
	    chmod 755 ${pp_destdir}/private
	    if test -d ${pp_destdir}/etc; then
		mv ${pp_destdir}/etc ${pp_destdir}/private/etc
	    fi
	    sysconfdir="/private${sysconfdir}"
	    ;;
	esac
	case "$vardir" in
	/var|/var/*)
	    mkdir -p ${pp_destdir}/private
	    chmod 755 ${pp_destdir}/private
	    if test -d ${pp_destdir}/var; then
		mv ${pp_destdir}/var ${pp_destdir}/private/var
	    fi
	    vardir="/private${vardir}"
	    ;;
	esac
	case "$rundir" in
	/var|/var/*)
	    mkdir -p ${pp_destdir}/private
	    chmod 755 ${pp_destdir}/private
	    if test -d ${pp_destdir}/var; then
		mv ${pp_destdir}/var ${pp_destdir}/private/var
	    fi
	    rundir="/private${rundir}"
	    ;;
	esac
%endif

%if [rpm,deb]
	# Convert patch level into release and remove from version
	pp_rpm_release="`expr \( $version : '.*p\([0-9][0-9]*\)$' \| 0 \) + 1`"
	pp_rpm_version="`expr \( $version : '\(.*\)p[0-9][0-9]*$' \| $version \)`"
	pp_rpm_license="BSD"
	pp_rpm_url="https://www.sudo.ws"
	pp_rpm_group="Applications/System"
	pp_rpm_packager="Todd C. Miller <Todd.Miller@sudo.ws>"
%else
	# We install sudo_logsrvd.conf from the example dir during post-install
	rm -f ${pp_destdir}$sysconfdir/sudo_logsrvd.conf
%endif

	# Stash original docdir and exampledir
	odocdir="${docdir}"
	oexampledir="${exampledir}"

	# docdir and exampledir are installed with "sudo" as the package
	# name which may not be correct.
	docdir="`echo \"${docdir}\" | sed 's#/sudo$#/'\"${name}\"'#'`"
	if test "${exampledir}" = "${odocdir}/examples"; then
	    exampledir="${docdir}/examples"
	else
	    exampledir="`echo \"${exampledir}\" | sed 's#/sudo$#/'\"${name}\"'#'`"
	fi

	# For RedHat the doc dir is expected to include version and release
	case "$pp_rpm_distro" in
	centos*|rhel*|f[0-9]*)
		docdir="${docdir}-${pp_rpm_version}-${pp_rpm_release}"
		exampledir="${docdir}/examples"
		;;
	esac

	# Copy docdir and exampledir to new names if needed
	if test ! -d "${pp_destdir}${docdir}"; then
	    cp -R ${pp_destdir}${odocdir} ${pp_destdir}${docdir}
	    find ${pp_destdir}${docdir} -depth | sed "s#^${pp_destdir}##" >> ${pp_wrkdir}/pp_cleanup
	fi
	if test ! -d "${pp_destdir}${exampledir}"; then
	    cp -R ${pp_destdir}${oexampledir} ${pp_destdir}${exampledir}
	    find ${pp_destdir}${exampledir} -depth | sed "s#^${pp_destdir}##" >> ${pp_wrkdir}/pp_cleanup
	fi

%if [deb]
	pp_deb_maintainer="$pp_rpm_packager"
	pp_deb_release="$pp_rpm_release"
	pp_deb_version="$pp_rpm_version"
	pp_deb_section=admin
	install -D -m 644 ${pp_destdir}$docdir/LICENSE.md ${pp_wrkdir}/${name}/usr/share/doc/${name}/copyright
	install -D -m 644 ${pp_destdir}$docdir/ChangeLog ${pp_wrkdir}/${name}/usr/share/doc/${name}/changelog
	gzip -9f ${pp_wrkdir}/${name}/usr/share/doc/${name}/changelog
	printf "$name ($pp_deb_version-$pp_deb_release) admin; urgency=low\n\n  * see upstream changelog\n\n -- $pp_deb_maintainer  `date '+%a, %d %b %Y %T %z'`\n" > ${pp_wrkdir}/${name}/usr/share/doc/${name}/changelog.Debian
	chmod 644 ${pp_wrkdir}/${name}/usr/share/doc/${name}/changelog.Debian
	gzip -9f ${pp_wrkdir}/${name}/usr/share/doc/${name}/changelog.Debian
	# Create lintian override file
	mkdir -p ${pp_wrkdir}/${name}/usr/share/lintian/overrides
	cat >${pp_wrkdir}/${name}/usr/share/lintian/overrides/${name} <<-EOF
	# Sudo ships with debugging symbols
	$name: unstripped-binary-or-object
	EOF
	chmod 644 ${pp_wrkdir}/${name}/usr/share/lintian/overrides/${name}
	# If libssl_dep not passed in, try to figure it out
	if test -z "$libssl_dep"; then
	    libssl_dep="`ldd $libexecdir/sudo/sudoers.so 2>&1 | sed -n 's/^[ 	]*libssl\.so\([0-9.]*\).*/libssl\1/p'`"
	fi
%endif

%if [rpm]
	# Add distro info to release
	osrelease=`echo "$pp_rpm_distro" | sed -e 's/^[^0-9]*\([0-9]\{1,2\}\).*/\1/'`
	case "$pp_rpm_distro" in
	centos*|rhel*|f[0-9]*)
		# CentOS Stream has a single-digit version
		if test $osrelease -lt 10; then
		    osrelease="${osrelease}0"
		fi
		pp_rpm_release="$pp_rpm_release.el${osrelease%%[0-9]}"
		;;
	sles*)
		pp_rpm_release="$pp_rpm_release.sles$osrelease"
		;;
	esac
%endif

%if [macos]
	pp_macos_pkg_type=flat
	pp_macos_bundle_id=ws.sudo.pkg.sudo-logsrvd
	pp_macos_pkg_background=${srcdir}/etc/macos-background.png
	pp_macos_pkg_background_dark=${srcdir}/etc/macos-background.png
	pp_macos_pkg_license=${pp_destdir}$docdir/LICENSE.md
	pp_macos_pkg_readme=${pp_wrkdir}/ReadMe.txt
	perl -pe 'last if (/^What/i && $seen++)' ${pp_destdir}$docdir/NEWS > ${pp_wrkdir}/ReadMe.txt
%endif

%if X"$aix_freeware" = X"true"
	# Create links from /opt/freeware/sbin -> /usr/sbin
	mkdir -p ${pp_destdir}/usr/sbin
	ln -s -f ${sbindir}/sudo_logsrvd ${pp_destdir}/usr/sbin
%endif

%if [!rpm,deb]
	# Package parent directories when not installing under /usr
	if test "${prefix}" != "/usr"; then
	    extradirs=`echo ${pp_destdir}${mandir}/[mc]* | sed "s#${pp_destdir}##g"`
	    extradirs="$extradirs `dirname $docdir` `dirname $rundir`"
	    test "`dirname $exampledir`" != "$docdir" && extradirs="$extradirs `dirname $exampledir`"
	    for dir in $sbindir $extradirs; do
		    while test "$dir" != "/"; do
			    parentdirs="${parentdirs}${parentdirs+ }$dir/"
			    dir=`dirname $dir`
		    done
	    done
	    parentdirs=`echo $parentdirs | tr " " "\n" | sort -u`
	fi
%endif

%depend [deb]
	libc6, zlib1g, sudo

%fixup [deb]
	if test -n "%{libssl_dep}"; then
	    DEPENDS="%{libssl_dep}"
	    cp -p %{pp_wrkdir}/%{name}/DEBIAN/control %{pp_wrkdir}/%{name}/DEBIAN/control.$$
	    sed "s/^\(Depends:.*\) *$/\1, ${DEPENDS}/" %{pp_wrkdir}/%{name}/DEBIAN/control.$$ > %{pp_wrkdir}/%{name}/DEBIAN/control
	    rm -f %{pp_wrkdir}/%{name}/DEBIAN/control.$$
	fi
	echo "Homepage: https://www.sudo.ws" >> %{pp_wrkdir}/%{name}/DEBIAN/control
	echo "Bugs: https://bugzilla.sudo.ws" >> %{pp_wrkdir}/%{name}/DEBIAN/control

%fixup [rpm]
	cat > %{pp_wrkdir}/${name}.spec.sed <<-'EOF'
		/^%files/ {
			i\
			%clean\
			:\

		}
	EOF
	mv %{pp_wrkdir}/${name}.spec %{pp_wrkdir}/${name}.spec.bak
	sed -f %{pp_wrkdir}/${name}.spec.sed %{pp_wrkdir}/${name}.spec.bak > %{pp_wrkdir}/${name}.spec

%files
	/**			ignore
%if X"$parentdirs" != X""
	$parentdirs		-    ignore-others
%endif
	$sbindir/sudo_logsrvd   0755 ignore-others
	$mandir/man*/*logsrv*	0644 ignore-others
	$rundir/		0711 root: ignore-others
	$docdir/		0755 ignore-others
	$exampledir/		0755 ignore-others
	$exampledir/*logsrv*	0644 ignore-others
%if [rpm,deb]
	$sysconfdir/sudo_logsrvd.conf 0644 root: volatile,ignore-others
%endif
%if X"$aix_freeware" = X"true"
	# Links for binaries from /opt/freeware to /usr
	/usr/sbin/sudo_logsrvd  0755 root: symlink,ignore-others $sbindir/logsrvd
%endif

%post [!rpm,deb]
	# Don't overwrite existing sudo_logsrvd.conf files
%if [solaris]
	sysconfdir=${PKG_INSTALL_ROOT}%{sysconfdir}
	exampledir=${PKG_INSTALL_ROOT}%{exampledir}
%else
	sysconfdir=%{sysconfdir}
	exampledir=%{exampledir}
%endif
	if test ! -r $sysconfdir/sudo_logsrvd.conf; then
		cp $exampledir/sudo_logsrvd.conf $sysconfdir/sudo_logsrvd.conf
		chmod 644 $sysconfdir/sudo_logsrvd.conf
		chown root $sysconfdir/sudo_logsrvd.conf
	fi

%service sudo_logsrvd
%if [aix,macos]
	cmd="${sbindir}/sudo_logsrvd -n"
%else
	cmd=${sbindir}/sudo_logsrvd
	pidfile=${rundir}/sudo_logsrvd.pid
%endif
%if [macos]
	pp_macos_service_id=ws.sudo.sudo_logsrvd
%endif
%if [rpm,deb]
	# Only include systemd support if it exists on the build machine.
	# This assumes that we are building on the same distro that the
	# package will be installed on (which is the case for sudo).
	if test -d /etc/systemd; then
	    for d in `pkg-config systemd --variable=systemdsystemunitdir 2>/dev/null` /lib/systemd/system /usr/lib/systemd/system; do
		if test -d "$d"; then
		    break
		fi
	    done
	    pp_systemd_service_description="Sudo central log server"
	    pp_systemd_service_dir="$d"
	    pp_systemd_service_exec="${cmd}"
	    pp_systemd_service_exec_args="-n"
	    pp_systemd_service_man="man:sudo_logsrvd(8) man:sudo_logsrvd.conf(5)"
	    pp_systemd_service_documentation="https://www.sudo.ws/man.html"
	    pp_systemd_service_after="syslog.target network.target auditd.service"
	    pp_systemd_service_killmode="process"
	    pp_systemd_service_type="exec"
	    pp_systemd_system_target="multi-user.target"
	else
	    # No systemd support
	    pp_systemd_disabled=true
	fi
%endif
