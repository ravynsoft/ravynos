#ld: -shared -z separate-code -z relro
#xfail: ![check_shared_lib_support]
#xfail: ![check_relro_support]
#readelf: -Wl

#failif
#...
 +LOAD +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ .* 0x8000
#...
 +LOAD +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ .* 0x8000
#...
