#!/usr/local/bin/zsh
#
# NAME:
#	reporter
#
# SYNOPSIS:
#	reporter [all | aliases | bindings | completion | functions |
#			limits | options | variables | zstyles]
#
# DESCRIPTION:
#	"reporter" prints your current environment variables, shell
#	variables, limits, completion settings, and option settings to
#	stdout in the form of a script.
#
#	If you run into a zsh bug, someone can source the output script to
#	recreate most of the environment under which you were working.
#
#	IMPORTANT:	"source" this script, don't try to run it directly.
#			Otherwise it won't report the settings for your
#			current shell session.
#
# OPTIONS:
#	All command-line options can be abbreviated.
#
#	"aliases"	prints only aliases.
#	"bindings"	prints only "bindkey" commands.
#	"completion"	prints only "compctl" commands.
#	"functions"	prints "autoload" commands or actual functions.
#	"limits"	prints "limit" commands for things like cputime, etc.
#	"modules"	prints "zmodload" commands.
#	"options"	prints "setopt" commands.
#	"variables"	prints both shell and environment variables.
#	"zstyles"	prints "zstyle" commands
#
#	"all"		tries to find every useful setting under your shell.
#			This is the default, and it's the same as typing all
#			of the above options on the command line.
#
# CAVEATS:
#	Assumes that you have the following programs in your search path:
#		awk, cut, echo, grep, sed, sort
#	Assumes that your C preprocessor lives in /lib/cpp or /usr/ccs/lib/cpp.
#	Uses (and unsets) variables beginning with "reporter_".
#	Won't work for versions of zsh that are older than 3.1.3 or so.
#
# RESTRICTIONS:
#	DON'T:	pretend you wrote it, sell it, or blame me if it breaks.
#	DO:	as ye will an' ye harm none.
#					--Wiccan saying, I think
#
# BUGS:
#	I'm sure there are more than a few.  To be safe, run "zsh -f" before
#	sourcing the output from this script.  If you have "screen", you may
#	want to use that, too; I hammered my terminal settings beyond repair
#	when using an early version, and "screen" saved me from having to
#	login on another terminal.
#
# HISTORY:
#	The name was ripped off from the Emacs "reporter.el" function.
#	The idea came from a mail message to the ZSH mailing list:
#
# Begin Configuration Section
#

reporter_OSVersion="`uname -s`_`uname -r`"

#
# Solaris 2.x
#
case ${reporter_OSVersion} in
	SunOS_5.*)
		CPP=${CPP:-/usr/ccs/lib/cpp}
		AWK=${AWK:-nawk}	# GNU AWK doesn't come standard :-(
		;;
esac

#
# Default Values 
#

CPP=${CPP:-/lib/cpp}
AWK=${AWK:-awk}

#
# End Configuration Section
#

reporter_do_all=yes

for reporter_each
do
	case "$reporter_each"
	in
		ali*)	reporter_do_aliases=yes; reporter_do_all=no ;;
		b*)	reporter_do_bindings=yes; reporter_do_all=no ;;
		c*)	reporter_do_compctl=yes; reporter_do_all=no ;;
		f*)	reporter_do_fun=yes; reporter_do_all=no ;;
		l*)	reporter_do_lim=yes; reporter_do_all=no ;;
		m*)	reporter_do_mod=yes; reporter_do_all=no ;;
		o*)	reporter_do_setopt=yes; reporter_do_all=no ;;
		v*)	reporter_do_vars=yes; reporter_do_all=no ;;
		zs*|s*)	reporter_do_zstyle=yes; reporter_do_all=no ;;
		*)	;;
	esac
done

#
#	The "cshjunkiequotes" option can break some of the commands
#	used in the remainder of this script, so we check for that first
#	and disable it.  Similarly "shwordsplit" and "kshoptionprint".
#	We'll re-enable them later.
#

reporter_junkiequotes="no"
reporter_shwordsplit="no"
reporter_kshoptprint="no"
reporter_nounset="no"

if [[ -o cshjunkiequotes ]]
then
	reporter_junkiequotes="yes"
	unsetopt cshjunkiequotes
fi
if [[ -o shwordsplit ]]
then
	reporter_shwordsplit="yes"
	unsetopt shwordsplit
fi
if [[ -o kshoptionprint ]]
then
	reporter_kshoptprint="yes"
	unsetopt kshoptionprint
fi
if [[ -o nounset ]]
then
	reporter_nounset="yes"
	unsetopt nounset
fi

#
#	UNAME
#
#	This shows your system name.  It's extremely system-dependent, so
#	we need a way to find out what system you're on.  The easiest
#	way to do this is by using "uname", but not everyone has that,
#	so first we go through the search path.
#
#	If we don't find it, then the only thing I can think of is to
#	check what's defined in your C compiler, and code in some exceptions
#	for the location of "uname" or an equivalent.  For example, Pyramid
#	has "uname" only in the ATT universe.  This code assumes that
#	the "-a" switch is valid for "uname".
#
#	This section of code sees what is defined by "cpp".  It was
#	originally written by brandy@tramp.Colorado.EDU (Carl Brandauer).
#	Additional error checking and sed hacking added by Ken Phelps.
#

reporter_cppdef=(`strings -3 ${CPP} |
	sed -n '
	/^[a-zA-Z_][a-zA-Z0-9_]*$/{
	s/.*/#ifdef &/p
	s/.* \(.*\)/"\1";/p
	s/.*/#endif/p
	}
	' | ${CPP} |sed '
	/^[	 ]*$/d
	/^#/d
	s/.*"\(.*\)".*/\1/'`)

reporter_uname=""

for reporter_each in `echo $PATH | sed -e 's/:/ /g'`
do
	if [[ -x $reporter_each/uname ]]
	then
		reporter_uname="$reporter_each/uname"
		break
	fi
done

case "$reporter_uname"
in
	"")	reporter_uname="echo not found on this system" ;;
	*)	;;
esac

for reporter_each in $reporter_cppdef
do
	case "$reporter_each"
	in
		pyr)	reporter_uname="/bin/att uname" ;;
		*)	;;
	esac
done

echo '# START zsh saveset'
echo '# uname: ' `eval $reporter_uname -a`
echo

unset reporter_cppdef
unset reporter_uname
unset reporter_each

#
#	ALIASES
#
#	Use "alias -L" to get a listing of the aliases in the form we want.
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_aliases" = "yes" ]]
then
	echo '# Aliases.'
	echo

	alias -L
fi

#
#	KEY BINDINGS
#
#	The -L option does most of the work.  The subshell is used to
#	avoid modifying things that will be recorded later.
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_bindings" = "yes" ]]
then
	echo
	echo "# Key bindings."
	echo
	bindkey -lL | grep -v ' \.safe$'
	(
		alias bindkey=bindkey
		bindkey () {
			[[ "$1" == "-N" ]] || return
			[[ "$2" == "--" ]] && shift
			[[ "$2" == ".safe" ]] && return
			echo
			builtin bindkey -L -M -- "$2"
		}
		eval "`builtin bindkey -lL`"
	)
fi

#
#	COMPLETION COMMANDS
#	Warning:  this won't work for zsh-2.5.03.
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_compctl" = "yes" ]]
then
	echo
	echo "# Completions."
	echo

	compctl -L
fi

#
#	FUNCTIONS
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_fun" = "yes" ]]
then
	echo 
	echo "# Undefined functions."
	echo

	autoload + | ${AWK} '{print "autoload " $1}'

	echo 
	echo "# Defined functions."
	echo

	(
		unfunction `autoload +` 2>/dev/null
		functions
	)
fi

#
#	LIMITS
#
#	"cputime" has to be handled specially, because you can specify
#	the time as just hours, or "minutes:seconds".
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_lim" = "yes" ]]
then
	echo
	echo '# Limits.'
	echo

	(
		set X `limit | grep "cputime" | grep -v "unlimited" |
			sed -e 's/:/ /g'`

		if test "$#" -gt 1
		then
			hr=$3
			min=$4
			sec=$5

			if test "$hr" -gt 0
			then
				echo "limit cputime ${hr}h"
			else
				echo "limit cputime $min:$sec"
			fi
		fi
	)

	limit | grep -v "cputime" | grep -v "unlimited" |
		sed -e 's/Mb/m/' -e 's/Kb/k/' |
		${AWK} 'NF > 1 {print "limit " $0}'
fi

#
#	MODULE LOADING COMMANDS
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_mod" = "yes" ]]
then
	echo
	if ( zmodload ) >& /dev/null; then
		echo "# Modules."
		echo
		zmodload -d -L
		echo
		zmodload -ab -L
		echo
		zmodload -ac -L
		echo
		zmodload -ap -L
		echo
		zmodload -L
	else
		echo "# Modules: zmodload not available."
	fi
fi

#
#	NON-ARRAY VARIABLES
#
#	We run this in a subshell to preserve the parameter module state
#	in the current shell.  Also, reset the prompt to show you're now
#	in a test shell.
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_vars" = "yes" ]]
then
	echo
	echo "# Non-array variables."
	echo

	(
		zmodload -u `zmodload | grep parameter` 2>/dev/null

		echo "ARGC=0"
		eval `typeset + |
			grep -v 'array ' |
			grep -v 'association ' |
			grep -v 'undefined ' |
			grep -v ' ARGC$' |
			grep -v '^reporter_' |
			grep -wv '[!#$*0?@_-]$' |
			sed -e 's/.* \(.*\)/print -r -- \1=${(qq)\1};/' \
			    -e 's/^\([^ ]*\)$/print -r -- \1=${(qq)\1};/'`
		echo "prompt='test%'"
	)

#
#	ARRAY VARIABLES
#
#	Run this in a subshell to preserve the parameter module state in
#	the current shell.
#

	echo
	echo "# Array variables."
	echo

	(
		zmodload -u `zmodload | grep parameter` 2>/dev/null

		echo "argv=()"
		eval `{ typeset + | grep 'array ' ;
				typeset + | grep 'association ' } |
			grep -v 'undefined ' |
			grep -v ' argv$' |
			grep -v ' reporter_' |
			grep -v ' [!#$*0?@_-]$' |
			sed 's/.* \(.*\)/print -r -- \1=\\\(${(qq)\1}\\\);/'`
	)

#
#	EXPORTED VARIABLES
#
#	Run this in a subshell to preserve the parameter module state in
#	the current shell.
#

	echo
	echo "# Exported variables."
	echo

	(
		zmodload -u `zmodload | grep parameter` 2>/dev/null

		export | grep -v "^'*"'[!#$*0?@_-]'"'*=" |
			${AWK} -F'=' '{print "export " $1}'
	)
fi

#
#	SETOPT
#
#	We exclude interactive because "setopt interactive" has no effect.
#	A few special options are dealt with separately; see the comments
#	near the start of the script.
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_setopt" = "yes" ]]
then
	echo
	echo '# Setopt.'
	echo

	(
		setopt | grep -v 'interactive' | ${AWK} '{print "setopt " $0}'

		case "$reporter_junkiequotes"
		in
			yes)	echo "setopt cshjunkiequotes" ;;
			*)	;;
		esac
		case "$reporter_shwordsplit"
		in
			yes)	echo "setopt shwordsplit" ;;
			*)	;;
		esac
		case "$reporter_kshoptprint"
		in
			yes)	echo "setopt kshoptionprint" ;;
			*)	;;
		esac
		case "$reporter_nounset"
		in
			yes)	echo "setopt nounset" ;;
			*)	;;
		esac
	) | sort
fi

#
#	STYLES
#

if [[ "$reporter_do_all" = "yes" || "$reporter_do_zstyle" = "yes" ]]
then
	echo
	echo '# Styles.'
	echo

	zstyle -L
fi

echo
echo '# END zsh saveset'

#
#	Don't put an exit here, or you'll get a nasty surprise when you
#	source this thing.  Get rid of variables created when processing
#	command line.
#

unset reporter_do_all
unset reporter_do_aliases
unset reporter_do_bindings
unset reporter_do_compctl
unset reporter_do_fun
unset reporter_do_lim
unset reporter_do_setopt
unset reporter_do_vars

#
#	Turn various options back on if necessary, in case run via ".".
#

case "$reporter_junkiequotes"
in
	yes)	setopt cshjunkiequotes ;;
	*)	;;
esac
case "$reporter_shwordsplit"
in
	yes)	setopt shwordsplit ;;
	*)	;;
esac
case "$reporter_kshoptprint"
in
	yes)	setopt kshoptionprint ;;
	*)	;;
esac
case "$reporter_nounset"
in
	yes)	setopt nounset ;;
	*)	;;
esac

unset reporter_junkiequotes
unset reporter_shwordsplit
unset reporter_kshoptprint
unset reporter_nounset

unset reporter_OSVersion
