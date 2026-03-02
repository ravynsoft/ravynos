#
# Test whether $ZSH_VERSION (or some value of your choice, if a second argument
# is provided) is greater than or equal to x.y.z-r (in argument one). In fact,
# it'll accept any dot/dash-separated string of numbers as its second argument
# and compare it to the dot/dash-separated first argument. Leading non-number
# parts of a segment (such as the "zefram" in 3.1.2-zefram4) are not considered
# when the comparison is done; only the numbers matter. Any left-out segments
# in the first argument that are present in the version string compared are
# considered as zeroes, eg 3 == 3.0 == 3.0.0 == 3.0.0.0 and so on.
#
# Usage examples:
# is-at-least 3.1.6-15 && setopt NO_GLOBAL_RCS
# is-at-least 3.1.0 && setopt HIST_REDUCE_BLANKS
# is-at-least 586 $MACHTYPE && echo 'You could be running Mandrake!'
# is-at-least $ZSH_VERSION || print 'Something fishy here.'
#
# Note that segments that contain no digits at all are ignored, and leading
# text is discarded if trailing digits follow, because this was the meaning
# of certain zsh version strings in the early 2000s.  Other segments that
# begin with digits are compared using NUMERIC_GLOB_SORT semantics, and any
# other segments starting with text are compared lexically.

emulate -L zsh

local IFS=".-" min_cnt=0 ver_cnt=0 part min_ver version order

min_ver=(${=1})
version=(${=2:-$ZSH_VERSION} 0)

while (( $min_cnt <= ${#min_ver} )); do
  while [[ "$part" != <-> ]]; do
    (( ++ver_cnt > ${#version} )) && return 0
    if [[ ${version[ver_cnt]} = *[0-9][^0-9]* ]]; then
      # Contains a number followed by text.  Not a zsh version string.
      order=( ${version[ver_cnt]} ${min_ver[ver_cnt]} )
      if [[ ${version[ver_cnt]} = <->* ]]; then
        # Leading digits, compare by sorting with numeric order.
        [[ $order != ${${(On)order}} ]] && return 1
      else
        # No leading digits, compare by sorting in lexical order.
        [[ $order != ${${(O)order}} ]] && return 1
      fi
      [[ $order[1] != $order[2] ]] && return 0
    fi
    part=${version[ver_cnt]##*[^0-9]}
  done

  while true; do
    (( ++min_cnt > ${#min_ver} )) && return 0
    [[ ${min_ver[min_cnt]} = <-> ]] && break
  done

  (( part > min_ver[min_cnt] )) && return 0
  (( part < min_ver[min_cnt] )) && return 1
  part=''
done
