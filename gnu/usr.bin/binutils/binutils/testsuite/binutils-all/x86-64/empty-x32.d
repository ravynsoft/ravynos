#source: empty.s
#PROG: objcopy
#as: --x32 -mx86-used-note=yes
#objcopy:
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x0000000c	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 feature: <None>
  GNU                  0x00000018	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA used: 
	x86 feature used: x86
