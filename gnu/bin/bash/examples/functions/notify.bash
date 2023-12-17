#
#  Chet Ramey <chet.ramey@case.edu>
#
#  Copyright 1992 Chester Ramey
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2, or (at your option)
#   any later version.
#
#   TThis program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software Foundation,
#   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

trap _notify CHLD
NOTIFY_ALL=false
unset NOTIFY_LIST
unalias false

false()
{
	return 1
}

_notify ()
{
	local i j
	local newlist=

	if $NOTIFY_ALL
	then
		return		# let bash take care of this itself
	elif [ -z "$NOTIFY_LIST" ]; then
		return
	else
		set -- $NOTIFY_LIST
		for i in "$@"
		do
			j=$(jobs -n %$i)
			if [ -n "$j" ]; then
				echo "$j"
				jobs -n %$i >/dev/null
			else
				newlist="newlist $i"
			fi
		done
		NOTIFY_LIST="$newlist"
	fi
}

notify ()
{
	local i j

	if [ $# -eq 0 ]; then
		NOTIFY_ALL=:
		set -b
		return
	else
		for i in "$@"
		do
			# turn a valid job spec into a job number
			j=$(jobs $i)
			case "$j" in
			[*)	j=${j%%]*}
				j=${j#[}
				NOTIFY_LIST="$NOTIFY_LIST $j"
				;;
			esac
		done
	fi
}
