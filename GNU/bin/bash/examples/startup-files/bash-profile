# This is the filename where your incoming mail arrives.
MAIL=~/mbox
MAILCHECK=30

HISTFILE=~/.history/history.$HOSTNAME

PATH1=/usr/homes/chet/bin.$HOSTTYPE:/usr/local/bin/gnu:
PATH2=/usr/local/bin:/usr/ucb:/bin:/usr/bin/X11:.
PATH3=/usr/bin:/usr/new/bin:/usr/contrib/bin
PATH=$PATH1:$PATH2:$PATH3

EDITOR=/usr/local/bin/ce VISUAL=/usr/local/bin/ce FCEDIT=/usr/local/bin/ce

SHELL=${SHELL:-${BASH:-/bin/bash}}

PAGER=/usr/local/bin/less
LESS='-i -e -M -P%t?f%f :stdin .?pb%pb\%:?lbLine %lb:?bbByte %bb:-...'
#
# Bogus 1003.2 variables.  This should really be in /etc/profile
#
LOGNAME=${USER-$(whoami)}
TZ=US/Eastern

export HOME VISUAL EDITOR MAIL SHELL PATH TERM 
export PAGER LESS TERMCAP HISTSIZE HISTFILE MAIL MAILCHECK LOGNAME TZ

PS1="${HOSTNAME}\$ "
PS2='> '
export PS1 PS2

umask 022

if [ -f /unix ] ; then
	stty intr ^c	# bogus
fi

if [ -f ~/.bashrc ] ; then
	. ~/.bashrc
fi
