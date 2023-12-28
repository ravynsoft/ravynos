#name: -u --export-dynamic-symbol "f*" archive
#source: export-dynamic-symbol.s
#ld: -pie -u foo --export-dynamic-symbol "f*" tmpdir/libpr25910.a
#nm: -D
# See comment in export-dynamic-symbol-2.d for why A is allowed here.

#...
[0-9a-f]+ [AT] +foo
#...
