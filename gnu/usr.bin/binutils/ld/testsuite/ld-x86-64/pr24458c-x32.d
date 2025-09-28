#source: pr24458.s
#as: --x32 -mx86-used-note=yes
#ld: -shared -m elf32_x86_64
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x[0-9a-f]+	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 feature used: x86
	x86 ISA used: 
