#source: empty-address-2.s
#ld: -Ttext 0x0000000 -Tdata 0x200 -T empty-address-2b.t
#nm: -n
#xfail: bfin-*-linux* frv-*-*linux* riscv64*-*-* tic54x-*-*
#...
0+0 T _start
#...
0+10 [ADT] __data_end
#pass
