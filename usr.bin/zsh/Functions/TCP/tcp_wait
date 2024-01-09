# Wait for given number of seconds, reading any data from
# all TCP connections while doing so.

if [[ ${(t)SECONDS} != float* ]]; then
    # If called from tcp_expect, don't override
    typeset -F TCP_SECONDS_START=$SECONDS
    # Get extra accuracy by making SECONDS floating point locally
    typeset -F SECONDS
fi

typeset to end

(( to = $1, end = SECONDS + to ))
while (( SECONDS < end )); do
  tcp_read -a -T $to
  (( to = end - SECONDS ))
done
return
