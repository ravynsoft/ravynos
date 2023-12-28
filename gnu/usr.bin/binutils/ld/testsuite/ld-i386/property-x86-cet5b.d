#source: property-x86-empty.s
#as: --32 -mx86-used-note=yes
#ld: -r -m elf_i386 -z cet-report=error -z ibt -z shstk
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000024	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 feature: IBT, SHSTK
	x86 feature used: x86
	x86 ISA used: 
