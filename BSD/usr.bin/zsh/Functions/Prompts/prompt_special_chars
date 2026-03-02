# Utility function to set up some special characters
# used by prompts.
#
# These used to be defined to characters found in particular
# character sets (e.g. IBM852) which now aren't widely used.
# We still provide them in that form if the current character
# set isn't UTF-8.  We could in principle use iconv if available.

typeset -gA schars

if [[ ${LC_ALL:-${LC_CTYPE:-$LANG}} = *(UTF-8|utf8)* ]]; then
  schars[300]=$'\xe2\x94\x94'
  schars[304]=$'\xe2\x94\x8c'
  schars[332]=$'\xe2\x94\x8c'
  schars[333]=$'\xe2\x96\x88'
  schars[371]=$'\xc2\xa8'
  schars[372]=$'\xcb\x99'
  schars[262]=$'\xe2\x96\x93'
  schars[261]=$'\xe2\x96\x92'
  schars[260]=$'\xe2\x96\x91'
else
  local code
  for code in 300 304 332 333 371 372 262 261 260; do
    eval "schars[$code]=\$'\\$code'"
  done
fi
