local line calendar
local -a lockfiles editor

integer cal_running

if (( $# )); then
  editor=("$@")
else
  editor=(${VISUAL:-${EDITOR:-vi}})
fi

sched | while read line; do
  [[ $line = *" calendar -s "<->" "<-> ]] && (( cal_running = 1 ))
done

zstyle -s ':datetime:calendar:' calendar-file calendar || calendar=~/calendar

# start of subshell for OS file locking
(
# start of block for following always to clear up lockfiles.
# Not needed but harmless if OS file locking is used.
{
  if zmodload -F zsh/system b:zsystem && zsystem supports flock &&
    zsystem flock $calendar 2>/dev/null; then
    # locked OK
    :
  else
    calendar_lockfiles $calendar || exit 1
  fi

  eval $editor \$calendar
} always {
  (( ${#lockfiles} )) && rm -f $lockfiles
}
)

(( cal_running )) && calendar -s
