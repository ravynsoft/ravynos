#source: ../ld-x86-64/pr23486c.s
#source: ../ld-x86-64/pr23486d.s
#as: --32
#ld: -r -m elf_i386
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x0000000c	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA needed: x86-64-baseline, x86-64-v2
