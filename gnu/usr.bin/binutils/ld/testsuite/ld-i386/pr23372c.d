#source: ../ld-x86-64/pr23372d.s
#source: ../ld-x86-64/pr23372e.s
#as: --32
#ld: -r -m elf_i386
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA used: 
