#source: pr22836-2.s
#ld: -r -S
#readelf: -g --wide

group section \[[ 0-9]+\] `\.group' \[foo\] contains 1 section.*
   \[Index\]    Name
   \[[ 0-9]+\]   \.comment
