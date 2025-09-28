#source: maxpage5.s
#as: --32
#ld: -z max-page-size=0x200000 -T maxpage5.t --no-warn-rwx-segments
#objcopy_linked_file: -R .foo
#readelf: -l --wide
#target: x86_64-*-linux* i?86-*-linux-gnu i?86-*-gnu*

#...
Program Headers:
  Type.*
  LOAD +0x[0-9a-f]+ .*0x200000
  NOTE +0x[0-9a-f]+ .*

#...
  Segment Sections...
   00[ \t]+.text *
   01[ \t]+.note *
#pass
