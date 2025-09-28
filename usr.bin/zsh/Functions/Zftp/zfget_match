# function zfget_match {

emulate -L zsh
zmodload -F zsh/files b:zf_ln || return 1

# the zfcd hack:  this may not be necessary here
if [[ $1 == $HOME || $1 == $HOME/* ]]; then
  1="~${1#$HOME}"
fi

if [[ $ZFTP_SYSTEM == UNIX* && $1 == */* ]]; then
  setopt localoptions clobber extendedglob
  local tmpf=${TMPPREFIX}zfgm$$
  zf_ln -fn =(<<<'') $tmpf || return 1

  if [[ -n $WIDGET ]]; then
    local dir=${1%/*}
    [[ $dir = */ ]] || dir="$dir/"
    zftp ls -LF $dir >$tmpf
    local reply1 reply2

    # dirs in reply1, files in reply2
    reply1=(${${(M)${${(f)"$(<$tmpf)"}##$dir}:#*/}%/})
    reply2=(${${${${(f)"$(<$tmpf)"}##$dir}%\*}:#*/})

    # try dir if ls -F doesn't work
    if ! (($#reply1)); then
      zftp dir $dir >$tmpf
      reply1=(${(M)${(f)"$(<$tmpf)"}:#d([^[:space:]]##[[:space:]]##)(#c8)?##})
      reply1=(${reply1/(#b)d([^[:space:]]##[[:space:]]##)(#c8)([^\/]##)\/#/$match[2]})

      reply2=(${${(f)"$(<$tmpf)"}:#d([^[:space:]]##[[:space:]]##)(#c8)?##})
      reply2=(${reply2/(#b)([^[:space:]]##[[:space:]]##)(#c8)(?##)/$match[2]})
    fi
    _wanted directories expl 'remote directory' compadd -S/ -q -P $dir - $reply1
    _wanted files expl 'remote file' compadd -P $dir - $reply2
  else
    # On the first argument to ls, we usually get away with a glob.
    zftp ls "$1*$2" >$tmpf
    reply=($(<$tmpf))
  fi
else
  local fcache_name
  zffcache
  if [[ -n $WIDGET ]]; then
    _wanted files expl 'remote file' compadd -F fignore - ${(P)fcache_name}
  else
    reply=(${(P)fcache_name});
  fi
fi
# }
