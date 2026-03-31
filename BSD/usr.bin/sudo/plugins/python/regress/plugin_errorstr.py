import sudo


# The purpose of this class is that all methods you call on its object
# raises a PluginError with a message containing the name of the called method.
# Eg. if you call "ErrorMsgPlugin().some_method()" it will raise
#   "Something wrong in some_method"
class ErrorMsgPlugin(sudo.Plugin):
    def __getattr__(self, name):
        def raiser_func(*args):
            raise sudo.PluginError("Something wrong in " + name)

        return raiser_func


class ConstructErrorPlugin(sudo.Plugin):
    def __init__(self, **kwargs):
        raise sudo.PluginError("Something wrong in plugin constructor")
