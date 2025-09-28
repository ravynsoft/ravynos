#name: --entry foo archive
#source: dummy.s
#ld: --entry foo tmpdir/libentry.a
#nm: -n

#...
[0-9a-f]+ T +foo
#...
