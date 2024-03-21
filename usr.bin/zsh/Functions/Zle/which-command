zmodload -i zsh/parameter zsh/zutil

zle -I

local -a whencecmd wds

# Set the whence style to your favourite function
# (but NOT which-command!)
zstyle -a :zle:$WIDGET whence whencecmd || whencecmd=(whence -c --)

wds=(${(z)LBUFFER})
local wd barewd
local -A seen

while true; do
  wd=${wds[1]}
  barewd=${(Q)wd}

  if [[ $barewd != $wd || -n $seen[$barewd] ]]; then
    # quoted or already expanded, see if original word is an alias...
    if [[ -z $seen[$barewd] && -n $aliases[$wd] ]]; then
      # yes, so we need to decode that, with no extra expansion...
      $whencecmd $wd
      seen[$wd]=1
      wds=(${(z)aliases[$wd]})
      continue
    else
      # use unquoted word, don't expand alias
      (unalias -- $barewd 2>/dev/null; $whencecmd $barewd)
    fi
  else
    # turn on globsubst for =ls etc.
    $whencecmd ${~barewd}
    if [[ -n $aliases[$barewd] && -z $seen[$barewd] ]]; then
      # Recursively expand aliases
      seen[$barewd]=1
      wds=(${(z)aliases[$barewd]})
      continue
    fi
  fi
  break
done
