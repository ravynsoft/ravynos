#source: linkonce1a.s
#source: linkonce1b.s
#ld: -emit-relocs
#objdump: -r
#xfail: [is_generic]
# generic elf targets don't emit relocs

.*:     file format .*
#...
RELOCATION RECORDS FOR \[.debug_frame\]:
OFFSET[ 	]+TYPE[ 	]+VALUE[ 	]*
.*(NONE|unused|UNUSED).*\*ABS\*

#pass
