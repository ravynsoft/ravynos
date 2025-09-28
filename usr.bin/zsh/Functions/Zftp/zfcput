# function zfcput {
# Continuation put of files from remote server.
# For each file, if it's shorter over there, put the remainder from
# over here.  This uses append, which is standard, so unlike zfcget it's
# expected to work on any reasonable server... err, as long as it
# supports SIZE and MDTM.  (It could be enhanced so you can enter the
# size so far by hand.)  You should probably be in binary transfer
# mode, thought it's not enforced.
#
# To read from midway through a local file, `tail +<n>c' is used.
# It would be nice to find a way of doing this which works on all OS's.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfcput
local loc rem stat=0 locst remst offs tailtype

# find how tail works.  this is intensely annoying, since it's completely
# standard in C.  od's no use, since we can only skip whole blocks.
if [[ $(echo abcd | tail +2c) = bcd ]]; then
  tailtype=c
elif [[ $(echo abcd | tail --bytes=+2) = bcd ]]; then
  tailtype=b
else
  print "I can't get your \`tail' to start from arbitrary characters.\n" \
  "If you know how to do this, let me know." 2>&1
  return 1
fi

for loc in $*; do
  # zfcd directory hack to put the front back to ~
  rem=$loc
  if [[ $rem = $HOME || $rem = $HOME/* ]]; then
    rem="~${rem#$HOME}"
  fi
  if [[ ! -r $loc ]]; then
    print "Can't read file $loc"
    stat=1
  else
    # Compare the sizes.
    locst=($(zftp local $loc))
    () {
      zftp remote $rem >|$1
      rstat=$?
      remst=($(<$1))
    } =(<<<'temporary file')
    if [[ $rstat = 2 ]]; then
      print "Server does not support remote status commands.\n" \
      "You will have to find out the size by hand and use zftp append." 2>&1
      stat=1
      continue
    elif [[ $rstat = 1 ]]; then
      # Not found, so just do a standard put.
      zftp put $rem <$loc
    elif [[ $remst[1] -gt $locst[1] ]]; then
      print "Remote file is larger!" 2>&1
      continue;
    elif [[ $locst[1] == $remst[1] ]]; then
      print "Files are already the same size." 2>&1
      continue
    else
      # tail +<N>c takes the count of the character
      # to start from, not the offset from zero. if we did
      # this with years, then 2000 would be 1999.  no y2k bug!
      # brilliant.
      (( offs = $remst[1] + 1 ))
      if [[ $tailtype = c ]]; then
	tail +${offs}c $loc | zftp append $rem || stat=1
      else
	tail --bytes=+$offs $loc | zftp append $rem || stat=1
      fi
    fi
  fi
done

return $stat
# }
