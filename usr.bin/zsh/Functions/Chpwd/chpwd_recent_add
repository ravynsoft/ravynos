# Helper for chpwd_recent_dirs.
# Add a directory to the reply array unless we're skipping it.
# If skipping, return non-zero status.

local pat
local add=$1
local -a prune patterns

zstyle -a ':chpwd:' recent-dirs-prune prune
if (( ${#prune} )); then
  patterns=(${${prune:#^pattern:*}##pattern:})
fi

for pat in $patterns; do
  if [[ $add =~ ${~pat} ]]; then
    return 1
  fi
done

if [[ ${prune[(I)parent]} -ne 0 && $add = $reply[1]/* ]]; then
  # replace
  reply=($reply[2,-1])
fi
reply=($add $reply)
