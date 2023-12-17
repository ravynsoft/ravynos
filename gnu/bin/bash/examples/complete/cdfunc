#  cdfunc - example completion function for cd
#
#  based on the cd completion function from the bash_completion package
#
#  Chet Ramey <chet.ramey@case.edu>
#
#  Copyright 2011 Chester Ramey
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

_comp_cd()
{
	local IFS=$' \t\n'	# normalize IFS
	local cur _skipdot _cdpath
	local i j k

	# Tilde expansion, with side effect of expanding tilde to full pathname
	case "$2" in
	\~*)	eval cur="$2" ;;
	*)	cur=$2 ;;
	esac

	# no cdpath or absolute pathname -- straight directory completion
	if [[ -z "${CDPATH:-}" ]] || [[ "$cur" == @(./*|../*|/*) ]]; then
		# compgen prints paths one per line; could also use while loop
		IFS=$'\n'
		COMPREPLY=( $(compgen -d -- "$cur") )
		IFS=$' \t\n'
	# CDPATH+directories in the current directory if not in CDPATH
	else
		IFS=$'\n'
		_skipdot=false
		# preprocess CDPATH to convert null directory names to .
		_cdpath=${CDPATH/#:/.:}
		_cdpath=${_cdpath//::/:.:}
		_cdpath=${_cdpath/%:/:.}
		for i in ${_cdpath//:/$'\n'}; do
			if [[ $i -ef . ]]; then _skipdot=true; fi
			k="${#COMPREPLY[@]}"
			for j in $( compgen -d -- "$i/$cur" ); do
				COMPREPLY[k++]=${j#$i/}		# cut off directory
			done
		done
		$_skipdot || COMPREPLY+=( $(compgen -d -- "$cur") )
		IFS=$' \t\n'
	fi

	# variable names if appropriate shell option set and no completions
	if shopt -q cdable_vars && [[ ${#COMPREPLY[@]} -eq 0 ]]; then
		COMPREPLY=( $(compgen -v -- "$cur") )
	fi

	# append slash to passed directory name that is the only completion.
	# readline will not do this if we complete from CDPATH
	if [[ ${#COMPREPLY[@]} -eq 1 ]]; then
		i=${COMPREPLY[0]}	# shorthand
		if [[ "$cur" == "$i" ]] && [[ "$i" != "*/" ]]; then
			COMPREPLY[0]+=/
		fi
	fi
	return 0
}

complete -o filenames -o nospace -o bashdefault -F _comp_cd cd
