#name: Merge SHF_GNU_RETAIN for non-unique sections (check no unmerged)
#notarget: ![supports_gnu_osabi]
#source: section24.s
#readelf: -S --wide

#...
  \[..\] .text[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +AX .*
#...
  \[..\] .data[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +WA .*
#...
  \[..\] .bss[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +WA .*
#...
  \[..\] .rodata[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +A .*
#pass
