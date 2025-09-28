# function zfls {

emulate -L zsh
[[ $curcontext = :zf* ]] || local curcontext=:zfls

# directory hack, see zfcd
if [[ $1 = $HOME || $1 = $HOME/* ]]; then
  1="~${1#$HOME}"
fi

zfautocheck -d

zftp ls $*
# }
