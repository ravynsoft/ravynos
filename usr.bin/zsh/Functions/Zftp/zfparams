# function zfparams {

emulate -L zsh
[[ $curcontext = :zf* ]] || local curcontext=:zfparams

if [[ $# -eq 1 && $1 = - ]]; then
  # Delete existing parameter set.
  local sess=$ZFTP_SESSION key
  key=${zfconfig[fcache_$sess]}
  [[ -n $key ]] && unset $key
  for key in fcache lastloc lastdir curdir otherdir otherargs lastuser; do
    unset "zfconfig[${key}_${sess}]"
  done
elif (( $# > 0 )); then
  # Set to prompt for any user or password if not given.
  # Don't worry about accounts here.
  (( $# < 2 )) && 2='?'
  if (( $# < 3 )); then
    if [[ $2 = '?'* ]]; then
      3="?Password on ${1}: "
    else
      3="?Password for ${2##\\?} on ${1}: "
    fi
  fi
fi
zftp params $*
# }
