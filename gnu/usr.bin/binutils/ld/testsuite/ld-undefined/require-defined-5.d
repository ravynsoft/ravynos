#name: Check require-defined can require two symbols
#source: require-defined.s
#ld: -e _start --gc-sections --require-defined=bar --require-defined=foo tmpdir/libfoo.a
#nm: -n

#...
[0-9a-f]+ T foo
#...
[0-9a-f]+ T bar
#...