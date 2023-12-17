#
# Completion examples
#
#  Chet Ramey <chet.ramey@case.edu>
#
#  Copyright 2002 Chester Ramey
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

#
# This encapsulates the default bash completion code
# call with the word to be completed as $1
#
# Since programmable completion does not use the bash default completions
# or the readline default of filename completion when the compspec does
# not generate any matches, this may be used as a `last resort' in a
# completion function to mimic the default bash completion behavior.
#
_bash_def_completion ()
{
	local h t
	COMPREPLY=()

	# command substitution
	if [[ "$1" == \$\(* ]]; then
		t=${1#??}
		COMPREPLY=( $(compgen -c -P '$(' $t) )
	fi
	# variables with a leading `${'
	if [ ${#COMPREPLY[@]} -eq 0 ] && [[ "$1" == \$\{* ]]; then
		t=${1#??}
		COMPREPLY=( $(compgen -v -P '${' -S '}' $t) )
	fi
	# variables with a leading `$'
	if [ ${#COMPREPLY[@]} -eq 0 ] && [[ "$1" == \$* ]]; then
		t=${1#?}
		COMPREPLY=( $(compgen -v -P '$' $t ) )
	fi
	# username expansion
	if [ ${#COMPREPLY[@]} -eq 0 ] && [[ "$1" == ~* ]] && [[ "$1" != */* ]]; then
		t=${1#?}
		COMPREPLY=( $( compgen -u -P '~' $t ) )
	fi
	# hostname
	if [ ${#COMPREPLY[@]} -eq 0 ] && [[ "$1" == *@* ]]; then
		h=${1%%@*}
		t=${1#*@}
		COMPREPLY=( $( compgen -A hostname -P "${h}@" $t ) )
	fi
	# glob pattern
	if [ ${#COMPREPLY[@]} -eq 0 ]; then
		# sh-style glob pattern
		if [[ $1 == *[*?[]* ]]; then
			COMPREPLY=( $( compgen -G "$1" ) )
		# ksh-style extended glob pattern - must be complete
		elif shopt -q extglob && [[ $1 == *[?*+\!@]\(*\)* ]]; then
			COMPREPLY=( $( compgen -G "$1" ) )
		fi
	fi

	# final default is filename completion
	if [ ${#COMPREPLY[@]} -eq 0 ]; then
		COMPREPLY=( $(compgen -f "$1" ) )
	fi
}

# 
# Return 1 if $1 appears to contain a redirection operator.  Handles backslash
# quoting (barely).
#
_redir_op()
{
	case "$1" in
	*\\'[\<\>]'*)	return 1;;
	*[\<\>]*)	return 0;;
	*)		return 1;;
	esac
}


# _redir_test tests the current word ($1) and the previous word ($2) for
# redirection operators and does filename completion on the current word
# if either one contains a redirection operator
_redir_test()
{
	if _redir_op "$1" ; then
		COMPREPLY=( $( compgen -f "$1" ) )
		return 0
	elif _redir_op "$2" ; then
		COMPREPLY=( $( compgen -f "$1" ) )
		return 0
	fi
	return 1
}

# optional, but without this you can't use extended glob patterns
shopt -s extglob

#
# Easy ones for the shell builtins
#
# nothing for: alias, break, continue, dirs, echo, eval, exit, getopts,
# let, logout, popd, printf, pwd, return, shift, suspend, test, times,
# umask
#

complete -f -- . source
complete -A enabled builtin
complete -d cd

# this isn't exactly right yet -- needs to skip shell functions and
# do $PATH lookup (or do compgen -c and filter out matches that also
# appear in compgen -A function)
complete -c command

# could add -S '=', but that currently screws up because readline appends
# a space unconditionally

complete -v export local readonly
complete -A helptopic help	# currently same as builtins

complete -d pushd

complete -A shopt shopt

complete -c type

complete -a unalias
complete -v unset 

#
# Job control builtins: fg, bg, disown, kill, wait
# kill not done yet
#

complete -A stopped -P '%' bg
complete -j -P '%' fg jobs disown

# this is not quite right at this point

_wait_func ()
{
	local cur
	cur=${COMP_WORDS[COMP_CWORD]}

	case "$cur" in
	%*)	COMPREPLY=( $(compgen -A running -P '%' ${cur#?} ) ) ;;
	[0-9]*)	COMPREPLY=( $(jobs -p | grep ^${cur}) ) ;;
	*)	COMPREPLY=( $(compgen -A running -P '%') $(jobs -p) )
		;;
	esac
}
complete -F _wait_func wait

#
# more complicated things, several as yet unimplemented
#

#complete -F _bind_func bind

_declare_func()
{
	local cur prev nflag opts

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	COMPREPLY=()
	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-a -f -F -i -p -r -t -x)
		return 0;
	fi
	if [[ $cur == '+' ]]; then
		COMPREPLY=(+i +t +x)
		return 0;
	fi
	if [[ $prev == '-p' ]]; then
		COMPREPLY=( $(compgen -v $cur) )
		return 0;
	fi
	return 1
}
complete -F _declare_func declare typeset

_enable_func()
{
	local cur prev nflag opts

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	COMPREPLY=()
	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-a -d -f -n -p -s)
		return 0;
	fi
	if [[ $prev == '-f' ]]; then
		COMPREPLY=( $( compgen -f $cur ) )
		return 0;
	fi
	for opts in "${COMP_WORDS[@]}" ; do
		if [[ $opts == -*n* ]]; then nflag=1; fi
	done

	if [ -z "$nflag" ] ; then
		COMPREPLY=( $( compgen -A enabled $cur ) )
	else
		COMPREPLY=( $( compgen -A disabled $cur ) )
	fi
	return 0;
}
complete -F _enable_func enable

_exec_func()
{
	local cur prev

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-a -c -l)
		return 0;
	fi
	if [[ $prev != -*a* ]]; then
		COMPREPLY=( $( compgen -c $cur ) )
		return 0
	fi
	return 1;
}
complete -F _exec_func exec

_fc_func()
{
	local cur prev

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-e -n -l -r -s)
		return 0;
	fi
	if [[ $prev == -*e ]]; then
		COMPREPLY=( $(compgen -c $cur) )
		return 0
	fi
	return 1
}
complete -F _fc_func fc

_hash_func()
{
	local cur prev

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-p -r -t)
		return 0;
	fi

	if [[ $prev == '-p' ]]; then
		COMPREPLY=( $( compgen -f $cur ) )
		return 0;
	fi
	COMPREPLY=( $( compgen -c $cur ) )
	return 0
}
complete -F _hash_func hash

_history_func()
{
	local cur prev

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	COMPREPLY=()
	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-a -c -d -n -r -w -p -s)
		return 0;
	fi
	if [[ $prev == -[anrw] ]]; then
		COMPREPLY=( $( compgen -f $cur ) )
	fi
	return 0
}
complete -F _history_func history

#complete -F _read_func read

_set_func ()
{
	local cur prev

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	COMPREPLY=()

	_redir_test "$cur" "$prev" && return 0;

	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-a -b -e -f -k -m -n -o -p -t -u -v -x -B -C -H -P --)
		return 0;
	fi
	if [[ $cur == '+' ]]; then
		COMPREPLY=(+a +b +e +f +k +m +n +o +p +t +u +v +x +B +C +H +P)
		return 0;
	fi
	if [[ $prev == [+-]o ]]; then
		COMPREPLY=( $(compgen -A setopt $cur) )
		return 0;
	fi
	return 1;
}
complete -F _set_func set

_trap_func ()
{
	local cur
	cur=${COMP_WORDS[COMP_CWORD]}

	if (( $COMP_CWORD <= 1 )) || [[ $cur == '-' ]]; then
		COMPREPLY=(-l -p)
		return 0;
	fi
	COMPREPLY=( $( compgen -A signal ${cur}) )
	return 0
}
complete -F _trap_func trap

#
# meta-completion (completion for complete/compgen)
#
_complete_meta_func()
{
	local cur prev cmd
	COMPREPLY=()

	cmd=$1

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	_redir_test "$cur" "$prev" && return 0;

	if (( $COMP_CWORD <= 1 )) || [[ "$cur" == '-' ]]; then
		case "$cmd" in
		complete) COMPREPLY=(-a -b -c -d -e -f -j -k -s -v -u -r -p -A -G -W -P -S -X -F -C);;
		compgen)  COMPREPLY=(-a -b -c -d -e -f -j -k -s -v -u -A -G -W -P -S -X -F -C);;
		esac
		return 0
	fi

	if [[ $prev == -A ]]; then
		COMPREPLY=(alias arrayvar binding builtin command directory \
disabled enabled export file 'function' helptopic hostname job keyword \
running service setopt shopt signal stopped variable)
		return 0
	elif [[ $prev == -F ]]; then
		COMPREPLY=( $( compgen -A function $cur ) )
	elif [[ $prev == -C ]]; then
		COMPREPLY=( $( compgen -c $cur ) )
	else
		COMPREPLY=( $( compgen -c $cur ) )
	fi
	return 0
}
complete -F _complete_meta_func complete compgen

#
# some completions for shell reserved words
#
#complete -c -k time do if then else elif '{'

#
# external commands
#

complete -e printenv

complete -c nohup exec nice eval trace truss strace sotruss gdb

_make_targets ()
{
	local mdef makef gcmd cur prev i

	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	# if prev argument is -f, return possible filename completions.
	# we could be a little smarter here and return matches against
	# `makefile Makefile *.mk', whatever exists
	case "$prev" in
	-*f)	COMPREPLY=( $(compgen -f $cur ) ); return 0;;
	esac

	# if we want an option, return the possible posix options
	case "$cur" in
	-)	COMPREPLY=(-e -f -i -k -n -p -q -r -S -s -t); return 0;;
	esac

	# make reads `makefile' before `Makefile'
	# GNU make reads `GNUmakefile' before all other makefiles, but we
	# check that we're completing `gmake' before checking for it
	if [ -f GNUmakefile ] && [ ${COMP_WORDS[0]} == gmake ]; then
		mdef=GNUmakefile
	elif [ -f makefile ]; then
		mdef=makefile
	elif [ -f Makefile ]; then
		mdef=Makefile
	else
		mdef=*.mk		# local convention
	fi

	# before we scan for targets, see if a makefile name was specified
	# with -f
	for (( i=0; i < ${#COMP_WORDS[@]}; i++ )); do
		if [[ ${COMP_WORDS[i]} == -*f ]]; then
			eval makef=${COMP_WORDS[i+1]}	# eval for tilde expansion
			break
		fi
	done

	[ -z "$makef" ] && makef=$mdef

	# if we have a partial word to complete, restrict completions to
	# matches of that word
	if [ -n "$2" ]; then gcmd='grep "^$2"' ; else gcmd=cat ; fi

	# if we don't want to use *.mk, we can take out the cat and use
	# test -f $makef and input redirection	
	COMPREPLY=( $(cat $makef 2>/dev/null | awk 'BEGIN {FS=":"} /^[^.# 	][^=]*:/ {print $1}' | tr -s ' ' '\012' | sort -u | eval $gcmd ) )
}
complete -F _make_targets -X '+($*|*.[cho])' make gmake pmake

_umount_func ()
{
	COMPREPLY=( $(mount | awk '{print $1}') )
}
complete -F _umount_func umount

_configure_func ()
{
	case "$2" in
	-*)	;;
	*)	return ;;
	esac

	case "$1" in
	\~*)	eval cmd=$1 ;;
	*)	cmd="$1" ;;
	esac

	COMPREPLY=( $("$cmd" --help | awk '{if ($1 ~ /--.*/) print $1}' | grep ^"$2" | sort -u) )
}
complete -F _configure_func configure

complete -W '"${GROUPS[@]}"' newgrp

complete -f chown ln more cat
complete -d mkdir rmdir
complete -f strip

complete -f -X '*.gz' gzip
complete -f -X '*.bz2' bzip2
complete -f -X '*.Z' compress
complete -f -X '!*.+(gz|tgz|Gz)' gunzip gzcat zcat zmore
complete -f -X '!*.Z' uncompress zmore zcat
complete -f -X '!*.bz2' bunzip2 bzcat
complete -f -X '!*.zip' unzip
complete -f -X '!*.+(gif|jpg|jpeg|GIF|JPG|JPEG|bmp)' xv

complete -f -X '!*.pl' perl perl5

complete -A hostname rsh telnet rlogin ftp ping xping host traceroute nslookup
complete -A hostname rxterm rxterm3 rxvt2

complete -u su
complete -g newgrp groupdel groupmod

complete -f -X '!*.+(ps|PS)' gs gv ghostview psselect pswrap
complete -f -X '!*.+(dvi|DVI)' dvips xdvi dviselect dvitype catdvi
complete -f -X '!*.+(pdf|PDF)' acroread4
complete -f -X '!*.texi*' makeinfo texi2dvi texi2html
complete -f -X '!*.+(tex|TEX)' tex latex slitex

complete -f -X '!*.+(mp3|MP3)' mpg123
complete -f -X '!*.+(htm|html)' links w3m lynx

#
# other possibilities, left as exercises
#
#complete -F _find_func find
#complete -F _man_func man
#complete -F _stty_func stty
