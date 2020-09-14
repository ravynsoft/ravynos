# Make Qt applications use native Gtk widgets on desktops that use Gtk
export QT_QPA_PLATFORMTHEME=gtk2
# FIXME: Does not seem to work yet
# This environment variable does not appear in env in the Xfce session?
# Is there a better way to set it?
