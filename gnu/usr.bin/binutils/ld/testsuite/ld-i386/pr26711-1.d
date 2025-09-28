#source: ../ld-x86-64/pr26711.s
#source: ../ld-x86-64/start.s
#as: --32 -mx86-used-note=no
#ld: -m elf_i386 -z ibt
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 feature: IBT
