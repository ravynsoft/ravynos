Troubleshooting tips and FAQ for Sudo
=====================================

#### When I run configure, it says "C compiler cannot create executables".

> This usually means you either don't have a working compiler.  This
> could be due to the lack of a license or that some component of the
> compiler suite could not be found.  Check config.log for clues as
> to why this is happening.  On many systems, compiler components live
> in /usr/ccs/bin which may not be in your PATH environment variable.

#### When I run configure, it says "sudo requires the 'ar' utility to build".

> As part of the build process, sudo creates a temporary library
> containing objects that are shared amongst the different sudo
> executables.  On Unix systems, the 'ar' utility is used to do this.
> This error indicates that 'ar' is missing on your system.  On Solaris
> systems, you may need to install the SUNWbtool package.  On other
> systems 'ar' may be included in the GNU binutils package.

#### Sudo compiles and installs successfully but when I try to run it I get:

    The "no new privileges" flag is set, which prevents sudo from
    running as root.  If sudo is running in a container, you may
    need to adjust the container configuration to disable the flag.

> Sudo was run by a process that has the Linux "no new privileges"
> flag set.  This causes the set-user-ID bit to be ignored when running
> an executable, which will prevent sudo from functioning.  The most
> likely cause for this is running sudo within a container that sets
> this flag.  Check the documentation to see if it is possible to
> configure the container such that the flag is not set.

#### Sudo compiles and installs successfully but when I try to run it I get:

    /usr/local/bin/sudo must be owned by uid 0 and have the setuid bit set

> Sudo must be set-user-ID root to do its work.  Either `/usr/local/bin/sudo`
> is not owned by user-ID 0 or the set-user-ID bit is not set.  This should
> have been done for you by `make install` but you can fix it manually by
> running the following as root:

    chown root /usr/local/bin/sudo; chmod 4755 /usr/local/bin/sudo

#### Sudo compiles and installs successfully but when I try to run it I get:

    effective uid is not 0, is /usr/local/bin/sudo on a file system with the
    'nosuid' option set or an NFS file system without root privileges?

> The owner and permissions on the sudo binary appear to be OK but when
> sudo ran, the set-user-ID bit did not have an effect.  There are two
> common causes for this.  The first is that the file system the sudo
> binary is located on is mounted with the 'nosuid' mount option, which
> disables set-user-ID binaries.  The output of the 'mount' command should
> tell you if the file system is mounted with the 'nosuid' option.  The
> other possible cause is that sudo is installed on an NFS-mounted file
> system that is exported without root privileges.  By default, NFS file
> systems are exported with user-ID 0 mapped to a non-privileged ID (usually
> -2).  You should be able to determine whether sudo is located on an
> NFS-mounted filesystem by running "df \`which sudo\`".

#### Sudo never gives me a chance to enter a password using PAM

It just says "Sorry, try again." three times and exits.

> You didn't setup PAM to work with sudo.  On RedHat or Fedora Linux
> this generally means installing the sample pam.conf file as
> /etc/pam.d/sudo.  See the example pam.conf file for hints on what
> to use for other Linux systems.

#### Sudo says my account has expired but I know it has not

> If you get the following error from sudo:

    Account expired or PAM config lacks an 'account' section for sudo,
    contact your system administrator`

> double-check the `/etc/shadow` file to verify that the target user
> (for example, root) does not have the password expiration field set.
> A common way to disable access to an account is to set the expiration
> date to 1, such as via `usermod -e 1`.  If the account is marked as
> expired, sudo will not allow you to access it.
>
> If, however, the account has not expired, it is possible that the PAM
> configuration lacks an 'account' specification.  On Linux this usually
> means you are missing a line in /etc/pam.d/sudo similar to:

    account    required    pam_unix.so

#### Sudo is configured use syslog but nothing gets logged

> Make sure you have an entry in your syslog.conf file to save
> the sudo messages (see the example syslog.conf file).  The default
> log facility is authpriv (changeable via configure or in sudoers).
> Don't forget to send a SIGHUP to your syslogd so that it re-reads
> its conf file.  Also, remember that syslogd does *not* create
> log files, you need to create the file before syslogd will log
> to it (e.g.: touch /var/log/sudo).

> The facility (e.g. 'auth.debug') must be separated from
> the destination (e.g. '/var/log/auth' or '@loghost') by tabs,
> *not* spaces.  This is a common error.

#### Sudo won't accept my password, even when entered correctly

> If you are not using pam and your system uses shadow passwords,
> it is possible that sudo didn't properly detect that shadow
> passwords are in use.  Take a look at the generated config.h
> file and verify that the C function used for shadow password
> look ups was detected.  For instance, for SVR4-style shadow
> passwords, `HAVE_GETSPNAM` should be defined (you can search for
> the string 'shadow passwords' in config.h with your editor).
> There is no define needed for 4.4BSD-based shadow passwords
> which just use the standard getpw* routines.

#### Can sudo use the ssh agent instead of asking for the user's password?

> Not directly, but you can use a PAM module like pam_ssh_agent_auth
> or pam_ssh for this purpose.

#### I want to place the sudoers file in a directory other than /etc

> Use the `--sysconfdir` option to configure.  For example:

    configure --sysconfdir=/dir/you/want/sudoers/in

> Alternately, you can set the path in the sudo.conf file as an
> argument to the sudoers.so plugin. For example:

    Plugin sudoers_policy sudoers.so sudoers_file=/path/to/sudoers

#### Can I put the sudoers file in NIS/NIS+?

> There is no support for making an NIS/NIS+ map/table out of the sudoers
> file at this time.  You can distribute the sudoers file via rsync or rdist.
> It is also possible to NFS-mount the sudoers file.  If you use LDAP at your
> site you may be interested in sudo's LDAP sudoers support, see
> [README.LDAP.md](../README.LDAP.md) and the sudoers.ldap manual.

#### I don't run sendmail, does this mean that I cannot use sudo?

> No, you just need to disable mailing with a line like:

    Defaults !mailerpath

> in your sudoers file or run configure with the `--without-sendmail`
> option.

#### How can I make visudo use a different editor?

> You can specify the editor to use in visudo in the sudoers file.
> See the 'editor' and 'env_editor' entries in the sudoers manual.
> The defaults can also be set at configure time using the
> `--with-editor` and `--with-env-editor` configure options.

#### Why does sudo modify the command's environment?

> By default, sudo runs commands with a new, minimal environment.
> The 'env_keep' setting in sudoers can be used to control which
> environment variables are preserved from the invoking user's
> environment via the 'env_keep' setting in sudoers.
>
> While it is possible to disable the 'env_reset' setting, which
> will preserve all environment variables that don't match a black
> list, doing so is strongly discouraged.  See the "Command
> environment" section of the sudoers manual for more information.

#### Why does sudo reset the HOME environment variable?

> Many programs use the HOME environment variable to locate
> configuration and data files.  Often, these configuration files
> are treated as trusted input that affects how the program operates.
> By controlling the configuration files, a user may be able to
> cause the program to execute other commands without sudo's
> restrictions or logging.
>
> Some programs perform extra checks when the real and effective
> user-IDs differ, but because sudo runs commands with all user-IDs
> set to the target user, these checks are insufficient.
>
> While it is possible to preserve the value of the HOME environment
> variable by adding it to the 'env_keep' list in the sudoers file,
> doing so is strongly discouraged.  Users wishing to edit files
> with sudo should run sudoedit (or sudo -e) to get their accustomed
> editor configuration instead of invoking the editor directly.

#### How can I prevent sudo from asking for a password?

> To specify this on a per-user (and per-command) basis, use the
> 'NOPASSWD' tag right before the command list in sudoers.  See
> the sudoers man page and examples/sudoers for details.  To disable
> passwords completely, add '!authenticate' to the Defaults line
> in /etc/sudoers.  You can also turn off authentication on a
> per-user or per-host basis using a user or host-specific Defaults
> entry in sudoers.  To hard-code the global default, you can
> configure with the `--without-passwd` option.

#### The configure scripts says `no acceptable cc found in $PATH`

> /usr/ucb/cc was the only C compiler that configure could find.
> You need to tell configure the path to the 'real' C compiler
> via the `--with-CC option`.  On Solaris, the path is probably
> something like /opt/SUNWspro/SC4.0/bin/cc.  If you have gcc
> that will also work.

#### The configure scripts says "config.cache exists from another platform!"

> configure caches the results of its tests in a file called
> config.cache to make re-running configure speedy.  However,
> if you are building sudo for a different platform the results
> in config.cache will be wrong so you need to remove the config.cache file.
> You can do this via `rm config.cache`, or `make realclean` to also
> remove any object files and configure temp files that are present.

#### When I run 'visudo' it says "sudoers file busy, try again later."

> Someone else is currently editing the sudoers file with visudo.

#### When I try to use 'cd' with sudo it says "cd: command not found"

> 'cd' is a shell built-in command, you can't run it as a command
> since a child process (sudo) cannot affect the current working
> directory of the parent (your shell).

#### When I try to use 'cd' with sudo nothing happens.

> Even though 'cd' is a shell built-in command, some operating systems
> include a /usr/bin/cd command for completeness.  A standalone
> "cd' command is totally useless since a child process (cd) cannot
> affect the current working directory of the parent (your shell).
> Thus, `sudo cd /foo` will start a child process, change the
> directory and immediately exit without doing anything useful.

#### How can I run a command via sudo as a user other than root?

> The default user sudo tries to run things as is always root, even if
> the invoking user can only run commands as a single, specific user.
> This may change in the future but at the present time you have to
> work around this using the 'runas_default' option in sudoers.
> For example, given the following sudoers rule:

    bob ALL=(oracle) ALL

> You can cause sudo to run all commands as 'oracle' for user 'bob'
> with a sudoers entry like:

    Defaults:bob runas_default=oracle

#### When I try to run sudo via ssh, I get an error:

    sudo: a terminal is required to read the password; either use the -S
          option to read from standard input or configure an askpass helper

> If sudo needs to authenticate a user, it requires access to the user's
> terminal to disable echo so the password is not displayed to the screen.
> The above message indicates that no terminal was present.

> When running a command via ssh, a terminal is not allocated by default
> which can cause this message.  The '-t' option to ssh will force it to
> allocate a tty.  Alternately, you may be able to use the ssh-askpass
> utility to prompt for the password if X11 forwarding is enabled and an
> askpass helper is configured in the sudo.conf file.  If you do not mind
> your password being echoed to the screen, you may use sudo's -S option
> to read the password from the standard input.  Alternately, you may set
> the 'visiblepw' sudoers option which will allow the password to be entered
> even when echo cannot be disabled, though this is not recommended.

#### When I try to use SSL-enabled LDAP with sudo I get an error:

    unable to initialize SSL cert and key db: security library: bad database.
    you must set TLS_CERT in /etc/ldap.conf to use SSL

> On systems that use a Mozilla-derived LDAP SDK there must be a
> certificate database in place to use SSL-encrypted LDAP connections.
> This file is usually /var/ldap/cert8.db or /etc/ldap/cert8.db.
> The actual number after 'cert' will vary, depending on the version
> of the LDAP SDK that is being used.  If you do not have a certificate
> database you can either copy one from a mozilla-derived browser, such
> as firefox, or create one using the `certutil` command.  You can run
> `certutil` as follows and press the <return> (or <enter>) key at the
> password prompt:

    # certutil -N -d /var/ldap

> Enter a password which will be used to encrypt your keys.
> The password should be at least 8 characters long,
> and should contain at least one non-alphabetic character.

    Enter new password: <return>
    Re-enter password: <return>

#### After upgrading my system, sudo_logsrvd gives the error:

    X509_verify_cert: CA cert does not include key usage extension

> This can happen if you are using self-signed certificates that do not
> include the key usage extension.  This error can occur if the certificates
> were generated using OpenSSL 1.x but sudo_logsrvd now uses OpenSSL 3.x,
> for example after a system upgrade.  The x509 certificate validation in
> OpenSSL 3.x now requires that the key usage extension be present.
> One way to address this is to disable certificate verification in
> sudo_logsrvd by setting the _tls_verify_ key in the `[server]` section
> to _false_.  Alternately, you can simply remove your old CA and the
> associated certificates and create a new one using an updated
> `/etc/ssl/openssl.cnf` file.  See the sudo_logsrvd manual for more
> information on creating self-signed certificates.

#### On HP-UX, the umask setting in sudoers has no effect.

> If your /etc/pam.conf file has the libpam_hpsec.so.1 session module
> enabled, you may need to a add line like the following to pam.conf:
> sudo session required libpam_hpsec.so.1 bypass_umask

#### When I run `sudo -i shell_alias` I get "command not found"

> Commands run via `sudo -i` are executed by the shell in
> non-interactive mode.  The bash shell will only parse aliases in
> interactive mode unless the 'expand_aliases' shell option is
> set.  If you add `shopt -s expand_aliases` to your .bash_profile
> (or .profile if using that instead) the aliases should now be
> available to `sudo -i`.

#### When I run sudo on AIX I get the following error:

    setuidx(ID_EFFECTIVE|ID_REAL|ID_SAVED, ROOT_UID): Operation not permitted.

> AIX's Enhanced RBAC is preventing sudo from running.  To fix
> this, add the following entry to /etc/security/privcmds (adjust
> the path to sudo as needed) and run the setkst command as root:

    /usr/local/bin/sudo:
        accessauths = ALLOW_ALL
        innateprivs = PV_DAC_GID,PV_DAC_R,PV_DAC_UID,PV_DAC_X,PV_FS_CHOWN,PV_PROC_PRIO,PV_NET_PORT,PV_NET_CNTL,PV_SU_UID
        secflags = FSF_EPS

#### Sudo builds without error but when I run it I get a Segmentation fault.

> If you are on a Linux system, the first thing to try is to run
> configure with the `--disable-pie` option, then `make clean` and
> `make`.  If that fixes the problem then your operating system
> does not properly support position independent executables.
> Send a message to sudo@sudo.ws with system details such as the
> Linux distro, kernel version, and CPU architecture.

#### When I run configure I get the following error:

    dlopen present but libtool doesn't appear to support your platform.

> Libtool doesn't know how to support dynamic linking on the operating
> system you are building for.  If you are cross-compiling, you need to
> specify the operating system, not just the CPU type.  For example,
> `--host powerpc-unknown-linux`
> instead of just:
> `--host powerpc`

#### How do you pronounce 'sudo'?

> The official pronunciation is soo-doo (for su 'do').  However, an
> alternate pronunciation, a homophone of 'pseudo', is also common.
