#source: provide-2.s
#ld: -T provide-2.t
#nm: -B
#xfail: tic54x-*-*

#...
0+3 A baz
#...
0+2000 D foo
#pass
