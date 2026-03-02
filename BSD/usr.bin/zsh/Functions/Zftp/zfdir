# function zfdir {
# Long directory of remote server.
# The remote directory is cached.  In fact, two caches are kept:
# one of the standard listing of the current directory, i.e. zfdir
# with no arguments, and another for everything else.
# To access the appropriate cache, just use zfdir with the same
# arguments as previously.  zfdir -r will also re-use the `everything
# else' cache; you can always reuse the current directory cache just
# with zfdir on its own.
#
# The current directory cache is emptied when the directory changes;
# the other is kept until a new zfdir with a non-empty argument list.
# Both are removed when the connection is closed.
#
# zfdir -f will force the existing cache to be ignored, e.g. if you know
#          or suspect the directory has changed.
# zfdir -d will remove both caches without listing anything.
# If you need to pass -r, -f or -d to the dir itself, use zfdir -- -d etc.;
# unrecognised options are passed through to dir, but zfdir options must
# appear first and unmixed with the others.

emulate -L zsh
setopt extendedglob

[[ $curcontext = :zf* ]] || local curcontext=:zfdir
local file opt optlist redir i newargs force
local curdir=$zfconfig[curdir_$ZFTP_SESSION]
local otherdir=$zfconfig[otherdir_$ZFTP_SESSION]

while [[ $1 = -* ]]; do
  if [[ $1 = - || $1 = -- ]]; then
    shift;
    break;
  elif [[ $1 != -[rfd]## ]]; then
    # pass options through to ls
    break;
  fi
  optlist=${1#-}
  for (( i = 1; i <= $#optlist; i++)); do
    opt=$optlist[$i]
    case $optlist[$i] in
      r) redir=1
	 ;;
      f) force=1
	 ;;
      d) [[ -n $curdir && -f $curdir ]] && rm -f $curdir
	 [[ -n $otherdir && -f $otherdir ]] && rm -f $otherdir
	 zffcache -d
	 return 0
	 ;;
    esac
  done
  shift
done

zfautocheck -d || return 1

# directory hack, see zfcd
for (( i = 1; i <= $#argv; i++ )); do
  if [[ $argv[$i] = $HOME || $argv[$i] = $HOME/* ]]; then
    argv[$i]="~${argv[$i]#$HOME}"
  fi
done

if [[ $# -eq 0 && $redir -ne 1 ]]; then
  # Cache it in the current directory file.  This means that repeated
  # calls to zfdir with no arguments always use a cached file.
  if [[ -z $curdir ]]; then
    curdir=${TMPPREFIX}zfcurdir_${ZFTP_SESSION}_$$
    zfconfig[curdir_$ZFTP_SESSION]=$curdir
  fi
  file=$curdir
else
  # Last directly looked at was not the current one, or at least
  # had non-standard arguments.
  if [[ -z $otherdir ]]; then
    otherdir=${TMPPREFIX}zfotherdir_${ZFTP_SESSION}_$$
    zfconfig[otherdir_$ZFTP_SESSION]=$otherdir
  fi
  file=$otherdir
  newargs="$*"
  if [[ -f $file && -n $newargs && $force -ne 1 ]]; then
    # Don't use the cached file if the arguments changed.
    # Even in zfdir -r new_args ...
    [[ $newargs = $zfconfig[otherargs_$ZFTP_SESSION] ]] || rm -f $file
  fi
  [[ -n $newargs ]] && zfconfig[otherargs_$ZFTP_SESSION]=$newargs
fi

if [[ $force -eq 1 ]]; then
  rm -f $file
  # if it looks like current directory has changed, better invalidate
  # the filename cache, too.
  (( $# == 0 )) && zffcache -d
fi

if [[ -n $file && -f $file ]]; then
  eval ${PAGER:-more} \$file
else
  if (zftp test); then
    # Works OK in subshells
    zftp dir $* | tee $file | eval ${PAGER:-more}
  else
    # Doesn't work in subshells (IRIX 6.2 --- why?)
    zftp dir $* >$file
    eval ${PAGER:-more} $file
  fi
fi
# }
