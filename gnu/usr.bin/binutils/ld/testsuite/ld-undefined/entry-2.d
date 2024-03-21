#name: --entry foo -u foo archive
#source: dummy.s
#ld: --entry foo -u foo tmpdir/libentry.a
#nm: -n

#...
[0-9a-f]+ T +foo
#...
