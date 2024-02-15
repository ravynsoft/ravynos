Notes on upgrading from an older release
========================================

 * Upgrading from a version prior to 1.9.15:

    The sudoers plugin now uses a time stamp path name that is based
    on the user-ID instead of the user name.  For example, a time
    stamp file that was /var/run/sudo/ts/root in sudo 1.9.14 will
    now be /var/run/sudo/ts/0.  The lecture flag file name is now
    also based on the user-ID, which will result in users receiving
    the sudo lecture again on upgrade to sudo 1.9.15.

 * Upgrading from a version prior to 1.9.14:

   Sudo now runs commands in a new pseudo-terminal by default.  This
   can prevent a malicious program run via sudo from accessing the
   user's terminal device after the command completes.

   When sudo runs a command in a new pseudo-terminal, an additional
   process is created to monitor the command's status and pass
   terminal control signals between the two terminals.  See the
   "Process model" subsection in the sudo manual and the description
   of the "use_pty" option in the sudoers manual for more information.

   A side effect of running the command in a new pseudo-terminal
   is that sudo must pass input from the user's terminal to the
   pseudo-terminal, even if the command being run does not require
   the input.  The "exec_background" option in sudoers can be used
   to prevent this, but some screen-oriented commands may not operate
   properly when run as a background process.

   To restore the historic behavior where a command is run in the
   user's terminal, add the following line to the sudoers file:

       Defaults !use_pty

 * Upgrading from a version prior to 1.9.13:
   
   Sudo now builds AIX-style shared libraries and dynamic shared
   objects by default instead of svr4-style.  This means that the
   default sudo plugins are now .a (archive) files that contain a
   .so shared object file instead of bare .so files.  This was done
   to improve compatibility with the AIX Freeware ecosystem,
   specifically, the AIX Freeware build of OpenSSL.  When loading
   a .a file as a plugin the name of the included .so file must
   also be specified, for example /usr/libexec/sudo/sudoers.a(sudoers.so).

   Sudo is still capable of loading svr4-style .so plugins and if
   a .so file is requested, either via sudo.conf or the sudoers
   file, and only the .a file is present, sudo will convert the
   path from plugin.so to plugin.a(plugin.so).  This ensures
   compatibility with existing configurations.  To restore the old,
   pre-1.9.13 behavior, run configure using the --with-aix-soname=svr4
   option.

 * Upgrading from a version prior to 1.9.10:

   Sudo now interprets a command line argument in sudoers that
   begins with a '^' character as a regular expression.  To start
   a command argument with a literal '^' character, it must be
   escaped with a backslash ('\').  This may result in a syntax
   error after upgrading for existing sudoers rules where the command
   line arguments begin with a '^'.

   A user may now only run "sudo -U otheruser -l" if they have a
   "sudo ALL" privilege where the RunAs user contains either "root"
   or "otheruser".  Previously, having "sudo ALL" was sufficient,
   regardless of the RunAs user.

 * Upgrading from a version prior to 1.9.9:

   Sudo now runs commands with the core limit resource limit set
   to 0 by default.  While most operating systems restrict core
   dumps of set-user-ID programs like sudo, this protection is
   lost when sudo executes a command.  By disabling core dumps by
   default, it is possible to avoid potential security problems
   such as those seen with the Linux logrotate utility, which could
   interpret a core dump as a valid configuration file.

   To restore the historic core dump file size behavior, add the
   following line to the sudoers file:

       Defaults rlimit_core=default

 * Upgrading from a version prior to 1.9.7:

   Sudo now links with OpenSSL 1.0.1 or higher by default if it
   is present on the system unless it is explicitly disabled (via
   `--disable-openssl`), or unless the sudo log client and server
   code is disabled (via `--disable-log-client` and `--disable-log-server`).
   As a result, the sudo log server (and the client built into the
   sudoers plugin) now support TLS connections by default.

 * Upgrading from a version prior to 1.9.3:

   Due to the addition of the CHROOT and CWD options, it is no
   longer possible to declare an alias with one of those names.
   If a sudoers file has an alias with one of those names, sudo
   and visudo will report a syntax error with a message like
   "syntax error: unexpected CHROOT, expecting ALIAS".

   Starting with version 1.9.3, sudoers rules must end in either
   a newline or the end-of-file.  This makes it possible to provide
   better error messages.  Previously, it was possible to include
   multiple rules on a single line, separated by white space.

   Starting with version 1.9.3, sudo will attempt to recover from
   a syntax error in the sudoers file by discarding the portion
   of the line that contains the error until the end of the line.
   To restore the historic behavior of refusing to run when a
   syntax error is encountered, add `error_recovery=false` as a
   plugin option in sudo.conf for the "sudoers_audit" plugin, (or
   "sudoers_policy" if there is no "sudoers_audit" plugin configured).

 * Upgrading from a version prior to 1.9.1:

   Starting with version 1.9.1, sudoers plugin arguments in sudo.conf
   should be specified for the "sudoers_audit" plugin, not
   "sudoers_policy".  This is because the sudoers file is now
   opened and parsed by the "sudoers_audit" plugin.  Previously,
   this was done by the "sudoers_policy" plugin.  The use of an
   audit plugin makes it possible for the sudoers module to detect
   when a command has been rejected by an approval plugin and only
   log commands that are allowed by both policy and approval
   plugins.

 * Upgrading from a version prior to 1.8.30:

   Starting with version 1.8.30, sudo will no longer allow commands
   to be run as a user or group ID that is not in the password or
   group databases by default.  Previously, sudo would always allow
   unknown user or group IDs if the sudoers entry permitted it,
   including via the _ALL_ alias.  The old behavior can be restored
   by setting the new "allow_unknown_runas_id" Defaults setting
   in the sudoers file.

 * Upgrading from a version prior to 1.8.29:

   Starting with version 1.8.29, if the umask is explicitly set
   in sudoers, that value is used regardless of the umask specified
   by PAM or login.conf.  However, if the umask is not explicitly
   set in sudoers, PAM, or login.conf may now override the default
   sudoers umask.  Previously, the sudoers umask always overrode
   the umask set by PAM, which was not the documented behavior.

 * Upgrading from a version prior to 1.8.28:

   Starting with version 1.8.28, sudo stores the signal that caused
   a command to be suspended or resumed as a string in the I/O log
   timing file.  The version of sudoreplay included with sudo
   1.8.28 can process either type of I/O log file but older versions
   of sudoreplay are unable to replay the newer logs.

   Starting with version 1.8.28, sudoedit honors the umask and
   umask_override settings in sudoers.  Previously, the user's
   umask was used as-is.

 * Upgrading from a version prior to 1.8.26:

   Starting with version 1.8.26, sudo no long sets the USERNAME
   environment variable when running commands. This is a non-standard
   environment variable that was set on some older Linux systems.
   Sudo still sets the LOGNAME, USER, and, on AIX systems, LOGIN
   environment variables.

   Handling of the LOGNAME, USER (and on AIX, LOGIN) environment
   variables has changed slightly in version 1.8.26.  Sudo now
   treats those variables as a single unit.  This means that if
   one variable is preserved or removed from the environment using
   env_keep, env_check, or env_delete, the others are too.

 * Upgrading from a version prior to 1.8.23:

   In sudo 1.8.23 the "sudoers2ldif" script and the `visudo -x`
   functionality has been superseded by the "cvtsudoers" utility.
   The cvtsudoers utility is intended to be a drop-in replacement
   for "sudoers2ldif".  Because it uses the same parser as sudo
   and visudo, cvtsudoers can perform a more accurate conversion
   than sudoers2ldif could.

   To convert a sudoers file to JSON, the format option must be
   specified.  For example, instead of:

   visudo -f sudoers_file -x output_file

   one would use:

   cvtsudoers -f json -o output_file sudoers_file

   Unlike "visudo -x", "cvtsudoers" reads from the standard input
   by default.  Also, the base DN may be specified on the command
   line, if desired, using the -b option.

 * Upgrading from a version prior to 1.8.20:

   Due to the addition of the TIMEOUT, NOTBEFORE, and NOTAFTTER
   options, it is no longer possible to declare an alias with one
   of those names.  If a sudoers file has an alias with one of
   those names, sudo, and visudo will report a syntax error with a
   message like "syntax error: unexpected TIMEOUT, expecting ALIAS".

   Prior to version 1.8.20, when log_input, log_output, or use_pty
   were enabled, if any of the standard input, output, or error
   were not connected to a terminal, sudo would use a pipe.  The
   pipe allows sudo to interpose itself between the old standard
   input, output, or error and log the contents.  Beginning with
   version 1.8.20, a pipe is only used when I/O logging is enabled.
   If use_pty is set without log_input or log_output, no pipe will
   be used.  Additionally, if log_input is set without log_output,
   a pipe is only used for the standard input.  Likewise, if
   log_output is set without log_input, a pipe is only used for
   the standard output and standard error.  This results in a
   noticeable change in behavior if the use_pty flag is set and no
   terminal is present when running commands such as scripts that
   execute other commands asynchronously (in the background).
   Previously, sudo would exit immediately, causing background
   commands to terminate with a broken pipe if they attempt to
   write to the standard output or standard error.  As of version
   1.8.20, a pipe will not be used in this case so the command
   will no longer be terminated.

 * Upgrading from a version prior to 1.8.16:

   When editing files with sudoedit, files in a directory that is
   writable by the invoking user may no longer be edited by default.
   Also, sudoedit will refuse to follow a symbolic link in the
   path to be edited if that directory containing the link is
   writable by the user.  This behavior can be disabled by negating
   the sudoedit_checkdir sudoers option, which is now enabled by
   default.

 * Upgrading from a version prior to 1.8.15:

   Prior to version 1.8.15, when env_reset was enabled (the default)
   and the -s option was not used, the SHELL environment variable
   was set to the shell of the invoking user.  In 1.8.15 and above,
   when env_reset is enabled and the -s option is not used, SHELL
   is set based on the target user.

   When editing files with sudoedit, symbolic links will no longer
   be followed by default.  The old behavior can be restored by
   enabling the sudoedit_follow option in sudoers or on a per-command
   basis with the FOLLOW and NOFOLLOW tags.

   Prior to version 1.8.15, groups listed in sudoers that were not
   found in the system group database were passed to the group
   plugin, if any.  Starting with 1.8.15, only groups of the form
   %:group are resolved via the group plugin by default.  The old
   behavior can be restored by using the always_query_group_plugin
   sudoers option.

   Locking of the time stamp file has changed in sudo 1.8.15.
   Previously, the user's entire time stamp file was locked while
   retrieving and updating a time stamp record.  Now, only a single
   record, specific to the tty or parent process ID, is locked.
   This lock is held while the user enters their password.  If
   sudo is suspended at the password prompt (or run in the
   background), the lock is dropped until sudo is resumed, at which
   point it will be reacquired.  This allows sudo to be used in a
   pipeline even when a password is required--only one instance
   of sudo will prompt for a password.

 * Upgrading from a version prior to 1.8.14:

   On HP-UX, sudo will no longer check for "plugin.sl" if "plugin.so"
   is specified but does not exist.  This was a temporary hack for
   backward compatibility with Sudo 1.8.6 and below when the
   plugin path name was not listed in sudo.conf.  A plugin path
   name that explicitly ends in ".sl" will still work as expected.

 * Upgrading from a version prior to 1.8.12:

   On Solaris, sudo is now able to determine the NIS domain name.
   As a result, if you had previously been using netgroups that
   do not include the domain, you will need to either set the
   domain in the entry or leave the domain part of the tuple blank.

   For example, the following will no longer work:

        my-hosts (foo,-,-) (bar,-,-) (baz,-,-)

   and should be changed to:

        my-hosts (foo,-,) (bar,-,) (baz,-,)

 * Upgrading from a version prior to 1.8.10:

   The time stamp file format has changed in sudo 1.8.10.  There
   is now a single time stamp file for each user, even when tty-based
   time stamps are used.  Each time stamp file may contain multiple
   records to support tty-based time stamps as well as multiple
   authentication users.  On systems that support it, monotonic
   time is stored instead of wall clock time.  As a result, it is
   important that the time stamp files not persist when the system
   reboots.  For this reason, the default location for the time
   stamp files has changed back to a directory located in `/var/run`.
   Systems that do not have `/var/run` (e.g. AIX) or that do not clear
   it on boot (e.g. HP-UX) will need to clear the time stamp
   directory via a start up script.  Such a script is installed by
   default on AIX and HP-UX systems.

   Because there is now a single time stamp file per user, the -K
   option will remove all of the user's time stamps, not just the
   time stamp for the current terminal.

   Lecture status is now stored separately from the time stamps in a
   separate directory: `/var/db/sudo/lectured`, `/var/lib/sudo/lectured`
   or `/var/adm/sudo/lectured` depending on what is present on the system.

   LDAP-based sudoers now uses a default search filter of
   (objectClass=sudoRole) for more efficient queries.  It is
   possible to disable the default search filter by specifying
   SUDOERS_SEARCH_FILTER in ldap.conf but omitting a value.

 * Upgrading from a version prior to 1.8.7:

   Sudo now stores its libexec files in a "sudo" sub-directory
   instead of in libexec itself.  For backward compatibility, if
   the plugin is not found in the default plugin directory, sudo
   will check the parent directory default directory ends in `/sudo`.

   The default sudo plugins now all use the .so extension, regardless
   of the extension used by system shared libraries.  For backward
   compatibility, sudo on HP-UX will also search for a plugin with
   an .sl extension if the .so version is not found.

   Handling of users belonging to a large number of groups has
   changed.  Previously, sudo would only use the group list from
   the kernel unless the system_group plugin was enabled in sudoers.
   Now, sudo will query the groups database if the user belongs
   to the maximum number of groups supported by the kernel.  See
   the group_source and max_groups settings in the sudo.conf manual
   for details.

 * Upgrading from a version prior to 1.8.2:

   When matching Unix groups in the sudoers file, sudo will now
   match based on the name of the group as it appears in sudoers
   instead of the group-ID.  This can substantially reduce the
   number of group lookups for sudoers files that contain a large
   number of groups.  There are a few side effects of this change.

   1) Unix groups with different names but the same group-ID are
      can no longer be used interchangeably.  Sudo will look up all
      of a user's groups by group-ID and use the resulting group
      names when matching sudoers entries.  If there are multiple
      groups with the same ID, the group name returned by the
      system getgrgid() library function is the name that will be
      used when matching sudoers entries.

   2) Unix group names specified in the sudoers file that are
      longer than the system maximum will no longer match.  For
      instance, if there is a Unix group "fireflie" on a system
      where group names are limited to eight characters, "%fireflies"
      in sudoers will no longer match "fireflie".  Previously, a
      lookup by name of the group "fireflies" would have matched
      the "fireflie" group on most systems.

   The legacy group matching behavior may be restored by enabling
   the match_group_by_gid Defaults option in sudoers available
   in sudo 1.8.18 and higher.

 * Upgrading from a version prior to 1.8.1:

   Changes in the sudoers parser could result in parse errors for
   existing sudoers file.  These changes cause certain erroneous
   entries to be flagged as errors where before they allowed.
   Changes include:

   Combining multiple Defaults entries with a backslash.  E.g.

        Defaults set_path \
        Defaults syslog

   which should be:

        Defaults set_path
        Defaults syslog

   Also, double-quoted strings with a missing end-quote are now
   detected and result in an error.  Previously, text starting a
   double quote and ending with a newline was ignored.  E.g.

        Defaults set_path"foo

   In previous versions of sudo, the _"foo_ portion would have
   been ignored.

   To avoid problems, sudo 1.8.1's `make install` will not install
   a new sudo binary if the existing sudoers file has errors.

   In Sudo 1.8.1 the _noexec_ functionality has moved out of the
   sudoers policy plugin and into the sudo front-end.  As a result,
   the path to the noexec file is now specified in the sudo.conf
   file instead of the sudoers file.  If you have a sudoers file
   that uses the "noexec_file" option, you will need to move the
   definition to the sudo.conf file instead.

   Old style in `/etc/sudoers`:

        Defaults noexec_file=/usr/local/libexec/sudo_noexec.so

   New style in `/etc/sudo.conf`:

        Path noexec /usr/local/libexec/sudo_noexec.so

 * Upgrading from a version prior to 1.8.0:

   Starting with version 1.8.0, sudo uses a modular framework to
   support policy and I/O logging plugins.  The default policy
   plugin is "sudoers" which provides the traditional sudoers
   evaluation and I/O logging.  Plugins are typically located in
   `/usr/libexec` or `/usr/local/libexec`, though this is system-dependent.
   The sudoers plugin is named "sudoers.so" on most systems.

   The sudo.conf file, usually stored in `/etc`, is used to configure
   plugins.  This file is optional--if no plugins are specified
   in sudo.conf, the "sudoers" plugin is used.  See the example
   sudo.conf file in the docs directory or refer to the updated
   sudo manual to see how to configure sudo.conf.

   The "askpass" setting has moved from the sudoers file to the
   sudo.conf file.  If you have a sudoers file that uses the
   "askpass" option, you will need to move the definition to the
   sudo.conf file.

   Old style in `/etc/sudoers`:

        Defaults askpass=/usr/X11R6/bin/ssh-askpass

   New style in `/etc/sudo.conf`:

        Path askpass /usr/X11R6/bin/ssh-askpass

 * Upgrading from a version prior to 1.7.5:

   Sudo 1.7.5 includes an updated LDAP schema with support for
   the sudoNotBefore, sudoNotAfter, and sudoOrder attributes.

   The sudoNotBefore and sudoNotAfter attribute support is only
   used when the SUDOERS_TIMED setting is enabled in ldap.conf.
   If enabled, those attributes are used directly when constructing
   an LDAP filter.  As a result, your LDAP server must have the
   updated schema if you want to use sudoNotBefore and sudoNotAfter.

   The sudoOrder support does not affect the LDAP filter sudo
   constructs and so there is no need to explicitly enable it in
   ldap.conf.  If the sudoOrder attribute is not present in an
   entry, a value of 0 is used.  If no entries contain sudoOrder
   attributes, the results are in whatever order the LDAP server
   returns them, as in past versions of sudo.

   Older versions of sudo will simply ignore the new attributes
   if they are present in an entry.  There are no compatibility
   problems using the updated schema with older versions of sudo.

 * Upgrading from a version prior to 1.7.4:

   Starting with sudo 1.7.4, the time stamp files have moved from
   `/var/run/sudo` to either `/var/db/sudo`, `/var/lib/sudo`, or
   `/var/adm/sudo`.  The directories are checked for existence in
   that order.  This prevents users from receiving the sudo lecture
   every time the system reboots.  Time stamp files older than the
   boot time are ignored on systems where it is possible to determine
   this.

   Additionally, the tty_tickets sudoers option is now enabled by
   default.  To restore the old behavior (single time stamp per user),
   add a line like:

        Defaults !tty_tickets

   to sudoers or use the `--without-tty-tickets` configure option.

   The HOME and MAIL environment variables are now reset based on the
   target user's password database entry when the env_reset sudoers option
   is enabled (which is the case in the default configuration).  Users
   wishing to preserve the original values should use a sudoers entry like:

        Defaults env_keep += HOME

   to preserve the old value of HOME and

        Defaults env_keep += MAIL

   to preserve the old value of MAIL.

   Preserving HOME has security implications since many programs
   use it when searching for configuration files.  Adding HOME to
   env_keep may enable a user to run unrestricted commands via sudo.

   The default syslog facility has changed from "local2" to "authpriv"
   (or "auth" if the operating system doesn't have "authpriv").
   The `--with-logfac` configure option can be used to change this
   or it can be changed in the sudoers file.

 * Upgrading from a version prior to 1.7.0:

   Starting with sudo 1.7.0, comments in the sudoers file must not
   have a digit or minus sign immediately after the comment character
   ('#').  Otherwise, the comment may be interpreted as a user or
   group-ID.

   When sudo is build with LDAP support the `/etc/nsswitch.conf` file is
   now used to determine the sudoers sea ch order.  sudo will default to
   only using `/etc/sudoers` unless `/etc/nsswitch.conf` says otherwise.
   This can be changed with an nsswitch.conf line, e.g.:

       sudoers: ldap files

   Would case LDAP to be searched first, then the sudoers file.
   To restore the pre-1.7.0 behavior, run configure with the
   `--with-nsswitch=no` flag.

   Sudo now ignores user .ldaprc files as well as system LDAP defaults.
   All LDAP configuration is now in `/etc/ldap.conf` (or whichever file
   was specified by configure's `--with-ldap-conf-file` option).
   If you are using TLS, you may now need to specify:

        tls_checkpeer no

   in sudo's ldap.conf unless ldap.conf references a valid certificate
   authority file(s).

 * Upgrading from a version prior to 1.6.9:

   Starting with sudo 1.6.9, if an OS supports a modular authentication
   method such as PAM, it will be used by default by configure.

   Environment variable handling has changed significantly in sudo
   1.6.9.  Prior to version 1.6.9, sudo would preserve the user's
   environment, pruning out potentially dangerous variables.
   Beginning with sudo 1.6.9, the environment is reset to a default
   set of values with only a small number of "safe" variables
   preserved.  To preserve specific environment variables, add
   them to the "env_keep" list in sudoers.  E.g.

        Defaults env_keep += "EDITOR"

   The old behavior can be restored by negating the "env_reset"
   option in sudoers.  E.g.

        Defaults !env_reset

   There have  also been changes to how the "env_keep" and
   "env_check" options behave.

   Prior to sudo 1.6.9, the TERM and PATH environment variables
   would always be preserved even if the env_keep option was
   redefined.  That is no longer the case.  Consequently, if
   env_keep is set with "=" and not simply appended to (i.e. using
   "+="), PATH and TERM must be explicitly included in the list
   of environment variables to keep.  The LOGNAME, SHELL, USER,
   and USERNAME environment variables are still always set.

   Additionally, the env_check setting previously had no effect
   when env_reset was set (which is now on by default).  Starting
   with sudo 1.6.9, environment variables listed in env_check are
   also preserved in the env_reset case, provided that they do not
   contain a '/' or '%' character.  It is not necessary to also
   list a variable in env_keep--having it in env_check is sufficient.

   The default lists of variables to be preserved and/or checked
   are displayed when sudo is run by root with the -V flag.

 * Upgrading from a version prior to 1.6.8:

   Prior to sudo 1.6.8, if `/var/run` did not exist, sudo would put
   the time stamp files in `/tmp/.odus`.  As of sudo 1.6.8, the
   time stamp files will be placed in `/var/adm/sudo` or `/usr/adm/sudo`
   if there is no `/var/run directory`.  This directory will be
   created if it does not already exist.

   Previously, a sudoers entry that explicitly prohibited running
   a command as a certain user did not override a previous entry
   allowing the same command.  This has been fixed in sudo 1.6.8
   such that the last match is now used (as it is documented).
   Hopefully no one was depending on the previous (buggy) behavior.

 * Upgrading from a version prior to 1.6:

   As of sudo 1.6, parsing of runas entries and the NOPASSWD tag
   has changed.  Prior to 1.6, a runas specifier applied only to
   a single command directly following it.  Likewise, the NOPASSWD
   tag only allowed the command directly following it to be run
   without a password.  Starting with sudo 1.6, both the runas
   specifier and the NOPASSWD tag are "sticky" for an entire
   command list.  So, given the following line in sudo < 1.6

        millert ALL=(daemon) NOPASSWD:/usr/bin/whoami,/bin/ls

   millert would be able to run `/usr/bin/whoami` as user daemon
   without a password and `/bin/ls` as root with a password.

   As of sudo 1.6, the same line now means that millert is able
   to run run both `/usr/bin/whoami` and `/bin/ls` as user daemon
   without a password.  To expand on this, take the following
   example:

        millert ALL=(daemon) NOPASSWD:/usr/bin/whoami, (root) /bin/ls, \
            /sbin/dump

   millert can run `/usr/bin/whoami` as daemon and `/bin/ls` and
   `/sbin/dump` as root.  No password need be given for either
   command.  In other words, the "(root)" sets the default runas
   user to root for the rest of the list.  If we wanted to require
   a password for `/bin/ls` and `/sbin/dump` the line could be written
   as:

        millert ALL=(daemon) NOPASSWD:/usr/bin/whoami, \
            (root) PASSWD:/bin/ls, /sbin/dump

   Additionally, sudo now uses a per-user time stamp directory
   instead of a time stamp file.  This allows tty time stamps to
   simply be files within the user's time stamp dir.  For the
   default, non-tty case, the time stamp on the directory itself
   is used.

   Also, the temporary file used by visudo is now `/etc/sudoers.tmp`
   since some versions of vipw on systems with shadow passwords use
   `/etc/stmp` for the temporary shadow file.

 * Upgrading from a version prior to 1.5:

   By default, sudo expects the sudoers file to be mode 0440 and
   to be owned by user and group 0.  This differs from version 1.4
   and below which expected the sudoers file to be mode 0400 and
   to be owned by root.  Doing a `make install` will set the sudoers
   file to the new mode and group.  If sudo encounters a sudoers
   file with the old permissions it will attempt to update it to
   the new scheme.  You cannot, however, use a sudoers file with
   the new permissions with an old sudo binary.  It is suggested
   that if have a means of distributing sudo you distribute the
   new binaries first, then the new sudoers file (or you can leave
   sudoers as is and sudo will fix the permissions itself as long
   as sudoers is on a local file system).
