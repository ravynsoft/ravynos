#source: start.s
#ld: -shared -z global
#readelf: -d
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
 0x0*6ffffffb \(FLAGS_1\) *Flags: GLOBAL
#pass
