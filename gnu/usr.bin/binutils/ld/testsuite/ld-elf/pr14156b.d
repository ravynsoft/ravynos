#source: fini0.s
#source: fini1.s
#source: fini2.s
#source: fini3.s
#source: finin.s
#ld: --sort-section=alignment
#nm: -n

#...
[0-9a-f]+ [TD] foo
[0-9a-f]+ [td] foo1
#...
[0-9a-f]+ [td] foo2
[0-9a-f]+ [td] foo3
[0-9a-f]+ [td] last
#pass
