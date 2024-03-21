# Helper for zsh-mime-handler.
#
# Pass in a zstyle option, a suffix, which might include multiple parts
# (e.g. pdf.gz), plus remaining zstyle arguments plus arguments to zstyle.
# Try to match the style starting with the longest possible suffix.

local context suffix option

option=$1
shift
suffix=$1
shift

while true; do
  context=":mime:.${suffix}:"
  zstyle $option $context "$@" && return 0
  if [[ $suffix = *.* ]]; then
    suffix=${suffix#*.}
  else
    break
  fi
done

return 1
