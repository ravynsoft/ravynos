#source: start.s
#as: --x32 -mx86-used-note=yes
#ld: -r -m elf32_x86_64 -z ibt
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000024	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 feature: IBT
	x86 feature used: x86
	x86 ISA used: 
