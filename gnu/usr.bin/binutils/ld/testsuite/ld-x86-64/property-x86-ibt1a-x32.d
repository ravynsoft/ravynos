#source: property-x86-empty.s
#source: property-x86-ibt.s
#as: --x32 -mx86-used-note=yes
#ld: -r -m elf32_x86_64 -Map tmpdir/property-x86-ibt1a-x32.map
#readelf: -n
#map: property-x86-ibt1a.map

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000018	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 feature used: x86
	x86 ISA used: 
