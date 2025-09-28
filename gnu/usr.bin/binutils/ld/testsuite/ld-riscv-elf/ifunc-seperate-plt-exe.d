#name: Link shared ifunc resolver with PLT caller (exe)
#source: ifunc-seperate-caller-plt.s
#as:
#ld: -z nocombreloc tmpdir/ifunc-seperate-resolver.so
#warning: .*
#readelf: -rW

Relocation section '.rela.got' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_(32|64)[ 	]+[0-9a-f]+[ 	]+foo \+ 0
#...
Relocation section '.rela.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_JUMP_SLOT[ 	]+[0-9a-f]+[ 	]+foo \+ 0
