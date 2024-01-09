# insert-files() {

# Autoload this function, run `zle -N <func-name>' and bind <func-name>
# to a key.

# This function allows you type a file pattern, and see the results of the
# expansion at each step.  When you hit return, they will be inserted into
# the command line.

emulate -L zsh
setopt nobadpattern

local key str files

files=( *(N:q) )
if (( $#files )); then
  zle -R "files: ${str}_" "$files[@]"
else
  zle -R "files: ${str}_ (failed)"
fi
read -k key
while [[ '#key' -ne '#\\r' && '#key' -ne '#\\n' &&
         '#key' -ne '#\\C-g' ]]; do
  if [[ '#key' -eq '#\\C-h' || '#key' -eq '#\\C-?' ]]; then
    [[ -n "$str" ]] && str="$str[1,-2]"
  else
    str="$str$key"
  fi
  eval "files=( \${~str}*(N:q) )"
  if (( $#files )); then
    zle -R "files: ${str}_" "$files[@]"
  else
    zle -R "files: ${str}_ (failed)"
  fi
  read -k key
done
zle -Rc
if [[ '#key' -ne '#\\C-g' && $#files -gt 0 ]]; then
  [[ "$LBUFFER[-1]" = ' ' ]] || files=('' "$files[@]")
  LBUFFER="$LBUFFER$files "
fi
# }
