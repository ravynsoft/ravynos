# See if we can extend the word context to something more specific.
# curcontext must be set to the base context by this point; it
# will be appended to directly.

emulate -L zsh
setopt extendedglob

local -a worcon bufwords
local pat tag lastword word backword forword
integer iword between

zstyle -a $curcontext word-context worcon || return 0

if (( ${#worcon} % 2 )); then
  zle -M "Bad word-context style in context $curcontext"
  return
fi

bufwords=(${(z)LBUFFER})
iword=${#bufwords}
lastword=${bufwords[-1]}
bufwords=(${(z)BUFFER})

if [[ $lastword = ${bufwords[iword]} ]]; then
  # If the word immediately left of the cursor is complete,
  # we're not on it for forward operations.
  forword=${bufwords[iword+1]}
  # If, furthermore, we're on whitespace, then we're between words.
  # It can't be significant whitespace because the previous word is complete.
  [[ $RBUFFER[1] = [[:space:]] ]] && between=1
else
  # We're on a word.
  forword=${bufwords[iword]}
fi
backword=${bufwords[iword]}

if [[ between -ne 0 && $curcontext = *between* ]]; then
  word=' '
elif [[ $curcontext = *back* ]]; then
  word=$backword
else
  word=$forword
fi

for pat tag in "${worcon[@]}"; do
  if [[ $word = ${~pat} ]]; then
    curcontext+=":$tag"
    return
  fi
done
