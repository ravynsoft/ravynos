# With arguments, output those files to the recent directory file.
# With no arguments, read in the directories from the file into $reply.
#
# Handles recent-dirs-file and recent-dirs-max styles.

emulate -L zsh
setopt extendedglob

integer max
local file line
local -a files dir
local default=${ZDOTDIR:-$HOME}/.chpwd-recent-dirs

if zstyle -a ':chpwd:' recent-dirs-file files; then
  files=(${files//(#s)+(#e)/$default})
fi
if (( ${#files} == 0 )); then
  files=($default)
fi

zstyle -s ':chpwd:' recent-dirs-max max || max=20

if (( $# )); then
  if (( max > 0 && ${#argv} > max )); then
    argv=(${argv[1,max]})
  fi
  # Quote on write.
  # Use $'...' quoting... this fixes newlines and other nastiness.
  print -rl ${(qqqq)argv} >${files[1]}
else
  typeset -g reply
  # Unquote on read.
  reply=()
  for file in $files; do
    [[ -r $file ]] || continue
    # Strip anything after the directory from the line.
    # At the moment there isn't anything, but we'll make this
    # future proof.
    for line in ${(f)"$(<$file)"}; do
      dir=(${(z)line})
      reply+=(${(Q)${dir[1]}})
      if (( max > 0 && ${#reply} == max )); then
	break 2
      fi
    done
  done
fi
