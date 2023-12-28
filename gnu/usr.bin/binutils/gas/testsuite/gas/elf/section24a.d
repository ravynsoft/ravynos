#name: Merge SHF_GNU_RETAIN for non-unique sections
#notarget: ![supports_gnu_osabi]
#source: section24.s
#readelf: -h -S --wide

#...
 +OS/ABI: +UNIX - (GNU|FreeBSD)
#...
  \[..\] .text[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 AXR .*
#...
  \[..\] .data[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 WAR .*
#...
  \[..\] .bss[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 WAR .*
#...
  \[..\] .rodata[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  AR .*
#pass

