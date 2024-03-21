# Print the a relative path from the second directory to the first,
# defaulting the second directory to $PWD if none is specified.
emulate -L zsh || return 1

[[ $1 != /* ]] && print $1 && return
[[ -f $1 ]] && 3=$1:t 1=$1:h
[[ -d $1 && -d ${2:=$PWD} ]] || return 1
[[ $1 -ef $2 ]] && print ${3:-.} && return

# The simplest way to eliminate symlinks and ./ and ../ in the paths:
1=$(cd $1; pwd -r)
2=$(cd $2; pwd -r)

local -a cur abs
cur=(${(s:/:)2})	# Split 'current' directory into cur
abs=(${(s:/:)1} $3)	# Split target directory into abs

# Compute the length of the common prefix, or discover a subdirectory:
integer i=1
while [[ i -le $#abs && $abs[i] == $cur[i] ]]
do
    ((++i > $#cur)) && print ${(j:/:)abs[i,-1]} && return
done

2=${(j:/:)cur[i,-1]/*/..}	# Up to the common prefix directory and
1=${(j:/:)abs[i,-1]}		# down to the target directory or file

print $2${1:+/$1}
