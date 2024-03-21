# function zfrglob {
# Do the remote globbing for zfput, etc.
# We have two choices:
#  (1) Get the entire file list and match it one by one
#      locally against the pattern.
#      Causes problems if we are globbing directories (rare, presumably).
#      But: we can cache the current directory, which
#      we need for completion anyway.  Works on any OS if you
#      stick with a single directory.  This is the default.
#  (2) Use remote globbing, i.e. pass it to ls at the site.
#      Faster, but only works with UNIX, and only basic globbing.
#      We do this if the remote-glob style (or $zfrglob for
#      backward compatibility) is set.

# There is only one argument, the variable containing the
# pattern to be globbed.  We set this back to an array containing
# all the matches.

emulate -L zsh
setopt extendedglob

local pat dir nondir files i zfrglob

zstyle -t "$curcontext" remote-glob && zfrglob=1

eval pat=\$$1

# Check if we really need to do anything.  Look for standard
# globbing characters, and if we are
# using zsh for the actual pattern matching also look for
# extendedglob characters.
if [[ $pat != *[][*?]* &&
  ( -n $zfrglob || $pat != *[(|)#^]* ) ]]; then
  return 0
fi

if [[ $zfrglob != '' ]]; then
  () {
    zftp ls "$pat" >|$1 2>/dev/null
    eval "$1=(\$(<\$1))"
  } =(<<<'temporary file')
else
  if [[ $ZFTP_SYSTEM = UNIX* && $pat = */* ]]; then
    # not the current directory and we know how to handle paths
    if [[ $pat = ?*/* ]]; then
      # careful not to remove too many slashes
      dir=${pat%/*}
    else
      dir=/
    fi
    nondir=${pat##*/}
    () {
      zftp ls "$dir" 2>/dev/null >|$1
      files=($(<$1))
    } =(<<<'temporary file')
    files=(${files:t})
  else
    # we just have to do an ls and hope that's right
    local fcache_name
    zffcache
    nondir=$pat
    files=(${(P)fcache_name})
  fi
  # now we want to see which of the $files match $nondir:
  # ${...:/foo} deletes occurrences of foo matching a complete word,
  # while the ^ inverts the sense so that anything not matching the
  # pattern in $nondir is excluded.
  eval "$1=(\${files:/^\${~nondir}})"
fi
# }
