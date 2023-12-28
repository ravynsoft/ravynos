#name: Link shared ifunc resolver with non-PLT caller (pic)
#source: ifunc-seperate-caller-nonplt.s
#as:
#ld: -z nocombreloc -shared tmpdir/ifunc-seperate-resolver.so
#readelf: -rW

Relocation section '.rela.data' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_(32|64)[ 	]+[0-9a-f]+[ 	]+foo \+ 0
#...
Relocation section '.rela.got' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_(32|64)[ 	]+[0-9a-f]+[ 	]+foo \+ 0
