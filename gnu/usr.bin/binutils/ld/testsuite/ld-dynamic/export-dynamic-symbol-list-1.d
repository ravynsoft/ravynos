#name: --export-dynamic-symbol-list foo archive
#source: export-dynamic-symbol.s
#ld: -pie --export-dynamic-symbol-list foo.list tmpdir/libpr25910.a
#nm: -n

#failif
#...
[0-9a-f]+ T +foo
#...
