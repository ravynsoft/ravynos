#ld: -shared
#readelf: -W -x .strtab
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#failif
#...
 +0x[0-9 ]+.*\.xxxx\..*
#...
