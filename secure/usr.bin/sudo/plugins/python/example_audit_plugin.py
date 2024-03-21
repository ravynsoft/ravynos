import sudo

import os


VERSION = 1.0


class SudoAuditPlugin(sudo.Plugin):
    def __init__(self, plugin_options, user_info, **kwargs):
        # For loading multiple times, an optional "Id" can be specified
        # as argument to identify the log lines
        plugin_id = sudo.options_as_dict(plugin_options).get("Id", "")
        self._log_line_prefix = "(AUDIT{}) ".format(plugin_id)

        user_info_dict = sudo.options_as_dict(user_info)
        user = user_info_dict.get("user", "???")
        uid = user_info_dict.get("uid", "???")
        self._log("-- Started by user {} ({}) --".format(user, uid))

    def __del__(self):
        self._log("-- Finished --")

    def open(self, submit_optind: int, submit_argv: tuple) -> int:
        # To cut out the sudo options, use "submit_optind":
        program_args = submit_argv[submit_optind:]
        if program_args:
            self._log("Requested command: " + " ".join(program_args))

    def accept(self, plugin_name, plugin_type,
               command_info, run_argv, run_envp) -> int:
        info = sudo.options_as_dict(command_info)
        cmd = list(run_argv)
        cmd[0] = info.get("command")
        self._log("Accepted command: {}".format(" ".join(cmd)))
        self._log("  By the plugin: {} (type={})".format(
            plugin_name, self.__plugin_type_str(plugin_type)))

        self._log("  Environment: " + " ".join(run_envp))

    def reject(self, plugin_name, plugin_type, audit_msg, command_info) -> int:
        self._log("Rejected by plugin {} (type={}): {}".format(
            plugin_name, self.__plugin_type_str(plugin_type), audit_msg))

    def error(self, plugin_name, plugin_type, audit_msg, command_info) -> int:
        self._log("Plugin {} (type={}) got an error: {}".format(
            plugin_name, self.__plugin_type_str(plugin_type), audit_msg))

    def close(self, status_kind: int, status: int) -> None:
        if status_kind == sudo.EXIT_REASON.NO_STATUS:
            self._log("The command was not executed")

        elif status_kind == sudo.EXIT_REASON.WAIT_STATUS:
            if os.WIFEXITED(status):
                self._log("Command returned with exit code "
                          "{}".format(os.WEXITSTATUS(status)))
            elif os.WIFSIGNALED(status):
                self._log("Command exited due to signal "
                          "{}".format(os.WTERMSIG(status)))
            else:
                raise sudo.PluginError("Failed to understand wait exit status")

        elif status_kind == sudo.EXIT_REASON.EXEC_ERROR:
            self._log("Sudo has failed to execute the command, "
                      "execve returned {}".format(status))

        elif status_kind == sudo.EXIT_REASON.SUDO_ERROR:
            self._log("Sudo has run into an error: {}".format(status))

        else:
            raise Exception("Command returned unknown status kind {}".format(
                status_kind))

    def show_version(self, is_verbose: bool) -> int:
        version_str = " (version=1.0)" if is_verbose else ""
        sudo.log_info("Python Example Audit Plugin" + version_str)

    def _log(self, string):
        # For the example, we just log to output (this could be a file)
        sudo.log_info(self._log_line_prefix, string)

    @staticmethod
    def __plugin_type_str(plugin_type):
        return sudo.PLUGIN_TYPE(plugin_type).name
