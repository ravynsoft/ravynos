#source: pr22393-1.s
#ld: -shared -z relro -z separate-code
#readelf: -l --wide
#target: *-*-linux-gnu *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 
#xfail: ![check_relro_support]

#failif
#...
 +[0-9]+  +.*.text.*(.eh_frame|\.rodata).*
#...
