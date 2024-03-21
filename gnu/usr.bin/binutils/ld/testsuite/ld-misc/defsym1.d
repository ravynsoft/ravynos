#ld: -e 0 --defsym foo=1 tmpdir/start.o tmpdir/libfoo.a
#nm: -B
#source: dummy.s
#xfail: bfin-*-linux* frv-*-linux*

# Check that --defsym works on archive.
#failif
#...
0.* T bar
#pass
