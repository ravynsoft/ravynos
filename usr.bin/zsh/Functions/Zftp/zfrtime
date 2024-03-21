# function zfrtime {
# Set the modification time of file LOCAL to that of REMOTE.
# If the optional TIME is passed, it should be in the FTP format
# CCYYMMDDhhmmSS, i.e. no dot before the seconds, and in GMT.
# This is what both `zftp remote' and `zftp local' return.
#
# Unfortunately, since the time returned from FTP is GMT and
# your file needs to be set in local time, we need to do some
# hacking around with time.

emulate -L zsh
zmodload zsh/datetime

local time gmtime loctime year mon mday hr min sec y tmpdate
local -i days_since_epoch

if [[ -n $3 ]]; then
  time=$3
else
  time=($(zftp remote $2 2>/dev/null))
  [[ -n $time ]] && time=$time[2]
fi
[[ -z $time ]] && return 1

year=$time[1,4]
mon=$time[5,6]
mday=$time[7,8]
hr=$time[9,10]
min=$time[11,12]
sec=$time[13,14]

#count the number of days since epoch without the current day
for y  in {1970..$(( $year - 1))}; do
  strftime -s tmpdate -r "%Y/%m/%d" ${y}/12/31
  days_since_epoch+=$(strftime "%j" $tmpdate)
done
strftime -s tmpdate -r "%Y/%m/%d" $year/$mon/$(( $mday - 1 ))
days_since_epoch+=$(strftime "%j" $tmpdate)
# convert the time in number of seconds (this should be equivalent to timegm)
time=$(( $sec + 60 * ( $min + 60 * ($hr + 24 * $days_since_epoch))  ))
#Convert it back to CCYYMMDDhhmmSS
strftime -s time "%Y%m%d%H%M%S" ${EPOCHSECONDS}
touch -t ${time[1,12]}.${time[13,14]} $1
