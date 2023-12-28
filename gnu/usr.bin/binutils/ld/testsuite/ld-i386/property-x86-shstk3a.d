#source: property-x86-3.s
#source: property-x86-shstk.s
#as: --32 -mx86-used-note=yes
#ld: -r -melf_i386
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000024	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA needed: x86-64-v2, x86-64-v4, <unknown: 20>, <unknown: 80>
	x86 feature used: x86
	x86 ISA used: x86-64-baseline, x86-64-v2, <unknown: 10>, <unknown: 20>
