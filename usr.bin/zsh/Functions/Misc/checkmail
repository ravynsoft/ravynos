#!/bin/zsh
#
# This autoloadable function checks the folders specified as arguments
# for new mails.  The arguments are interpreted in exactly the same way
# as the mailpath special zsh parameter (see zshparam(1)).
#
# If no arguments are given mailpath is used.  If mailpath is empty, $MAIL
# is used and if that is also empty, /var/spool/mail/$LOGNAME is used.
# This function requires zsh-3.0.1 or newer.
#

emulate -L zsh
local file message

for file in "${@:-${mailpath[@]:-${MAIL:-/var/spool/mail/$LOGNAME}}}"
do
	message="${${(M)file%%\?*}#\?}"
	file="${file%%\?*}"
	if [[ -d "$file" ]] then
		file=( "$file"/**/*(.ND) )
		if (($#file)) then
			checkmail ${^file}\?$message
		fi
	elif test -s "$file" -a -N "$file"; then  # this also sets $_ to $file
		print -r -- "${(e)message:-You have new mail.}"
	fi
done
