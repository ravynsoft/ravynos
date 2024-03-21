# This tries to find wordcode files and automatically re-compile them if
# at least one of the original files is newer than the wordcode file.
# This will only work if the original files were added with their full
# paths or if the names stored in the wordcode files are relative to the
# directory where the wordcode file is.
#
# Arguments are the names of wordcode files and directories containing
# wordcode files that should be checked. If no arguments are given, the
# directories and wordcode files in $fpath are used.
#
# And then there are two options:
#   -t: Only check if there are wordcode files that have to be 
#       re-compiled. The return status is zero if there are files
#       that need to be re-compiled and non-zero otherwise.
#   -q: Be quiet, i.e.: only set the return status.
#   -p: If this is given, the arguments are interpreted differently:
#       they should form one or more sets of arguments for zcompile,
#       separated by `--'. For example:
#
#         zrecompile -p \
#                    -R ~/.zshrc -- \
#                    -M ~/.zcompdump -- \
#                    ~/zsh/comp.zwc ~/zsh/Completion/*/_* \
#
#       This makes ~/.zshrc be compiled into ~/.zshrc.zwc if that doesn't
#       exist or if it is older than ~/.zshrc. The wordcode file will be
#       marked for reading instead of mapping. The same is done for
#       ~/.zcompdump and ~/.zcompdump.zwc, but the wordcode file is marked
#       for mapping. The last line re-creates the file ~/zsh/comp.zwc if
#       any of the files matching the given pattern is newer than it.
#
# Without the -t option, the return status is zero if all wordcode files
# that needed re-compilation could be compiled and non-zero if compilation
# for at least one of the files failed.

setopt localoptions extendedglob noshwordsplit noksharrays

local opt check quiet zwc files re file pre ret map tmp mesg pats

tmp=()
while getopts ":tqp" opt; do
  case $opt in
  t) check=yes ;;
  q) quiet=yes ;;
  p) pats=yes  ;;
  *)
    if [[ -n $pats ]]; then
      tmp=( $tmp $OPTARG )
    else
      print -u2 zrecompile: bad option: -$OPTARG
      return 1
    fi
  esac
done
shift OPTIND-${#tmp}-1

if [[ -n $check ]]; then
  ret=1
else
  ret=0
fi

if [[ -n $pats ]]; then
  local end num

  while (( $# )); do
    end=$argv[(i)--]

    if [[ end -le $# ]]; then
      files=( $argv[1,end-1] )
      shift end
    else
      files=( $argv )
      argv=()
    fi

    tmp=()
    map=()
    OPTIND=1
    while getopts :MR opt $files; do
      case $opt in
      [MR]) map=( -$opt ) ;;
      *) tmp=( $tmp $files[OPTIND] );;
      esac
    done
    shift OPTIND-1 files
    (( $#files )) || continue

    files=( $files[1] ${files[2,-1]:#*(.zwc|~)} )

    (( $#files )) || continue

    zwc=${files[1]%.zwc}.zwc
    shift 1 files

    (( $#files )) || files=( ${zwc%.zwc} )

    if [[ -f $zwc ]]; then
      num=$(zcompile -t $zwc | wc -l)
      if [[ num-1 -ne $#files ]]; then
        re=yes
      else
        re=
        for file in $files; do
          if [[ $file -nt $zwc ]]; then
            re=yes
	    break
          fi
        done
      fi
    else
      re=yes
    fi

    if [[ -n $re ]]; then
      if [[ -n $check ]]; then

        # ... say so.

        [[ -z $quiet ]] && print $zwc needs re-compilation
        ret=0
      else

        # ... or do it.

        [[ -z $quiet ]] && print -n "re-compiling ${zwc}: "

        # If the file is mapped, it might be mapped right now, so keep the
	# old file by renaming it.

	if [[ -z "$quiet" ]] &&
           { [[ ! -f $zwc ]] || mv -f $zwc ${zwc}.old } &&
           zcompile $map $tmp $zwc $files; then
          print succeeded
	elif ! { { [[ ! -f $zwc ]] || mv -f $zwc ${zwc}.old } &&
                 zcompile $map $tmp $zwc $files 2> /dev/null } then
          [[ -z $quiet ]] && print "re-compiling ${zwc}: failed"
          ret=1
        fi
      fi
    fi
  done

  return ret
fi

# Get the names of wordcode files.

if (( $# )); then
  argv=( ${^argv}/*.zwc(ND)  ${^argv}.zwc(ND)  ${(M)argv:#*.zwc}  )
else
  argv=( ${^fpath}/*.zwc(ND) ${^fpath}.zwc(ND) ${(M)fpath:#*.zwc} )
fi

# We only handle *.zwc files. zcompile only handles *.zwc files. Everybody
# seems to handle only *.zwc files.

argv=( ${^argv%.zwc}.zwc )

for zwc; do

  # Get the files in the wordcode file.

  files=( ${(f)"$(zcompile -t $zwc)"} )

  # See if the wordcode file will be mapped.

  if [[ $files[1] = *\(mapped\)* ]]; then
    map=-M
    mesg='succeeded (old saved)'
  else
    map=-R
    mesg=succeeded
  fi

  # Get the path prefix of the wordcode file to prepend it to names of
  # original files that are relative pathnames.
  
  if [[ $zwc = */* ]]; then
    pre=${zwc%/*}/
  else
    pre=
  fi

  # Maybe this is even for an older version of the shell?

  if [[ $files[1] != *$ZSH_VERSION ]]; then
    re=yes
  else
    re=
  fi

  files=( ${pre}${^files[2,-1]:#/*} ${(M)files[2,-1]:#/*} )

  # If the version is correct, compare the age of every original file
  # to the age of the wordcode file.

  [[ -z $re ]] &&
    for file in $files; do
      if [[ $file -nt $zwc ]]; then
        re=yes
        break
      fi
    done

  if [[ -n $re ]]; then

    # The wordcode files needs re-compilation...

    if [[ -n $check ]]; then

      # ... say so.

      [[ -z $quiet ]] && print $zwc needs re-compilation
      ret=0
    else

      # ... or do it.

      [[ -z $quiet ]] && print -n "re-compiling ${zwc}: "

      tmp=( ${^files}(N) )

      # Here is the call to zcompile, but if we can't find all the original
      # files, we don't try compilation.

      if [[ $#tmp -ne $#files ]]; then
        [[ -z $quiet ]] && print 'failed (missing files)'
        ret=1
      else

        # If the file is mapped, it might be mapped right now, so keep the
	# old file by renaming it.

	if [[ -z "$quiet" ]] &&
           mv -f $zwc ${zwc}.old &&
           zcompile $map $zwc $files; then
          print $mesg
	elif ! { mv -f $zwc ${zwc}.old &&
                 zcompile $map $zwc $files 2> /dev/null } then
          [[ -z $quiet ]] && print "re-compiling ${zwc}: failed"
          ret=1
        fi
      fi
    fi
  fi
done

return ret
