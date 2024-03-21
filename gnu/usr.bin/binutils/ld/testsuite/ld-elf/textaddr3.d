#source: maxpage1.s
#ld: -Ttext-segment 0x10000 -z max-page-size=0x200000
#target: *-*-linux-gnu *-*-gnu* arm*-*-uclinuxfdpiceabi
#warning: .*address of `text-segment' isn't multiple of maximum page size
