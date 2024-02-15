import sudo


class SudoGroupPlugin(sudo.Plugin):
    """Example sudo input/output plugin

    Demonstrates how to use the sudo group plugin API. Typing annotations are
    just here for the help on the syntax (requires python >= 3.5).

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

    # -- Plugin API functions --
    def query(self, user: str, group: str, user_pwd: tuple):
        """Query if user is part of the specified group.

        Beware that user_pwd can be None if user is not present in the password
        database. Otherwise it is a tuple convertible to pwd.struct_passwd.
        """
        hardcoded_user_groups = {
            "testgroup": ["testuser1", "testuser2"],
            "mygroup": ["test"]
        }

        group_has_user = user in hardcoded_user_groups.get(group, [])
        return sudo.RC.ACCEPT if group_has_user else sudo.RC.REJECT
