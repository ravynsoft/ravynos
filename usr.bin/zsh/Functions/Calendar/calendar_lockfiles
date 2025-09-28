# Lock the given files.
# Append the names of lockfiles to the array lockfiles.

local file lockfile msgdone
# Number of attempts to lock a file.  Probably not worth stylising.
integer lockattempts=4 loadtried

# The lockfile name is not stylised: it has to be a fixed
# derivative of the main fail.
for file; do
  lockfile=$file.lockfile
  for (( i = 0; i <= lockattempts; i++ )); do
    if ln -s $file $lockfile >/dev/null 2>&1; then
      lockfiles+=($lockfile)
      break
    fi
    if zle && [[ -z $msgdone ]]; then
      msgdone="${lockfile}: waiting to acquire lock"
      zle -M $msgdone
    fi
    if (( ! loadtried )); then
      zmodload -i zsh/zselect 2>/dev/null
      (( loadtried = 1 ))
    fi
    if zmodload -e zsh/zselect; then
      # This gives us finer grained timing (100th second).
      # Randomize the sleep between .1 and 2 seconds so that
      # we are much less likely to have multiple instances
      # retrying at once.
      zselect -t $(( 10 + RANDOM * 190 / 32768 ))
    else
      sleep 1
    fi
  done
  if [[ -n $msgdone ]]; then
    zle -M ${msgdone//?/ }
    msgdone=
  fi
  if [[ ${lockfiles[-1]} != $lockfile ]]; then
    msgdone="Failed to lock $file; giving up after $lockattempts attempts.
Another instance of calendar may be using it.
Delete $lockfiles if you believe this to be an error."
    if zle; then
      zle -M $msgdone
    else
      print $msgdone >&2
    fi
    # The parent should take action to delete any lockfiles
    # already locked.  Typically this won't be necessary, since
    # we will always lock the main calendar file first.
    return 1
  fi
done

return 0
