#name: Link shared ifunc resolver with non-PLT caller (pie)
#source: ifunc-seperate-caller-nonplt.s
#as:
#ld: -z nocombreloc -pie tmpdir/ifunc-seperate-resolver.so
#warning: .*
#readelf: -rW

Relocation section '.rela.data' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_(32|64)[ 	]+[0-9a-f]+[ 	]+foo \+ 0
#...
Relocation section '.rela.got' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_(32|64)[ 	]+[0-9a-f]+[ 	]+foo \+ 0
