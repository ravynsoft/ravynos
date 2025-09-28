Sudo installation instructions
==============================

Sudo uses a `configure` script to probe the capabilities and type of the
system in question.  Sudo's `configure` script has a large number of options
that control its behavior and enable or disable optional functionality.
Be sure to read this document fully before configuring and building sudo.
You may also wish to read the file INSTALL.configure which explains more
about the `configure` script itself.

## System requirements

To build sudo from the source distribution you will need a
POSIX-compliant operating system (any modern version of BSD, Linux,
or Unix should work), a C compiler that conforms to ISO C99 or
higher, and the ar, make, and ranlib utilities.

If you wish to modify the parser then you will need flex version
2.5.2 or later and either bison or byacc (sudo comes with a parser
generated with GNU bison).  You'll also have to run configure with
the --with-devel option or pass DEVEL=1 to make.  You can get flex
from https://github.com/westes/flex/.  You can get GNU bison from
https://ftp.gnu.org/pub/gnu/bison/ or any GNU mirror.

Some systems will also require that development library packages be
installed.  The sudo source distribution includes docker configurations
for common Linux distributions that are used for continuous integration
in the `docker` directory.  See the appropriate OS-specific Dockerfile
for a list of packages required to build sudo.

## Simple sudo installation

1. If you are upgrading from a previous version of sudo, read
   [docs/UPGRADE.md](docs/UPGRADE.md) before proceeding.

2. Read the "OS dependent notes" section for any particular
   "gotchas" relating to your operating system.

3. `cd` to the source or build directory and type `./configure`
   to generate a Makefile and config.h file suitable for building
   sudo.  Before you actually run configure you should read the
   "Available configure options" section to see if there are
   any special options you may want or need.

4. Type `make` to compile sudo.  If `configure` did its job properly (and
   you have a supported configuration) there won't be any problems.  If you
   have a problem, check [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)
   for tips on what might have gone wrong.  If your problem is not covered,
   you may file a bug report at https://bugzilla.sudo.ws/ or an issue at
   https://github.com/sudo-project/sudo/issues/ (not both).

5. Optionally, type `make check` to build and run the sudo unit and
   regression tests.  For more verbose output, use `make check-verbose`.

6. Type `make install` (as root) to install sudo, visudo, the man
   pages, and a skeleton sudoers file.  The install will not overwrite
   an existing sudoers file.  You can also install various pieces of
   the package via the install-binaries, install-doc, and install-sudoers
   make targets.

7. Edit the sudoers file with `visudo` as necessary for your
   site.  You will probably want to refer the example sudoers
   file and sudoers man page included with the sudo package.

8. If you want to use syslogd(8) to do the logging, you'll need to
   update your `/etc/syslog.conf` file.  See the examples/syslog.conf
   file included in the distribution for an example.

## Available configure options

This section describes flags accepted by the sudo's `configure` script.
Defaults are listed in brackets after the description.

### Configuration:

    --cache-file=FILE
        Cache test results in FILE

    --config-cache, -C
        Alias for --cache-file=config.cache

    --help, -h
        Print the usage/help info

    --no-create, -n
        Do not create output files

    --quiet, --silent, -q
        Do not print "checking..." messages

    --srcdir=DIR
        Find the sources in DIR [configure dir or ".."]

### Directory and file names:

    --prefix=PREFIX
        Install architecture-independent files in PREFIX.  [/usr/local]

    --exec-prefix=EPREFIX
        Install architecture-dependent files in EPREFIX.
        This includes the executables and plugins.  [same as PREFIX]

    --bindir=DIR
        Install cvtsudoers, sudo, sudoedit, and sudoreplay in DIR. [EPREFIX/bin]

    --sbindir=DIR
        Install sudo_logsrvd, sudo_sendlog, and visudo in DIR. [EPREFIX/sbin]

    --libexecdir=DIR
        Install plugins and helper programs in DIR/sudo [PREFIX/libexec/sudo]

    --sysconfdir=DIR
        Look for configuration files such as `sudo.conf` and `sudoers`
        in DIR. [/etc]

    --includedir=DIR
        Install sudo_plugin.h include file in DIR [PREFIX/include]

    --datarootdir=DIR
        Root directory for platform-independent data files [PREFIX/share]

    --localedir=DIR
        Install sudo and sudoers locale files in DIR [DATAROOTDIR/locale]

    --mandir=DIR
        Install man pages in DIR [PREFIX/man]

    --docdir=DIR
        Install other sudo documentation in DIR [DATAROOTDIR/doc/sudo]

    --with-exampledir=DIR
        Install sudo example files in DIR [DATAROOTDIR/doc/sudo/examples]

    --with-plugindir=DIR
        The directory that sudo looks in to find the policy and I/O
        logging plugins.  Defaults to the LIBEXEC/sudo.

    --with-rundir=DIR
        The directory to be used for sudo-specific files that do
        not survive a system reboot.  This is typically where the
        time stamp directory is located.  By default, configure
        will choose from the following list: /run/sudo /var/run/sudo,
        /var/db/sudo, /var/lib/sudo, /var/adm/sudo, /usr/adm/sudo.

        This directory should be cleared when the system reboots.
        On systems that lack /run or /var/run, the default rundir and
        vardir may be the same.  In this case, only the ts directory
        inside the rundir needs to be cleared at boot time.

    --with-vardir=DIR
        The directory to be used for sudo-specific files that survive
        a system reboot.  This is typically where the lecture status
        directory is stored.  By default, configure will choose
        from the following list: /var/db/sudo, /var/lib/sudo,
        /var/adm/sudo, /usr/adm/sudo.

        This directory should **not** be cleared when the system boots.

    --with-relaydir=DIR
        The directory to be used for sudo_logsrvd relay temporary files.
        When sudo_logsrvd is configured as a store-and-forward relay,
        the journaled data is written to this directory before it is
        forwarded to a relay server.

    --with-tzdir=DIR
        The directory to the system's time zone data files.  This
        is only used when sanitizing the TZ environment variable
        to allow for fully-qualified paths in TZ.  By default,
        configure will look for an existing "zoneinfo" directory
        in the following locations: /usr/share, /usr/share/lib,
        /usr/lib, /etc.

        If no zoneinfo directory is found, the TZ variable may not
        contain a fully-qualified path.

### Compilation options:

    --enable-sanitizer=[flags]
        Enable the use of sanitizers such as AddressSanitizer and
        UndefinedBehaviorSanitizer if supported by the compiler.
        This can help detect common problems such as buffer overflows
        and use after free bugs as well as behavior not defined by
        the C standard.  For more information see:
            https://github.com/google/sanitizers/wiki

        If no flags are specified by the user, a default value of
        "-fsanitize=address,undefined" will be used.

        This option should only be used for testing and not in a
        production environment.  Due to some sanitizers' unchecked
        use of environment variables, it is trivial to exploit a
        set-user-ID root executable such as sudo.

    --enable-fuzzer
        Enable building sudo with the LLVM libFuzzer, see
        https://www.llvm.org/docs/LibFuzzer.html for details.
        The resulting binaries, beginning with "fuzz_" can be used
        to test sudo.  To run all the fuzzers for 8192 iterations,
        "make fuzz" can be used.  This option is generally used in
        conjunction with --enable-sanitizer.

        Fuzzing currently requires the clang C compiler--it is not
        supported by gcc.  For best results, it is suggested to use
        clang 11 or higher.  Some of the fuzzers are known to hang
        when used with earlier versions.

        This option should only be used for testing and not in a
        production environment.

    --enable-fuzzer-engine=library
        The library to use when linking fuzz targets instead of
        LLVM's libFuzzer.  It is intended to be set to the path to
        an alternate fuzzing library, such as AFL++ or Honggfuzz.

    --enable-fuzzer-linker=command
        An alternate linker command to use when building fuzz
        targets, instead of clang.  It may be necessary to set this
        when using the --enable-fuzzer-engine option to link with
        a fuzzer engine that requires C++ libraries.  For oss-fuzz,
        this option is used to cause fuzz targets to be linked with
        clang++.

    --disable-hardening
        Disable the use of compiler/linker exploit mitigation options
        which are enabled by default.  This includes compiling with
        _FORTIFY_SOURCE defined to 2, building with -fstack-protector,
        -fstack-clash-protection, -fcf-protection and linking with
        -zrelro, -znow, and -znoexecstack where supported.

    --disable-largefile
        Disable support for large (64-bit) files on 32-bit systems
        where the maximum file size is normally 4GB.  By default,
        configure will enable support for 64-bit file sizes if
        supported by the operating system.

    --disable-leaks
        Avoid leaking memory even when we are headed for exit,
        which helps reduce the noise from static and active analyzers.
        This option should only be used for testing and not in a
        production environment.

    --enable-pie
        Build sudo and related programs as as a position independent
        executables (PIE).  This improves the effectiveness of address
        space layout randomization (ASLR) on systems that support it.
        Sudo will create PIE binaries by default on Linux systems.

    --disable-pie
        Disable the creation of position independent executables (PIE),
        even if the compiler creates PIE binaries by default.  This
        option may be needed on some Linux systems where PIE binaries
        are not fully supported.

    --disable-poll
        Use select() instead of poll() in the event loop.  By default,
        sudo will use poll() on systems that support it.  Some systems
        have a broken poll() implementation and need to use select instead.
        On macOS, select() is always used since its poll() doesn't
        support character devices.

    --disable-rpath
        By default, configure will use -Rpath in addition to -Lpath
        when passing library paths to the loader.  This option will
        disable the use of -Rpath.

    --disable-shared
        Disable dynamic shared object support.  By default, sudo
        is built with a plugin API capable of loading arbitrary
        policy and I/O logging plugins.  If the --disable-shared
        option is specified, this support is disabled and the default
        sudoers policy and I/O plugins are embedded in the sudo
        binary itself.  This will also disable the intercept and noexec
        options as they also rely on dynamic shared object support.

    --disable-shared-libutil
        Disable the use of the dynamic libsudo_util library.  By
        default, sudo, the sudoers plugin and the associated sudo
        utilities are linked against a shared version of libsudo_util.
        If the --disable-shared-libutil option is specified, a
        static version of the libsudo_util library will be used
        instead.  This option may only be used in conjunction with
        the --enable-static-sudoers option.

    --disable-ssp
        Disable use of the -fstack-protector compiler option.
        This does not affect the other hardening options.

    --enable-static-sudoers
        By default, the sudoers plugin is built and installed as a
        dynamic shared object.  When the --enable-static-sudoers
        option is specified, the sudoers plugin is compiled directly
        into the sudo binary.  Unlike --disable-shared, this does
        not prevent other plugins from being used and the intercept
        and noexec options will continue to function.

    --enable-tmpfiles.d=DIR
        Set the directory to be used when installing the sudo
        tmpfiles.d file.  This is used to create (or clear) the
        sudo time stamp directory on operating systems that use
        systemd.  If this option is not specified, configure will
        use the /usr/lib/tmpfiles.d directory if the file
        /usr/lib/tmpfiles.d/systemd.conf exists.

    --disable-year2038
	Disable support for dates after January 2038.  By default,
        configure will enable support for 64-bit time_t values if
	supported by the operating system.

    --enable-zlib[=location]
        Enable the use of the zlib compress library when storing
        I/O log files.  If specified, location is the base directory
        containing the zlib include and lib directories.  The special
        values "system", "builtin", "shared", and "static" can be
        used to indicate that the system version of zlib should be
        used or that the version of zlib shipped with sudo should
        be used instead.  If "static" is specified, sudo will
        statically link the builtin zlib and not install it.  If
        this option is not specified, configure will use the system
        zlib if it is present, falling back on the sudo version.

    --with-incpath=DIR
        Adds the specified directory (or directories) to CPPFLAGS
        so configure and the compiler will look there for include
        files.  Multiple directories may be specified as long as
        they are space separated.
        E.g. --with-incpath="/usr/local/include /opt/include"

    --with-libpath=DIR
        Adds the specified directory (or directories) to LDFLAGS
        so configure and the compiler will look there for libraries.
        Multiple directories may be specified as with --with-incpath.

    --with-libraries=LIBRARY
        Adds the specified library (or libraries) to SUDO_LIBS and
        and VISUDO_LIBS so sudo will link against them.  If the
        library doesn't start with "-l" or end in ".a" or ".o" a
        "-l" will be prepended to it.  Multiple libraries may be
        specified as long as they are space separated.

    --with-libtool=PATH
        By default, sudo will use the included version of libtool
        to build shared libraries.  The --with-libtool option can
        be used to specify a different version of libtool to use.
        The special values "system" and "builtin" can be used in
        place of a path to denote the default system libtool (obtained
        via the user's PATH) and the default libtool that comes
        with sudo.

    --with-aix-soname=svr4
        Starting with version 1.9.13, sudo will build AIX-style
        shared libraries and dynamic shared objects by default
        instead of svr4-style..  This means that the default sudo
        plugins are now .a (archive) files that contain a .so shared
        object file instead of bare .so files.  This was done to
        improve compatibility with the AIX Freeware ecosystem,
        specifically, the AIX Freeware build of OpenSSL.  To restore
        the old, pre-1.9.13 behavior, run configure using the
        --with-aix-soname=svr4 option.

### Optional features:

    --enable-adminconf=[DIR]
        Search for configuration files in adminconfdir (PREFIX/etc
        by default) in preference to configuration files in sysconfdir
        (/etc by default).  This can be used on systems where
        sysconfdir is located on a read-only filesystem.  When this
        option is enabled, the visudo utility will store edited
        sudoers files in adminconfdir if the original was located
        in sysconfdir.

    --disable-root-mailer
        By default sudo will run the mailer as root when tattling
        on a user so as to prevent that user from killing the mailer.
        With this option, sudo will run the mailer as the invoking
        user which some people consider to be safer.

    --enable-nls[=location]
        Enable natural language support using the gettext() family
        of functions.  If specified, location is the base directory
        containing the libintl include and lib directories.  If
        this option is not specified, configure will look for the
        gettext() family of functions in the standard C library
        first, then check for a standalone libintl (linking with
        libiconv as needed).

    --disable-nls
        Disable natural language support.  By default, sudo will
        use the gettext() family of functions, if available, to
        implement messages in the invoking user's native language.
        Translations do not exist for all languages.

    --with-ldap[=DIR]
        Enable LDAP support.  If specified, DIR is the base directory
        containing the LDAP include and lib directories.  See
        [README.LDAP.md](README.LDAP.md) for more information.

    --with-ldap-conf-file=PATH
        Path to LDAP configuration file.  If specified, sudo reads
        this file instead of `/etc/ldap.conf` to locate the LDAP server.

    --with-ldap-secret-file=PATH
        Path to LDAP secret password file.  If specified, sudo uses
        this file instead of `/etc/ldap.secret` to read the secret password
        when rootbinddn is specified in the ldap config file.

    --disable-sasl
        Disable SASL authentication for LDAP.  By default, sudo
        will compile in support for SASL authentication if the
        ldap_sasl_interactive_bind_s() function is present in the
        LDAP libraries.

    --with-apparmor
        Enable support for the AppArmor Linux Security Module (LSM) on
        supported systems.

    --with-logincap
        This adds support for login classes specified in `/etc/login.conf`.
        It is enabled by default on BSD/OS, Darwin, FreeBSD, OpenBSD, and
        NetBSD (where available).  By default, a login class is not applied
        unless the "use_loginclass" option is defined in sudoers or the user
        specifies a class on the command line.

    --with-interfaces=no, --without-interfaces
        This option keeps sudo from trying to glean the ip address
        from each attached network interface.  It is only useful
        on a machine where sudo's interface reading support does
        not work, which may be the case on some SysV-based OS's
        using STREAMS.

    --enable-intercept[=PATH]
        Enable support for the "intercept" functionality which
        allows sudo to perform a policy check when a dynamically-linked
        program run by sudo attempts to execute another program.
        This is also used to support the "log_subcmds" sudoers
        setting.  For example, this means that for a shell run
        through sudo, the individual commands run by the shell are
        also subject to rules in the sudoers file.  See the "Preventing
        Shell Escapes" section in the sudoers man page for details.
        If specified, PATH should either be a fully-qualified path
        name such as /usr/local/libexec/sudo/sudo_intercept.so, or,
        for AIX and Solaris systems, it may optionally be set to a
        32-bit shared library followed by a 64-bit shared library,
        separated by a colon.  If PATH is "no", intercept support
        will not be compiled in.  The default is to compile intercept
        support if libtool supports building shared objects on your
        system.

    --with-noexec[=PATH]
        Enable support for the "noexec" functionality which prevents
        a dynamically-linked program being run by sudo from executing
        another program (think shell escapes).  See the "Preventing
        Shell Escapes" section in the sudoers man page for details.
        If specified, PATH should either be a fully-qualified path
        name such as /usr/local/libexec/sudo/sudo_noexec.so, or,
        for AIX and Solaris systems, it may optionally be set to a
        32-bit shared library followed by a 64-bit shared library,
        separated by a colon.  If PATH is "no", noexec support
        will not be compiled in.  The default is to compile noexec
        support if libtool supports building shared objects on your
        system.

    --with-selinux
        Enable support for role based access control (RBAC) on systems
        that support SELinux.

    --with-sssd
        Enable support for using the System Security Services Daemon
        (SSSD) as a sudoers data source.  For more information on
        SSD, see https://fedoraproject.org/wiki/Features/SSSD.

    --with-sssd-conf=PATH
        Specify the path to the SSSD configuration file, if different
        from the default value of `/etc/sssd/sssd.conf`.

    --with-sssd-lib=PATH
        Specify the path to the SSSD shared library, which is loaded
        at run-time.

    --enable-offensive-insults
        Enable potentially offensive sudo insults from the classic
        version of sudo.

    --enable-pvs-studio
        Generate a sample PVS-Studio.cfg file based on the compiler and
        platform type.  The "pvs-studio" Makefile target can then be
        used if PVS-Studio is installed.

    --enable-python
        Enable support for sudo plugins written in Python 3.
        This requires a Python 3 development environment (including
        Python 3 header files).

    --disable-log-server
        Disable building the sudo_logsrvd log server.

    --disable-log-client
        Disable sudoers support for using the sudo_logsrvd log server.

### Operating system-specific options:

    --disable-setreuid
        Disable use of the setreuid() function for operating systems
        where it is broken.  For instance, 4.4BSD has setreuid() that
        is not fully functional.

    --disable-setresuid
        Disable use of the setresuid() function for operating systems
        where it is broken (none currently known).

    --enable-admin-flag[=PATH]
        Enable the creation of an Ubuntu-style admin flag file the
        first time sudo is run.  If PATH is not specified, the
        default value is:
            ~/.sudo_as_admin_successful

    --enable-devsearch=PATH
        Set a system-specific search path of directories to look in
        for device nodes.  Sudo uses this when mapping the process's
        tty device number to a device name.  The default value is:
            /dev/pts:/dev/vt:/dev/term:/dev/zcons:/dev/pty:/dev

    --with-bsm-audit
        Enable support for sudo BSM audit logs on systems that support it.
        This includes recent versions of FreeBSD, macOS and Solaris.

    --with-linux-audit
        Enable audit support for Linux systems.  Audits attempts
        to run a command as well as SELinux role changes.

    --with-man
        Use the "man" macros for manual pages.  By default, mdoc versions
        of the manuals are installed if supported.  This can be used to
        override configure's test for "nroff -mdoc" support.

    --with-mdoc
        Use the "mdoc" macros for manual pages.  By default, mdoc versions
        of the manuals are installed if supported.  This can be used to
        override configure's test for "nroff -mdoc" support.

    --with-netsvc[=PATH]
        Path to netsvc.conf or "no" to disable netsvc.conf support.
        If specified, sudo uses this file instead of /etc/netsvc.conf
        on AIX systems.  If netsvc support is disabled but LDAP is
        enabled, sudo will check LDAP first, then the sudoers file.

    --with-nsswitch[=PATH]
        Path to nsswitch.conf or "no" to disable nsswitch support.
        If specified, sudo uses this file instead of /etc/nsswitch.conf.
        If nsswitch support is disabled but LDAP is enabled, sudo will
        check LDAP first, then the sudoers file.

    --with-project
        Enable support for Solaris project resource limits.
        This option is only available on Solaris 9 and above.

### Authentication options:

    --with-AFS
        Enable AFS support with Kerberos authentication.  Should work under
        AFS 3.3.  If your AFS doesn't have -laudit you should be able to
        link without it.

    --with-aixauth
        Enable support for the AIX general authentication function.
        This will use the authentication scheme specified for the
        user on the machine.  By default, sudo will use either AIX
        authentication or PAM depending on the value of the auth_type
        setting in the `/etc/security/login.cfg` file.

    --with-bsdauth
        Enable support for BSD authentication.  This is the default
        for BSD/OS and OpenBSD systems that support it.
        It is not possible to mix BSD authentication with other
        authentication methods (and there really should be no need
        to do so).  Only the newer BSD authentication API is
        supported.  If you don't have /usr/include/bsd_auth.h then
        you cannot use this.

    --with-DCE
        Enable DCE support for systems without PAM.  Known to work on
        HP-UX 9.X, 10.X, and 11.0; other systems may require source
        code and/or `configure` changes.  On systems with PAM support
        (such as HP-UX 11.0 and higher, Solaris, FreeBSD, and Linux), the
        DCE PAM module (usually libpam_dce) should be used instead.

    --with-fwtk[=DIR]
        Enable TIS Firewall Toolkit (FWTK) "authsrv" support. If specified,
        DIR is the base directory containing the compiled FWTK package
        (or at least the library and header files).

    --with-kerb5[=DIR]
        Enable Kerberos V support.  If specified, DIR is the base
        directory containing the Kerberos V include and lib dirs.
        This uses Kerberos pass phrases for authentication but
        does not use the Kerberos cookie scheme.  Will not work for
        Kerberos V older than version 1.1.

    --enable-kerb5-instance=string
        By default, the user name is used as the principal name
        when authenticating via Kerberos V.  If this option is
        enabled, the specified instance string will be appended to
        the user name (separated by a slash) when creating the
        principal name.

    --with-solaris-audit
        Enable audit support for Solaris 11 and above.
        For older versions of Solaris, use --with-bsm-audit

    --with-opie[=DIR]
        Enable NRL OPIE OTP (One Time Password) support.  If specified,
        DIR should contain include and lib directories with opie.h
        and libopie.a respectively.

    --with-otp-only
        This option is now just an alias for --without-passwd.

    --with-pam
        Enable PAM support.  This is on by default for Darwin, FreeBSD,
        Linux, NetBSD, Solaris, and HP-UX (version 11 and higher).

        On RedHat Linux and Fedora you **must** have an `/etc/pam.d/sudo`
        file installed.  You may either use the example pam.conf file included
        with sudo or use `/etc/pam.d/su` as a reference.  The pam.conf file
        included with sudo may or may not work with other Linux distributions.
        On Solaris and HP-UX 11 systems you should check (and understand)
        the contents of `/etc/pam.conf`.  Do a `man pam.conf` for more
        information and consider using the "debug" option, if available,
        with your PAM libraries in `/etc/pam.conf` to obtain syslog output
        for debugging purposes.

    --with-pam-login
        Enable a specific PAM session when sudo is given the -i option.
        This changes the PAM service name when sudo is run with the -i
        option from "sudo" to "sudo-i", allowing for a separate pam
        configuration for sudo's initial login mode.

    --disable-pam-session
        Disable sudo's PAM session support.  This may be needed on
        older PAM implementations or on operating systems where
        opening a PAM session changes the utmp or wtmp files.  If
        PAM session support is disabled, resource limits may not
        be updated for the command being run.

    --with-passwd=no, --without-passwd
        This option excludes authentication via the passwd (or
        shadow) file.  It should only be used when another, alternative,
        authentication scheme is in use.

    --with-SecurID[=DIR]
        Enable SecurID support.  If specified, DIR is directory containing
        libaceclnt.a, acexport.h, and sdacmvls.h.

    --with-skey[=DIR]
        Enable S/Key OTP (One Time Password) support.  If specified,
        DIR should contain include and lib directories with skey.h
        and libskey.a respectively.

    --disable-sia
        Disable SIA support.  This is the "Security Integration
        Architecture" on Digital UNIX. If you disable SIA sudo will
        use its own authentication routines.

    --disable-shadow
        Disable shadow password support.  Normally, sudo will compile
        in shadow password support and use a shadow password if it
        exists.

    --enable-gss-krb5-ccache-name
        Use the gss_krb5_ccache_name() function to set the Kerberos
        V credential cache file name.  By default, sudo will use
        the KRB5CCNAME environment variable to set this.  While
        gss_krb5_ccache_name() provides a better API to do this it
        is not supported by all Kerberos V and SASL combinations.

    --enable-gcrypt[=DIR]
        Use GNU crypt's SHA-2 message digest functions instead of
        OpenSSL or the ones bundled with sudo (or in the system's
        C library).  If specified, DIR should contain the GNU crypt
        include and lib directories.  This option only has an effect
        when OpenSSL 1.0.1 or higher is not present on the system
        or the --disable-openssl option is also specified.

    --enable-openssl[=DIR]
        Use OpenSSL's TLS and SHA-2 message digest functions.  If
        it is detected, OpenSSL will be used by default unless the
        sudo log client and server are disabled via the
        --disable-log-client and --disable-log-server options.  To
        explicitly disable the use of OpenSSL, the --disable-openssl
        option can be used.  OpenSSL versions prior to 1.0.1 will
        not be used as they do not support TLS 1.2.  If specified,
        DIR should contain the OpenSSL include and lib directories.

    --enable-openssl-pkgconfig-template=template
        A printf-style template used to construct the name of the
        openssl and libcrypto pkg-config files.  For example, a
        template of "e%s30" would cause "eopenssl30" and "libecrypto30"
        to be used instead.  This makes it possible to link with
        the OpenSSL 3.0 package on OpenBSD.  Defaults to "%s".

    --enable-wolfssl[=DIR]
        Use wolfSSL's TLS and SHA-2 message digest functions.  If
        specified, DIR should contain the OpenSSL include and lib
        directories.

### Development options:

    --enable-env-debug
        Enable debugging of the environment setting functions.  This
        enables extra checks to make sure the environment does not
        become corrupted.

    --enable-postinstall=PATH
        Enable the use of a postinstall script that is run after
        the "install" target but before packages as built as part
        of the "package" target.

    --enable-warnings
        Enable compiler warnings when building sudo with gcc or clang.

    --enable-werror
        Enable the -Werror compiler option when building sudo with
        gcc or clang.

    --with-devel
        Configure development options.  This will enable compiler warnings
        and set up the Makefile to be able to regenerate the sudoers parser
        as well as the manual pages.

### Options that set runtime-changeable default values:

    --disable-authentication
        By default, sudo requires the user to authenticate via a
        password or similar means.  This options causes sudo to
        **not** require authentication.  It is possible to turn
        authentication back on in sudoers via the PASSWD attribute.  
        Sudoers option: !authenticate

    --disable-env-reset
        Disable environment resetting.  This sets the default value
        of the "env_reset" Defaults option in sudoers to false.  
        Sudoers option: !env_reset

    --disable-path-info
        Normally, sudo will tell the user when a command could not be found
        in their $PATH.  Some sites may wish to disable this as it could
        be used to gather information on the location of executables that
        the normal user does not have access to.  The disadvantage is that
        if the executable is simply not in the user's path, sudo will tell
        the user that they are not allowed to run it, which can be confusing.  
        Sudoers option: path_info

    --disable-root-sudo
        Don't let root run sudo.  This can be used to prevent people from
        "chaining" sudo commands to get a root shell by doing something
        like `sudo sudo /bin/sh`.  
        Sudoers option: !root_sudo

    --disable-zlib
        Disable the use of the zlib compress library when storing
        I/O log files.  
        Sudoers option: !compress_io

    --enable-log-host
        Log the hostname in the log file.  
        Sudoers option: log_host

    --enable-noargs-shell
        If sudo is invoked with no arguments it acts as if the "-s" flag had
        been given.  That is, it runs a shell as root (the shell is determined
        by the SHELL environment variable, falling back on the shell listed
        in the invoking user's `/etc/passwd` entry).  
        Sudoers option: shell_noargs

    --enable-shell-sets-home
        If sudo is invoked with the "-s" flag the HOME environment variable
        will be set to the home directory of the target user (which is root
        unless the "-u" option is used).  This option effectively makes the
        "-s" flag imply "-H".  
        Sudoers option: set_home

    --enable-timestamp-type=TYPE
        Set the default time stamp record type.  The TYPE may be "global"
        (a single record per user), "ppid" (a single record for process
        with the same parent process), or "tty" (a separate record for
        each login session).  The default is "tty".  
        Sudoers option: timestamp_type

    --with-all-insults
        Include all the insult sets listed below.  You must either specify
        --with-insults or enable insults in the sudoers file for this to
        have any effect.

    --with-askpass=PATH
        Set PATH as the "askpass" program to use when no tty is
        available.  Typically, this is a graphical password prompter,
        similar to the one used by ssh.  The program must take a
        prompt as an argument and print the received password to
        the standard output.  This value may overridden at run-time
        in the sudo.conf file.

    --with-badpass-message="MESSAGE"
        Message that is displayed if a user enters an incorrect password.
        The default is "Sorry, try again." unless insults are turned on.  
        Sudoers option: badpass_message

    --with-badpri=PRIORITY
        Determines which syslog priority to log unauthenticated
        commands and errors.  The following priorities are supported:
        alert, crit, debug, emerg, err, info, notice, and warning.  
        Sudoers option: syslog_badpri

    --with-classic-insults
        Uses insults from sudo "classic."  If you just specify --with-insults
        you will get the classic and CSOps insults.  This is on by default if
        --with-insults is given.

    --with-csops-insults
        Insults the user with an extra set of insults (some quotes, some
        original) from a sysadmin group at CU (CSOps).  You must specify
        --with-insults as well for this to have any effect.  This is on by
        default if --with-insults is given.

    --with-editor=PATH
        Specify the default editor path for use by visudo.  This may be a
        single path name or a colon-separated list of editors.  In the latter
        case, visudo will choose the editor that matches the user's SUDO_EDITOR,
        VISUAL or EDITOR environment variable, or the first editor in the list
        that exists.  The default is the path to vi on your system.  
        Sudoers option: editor

    --with-env-editor=no, --without-env-editor
        By default, visudo will consult the SUDO_EDITOR, VISUAL, and EDITOR
        environment variables before falling back on the default editor list
        (as specified by --with-editor).  visudo is typically run as root so
        this option may allow a user with visudo privileges to run arbitrary
        commands as root without logging.  Some sites may with to disable this
        and use a colon-separated list of "safe" editors with the --with-editor
        option.  visudo will then only use the SUDO_EDITOR, VISUAL, or EDITOR
        variables if they match a value specified via --with-editor.  
        Sudoers option: env_editor

    --with-exempt=GROUP
        Users in the specified group don't need to enter a password when
        running sudo.  This may be useful for sites that don't want their
        "core" sysadmins to have to enter a password but where Jr. sysadmins
        need to.  You should probably use NOPASSWD in sudoers instead.  
        Sudoers option: exempt_group

    --with-fqdn
        Define this if you want to put fully-qualified host names in the sudoers
        file.  Ie: instead of myhost you would use myhost.mydomain.edu.  You may
        still use the short form if you wish (and even mix the two).  Beware
        that turning FQDN on requires sudo to make DNS lookups which may make
        sudo unusable if your DNS is totally hosed.  You must use the host's
        official name as DNS knows it.  That is, you may not use a host alias
        (CNAME entry) due to performance issues and the fact that there is no
        way to get all aliases from DNS.  
        Sudoers option: fqdn

    --with-goodpri=PRIORITY
        Determines which syslog priority to log successfully authenticated
        commands.  The following priorities are supported: alert, crit, debug,
        emerg, err, info, notice, and warning.  
        Sudoers option: syslog_goodpri

    --with-python-insults
        Insults the user with lines from "Monty Python's Flying Circus" when an
        incorrect password is entered. You must either specify --with-insults or
        enable insults in the sudoers file for this to have any effect.

    --with-goons-insults
        Insults the user with lines from the "Goon Show" when an incorrect
        password is entered.  You must either specify --with-insults or
        enable insults in the sudoers file for this to have any effect.

    --with-hal-insults
        Uses 2001-like insults when an incorrect password is entered.
        You must either specify --with-insults or enable insults in the
        sudoers file for this to have any effect.

    --with-ignore-dot
        If set, sudo will ignore "." or "" (current dir) in $PATH.
        The $PATH itself is not modified.  
        Sudoers option: ignore_dot

    --with-insults
        Define this if you want to be insulted for typing an incorrect password
        just like the original sudo(8).  This is off by default.  
        Sudoers option: insults

    --with-insults=disabled
        Include support for insults but disable them unless explicitly
        enabled in sudoers.  
        Sudoers option: !insults

    --with-iologdir[=DIR]
        By default, sudo stores I/O log files in either /var/log/sudo-io,
        /var/adm/sudo-io, or /usr/log/sudo-io.  If this option is specified,
        I/O logs will be stored in the indicated directory instead.  
        Sudoers option: iolog_dir

    --with-lecture=no, --without-lecture
        Don't print the lecture the first time a user runs sudo.  
        Sudoers option: !lecture

    --with-logfac=FACILITY
        Determines which syslog facility to log to.  This requires
        a 4.3BSD or later version of syslog.  You can still set
        this for ancient syslogs but it will have no effect.  The
        following facilities are supported: authpriv (if your OS
        supports it), auth, daemon, user, local0, local1, local2,
        local3, local4, local5, local6, and local7.  
        Sudoers option: syslog

    --with-logging=TYPE
        How you want to do your logging.  You may choose "syslog",
        "file", or "both".  Setting this to "syslog" is nice because
        you can keep all of your sudo logs in one place (see the
        example syslog.conf file).  The default is "syslog".  
        Sudoers options: syslog and logfile

    --with-loglen=NUMBER
        Number of characters per line for the file log.  This is only used if
        you are to "file" or "both".  This value is used to decide when to wrap
        lines for nicer log files.  The default is 80.  Setting this to 0
        will disable the wrapping.  
        Sudoers options: loglinelen

    --with-logpath=PATH
        Override the default location of the sudo log file and use
        "path" instead.  By default will use /var/log/sudo.log if
        there is a /var/log dir, falling back to /var/adm/sudo.log
        or /usr/adm/sudo.log if not.  
        Sudoers option: logfile

    --with-long-otp-prompt
        When validating with a One Time Password scheme (S/Key or
        OPIE), a two-line prompt is used to make it easier to cut
        and paste the challenge to a local window.  It's not as
        pretty as the default but some people find it more convenient.  
        Sudoers option: long_otp_prompt

    --with-mail-if-no-user=no, --without-mail-if-no-user
        Normally, sudo will mail to the "alertmail" user if the user invoking
        sudo is not in the sudoers file.  This option disables that behavior.  
        Sudoers option: mail_no_user

    --with-mail-if-no-host
        Send mail to the "alermail" user if the user exists in the sudoers
        file, but is not allowed to run commands on the current host.  
        Sudoers option: mail_no_host

    --with-mail-if-noperms
        Send mail to the "alermail" user if the user is allowed to use sudo but
        the command they are trying is not listed in their sudoers file entry.  
        Sudoers option: mail_no_perms

    --with-mailsubject="SUBJECT"
        Subject of the mail sent to the "mailto" user. The token "%h"
        will expand to the hostname of the machine.
        The default value is "*** SECURITY information for %h ***".  
        Sudoers option: mailsub

    --with-mailto=USER|MAIL_ALIAS
        User (or mail alias) that mail from sudo is sent to.
        This should go to a sysadmin at your site.  The default value is "root".  
        Sudoers option: mailto

    --with-passprompt="PROMPT"
        Default prompt to use when asking for a password; can be overridden
        via the -p option and the SUDO_PROMPT environment variable. Supports
        the "%H", "%h", "%U", and "%u" escapes as documented in the sudo
        manual page.  The default value is "Password:".  
        Sudoers option: passprompt

    --with-password-timeout=NUMBER
        Number of minutes before the sudo password prompt times out.
        The default is 5, set this to 0 for no password timeout.  
        Sudoers option: passwd_timeout

    --with-passwd-tries=NUMBER
        Number of tries a user gets to enter his/her password before sudo logs
        the failure and exits.  The default is 3.  
        Sudoers option: passwd_tries

    --with-runas-default=USER
        The default user to run commands as if the -u flag is not specified
        on the command line.  This defaults to "root".  
        Sudoers option: runas_default

    --with-secure-path[=PATH]
        Path used for every command run from sudo(8).  If you don't trust
        users to have a reasonable PATH environment variable you may want
        to use this.  Another use is if you want to have the "root path"
        be separate from the "user path."  You will need to customize the
        path for your site.  This is not applied to users in the group
        specified by --with-exemptgroup.  If you do not specify a path,
        "/bin:/usr/ucb:/usr/bin:/usr/sbin:/sbin:/usr/etc:/etc" is used.  
        Sudoers option: secure_path

    --with-sendmail=PATH
        Override configure's guess as to the location of sendmail.  
        Sudoers option: mailerpath

    --with-sendmail=no, --without-sendmail
        Do not use sendmail to mail messages to the "mailto" user.
        Use only if you don't run sendmail or the equivalent.  
        Sudoers options: !mailerpath or !mailto

    --with-sudoers-mode=MODE
        File mode for the sudoers file (octal).  If you wish to
        NFS-mount the sudoers file this must be group readable.
        This value may overridden at run-time in the sudo.conf file.
        The default mode is 0440.

    --with-sudoers-uid=UID
        User id that "owns" the sudoers file.  This is the numeric
        id, **not** the symbolic name.  This value may overridden
        at run-time in the sudo.conf file.  The default is 0.

    --with-sudoers-gid=GID
        Group id that "owns" the sudoers file.  This is the numeric
        id, **not** the symbolic name.  This value may overridden
        at run-time in the sudo.conf file.  The default is 0.

    --with-timeout=NUMBER
        Number of minutes that can elapse before sudo will ask for a passwd
        again.  The default is 5, set it to 0 to always prompt for a password.  
        Sudoers option: timestamp_timeout

    --with-umask=MASK
        Umask to use when running the root command.  The default is 0022.  
        Sudoers option: umask

    --with-umask=no, --without-umask
        Preserves the umask of the user invoking sudo.  
        Sudoers option: !umask

    --with-umask-override
        Use the umask specified in sudoers even if it is less restrictive
        than the user's.  The default is to use the intersection of the
        user's umask and the umask specified in sudoers.  
        Sudoers option: umask_override

## OS dependent notes

#### HP-UX

The default C compiler shipped with HP-UX is not an ANSI compiler.
You must use either the HP ANSI C compiler or gcc to build sudo.
Binary packages of gcc are available from http://hpux.connect.org.uk/.

To prevent PAM from overriding the value of umask on HP-UX 11,
you will need to add a line like the following to /etc/pam.conf:

    sudo	session	required	libpam_hpsec.so.1 bypass_umask

#### Linux

PAM and LDAP headers are not installed by default on most Linux
systems.  You will need to install the "pam-dev" (rpm) or libpam0g-dev
(deb) package if `/usr/include/security/pam_appl.h` is not present
on your system.  If you wish to build with LDAP support you will
also need the "openldap-devel" (rpm) or "libldap2-dev" (deb) package.

#### macOS

The pseudo-tty support in the Darwin kernel has bugs related to
its handling of the SIGTSTP, SIGTTIN, and SIGTTOU signals.  It does
not restart reads and writes when those signals are delivered.  This
may cause problems for some commands when I/O logging is enabled.
The issue has been reported to Apple and is bug id #7952709.

#### Solaris

You need to have a C compiler in order to build sudo.  Since Solaris
does not come with one by default this means that you either need
to either install the Solaris Studio compiler suite, available for
free from www.oracle.com, or install the GNU C compiler (gcc) which
is can be installed via the pkg utility on Solaris 11 and higher
and is distributed on the Solaris Companion CD for older Solaris
releases.  You can also download gcc packages from
https://www.opencsw.org/packages/CSWgcc4core/.
