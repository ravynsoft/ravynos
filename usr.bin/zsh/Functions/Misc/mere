# read a man page

setopt localoptions extendedglob

local manual="$1" col=col terminal=man magic line

# /usr/bin/col on SunOS 4 doesn't support -x.
if [[ -x /usr/5bin/col ]]; then
  col=/usr/5bin/col;
fi

# SunOS 5 has no `man' terminal.
if [[ -d /usr/share/lib/nterm &&
    ! -e /usr/share/lib/nterm/tab.$terminal ]]; then
  terminal=lp;
fi

# HP-UX has no `man' terminal.
if [[ -d /usr/share/lib/term &&
    ! -e /usr/share/lib/term/tab$terminal ]]; then
  terminal=lp;
fi

# IRIX has no `man' terminal.
if [[ -d /usr/lib/nterm &&
    ! -e /usr/lib/nterm/tab.$terminal ]]; then
  terminal=lp;
fi

# Unixware has no `man' terminal.
if [[ -d /usr/ucblib/doctools/nterm &&
    ! -e /usr/ucblib/doctools/nterm/tab.$terminal ]]; then
  terminal=lp;
fi

# Solaris has SGML manuals.
if [[ -f /usr/lib/sgml/sgml2roff ]] && 
   [[ "$(read -er < $manual)" = "<!DOCTYPE"* ]]; then
  /usr/lib/sgml/sgml2roff $manual | {
    read -r line
    if [[ $line = ".so "* ]]; then
      # There is no cascading .so directive.
      # On Solaris 7, at least.
      /usr/lib/sgml/sgml2roff ${line#.so }
    else
      print -lr - "$line"
      cat
    fi
  }
else
  read -u0 -k 2 magic < $manual
  case $magic in
  $'\037\235') zcat $manual;;
  $'\037\213') gzip -dc $manual;;
  *) cat $manual;;
  esac
fi | (
  # cd is required to work soelim called by nroff.
  case $manual in
  */man/man*/*) cd ${manual:h:h};;
  */man/sman*/*) cd ${manual:h:h};;
  esac
  read -r line
  # The first line beginning with '\" shows preprocessors.
  # Unknown preprocessors is ignored.
  if [[ $line = "'\\\" "* ]]; then
    typeset -A filter
    filter=(
      e neqn
      g grap
      p pic
      r refer
      t tbl
      v vgrind
    )
    eval ${(j:|:)${${(s::)line#\'\\\" }//(#m)?/$filter[$MATCH]}}
  elif [[ $line = "'\\\"! "* ]]; then
    typeset -A filter
    filter=(
      eqn neqn
    )
    eval ${(j:|:)${${${${(s:|:)line#\'\\\"! }# ##}% ##}//(#m)*/$filter[$MATCH]}}
  else
    print -lr - "$line"
    cat
  fi |
  nroff -T$terminal -man | $col -x
) |
${=MANPAGER:-${PAGER:-more}} -s
