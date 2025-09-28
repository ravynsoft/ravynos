#source: ehdr_start.s
#ld: -e _start -T ehdr_start-missing.t --no-dynamic-linker
#nm: -n
#target: *-*-linux* *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
#xfail: bfin-*-* frv-*-*

#failif
#...
.* __ehdr_start
#...
