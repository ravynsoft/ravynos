#source: pr23372a.s
#source: pr23372b.s
#as: --64 -defsym __64_bit__=1
#ld: -r -m elf_x86_64
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA used: 
