#ld: -r tmpdir/lto-3b.o
#source: dummy.s
#nm: -p

#...
[0-9a-f]+ [TD] _?foo
#pass
