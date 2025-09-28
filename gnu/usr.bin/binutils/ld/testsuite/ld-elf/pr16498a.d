#ld: -shared -T pr16498a.t
#readelf: -l --wide
#target: *-*-linux* *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
  TLS .*
#...
[ ]+[0-9]+[ ]+.tdata .tbss[ ]*
#pass
