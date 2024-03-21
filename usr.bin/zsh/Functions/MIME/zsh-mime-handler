# Handler for MIME types using associative arrays
# zsh_mime_handlers and zsh_mime_flags set up by zsh-mime-setup.
#
# The only flags it handles are copiousoutput and needsterminal.
# copiousoutput is assumed to imply needsterminal.  Apart from
# those, it tries to be a bit cunning about quoting, which
# can be a nightmare in MIME handling.  If it sees something like
#   netscape %s
# and it only has one file to handle (the usual case) then it will handle it
# internally just by appending a file.
#
# Anything else is handled by passing to sh -c, which is the only think
# with a high probability of working.  If it sees something with
# quotes, e.g.
#   /usr/bin/links "%s"
# it will assume someone else has tried to fix the quoting problem and not
# do that.  If it sees something with no quotes but other metacharacters,
# e.g.
#   cat %s | handler
# then it will do any quoting and pass the result to sh -c.
# So for example if the argument is "My File", the command executed
# is supposedly
#   sh -c 'cat My\ File | handler'
#
# This note is mostly here so you can work out what I tried to do when
# it goes horribly wrong.

local autocd
[[ -o autocd ]] && autocd=autocd

emulate -L zsh
setopt extendedglob cbases nullglob $autocd

# We need zformat from zsh/zutil for %s replacement.
zmodload -i zsh/zutil

autoload -Uz zsh-mime-contexts

# Look for options.  Because of the way this is usually invoked,
# (there is always a command to be handled), only handle options
# up to second last argument.
local opt
integer list
while (( $# - $OPTIND > 0 )); do
  if getopts "l" opt; then
    case $opt in
      (l)
      list=1
      ;;

      (*)
      return 1
      ;;
    esac
  else
    break
  fi
done
shift $(( OPTIND - 1 ))

# Always called with a filename argument first.
# There might be other arguments; don't really know what to do
# with these, but if they came from e.g. `*.ps' then we might
# just as well pass them all down.  However, we just take the
# suffix from the first since that's what invoked us via suffix -s.

local suffix s
local -a match mbegin mend

suffix=${1:t}
if [[ $suffix != *.* ]]; then
  "No suffix in command: $1" >&2
  return 1
fi
suffix=${suffix#*.}

local handler flags no_sh no_bg arg bg_flag="&"
integer i
local -a exec_asis hand_nonex exec_never

# Set to a list of patterns which are ignored and executed as they are,
# despite being called for interpretation by the mime handler.
# Defaults to executable files, which ensures that they are executed as
# they are, even if they have a suffix.
zsh-mime-contexts -a $suffix execute-as-is exec_asis || exec_asis=('*(*)' '*(/)')
zsh-mime-contexts -a $suffix execute-never exec_never

# Set to a list of patterns for which the handler will be used even
# if the file doesn't exist on the disk.
zsh-mime-contexts -a $suffix handle-nonexistent hand_nonex ||
  hand_nonex=('[[:alpha:]]#:/*')

# Set to true if the job should be disowned.
zsh-mime-contexts -t $suffix disown && bg_flag="&!"

local pattern
local -a files

# Search some path for the file, if required.
# We do this before any other tests that need to find the
# actual file or its directory.
local dir
local -a filepath
if zsh-mime-contexts -t $suffix find-file-in-path && [[ $1 != /* ]] &&
  [[ $1 != */* || -o pathdirs ]]; then
  zsh-mime-contexts -a $suffix file-path filepath || filepath=($path)
  for dir in $filepath; do
    if [[ -e $dir/$1 ]]; then
      1=$dir/$1
      break
    fi
  done
fi

# In case the pattern contains glob qualifiers, as it does by default,
# we need to do real globbing, not just pattern matching.
# The strategy is to glob the files in the directory using the
# pattern and see if the one we've been passed is in the list.
local dirpref=${1%/*}
if [[ $dirpref = $1 ]]; then
  dirpref=
else
  dirpref+=/
fi

for pattern in $exec_asis; do
  files=(${dirpref}${~pattern})
  if [[ -n ${files[(r)$1]} ]]; then
    for pattern in $exec_never; do
      [[ ${1:P} = ${~pattern} ]] && break 2
    done
    if (( list )); then
      for (( i = 1; i <= $#; i++ )); do
	(( i == 1 )) || print -n " "
	arg=${argv[i]}
	if [[ -n $arg ]]; then
	  print -rn -- ${(q)arg}
	else
	  print "''"
	fi
      done
      print
    else
      "$@"
    fi
    return
  fi
done

if [[ ! -e $1 ]]; then
  local nonex_ok
  for pattern in $hand_nonex; do
    if [[ $1 = ${~pattern} ]]; then
      nonex_ok=1
      break
    fi
  done
  if [[ -z $nonex_ok ]]; then
    if (( list )); then
      print -r -- "${(q)@}"
    else
      "$@"
    fi
    return
  fi
fi

if ! zsh-mime-contexts -s $suffix handler handler; then
  # Look for handler starting with longest suffix match.
  # Typically we'd only get a match for the shortest, but don't assume so.
  s=$suffix
  while true; do
    handler="${zsh_mime_handlers[$s]}"
    if [[ -n $handler ]]; then
      break
    fi
    if [[ $s = *.* ]]; then
      s=${s#*.}
    else
      break
    fi
  done
  if [[ -z $handler ]]; then
    if [[ $suffix = *.* ]]; then
      print "No handler specified for suffix .$suffix or any final part" >&2
    else
      print "No handler specified for suffix .$suffix" >&2
    fi
    return 1
  fi
fi
if ! zsh-mime-contexts -s $suffix flags flags; then
  # Same again for flags.
  s=$suffix
  while true; do
    flags="${zsh_mime_flags[$suffix]}"
    if [[ -n $flags ]]; then
      break
    fi
    if [[ $s = *.* ]]; then
      s=${s#*.}
    else
      break
    fi
  done
fi

# Set to yes if we use eval instead of sh -c for complicated mailcap lines
# Can possibly break some mailcap entries which expect sh compatibility,
# but is faster, as a new process is not spawned.
zsh-mime-contexts -t $suffix current-shell && no_sh=yes

# Set to yes if the process shouldn't be backgrounded even if it doesn't need a
# terminal and display is set.
zsh-mime-contexts -t $suffix never-background && no_bg=yes

local hasmeta stdin

# See if the handler has shell metacharacters in.
# Don't count whitespace since we can split that when it's unquoted.
if [[ $handler = *[\\\;\*\?\|\"\'\`\$]* ]]; then
    hasmeta=1
fi

local -a execargs files

if [[ $handler = *%s* ]]; then
  # We need to replace %s with the file(s).
  local command
  if [[ -n $hasmeta || $# -gt 1 ]]; then
    # The handler is complicated, either due to special
    # characters or multiple files.  We are going to pass it
    # down to sh, since it's probably written for sh syntax.
    #
    # See if it's a good idea to quote the filename(s).
    # It isn't if there are already quotes in the handler, since
    # that means somebody already tried to take account of that.
    if [[ $handler = *[\'\"]* ]]; then
      # Probably we ought not even to handle multiple
      # arguments, but at least the error message ought
      # to make it obvious what's going on.
      zformat -f command $handler s:"$argv[1]"
    else
      zformat -f command $handler s:"${(q)argv[1]}"
    fi
    if (( list )); then
      execargs=(${(Q)${(z)command}} ${argv[1,-1]})
    elif [[ $no_sh = yes ]]; then
      execargs=(eval $command)
    else
      execargs=(sh -c $command)
    fi
  else
    # Simple command, one filename.
    # Split and add the file without extra quoting,
    # since later we will just execute the array as is.
    for command in ${=handler}; do
	zformat -f command $command s:"$1"
	execargs+=($command)
    done
  fi
else
  # If there's no %s, the input is supposed to come from stdin.
  stdin=1
  if [[ -n $hasmeta && $no_sh != yes && list -eq 0 ]]; then
    execargs=(sh -c "$handler")
  else
    execargs=(${=handler})
  fi
fi

if (( list )); then
  for (( i = 1; i <= ${#execargs}; i++ )); do
    (( i == 1 )) || print -n " "
    arg=${execargs[i]}
    if [[ -n $arg ]]; then
      print -rn -- ${(q)arg}
    else
      print -n "''"
    fi
  done
  print
  return 0
fi

# Now execute the command in the appropriate fashion.
if [[ $flags = *copiousoutput* ]]; then
  # We need to page the output.
  # Careful in case PAGER is a set of commands and arguments.
  local -a pager
  zsh-mime-contexts -a $suffix pager pager || pager=(${=PAGER:-more})
  if [[ -n $stdin ]]; then
    cat $argv | $execargs | $pager
  else
    $execargs | eval ${PAGER:-more}
  fi
elif [[ $no_bg = yes || $flags = *needsterminal* || -z $DISPLAY ]]; then
  # Needs a terminal, so run synchronously.
  # Obviously, if $DISPLAY is empty but the handler needs a
  # GUI we are in trouble anyway.  However, it's possible for
  # the handler to be smart about this, like pick-web-browser,
  # and even if it just produces an error message it's better to
  # have it run synchronously.
  if [[ -n $stdin ]]; then
    cat $argv | $execargs
  else
    $execargs
  fi
else
  # Doesn't need a terminal and we have a $DISPLAY, so run
  # it in the background.  sh probably isn't smart enough to
  # exec the last command in the list, but it's not a big deal.
  #
  # The following Rococo construction is to try to make
  # the job output for the backgrounded command descriptive.
  # Otherwise it's equivalent to removing the eval and all the quotes,
  # including the (q) flags.
  if [[ -n $stdin ]]; then
    eval cat ${(q)argv} "|" ${(q)execargs} $bg_flag
  else
    eval ${(q)execargs} $bg_flag
  fi
fi
