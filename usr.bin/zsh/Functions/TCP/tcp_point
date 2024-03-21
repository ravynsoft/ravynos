emulate -L zsh
setopt extendedglob cbases


if [[ $# -ne 1 ]]; then
    print "Usage: $0 port
Listen on the given port; send anything that arrives to standard output." >&2
    return 1
fi

local REPLY lfd afd
if ! ztcp -l $1; then
    print "Failed to listen on port $1" >&2
    return 1
fi

lfd=$REPLY

if ! ztcp -a $lfd; then
    print "Failed to accept on fd $lfd" >&2
    ztcp -c $lfd
fi

afd=$REPLY

cat <&$afd

ztcp -c $lfd
ztcp -c $afd
