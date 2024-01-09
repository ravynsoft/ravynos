# Start of cdmatch.
# Save in your functions directory and autoload, then do
# compctl -x 'S[/][~][./][../]' -g '*(-/)' - \
#         'n[-1,/], s[]' -K cdmatch -S '/' -- cd pushd
#
# Completes directories for cd, pushd, ... anything which knows about cdpath.
# You do not have to include `.' in your cdpath.
#
# It works properly only if $ZSH_VERSION > 3.0-pre4.  Remove `emulate -R zsh'
# for all other values of $ZSH_VERSION > 2.6-beta2. For earlier versions
# it still works if RC_EXPAND_PARAM is not set or when cdpath is empty.
emulate -R zsh
setopt localoptions
local narg pref cdp

read -nc narg
read -Ac pref

cdp=(. $cdpath)
reply=( ${^cdp}/${pref[$narg]%$2}*$2(-/DN^M:t) )

return
# End of cdmatch.
