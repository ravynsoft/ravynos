#ld: -z separate-code
#objcopy_linked_file: -R .foo
#readelf: -lW
#target: *-*-linux-gnu *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
#xfail: h8300-*-* mips*-*-* rx-*-linux*

#...
 Section to Segment mapping:
  Segment Sections...
#...
   0.     
#...
   0.     .text 
#pass
