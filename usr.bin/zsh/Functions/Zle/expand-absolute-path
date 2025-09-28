# expand-absolute-path
# This is a ZLE widget to expand the absolute path to a file,
# using directory naming to shorten the path where possible.

emulate -L zsh
setopt extendedglob cbases

autoload -Uz modify-current-argument

if (( ! ${+functions[glob-expand-absolute-path]} )); then
  glob-expand-absolute-path() {
    local -a files
    files=(${~1}(N:P))
    (( ${#files} )) || return
    REPLY=${(D)files[1]}
  }
fi

modify-current-argument glob-expand-absolute-path
