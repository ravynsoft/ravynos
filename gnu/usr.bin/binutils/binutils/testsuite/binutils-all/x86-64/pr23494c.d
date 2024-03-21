#source: pr23494b.s
#PROG: objcopy
#as: --64 -defsym __64_bit__=1 -mx86-used-note=no
#objcopy: -O elf32-x86-64
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x0000000c	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA used: x86-64-v2, x86-64-v4
