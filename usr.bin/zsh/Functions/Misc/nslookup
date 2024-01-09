# Simple wrapper function for `nslookup'. With completion if you are using
# the function based completion system.

if [[ $argv[(I)-] -eq 0 && $argv[(I)[^-]*] -ne 0 ]]; then
  command nslookup "$@"
  return
fi

setopt localoptions localtraps completealiases

local tmp line compcontext=nslookup curcontext='nslookup:::' pmpt
local pager opager="$PAGER"
typeset +g -x PAGER=cat

zmodload -e zsh/zpty || zmodload -i zsh/zpty

trap 'return 130' INT
trap 'zpty -d nslookup' EXIT

pmpt=()
zstyle -s ':nslookup' prompt tmp && pmpt=(-p "$tmp")
zstyle -s ':nslookup' rprompt tmp && pmpt=("$pmpt[@]" -r "$tmp")
zstyle -s ':nslookup' pager tmp &&
    [[ -z "$pager" ]] && pager="${opager:-more}"
(( $#pmpt )) || pmpt=(-p '> ')

zpty nslookup command nslookup "${(q)@}"

zpty -r nslookup line '*> '
print -nr "$line"

while line=''; vared -he "$pmpt[@]" line; do
  print -s "$line"
  [[ "$line" = exit ]] && break

  zpty -w nslookup "$line"

  zpty -r nslookup line '(|*
)> '
  if [[ -n "$pager" && ${#${(f)line}} -gt LINES ]]; then
    print -nr "$line" | eval "$pager"
  else
    print -nr "$line"
  fi
done

zpty -w nslookup 'exit'
