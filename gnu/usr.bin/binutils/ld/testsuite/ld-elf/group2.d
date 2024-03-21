#source: ../../../binutils/testsuite/binutils-all/group.s
#ld: -r
#readelf: -Sg --wide
# xstormy uses a non-standard script, putting .data before .text.
#xfail: xstormy*-*-*

#...
  \[[ 0-9]+\] \.group[ \t]+GROUP[ \t]+.*
#...
  \[[ 0-9]+\] \.text.*[ \t]+PROGBITS[ \t0-9a-f]+AXG.*
#...
  \[[ 0-9]+\] \.data.*[ \t]+PROGBITS[ \t0-9a-f]+WAG.*
#...
COMDAT group section \[[ 0-9]+\] `\.group' \[foo_group\] contains . sections:
   \[Index\]    Name
   \[[ 0-9]+\]   .text.*
#...
   \[[ 0-9]+\]   .data.*
#pass
