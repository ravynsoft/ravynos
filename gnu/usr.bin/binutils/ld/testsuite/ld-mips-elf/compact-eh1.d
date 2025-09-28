#name: MIPS Compact EH 1
#source: compact-eh1.s
#source: compact-eh1a.s
#source: compact-eh1b.s
#as: -EB
#readelf: -x .eh_frame_hdr
#ld: -EB -Tcompact-eh.ld  -e main
#

Hex dump of section \'\.eh_frame_hdr\':

  0x[0-9a-f]+ 021b0000 00000005 ffffff[0-9a-f]+ ffffff[0-9a-f]+.*
  0x[0-9a-f]+ ffffff[0-9a-f]+ ffffff[0-9a-f]+ ffffff[0-9a-f]+ 01555c5c.*
  0x[0-9a-f]+ ffffff[0-9a-f]+ 01555c5c ffffff[0-9a-f]+ 015d5d01.*

