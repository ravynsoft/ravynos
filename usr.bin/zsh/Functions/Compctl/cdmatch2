# This function should be called from compctl to complete the
# second argument of cd and pushd.

emulate -R zsh				# Requires zsh 3.0-pre4 or later
setopt localoptions extendedglob
local from

read -Ac from
from="${from[2]}"

eval "reply=( \${PWD:s@$from@$1*$2@}~$PWD(ND-/:) )"
reply=( "${${reply[@]#${PWD%%$from*}}%${PWD#*$from}}" )
[[ ${#reply[(r),-1]} != 0 ]] && reply[(r)]="''"

return
