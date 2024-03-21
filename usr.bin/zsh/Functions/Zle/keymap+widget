#autoload

##
# self-insert-by-keymap originally appeared in zsh-users/10559 (July 2006).
# Changes have been made to the widget naming scheme, based on feedback on
# the mailing list thread.
##

emulate -L zsh
zmodload -i zsh/zleparameter || return 1

# Rebind the most common widgets to override in multiple keymaps.  Ideally
# complete-word would also be in this list, but so many other things
# already rebind complete-word that doing so here is a waste of effort.

local -a m
local w='' k=''
for w in self-insert accept-line forward-char backward-char \
         up-{,line-or-}history down-{,line-or-}history \
         magic-space backward-delete-char delete-char-or-list
do

  # If this is run early enough that all the widgets are still builtins,
  # no explicit remapping is needed.  If they've already been rebound,
  # it's not safe to assume we can do so again.

  if [[ $widgets[$w] != (builtin|user:$w-by-keymap) ]]
  then
      m+="Cannot rebind $w: $widgets[$w]"
      continue
  fi

  function $w-by-keymap {
      if (( $+widgets[$KEYMAP+$WIDGET] == 1 ))
      then zle $KEYMAP+$WIDGET "$@"
      else zle .$WIDGET "$@"
      fi
  }

  zle -N $w $w-by-keymap

done

[[ -n $m ]] && { zle && zle -M "${(F)m}" || print -l -u2 -R $m }

return 0

# With this in place, you should rarely need "zle -N self-insert frob"
# again.  Instead you do this:
#
# 	bindkey -N frobber main
# 	zle -N frobber+self-insert frob
#
# Then, whenever you wish to replace self-insert with frob, change
# keymaps:
#
# 	zle recursive-edit -K frobber

# Here's a simple example, which improves upon the caps-lock example in
# the zsh manual entry for recursive-edit:
#
#   ucase+self-insert() {
#     LBUFFER+=${(U)KEYS[-1]}
#   }
#   zle -N ucase+self-insert
#   caps-lock() {
#     bindkey -N ucase $KEYMAP
#     bindkey -M ucase "$KEYS" .accept-line
#     zle recursive-edit -K ucase || zle send-break
#   }
#   zle -N caps-lock
#
# To turn this on, pick a key sequence (I've chosen ctrl-x shift-L) and
# bind the caps-lock widget to it:
#
#   bindkey -M main '^XL' caps-lock

# Another example of using a continuation widget to propagate accept-line
# (or any other binding from the original keymap) through the caller:
#
#  bindkey -N newkeymap $KEYMAP
#  recursive-edit-and-accept() {
#    local -a __accepted
#    zle -N newkeymap+accept-line end-recursive-edit
#    zle recursive-edit -K newkeymap || zle send-break
#    if [[ ${__accepted[0]} != end-recursive-edit ]]
#    then zle "${__accepted[@]}"; return
#    else return 0
#    fi
#  }
#  end-recursive-edit() {
#    __accepted=($WIDGET ${=NUMERIC:+-n $NUMERIC} "$@")
#    zle .accept-line
#    return 0
#  }
