#ld: -Tnote-2.t --no-warn-rwx-segments
#objcopy_linked_file: -R .foo 
#readelf: -l --wide

#...
Program Headers:
  Type.*
  LOAD +0x[0-9a-f]+ .*
  NOTE +0x[0-9a-f]+ .*

#...
  Segment Sections...
   00[ \t]+.text *
   01[ \t]+.note *
#pass
