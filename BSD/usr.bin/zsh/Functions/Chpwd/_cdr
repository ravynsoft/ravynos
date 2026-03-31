#compdef cdr

local expl insert_string
integer default insert

zstyle -t ':chpwd:' recent-dirs-default && default=1
if (( default )); then
  zstyle -s ":completion:${curcontext}:" recent-dirs-insert insert_string
  case $insert_string in
    (both)
    insert=4
    ;;

    (fallback)
    insert=3
    ;;

    (always)
    insert=2
    ;;

    ([tT]*|1|[yY]*)
    insert=1
    ;;

    (*)
    insert=0
  esac
fi

# See if we should fall back to cd completion.
if [[ default -ne 0 && insert -lt 2 && \
  ( CURRENT -ne 2 || (-n $words[2] && $words[2] != <->) ) ]]; then
  $_comps[cd] "$@"
  return
fi

local -a values keys

if (( insert )); then
  # insert the actual directory, not the number
  values=(${${(f)"$(cdr -l)"}##<-> ##})
  # Suppress the usual space suffix, since there's no further argument
  # and it's useful to be able to edit the directory e.g. add /more/stuff.
  if _wanted -V recent-dirs expl 'recent directory' compadd -S '' -Q -a values
  then
    (( insert == 4 )) || return 0
  fi

  (( insert >= 3 )) || return
  $_comps[cd] "$@"
else
  values=(${${(f)"$(cdr -l)"}/ ##/:})
  keys=(${values%%:*})

  _describe -t dir-index 'recent directory index' values keys -V unsorted
fi
