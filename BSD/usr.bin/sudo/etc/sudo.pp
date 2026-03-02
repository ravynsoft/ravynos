%set
	if test -n "$flavor"; then
		name="sudo-$flavor"
	else
		name="sudo"
	fi
	summary="Provide limited super-user privileges to specific users"
	description="Sudo is a program designed to allow a sysadmin to give \
limited root privileges to users and log root activity.  \
The basic philosophy is to give as few privileges as possible but \
still allow people to get their work done."
	vendor="Todd C. Miller"
	copyright="(c) 1993-1996,1998-2021 Todd C. Miller"
	sudoedit_man=`echo ${pp_destdir}$mandir/*/sudoedit.*|sed "s:^${pp_destdir}::"`
	sudoedit_man_target=`basename $sudoedit_man | sed 's/edit//'`

%if [aix]
	# AIX package summary is limited to 40 characters
	summary="Configurable super-user privileges"

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
	if test -n "$linux_audit"; then
		pp_rpm_requires="audit-libs >= $linux_audit"
	fi
	# The package manager will handle an existing sudoers file
	rm -f ${pp_destdir}${sysconfdir}/sudoers.dist
%else
	# For all but RPM and Debian we copy sudoers in a post-install script.
	rm -f ${pp_destdir}${sysconfdir}/sudoers
	# We install sudo.conf from the example dir in a post-install script.
	rm -f ${pp_destdir}${sysconfdir}/sudo.conf
%endif

	# Stash original docdir and exampledir
	odocdir="${docdir}"
	oexampledir="${exampledir}"

	# For RedHat the doc dir is expected to include version and release
	case "$pp_rpm_distro" in
	centos*|rhel*|f[0-9]*)
		docdir="${docdir}-${pp_rpm_version}-${pp_rpm_release}"
		exampledir="${docdir}/examples"
		;;
	esac

	if test -n "$flavor"; then
	    # docdir and exampledir are installed with "sudo" as the package
	    # name which is not be correct for flavors.
	    docdir="`echo \"${docdir}\" | sed \"s#/sudo#/${name}#g\"`"
	    exampledir="`echo \"${exampledir}\" | sed \"s#/sudo#/${name}#g\"`"
	fi

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
	# Create lintian override file, must be tab indented for "<<-"
	mkdir -p ${pp_wrkdir}/${name}/usr/share/lintian/overrides
	cat >${pp_wrkdir}/${name}/usr/share/lintian/overrides/${name} <<-EOF
	# The sudo binary must be setuid root
	$name: setuid-binary usr/bin/sudo 4755 root/root
	# Sudo configuration and data dirs must not be world-readable
	$name: non-standard-file-perm etc/sudoers 0440 != 0644
	$name: non-standard-dir-perm etc/sudoers.d/ 0750 != 0755
	$name: non-standard-dir-perm var/lib/sudo/ 0700 != 0755
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

	# Uncomment some Defaults in sudoers
	case "$pp_rpm_distro" in
	centos*|rhel*|f[0-9]*)
		sed -e '/Locale settings/{ N;s/\(\n\)# /\1/; }' -e '/Desktop path settings/{ N;s/\(\n\)# /\1/; }' -e '/allow members of group wheel to execute any command/{ N;s/\(\n\)# /\1/; }' ${pp_destdir}${sysconfdir}/sudoers > ${pp_destdir}${sysconfdir}/sudoers.$$
		mv -f ${pp_destdir}${sysconfdir}/sudoers.$$ ${pp_destdir}${sysconfdir}/sudoers
		;;
	sles*)
		sed -e '/Locale settings/{ N;s/\(\n\)# /\1/; }' -e '/ConsoleKit session/{ N;s/\(\n\)# /\1/; }' -e '/allow any user to run sudo if they know the password/{ N;N;N;s/\(\n\)# /\1/g; }' ${pp_destdir}${sysconfdir}/sudoers > ${pp_destdir}${sysconfdir}/sudoers.$$
		mv -f ${pp_destdir}${sysconfdir}/sudoers.$$ ${pp_destdir}${sysconfdir}/sudoers
		;;
	esac

	# Choose the correct PAM file by distro, must be tab indented for "<<-"
	case "$pp_rpm_distro" in
	centos*|rhel*)
		mkdir -p ${pp_destdir}/etc/pam.d
		if test $osrelease -lt 50; then
			cat > ${pp_destdir}/etc/pam.d/sudo <<-EOF
			#%PAM-1.0
			auth       required	pam_stack.so service=system-auth
			account    required	pam_stack.so service=system-auth
			password   required	pam_stack.so service=system-auth
			session    required	pam_limits.so
			EOF
		else
			cat > ${pp_destdir}/etc/pam.d/sudo <<-EOF
			#%PAM-1.0
			auth       include	system-auth
			account    include	system-auth
			password   include	system-auth
			session    optional	pam_keyinit.so revoke
			session    required	pam_limits.so
			EOF
			cat > ${pp_destdir}/etc/pam.d/sudo-i <<-EOF
			#%PAM-1.0
			auth       include	sudo
			account    include	sudo
			password   include	sudo
			session    optional	pam_keyinit.so force revoke
			session    required	pam_limits.so
			EOF
		fi
		;;
	f[0-9]*)
		# XXX - share with rhel
		mkdir -p ${pp_destdir}/etc/pam.d
		cat > ${pp_destdir}/etc/pam.d/sudo <<-EOF
		#%PAM-1.0
		auth       include	system-auth
		account    include	system-auth
		password   include	system-auth
		session    optional	pam_keyinit.so revoke
		session    required	pam_limits.so
		EOF
		cat > ${pp_destdir}/etc/pam.d/sudo-i <<-EOF
		#%PAM-1.0
		auth       include	sudo
		account    include	sudo
		password   include	sudo
		session    optional	pam_keyinit.so force revoke
		session    required	pam_limits.so
		EOF
		;;
	sles*)
		mkdir -p ${pp_destdir}/etc/pam.d
		if test $osrelease -lt 10; then
			cat > ${pp_destdir}/etc/pam.d/sudo <<-EOF
			#%PAM-1.0
			auth     required       pam_unix2.so
			session  required       pam_limits.so
			EOF
		else
			cat > ${pp_destdir}/etc/pam.d/sudo <<-EOF
			#%PAM-1.0
			auth     include	common-auth
			account  include	common-account
			password include	common-password
			session  include	common-session
			# session  optional	pam_xauth.so
			EOF
		fi
		;;
	esac
%endif

%if [deb]
	# Uncomment some Defaults and the %sudo rule in sudoers
	sed -e '/Locale settings/{ N;s/\(\n\)# /\1/; }' -e '/X11 resource/{ N;s/\(\n\)# /\1/; }' -e 's/^# \(Defaults secure_path\)/\1/' -e 's/^# \(Defaults mail_badpass\)/\1/' -e 's/^# \(\%sudo\)/\1/' ${pp_destdir}${sysconfdir}/sudoers > ${pp_destdir}${sysconfdir}/sudoers.$$
	mv -f ${pp_destdir}${sysconfdir}/sudoers.$$ ${pp_destdir}${sysconfdir}/sudoers
	mkdir -p ${pp_destdir}/etc/pam.d
	# Create Debian PAM file, must be tab indented for "<<-"
	cat > ${pp_destdir}/etc/pam.d/sudo <<-EOF
	#%PAM-1.0

	@include common-auth
	@include common-account

	session required pam_permit.so
	session required pam_limits.so
	EOF
%endif

%if [macos]
	pp_macos_pkg_type=flat
	pp_macos_bundle_id=ws.sudo.pkg.sudo
	pp_macos_pkg_background=${srcdir}/etc/macos-background.png
	pp_macos_pkg_background_dark=${srcdir}/etc/macos-background.png
	pp_macos_pkg_license=${pp_destdir}$docdir/LICENSE.md
	pp_macos_pkg_readme=${pp_wrkdir}/ReadMe.txt
	perl -pe 'last if (/^What/i && $seen++)' ${pp_destdir}$docdir/NEWS > ${pp_wrkdir}/ReadMe.txt
%endif

%if X"$aix_freeware" = X"true"
	# Create links from /opt/freeware/{bin,sbin} -> /usr/{bin,sbin}
	mkdir -p ${pp_destdir}/usr/bin ${pp_destdir}/usr/sbin
	ln -s -f ${bindir}/cvtsudoers ${pp_destdir}/usr/bin
	ln -s -f ${bindir}/sudo ${pp_destdir}/usr/bin
	ln -s -f ${bindir}/sudoedit ${pp_destdir}/usr/bin
	ln -s -f ${bindir}/sudoreplay ${pp_destdir}/usr/bin
	ln -s -f ${sbindir}/sudo_sendlog ${pp_destdir}/usr/sbin
	ln -s -f ${sbindir}/visudo ${pp_destdir}/usr/sbin
%endif

%if [!rpm,deb]
	# Package parent directories when not installing under /usr
	if test "${prefix}" != "/usr"; then
	    extradirs=`echo ${pp_destdir}${mandir}/[mc]* | sed "s#${pp_destdir}##g"`
	    extradirs="$extradirs `dirname $docdir` `dirname $rundir` `dirname $vardir`"
	    test "`dirname $exampledir`" != "$docdir" && extradirs="$extradirs `dirname $exampledir`"
	    test -d ${pp_destdir}${localedir} && extradirs="$extradirs $localedir"
	    for dir in $bindir $sbindir $libexecdir $includedir $extradirs; do
		    while test "$dir" != "/"; do
			    parentdirs="${parentdirs}${parentdirs+ }$dir/"
			    dir=`dirname $dir`
		    done
	    done
	    parentdirs=`echo $parentdirs | tr " " "\n" | sort -u`
	fi
%endif

%depend [deb]
	libc6, libpam0g, libpam-modules, zlib1g, libapparmor1

%fixup [deb]
	# Add Conflicts, Replaces headers and add libldap dependency as needed.
	DEPENDS="%{linux_audit}"
	if test -z "%{flavor}"; then
	    echo "Conflicts: sudo-ldap" >> %{pp_wrkdir}/%{name}/DEBIAN/control
	    echo "Replaces: sudo-ldap" >> %{pp_wrkdir}/%{name}/DEBIAN/control
	elif test "%{flavor}" = "ldap"; then
	    echo "Conflicts: sudo" >> %{pp_wrkdir}/%{name}/DEBIAN/control
	    echo "Replaces: sudo" >> %{pp_wrkdir}/%{name}/DEBIAN/control
	    echo "Provides: sudo" >> %{pp_wrkdir}/%{name}/DEBIAN/control
	    DEPENDS="${DEPENDS}${DEPENDS:+, }libldap-2.4-2"
	fi
	if test -n "%{libssl_dep}"; then
	    DEPENDS="${DEPENDS}${DEPENDS:+, }%{libssl_dep}"
	fi
	cp -p %{pp_wrkdir}/%{name}/DEBIAN/control %{pp_wrkdir}/%{name}/DEBIAN/control.$$
	if test -n "${DEPENDS}"; then
	    sed "s/^\(Depends:.*\) *$/\1, ${DEPENDS}/" %{pp_wrkdir}/%{name}/DEBIAN/control.$$ > %{pp_wrkdir}/%{name}/DEBIAN/control
	fi
	rm -f %{pp_wrkdir}/%{name}/DEBIAN/control.$$
	echo "Homepage: https://www.sudo.ws" >> %{pp_wrkdir}/%{name}/DEBIAN/control
	echo "Bugs: https://bugzilla.sudo.ws" >> %{pp_wrkdir}/%{name}/DEBIAN/control

%fixup [rpm]
	# Must be tab indented for "<<-".
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
%if X"$parentdirs" != X""
	$parentdirs		-
%endif
%if X"$odocdir" != X"$docdir"
	$odocdir/		ignore
	$odocdir/**		ignore
%endif
%if X"$oexampledir" != X"$exampledir" -a X"$exampledir" != X"$docdir/examples"
	$oexampledir/		ignore
	$oexampledir/**		ignore
%endif
	$bindir/cvtsudoers  	0755 root:
	$bindir/sudo        	4755 root:
	$bindir/sudoedit    	0755 root: symlink sudo
	$bindir/sudoreplay  	0755
	$sbindir/sudo_sendlog   0755
	$sbindir/sudo_logsrvd        optional,ignore
	$sbindir/visudo     	0755
	$includedir/sudo_plugin.h 0644
	$libexecdir/sudo/	0755
	$libexecdir/sudo/sesh	0755 optional,ignore-others
	$libexecdir/sudo/python*     optional,ignore,ignore-others
	$libexecdir/sudo/*	$shlib_mode optional
	$sysconfdir/sudoers.d/	0750 $sudoers_uid:$sudoers_gid
	$rundir/		0711 root:
	$vardir/		0711 root: ignore-others
	$vardir/lectured/	0700 root:
	$docdir/		0755
	$docdir/**		0644
%if [deb]
	$docdir/LICENSE.md	ignore,ignore-others
	$docdir/ChangeLog	ignore,ignore-others
%endif
	$exampledir/		0755 ignore-others
%if X"$exampledir" != X"$docdir/examples"
	$exampledir/*		0644
%endif
	$exampledir/sudo_logsrv*     optional,ignore,ignore-others
	$exampledir/*.py             optional,ignore,ignore-others
	$localedir/*/		-    optional
	$localedir/*/LC_MESSAGES/ -    optional
	$localedir/*/LC_MESSAGES/* 0644    optional
	/etc/pam.d/*		0644 volatile,optional
%if [rpm,deb]
	$sysconfdir/sudoers $sudoers_mode $sudoers_uid:$sudoers_gid volatile
	$sysconfdir/sudo.conf 0644 root: volatile
%else
	$sysconfdir/sudoers.dist $sudoers_mode $sudoers_uid:$sudoers_gid
%endif
	$sysconfdir/sudo_logsrvd.conf optional,ignore,ignore-others
%if X"$aix_freeware" = X"true"
	# Links for binaries from /opt/freeware to /usr
	/usr/bin/cvtsudoers    	0755 root: symlink $bindir/cvtsudoers
	/usr/bin/sudo    	0755 root: symlink $bindir/sudo
	/usr/bin/sudoedit    	0755 root: symlink $bindir/sudoedit
	/usr/bin/sudoreplay    	0755 root: symlink $bindir/sudoreplay
	/usr/sbin/sudo_sendlog  0755 root: symlink $sbindir/sendlog
	/usr/sbin/visudo    	0755 root: symlink $sbindir/visudo
%endif
%if [rpm]
	/etc/rc.d/init.d/sudo	0755 root: optional
%endif
%if [aix]
	/etc/rc.d/		ignore
	/etc/rc.d/rc2.d/	ignore
	/etc/rc.d/rc2.d/**	ignore
	/etc/rc.d/init.d/	ignore
	/etc/rc.d/init.d/sudo	0755 root:
%endif
%if [sd]
	/sbin/			ignore
	/sbin/rc2.d/		ignore
	/sbin/rc2.d/**		ignore
	/sbin/init.d/		ignore
	/sbin/init.d/sudo	0755 root:
%endif
	$mandir/man*/*		0644
	$mandir/man*/sudo_logsrv*    ignore,ignore-others
	$mandir/man*/*python*        ignore,ignore-others
	$sudoedit_man		0644 symlink,ignore-others $sudoedit_man_target

%pre [aix]
	if rpm -q %{name} >/dev/null 2>&1; then
		echo "Another version of sudo is currently installed via rpm." 2>&1
		echo "Please either uninstall the rpm version of sudo by running \"rpm -e sudo\"" 2>&1
		echo "or upgrade the existing version of sudo using the .rpm packagae instead" 2>&1
		echo "instead of the .bff package." 2>&1
		echo "" 2>&1
		echo "Note that you may need to pass rpm the --oldpackage flag when upgrading" 2>&1
		echo "the AIX Toolbox version of sudo to the latest sudo rpm from sudo.ws." 2>&1
		echo "" 2>&1
		exit 1
	fi

%post [!rpm,deb]
	# Don't overwrite existing sudoers or sudo.conf files
%if [solaris]
	sysconfdir=${PKG_INSTALL_ROOT}%{sysconfdir}
	exampledir=${PKG_INSTALL_ROOT}%{exampledir}
%else
	sysconfdir=%{sysconfdir}
	exampledir=%{exampledir}
%endif
	if test ! -r $sysconfdir/sudoers; then
		cp $sysconfdir/sudoers.dist $sysconfdir/sudoers
		chmod %{sudoers_mode} $sysconfdir/sudoers
		chown %{sudoers_uid} $sysconfdir/sudoers
		chgrp %{sudoers_gid} $sysconfdir/sudoers
	fi
	if test ! -r $sysconfdir/sudo.conf; then
		cp $exampledir/sudo.conf $sysconfdir/sudo.conf
		chmod 644 $sysconfdir/sudo.conf
		chown root $sysconfdir/sudo.conf
	fi

%post [deb]
	set -e

	# dpkg-deb does not maintain the mode on the sudoers file, and
	# installs it 0640 when sudo requires 0440
	chmod %{sudoers_mode} %{sysconfdir}/sudoers

	# create symlink to ease transition to new path for ldap config
	# if old config file exists and new one doesn't
	if test X"%{flavor}" = X"ldap" -a \
	    -r /etc/ldap/ldap.conf -a ! -r /etc/sudo-ldap.conf; then
		ln -s /etc/ldap/ldap.conf /etc/sudo-ldap.conf
	fi

	# Debian uses a sudo group in its default sudoers file
	perl -e '
		exit 0 if getgrnam("sudo");
		$gid = 27; # default debian sudo gid
		setgrent();
		while (getgrgid($gid)) { $gid++; }
		if ($gid != 27) {
			print "On Debian we normally use gid 27 for \"sudo\".\n";
			$gname = getgrgid(27);
			print "However, on your system gid 27 is group \"$gname\".\n\n";
			print "Would you like me to stop configuring sudo so that you can change this? [n] ";
			$ans = <STDIN>;
			if ($ans =~ /^[yY]/) {
				print "\"dpkg --pending --configure\" will restart the configuration.\n\n";
				exit 1;
			}
		}
		print "Creating group \"sudo\" with gid = $gid\n";
		system("groupadd -g $gid sudo");
		exit 0;
	'

%post [rpm]
	case "%{pp_rpm_distro}" in
	aix*)
		# Create /etc/rc.d/rc2.d/S90sudo link if possible
		if [ -d /etc/rc.d/rc2.d ]; then
			rm -f /etc/rc.d/rc2.d/S90sudo
			ln -s /etc/rc.d/init.d/sudo /etc/rc.d/rc2.d/S90sudo
		fi
		;;
	esac

%post [rpm,deb]
	# Create /usr/lib/tmpfiles.d/sudo.conf if systemd is configured.
	# Must be tab indented for "<<-".
	if [ -f /usr/lib/tmpfiles.d/systemd.conf ]; then
		cat > /usr/lib/tmpfiles.d/sudo.conf <<-EOF
		# Create an empty sudo time stamp directory on OSes using systemd.
		# Sudo will create the directory itself but this can cause problems
		# on systems that have SELinux enabled since the directories will be
		# created with the user's security context.
		d %{rundir} 0711 root root
		D %{rundir}/ts 0700 root root
		EOF
	fi

%post [aix]
	# Create /etc/rc.d/rc2.d/S90sudo link if /etc/rc.d exists
	if [ -d /etc/rc.d ]; then
		rm -f /etc/rc.d/rc2.d/S90sudo
		ln -s /etc/rc.d/init.d/sudo /etc/rc.d/rc2.d/S90sudo
	fi

%post [sd]
	# Create /sbin/rc2.d/S900sudo link
	rm -f /sbin/rc2.d/S900sudo
	ln -s /sbin/init.d/sudo /sbin/rc2.d/S900sudo

%preun
	# Remove the time stamp dir and its contents
	# We currently leave the lecture status files installed
	rm -rf %{rundir}/ts
%if [deb]
	set -e

	# Remove the /etc/ldap/ldap.conf -> /etc/sudo-ldap.conf symlink if
	# it matches what we created in the postinstall script.
	if test X"%{flavor}" = X"ldap" -a \
	    X"`readlink /etc/sudo-ldap.conf 2>/dev/null`" = X"/etc/ldap/ldap.conf"; then
		rm -f /etc/sudo-ldap.conf
	fi

	# Remove systemd tmpfile config
	rm -f /usr/lib/tmpfiles.d/sudo.conf
%endif
%if [rpm]
	case "%{pp_rpm_distro}" in
	aix*)
		# Remove /etc/rc.d/rc2.d/S90sudo link
		rm -f /etc/rc.d/rc2.d/S90sudo
		;;
	*)
		# Remove systemd tmpfile config
		rm -f /usr/lib/tmpfiles.d/sudo.conf
		;;
	esac
%endif
%if [aix]
	# Remove /etc/rc.d/rc2.d/S90sudo link
	rm -f /etc/rc.d/rc2.d/S90sudo
%endif
%if [sd]
	# Remove /sbin/rc2.d/S900sudo link
	rm -f /sbin/rc2.d/S900sudo
%endif
