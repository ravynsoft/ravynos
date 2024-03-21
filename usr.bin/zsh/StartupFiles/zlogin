#
# Generic .zlogin file for zsh 2.7
#
# .zlogin is sourced in login shells.  It should
# contain commands that should be executed only in
# login shells.  It should be used to set the terminal
# type and run a series of external commands (fortune,
# msgs, from, etc).
#

# THIS FILE IS NOT INTENDED TO BE USED AS /etc/zlogin, NOR WITHOUT EDITING
return 0	# Remove this line after editing this file as appropriate

clear
stty dec new cr0 -tabs
ttyctl -f  # freeze the terminal modes... can't change without a ttyctl -u
mesg y
uptime
fortune
log
from 2>/dev/null
cat notes
msgs -fp
