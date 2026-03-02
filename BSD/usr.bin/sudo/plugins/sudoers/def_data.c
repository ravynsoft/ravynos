/* generated file, do not edit */

static struct def_values def_data_lecture[] = {
    { "never", never },
    { "once", once },
    { "always", always },
    { NULL, 0 },
};

static struct def_values def_data_listpw[] = {
    { "never", never },
    { "any", any },
    { "all", all },
    { "always", always },
    { NULL, 0 },
};

static struct def_values def_data_verifypw[] = {
    { "never", never },
    { "all", all },
    { "any", any },
    { "always", always },
    { NULL, 0 },
};

static struct def_values def_data_fdexec[] = {
    { "never", never },
    { "digest_only", digest_only },
    { "always", always },
    { NULL, 0 },
};

static struct def_values def_data_timestamp_type[] = {
    { "global", global },
    { "ppid", ppid },
    { "tty", tty },
    { "kernel", kernel },
    { NULL, 0 },
};

static struct def_values def_data_log_format[] = {
    { "sudo", sudo },
    { "json", json },
    { NULL, 0 },
};

static struct def_values def_data_intercept_type[] = {
    { "dso", dso },
    { "trace", trace },
    { NULL, 0 },
};

struct sudo_defs_types sudo_defs_table[] = {
    {
	"syslog", T_LOGFAC|T_BOOL,
	N_("Syslog facility if syslog is being used for logging: %s"),
	NULL,
    }, {
	"syslog_goodpri", T_LOGPRI|T_BOOL,
	N_("Syslog priority to use when user authenticates successfully: %s"),
	NULL,
    }, {
	"syslog_badpri", T_LOGPRI|T_BOOL,
	N_("Syslog priority to use when user authenticates unsuccessfully: %s"),
	NULL,
    }, {
	"long_otp_prompt", T_FLAG,
	N_("Put OTP prompt on its own line"),
	NULL,
    }, {
	"ignore_dot", T_FLAG,
	N_("Ignore '.' in $PATH"),
	NULL,
    }, {
	"mail_always", T_FLAG,
	N_("Always send mail when sudo is run"),
	NULL,
    }, {
	"mail_badpass", T_FLAG,
	N_("Send mail if user authentication fails"),
	NULL,
    }, {
	"mail_no_user", T_FLAG,
	N_("Send mail if the user is not in sudoers"),
	NULL,
    }, {
	"mail_no_host", T_FLAG,
	N_("Send mail if the user is not in sudoers for this host"),
	NULL,
    }, {
	"mail_no_perms", T_FLAG,
	N_("Send mail if the user is not allowed to run a command"),
	NULL,
    }, {
	"mail_all_cmnds", T_FLAG,
	N_("Send mail if the user tries to run a command"),
	NULL,
    }, {
	"tty_tickets", T_FLAG,
	N_("Use a separate timestamp for each user/tty combo"),
	NULL,
    }, {
	"lecture", T_TUPLE|T_BOOL,
	N_("Lecture user the first time they run sudo"),
	def_data_lecture,
    }, {
	"lecture_file", T_STR|T_PATH|T_BOOL,
	N_("File containing the sudo lecture: %s"),
	NULL,
    }, {
	"authenticate", T_FLAG,
	N_("Require users to authenticate by default"),
	NULL,
    }, {
	"root_sudo", T_FLAG,
	N_("Root may run sudo"),
	NULL,
    }, {
	"log_host", T_FLAG,
	N_("Log the hostname in the (non-syslog) log file"),
	NULL,
    }, {
	"log_year", T_FLAG,
	N_("Log the year in the (non-syslog) log file"),
	NULL,
    }, {
	"shell_noargs", T_FLAG,
	N_("If sudo is invoked with no arguments, start a shell"),
	NULL,
    }, {
	"set_home", T_FLAG,
	N_("Set $HOME to the target user when starting a shell with -s"),
	NULL,
    }, {
	"always_set_home", T_FLAG,
	N_("Always set $HOME to the target user's home directory"),
	NULL,
    }, {
	"path_info", T_FLAG,
	N_("Allow some information gathering to give useful error messages"),
	NULL,
    }, {
	"fqdn", T_FLAG,
	N_("Require fully-qualified hostnames in the sudoers file"),
	NULL,
    }, {
	"insults", T_FLAG,
	N_("Insult the user when they enter an incorrect password"),
	NULL,
    }, {
	"requiretty", T_FLAG,
	N_("Only allow the user to run sudo if they have a tty"),
	NULL,
    }, {
	"env_editor", T_FLAG,
	N_("Visudo will honor the EDITOR environment variable"),
	NULL,
    }, {
	"rootpw", T_FLAG,
	N_("Prompt for root's password, not the users's"),
	NULL,
    }, {
	"runaspw", T_FLAG,
	N_("Prompt for the runas_default user's password, not the users's"),
	NULL,
    }, {
	"targetpw", T_FLAG,
	N_("Prompt for the target user's password, not the users's"),
	NULL,
    }, {
	"use_loginclass", T_FLAG,
	N_("Apply defaults in the target user's login class if there is one"),
	NULL,
    }, {
	"set_logname", T_FLAG,
	N_("Set the LOGNAME and USER environment variables"),
	NULL,
    }, {
	"stay_setuid", T_FLAG,
	N_("Only set the effective uid to the target user, not the real uid"),
	NULL,
    }, {
	"preserve_groups", T_FLAG,
	N_("Don't initialize the group vector to that of the target user"),
	NULL,
    }, {
	"loglinelen", T_UINT|T_BOOL,
	N_("Length at which to wrap log file lines (0 for no wrap): %u"),
	NULL,
    }, {
	"timestamp_timeout", T_TIMESPEC|T_BOOL,
	N_("Authentication timestamp timeout: %.1f minutes"),
	NULL,
    }, {
	"passwd_timeout", T_TIMESPEC|T_BOOL,
	N_("Password prompt timeout: %.1f minutes"),
	NULL,
    }, {
	"passwd_tries", T_UINT,
	N_("Number of tries to enter a password: %u"),
	NULL,
    }, {
	"umask", T_MODE|T_BOOL,
	N_("Umask to use or 0777 to use user's: 0%o"),
	NULL,
    }, {
	"logfile", T_STR|T_BOOL|T_PATH,
	N_("Path to log file: %s"),
	NULL,
    }, {
	"mailerpath", T_STR|T_BOOL|T_PATH,
	N_("Path to mail program: %s"),
	NULL,
    }, {
	"mailerflags", T_STR|T_BOOL,
	N_("Flags for mail program: %s"),
	NULL,
    }, {
	"mailto", T_STR|T_BOOL,
	N_("Address to send mail to: %s"),
	NULL,
    }, {
	"mailfrom", T_STR|T_BOOL,
	N_("Address to send mail from: %s"),
	NULL,
    }, {
	"mailsub", T_STR,
	N_("Subject line for mail messages: %s"),
	NULL,
    }, {
	"badpass_message", T_STR,
	N_("Incorrect password message: %s"),
	NULL,
    }, {
	"lecture_status_dir", T_STR|T_PATH,
	N_("Path to lecture status dir: %s"),
	NULL,
    }, {
	"timestampdir", T_STR|T_PATH,
	N_("Path to authentication timestamp dir: %s"),
	NULL,
    }, {
	"timestampowner", T_STR,
	N_("Owner of the authentication timestamp dir: %s"),
	NULL,
    }, {
	"exempt_group", T_STR|T_BOOL,
	N_("Users in this group are exempt from password and PATH requirements: %s"),
	NULL,
    }, {
	"passprompt", T_STR,
	N_("Default password prompt: %s"),
	NULL,
    }, {
	"passprompt_override", T_FLAG,
	N_("If set, passprompt will override system prompt in all cases."),
	NULL,
    }, {
	"runas_default", T_STR,
	N_("Default user to run commands as: %s"),
	NULL,
    }, {
	"secure_path", T_STR|T_BOOL,
	N_("Value to override user's $PATH with: %s"),
	NULL,
    }, {
	"editor", T_STR|T_PATH,
	N_("Path to the editor for use by visudo: %s"),
	NULL,
    }, {
	"listpw", T_TUPLE|T_BOOL,
	N_("When to require a password for 'list' pseudocommand: %s"),
	def_data_listpw,
    }, {
	"verifypw", T_TUPLE|T_BOOL,
	N_("When to require a password for 'verify' pseudocommand: %s"),
	def_data_verifypw,
    }, {
	"noexec", T_FLAG,
	N_("Preload the sudo_noexec library which replaces the exec functions"),
	NULL,
    }, {
	"ignore_local_sudoers", T_FLAG,
	N_("If LDAP directory is up, do we ignore local sudoers file"),
	NULL,
    }, {
	"closefrom", T_INT,
	N_("File descriptors >= %d will be closed before executing a command"),
	NULL,
    }, {
	"closefrom_override", T_FLAG,
	N_("If set, users may override the value of \"closefrom\" with the -C option"),
	NULL,
    }, {
	"setenv", T_FLAG,
	N_("Allow users to set arbitrary environment variables"),
	NULL,
    }, {
	"env_reset", T_FLAG,
	N_("Reset the environment to a default set of variables"),
	NULL,
    }, {
	"env_check", T_LIST|T_BOOL,
	N_("Environment variables to check for safety:"),
	NULL,
    }, {
	"env_delete", T_LIST|T_BOOL,
	N_("Environment variables to remove:"),
	NULL,
    }, {
	"env_keep", T_LIST|T_BOOL,
	N_("Environment variables to preserve:"),
	NULL,
    }, {
	"role", T_STR,
	N_("SELinux role to use in the new security context: %s"),
	NULL,
    }, {
	"type", T_STR,
	N_("SELinux type to use in the new security context: %s"),
	NULL,
    }, {
	"env_file", T_STR|T_PATH|T_BOOL,
	N_("Path to the sudo-specific environment file: %s"),
	NULL,
    }, {
	"restricted_env_file", T_STR|T_PATH|T_BOOL,
	N_("Path to the restricted sudo-specific environment file: %s"),
	NULL,
    }, {
	"sudoers_locale", T_STR,
	N_("Locale to use while parsing sudoers: %s"),
	NULL,
    }, {
	"visiblepw", T_FLAG,
	N_("Allow sudo to prompt for a password even if it would be visible"),
	NULL,
    }, {
	"pwfeedback", T_FLAG,
	N_("Provide visual feedback at the password prompt when there is user input"),
	NULL,
    }, {
	"fast_glob", T_FLAG,
	N_("Use faster globbing that is less accurate but does not access the filesystem"),
	NULL,
    }, {
	"umask_override", T_FLAG,
	N_("The umask specified in sudoers will override the user's, even if it is more permissive"),
	NULL,
    }, {
	"log_input", T_FLAG,
	N_("Log user's input for the command being run"),
	NULL,
    }, {
	"log_stdin", T_FLAG,
	N_("Log the command's standard input if not connected to a terminal"),
	NULL,
    }, {
	"log_ttyin", T_FLAG,
	N_("Log the user's terminal input for the command being run"),
	NULL,
    }, {
	"log_output", T_FLAG,
	N_("Log the output of the command being run"),
	NULL,
    }, {
	"log_stdout", T_FLAG,
	N_("Log the command's standard output if not connected to a terminal"),
	NULL,
    }, {
	"log_stderr", T_FLAG,
	N_("Log the command's standard error if not connected to a terminal"),
	NULL,
    }, {
	"log_ttyout", T_FLAG,
	N_("Log the terminal output of the command being run"),
	NULL,
    }, {
	"compress_io", T_FLAG,
	N_("Compress I/O logs using zlib"),
	NULL,
    }, {
	"use_pty", T_FLAG,
	N_("Always run commands in a pseudo-tty"),
	NULL,
    }, {
	"group_plugin", T_STR,
	N_("Plugin for non-Unix group support: %s"),
	NULL,
    }, {
	"iolog_dir", T_STR|T_PATH,
	N_("Directory in which to store input/output logs: %s"),
	NULL,
    }, {
	"iolog_file", T_STR,
	N_("File in which to store the input/output log: %s"),
	NULL,
    }, {
	"set_utmp", T_FLAG,
	N_("Add an entry to the utmp/utmpx file when allocating a pty"),
	NULL,
    }, {
	"utmp_runas", T_FLAG,
	N_("Set the user in utmp to the runas user, not the invoking user"),
	NULL,
    }, {
	"privs", T_STR,
	N_("Set of permitted privileges: %s"),
	NULL,
    }, {
	"limitprivs", T_STR,
	N_("Set of limit privileges: %s"),
	NULL,
    }, {
	"exec_background", T_FLAG,
	N_("Run commands on a pty in the background"),
	NULL,
    }, {
	"pam_service", T_STR,
	N_("PAM service name to use: %s"),
	NULL,
    }, {
	"pam_login_service", T_STR,
	N_("PAM service name to use for login shells: %s"),
	NULL,
    }, {
	"pam_askpass_service", T_STR,
	N_("PAM service name to use when sudo is run with the -A option: %s"),
	NULL,
    }, {
	"pam_setcred", T_FLAG,
	N_("Attempt to establish PAM credentials for the target user"),
	NULL,
    }, {
	"pam_session", T_FLAG,
	N_("Create a new PAM session for the command to run in"),
	NULL,
    }, {
	"pam_acct_mgmt", T_FLAG,
	N_("Perform PAM account validation management"),
	NULL,
    }, {
	"maxseq", T_STR,
	N_("Maximum I/O log sequence number: %s"),
	NULL,
    }, {
	"use_netgroups", T_FLAG,
	N_("Enable sudoers netgroup support"),
	NULL,
    }, {
	"sudoedit_checkdir", T_FLAG,
	N_("Check parent directories for writability when editing files with sudoedit"),
	NULL,
    }, {
	"sudoedit_follow", T_FLAG,
	N_("Follow symbolic links when editing files with sudoedit"),
	NULL,
    }, {
	"always_query_group_plugin", T_FLAG,
	N_("Query the group plugin for unknown system groups"),
	NULL,
    }, {
	"netgroup_tuple", T_FLAG,
	N_("Match netgroups based on the entire tuple: user, host and domain"),
	NULL,
    }, {
	"ignore_audit_errors", T_FLAG,
	N_("Allow commands to be run even if sudo cannot write to the audit log"),
	NULL,
    }, {
	"ignore_iolog_errors", T_FLAG,
	N_("Allow commands to be run even if sudo cannot write to the I/O log"),
	NULL,
    }, {
	"ignore_logfile_errors", T_FLAG,
	N_("Allow commands to be run even if sudo cannot write to the log file"),
	NULL,
    }, {
	"match_group_by_gid", T_FLAG,
	N_("Resolve groups in sudoers and match on the group ID, not the name"),
	NULL,
    }, {
	"syslog_maxlen", T_UINT,
	N_("Log entries larger than this value will be split into multiple syslog messages: %u"),
	NULL,
    }, {
	"iolog_user", T_STR|T_BOOL,
	N_("User that will own the I/O log files: %s"),
	NULL,
    }, {
	"iolog_group", T_STR|T_BOOL,
	N_("Group that will own the I/O log files: %s"),
	NULL,
    }, {
	"iolog_mode", T_MODE,
	N_("File mode to use for the I/O log files: 0%o"),
	NULL,
    }, {
	"fdexec", T_TUPLE|T_BOOL,
	N_("Execute commands by file descriptor instead of by path: %s"),
	def_data_fdexec,
    }, {
	"ignore_unknown_defaults", T_FLAG,
	N_("Ignore unknown Defaults entries in sudoers instead of producing a warning"),
	NULL,
    }, {
	"command_timeout", T_TIMEOUT|T_BOOL,
	N_("Time in seconds after which the command will be terminated: %u"),
	NULL,
    }, {
	"user_command_timeouts", T_FLAG,
	N_("Allow the user to specify a timeout on the command line"),
	NULL,
    }, {
	"iolog_flush", T_FLAG,
	N_("Flush I/O log data to disk immediately instead of buffering it"),
	NULL,
    }, {
	"syslog_pid", T_FLAG,
	N_("Include the process ID when logging via syslog"),
	NULL,
    }, {
	"timestamp_type", T_TUPLE,
	N_("Type of authentication timestamp record: %s"),
	def_data_timestamp_type,
    }, {
	"authfail_message", T_STR,
	N_("Authentication failure message: %s"),
	NULL,
    }, {
	"case_insensitive_user", T_FLAG,
	N_("Ignore case when matching user names"),
	NULL,
    }, {
	"case_insensitive_group", T_FLAG,
	N_("Ignore case when matching group names"),
	NULL,
    }, {
	"log_allowed", T_FLAG,
	N_("Log when a command is allowed by sudoers"),
	NULL,
    }, {
	"log_denied", T_FLAG,
	N_("Log when a command is denied by sudoers"),
	NULL,
    }, {
	"log_servers", T_LIST|T_BOOL,
	N_("Sudo log server(s) to connect to with optional port"),
	NULL,
    }, {
	"log_server_timeout", T_TIMEOUT|T_BOOL,
	N_("Sudo log server timeout in seconds: %u"),
	NULL,
    }, {
	"log_server_keepalive", T_FLAG,
	N_("Enable SO_KEEPALIVE socket option on the socket connected to the logserver"),
	NULL,
    }, {
	"log_server_cabundle", T_STR|T_BOOL|T_PATH,
	N_("Path to the audit server's CA bundle file: %s"),
	NULL,
    }, {
	"log_server_peer_cert", T_STR|T_BOOL|T_PATH,
	N_("Path to the sudoers certificate file: %s"),
	NULL,
    }, {
	"log_server_peer_key", T_STR|T_BOOL|T_PATH,
	N_("Path to the sudoers private key file: %s"),
	NULL,
    }, {
	"log_server_verify", T_FLAG,
	N_("Verify that the log server's certificate is valid"),
	NULL,
    }, {
	"runas_allow_unknown_id", T_FLAG,
	N_("Allow the use of unknown runas user and/or group ID"),
	NULL,
    }, {
	"runas_check_shell", T_FLAG,
	N_("Only permit running commands as a user with a valid shell"),
	NULL,
    }, {
	"pam_ruser", T_FLAG,
	N_("Set the pam remote user to the user running sudo"),
	NULL,
    }, {
	"pam_rhost", T_FLAG,
	N_("Set the pam remote host to the local host name"),
	NULL,
    }, {
	"runcwd", T_STR|T_BOOL|T_CHPATH,
	N_("Working directory to change to before executing the command: %s"),
	NULL,
    }, {
	"runchroot", T_STR|T_BOOL|T_CHPATH,
	N_("Root directory to change to before executing the command: %s"),
	NULL,
    }, {
	"log_format", T_TUPLE,
	N_("The format of logs to produce: %s"),
	def_data_log_format,
    }, {
	"selinux", T_FLAG,
	N_("Enable SELinux RBAC support"),
	NULL,
    }, {
	"admin_flag", T_STR|T_BOOL|T_CHPATH,
	N_("Path to the file that is created the first time sudo is run: %s"),
	NULL,
    }, {
	"intercept", T_FLAG,
	N_("Intercept further commands and apply sudoers restrictions to them"),
	NULL,
    }, {
	"log_subcmds", T_FLAG,
	N_("Log sub-commands run by the original command"),
	NULL,
    }, {
	"log_exit_status", T_FLAG,
	N_("Log the exit status of commands"),
	NULL,
    }, {
	"intercept_authenticate", T_FLAG,
	N_("Subsequent commands in an intercepted session must be authenticated"),
	NULL,
    }, {
	"intercept_allow_setid", T_FLAG,
	N_("Allow an intercepted command to run set setuid or setgid programs"),
	NULL,
    }, {
	"rlimit_as", T_RLIMIT|T_BOOL,
	N_("The maximum size to which the process's address space may grow (in bytes): %s"),
	NULL,
    }, {
	"rlimit_core", T_RLIMIT|T_BOOL,
	N_("The largest size core dump file that may be created (in bytes): %s"),
	NULL,
    }, {
	"rlimit_cpu", T_RLIMIT|T_BOOL,
	N_("The maximum amount of CPU time that the process may use (in seconds): %s"),
	NULL,
    }, {
	"rlimit_data", T_RLIMIT|T_BOOL,
	N_("The maximum size of the data segment for the process (in bytes): %s"),
	NULL,
    }, {
	"rlimit_fsize", T_RLIMIT|T_BOOL,
	N_("The largest size file that the process may create (in bytes): %s"),
	NULL,
    }, {
	"rlimit_locks", T_RLIMIT|T_BOOL,
	N_("The maximum number of locks that the process may establish: %s"),
	NULL,
    }, {
	"rlimit_memlock", T_RLIMIT|T_BOOL,
	N_("The maximum size that the process may lock in memory (in bytes): %s"),
	NULL,
    }, {
	"rlimit_nofile", T_RLIMIT|T_BOOL,
	N_("The maximum number of files that the process may have open: %s"),
	NULL,
    }, {
	"rlimit_nproc", T_RLIMIT|T_BOOL,
	N_("The maximum number of processes that the user may run simultaneously: %s"),
	NULL,
    }, {
	"rlimit_rss", T_RLIMIT|T_BOOL,
	N_("The maximum size to which the process's resident set size may grow (in bytes): %s"),
	NULL,
    }, {
	"rlimit_stack", T_RLIMIT|T_BOOL,
	N_("The maximum size to which the process's stack may grow (in bytes): %s"),
	NULL,
    }, {
	"noninteractive_auth", T_FLAG,
	N_("Attempt authentication even when in non-interactive mode"),
	NULL,
    }, {
	"log_passwords", T_FLAG,
	N_("Store plaintext passwords in I/O log input"),
	NULL,
    }, {
	"passprompt_regex", T_LIST|T_SPACE|T_BOOL,
	N_("List of regular expressions to use when matching a password prompt"),
	NULL,
    }, {
	"intercept_type", T_TUPLE,
	N_("The mechanism used by the intercept and log_subcmds options: %s"),
	def_data_intercept_type,
    }, {
	"intercept_verify", T_FLAG,
	N_("Attempt to verify the command and arguments after execution"),
	NULL,
    }, {
	"apparmor_profile", T_STR,
	N_("AppArmor profile to use in the new security context: %s"),
	NULL,
    }, {
	NULL, 0, NULL
    }
};
