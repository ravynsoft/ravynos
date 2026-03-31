import sudo

import errno
import sys
import os
import pwd
import grp
import shutil


VERSION = 1.0


class SudoPolicyPlugin(sudo.Plugin):
    """Example sudo policy plugin

    Demonstrates how to use the sudo policy plugin API. All functions are added
    as an example on their syntax, but note that most of them are optional
    (except check_policy).

    On detailed description of the functions refer to sudo_plugin manual (man
    sudo_plugin).

    Most functions can express error or reject through their "int" return value
    as documented in the manual. The sudo module also has constants for these:
        sudo.RC.ACCEPT / sudo.RC.OK  1
        sudo.RC.REJECT               0
        sudo.RC.ERROR               -1
        sudo.RC.USAGE_ERROR         -2

    If the plugin encounters an error, instead of just returning sudo.RC.ERROR
    result code it can also add a message describing the problem.
    This can be done by raising the special exception:
        raise sudo.PluginError("Message")
    This added message will be used by the audit plugins.

    If the function returns "None" (for example does not call return), it will
    be considered sudo.RC.OK. If an exception other than sudo.PluginError is
    raised, its backtrace will be shown to the user and the plugin function
    returns sudo.RC.ERROR. If that is not acceptable, catch it.
    """

    _allowed_commands = ("id", "whoami")
    _safe_password = "12345"

    # -- Plugin API functions --

    def __init__(self, user_env: tuple, settings: tuple,
                 version: str, **kwargs):
        """The constructor matches the C sudo plugin API open() call

        Other variables you can currently use as arguments are:
            user_info: tuple
            plugin_options: tuple

        For their detailed description, see the open() call of the C plugin API
        in the sudo manual ("man sudo").
        """
        if not version.startswith("1."):
            raise sudo.PluginError(
                "This plugin plugin is not compatible with python plugin"
                "API version {}".format(version))

        self.user_env = sudo.options_as_dict(user_env)
        self.settings = sudo.options_as_dict(settings)

    def check_policy(self, argv: tuple, env_add: tuple):
        cmd = argv[0]
        # Example for a simple reject:
        if not self._is_command_allowed(cmd):
            sudo.log_error("You are not allowed to run this command!")
            return sudo.RC.REJECT

            raise sudo.PluginError("You are not allowed to run this command!")

        # The environment the command will be executed with (we allow any here)
        user_env_out = sudo.options_from_dict(self.user_env) + env_add

        command_info_out = sudo.options_from_dict({
            "command": self._find_on_path(cmd),  # Absolute path of command
            "runas_uid": self._runas_uid(),      # The user id
            "runas_gid": self._runas_gid(),      # The group id
        })

        return (sudo.RC.ACCEPT, command_info_out, argv, user_env_out)

    def init_session(self, user_pwd: tuple, user_env: tuple):
        """Perform session setup

        Beware that user_pwd can be None if user is not present in the password
        database. Otherwise it is a tuple convertible to pwd.struct_passwd.
        """
        # conversion example:
        user_pwd = pwd.struct_passwd(user_pwd) if user_pwd else None

        # This is how you change the user_env:
        return (sudo.RC.OK, user_env + ("PLUGIN_EXAMPLE_ENV=1",))

        # If you do not want to change user_env, you can just return (or None):
        # return sudo.RC.OK

    def list(self, argv: tuple, is_verbose: int, user: str):
        cmd = argv[0] if argv else None
        as_user_text = "as user '{}'".format(user) if user else ""

        if cmd:
            allowed_text = "" if self._is_command_allowed(cmd) else "NOT "
            sudo.log_info("You are {}allowed to execute command '{}'{}"
                          .format(allowed_text, cmd, as_user_text))

        if not cmd or is_verbose:
            sudo.log_info("Only the following commands are allowed:",
                          ", ".join(self._allowed_commands), as_user_text)

    def validate(self):
        pass  # we have no cache

    def invalidate(self, remove: int):
        pass  # we have no cache

    def show_version(self, is_verbose: int):
        sudo.log_info("Python Example Policy Plugin "
                      "version: {}".format(VERSION))
        if is_verbose:
            sudo.log_info("Python interpreter version:", sys.version)

    def close(self, exit_status: int, error: int) -> None:
        if error == 0:
            sudo.log_info("The command returned with exit_status {}".format(
                exit_status))
        else:
            error_name = errno.errorcode.get(error, "???")
            sudo.log_error(
                "Failed to execute command, execve syscall returned "
                "{} ({})".format(error, error_name))

    # -- Helper functions --

    def _is_command_allowed(self, cmd):
        return os.path.basename(cmd) in self._allowed_commands

    def _find_on_path(self, cmd):
        if os.path.isabs(cmd):
            return cmd

        path = self.user_env.get("PATH", "/usr/bin:/bin")
        absolute_cmd = shutil.which(cmd, path=path)
        if not absolute_cmd:
            raise sudo.PluginError("Can not find cmd '{}' on PATH".format(cmd))
        return absolute_cmd

    def _runas_pwd(self):
        runas_user = self.settings.get("runas_user") or "root"
        try:
            return pwd.getpwnam(runas_user)
        except KeyError:
            raise sudo.PluginError("Could not find user "
                                   "'{}'".format(runas_user))

    def _runas_uid(self):
        return self._runas_pwd().pw_uid

    def _runas_gid(self):
        runas_group = self.settings.get("runas_group")
        if runas_group is None:
            return self._runas_pwd().pw_gid

        try:
            return grp.getgrnam(runas_group).gr_gid
        except KeyError:
            raise sudo.PluginError(
                "Could not find group '{}'".format(runas_group))
