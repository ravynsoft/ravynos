# function zftp_progress {
# Basic progress metre, showing the percent of the file transferred.
# You want growing bars?  You gottem.
# styles used (context :zftp:zfparent_function:):
#   progress
#       empty or `none'                  no progress meter
#       `bar'                            use a growing bar of inverse video
#       `percent' or other non-blank     show the percentage transferred
#     If size of remote file is not available, `bar' or `percent' just show
#     bytes.
#   update
#       Minimum time in seconds between updates of the progress display.

local style update=1

# What style: either bar for growing bars, or anything else for simple
# percentage.  For bar we need to have the terminal width in COLUMNS,
# which is often set automatically, but you never know.
zstyle -s ":zftp$curcontext" progress style
# How many seconds to wait before printing an updated progress report.
zstyle -s ":zftp$curcontext" update update

# Don't show progress unless stderr is a terminal
[[ ! -t 2 || $style = (|none) ]] && return 0

if [[ -n $ZFTP_TRANSFER ]]; then
  # avoid a `parameter unset' message
  [[ $ZFTP_TRANSFER != *F ]] &&
    (( ${+zftpseconds} )) && (( SECONDS - zftpseconds < update )) && return
  # size is usually ZFTP_SIZE, but zftransfer may set ZFTP_TSIZE
  local size=${ZFTP_TSIZE:-$ZFTP_SIZE}
  if [[ ${size:-0} -ne 0 ]]; then
    local frac="$(( ZFTP_COUNT * 100 / size ))%"
    if [[ $style = bar && ${+COLUMNS} = 1 && $COLUMNS -gt 0 ]]; then
      if (( ! ${+zftpseconds} )); then
	print "$ZFTP_FILE ($size bytes): $ZFTP_TRANSFER" 1>&2
      fi
      integer maxwidth=$(( COLUMNS - 7 ))
      local width="$(( ZFTP_COUNT * maxwidth / size ))"
      print -nP "\r%S${(l:width:):-}%s${(l:maxwidth-width:):-}: ${frac}%%" 1>&2
    else
      print -n "\r$ZFTP_FILE ($size bytes): $ZFTP_TRANSFER $frac" 1>&2
    fi
  else
    print -n "\r$ZFTP_FILE: $ZFTP_TRANSFER $ZFTP_COUNT" 1>&2
  fi
  if [[ $ZFTP_TRANSFER = *F && ${+zftpseconds} = 1 ]]; then
    unset zftpseconds
    print 1>&2
  else
    typeset -g zftpseconds=$SECONDS
  fi
fi
# }
