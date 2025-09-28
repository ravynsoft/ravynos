#source: init0.s
#source: init1.s
#source: init2.s
#source: init3.s
#source: initn.s
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
