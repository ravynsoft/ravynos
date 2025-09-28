#ld: -T pr14962-2.t
#source: pr14962a.s
#nm: -n
#xfail: bfin-*-linux* frv-*-*linux*

#...
0+2000 [AT] _start
#...
0+2000 A x
#pass
