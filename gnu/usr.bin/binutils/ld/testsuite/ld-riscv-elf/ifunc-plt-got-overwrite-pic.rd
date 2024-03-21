Relocation section '.rela.got' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_(32|64)[ 	]+foo2\(\)[ 	]+foo2 \+ 0
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_(32|64)[ 	]+foo1\(\)[ 	]+foo1 \+ 0
#...
Relocation section '.rela.plt' at .*
[ ]+Offset[ ]+Info[ ]+Type[ ]+.*
[0-9a-f]+[ 	]+[0-9a-f]+[ 	]+R_RISCV_JUMP_SLOT[ 	]+foo1\(\)[ 	]+foo1 \+ 0
