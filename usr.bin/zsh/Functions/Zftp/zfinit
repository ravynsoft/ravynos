emulate -L zsh

if [[ $1 != -n ]]; then
   zmodload -i zsh/net/tcp || return 1
   zmodload -ia zsh/zftp zftp || return 1
fi

if zmodload -i zsh/zutil; then
  local arr
  # Set defaults for styles if none set.
  zstyle -g arr ':zftp:*' progress || zstyle ':zftp:*' progress bar
  zstyle -g arr ':zftp:*' update   || zstyle ':zftp:*' update   1
  zstyle -g arr ':zftp:*' titlebar || zstyle ':zftp:*' titlebar true
  if functions chpwd >&/dev/null && ! zstyle -g arr ':zftp:*' chpwd; then
    zstyle ':zftp:*' chpwd true
  fi

  typeset -gA zfconfig
  zfconfig=(lastsession default)
fi

alias zfcd='noglob zfcd'
alias zfget='noglob zfget'
alias zfls='noglob zfls'
alias zfdir='noglob zfdir'
alias zfuget='noglob zfuget'

autoload -Uz zfanon zfautocheck zfcd zfcd_match zfcget zfclose zfcput
autoload -Uz zfdir zffcache zfgcp zfget zfget_match zfgoto zfhere zfinit zfls
autoload -Uz zfmark zfopen zfparams zfpcp zfput zfrglob zfrtime zfsession
autoload -Uz zfstat zftp_chpwd zftp_progress zftransfer zftype zfuget zfuput

#
# zftp completions: only use these if new-style completion is not
# active.
#
if [[ ${#_patcomps} -eq 0 || -z ${_patcomps[(r)_zf*]} ]] &&
  (compctl >/dev/null 2>&1); then
  # only way of getting that noglob out of the way: this is unnecessary with
  # widget-based completion
  setopt completealiases

  compctl -f -x 'p[1]' \
    -k '(open params user login type ascii binary mode put putat
    get getat append appendat ls dir local remote mkdir rmdir delete
    close quit)'  - \
    'w[1,cd][1,ls][1,dir][1,rmdir]' -K zfcd_match -S/ -q - \
    'W[1,get*]' -K zfget_match - 'w[1,delete][1,remote]' -K zfget_match - \
    'w[1,open][1,params]' -k hosts - \
    'w[1,session]' -s '${$(zftp session):#$ZFTP_SESSION}' -- zftp
  compctl -K zfcd_match -S/ -q zfcd zfdir zfls
  compctl -K zfget_match zfget zfgcp zfuget zfcget
  compctl -k hosts zfanon zfopen zfparams
  compctl -s \
    '$(awk '\''{print $1}'\'' ${ZFTP_BMFILE:-${ZDOTDIR:-$HOME}/.zfbkmarks})' \
    -x 'W[1,-*n*]' \
    -s '$(awk -F, '\''NR > 2 { print $1 }'\'' ~/.ncftp/bookmarks)' -- \
    zfgoto zfmark
  compctl -s '${$(zftp session):#$ZFTP_SESSION}' zfsession
  # in _zftp for new completion, but hard to inline into a compctl
  zftransfer_match() {
    local sess=${1%%:*} oldsess=$ZFTP_SESSION
    [[ -n $sess ]] && zftp session $sess
    zfget_match ${1#*:} $2
    [[ -n $sess && -n $oldsess ]] && zftp session $oldsess
    reply=(${sess}:${^reply})
  }
  compctl -s '$(zftp session)' -S : -x 'C[0,*:*]' \
    -K zftransfer_match -- zftransfer
fi

return 0
