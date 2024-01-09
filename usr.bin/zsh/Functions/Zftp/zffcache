# Generate an array name for storing the cache for the current session,
# storing it in fcache_name, then generate the cache for the current
# directory, or with argument -d clear the cache.

[[ $1 = -d ]] && local fcache_name

fcache_name=$zfconfig[fcache_$ZFTP_SESSION]
if [[ -z $fcache_name ]]; then
  local vals
  vals=(${(v)zfconfig[(I)fcache_*]##zftp_fcache_})
  integer i
  while [[ -n ${vals[(r)zftp_fcache_$i]} ]]; do
    (( i++ ))
  done
  fcache_name=zftp_fcache_$i
  zfconfig[fcache_$ZFTP_SESSION]=$fcache_name
fi

if [[ $1 = -d ]]; then
  unset $fcache_name
elif (( ${(P)#fcache_name} == 0 )); then
  eval "$fcache_name=(\${(f)\"\$(zftp ls)\"})"
fi
