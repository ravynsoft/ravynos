#source: pr23494a.s
#PROG: objcopy
#as: --x32 -mx86-used-note=yes
#objcopy: -O elf64-x86-64
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000030	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA needed: <unknown: 20>, <unknown: 80>
	x86 feature used: x86
	x86 ISA used: x86-64-v2, x86-64-v4, <unknown: 20>, <unknown: 80>
