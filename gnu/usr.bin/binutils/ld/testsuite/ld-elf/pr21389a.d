#source: pr21389.s
#ld: -shared --version-script pr21389.map -soname=pr21389.so
#objdump: -p
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
Version definitions:
1 0x01 0x[0-9a-f]* pr21389.so
2 0x00 0x[0-9a-f]* FOO
#pass
