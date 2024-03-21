#name: AArch64 mapping symbol for plt section test.
#source: plt_mapping_symbol.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#objdump: --syms --special-syms
#...

SYMBOL TABLE:
#...
[0]+10010 l       .plt	0[0]+00 \$x
#...
