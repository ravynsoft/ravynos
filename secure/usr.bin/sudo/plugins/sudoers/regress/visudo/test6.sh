#!/bin/sh
#
# Verify parsing of Defaults syntax
#

: ${VISUDO=visudo}

$VISUDO -csf - <<EOF
Defaults		syslog=auth
Defaults>root		!set_logname
Defaults:FULLTIMERS	!lecture
Defaults:millert	!authenticate
Defaults@SERVERS	log_year, logfile=/var/log/sudo.log
Defaults!PAGERS		noexec

Defaults		env_keep -= "HOME"
Defaults		env_keep =  "COLORS DISPLAY HOSTNAME HISTSIZE KDEDIR LS_COLORS"
Defaults		env_keep += "MAIL PS1 PS2 QTDIR LANG LC_ADDRESS LC_CTYPE"

User_Alias		FULLTIMERS = millert, mikef, dowdy

Cmnd_Alias		PAGERS = /usr/bin/more, /usr/bin/pg, /usr/bin/less

Host_Alias		SERVERS = primary, mail, www, ns
EOF

exit 0
