#ld: -T empty-address-1.t
#nm: -n
#xfail: bfin-*-linux* frv-*-*linux*
#...
0+0 [AT] _start
#...
0+200 [ADT] __data_end
0+200 [ADT] __data_start
#pass
