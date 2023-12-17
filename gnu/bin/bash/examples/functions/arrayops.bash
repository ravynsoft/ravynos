# arrayops.bash --- hide some of the nasty syntax for manipulating bash arrays
# Author: Noah Friedman <friedman@splode.com>
# Created: 2016-07-08
# Public domain

# $Id: arrayops.bash,v 1.3 2016/07/28 15:38:55 friedman Exp $

# Commentary:

# These functions try to tame the syntactic nightmare that is bash array
# syntax, which makes perl's almost look reasonable.
#
# For example the apush function below lets you write:
#
#	apush arrayvar newval
#
# instead of
#
#	${arrayvar[${#arrayvar[@]}]}=newval
#
# Because seriously, you've got to be kidding me.

# These functions avoid the use of local variables as much as possible
# (especially wherever modification occurs) because those variable names
# might shadow the array name passed in.  Dynamic scope!

# Code:

#:docstring apush:
# Usage: apush arrayname val1 {val2 {...}}
#
# Appends VAL1 and any remaining arguments to the end of the array
# ARRAYNAME as new elements.
#:end docstring:
apush()
{
    eval "$1=(\"\${$1[@]}\" \"\${@:2}\")"
}

#:docstring apop:
# Usage: apop arrayname {n}
#
# Removes the last element from ARRAYNAME.
# Optional argument N means remove the last N elements.
#:end docstring:
apop()
{
    eval "$1=(\"\${$1[@]:0:\${#$1[@]}-${2-1}}\")"
}

#:docstring aunshift:
# Usage: aunshift arrayname val1 {val2 {...}}
#
# Prepends VAL1 and any remaining arguments to the beginning of the array
# ARRAYNAME as new elements.  The new elements will appear in the same order
# as given to this function, rather than inserting them one at a time.
#
# For example:
#
#	foo=(a b c)
#	aunshift foo 1 2 3
#       => foo is now (1 2 3 a b c)
# but
#
#	foo=(a b c)
#	aunshift foo 1
#       aunshift foo 2
#       aunshift foo 3
#       => foo is now (3 2 1 a b c)
#
#:end docstring:
aunshift()
{
    eval "$1=(\"\${@:2}\" \"\${$1[@]}\")"
}

#:docstring ashift:
# Usage: ashift arrayname {n}
#
# Removes the first element from ARRAYNAME.
# Optional argument N means remove the first N elements.
#:end docstring:
ashift()
{
    eval "$1=(\"\${$1[@]: -\${#$1[@]}+${2-1}}\")"
}

#:docstring aset:
# Usage: aset arrayname idx newval
#
# Assigns ARRAYNAME[IDX]=NEWVAL
#:end docstring:
aset()
{
    eval "$1[\$2]=${@:3}"
}

#:docstring aref:
# Usage: aref arrayname idx {idx2 {...}}
#
# Echoes the value of ARRAYNAME at index IDX to stdout.
# If more than one IDX is specified, each one is echoed.
#
# Unfortunately bash functions cannot return arbitrary values in the usual way.
#:end docstring:
aref()
{
    eval local "v=(\"\${$1[@]}\")"
    local x
    for x in ${@:2} ; do echo "${v[$x]}"; done
}

#:docstring aref:
# Usage: alen arrayname
#
# Echoes the length of the number of elements in ARRAYNAME.
#
# It also returns number as a numeric value, but return values are limited
# by a maximum of 255 so don't rely on this unless you know your arrays are
# relatively small.
#:end docstring:
alen()
{
    eval echo   "\${#$1[@]}"
    eval return "\${#$1[@]}"
}

#:docstring anreverse:
# Usage: anreverse arrayname
#
# Reverse the order of the elements in ARRAYNAME.
# The array variable is altered by this operation.
#:end docstring:
anreverse()
{
    eval set $1 "\"\${$1[@]}\""
    eval unset $1
    while [ $# -gt 1 ]; do
        eval "$1=(\"$2\" \"\${$1[@]}\")"
        set $1 "${@:3}"
    done
}

#provide arrayops

# arrayops.bash ends here
