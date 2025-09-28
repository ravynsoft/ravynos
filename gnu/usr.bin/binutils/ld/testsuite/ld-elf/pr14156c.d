#source: foo0.s
#source: foo1.s
#source: foo2.s
#source: foo3.s
#source: foon.s
#ld: --sort-section=alignment -T pr14156c.t
#nm: -n

#...
[0-9a-f]+ T foo
[0-9a-f]+ t foo1
[0-9a-f]+ t foo2
[0-9a-f]+ t foo3
[0-9a-f]+ t last
#pass
