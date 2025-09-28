# Listen on the given port and for every connection, start a new 
# command (defaults to $SHELL) in the background on the accepted fd.
# WARNING: this can leave your host open to the universe.  For use
# in a restricted fashion on a secure network.
#
# Remote logins are much more efficient...

local TCP_LISTEN_FD
trap '[[ -n $TCP_LISTEN_FD ]] && ztcp -c $TCP_LISTEN_FD; return 1' \
    HUP INT TERM EXIT PIPE

if [[ $1 != <-> ]]; then
    print "Usage: $0 port [cmd args... ]" >&2
    return 1
fi

integer port=$1
shift
ztcp -l $port || return 1
TCP_LISTEN_FD=$REPLY

(( $# )) || set -- ${SHELL:-zsh}
local cmd=$1
shift

while ztcp -a $TCP_LISTEN_FD; do
    # hack to expand aliases without screwing up arguments
    eval $cmd '$*  <&$REPLY >&$REPLY 2>&$REPLY &'
    # Close the session fd; we don't need it here any more.
    ztcp -c $REPLY
done
