emulate -L zsh
setopt extendedglob

local REPLY tfd

if [[ $# -ne 2 ]]; then
    print "Usage: $0 host port
Connect to the given host and port; send standard input." >&2
    return 1
fi

if ! ztcp $1 $2; then
    print "Failed to open connection to host $1 port $2" >&2
    return 1
fi

tfd=$REPLY

cat >&$tfd

ztcp -c $tfd
