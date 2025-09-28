#source: pr16498a.s
#ld: -shared -T pr16498b.t
#readelf: -l --wide
#target: *-*-linux* *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
  TLS .*
#...
[ ]+[0-9]+[ ]+tls_data_init .tbss[ ]*
#pass
