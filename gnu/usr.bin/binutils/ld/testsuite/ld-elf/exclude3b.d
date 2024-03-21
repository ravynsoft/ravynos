#source: exclude3.s
#ld: --shared
#readelf: -S --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#failif
#...
[ 	]*\[.*\][ 	]+\.foo1[ 	]+.*
#...
