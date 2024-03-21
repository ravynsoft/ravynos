#source: provide-2.s
#ld: -T provide-4.t
#nm: -B
#map: provide-4.map
#xfail: tic54x-*-*

#...
0+3 A baz
#...
0+2000 D foo
#...
0+2010 D loc2
#...
0+2030 A loc4
#pass
