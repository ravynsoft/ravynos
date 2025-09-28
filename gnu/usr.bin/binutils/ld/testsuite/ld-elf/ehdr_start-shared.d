#source: ehdr_start.s
#ld: -e _start -shared -z notext
#nm: -n
#target: *-*-linux* *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
#xfail: bfin-*-* cris*-*-* frv-*-* lm32-*-* ![check_shared_lib_support]

#...
[0-9a-f]*000 [Adrt] __ehdr_start
#pass
