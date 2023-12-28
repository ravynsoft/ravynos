#name: Check require-defined can require a symbol from an archive
#source: require-defined.s
#ld: -e _start --require-defined=foo tmpdir/libfoo.a
#nm: -n

#...
[0-9a-f]+ T foo
#...
