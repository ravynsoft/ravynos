import sudo

import sys

sys.path = []

class ConflictPlugin(sudo.Plugin):
    def __init__(self, plugin_options, **kwargs):
        sudo.log_info("PATH before: {} (should be empty)".format(sys.path))
        sys.path = [sudo.options_as_dict(plugin_options).get("Path")]
        sudo.log_info("PATH set: {}".format(sys.path))
