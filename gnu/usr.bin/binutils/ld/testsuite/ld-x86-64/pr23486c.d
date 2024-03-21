#source: pr23486c.s
#source: pr23486d.s
#as: --64 -defsym __64_bit__=1
#ld: -r -m elf_x86_64
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000010	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA needed: x86-64-baseline, x86-64-v2
