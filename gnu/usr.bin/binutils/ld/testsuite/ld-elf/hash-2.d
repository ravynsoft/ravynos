#source: start.s
#readelf: -d -s
#ld: -shared --hash-style=gnu -z nosectionheader
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] mips*-*-*
# MIPS uses a different style of GNU hash due to psABI restrictions
# on dynsym table ordering.

#...
 +0x[0-9a-z]+ +\(GNU_HASH\) +0x[0-9a-z]+
#pass
