import sudo
import json


class ApprovalTestPlugin(sudo.Plugin):
    def __init__(self, plugin_options, **kwargs):
        id = sudo.options_as_dict(plugin_options).get("Id", "")
        super().__init__(plugin_options=plugin_options, **kwargs)
        self._id = "(APPROVAL {})".format(id)
        sudo.log_info("{} Constructed:".format(self._id))
        sudo.log_info(json.dumps(self.__dict__, indent=4, sort_keys=True))

    def __del__(self):
        sudo.log_info("{} Destructed successfully".format(self._id))

    def check(self, *args):
        sudo.log_info("{} Check was called with arguments: "
                      "{}".format(self._id, args))

    def show_version(self, *args):
        sudo.log_info("{} Show version was called with arguments: "
                      "{}".format(self._id, args))
