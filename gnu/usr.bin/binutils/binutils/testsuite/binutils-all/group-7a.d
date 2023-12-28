#name: copy removing reloc group member
#source: group-7.s
#PROG: objcopy
#objcopy: --remove-section .data.foo
#readelf: -Sg --wide

#...
  \[[ 0-9]+\] \.group[ \t]+GROUP[ \t]+.*
#...
  \[[ 0-9]+\] \.data2\.foo[ \t]+PROGBITS[ \t0-9a-f]+WAG.*
#...
COMDAT group section \[[ 0-9]+\] `\.group' \[foo\] contains 1 section.*
   \[Index\]    Name
   \[[ 0-9]+\]   \.data2\.foo
#pass
