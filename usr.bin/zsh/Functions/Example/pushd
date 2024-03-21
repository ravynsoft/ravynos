# pushd function to emulate the old zsh behaviour.  With this function
# pushd +/-n just lifts the selected element to the top of the stack
# instead of just cycling the stack.

local puid
[[ -o pushdignoredups ]] && puid=1

emulate -R zsh
setopt localoptions

if [[ ARGC -eq 1 && "$1" == [+-]<-> ]] then
	setopt pushdignoredups
	builtin pushd ~$1
else
	[[ -n $puid ]] && setopt pushdignoredups
	builtin pushd "$@"
fi
