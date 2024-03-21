# zfmark [bname]
# Set a bookmark for the current zftp connection, or use the
# information about the last session if there isn't one.
# A bookmark includes both the host *and* the directory on that host.
#
# Without bname, list the current bookmarks and their locations.

emulate -L zsh
setopt extendedglob
[[ $curcontext = :zf* ]] || local curcontext=:zfmark

# Set ZFTP_BMFILE if not already set.  This should agree with
# the corresponding line in zfgoto.
: ${ZFTP_BMFILE:=${ZDOTDIR:-$HOME}/.zfbkmarks}

typeset -A bkmarks
local line

if [[ -f $ZFTP_BMFILE ]]; then
  # read in file:  could optimise this by recording last read time
  # comparing with file
  while read -r line; do
    # ignore blank and comment lines
    [[ $line = [[:blank:]]# || $line = [[:blank:]]#'#'* ]] && continue
    bkmarks[${line%% *}]="${line#* }"
  done <$ZFTP_BMFILE
fi

if (( $# == 0 )); then
  for line in ${(ko)bkmarks}; do
    print -r- "$line ${bkmarks[$line]}"
  done
  return 0
elif (( $# > 1 )); then
  print "Usage: zfmark [bookmark]" >&2
  return 1
fi

if [[ -n $ZFTP_HOST ]]; then
  bkmarks[$1]="${ZFTP_USER}@${ZFTP_HOST}:${ZFTP_PWD}"
elif [[ -n $zfconfig[lastloc_$ZFTP_SESSION] ]]; then
  bkmarks[$1]="${zfconig[lastuser_$ZFTP_SESSION]}@\
${zfconfig[lastloc_$ZFTP_SESSION]}"
else
  print "No current or recent ZFTP session to bookmark." >&2
  return 1
fi

for line in ${(ko)bkmarks}; do
  print -r- "$line ${bkmarks[$line]}"
done >$ZFTP_BMFILE
