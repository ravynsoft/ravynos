#source: maxpage1.s
#ld: -shared -z max-page-size=0x200000 -Ttext-segment 0x10000
#target: *-*-linux-gnu *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support]
#warning: .*address of `text-segment' isn't multiple of maximum page size
