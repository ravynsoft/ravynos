#name: MIPS Compact EH 3
#source: compact-eh3.s
#source: compact-eh3a.s
#as: -EB
#readelf: -x .eh_frame_hdr
#ld: -EB -Tcompact-eh.ld  -e main
#

Hex dump of section \'\.eh_frame_hdr\':

  0x[0-9a-f]+ 021b0000 00000004 ffffff[0-9a-f]+ ffffff[0-9a-f][0-9a-f].*
  0x[0-9a-f]+ ffffff[0-9a-f]+ 00000041 ffffff[0-9a-f]+ 0000004d.*
  0x[0-9a-f]+ ffffff[0-9a-f]+ 015d5d01.*
