## vim:ft=zsh

# VCS_INFO_hexdump FILENAME BYTECOUNT
#
# Return in $REPLY a hexadecimal representation (lowercase, no whitespace)
# of the first BYTECOUNT bytes of FILENAME.

if [[ -r $1 ]]; then
  setopt localoptions nomultibyte extendedglob
  local val
  read -k $2 -u 0 val <$1
  REPLY=${(Lj::)${(l:2::0:)${(@s//)val}//(#m)*/$(( [##16] ##$MATCH ))}}
else
  return 1
fi

