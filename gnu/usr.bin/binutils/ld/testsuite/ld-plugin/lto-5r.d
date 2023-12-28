#ld: -r tmpdir/lto-5a.o tmpdir/lto-5b.o
#source: dummy.s
#nm: -p

#...
[0-9a-f]+ [TD] _?foo
#pass
