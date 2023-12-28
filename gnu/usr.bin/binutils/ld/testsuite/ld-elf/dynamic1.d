#ld: -shared -T dynamic1.ld
#readelf: -l --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
 Section to Segment mapping:
  Segment Sections...
#...
   0[1-9]     .dynamic[ 	]*
#pass
