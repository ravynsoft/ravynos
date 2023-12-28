#source: start.s
#readelf: -d -s -D
#ld: -shared --hash-style=gnu
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 
#xfail: mips*-*-*
# MIPS uses a different style of GNU hash due to psABI restrictions
# on dynsym table ordering.

#...
 +0x[0-9a-z]+ +\(GNU_HASH\) +0x[0-9a-z]+
#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] _start
#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] main
#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] start
#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +NOTYPE +GLOBAL +DEFAULT +[1-9] __start
#...
