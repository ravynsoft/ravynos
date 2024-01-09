# zls () {
# simple internal ls using the stat module

zmodload -F zsh/stat b:zstat || return 1

emulate -R zsh
setopt localoptions

local f opts='' L=L mod=: dirs list
typeset -A stat

dirs=()
list=()

while getopts ailLFdtuc f
do
    opts=$opts$f
    if [[ $f == '?' ]] then
	echo Usage: $0 [ -ailLFd ] [ filename ... ]
	return 1
    fi
done
shift OPTIND-1

[[ $opts == *L* ]] && L=''
[[ $opts == *F* ]] && mod=T$mod
[[ $opts == *a* ]] && setopt globdots

local time=mtime tmod=m
[[ $opts == *u* ]] && time=atime tmod=a
[[ $opts == *c* ]] && time=ctime tmod=c

if ((! ARGC)) then
    if [[ $opts = *t* ]]; then
        set *(o$tmod)
    else
        set *
    fi
    opts=d$opts
elif [[ $opts = *t* && $ARGC -gt 1 ]]; then
    # another glaringly obvious zsh trick:  reorder the argv list
    # by time, without messing up metacharacters inside
    local n='$1'
    for (( f = 2; f <= $ARGC; f++ )); do
	n="$n|\$$f"
    done
    eval "argv=(($n)(o$tmod))"
fi

for f in $*
do
    zstat -s$L -H stat -F "%b %e %H:%M" - $f || continue
    if [[ $opts != *d* && $stat[mode] == d* ]] then
	dirs=( $dirs $f )
    elif [[ $opts == *l* ]] then
	[[ $opts == *i* ]] && print -n "${(l:7:)stat[inode]} "
	[[ -n $stat[link] ]] && f=( $f '->' $stat[link] ) || f=( $f($mod) )
	print -r -- "$stat[mode] ${(l:3:)stat[nlink]} ${(r:8:)stat[uid]} " \
		    "${(r:8:)stat[gid]} ${(l:8:)stat[size]} $stat[$time] $f"
    else
	f=( $f($mod) )
	list=( "$list[@]" "${${(M)opts:%*i*}:+${(l:7:)stat[inode]} }$f" )
    fi
done
(($#list)) && print -cr -- "$list[@]"
while (($#dirs)) do
    ((ARGC > $#dirs)) && echo
    ((ARGC > 1)) && echo $dirs[1]:
    (cd $dirs[1] && $0 -d$opts)
    shift dirs
done
# }
