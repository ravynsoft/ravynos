#source: pr24322c.s
#source: pr24322b.s
#as: --x32 -mx86-used-note=yes
#ld: -z shstk -m elf32_x86_64
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000024	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 feature: SHSTK
	x86 feature used: x86
	x86 ISA used: 
