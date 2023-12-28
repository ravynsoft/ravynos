#source: loadaddr.s
#ld: -T loadaddr3.t -z max-page-size=0x200000
#objdump: -t
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi

#...
0+0000110 g       \*ABS\*	0+0000000 data_load
#...
0+0000200 g       .data	0+0000000 data_start
#pass
