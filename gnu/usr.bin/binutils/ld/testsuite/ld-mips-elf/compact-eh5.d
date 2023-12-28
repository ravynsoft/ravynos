#name: MIPS Compact EH 5
#source: compact-eh2.s
#as: -EB
#readelf: -x .eh_frame_hdr
#ld: -EB -e main

Hex dump of section \'\.eh_frame_hdr\':

  0x[0-9a-f]+ 021b0000 00000002 ffffff[0-9a-f]+ 00000025.*
  0x[0-9a-f]+ ffffff[0-9a-f]+ 015d5d01.*
