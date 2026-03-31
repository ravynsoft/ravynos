#autoload

# Line motions that do not leave the current history entry,
# for editing in multi-line buffers.

# To use:
#  autoload -Uz move-line-in-buffer
#  zle -N up-line-in-buffer move-line-in-buffer
#  zle -N down-line-in-buffer move-line-in-buffer
#
# then bindkey as you prefer

local hno=$HISTNO curs=$CURSOR
zle .${WIDGET:s/in-buffer/or-history} "$@" &&
    (( HISTNO != hno && (HISTNO=hno, CURSOR=curs) ))
return 0
