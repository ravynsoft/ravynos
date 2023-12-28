#name: copy removing non-reloc group member
#source: group-7.s
#PROG: objcopy
#objcopy: --remove-section .data2.foo
#readelf: -Sg --wide

#...
  \[[ 0-9]+\] \.group[ \t]+GROUP[ \t]+.*
#...
  \[[ 0-9]+\] \.data\.foo[ \t]+PROGBITS[ \t0-9a-f]+WAG.*
#...
  \[[ 0-9]+\] \.rela?\.data\.foo[ \t]+RELA?[ \t0-9a-f]+IG.*
#...
COMDAT group section \[[ 0-9]+\] `\.group' \[foo\] contains 2 sections:
   \[Index\]    Name
   \[[ 0-9]+\]   \.data\.foo
   \[[ 0-9]+\]   \.rela?\.data\.foo
#pass
