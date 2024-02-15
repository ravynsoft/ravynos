import sudo

import logging


class DebugDemoPlugin(sudo.Plugin):
    """
    An example sudo plugin demonstrating the debugging capabilities.

    You can install it as an extra IO plugin for example by adding the
    following line to sudo.conf:
        Plugin python_io python_plugin.so \
            ModulePath=<path>/example_debugging.py \
            ClassName=DebugDemoPlugin

    To see the plugin's debug output, use the following line in sudo.conf:
        Debug python_plugin.so \
            /var/log/sudo_python_debug plugin@trace,c_calls@trace
            ^                          ^-- the options for the logging
            ^----- the output will be placed here

    The options for the logging is in format of multiple "subsystem@level"
    separated by commas (",").
    The most interesting subsystems are:
        plugin      Shows each call of sudo.debug API in the log
      - py_calls    Logs whenever a C function calls into the python module.
                    (For example calling this __init__ function.)
        c_calls     Logs whenever python calls into a C sudo API function

    You can also specify "all" as subsystem name to get the debug messages of
    all subsystems.

    Other subsystems available:
        internal    logs internal functions of the python language wrapper
        sudo_cb     logs when sudo calls into its plugin API
        load        logs python plugin loading / unloading

    Log levels
        crit      sudo.DEBUG.CRIT       --> only critical messages
        err       sudo.DEBUG.ERROR
        warn      sudo.DEBUG.WARN
        notice    sudo.DEBUG.NOTICE
        diag      sudo.DEBUG.DIAG
        info      sudo.DEBUG.INFO
        trace     sudo.DEBUG.TRACE
        debug     sudo.DEBUG.DEBUG      --> very extreme verbose debugging

    See the sudo.conf manual for more details ("man sudo.conf").

    """

    def __init__(self, plugin_options, **kwargs):
        # Specify: "py_calls@info" debug option to show the call to this
        # constructor and the arguments passed in

        # Specifying "plugin@err" debug option will show this message
        # (or any more verbose level)
        sudo.debug(sudo.DEBUG.ERROR, "My demo purpose plugin shows "
                   "this ERROR level debug message")

        # Specifying "plugin@info" debug option will show this message
        # (or any more verbose level)
        sudo.debug(sudo.DEBUG.INFO, "My demo purpose plugin shows "
                   "this INFO level debug message")

        # You can also use python log system, because sudo sets its log handler
        # on the root logger.
        # Note that the level of python logging is separate than the one set in
        # sudo.conf. If using the python logger, each will have effect.
        logger = logging.getLogger()
        logger.setLevel(logging.INFO)
        logger.error("Python log system shows this ERROR level debug message")
        logger.info("Python log system shows this INFO level debug message")

        # If you raise the level to info or below, the call of the debug
        # will also be logged.
        # An example output you will see in the debug log file:
        #   Dec  5 15:19:19 sudo[123040] __init__ @ /.../example_debugging.py:54 debugs:
        #   Dec  5 15:19:19 sudo[123040] My demo purpose plugin shows this ERROR level debug message

        # Specify: "c_calls@diag" debug option to show this call and its
        # arguments. If you specify info debug level instead ("c_calls@info"),
        # you will also see the python function and line from which you called
        # the 'options_as_dict' function.
        self.plugin_options = sudo.options_as_dict(plugin_options)
