%set
	name="sudo-python"
	summary="Sudo Python plugin framework"
	description="The sudo Python plugin allows you to extend sudo using Python."
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

%if [rpm,deb]
	# Convert patch level into release and remove from version
	pp_rpm_release="`expr \( $version : '.*p\([0-9][0-9]*\)$' \| 0 \) + 1`"
	pp_rpm_version="`expr \( $version : '\(.*\)p[0-9][0-9]*$' \| $version \)`"
	pp_rpm_license="BSD"
	pp_rpm_url="https://www.sudo.ws"
	pp_rpm_group="Applications/System"
	pp_rpm_packager="Todd C. Miller <Todd.Miller@sudo.ws>"
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
	pp_macos_bundle_id=ws.sudo.pkg.sudo-python
	pp_macos_pkg_background=${srcdir}/etc/macos-background.png
	pp_macos_pkg_background_dark=${srcdir}/etc/macos-background.png
	pp_macos_pkg_license=${pp_destdir}$docdir/LICENSE.md
	pp_macos_pkg_readme=${pp_wrkdir}/ReadMe.txt
	perl -pe 'last if (/^What/i && $seen++)' ${pp_destdir}$docdir/NEWS > ${pp_wrkdir}/ReadMe.txt
%endif

%if [!rpm,deb]
	# Package parent directories when not installing under /usr
	if test "${prefix}" != "/usr"; then
	    extradirs=`echo ${pp_destdir}${mandir}/[mc]* | sed "s#${pp_destdir}##g"`
	    extradirs="$extradirs `dirname $docdir`"
	    test "`dirname $exampledir`" != "$docdir" && extradirs="$extradirs `dirname $exampledir`"
	    for dir in $libexecdir $extradirs; do
		    while test "$dir" != "/"; do
			    parentdirs="${parentdirs}${parentdirs+ }$dir/"
			    dir=`dirname $dir`
		    done
	    done
	    parentdirs=`echo $parentdirs | tr " " "\n" | sort -u`
	fi
%endif

%depend [deb]
	libc6, libpython@PYTHON_VERSION@, sudo

%fixup [deb]
	cp -p %{pp_wrkdir}/%{name}/DEBIAN/control %{pp_wrkdir}/%{name}/DEBIAN/control.$$
	sed "s/@PYTHON_VERSION@/%{python_version}/g" %{pp_wrkdir}/%{name}/DEBIAN/control.$$ > %{pp_wrkdir}/%{name}/DEBIAN/control
	rm -f %{pp_wrkdir}/%{name}/DEBIAN/control.$$
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
	$libexecdir/sudo/	0755 ignore-others
	$libexecdir/sudo/python* $shlib_mode ignore-others
	$docdir/		0755 ignore-others
	$exampledir/		0755 ignore-others
	$exampledir/*.py	0644 ignore-others
	$mandir/man*/*python*	0644 ignore-others
