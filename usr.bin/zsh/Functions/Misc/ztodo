# vim: set ft=zsh et sw=2 sts=2:

emulate -L zsh
setopt no_sh_word_split null_glob no_ksh_arrays
typeset -gHA __ztodolist
typeset -gH __ztodolastwrite
local cachefile short_format list_format
local tmp needupdate=0
local -a todos

zstyle -s ':ztodo:*' cache-file cachefile ||
  cachefile="~/.ztodolist"
zstyle -s ':ztodo:*' short-format short_format ||
  short_format="You have %n thing%1(n..s) to do here."
zstyle -s ':ztodo:*' list-format list_format ||
  list_format="%-2n: %e"

tmp=(${~tmp::=$cachefile(ms-$(( ${(%)tmp::="%D{%s}"} - ${__ztodolastwrite:-0} )))})
(( $#tmp )) &&
  . $~cachefile

todos=( ${(ps:\0:)__ztodolist[$PWD]} )

if (( $# )); then
  case "$1" in
    (add)
      shift
      todos=( $todos "$*" )
      needupdate=1
      ;;
    (del)
      shift
      todos[$1]=()
      needupdate=1
      ;;
    (clear)
      shift
      todos=()
      needupdate=1
      ;;
    (list)
      shift
      local i
      for (( i = 1; i <= $#todos; i++ )); do
        zformat -f tmp $list_format n:$i e:"${todos[$i]//\%/%%}"
        print -P "$tmp"
      done
      ;;
  esac
else
  if [[ $#todos -gt 0 ]]; then
    zformat -f tmp $short_format n:$#todos
    print -P "$tmp"
  fi
fi

(( $#todos )) &&
  __ztodolist[$PWD]=${(pj:\0:)todos} ||
  unset "__ztodolist[$PWD]"
(( needupdate )) &&
  print -r "__ztodolist=( ${(kv@qq)^^__ztodolist} )" > ${~cachefile}
__ztodolastwrite="${(%)tmp::="%D{%s}"}"
