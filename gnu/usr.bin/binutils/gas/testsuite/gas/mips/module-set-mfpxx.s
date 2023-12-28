.module mips32r2
.module fp=xx
.module doublefloat
.module hardfloat
.module oddspreg

add.s $f1,$f1,$f1
.set push
.set fp=32
add.s $f1,$f1,$f1
.set pop

.set push
.set fp=64
add.d $f1,$f1,$f1
.set pop
