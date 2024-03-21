#source: pr23486a.s
#source: pr23486b.s
#as: --x32 -mx86-used-note=no
#ld: -r -m elf32_x86_64
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x0000000c	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA needed: i486, 586
